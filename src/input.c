/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright 2004-2020 Igor van den Hoven                                    *
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
*   You should have received a copy of the GNU General Public License         *
*   along with TinTin++.  If not, see https://www.gnu.org/licenses.           *
******************************************************************************/

/******************************************************************************
*                               T I N T I N + +                               *
*                                                                             *
*                      coded by Igor van den Hoven 2006                       *
******************************************************************************/

#include "tintin.h"

void process_input(void)
{
	char input[STRING_SIZE];
	int len, out;

	push_call("process_input(void)");

	gtd->time_input = gtd->time;

	if (gtd->detach_port)
	{
		if (gtd->detach_sock > 0)
		{
			len = read(gtd->detach_sock, input, 1);

			if (len <= 0)
			{
				if (len == -1)
				{
					syserr_printf(gtd->ses, "process_input: read", gtd->detach_sock);
				}
				else
				{
					show_message(gtd->ses, LIST_COMMAND, "#DAEMON UPDATE: DETACHING FROM PID {%d} DUE TO READ FAILURE.", gtd->detach_pid);
				}

				gtd->detach_sock = close(gtd->detach_sock);

				pop_call();
				return;
			}
		}
		else
		{
			printf("process_input: error?\n");
		}
	}
	else
	{
		len = read(0, input, 1);
	}

	input[len] = 0;

	if (gtd->attach_sock)
	{
		out = write(gtd->attach_sock, input, 1);

		if (out >= 0)
		{
			pop_call();
			return;
		}

		gtd->attach_sock = close(gtd->attach_sock);

		show_message(gtd->ses, LIST_COMMAND, "#DAEMON ATTACH: WRITE ERROR: UNATTACHING.");
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR))
	{
		if (gtd->convert_time == 0)
		{
			gtd->convert_time = 100000LL + utime();
		}
		else
		{
			if (gtd->convert_time < gtd->utime)
			{
				gtd->convert_time = 0;
				DEL_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR);
			}
		}
	}

	if (HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_SGA) && !HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO))
	{
		read_key(input, len);
	}
	else
	{
		read_line(input, len);
	}

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_PROCESSINPUT))
	{
		pop_call();
		return;
	}

	DEL_BIT(gtd->flags, TINTIN_FLAG_PROCESSINPUT);

	if (gtd->chat && gtd->chat->paste_time)
	{
		chat_paste(gtd->ses->input->buf, NULL);

		pop_call();
		return;
	}

	if (HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO))
	{
		add_line_history(gtd->ses, gtd->ses->input->buf);
	}

	if (HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO))
	{
		echo_command(gtd->ses, gtd->ses->input->buf);
	}
	else
	{
		echo_command(gtd->ses, "");
	}

	if (gtd->ses->scroll->line != -1)
	{
		buffer_end(gtd->ses, "");
	}

	check_all_events(gtd->ses, SUB_ARG|SUB_SEC, 0, 1, "RECEIVED INPUT", gtd->ses->input->buf);

	if (check_all_events(gtd->ses, SUB_ARG|SUB_SEC, 0, 1, "CATCH RECEIVED INPUT", gtd->ses->input->buf) == 1)
	{
		pop_call();
		return;
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK))
	{
		parse_input(gtd->ses, gtd->ses->input->buf);
	}
	else
	{
		gtd->ses = script_driver(gtd->ses, LIST_COMMAND, gtd->ses->input->buf);
	}

	if (IS_SPLIT(gtd->ses))
	{
		erase_toeol();
	}

	gtd->ses->input->buf[0] = 0;

	fflush(NULL);

	pop_call();
	return;
}

void read_line(char *input, int len)
{
	int size, width, index;

//	gtd->ses->input->buf[gtd->ses->input->len] = 0;

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA) || gtd->level->convert)
	{
		convert_meta(input, &gtd->macro_buf[strlen(gtd->macro_buf)], FALSE);
	}
	else if (HAS_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR))
	{
		convert_meta(input, &gtd->macro_buf[strlen(gtd->macro_buf)], TRUE);
	}
	else
	{
		strcat(gtd->macro_buf, input);
	}

	get_utf8_index(input, &index);

	if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(gtd->macro_buf))
	{
		if (get_utf8_size(gtd->macro_buf) == 1)
		{
			return;
		}
	}

	check_all_events(gtd->ses, SUB_ARG|SUB_SIL, 0, 2, "RECEIVED KEYPRESS", input, ntos(index));

	if (check_all_events(gtd->ses, SUB_ARG|SUB_SIL, 0, 2, "CATCH RECEIVED KEYPRESS", input, ntos(index)) == 1)
	{
		return;
	}

	if (check_key(input, len))
	{
		return;
	}

	if (gtd->macro_buf[0] == ASCII_ESC)
	{
		strcpy(input, gtd->macro_buf);

		convert_meta(input, gtd->macro_buf, FALSE);
	}

	while (gtd->macro_buf[0])
	{
		switch (gtd->macro_buf[0])
		{
			case ASCII_CR:
			case ASCII_LF:
				cursor_enter(gtd->ses, "");

				memmove(gtd->macro_buf, &gtd->macro_buf[1], 1 + strlen(&gtd->macro_buf[1]));
				break;

			default:
				if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8) && gtd->macro_buf[0] & 128 && gtd->macro_buf[1])
				{
					size = get_utf8_width(gtd->macro_buf, &width);

					if (HAS_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT) && gtd->ses->input->len != gtd->ses->input->cur)
					{
						if (width)
						{
							cursor_delete(gtd->ses, "");
						}
					}

					ins_sprintf(&gtd->ses->input->buf[gtd->ses->input->cur], "%.*s", size, gtd->macro_buf);

					gtd->ses->input->pos += width;
					gtd->ses->input->cur += size;
					gtd->ses->input->len += size;

					if (width && gtd->ses->input->len != gtd->ses->input->cur)
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
					if (HAS_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT) && gtd->ses->input->len != gtd->ses->input->cur)
					{
						cursor_delete(gtd->ses, "");
					}

					ins_sprintf(&gtd->ses->input->buf[gtd->ses->input->cur], "%c", gtd->macro_buf[0]);

					gtd->ses->input->len++;
					gtd->ses->input->cur++;
					gtd->ses->input->pos++;

					if (gtd->ses->input->len != gtd->ses->input->cur)
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
				gtd->ses->input->tmp[0] = 0;
				gtd->ses->input->buf[gtd->ses->input->len] = 0;

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

void read_key(char *input, int len)
{
	int cnt;

	if (gtd->ses->input->buf[0] == gtd->tintin_char)
	{
		read_line(input, len);

		return;
	}

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA))
	{
		convert_meta(input, &gtd->macro_buf[strlen(gtd->macro_buf)], FALSE);
	}
	else if (HAS_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR))
	{
		convert_meta(input, &gtd->macro_buf[strlen(gtd->macro_buf)], TRUE);
	}
	else
	{
		strcat(gtd->macro_buf, input);
	}

	if (check_key(input, len))
	{
		return;
	}

	for (cnt = 0 ; gtd->macro_buf[cnt] ; cnt++)
	{
		switch (gtd->macro_buf[cnt])
		{
			case ASCII_CR:
			case ASCII_LF:
				gtd->ses->input->buf[0] = 0;
				gtd->macro_buf[0] = 0;
				gtd->ses->input->len = 0;

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
				if (gtd->macro_buf[cnt] == gtd->tintin_char && gtd->ses->input->buf[0] == 0)
				{
					if (gtd->ses->input->len != gtd->ses->input->cur)
					{
						print_stdout("\e[1@%c", gtd->macro_buf[cnt]);
					}
					else
					{
						print_stdout("%c", gtd->macro_buf[cnt]);
					}
					gtd->ses->input->buf[0] = gtd->tintin_char;
					gtd->ses->input->buf[1] = 0;
					gtd->macro_buf[0] = 0;
					gtd->ses->input->len = 1;
					gtd->ses->input->cur = 1;
					gtd->ses->input->pos = 1;
				}
				else
				{
					socket_printf(gtd->ses, 1, "%c", gtd->macro_buf[cnt]);
					gtd->ses->input->buf[0] = 127;
					gtd->macro_buf[0] = 0;
					gtd->ses->input->len = 0;
				}
				break;
		}
	}
}

int check_key(char *input, int len)
{
	char buf[BUFFER_SIZE];
	struct listroot *root;
	struct listnode *node;
	int cnt, val[5], partial;

	push_call("check_key(%p,%d)",input,len);

//	tintin_printf2(gtd->ses, "check_key(%d,%s)",*gtd->macro_buf, gtd->macro_buf);

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA))
	{
		root = gtd->ses->list[LIST_MACRO];

		if (!HAS_BIT(root->flags, LIST_FLAG_IGNORE))
		{
			partial = 0;

			for (root->update = 0 ; root->update < root->used ; root->update++)
			{
				node = root->list[root->update];

				if (*node->arg1 == '^' && gtd->ses->input->len)
				{
					continue;
				}
				else if (!strcmp(gtd->macro_buf, node->arg4))
				{
					strcpy(buf, node->arg2);

					if (node->shots && --node->shots == 0)
					{
						delete_node_list(gtd->ses, LIST_MACRO, node);
					}

					script_driver(gtd->ses, LIST_MACRO, buf);

					if (HAS_BIT(gtd->flags, TINTIN_FLAG_PRESERVEMACRO))
					{
						DEL_BIT(gtd->flags, TINTIN_FLAG_PRESERVEMACRO);
					}
					else
					{
						gtd->macro_buf[0] = 0;
					}

					pop_call();
					return TRUE;
				}
				else if (!strncmp(gtd->macro_buf, node->arg4, strlen(gtd->macro_buf)))
				{
					partial = TRUE;
				}
			}
		}

		if (partial)
		{
			pop_call();
			return TRUE;
		}

		if (!HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_SGA) || HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO) || gtd->ses->input->buf[0] == gtd->tintin_char)
		{
			for (cnt = 0 ; *cursor_table[cnt].fun != NULL ; cnt++)
			{
				if (*cursor_table[cnt].code)
				{
					if (!strcmp(gtd->macro_buf, cursor_table[cnt].code))
					{
						cursor_table[cnt].fun(gtd->ses, "");
						gtd->macro_buf[0] = 0;

						pop_call();
						return TRUE;
					}
					else if (!strncmp(gtd->macro_buf, cursor_table[cnt].code, strlen(gtd->macro_buf)))
					{
						pop_call();
						return TRUE;
					}
				}
			}
		}

		if (gtd->macro_buf[0] == ASCII_ESC)
		{
			if (gtd->macro_buf[1] == '[')
			{
				if (gtd->macro_buf[2] == '<' && HAS_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING))
				{
					val[0] = val[1] = val[2] = cnt = input[0] = 0;

					for (len = 3 ; gtd->macro_buf[len] ; len++)
					{
						if (isdigit((int) gtd->macro_buf[len]))
						{
							cat_sprintf(input, "%c", gtd->macro_buf[len]);
						}
						else
						{
							switch (gtd->macro_buf[len])
							{
								case ';':
									val[cnt++] = get_number(gtd->ses, input);
									input[0] = 0;
									break;

								case 'm':
									SET_BIT(val[0], MOUSE_FLAG_RELEASE);
								case 'M':
									val[cnt++] = get_number(gtd->ses, input);
									mouse_handler(gtd->ses, val[0], val[2], val[1]); // swap x y to row col
									gtd->macro_buf[0] = 0;
									pop_call();
									return TRUE;

								default:
									print_stdout("unknownmouse input error (%s)\n", gtd->macro_buf);
									gtd->macro_buf[0] = 0;
									pop_call();
									return TRUE;
							}
						}
					}
					pop_call();
					return TRUE;
				}
				else if (gtd->macro_buf[2] >= '0' && gtd->macro_buf[2] <= '9')
				{
					cnt = input[0] = 0;
					memset(val, 0, sizeof(val));

//					tintin_printf2(gtd->ses, "debug: %d %d %d %d %d", val[0], val[1], val[2], val[3], val[4], val[5]);
		
					for (len = 2 ; gtd->macro_buf[len] ; len++)
					{
						if (cnt > 5)
						{
							pop_call();
							return FALSE;
						}

						if (isdigit((int) gtd->macro_buf[len]))
						{
							cat_sprintf(input, "%c", gtd->macro_buf[len]);
						}
						else
						{
							switch (gtd->macro_buf[len])
							{
								case '-':
									if (input[0] == 0)
									{
										strcat(input, "-");
									}
									else
									{
										tintin_printf2(NULL, "\e[1;31merror: bad csi input (%s)\n", &gtd->macro_buf[1]);
										gtd->macro_buf[0] = 0;
										continue;
									}
									break;
								case ';':
									val[cnt++] = get_number(gtd->ses, input);
									input[0] = 0;
									break;

								case 't':
									val[cnt++] = get_number(gtd->ses, input);
									csit_handler(val[0], val[1], val[2]);
									gtd->macro_buf[0] = 0;
									pop_call();
									return TRUE;

								case '&':
									val[cnt++] = get_number(gtd->ses, input);
									input[0] = 0;
									break;

								case 'w':
									rqlp_handler(val[0], val[1], val[2], val[3]);
									gtd->macro_buf[0] = 0;
									pop_call();
									return TRUE;

								default:
									pop_call();
									return FALSE;
							}
						}
					}
					pop_call();
					return TRUE;
				}
				else if (gtd->macro_buf[2] == 0)
				{
					pop_call();
					return TRUE;
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
								input[len - 3] = 0;
								osc_handler(gtd->macro_buf[2], input);
								gtd->macro_buf[0] = 0;
								pop_call();
								return TRUE;
							}
							else
							{
								input[len - 3 ] = gtd->macro_buf[len];
							}
						}
						break;

					default:
						print_stdout("\e[1;31merror: unknown osc input (%s)\n", gtd->macro_buf);
						gtd->macro_buf[0] = 0;
						pop_call();
						return TRUE;
				}
			}
			else if (gtd->macro_buf[1] == 0)
			{
				pop_call();
				return TRUE;
			}
		}
	}
	pop_call();
	return FALSE;
}

void convert_meta(char *input, char *output, int eol)
{
	char *pti, *pto;

	push_call("convert_meta(%p,%p,%d)",input,output,eol);

	pti = input;
	pto = output;

	while (*pti && pti - input < BUFFER_SIZE / 2)
	{
		switch (*pti)
		{
			case ASCII_ESC:
				*pto++ = '\\';
				*pto++ = 'e';
				pti++;
				break;

			case ASCII_DEL:
				*pto++ = '\\';
				*pto++ = 'x';
				*pto++ = '7';
				*pto++ = 'F';
				pti++;
				break;

			case ASCII_BEL:
				*pto++ = '\\';
				*pto++ = 'a';
				pti++;
				break;

			case ASCII_BS:
				*pto++ = '\\';
				*pto++ = 'b';
				pti++;
				break;

			case ASCII_FF:
				*pto++ = '\\';
				*pto++ = 'f';
				pti++;
				break;

			case ASCII_HTAB:
				*pto++ = '\\';
				*pto++ = 't';
				pti++;
				break;

			case ASCII_CR:
				if (HAS_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR))
				{
					*pto++ = '\\';
					*pto++ = 'r';
					pti++;
				}
				else if (eol)
				{
					*pto++ = '\\';
					*pto++ = 'r';
					*pto++ = *pti++;
				}
				else
				{
					*pto++ = *pti++;
				}
				break;

			case ASCII_VTAB:
				*pto++ = '\\';
				*pto++ = 'v';
				pti++;
				break;

			case ASCII_LF:
				if (HAS_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR))
				{
					*pto++ = '\\';
					*pto++ = 'n';
					pti++;
				}
				else if (eol)
				{
					*pto++ = '\\';
					*pto++ = 'n';
					*pto++ = *pti++;
				}
				else
				{
					*pto++ = *pti++;
				}

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

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR))
	{
		gtd->convert_time = 200000LL + gtd->utime;
	}

	pop_call();
	return;
}

char *str_convert_meta(char *input, int eol)
{
	static char buf[BUFFER_SIZE];

	convert_meta(input, buf, eol);

	return buf;
}


/*
	Currenly only used in split mode.
*/

void echo_command(struct session *ses, char *line)
{
	char buffer[BUFFER_SIZE], output[BUFFER_SIZE];

	DEL_BIT(ses->telopts, TELOPT_FLAG_PROMPT);

	if (ses->check_output)
	{
		strcpy(output, ses->more_output);

		process_mud_output(ses, buffer, FALSE);
	}
	else
	{
		strcpy(output, "");
	}

	if (!HAS_BIT(ses->flags, SES_FLAG_SPLIT))
	{
		add_line_buffer(ses, line, -1);

		return;
	}

	if (HAS_BIT(ses->flags, SES_FLAG_ECHOCOMMAND))
	{
		sprintf(buffer, "%s%s\e[0m", ses->cmd_color, line);
	}
	else
	{
		if (strip_vt102_strlen(ses, output) == 0)
		{
			return;
		}
		sprintf(buffer, "\e[0m");
	}

//	if (ses->wrap == gtd->screen->cols)
	{
		gtd->level->scroll++;

		tintin_printf2(ses, "%s%s", ses->scroll->input, buffer);

		gtd->level->scroll--;
	}
	add_line_buffer(ses, buffer, -1);
}

void input_printf(char *format, ...)
{
	char buf[STRING_SIZE];
	va_list args;

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		if (!HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO) && gtd->ses->input->buf[0] != gtd->tintin_char)
		{
			return;
		}
	}

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	print_stdout("%s", buf);
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
