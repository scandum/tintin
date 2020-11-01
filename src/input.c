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

int processed_keypress(char *input, int index, int catch);

void process_input(void)
{
	char *input;
	struct session *input_ses;
	int len, out;

	push_call("process_input(void)");

	input = str_alloc_stack(0);

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
					show_message(gtd->ses, LIST_COMMAND, "#DAEMON UPDATE: DETACHING FROM {%s} DUE TO READ FAILURE.", gtd->detach_file);
				}

				gtd->detach_sock = close(gtd->detach_sock);

				dirty_screen(gtd->ses);

				pop_call();
				return;
			}
		}
		else
		{
			len = 0;

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

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_CONVERTMETACHAR))
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
				DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_CONVERTMETACHAR);
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

		cursor_enter_finish(gtd->ses, "");

		pop_call();
		return;
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
		buffer_end(gtd->ses, "", "", "");
	}

	input_ses = gtd->ses;

	check_all_events(gtd->ses, SUB_SEC|EVENT_FLAG_INPUT, 0, 1, "RECEIVED INPUT", gtd->ses->input->buf);

	if (*gtd->ses->input->line_name)
	{
		check_all_events(gtd->ses, SUB_SEC|EVENT_FLAG_INPUT, 1, 1, "RECEIVED INPUT %s", gtd->ses->input->line_name, gtd->ses->input->buf);

		if (check_all_events(gtd->ses, SUB_SEC|EVENT_FLAG_CATCH, 1, 1, "CATCH RECEIVED INPUT %s", gtd->ses->input->line_name, gtd->ses->input->buf))
		{
			cursor_enter_finish(input_ses, "");

			pop_call();
			return;
		}
	}

	if (check_all_events(gtd->ses, SUB_SEC|EVENT_FLAG_CATCH, 0, 1, "CATCH RECEIVED INPUT", gtd->ses->input->buf) == 1)
	{
		cursor_enter_finish(input_ses, "");

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

	if (HAS_BIT(input_ses->telopts, TELOPT_FLAG_ECHO))
	{
		add_line_history(input_ses, input_ses->input->buf);
	}

	cursor_enter_finish(input_ses, "");

	fflush(NULL);

	pop_call();
	return;
}

void read_line(char *input, int len)
{
	char buf[BUFFER_SIZE];
	int size, width, index;

	if (HAS_BIT(gtd->ses->config_flags, CONFIG_FLAG_CONVERTMETA) || gtd->level->convert)
	{
		convert_meta(input, &gtd->macro_buf[strlen(gtd->macro_buf)], FALSE);
	}
	else if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_CONVERTMETACHAR))
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

	if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_EUC) && is_euc_head(gtd->ses, gtd->macro_buf))
	{
		if (gtd->macro_buf[1] == 0)
		{
			return;
		}
	}

	if (HAS_BIT(gtd->ses->event_flags, EVENT_FLAG_INPUT))
	{
		check_all_events(gtd->ses, EVENT_FLAG_INPUT, 0, 2, "RECEIVED KEYPRESS", input, ntos(index));

		if (check_all_events(gtd->ses, EVENT_FLAG_CATCH, 0, 2, "CATCH RECEIVED KEYPRESS", input, ntos(index)) == 1)
		{
			check_all_events(gtd->ses, EVENT_FLAG_INPUT, 0, 4, "PROCESSED KEYPRESS", input, ntos(index), ntos(gtd->ses->input->edit->update + 1), ntos(gtd->ses->input->str_pos + 1));

			return;
		}
	}

	if (check_key(input, len))
	{
		if (HAS_BIT(gtd->ses->event_flags, EVENT_FLAG_INPUT))
		{
			check_all_events(gtd->ses, EVENT_FLAG_INPUT, 0, 4, "PROCESSED KEYPRESS", input, ntos(index), ntos(gtd->ses->input->edit->update + 1), ntos(gtd->ses->input->str_pos + 1));
		}
		return;
	}

	if (gtd->macro_buf[0] < 32)
	{
		switch (gtd->macro_buf[0])
		{
			case ASCII_CR:
			case ASCII_LF:
				break;

			default:
				strcpy(buf, gtd->macro_buf);

				convert_meta(buf, gtd->macro_buf, FALSE);
				break;
		}
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
				if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(gtd->macro_buf) && gtd->macro_buf[1])
				{
					size = get_utf8_width(gtd->macro_buf, &width);
				}
				else if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_EUC) && is_euc_head(gtd->ses, gtd->macro_buf) && gtd->macro_buf[1])
				{
					size = get_euc_width(gtd->ses, gtd->macro_buf, &width);
				}
				else
				{
					size = get_ascii_width(gtd->macro_buf, &width);
				}

				sprintf(buf, "%.*s", size, gtd->macro_buf);

				inputline_insert(buf, -1);
/*
				if (width && HAS_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT) && gtd->ses->input->raw_len != gtd->ses->input->raw_pos)
				{
					cursor_delete(gtd->ses, "");
				}

				
				str_ins_printf(&gtd->ses->input->buf, gtd->ses->input->raw_pos, "%.*s", size, gtd->macro_buf);

				gtd->ses->input->str_pos += width;
				gtd->ses->input->str_len += width;

				gtd->ses->input->raw_pos += size;
				gtd->ses->input->raw_len += size;
*/

				if (width && gtd->ses->input->raw_len != gtd->ses->input->raw_pos)
				{
					cursor_redraw_line(gtd->ses, "");
				}
				else
				{
					input_printf("%.*s", size, gtd->macro_buf);
				}
				memmove(gtd->macro_buf, &gtd->macro_buf[size], 1 + strlen(&gtd->macro_buf[size]));

				cursor_check_line_modified(gtd->ses, "");

				DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYBROWSE);

				kill_list(gtd->ses->list[LIST_COMMAND]);

				if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
				{
					cursor_history_find(gtd->ses, "");
				}
				break;
		}
	}
	if (HAS_BIT(gtd->ses->event_flags, EVENT_FLAG_INPUT))
	{
		check_all_events(gtd->ses, EVENT_FLAG_INPUT, 0, 4, "PROCESSED KEYPRESS", input, ntos(index), ntos(gtd->ses->input->edit->update + 1), ntos(gtd->ses->input->str_pos + 1));
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

	if (HAS_BIT(gtd->ses->config_flags, CONFIG_FLAG_CONVERTMETA))
	{
		convert_meta(input, &gtd->macro_buf[strlen(gtd->macro_buf)], FALSE);
	}
	else if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_CONVERTMETACHAR))
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
				str_cpy(&gtd->ses->input->buf, "");

				gtd->ses->input->raw_len = 0;
				gtd->ses->input->str_len = 0;

				gtd->ses->input->raw_pos = 0;
				gtd->ses->input->str_pos = 0;

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
					print_stdout(0, 0, "%c", gtd->macro_buf[cnt]);

					str_cpy_printf(&gtd->ses->input->buf, "%c", gtd->tintin_char);

					gtd->ses->input->raw_len = 1;
					gtd->ses->input->str_len = 1;

					gtd->ses->input->raw_pos = 1;
					gtd->ses->input->str_pos = 1;
				}
				else
				{
					socket_printf(gtd->ses, 1, "%c", gtd->macro_buf[cnt]);

					str_cpy(&gtd->ses->input->buf, "\r");
				}
				break;
		}
		gtd->macro_buf[0] = 0;
	}
}

int check_key(char *input, int len)
{
	char buf[BUFFER_SIZE];
	struct listroot *root;
	struct listnode *node;
	int cnt, val[6], partial;

	push_call("check_key(%p,%d)",input,len);

//	tintin_printf2(gtd->ses, "check_key(%d,%s)",*gtd->macro_buf, gtd->macro_buf);

	if (!HAS_BIT(gtd->ses->config_flags, CONFIG_FLAG_CONVERTMETA))
	{
		root = gtd->ses->list[LIST_MACRO];

		if (!HAS_BIT(root->flags, LIST_FLAG_IGNORE))
		{
			partial = 0;

			for (root->update = 0 ; root->update < root->used ; root->update++)
			{
				node = root->list[root->update];

				if (*node->arg1 == '^' && gtd->ses->input->raw_len)
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
			if (partial)
			{
				pop_call();
				return TRUE;
			}
		}


		if (!HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_SGA) || HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO) || gtd->ses->input->buf[0] == gtd->tintin_char)
		{
			for (cnt = 0 ; *cursor_table[cnt].fun != NULL ; cnt++)
			{
				if (*cursor_table[cnt].code)
				{
					if (!strcmp(gtd->macro_buf, cursor_table[cnt].code))
					{
						cursor_table[cnt].fun(gtd->ses, cursor_table[cnt].arg);

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
				if (gtd->macro_buf[2] == 'I')
				{
					gtd->macro_buf[0] = 0;

					gtd->screen->focus = 1;
					
					check_all_events(gtd->ses, EVENT_FLAG_SCREEN, 0, 1, "SCREEN FOCUS", ntos(gtd->screen->focus));
					
					msdp_update_all("SCREEN_FOCUS", "%d", gtd->screen->focus);
					
					pop_call();
					return TRUE;
				}

				if (gtd->macro_buf[2] == 'O')
				{
					gtd->macro_buf[0] = 0;

					gtd->screen->focus = 0;
					
					check_all_events(gtd->ses, EVENT_FLAG_SCREEN, 0, 1, "SCREEN FOCUS", ntos(gtd->screen->focus));
					
					msdp_update_all("SCREEN_FOCUS", "%d", gtd->screen->focus);
					 
					pop_call();
					return TRUE;
				}

				if (gtd->macro_buf[2] == '<' && HAS_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING))
				{
					val[0] = val[1] = val[2] = cnt = input[0] = 0;

					for (len = 3 ; gtd->macro_buf[len] ; len++)
					{
						if (is_digit(gtd->macro_buf[len]))
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
									print_stdout(0, 0, "unknownmouse input error (%s)\n", str_convert_meta(gtd->macro_buf, TRUE));
									gtd->macro_buf[0] = 0;
									pop_call();
									return TRUE;
							}
						}
					}
					pop_call();
					return TRUE;
				}

				if (is_digit(gtd->macro_buf[2]))
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

						if (is_digit(gtd->macro_buf[len]))
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


								case '&':
									val[cnt++] = get_number(gtd->ses, input);
									input[0] = 0;
									break;

								case '#':
									val[cnt++] = get_number(gtd->ses, input);
									input[0] = 0;
									break;

								case 'w':
									rqlp_handler(val[0], val[1], val[2], val[3]);
									gtd->macro_buf[0] = 0;
									pop_call();
									return TRUE;

								case 't':
									val[cnt++] = get_number(gtd->ses, input);
									csit_handler(val[0], val[1], val[2]);
									gtd->macro_buf[0] = 0;
									pop_call();
									return TRUE;

								case 'd':
									if (gtd->macro_buf[len - 1] == '#')
									{
										check_all_events(gtd->ses, EVENT_FLAG_SCREEN, 0, 3, "SCROLLBAR POSITION", ntos(val[0]), ntos(val[1]), ntos(val[2]));

										if (!check_all_events(gtd->ses, EVENT_FLAG_CATCH, 0, 3, "CATCH SCROLLBAR POSITION", ntos(val[0]), ntos(val[1]), ntos(val[2])))
										{
											command(gtd->ses, do_buffer, "JUMP %d", URANGE(0, val[0] + get_scroll_rows(gtd->ses), gtd->ses->scroll->used));
										}
										gtd->macro_buf[0] = 0;
										pop_call();
										return TRUE;
									}
									pop_call();
									return FALSE;

								case 'e':
									if (gtd->macro_buf[len - 1] == '#')
									{
										check_all_events(gtd->ses, EVENT_FLAG_SCREEN, 0, 3, "SCROLLBAR MOVE", ntos(val[0]), ntos(val[1]), ntos(val[2]));

										if (!check_all_events(gtd->ses, EVENT_FLAG_CATCH, 0, 3, "CATCH SCROLLBAR MOVE", ntos(val[0]), ntos(val[1]), ntos(val[2])))
										{
											switch (val[0])
											{
												case 5:
													cursor_page(gtd->ses, "up");
													break;
												case 6:
													cursor_page(gtd->ses, "down");
													break;
												case 65:
													cursor_page(gtd->ses, "up 1");
													break;
												case 66:
													cursor_page(gtd->ses, "down 1");
													break;
											}
										}
										gtd->macro_buf[0] = 0;
										pop_call();
										return TRUE;
									}
									pop_call();
									return FALSE;
										
									
								default:
									pop_call();
									return FALSE;
							}
						}
					}
					pop_call();
					return TRUE;
				}

				if (gtd->macro_buf[2] == 0)
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
						for (len = 3 ; gtd->macro_buf[len] ; len++)
						{
							if (gtd->macro_buf[len] == '\a' || (gtd->macro_buf[len] == '\e' && gtd->macro_buf[len + 1] == '\\'))
							{
								break;
							}
						}
						if (gtd->macro_buf[len] == 0 && len < 30)
						{
							pop_call();
							return TRUE;
						}
						print_stdout(0, 0, "\e[1;31merror: unknown osc input (%s)\n", str_convert_meta(gtd->macro_buf, TRUE));
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
/*
			case ASCII_FF:
				*pto++ = '\\';
				*pto++ = 'f';
				pti++;
				break;
*/
			case ASCII_HTAB:
				*pto++ = '\\';
				*pto++ = 't';
				pti++;
				break;

			case ASCII_CR:
				if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_CONVERTMETACHAR))
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
/*
			case ASCII_VTAB:
				*pto++ = '\\';
				*pto++ = 'v';
				pti++;
				break;
*/
			case ASCII_LF:
				if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_CONVERTMETACHAR) || gtd->level->convert)
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

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_CONVERTMETACHAR))
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

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_ECHOCOMMAND))
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

void init_input(struct session *ses, int top_row, int top_col, int bot_row, int bot_col)
{
	push_call("init_input(%p,%d,%d,%d,%d)",ses,top_row,top_col,bot_row,bot_col);

	if (ses->input->buf == NULL)
	{
		ses->input->str_off = 1;

		ses->input->edit = create_editor();
		ses->input->line = create_editor(); // unused for now

		ses->input->edit_name = str_dup("");
		ses->input->line_name = str_dup("");

		ses->input->buf  = str_dup("");
		ses->input->tmp  = str_dup("");
		ses->input->cut  = str_dup("");
	}

	if (top_row && top_col && bot_row && bot_col)
	{
		ses->input->sav_top_row = top_row;
		ses->input->sav_top_col = top_col;
		ses->input->sav_bot_row = bot_row;
		ses->input->sav_bot_col = bot_col;
	}
	else
	{
		if (ses == gts)
		{
			ses->input->sav_top_row = -1;
			ses->input->sav_top_col =  1;
			ses->input->sav_bot_row = -1;
			ses->input->sav_bot_col = -1;
		}
		else
		{
			ses->input->sav_top_row = gts->input->sav_top_row;
			ses->input->sav_top_col = gts->input->sav_top_col;
			ses->input->sav_bot_row = gts->input->sav_bot_row;
			ses->input->sav_bot_col = gts->input->sav_bot_col;
		}
	}

	top_row = get_row_index(ses, ses->input->sav_top_row);
	top_col = get_col_index(ses, ses->input->sav_top_col);
	bot_row = get_row_index(ses, ses->input->sav_bot_row);
	bot_col = get_col_index(ses, ses->input->sav_bot_col);

	ses->input->top_row = top_row;
	ses->input->top_col = top_col;
	ses->input->bot_row = bot_row;
	ses->input->bot_col = bot_col;

	ses->input->cur_row = top_row;

	pop_call();
	return;
}

void free_input(struct session *ses)
{
	delete_editor(ses->input->line);
	delete_editor(ses->input->edit);

	str_free(ses->input->edit_name);
	str_free(ses->input->line_name);
	str_free(ses->input->buf);
	str_free(ses->input->tmp);
}

void input_printf(char *format, ...)
{
	char *buf;
	va_list args;

	if (!HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		if (!HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO) && gtd->ses->input->buf[0] != gtd->tintin_char)
		{
			return;
		}
	}

	va_start(args, format);
	vasprintf(&buf, format, args);
	va_end(args);

	print_stdout(0, 0, "%s", buf);

	free(buf);

	return;
}

void modified_input(void)
{
	kill_list(gtd->ses->list[LIST_COMMAND]);

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		cursor_history_find(gtd->ses, "");
	}

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYBROWSE))
	{
		DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYBROWSE);
	}

}
