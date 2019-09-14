/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright 2004-2019 Igor van den Hoven                                    *
*                                                                             *
*   TinTin++ is free software; you can redistribute it and/or modify          *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 3 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with TinTin++.  If not, see https://www.gnu.org/licenses.           *
******************************************************************************/

/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                      coded by Igor van den Hoven 2006                       *
******************************************************************************/

#include "tintin.h"

void process_input(void)
{
	if (HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_SGA) && !HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO))
	{
		read_key();
	}
	else
	{
		read_line();
	}

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_PROCESSINPUT))
	{
		return;
	}

	DEL_BIT(gtd->flags, TINTIN_FLAG_PROCESSINPUT);

	if (gtd->chat && gtd->chat->paste_time)
	{
		chat_paste(gtd->input_buf, NULL);

		return;
	}

	if (HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO))
	{
		add_line_history(gtd->ses, gtd->input_buf);
	}

	if (HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO))
	{
		echo_command(gtd->ses, gtd->input_buf);
	}
	else
	{
		echo_command(gtd->ses, "");
	}

	if (gtd->ses->scroll->line != -1)
	{
		buffer_end(gtd->ses, "");
	}

	check_all_events(gtd->ses, SUB_ARG|SUB_SEC, 0, 1, "RECEIVED INPUT", gtd->input_buf);

	if (check_all_events(gtd->ses, SUB_ARG|SUB_SEC, 0, 1, "CATCH RECEIVED INPUT", gtd->input_buf) == 1)
	{
		return;
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK))
	{
		write_mud(gtd->ses, gtd->input_buf, SUB_EOL);
	}
	else
	{
		gtd->ses = script_driver(gtd->ses, LIST_COMMAND, gtd->input_buf);
	}

	if (IS_SPLIT(gtd->ses))
	{
		erase_toeol();
	}

	gtd->input_buf[0] = 0;
}

void read_line()
{
	char buffer[STRING_SIZE];
	struct listnode *node;
	struct listroot *root;
	int len, cnt, size, match, val[3], width, index;

	gtd->input_buf[gtd->input_len] = 0;

	len = read(0, buffer, 1);

	buffer[len] = 0;

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA) || HAS_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR))
	{
		convert_meta(buffer, &gtd->macro_buf[strlen(gtd->macro_buf)], FALSE);
	}
	else
	{
		strcat(gtd->macro_buf, buffer);
	}

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA))
	{
		match = 0;
		root  = gtd->ses->list[LIST_MACRO];

		if (!HAS_BIT(root->flags, LIST_FLAG_IGNORE))
		{
			for (root->update = 0 ; root->update < root->used ; root->update++)
			{
				node = root->list[root->update];

				if (!strcmp(gtd->macro_buf, node->arg3))
				{
					script_driver(gtd->ses, LIST_MACRO, node->arg2);

					gtd->macro_buf[0] = 0;

					return;
				}
				else if (!strncmp(gtd->macro_buf, node->arg3, strlen(gtd->macro_buf)))
				{
					match = 1;
				}
			}
		}

		for (cnt = 0 ; *cursor_table[cnt].fun != NULL ; cnt++)
		{
			if (*cursor_table[cnt].code)
			{
				if (!strcmp(gtd->macro_buf, cursor_table[cnt].code))
				{
					cursor_table[cnt].fun(gtd->ses, "");
					gtd->macro_buf[0] = 0;

					return;
				}
				else if (!strncmp(gtd->macro_buf, cursor_table[cnt].code, strlen(gtd->macro_buf)))
				{
					match = 1;
				}
			}
		}

		if (match)
		{
			return;
		}

		if (gtd->macro_buf[0] == ESCAPE)
		{
			if (gtd->macro_buf[1] == '[')
			{
				if (gtd->macro_buf[2] == '<')
				{
					val[0] = val[1] = val[2] = cnt = buffer[0] = 0;

					for (len = 3 ; gtd->macro_buf[len] ; len++)
					{
						if (isdigit(gtd->macro_buf[len]))
						{
							cat_sprintf(buffer, "%c", gtd->macro_buf[len]);
						}
						else
						{
							switch (gtd->macro_buf[len])
							{
								case ';':
									val[cnt++] = get_number(gtd->ses, buffer);
									buffer[0] = 0;
									break;

								case 'm':
								case 'M':
									val[cnt++] = get_number(gtd->ses, buffer);
									mouse_handler(gtd->ses, val[0], val[2], val[1], gtd->macro_buf[len]); // swap x y to row col
									gtd->macro_buf[0] = 0;
									return;

								default:
									printf("unknownmouse input error (%s)\n", gtd->macro_buf);
									gtd->macro_buf[0] = 0;
									return;
							}
						}
					}
					return;
				}

				if (gtd->macro_buf[2] >= '0' && gtd->macro_buf[2] <= '9')
				{
					val[0] = val[1] = val[2] = cnt = buffer[0] = 0;

					for (len = 2 ; gtd->macro_buf[len] ; len++)
					{
						if (isdigit(gtd->macro_buf[len]))
						{
							cat_sprintf(buffer, "%c", gtd->macro_buf[len]);
						}
						else
						{
							switch (gtd->macro_buf[len])
							{
								case '-':
									if (buffer[0] == 0)
									{
										strcat(buffer, "-");
									}
									else
									{
										tintin_printf2(NULL, "\e[1;31merror: bad csi input (%s)\n", &gtd->macro_buf[1]);
										gtd->macro_buf[0] = 0;
										continue;
									}
									break;
								case ';':
									val[cnt++] = get_number(gtd->ses, buffer);
									buffer[0] = 0;
									break;

								case 't':
									val[cnt++] = get_number(gtd->ses, buffer);
									csit_handler(val[0], val[1], val[2]);
									gtd->macro_buf[0] = 0;
									return;

								default:
									goto end_of_loop;
							}
						}
					}
					return;
				}
			}
			else if (gtd->macro_buf[1] == ']')
			{
				switch (gtd->macro_buf[2])
				{
					case 'L':
					case 'l':
						for (len = 3 ; gtd->macro_buf[len] ; len++)
						{
							if (gtd->macro_buf[len] < 32)
							{
								buffer[len - 3] = 0;
								osc_handler(gtd->macro_buf[2], buffer);
								gtd->macro_buf[0] = 0;
								return;
							}
							else
							{
								buffer[len - 3 ] = gtd->macro_buf[len];
							}
						}
						break;

					default:
						printf("\e[1;31merror: unknown osc input (%s)\n", gtd->macro_buf);
						gtd->macro_buf[0] = 0;
						return;
				}
			}
		}
	}

	end_of_loop:

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) && is_utf8_head(gtd->macro_buf))
	{
		if (get_utf8_size(gtd->macro_buf) == 1)
		{
			return;
		}
	}

	if (gtd->macro_buf[0] == ESCAPE)
	{
		strcpy(buffer, gtd->macro_buf);

		convert_meta(buffer, gtd->macro_buf, FALSE);
	}

	get_utf8_index(gtd->macro_buf, &index);

	check_all_events(gtd->ses, SUB_ARG, 0, 2, "RECEIVED KEYPRESS", gtd->macro_buf, ntos(index));

	if (check_all_events(gtd->ses, SUB_ARG, 0, 2, "CATCH RECEIVED KEYPRESS", gtd->macro_buf, ntos(index)) == 1)
	{
		gtd->macro_buf[0] = 0;
		return;
	}

	while (gtd->macro_buf[0])
	{
		switch (gtd->macro_buf[0])
		{
			case '\r':
			case '\n':
				cursor_enter(gtd->ses, "");

				memmove(gtd->macro_buf, &gtd->macro_buf[1], 1 + strlen(&gtd->macro_buf[1]));
				break;

			default:
				if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) && gtd->macro_buf[0] & 128 && gtd->macro_buf[1])
				{
					size = get_utf8_width(gtd->macro_buf, &width);

					if (HAS_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT) && gtd->input_len != gtd->input_cur)
					{
						if (width)
						{
							cursor_delete(gtd->ses, "");
						}
					}

					ins_sprintf(&gtd->input_buf[gtd->input_cur], "%.*s", size, gtd->macro_buf);

					gtd->input_pos += width;
					gtd->input_cur += size;
					gtd->input_len += size;

					if (width && gtd->input_len != gtd->input_cur)
					{
						input_printf("\e[%d@%.*s", width, size, gtd->macro_buf);
					}
					else
					{
						input_printf("%.*s", size, gtd->macro_buf);
					}
					memmove(gtd->macro_buf, &gtd->macro_buf[size], 1 + strlen(&gtd->macro_buf[size]));
				}
				else
				{
					if (HAS_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT) && gtd->input_len != gtd->input_cur)
					{
						cursor_delete(gtd->ses, "");
					}

					ins_sprintf(&gtd->input_buf[gtd->input_cur], "%c", gtd->macro_buf[0]);

					gtd->input_len++;
					gtd->input_cur++;
					gtd->input_pos++;

					if (gtd->input_len != gtd->input_cur)
					{
						input_printf("\e[1@%c", gtd->macro_buf[0]);
					}
					else
					{
						input_printf("%c", gtd->macro_buf[0]);
					}
					memmove(gtd->macro_buf, &gtd->macro_buf[1], 1 + strlen(&gtd->macro_buf[1]));
				}

//				gtd->macro_buf[0] = 0;
				gtd->input_tmp[0] = 0;
				gtd->input_buf[gtd->input_len] = 0;

				cursor_check_line_modified(gtd->ses, "");

				DEL_BIT(gtd->flags, TINTIN_FLAG_HISTORYBROWSE);

				kill_list(gtd->ses->list[LIST_COMMAND]);

				if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
				{
					cursor_history_find(gtd->ses, "");
				}
				break;
		}
	}
}

void read_key(void)
{
	char buffer[BUFFER_SIZE];
	struct listnode *node;
	struct listroot *root;
	int len, cnt, match;

	if (gtd->input_buf[0] == gtd->tintin_char)
	{
		read_line();

		return;
	}

	len = read(0, buffer, 1);

	buffer[len] = 0;

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA) || HAS_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR))
	{
		convert_meta(buffer, &gtd->macro_buf[strlen(gtd->macro_buf)], FALSE);
	}
	else
	{
		strcat(gtd->macro_buf, buffer);
	}

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA))
	{
		match = 0;

		root  = gtd->ses->list[LIST_MACRO];

		if (!HAS_BIT(root->flags, LIST_FLAG_IGNORE))
		{
			for (root->update = 0 ; root->update < root->used ; root->update++)
			{
				node = root->list[root->update];

				if (!strcmp(gtd->macro_buf, node->arg3))
				{
					script_driver(gtd->ses, LIST_MACRO, node->arg2);

					gtd->macro_buf[0] = 0;
					return;
				}
				else if (!strncmp(gtd->macro_buf, node->arg3, strlen(gtd->macro_buf)))
				{
					match = 1;
				}
			}
		}

		if (match)
		{
			return;
		}
	}

	for (cnt = 0 ; gtd->macro_buf[cnt] ; cnt++)
	{
		switch (gtd->macro_buf[cnt])
		{
			case '\n':
				gtd->input_buf[0] = 0;
				gtd->macro_buf[0] = 0;
				gtd->input_len = 0;

				if (HAS_BIT(gtd->ses->flags, SES_FLAG_RUN))
				{
					socket_printf(gtd->ses, 1, "%c", '\r');
				}
				else
				{
					socket_printf(gtd->ses, 2, "%c%c", '\r', '\n');
				}
				break;

			default:
				if (gtd->macro_buf[cnt] == gtd->tintin_char && gtd->input_buf[0] == 0)
				{
					if (gtd->input_len != gtd->input_cur)
					{
						printf("\e[1@%c", gtd->macro_buf[cnt]);
					}
					else
					{
						printf("%c", gtd->macro_buf[cnt]);
					}
					gtd->input_buf[0] = gtd->tintin_char;
					gtd->input_buf[1] = 0;
					gtd->macro_buf[0] = 0;
					gtd->input_len = 1;
					gtd->input_cur = 1;
					gtd->input_pos = 1;
				}
				else
				{
					socket_printf(gtd->ses, 1, "%c", gtd->macro_buf[cnt]);
					gtd->input_buf[0] = 127;
					gtd->macro_buf[0] = 0;
					gtd->input_len = 0;
				}
				break;
		}
	}
}

void convert_meta(char *input, char *output, int eol)
{
	char *pti, *pto;

	push_call("convert_meta(%p,%p,%d)",input,output,eol);

	DEL_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR);

	pti = input;
	pto = output;

	while (*pti)
	{
		switch (*pti)
		{
			case ESCAPE:
				*pto++ = '\\';
				*pto++ = 'e';
				pti++;
				break;

			case 127:
				*pto++ = '\\';
				*pto++ = 'x';
				*pto++ = '7';
				*pto++ = 'F';
				pti++;
				break;

			case '\a':
				*pto++ = '\\';
				*pto++ = 'a';
				pti++;
				break;

			case '\b':
				*pto++ = '\\';
				*pto++ = 'b';
				pti++;
				break;

			case '\f':
				*pto++ = '\\';
				*pto++ = 'f';
				pti++;
				break;

			case '\t':
				*pto++ = '\\';
				*pto++ = 't';
				pti++;
				break;

			case '\r':
				if (eol)
				{
					*pto++ = '\\';
					*pto++ = 'r';
				}
				*pto++ = *pti++;
				break;

			case '\v':
				*pto++ = '\\';
				*pto++ = 'v';
				pti++;
				break;

			case '\n':
				if (eol)
				{
					*pto++ = '\\';
					*pto++ = 'n';
				}
				*pto++ = *pti++;
				break;

			default:
				if (*pti > 0 && *pti < 32)
				{
					*pto++ = '\\';
					*pto++ = 'c';
					if (*pti <= 26)
					{
						*pto++ = 'a' + *pti - 1;
					}
					else
					{
						*pto++ = 'A' + *pti - 1;
					}
					pti++;
					break;
				}
				else
				{
					*pto++ = *pti++;
				}
				break;
		}
	}
	*pto = 0;

	pop_call();
	return;
}

char *str_convert_meta(char *input, int eol)
{
	static char buf[BUFFER_SIZE];

	convert_meta(input, buf, eol);

	return buf;
}

void unconvert_meta(char *input, char *output)
{
	char *pti, *pto;

	pti = input;
	pto = output;

	while (*pti)
	{
		switch (pti[0])
		{
			case '\\':
				switch (pti[1])
				{
					case 'C':
						if (pti[2] == '-' && pti[3])
						{
							*pto++  = pti[3] - 'a' + 1;
							pti    += 4;
						}
						else
						{
							*pto++ = *pti++;
						}
						break;

					case 'c':
						*pto++ = pti[2] % 32;
						pti += 3;
						break;

					case 'a':
						*pto++  = '\a';
						pti += 2;
						break;

					case 'b':
						*pto++  = 127;
						pti    += 2;
						break;

					case 'e':
						*pto++  = ESCAPE;
						pti    += 2;
						break;

					case 't':
						*pto++  = '\t';
						pti    += 2;
						break;

					case 'x':
						if (pti[2] && pti[3])
						{
							*pto++ = hex_number_8bit(&pti[2]);
							pti += 4;
						}
						else
						{
							*pto++ = *pti++;
						}
						break;
					default:
						*pto++ = *pti++;
						break;
				}
				break;

			default:
				*pto++ = *pti++;
				break;
		}
	}
	*pto = 0;
}

/*
	Currenly only used in split mode.
*/

void echo_command(struct session *ses, char *line)
{
	char buffer[STRING_SIZE], result[STRING_SIZE];

	if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
	{
		if (HAS_BIT(ses->flags, SES_FLAG_ECHOCOMMAND))
		{
			sprintf(buffer, "%s%s\e[0m", ses->cmd_color, line);
		}
		else
		{
			sprintf(buffer, "\e[0m");
		}
	}
	else
	{
		sprintf(buffer, "%s", line);
	}

	/*
		Deal with pending output
	*/

	if (ses->more_output[0])
	{
		if (ses->check_output)
		{
			strcpy(result, ses->more_output);
			ses->more_output[0] = 0;

			process_mud_output(ses, result, FALSE);
		}
	}

	DEL_BIT(ses->telopts, TELOPT_FLAG_PROMPT);

	if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
	{
		if (HAS_BIT(ses->flags, SES_FLAG_ECHOCOMMAND))
		{
			sprintf(result, "%s%s", ses->more_output, buffer);
		}
		else
		{
			if (strip_vt102_strlen(ses, ses->more_output) == 0)
			{
				return;
			}
			sprintf(result, "%s", ses->more_output);
		}

		add_line_buffer(ses, buffer, -1);

		gtd->scroll_level++;

		tintin_printf2(ses, "%s", result);

		gtd->scroll_level--;
	}
	else
	{
		add_line_buffer(ses, buffer, -1);
		add_line_screen(buffer);
	}
}

void input_printf(char *format, ...)
{
	char buf[STRING_SIZE];
	va_list args;

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		if (!HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO) && gtd->input_buf[0] != gtd->tintin_char)
		{
			return;
		}
	}

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	printf("%s", buf);
}

void modified_input(void)
{
	kill_list(gtd->ses->list[LIST_COMMAND]);

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		cursor_history_find(gtd->ses, "");
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYBROWSE))
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_HISTORYBROWSE);
	}

}
