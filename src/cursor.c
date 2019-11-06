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
*              (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                   *
*                                                                             *
*                     coded by Igor van den Hoven 2006                        *
******************************************************************************/


#include "tintin.h"

DO_COMMAND(do_cursor)
{
	char all[BUFFER_SIZE], arg1[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE];
	int cnt;

	get_arg_in_braces(ses, arg, all, GET_ALL);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = sub_arg_in_braces(ses, arg, right, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_header(ses, " CURSOR OPTIONS ");

		for (cnt = 0 ; *cursor_table[cnt].fun ; cnt++)
		{
			if (*cursor_table[cnt].name)
			{
				convert_meta(cursor_table[cnt].code, temp, FALSE);

				tintin_printf2(ses, "  [%-18s] [%-6s] %s", cursor_table[cnt].name, temp, cursor_table[cnt].desc);
			}
		}
		tintin_header(ses, "");
	}
	else
	{
		for (cnt = 0 ; ; cnt++)
		{
			if (HAS_BIT(cursor_table[cnt].flags, CURSOR_FLAG_GET_ALL))
			{
				if (is_abbrev(all, cursor_table[cnt].name))
				{
					cursor_table[cnt].fun(ses, right);

					return ses;
				}
			}
			else if (HAS_BIT(cursor_table[cnt].flags, CURSOR_FLAG_GET_ONE))
			{
				if (is_abbrev(arg1, cursor_table[cnt].name))
				{
					cursor_table[cnt].fun(ses, right);

					return ses;
				}
			}
			else
			{
				break;
			}
		}
		show_error(ses, LIST_COMMAND, "#ERROR: #CURSOR {%s} IS NOT A VALID OPTION.", capitalize(all));
	}
	return ses;
}

int inputline_str_str_len(int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt, width;

	raw_cnt = str_cnt = ret_cnt = 0;

	while (raw_cnt < gtd->input_len)
	{
		if (str_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&gtd->input_buf[raw_cnt]))
		{
			raw_cnt += get_utf8_width(&gtd->input_buf[raw_cnt], &width);

			if (str_cnt >= start)
			{
				ret_cnt += width;
			}
			str_cnt += width;
		}
		else
		{
			if (str_cnt >= start)
			{
				ret_cnt++;
			}
			raw_cnt++;
			str_cnt++;
		}
	}
	return ret_cnt;
}

// raw range

int inputline_raw_str_len(int start, int end)
{
	int raw_cnt, ret_cnt, width;

	raw_cnt = start;
	ret_cnt = 0;

	while (raw_cnt < gtd->input_len)
	{
		if (raw_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&gtd->input_buf[raw_cnt]))
		{
			raw_cnt += get_utf8_width(&gtd->input_buf[raw_cnt], &width);
			ret_cnt += width;
		}
		else
		{
			raw_cnt++;
			ret_cnt++;
		}
	}
	return ret_cnt;
}

// display range

int inputline_str_raw_len(int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt, width, tmp_cnt;

	raw_cnt = str_cnt = ret_cnt = 0;

	while (raw_cnt < gtd->input_len)
	{
		if (str_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&gtd->input_buf[raw_cnt]))
		{    
			tmp_cnt = get_utf8_width(&gtd->input_buf[raw_cnt], &width);

			if (str_cnt >= start)
			{
				ret_cnt += tmp_cnt;
			}
			raw_cnt += tmp_cnt;
			str_cnt += width;
		}
		else
		{
			if (str_cnt >= start)
			{
				ret_cnt++;
			}
			raw_cnt++;
			str_cnt++;
		}
	}
	return ret_cnt;
}

int inputline_raw_raw_len(int start, int end)
{
	if (start > end)
	{
		return 0;
	}
	return end - start;
}

// Get string length of the input area

int inputline_max_str_len(void)
{
	return gtd->screen->cols + 1 - gtd->input_off - (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH) ? 11 : 0);
}

int inputline_cur_str_len(void)
{
	return inputline_str_str_len(gtd->input_hid, gtd->input_hid + inputline_max_str_len());
}

// Get the position of the cursor

int inputline_cur_pos(void)
{
	return gtd->input_off + gtd->input_pos - gtd->input_hid;
}

// Check for invalid characters.

int inputline_str_chk(int offset, int totlen)
{
	int size;

	while (offset < totlen)
	{
		if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_BIG5))
		{
			if (HAS_BIT(gtd->input_buf[offset], 128) && (unsigned char) gtd->input_buf[offset] < 255)
			{
				if (offset + 1 >= totlen)
				{
					return FALSE;
				}

				if (is_big5(&gtd->input_buf[offset]))
				{
					offset += 2;
				}
				else
				{
					offset += 1;
				}
			}
			else
			{
				offset += 1;
			}
		}
		else if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8))
		{
			if (is_utf8_head(&gtd->input_buf[offset]))
			{
				size = get_utf8_size(&gtd->input_buf[offset]);

				if (size == 1 || offset + size > totlen)
				{
					return FALSE;
				}
				offset += size;
			}
			else
			{
				offset += 1;
			}
		}
		else
		{
			return TRUE;
		}
	}
	return TRUE;
}

DO_CURSOR(cursor_backspace)
{
	if (gtd->input_cur == 0)
	{
		return;
	}

	cursor_left(ses, "");
	cursor_delete(ses, "");

	modified_input();
}

DO_CURSOR(cursor_brace_open)
{
	ins_sprintf(&gtd->input_buf[gtd->input_cur], "{");

	gtd->input_len++;
	gtd->input_cur++;

	gtd->input_pos += inputline_raw_str_len(gtd->input_cur - 1, gtd->input_cur);

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_brace_close)
{
	ins_sprintf(&gtd->input_buf[gtd->input_cur], "}");

	gtd->input_len++;
	gtd->input_cur++;

	gtd->input_pos += inputline_raw_str_len(gtd->input_cur - 1, gtd->input_cur);

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_buffer_down)
{
	do_buffer(ses, "DOWN");
}

DO_CURSOR(cursor_buffer_end)
{
	do_buffer(ses, "END");
}

DO_CURSOR(cursor_buffer_home)
{
	do_buffer(ses, "HOME");
}

DO_CURSOR(cursor_buffer_lock)
{
	do_buffer(ses, "LOCK");
}

DO_CURSOR(cursor_buffer_up)
{
	do_buffer(ses, "UP");
}


DO_CURSOR(cursor_check_line)
{
	if (gtd->input_pos - gtd->input_hid > inputline_max_str_len() - 3)
	{
		return cursor_redraw_line(ses, "");
	}

	if (gtd->input_hid && gtd->input_pos - gtd->input_hid < 2)
	{
		return cursor_redraw_line(ses, "");
	}
}

DO_CURSOR(cursor_check_line_modified)
{
	if (gtd->input_hid + inputline_max_str_len() < inputline_raw_str_len(0, gtd->input_len))
	{
		return cursor_redraw_line(ses, "");
	}

	return cursor_check_line(ses, "");
}

DO_CURSOR(cursor_clear_left)
{
	if (gtd->input_cur == 0)
	{
		return;
	}

	sprintf(gtd->paste_buf, "%.*s", gtd->input_cur, gtd->input_buf);

	input_printf("\e[%dG\e[%dP", gtd->input_off, gtd->input_pos - gtd->input_hid);

	memmove(&gtd->input_buf[0], &gtd->input_buf[gtd->input_cur], gtd->input_len - gtd->input_cur);

	gtd->input_len -= gtd->input_cur;

	gtd->input_buf[gtd->input_len] = 0;

	gtd->input_cur  = 0;
	gtd->input_pos  = 0;

	cursor_check_line_modified(ses, "");

	modified_input();
}

DO_CURSOR(cursor_clear_line)
{
	if (gtd->input_len == 0)
	{
		return;
	}

	sprintf(gtd->paste_buf, "%s", gtd->input_buf);

	input_printf("\e[%dG\e[%dP", gtd->input_off, inputline_cur_str_len());

	gtd->input_len = 0;
	gtd->input_cur = 0;
	gtd->input_hid = 0;
	gtd->input_pos = 0;
	gtd->input_buf[0] = 0;

	modified_input();
}

DO_CURSOR(cursor_clear_right)
{
	if (gtd->input_cur == gtd->input_len)
	{
		return;
	}

	strcpy(gtd->paste_buf, &gtd->input_buf[gtd->input_cur]);

	input_printf("\e[%dP", inputline_max_str_len() - inputline_str_str_len(gtd->input_hid, gtd->input_pos));

	gtd->input_buf[gtd->input_cur] = 0;

	gtd->input_len = gtd->input_cur;

	modified_input();
}

DO_CURSOR(cursor_convert_meta)
{
	SET_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR);
}

DO_CURSOR(cursor_delete_or_exit)
{
	if (gtd->input_len == 0)
	{
		cursor_exit(ses, "");
	}
	else
	{
		cursor_delete(ses, "");
	}
}

DO_CURSOR(cursor_delete)
{
	int size, width;

	if (gtd->input_len == 0)
	{
		return;
	}

	if (gtd->input_len == gtd->input_cur)
	{
		return;
	}

	if (HAS_BIT(ses->charset, CHARSET_FLAG_BIG5) && is_big5(&gtd->input_buf[gtd->input_cur]))
	{
		gtd->input_len--;

		memmove(&gtd->input_buf[gtd->input_cur+1], &gtd->input_buf[gtd->input_cur+2], gtd->input_len - gtd->input_cur + 1);

		input_printf("\e[2P");
	}
	else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
	{
		size = get_utf8_width(&gtd->input_buf[gtd->input_cur], &width);

		gtd->input_len -= size;

		memmove(&gtd->input_buf[gtd->input_cur], &gtd->input_buf[gtd->input_cur + size], gtd->input_len - gtd->input_cur + 1);

		if (width)
		{
			input_printf("\e[%dP", width);
		}

		while (gtd->input_len > gtd->input_cur)
		{
			size = get_utf8_width(&gtd->input_buf[gtd->input_cur], &width);

			if (width)
			{
				break;
			}
			gtd->input_len -= size;

			memmove(&gtd->input_buf[gtd->input_cur], &gtd->input_buf[gtd->input_cur + size], gtd->input_len - gtd->input_cur + 1);
		}
	}
	else
	{
		gtd->input_len--;

		memmove(&gtd->input_buf[gtd->input_cur], &gtd->input_buf[gtd->input_cur+1], gtd->input_len - gtd->input_cur + 1);

		input_printf("\e[1P");
	}

	if (gtd->input_hid + inputline_max_str_len() <= inputline_raw_str_len(0, gtd->input_len))
	{
		cursor_redraw_line(ses, "");
	}

	modified_input();
}

DO_CURSOR(cursor_delete_word_left)
{
	int index_cur, width;

	if (gtd->input_cur == 0)
	{
		return;
	}

	index_cur = gtd->input_cur;

	while (gtd->input_cur > 0 && gtd->input_buf[gtd->input_cur - 1] == ' ')
	{
		gtd->input_pos--;
		gtd->input_cur--;
	}

	while (gtd->input_cur > 0 && gtd->input_buf[gtd->input_cur - 1] != ' ')
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			if (is_utf8_tail(&gtd->input_buf[gtd->input_cur]))
			{
				gtd->input_cur--;
			}
			else if (is_utf8_head(&gtd->input_buf[gtd->input_cur]))
			{
				get_utf8_width(&gtd->input_buf[gtd->input_cur], &width);

				gtd->input_cur -= 1;
				gtd->input_pos -= width;
			}
			else
			{
				gtd->input_cur--;
				gtd->input_pos--;
			}
		}
		else
		{
			gtd->input_pos--;
			gtd->input_cur--;
		}
	}

	sprintf(gtd->paste_buf, "%.*s", index_cur - gtd->input_cur, &gtd->input_buf[gtd->input_cur]);

	memmove(&gtd->input_buf[gtd->input_cur], &gtd->input_buf[index_cur], gtd->input_len - index_cur + 1);

//	input_printf("\e[%dD\e[%dP", index_cur - gtd->input_cur, index_cur - gtd->input_cur);

	gtd->input_len -= index_cur - gtd->input_cur;

	cursor_redraw_line(ses, "");

	modified_input();
}


DO_CURSOR(cursor_delete_word_right)
{
	int index_cur;

	if (gtd->input_cur == gtd->input_len)
	{
		return;
	}

	index_cur = gtd->input_cur;

	while (gtd->input_cur != gtd->input_len && gtd->input_buf[gtd->input_cur] == ' ')
	{
		gtd->input_cur++;
	}

	while (gtd->input_cur != gtd->input_len && gtd->input_buf[gtd->input_cur] != ' ')
	{
		gtd->input_cur++;
	}

	sprintf(gtd->paste_buf, "%.*s", gtd->input_cur - index_cur, &gtd->input_buf[gtd->input_cur]);

	memmove(&gtd->input_buf[index_cur], &gtd->input_buf[gtd->input_cur], gtd->input_len - gtd->input_cur + 1);

//	input_printf("\e[%dP", gtd->input_cur - index_cur);

	gtd->input_len -= gtd->input_cur - index_cur;

	gtd->input_cur = index_cur;

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_echo)
{
	if (*arg == 0)
	{
		TOG_BIT(ses->telopts, TELOPT_FLAG_ECHO);
	}
	else if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->telopts, TELOPT_FLAG_ECHO);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->telopts, TELOPT_FLAG_ECHO);
	}
	else
	{
		show_error(gtd->ses, LIST_COMMAND, "#SYNTAX: #CURSOR {ECHO} {ON|OFF}.");
	}
}

DO_CURSOR(cursor_end)
{
	gtd->input_cur = gtd->input_len;

	gtd->input_pos = inputline_raw_str_len(0, gtd->input_len);

	cursor_redraw_line(ses, "");
}

DO_CURSOR(cursor_enter)
{
	input_printf("\n");

	gtd->input_buf[gtd->input_len] = 0;

	gtd->input_len    = 0;
	gtd->input_cur    = 0;
	gtd->input_pos    = 0;
	gtd->input_off    = 1;
	gtd->input_hid    = 0;
	gtd->macro_buf[0] = 0;
	gtd->input_tmp[0] = 0;

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		struct listroot *root = ses->list[LIST_HISTORY];

		if (root->update >= 0 && root->update < root->used)
		{
			strcpy(gtd->input_buf, root->list[root->update]->arg1);
		}

		DEL_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH);

		gtd->input_off = 1;
	}

	SET_BIT(gtd->flags, TINTIN_FLAG_PROCESSINPUT);

	modified_input();
}

DO_CURSOR(cursor_exit)
{
	if (ses == gts)
	{
		do_end(ses, "");
	}
	else
	{
		cleanup_session(ses);
	}
}

DO_CURSOR(cursor_get)
{
	if (*arg == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR GET {variable}");
	}
	else
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg, "%s", gtd->input_buf);
	}
}

DO_CURSOR(cursor_history_next)
{
	struct listroot *root = ses->list[LIST_HISTORY];

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		if (root->update == root->used)
		{
			return;
		}

		for (root->update++ ; root->update < root->used ; root->update++)
		{
			if (*gtd->input_buf && find(ses, root->list[root->update]->arg1, gtd->input_buf, SUB_NONE, REGEX_FLAG_NONE))
			{
				break;
			}
		}

		if (root->update < root->used)
		{
			input_printf("\e[%dG  \e[0K%.*s\e[%dG", gtd->input_off + inputline_cur_str_len() + 2, gtd->input_off + inputline_max_str_len() - inputline_cur_str_len() - 4, root->list[root->update]->arg1, gtd->input_off + gtd->input_pos - gtd->input_hid);
		}
		return;
	}

	if (root->list[0] == NULL)
	{
		return;
	}

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYBROWSE))
	{
		return;
	}
	else if (root->update < root->used)
	{
		for (root->update++ ; root->update < root->used ; root->update++)
		{
			if (!strncmp(gtd->input_tmp, root->list[root->update]->arg1, strlen(gtd->input_tmp)))
			{
				break;
			}
		}
	}

	cursor_clear_line(ses, "");

	if (root->update == root->used)
	{
		strcpy(gtd->input_buf, gtd->input_tmp);
	}
	else
	{
		strcpy(gtd->input_buf, root->list[root->update]->arg1);
	}

	gtd->input_len = strlen(gtd->input_buf);

	cursor_end(ses, "");

	SET_BIT(gtd->flags, TINTIN_FLAG_HISTORYBROWSE);
}

DO_CURSOR(cursor_history_prev)
{
	struct listroot *root = ses->list[LIST_HISTORY];

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		if (root->update <= 0)
		{
			return;
		}

		for (root->update-- ; root->update >= 0 ; root->update--)
		{
			if (*gtd->input_buf && find(ses, root->list[root->update]->arg1, gtd->input_buf, SUB_NONE, REGEX_FLAG_NONE))
			{
				break;
			}
		}

		if (root->update >= 0)
		{
			input_printf("\e[%dG  \e[0K%.*s\e[%dG", gtd->input_off + inputline_cur_str_len() + 2, gtd->input_off + inputline_max_str_len() - inputline_cur_str_len() - 4, root->list[root->update]->arg1, gtd->input_off + gtd->input_pos - gtd->input_hid);
		}
		return;
	}

	if (root->list[0] == NULL)
	{
		return;
	}

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYBROWSE))
	{
		strcpy(gtd->input_tmp, gtd->input_buf);

		for (root->update = root->used - 1 ; root->update >= 0 ; root->update--)
		{
			if (!strncmp(gtd->input_tmp, root->list[root->update]->arg1, strlen(gtd->input_tmp)))
			{
				break;
			}
		}
	}
	else if (root->update >= 0)
	{
		for (root->update-- ; root->update >= 0 ; root->update--)
		{
			if (!strncmp(gtd->input_tmp, root->list[root->update]->arg1, strlen(gtd->input_tmp)))
			{
				break;
			}
		}
	}

	cursor_clear_line(ses, "");

	if (root->update == -1)
	{
		strcpy(gtd->input_buf, gtd->input_tmp);
	}
	else
	{
		strcpy(gtd->input_buf, root->list[root->update]->arg1);
	}

	gtd->input_len = strlen(gtd->input_buf);

	cursor_end(ses, "");

	SET_BIT(gtd->flags, TINTIN_FLAG_HISTORYBROWSE);
}

DO_CURSOR(cursor_history_search)
{
	struct listroot *root = ses->list[LIST_HISTORY];

	if (root->list[0] == NULL)
	{
		return;
	}

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		strcpy(gtd->input_tmp, gtd->input_buf);

		cursor_clear_line(ses, "");

		SET_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH);

		gtd->input_off = 11;

		root->update = root->used - 1;

		input_printf("(search) [ ] \e[3D");
	}
	else
	{
		if (root->update >= 0 && root->update < root->used)
		{
			strcpy(gtd->input_buf, root->list[root->update]->arg1);
		}
		input_printf("\e[1G\e[0K");

		gtd->input_len = strlen(gtd->input_buf);
		gtd->input_cur = gtd->input_len;
		gtd->input_pos = 0;

		root->update = -1;

		DEL_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH);

		gtd->input_off = 1;

		cursor_redraw_line(ses, "");
	}
}

DO_CURSOR(cursor_history_find)
{
	struct listroot *root = ses->list[LIST_HISTORY];

	push_call("cursor_history_find(%s)", gtd->input_buf);

	if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_BIG5))
	{
		if (inputline_str_chk(0, gtd->input_len) == FALSE)
		{
			pop_call();
			return;
		}
	}

	gtd->level->quiet++;

	for (root->update = root->used - 1 ; root->update >= 0 ; root->update--)
	{
		if (*gtd->input_buf && find(ses, root->list[root->update]->arg1, gtd->input_buf, SUB_NONE, REGEX_FLAG_NONE))
		{
			break;
		}
	}

	gtd->level->quiet--;

	if (root->update >= 0)
	{
		input_printf("\e[%dG ]  %.*s\e[%dG", gtd->input_off + inputline_cur_str_len(), gtd->input_off + inputline_max_str_len() - inputline_cur_str_len() - 4, root->list[root->update]->arg1, gtd->input_off + gtd->input_pos - gtd->input_hid);
	}
	else
	{
		input_printf("\e[%dG ] \e[0K\e[%dG", gtd->input_off + inputline_cur_str_len(), gtd->input_off + gtd->input_pos - gtd->input_hid);
	}
	pop_call();
	return;
}

DO_CURSOR(cursor_home)
{
	if (gtd->input_cur == 0)
	{
		return;
	}

	input_printf("\e[%dD", gtd->input_pos - gtd->input_hid);

	gtd->input_cur = 0;
	gtd->input_pos = 0;

	if (gtd->input_hid)
	{
		gtd->input_hid = 0;

		cursor_redraw_line(ses, "");
	}
}

DO_CURSOR(cursor_insert)
{
	if (*arg == 0)
	{
		TOG_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
	}
	else if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
	}
	else
	{
		show_error(gtd->ses, LIST_COMMAND, "#SYNTAX: #CURSOR {INSERT} {ON|OFF}.");
	}
}


DO_CURSOR(cursor_left)
{
	int width;

	if (gtd->input_cur > 0)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_BIG5))
		{
			gtd->input_cur--;
			gtd->input_pos--;
			input_printf("\e[1D");

			if (inputline_str_chk(0, gtd->input_cur) == FALSE)
			{
				gtd->input_cur--;
				gtd->input_pos--;
				input_printf("\e[1D");
			}
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			gtd->input_cur--;

			if (gtd->input_cur > 0 && is_utf8_tail(&gtd->input_buf[gtd->input_cur]))
			{
				do
				{
					gtd->input_cur--;
				}
				while (gtd->input_cur > 0 && is_utf8_tail(&gtd->input_buf[gtd->input_cur]));

				get_utf8_width(&gtd->input_buf[gtd->input_cur], &width);

				if (width == 0)
				{
					return cursor_left(ses, "");
				}
				input_printf("\e[%dD", width);
				gtd->input_pos -= width;
			}
			else
			{
				gtd->input_pos--;
				input_printf("\e[1D");
			}

		}
		else
		{
			gtd->input_cur--;
			gtd->input_pos--;
			input_printf("\e[1D");
		}
		cursor_check_line(ses, "");
	}
}

DO_CURSOR(cursor_left_word)
{
	int width;
//	int index_pos;

	if (gtd->input_cur == 0)
	{
		return;
	}

//	index_pos = gtd->input_pos;

	while (gtd->input_cur > 0 && gtd->input_buf[gtd->input_cur - 1] == ' ')
	{
		gtd->input_pos--;
		gtd->input_cur--;
	}

	while (gtd->input_cur > 0 && gtd->input_buf[gtd->input_cur - 1] != ' ')
	{
		gtd->input_cur--;

		if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			if (!is_utf8_tail(&gtd->input_buf[gtd->input_cur]))
			{
				get_utf8_width(&gtd->input_buf[gtd->input_cur], &width);

				gtd->input_pos -= width;
			}
		}
		else
		{
			gtd->input_pos--;
		}
	}

//	input_printf("\e[%dD", index_pos - gtd->input_pos);

	cursor_redraw_line(ses, "");
}


DO_CURSOR(cursor_paste_buffer)
{
	if (*gtd->paste_buf == 0)
	{
		return;
	}

	ins_sprintf(&gtd->input_buf[gtd->input_cur], "%s", gtd->paste_buf);

	gtd->input_len += strlen(gtd->paste_buf);
	gtd->input_cur += strlen(gtd->paste_buf);

	gtd->input_pos += inputline_raw_str_len(gtd->input_cur - strlen(gtd->paste_buf), gtd->input_cur);

	cursor_redraw_line(ses, "");

	modified_input();
}


DO_CURSOR(cursor_redraw_input)
{
	if (IS_SPLIT(ses))
	{
		cursor_redraw_line(ses, "");
	}
	else
	{
/*		if (*gtd->ses->scroll->input)
		{
			input_printf("\e[G%s", gtd->ses->scroll->input);
		}*/
		cursor_redraw_line(ses, "");
/*
		gtd->input_cur = gtd->input_len;

		gtd->input_pos = gtd->input_len % gtd->screen->cols;
*/
	}
}


DO_CURSOR(cursor_redraw_line)
{
	if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_BIG5))
	{
		if (inputline_str_chk(0, gtd->input_len) == FALSE)
		{
			return;
		}
	}

	// Erase current input

	input_printf("\e[%dG\e[%dP", gtd->input_off, inputline_max_str_len());

	// Center long lines of input

	if (gtd->input_pos > inputline_max_str_len() - 3)
	{
		while (gtd->input_pos - gtd->input_hid > inputline_max_str_len() - 3)
		{
			gtd->input_hid += inputline_max_str_len() / 2;
		}

		while (gtd->input_pos - gtd->input_hid < 2)
		{
			gtd->input_hid -= inputline_max_str_len() / 2;
		}
	}
	else
	{
		if (gtd->input_hid && gtd->input_pos - gtd->input_hid < 2)
		{
			gtd->input_hid = 0;
		}
	}

	// Print the entire thing

	if (gtd->input_hid)
	{
		if (gtd->input_hid + inputline_max_str_len() >= inputline_raw_str_len(0, gtd->input_len))
		{
			input_printf("<%.*s\e[%dG",  inputline_str_raw_len(gtd->input_hid + 1, gtd->input_hid + inputline_max_str_len() - 0), &gtd->input_buf[inputline_str_raw_len(0, gtd->input_hid + 1)], gtd->input_off + gtd->input_pos - gtd->input_hid);
		}
		else
		{
			input_printf("<%.*s>\e[%dG", inputline_str_raw_len(gtd->input_hid + 1, gtd->input_hid + inputline_max_str_len() - 1), &gtd->input_buf[inputline_str_raw_len(0, gtd->input_hid + 1)], gtd->input_off + gtd->input_pos - gtd->input_hid);
		}
	}
	else
	{
		if (gtd->input_hid + inputline_max_str_len() >= inputline_raw_str_len(0, gtd->input_len))
		{
			input_printf("%.*s\e[%dG",   inputline_str_raw_len(gtd->input_hid + 0, gtd->input_hid + inputline_max_str_len() - 0), &gtd->input_buf[inputline_str_raw_len(0, gtd->input_hid + 0)], gtd->input_off + gtd->input_pos - gtd->input_hid);
		}
		else
		{
			input_printf("%.*s>\e[%dG",  inputline_str_raw_len(gtd->input_hid + 0, gtd->input_hid + inputline_max_str_len() - 1), &gtd->input_buf[inputline_str_raw_len(0, gtd->input_hid + 0)], gtd->input_off + gtd->input_pos - gtd->input_hid);
		}
	}
}

DO_CURSOR(cursor_right)
{
	int size, width;

	if (gtd->input_cur < gtd->input_len)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_BIG5) && (gtd->input_buf[gtd->input_cur] & 128) == 128)
		{
			if (gtd->input_cur + 1 < gtd->input_len && gtd->input_buf[gtd->input_cur+1])
			{
				gtd->input_cur += 2;
				gtd->input_pos += 2;

				input_printf("\e[2C");
			}
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			gtd->input_cur += get_utf8_width(&gtd->input_buf[gtd->input_cur], &width);

			if (width == 0)
			{
				return cursor_right(ses, "");
			}
			input_printf("\e[%dC", width);
			gtd->input_pos += width;

			while (gtd->input_cur < gtd->input_len)
			{
				size = get_utf8_width(&gtd->input_buf[gtd->input_cur], &width);

				if (width)
				{
					break;
				}
				gtd->input_cur += size;
			}
		}
		else
		{
			input_printf("\e[1C");
			gtd->input_cur++;
			gtd->input_pos++;
		}
	}

	cursor_check_line(ses, "");
}

DO_CURSOR(cursor_right_word)
{
	if (gtd->input_cur == gtd->input_len)
	{
		return;
	}

	while (gtd->input_cur < gtd->input_len && gtd->input_buf[gtd->input_cur] == ' ')
	{
		gtd->input_cur++;
		gtd->input_pos++;
	}

	while (gtd->input_cur < gtd->input_len && gtd->input_buf[gtd->input_cur] != ' ')
	{
		if (!HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) || (gtd->input_buf[gtd->input_cur] & 192) != 128)
		{
			gtd->input_pos++;
		}
		gtd->input_cur++;
	}

	cursor_redraw_line(ses, "");
}

DO_CURSOR(cursor_set)
{
	if (*arg == 0)
	{
		return;
	}

	ins_sprintf(&gtd->input_buf[gtd->input_cur], "%s", arg);

	gtd->input_len += strlen(arg);
	gtd->input_cur += strlen(arg);

	gtd->input_pos += inputline_raw_str_len(gtd->input_cur - strlen(arg), gtd->input_cur);

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_suspend)
{
	do_suspend(ses, "");
}

DO_CURSOR(cursor_info)
{
	tintin_printf2(ses, "Width of input bar:             %10d", inputline_max_str_len());
	tintin_printf2(ses, "Offset of input bar:            %10d", gtd->input_off);
	tintin_printf2(ses, "Width of hidden text on left:   %10d", gtd->input_hid);
	tintin_printf2(ses, "VT100 position of cursor:       %10d", gtd->input_pos);
	tintin_printf2(ses, "internal position of cursor:    %10d", gtd->input_cur);
	tintin_printf2(ses, "internal length of input line:  %10d", gtd->input_len);
	tintin_printf2(ses, "VT100 length of displayed line: %10d", inputline_cur_str_len());
}

/*
	Improved tab handling by Ben Love
*/

int cursor_tab_add(int input_now, int stop_after_first)
{
	struct listroot *root = gtd->ses->list[LIST_TAB];
	struct listnode *node;

	char tab[BUFFER_SIZE];

	if (!HAS_BIT(root->flags, LIST_FLAG_IGNORE))
	{
		for (root->update = 0 ; root->update < root->used ; root->update++)
		{
			node = root->list[root->update];

			substitute(gtd->ses, node->arg1, tab, SUB_VAR|SUB_FUN);

			if (!strncmp(tab, &gtd->input_buf[input_now], strlen(&gtd->input_buf[input_now])))
			{
				if (search_node_list(gtd->ses->list[LIST_COMMAND], tab))
				{
					continue;
				}
				insert_node_list(gtd->ses->list[LIST_COMMAND], tab, "", "", "");

				if (HAS_BIT(node->flags, NODE_FLAG_ONESHOT))
				{
					delete_node_list(gtd->ses, LIST_TAB, node);
				}

				if (stop_after_first)
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


int cursor_auto_tab_add(int input_now, int stop_after_first)
{
	char tab[BUFFER_SIZE], buf[BUFFER_SIZE];
	int scroll_cnt, line_cnt, tab_len;
	char *arg;

	line_cnt = 0;

	for (scroll_cnt = gtd->ses->scroll->used - 1 ; scroll_cnt > 0 ; scroll_cnt--)
	{
		if (HAS_BIT(gtd->ses->scroll->buffer[scroll_cnt]->flags, BUFFER_FLAG_GREP))
		{
			continue;
		}

		if (line_cnt++ >= gtd->ses->auto_tab)
		{
			break;
		}

		strip_vt102_codes(gtd->ses->scroll->buffer[scroll_cnt]->str, buf);

		arg = buf;

		while (*arg)
		{
			arg = get_arg_in_braces(gtd->ses, arg, tab, GET_ONE);

			if (!strncmp(tab, &gtd->input_buf[input_now], strlen(&gtd->input_buf[input_now])))
			{
				tab_len = strlen(tab) -1;

				if (tab_len > 0)
				{
					if (tab[tab_len] == '.' || tab[tab_len] == ',' || tab[tab_len] == ';')
					{
						tab[tab_len] = 0;
					}
				}

				if (search_node_list(gtd->ses->list[LIST_COMMAND], tab))
				{
					continue;
				}
				insert_node_list(gtd->ses->list[LIST_COMMAND], tab, "", "", "");

				if (stop_after_first)
				{
					return TRUE;
				}
			}

			if (*arg == COMMAND_SEPARATOR)
			{
				arg++;
			}

		}
	}

	return FALSE;
}

void cursor_hide_completion(int input_now)
{
	struct listroot *root = gtd->ses->list[LIST_COMMAND];
	struct listnode *f_node;
	struct listnode *l_node;
	int len_change;

	f_node = root->list[0];
	l_node = root->list[root->used - 1];

	if (root->used && !strcmp(l_node->arg1, gtd->input_buf + input_now))
	{
		len_change = strlen(l_node->arg1) - strlen(f_node->arg1);

		if (len_change > 0)
		{
/*
			if (gtd->input_cur < gtd->input_len)
			{
				input_printf("\e[%dC", gtd->input_len - gtd->input_cur);
			}
			input_printf("\e[%dD\e[%dP", len_change, len_change);
*/
			gtd->input_len = gtd->input_len - len_change;
			gtd->input_buf[gtd->input_len] = 0;
			gtd->input_cur = gtd->input_len;
			gtd->input_pos = gtd->input_pos;
		}
	}
	return;
}

void cursor_show_completion(int input_now, int show_last_node)
{
	struct listroot *root = gtd->ses->list[LIST_COMMAND];
	struct listnode *node;

	if (!root->used)
	{
		return;
	}

	node = show_last_node ? root->list[root->used - 1] : root->list[0];
/*
	if (gtd->input_cur < gtd->input_len)
	{
		input_printf("\e[%dC", gtd->input_len - gtd->input_cur);
	}
	if (gtd->input_len > input_now)
	{
		input_printf("\e[%dD\e[%dP", gtd->input_len - input_now, gtd->input_len - input_now);
	}
	if (input_now + (int) strlen(node->arg1) < gtd->ses->cols - 2)
	{
		input_printf("%s", node->arg1);
	}
*/
	strcpy(&gtd->input_buf[input_now], node->arg1);

	gtd->input_len = input_now + strlen(node->arg1);
	gtd->input_cur = gtd->input_len;

	cursor_end(gtd->ses, "");

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		cursor_history_find(gtd->ses, "");
	}

	if (node == root->list[0])
	{
		kill_list(root);
	}

	return;
}

int cursor_calc_input_now(void)
{
	int input_now;

	if (gtd->input_len == 0 || gtd->input_buf[gtd->input_len - 1] == ' ')
	{
		return -1;
	}

	for (input_now = gtd->input_len - 1 ; input_now ; input_now--)
	{
		if (gtd->input_buf[input_now] == ' ')
		{
			return input_now + 1;
		}
	}

	return input_now;
}

DO_CURSOR(cursor_tab_forward)
{
	struct listroot *root = ses->list[LIST_COMMAND];
	int tab_found;

	if (!root->list[0])
	{
		gtd->input_tab = cursor_calc_input_now();
	}

	if (gtd->input_tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->input_tab);

	if (!root->list[0])
	{
		insert_node_list(root, &gtd->input_buf[gtd->input_tab], "", "", "");
	}
	tab_found = cursor_tab_add(gtd->input_tab, TRUE);

	cursor_show_completion(gtd->input_tab, tab_found);
}

DO_CURSOR(cursor_tab_backward)
{
	struct listroot *root = ses->list[LIST_COMMAND];

	if (!root->list[0])
	{
		gtd->input_tab = cursor_calc_input_now();
	}

	if (gtd->input_tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->input_tab);

	if (root->used)
	{
		delete_index_list(root, root->used - 1);
	}

	if (!root->list[0])
	{
		insert_node_list(root, &gtd->input_buf[gtd->input_tab], "", "", "");

		cursor_tab_add(gtd->input_tab, FALSE);
	}
	cursor_show_completion(gtd->input_tab, TRUE);
}

DO_CURSOR(cursor_auto_tab_forward)
{
	struct listroot *root = ses->list[LIST_COMMAND];
	int tab_found;

	if (!root->list[0])
	{
		gtd->input_tab = cursor_calc_input_now();
	}

	if (gtd->input_tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->input_tab);

	if (!root->list[0])
	{
		insert_node_list(root, &gtd->input_buf[gtd->input_tab], "", "", "");
	}

	tab_found = cursor_auto_tab_add(gtd->input_tab, TRUE);

	cursor_show_completion(gtd->input_tab, tab_found);
}

DO_CURSOR(cursor_auto_tab_backward)
{
	struct listroot *root = ses->list[LIST_COMMAND];

	if (!root->list[0])
	{
		gtd->input_tab = cursor_calc_input_now();
	}

	if (gtd->input_tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->input_tab);

	if (root->used)
	{
		delete_index_list(root, root->used - 1);
	}

	if (!root->list[0])
	{
		insert_node_list(root, &gtd->input_buf[gtd->input_tab], "", "", "");

		cursor_auto_tab_add(gtd->input_tab, FALSE);
	}

	cursor_show_completion(gtd->input_tab, TRUE);
}


DO_CURSOR(cursor_mixed_tab_forward)
{
	struct listroot *root = ses->list[LIST_COMMAND];
	int tab_found;

	if (!root->list[0])
	{
		gtd->input_tab = cursor_calc_input_now();
	}

	if (gtd->input_tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->input_tab);

	if (!root->list[0])
	{
		insert_node_list(root, &gtd->input_buf[gtd->input_tab], "", "", "");
	}
	tab_found = cursor_tab_add(gtd->input_tab, TRUE) || cursor_auto_tab_add(gtd->input_tab, TRUE);

	cursor_show_completion(gtd->input_tab, tab_found);
}

DO_CURSOR(cursor_mixed_tab_backward)
{
	struct listroot *root = ses->list[LIST_COMMAND];

	if (!root->list[0])
	{
		gtd->input_tab = cursor_calc_input_now();
	}

	if (gtd->input_tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->input_tab);

	if (root->used)
	{
		delete_index_list(root, root->used - 1);
	}

	if (!root->list[0])
	{
		insert_node_list(root, &gtd->input_buf[gtd->input_tab], "", "", "");

		cursor_tab_add(gtd->input_tab, FALSE);
		cursor_auto_tab_add(gtd->input_tab, FALSE);
	}

	cursor_show_completion(gtd->input_tab, TRUE);
}

DO_CURSOR(cursor_screen_focus_in)
{
	gtd->screen->focus = 1;

	check_all_events(gtd->ses, SUB_ARG, 1, 1, "WINDOW FOCUS IN", ntos(gtd->screen->focus));

	msdp_update_all("SCREEN_FOCUS", "%d", gtd->screen->focus);
}

DO_CURSOR(cursor_screen_focus_out)
{
	gtd->screen->focus = 0;

	check_all_events(gtd->ses, SUB_ARG, 0, 1, "WINDOW FOCUS OUT", ntos(gtd->screen->focus));

	msdp_update_all("SCREEN_FOCUS", "%d", gtd->screen->focus);
}
