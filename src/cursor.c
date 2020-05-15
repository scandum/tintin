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

DO_COMMAND(do_cursor)
{
	char all[BUFFER_SIZE], temp[BUFFER_SIZE];
	int cnt;

	get_arg_in_braces(ses, arg, all, GET_ALL);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_header(ses, " CURSOR OPTIONS ");

		for (cnt = 0 ; *cursor_table[cnt].fun ; cnt++)
		{
			if (*cursor_table[cnt].name)
			{
				convert_meta(cursor_table[cnt].code, temp, FALSE);

				tintin_printf2(ses, "  [%-18s] [%-7s] %s", cursor_table[cnt].name, temp, cursor_table[cnt].desc);
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
					cursor_table[cnt].fun(ses, arg);

					return ses;
				}
			}
			else if (HAS_BIT(cursor_table[cnt].flags, CURSOR_FLAG_GET_ONE))
			{
				if (is_abbrev(arg1, cursor_table[cnt].name))
				{
					cursor_table[cnt].fun(ses, arg);

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

// strlen with offset.

int inputline_str_str_len(int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt, width;

	raw_cnt = str_cnt = ret_cnt = 0;

	while (raw_cnt < gtd->ses->input->len)
	{
		if (str_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&gtd->ses->input->buf[raw_cnt]))
		{
			raw_cnt += get_utf8_width(&gtd->ses->input->buf[raw_cnt], &width);

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

	while (raw_cnt < gtd->ses->input->len)
	{
		if (raw_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&gtd->ses->input->buf[raw_cnt]))
		{
			raw_cnt += get_utf8_width(&gtd->ses->input->buf[raw_cnt], &width);
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

	while (raw_cnt < gtd->ses->input->len)
	{
		if (str_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&gtd->ses->input->buf[raw_cnt]))
		{    
			tmp_cnt = get_utf8_width(&gtd->ses->input->buf[raw_cnt], &width);

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
	return gtd->ses->input->bot_col - gtd->ses->input->top_col + 2 - gtd->ses->input->off - (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH) ? 11 : 0);

//	return gtd->screen->cols + 1 - gtd->ses->input->off - (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH) ? 11 : 0);
}

int inputline_cur_str_len(void)
{
	return inputline_str_str_len(gtd->ses->input->hid, gtd->ses->input->hid + inputline_max_str_len());
}

// Get the position of the cursor

int inputline_cur_row(void)
{
	return gtd->ses->input->top_row;
//	return gtd->screen->rows;
}

int inputline_cur_col(void)
{
	return gtd->ses->input->off + gtd->ses->input->pos - gtd->ses->input->hid;

//	return gtd->ses->input->off + gtd->ses->input->pos - gtd->ses->input->hid;
}

// Get the maximum number of rows of the input region

int inputline_max_row(void)
{
	return 1 + gtd->ses->input->bot_row - gtd->ses->input->top_row;
}

// Check for invalid characters.

int inputline_str_chk(int offset, int totlen)
{
	int size;

	while (offset < totlen)
	{
		if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_EUC))
		{
			if (is_euc_head(gtd->ses, &gtd->ses->input->buf[offset]))
			{
				size = get_euc_size(gtd->ses, &gtd->ses->input->buf[offset]);

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
		else if (HAS_BIT(gtd->ses->charset, CHARSET_FLAG_UTF8))
		{
			if (is_utf8_head(&gtd->ses->input->buf[offset]))
			{
				size = get_utf8_size(&gtd->ses->input->buf[offset]);

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
	if (gtd->ses->input->cur == 0)
	{
		return;
	}

	cursor_left(ses, "");
	cursor_delete(ses, "");

	modified_input();
}

DO_CURSOR(cursor_brace_open)
{
	ins_sprintf(&gtd->ses->input->buf[gtd->ses->input->cur], "{");

	gtd->ses->input->len++;
	gtd->ses->input->cur++;

	gtd->ses->input->pos += inputline_raw_str_len(gtd->ses->input->cur - 1, gtd->ses->input->cur);

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_brace_close)
{
	ins_sprintf(&gtd->ses->input->buf[gtd->ses->input->cur], "}");

	gtd->ses->input->len++;
	gtd->ses->input->cur++;

	gtd->ses->input->pos += inputline_raw_str_len(gtd->ses->input->cur - 1, gtd->ses->input->cur);

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_buffer_down)
{
	buffer_down(ses, "");
}

DO_CURSOR(cursor_buffer_end)
{
	buffer_end(ses, "");
}

DO_CURSOR(cursor_buffer_home)
{
	buffer_home(ses, "");
}

DO_CURSOR(cursor_buffer_lock)
{
	buffer_lock(ses, "");
}

DO_CURSOR(cursor_buffer_up)
{
	buffer_up(ses, "");
}


DO_CURSOR(cursor_check_line)
{
	if (gtd->ses->input->pos - gtd->ses->input->hid > inputline_max_str_len() - 3)
	{
		return cursor_redraw_line(ses, "");
	}

	if (gtd->ses->input->hid && gtd->ses->input->pos - gtd->ses->input->hid < 2)
	{
		return cursor_redraw_line(ses, "");
	}
}

DO_CURSOR(cursor_check_line_modified)
{
	if (gtd->ses->input->hid + inputline_max_str_len() < inputline_raw_str_len(0, gtd->ses->input->len))
	{
		return cursor_redraw_line(ses, "");
	}

	return cursor_check_line(ses, "");
}

DO_CURSOR(cursor_clear_left)
{
	if (gtd->ses->input->cur == 0)
	{
		return;
	}

	sprintf(gtd->paste_buf, "%.*s", gtd->ses->input->cur, gtd->ses->input->buf);

	input_printf("\e[%dG\e[%dP", gtd->ses->input->off, gtd->ses->input->pos - gtd->ses->input->hid);

	memmove(&gtd->ses->input->buf[0], &gtd->ses->input->buf[gtd->ses->input->cur], gtd->ses->input->len - gtd->ses->input->cur);

	gtd->ses->input->len -= gtd->ses->input->cur;

	gtd->ses->input->buf[gtd->ses->input->len] = 0;

	gtd->ses->input->cur  = 0;
	gtd->ses->input->pos  = 0;

	cursor_check_line_modified(ses, "");

	modified_input();
}

DO_CURSOR(cursor_clear_line)
{
	if (gtd->ses->input->len == 0)
	{
		return;
	}

	sprintf(gtd->paste_buf, "%s", gtd->ses->input->buf);

	input_printf("\e[%dG\e[%dP", gtd->ses->input->off, inputline_cur_str_len());

	gtd->ses->input->len = 0;
	gtd->ses->input->cur = 0;
	gtd->ses->input->hid = 0;
	gtd->ses->input->pos = 0;
	gtd->ses->input->buf[0] = 0;

	modified_input();
}

DO_CURSOR(cursor_clear_right)
{
	if (gtd->ses->input->cur == gtd->ses->input->len)
	{
		return;
	}

	strcpy(gtd->paste_buf, &gtd->ses->input->buf[gtd->ses->input->cur]);

	input_printf("\e[%dP", inputline_max_str_len() - inputline_str_str_len(gtd->ses->input->hid, gtd->ses->input->pos));

	gtd->ses->input->buf[gtd->ses->input->cur] = 0;

	gtd->ses->input->len = gtd->ses->input->cur;

	modified_input();
}

DO_CURSOR(cursor_convert_meta)
{
	SET_BIT(gtd->flags, TINTIN_FLAG_CONVERTMETACHAR);
}

DO_CURSOR(cursor_delete_or_exit)
{
	if (gtd->ses->input->len == 0)
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

	if (gtd->ses->input->len == 0)
	{
		return;
	}

	if (gtd->ses->input->len == gtd->ses->input->cur)
	{
		return;
	}

	if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->cur]))
	{
		size = get_euc_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->cur], &width);

		gtd->ses->input->len -= size;

		memmove(&gtd->ses->input->buf[gtd->ses->input->cur], &gtd->ses->input->buf[gtd->ses->input->cur + size], gtd->ses->input->len - gtd->ses->input->cur + 1);

		if (width)
		{
			input_printf("\e[%dP", width);
		}
	}
	else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
	{
		size = get_utf8_width(&gtd->ses->input->buf[gtd->ses->input->cur], &width);

		gtd->ses->input->len -= size;

		memmove(&gtd->ses->input->buf[gtd->ses->input->cur], &gtd->ses->input->buf[gtd->ses->input->cur + size], gtd->ses->input->len - gtd->ses->input->cur + 1);

		if (width)
		{
			input_printf("\e[%dP", width);
		}

		while (gtd->ses->input->len > gtd->ses->input->cur)
		{
			size = get_utf8_width(&gtd->ses->input->buf[gtd->ses->input->cur], &width);

			if (width)
			{
				break;
			}
			gtd->ses->input->len -= size;

			memmove(&gtd->ses->input->buf[gtd->ses->input->cur], &gtd->ses->input->buf[gtd->ses->input->cur + size], gtd->ses->input->len - gtd->ses->input->cur + 1);
		}
	}
	else
	{
		gtd->ses->input->len--;

		memmove(&gtd->ses->input->buf[gtd->ses->input->cur], &gtd->ses->input->buf[gtd->ses->input->cur+1], gtd->ses->input->len - gtd->ses->input->cur + 1);

		input_printf("\e[1P");
	}

	if (gtd->ses->input->hid + inputline_max_str_len() <= inputline_raw_str_len(0, gtd->ses->input->len))
	{
		cursor_redraw_line(ses, "");
	}

	modified_input();
}

DO_CURSOR(cursor_delete_word_left)
{
	int index_cur, width;

	if (gtd->ses->input->cur == 0)
	{
		return;
	}

	index_cur = gtd->ses->input->cur;

	while (gtd->ses->input->cur > 0 && gtd->ses->input->buf[gtd->ses->input->cur - 1] == ' ')
	{
		gtd->ses->input->pos--;
		gtd->ses->input->cur--;
	}

	while (gtd->ses->input->cur > 0 && gtd->ses->input->buf[gtd->ses->input->cur - 1] != ' ')
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			if (is_utf8_tail(&gtd->ses->input->buf[gtd->ses->input->cur]))
			{
				gtd->ses->input->cur--;
			}
			else if (is_utf8_head(&gtd->ses->input->buf[gtd->ses->input->cur]))
			{
				get_utf8_width(&gtd->ses->input->buf[gtd->ses->input->cur], &width);

				gtd->ses->input->cur -= 1;
				gtd->ses->input->pos -= width;
			}
			else
			{
				gtd->ses->input->cur--;
				gtd->ses->input->pos--;
			}
		}
		else
		{
			gtd->ses->input->pos--;
			gtd->ses->input->cur--;
		}
	}

	sprintf(gtd->paste_buf, "%.*s", index_cur - gtd->ses->input->cur, &gtd->ses->input->buf[gtd->ses->input->cur]);

	memmove(&gtd->ses->input->buf[gtd->ses->input->cur], &gtd->ses->input->buf[index_cur], gtd->ses->input->len - index_cur + 1);

//	input_printf("\e[%dD\e[%dP", index_cur - gtd->ses->input->cur, index_cur - gtd->ses->input->cur);

	gtd->ses->input->len -= index_cur - gtd->ses->input->cur;

	cursor_redraw_line(ses, "");

	modified_input();
}


DO_CURSOR(cursor_delete_word_right)
{
	int index_cur;

	if (gtd->ses->input->cur == gtd->ses->input->len)
	{
		return;
	}

	index_cur = gtd->ses->input->cur;

	while (gtd->ses->input->cur != gtd->ses->input->len && gtd->ses->input->buf[gtd->ses->input->cur] == ' ')
	{
		gtd->ses->input->cur++;
	}

	while (gtd->ses->input->cur != gtd->ses->input->len && gtd->ses->input->buf[gtd->ses->input->cur] != ' ')
	{
		gtd->ses->input->cur++;
	}

	sprintf(gtd->paste_buf, "%.*s", gtd->ses->input->cur - index_cur, &gtd->ses->input->buf[gtd->ses->input->cur]);

	memmove(&gtd->ses->input->buf[index_cur], &gtd->ses->input->buf[gtd->ses->input->cur], gtd->ses->input->len - gtd->ses->input->cur + 1);

//	input_printf("\e[%dP", gtd->ses->input->cur - index_cur);

	gtd->ses->input->len -= gtd->ses->input->cur - index_cur;

	gtd->ses->input->cur = index_cur;

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_echo)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		TOG_BIT(ses->telopts, TELOPT_FLAG_ECHO);
	}
	else if (!strcasecmp(arg1, "ON"))
	{
		SET_BIT(ses->telopts, TELOPT_FLAG_ECHO);
	}
	else if (!strcasecmp(arg1, "OFF"))
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
	gtd->ses->input->cur = gtd->ses->input->len;

	gtd->ses->input->pos = inputline_raw_str_len(0, gtd->ses->input->len);

	cursor_redraw_line(ses, "");
}

DO_CURSOR(cursor_enter)
{
	input_printf("\n");

	gtd->ses->input->buf[gtd->ses->input->len] = 0;

	gtd->ses->input->len    = 0;
	gtd->ses->input->cur    = 0;
	gtd->ses->input->pos    = 0;
	gtd->ses->input->off    = 1;
	gtd->ses->input->hid    = 0;
	gtd->macro_buf[0] = 0;
	gtd->ses->input->tmp[0] = 0;

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH))
	{
		struct listroot *root = ses->list[LIST_HISTORY];

		if (root->update >= 0 && root->update < root->used)
		{
			strcpy(gtd->ses->input->buf, root->list[root->update]->arg1);
		}

		DEL_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH);

		gtd->ses->input->off = 1;
	}

	SET_BIT(gtd->flags, TINTIN_FLAG_PROCESSINPUT);

	modified_input();
}

DO_CURSOR(cursor_exit)
{
	if (ses == gts)
	{
		execute(ses, "#END");
	}
	else
	{
		cleanup_session(ses);
	}
}

DO_CURSOR(cursor_get)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR GET {variable}");
	}
	else
	{
		set_nest_node_ses(ses, arg1, "%s", gtd->ses->input->buf);
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
			if (*gtd->ses->input->buf && find(ses, root->list[root->update]->arg1, gtd->ses->input->buf, SUB_NONE, REGEX_FLAG_NONE))
			{
				break;
			}
		}

		if (root->update < root->used)
		{
			input_printf("\e[%dG  \e[0K%.*s\e[%dG", gtd->ses->input->off + inputline_cur_str_len() + 2, gtd->ses->input->off + inputline_max_str_len() - inputline_cur_str_len() - 4, root->list[root->update]->arg1, gtd->ses->input->off + gtd->ses->input->pos - gtd->ses->input->hid);
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
			if (!strncmp(gtd->ses->input->tmp, root->list[root->update]->arg1, strlen(gtd->ses->input->tmp)))
			{
				break;
			}
		}
	}

	cursor_clear_line(ses, "");

	if (root->update == root->used)
	{
		strcpy(gtd->ses->input->buf, gtd->ses->input->tmp);
	}
	else
	{
		strcpy(gtd->ses->input->buf, root->list[root->update]->arg1);
	}

	gtd->ses->input->len = strlen(gtd->ses->input->buf);

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
			if (*gtd->ses->input->buf && find(ses, root->list[root->update]->arg1, gtd->ses->input->buf, SUB_NONE, REGEX_FLAG_NONE))
			{
				break;
			}
		}

		if (root->update >= 0)
		{
			input_printf("\e[%dG  \e[0K%.*s\e[%dG", gtd->ses->input->off + inputline_cur_str_len() + 2, gtd->ses->input->off + inputline_max_str_len() - inputline_cur_str_len() - 4, root->list[root->update]->arg1, gtd->ses->input->off + gtd->ses->input->pos - gtd->ses->input->hid);
		}
		return;
	}

	if (root->list[0] == NULL)
	{
		return;
	}

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HISTORYBROWSE))
	{
		strcpy(gtd->ses->input->tmp, gtd->ses->input->buf);

		for (root->update = root->used - 1 ; root->update >= 0 ; root->update--)
		{
			if (!strncmp(gtd->ses->input->tmp, root->list[root->update]->arg1, strlen(gtd->ses->input->tmp)))
			{
				break;
			}
		}
	}
	else if (root->update >= 0)
	{
		for (root->update-- ; root->update >= 0 ; root->update--)
		{
			if (!strncmp(gtd->ses->input->tmp, root->list[root->update]->arg1, strlen(gtd->ses->input->tmp)))
			{
				break;
			}
		}
	}

	cursor_clear_line(ses, "");

	if (root->update == -1)
	{
		strcpy(gtd->ses->input->buf, gtd->ses->input->tmp);
	}
	else
	{
		strcpy(gtd->ses->input->buf, root->list[root->update]->arg1);
	}

	gtd->ses->input->len = strlen(gtd->ses->input->buf);

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
		strcpy(gtd->ses->input->tmp, gtd->ses->input->buf);

		cursor_clear_line(ses, "");

		SET_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH);

		gtd->ses->input->off = 11;

		root->update = root->used - 1;

		input_printf("(search) [ ] \e[3D");
	}
	else
	{
		if (root->update >= 0 && root->update < root->used)
		{
			strcpy(gtd->ses->input->buf, root->list[root->update]->arg1);
		}
		input_printf("\e[1G\e[0K");

		gtd->ses->input->len = strlen(gtd->ses->input->buf);
		gtd->ses->input->cur = gtd->ses->input->len;
		gtd->ses->input->pos = 0;

		root->update = -1;

		DEL_BIT(gtd->flags, TINTIN_FLAG_HISTORYSEARCH);

		gtd->ses->input->off = 1;

		cursor_redraw_line(ses, "");
	}
}

DO_CURSOR(cursor_history_find)
{
	struct listroot *root = ses->list[LIST_HISTORY];

	push_call("cursor_history_find(%s)", gtd->ses->input->buf);

	if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_EUC))
	{
		if (inputline_str_chk(0, gtd->ses->input->len) == FALSE)
		{
			pop_call();
			return;
		}
	}

	gtd->level->quiet++;

	for (root->update = root->used - 1 ; root->update >= 0 ; root->update--)
	{
		if (*gtd->ses->input->buf && find(ses, root->list[root->update]->arg1, gtd->ses->input->buf, SUB_NONE, REGEX_FLAG_NONE))
		{
			break;
		}
	}

	gtd->level->quiet--;

	if (root->update >= 0)
	{
		input_printf("\e[%dG ]  %.*s\e[%dG", gtd->ses->input->off + inputline_cur_str_len(), gtd->ses->input->off + inputline_max_str_len() - inputline_cur_str_len() - 4, root->list[root->update]->arg1, gtd->ses->input->off + gtd->ses->input->pos - gtd->ses->input->hid);
	}
	else
	{
		input_printf("\e[%dG ] \e[0K\e[%dG", gtd->ses->input->off + inputline_cur_str_len(), gtd->ses->input->off + gtd->ses->input->pos - gtd->ses->input->hid);
	}
	pop_call();
	return;
}

DO_CURSOR(cursor_home)
{
	if (gtd->ses->input->cur == 0)
	{
		return;
	}

	input_printf("\e[%dD", gtd->ses->input->pos - gtd->ses->input->hid);

	gtd->ses->input->cur = 0;
	gtd->ses->input->pos = 0;

	if (gtd->ses->input->hid)
	{
		gtd->ses->input->hid = 0;

		cursor_redraw_line(ses, "");
	}
}

DO_CURSOR(cursor_insert)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		TOG_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
	}
	else if (!strcasecmp(arg1, "ON"))
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
	}
	else if (!strcasecmp(arg1, "OFF"))
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

	if (gtd->ses->input->cur > 0)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC))
		{
			gtd->ses->input->cur--;
			gtd->ses->input->pos--;
			input_printf("\e[1D");

			if (inputline_str_chk(0, gtd->ses->input->cur) == FALSE)
			{
				gtd->ses->input->cur--;
				gtd->ses->input->pos--;
				input_printf("\e[1D");
			}
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			gtd->ses->input->cur--;

			if (gtd->ses->input->cur > 0 && is_utf8_tail(&gtd->ses->input->buf[gtd->ses->input->cur]))
			{
				do
				{
					gtd->ses->input->cur--;
				}
				while (gtd->ses->input->cur > 0 && is_utf8_tail(&gtd->ses->input->buf[gtd->ses->input->cur]));

				get_utf8_width(&gtd->ses->input->buf[gtd->ses->input->cur], &width);

				if (width == 0)
				{
					return cursor_left(ses, "");
				}
				input_printf("\e[%dD", width);
				gtd->ses->input->pos -= width;
			}
			else
			{
				gtd->ses->input->pos--;
				input_printf("\e[1D");
			}

		}
		else
		{
			gtd->ses->input->cur--;
			gtd->ses->input->pos--;
			input_printf("\e[1D");
		}
		cursor_check_line(ses, "");
	}
}

DO_CURSOR(cursor_left_word)
{
	int width;
//	int index_pos;

	if (gtd->ses->input->cur == 0)
	{
		return;
	}

//	index_pos = gtd->ses->input->pos;

	while (gtd->ses->input->cur > 0 && gtd->ses->input->buf[gtd->ses->input->cur - 1] == ' ')
	{
		gtd->ses->input->pos--;
		gtd->ses->input->cur--;
	}

	while (gtd->ses->input->cur > 0 && gtd->ses->input->buf[gtd->ses->input->cur - 1] != ' ')
	{
		gtd->ses->input->cur--;

		if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			if (!is_utf8_tail(&gtd->ses->input->buf[gtd->ses->input->cur]))
			{
				get_utf8_width(&gtd->ses->input->buf[gtd->ses->input->cur], &width);

				gtd->ses->input->pos -= width;
			}
		}
		else
		{
			gtd->ses->input->pos--;
		}
	}

//	input_printf("\e[%dD", index_pos - gtd->ses->input->pos);

	cursor_redraw_line(ses, "");
}


DO_CURSOR(cursor_paste_buffer)
{
	if (*gtd->paste_buf == 0)
	{
		return;
	}

	ins_sprintf(&gtd->ses->input->buf[gtd->ses->input->cur], "%s", gtd->paste_buf);

	gtd->ses->input->len += strlen(gtd->paste_buf);
	gtd->ses->input->cur += strlen(gtd->paste_buf);

	gtd->ses->input->pos += inputline_raw_str_len(gtd->ses->input->cur - strlen(gtd->paste_buf), gtd->ses->input->cur);

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_preserve_macro)
{
	SET_BIT(gtd->flags, TINTIN_FLAG_PRESERVEMACRO);
}

DO_CURSOR(cursor_reset_macro)
{
	gtd->macro_buf[0] = 0;
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
		gtd->ses->input->cur = gtd->ses->input->len;

		gtd->ses->input->pos = gtd->ses->input->len % gtd->screen->cols;
*/
	}
}


DO_CURSOR(cursor_redraw_line)
{
	if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_EUC))
	{
		if (inputline_str_chk(0, gtd->ses->input->len) == FALSE)
		{
			return;
		}
	}

	// Erase current input

	input_printf("\e[%dG\e[%dP", gtd->ses->input->off, inputline_max_str_len());

	// Center long lines of input

	if (gtd->ses->input->pos > inputline_max_str_len() - 3)
	{
		while (gtd->ses->input->pos - gtd->ses->input->hid > inputline_max_str_len() - 3)
		{
			gtd->ses->input->hid += inputline_max_str_len() / 2;
		}

		while (gtd->ses->input->pos - gtd->ses->input->hid < 2)
		{
			gtd->ses->input->hid -= inputline_max_str_len() / 2;
		}
	}
	else
	{
		if (gtd->ses->input->hid && gtd->ses->input->pos - gtd->ses->input->hid < 2)
		{
			gtd->ses->input->hid = 0;
		}
	}

	// Print the entire thing

	if (gtd->ses->input->hid)
	{
		if (gtd->ses->input->hid + inputline_max_str_len() >= inputline_raw_str_len(0, gtd->ses->input->len))
		{
			input_printf("<%.*s\e[%dG",  inputline_str_raw_len(gtd->ses->input->hid + 1, gtd->ses->input->hid + inputline_max_str_len() - 0), &gtd->ses->input->buf[inputline_str_raw_len(0, gtd->ses->input->hid + 1)], gtd->ses->input->off + gtd->ses->input->pos - gtd->ses->input->hid);
		}
		else
		{
			input_printf("<%.*s>\e[%dG", inputline_str_raw_len(gtd->ses->input->hid + 1, gtd->ses->input->hid + inputline_max_str_len() - 1), &gtd->ses->input->buf[inputline_str_raw_len(0, gtd->ses->input->hid + 1)], gtd->ses->input->off + gtd->ses->input->pos - gtd->ses->input->hid);
		}
	}
	else
	{
		if (gtd->ses->input->hid + inputline_max_str_len() >= inputline_raw_str_len(0, gtd->ses->input->len))
		{
			input_printf("%.*s\e[%dG",   inputline_str_raw_len(gtd->ses->input->hid + 0, gtd->ses->input->hid + inputline_max_str_len() - 0), &gtd->ses->input->buf[inputline_str_raw_len(0, gtd->ses->input->hid + 0)], gtd->ses->input->off + gtd->ses->input->pos - gtd->ses->input->hid);
		}
		else
		{
			input_printf("%.*s>\e[%dG",  inputline_str_raw_len(gtd->ses->input->hid + 0, gtd->ses->input->hid + inputline_max_str_len() - 1), &gtd->ses->input->buf[inputline_str_raw_len(0, gtd->ses->input->hid + 0)], gtd->ses->input->off + gtd->ses->input->pos - gtd->ses->input->hid);
		}
	}
}

DO_CURSOR(cursor_right)
{
	int size, width;

	if (gtd->ses->input->cur < gtd->ses->input->len)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC))
		{
			gtd->ses->input->cur += get_euc_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->cur], &width);

			input_printf("\e[%dC", width);
			gtd->ses->input->pos += width;
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			gtd->ses->input->cur += get_utf8_width(&gtd->ses->input->buf[gtd->ses->input->cur], &width);

			if (width == 0)
			{
				return cursor_right(ses, "");
			}
			input_printf("\e[%dC", width);
			gtd->ses->input->pos += width;

			while (gtd->ses->input->cur < gtd->ses->input->len)
			{
				size = get_utf8_width(&gtd->ses->input->buf[gtd->ses->input->cur], &width);

				if (width)
				{
					break;
				}
				gtd->ses->input->cur += size;
			}
		}
		else
		{
			input_printf("\e[1C");
			gtd->ses->input->cur++;
			gtd->ses->input->pos++;
		}
	}

	cursor_check_line(ses, "");
}

DO_CURSOR(cursor_right_word)
{
	if (gtd->ses->input->cur == gtd->ses->input->len)
	{
		return;
	}

	while (gtd->ses->input->cur < gtd->ses->input->len && gtd->ses->input->buf[gtd->ses->input->cur] == ' ')
	{
		gtd->ses->input->cur++;
		gtd->ses->input->pos++;
	}

	while (gtd->ses->input->cur < gtd->ses->input->len && gtd->ses->input->buf[gtd->ses->input->cur] != ' ')
	{
		if (!HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) || (gtd->ses->input->buf[gtd->ses->input->cur] & 192) != 128)
		{
			gtd->ses->input->pos++;
		}
		gtd->ses->input->cur++;
	}

	cursor_redraw_line(ses, "");
}

DO_CURSOR(cursor_set)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		return;
	}

	ins_sprintf(&gtd->ses->input->buf[gtd->ses->input->cur], "%s", arg1);

	gtd->ses->input->len += strlen(arg1);
	gtd->ses->input->cur += strlen(arg1);

	gtd->ses->input->pos += inputline_raw_str_len(gtd->ses->input->cur - strlen(arg1), gtd->ses->input->cur);

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_suspend)
{
	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK))
	{
		do_suspend(ses, arg, NULL, NULL, NULL, NULL);
	}
}

DO_CURSOR(cursor_info)
{
	tintin_printf2(ses, "Width of input bar:             %10d", inputline_max_str_len());
	tintin_printf2(ses, "Offset of input bar:            %10d", gtd->ses->input->off);
	tintin_printf2(ses, "Width of hidden text on left:   %10d", gtd->ses->input->hid);
	tintin_printf2(ses, "VT100 position of cursor:       %10d", gtd->ses->input->pos);
	tintin_printf2(ses, "internal position of cursor:    %10d", gtd->ses->input->cur);
	tintin_printf2(ses, "internal length of input line:  %10d", gtd->ses->input->len);
	tintin_printf2(ses, "VT100 length of displayed line: %10d", inputline_cur_str_len());
	tintin_printf2(ses, "input->top_row:                 %10d", gtd->ses->input->top_row);
	tintin_printf2(ses, "input->top_col:                 %10d", gtd->ses->input->top_col);
	tintin_printf2(ses, "input->bot_row:                 %10d", gtd->ses->input->bot_row);
	tintin_printf2(ses, "input->bot_col:                 %10d", gtd->ses->input->bot_col);
	tintin_printf2(ses, "input->rel_row:                 %10d", gtd->ses->input->rel_row);
	tintin_printf2(ses, "input->rel_col:                 %10d", gtd->ses->input->rel_row);
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

			if (!strncmp(tab, &gtd->ses->input->buf[input_now], strlen(&gtd->ses->input->buf[input_now])))
			{
				if (search_node_list(gtd->ses->list[LIST_COMMAND], tab))
				{
					continue;
				}
				create_node_list(gtd->ses->list[LIST_COMMAND], tab, "", "", "");

				if (node->shots && --node->shots == 0)
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

			if (!strncmp(tab, &gtd->ses->input->buf[input_now], strlen(&gtd->ses->input->buf[input_now])))
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
				create_node_list(gtd->ses->list[LIST_COMMAND], tab, "", "", "");

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

	if (root->used && !strcmp(l_node->arg1, gtd->ses->input->buf + input_now))
	{
		len_change = strlen(l_node->arg1) - strlen(f_node->arg1);

		if (len_change > 0)
		{
/*
			if (gtd->ses->input->cur < gtd->ses->input->len)
			{
				input_printf("\e[%dC", gtd->ses->input->len - gtd->ses->input->cur);
			}
			input_printf("\e[%dD\e[%dP", len_change, len_change);
*/
			gtd->ses->input->len = gtd->ses->input->len - len_change;
			gtd->ses->input->buf[gtd->ses->input->len] = 0;
			gtd->ses->input->cur = gtd->ses->input->len;
			gtd->ses->input->pos = gtd->ses->input->pos;
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
	if (gtd->ses->input->cur < gtd->ses->input->len)
	{
		input_printf("\e[%dC", gtd->ses->input->len - gtd->ses->input->cur);
	}
	if (gtd->ses->input->len > input_now)
	{
		input_printf("\e[%dD\e[%dP", gtd->ses->input->len - input_now, gtd->ses->input->len - input_now);
	}
	if (input_now + (int) strlen(node->arg1) < gtd->ses->cols - 2)
	{
		input_printf("%s", node->arg1);
	}
*/
	strcpy(&gtd->ses->input->buf[input_now], node->arg1);

	gtd->ses->input->len = input_now + strlen(node->arg1);
	gtd->ses->input->cur = gtd->ses->input->len;

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

	if (gtd->ses->input->len == 0 || gtd->ses->input->buf[gtd->ses->input->len - 1] == ' ')
	{
		return -1;
	}

	for (input_now = gtd->ses->input->len - 1 ; input_now ; input_now--)
	{
		if (gtd->ses->input->buf[input_now] == ' ')
		{
			return input_now + 1;
		}
	}

	return input_now;
}

DO_CURSOR(cursor_tab)
{
	char arg1[BUFFER_SIZE];
	int flags;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	while (*arg)
	{
		if (is_abbrev(arg1, "LIST"))
		{
			SET_BIT(flags, TAB_FLAG_LIST);
		}
		else if (is_abbrev(arg1, "SCROLLBACK"))
		{
			SET_BIT(flags, TAB_FLAG_SCROLLBACK);
		}

		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}

	if (is_abbrev(arg1, "FORWARD"))
	{
		if (!HAS_BIT(flags, TAB_FLAG_LIST|TAB_FLAG_SCROLLBACK))
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR TAB {LIST|SCROLLBACK} FORWARD");
		}
		else
		{
			if (HAS_BIT(flags, TAB_FLAG_LIST|TAB_FLAG_SCROLLBACK) == (TAB_FLAG_LIST|TAB_FLAG_SCROLLBACK))
			{
				cursor_mixed_tab_forward(ses, "");
			}
			else if (HAS_BIT(flags, TAB_FLAG_LIST))
			{
				cursor_tab_forward(ses, "");
			}
			else
			{
				cursor_auto_tab_forward(ses, "");
			}
		}
		SET_BIT(flags, TAB_FLAG_FORWARD);
	}
	else if (is_abbrev(arg1, "BACKWARD"))
	{
		if (!HAS_BIT(flags, TAB_FLAG_LIST|TAB_FLAG_SCROLLBACK))
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR TAB {LIST|SCROLLBACK} BACKWARD");
		}
		else
		{
			if (HAS_BIT(flags, TAB_FLAG_LIST|TAB_FLAG_SCROLLBACK) == (TAB_FLAG_LIST|TAB_FLAG_SCROLLBACK))
			{
				cursor_mixed_tab_backward(ses, "");
			}
			else if (HAS_BIT(flags, TAB_FLAG_LIST))
			{
				cursor_tab_backward(ses, "");
			}
			else
			{
				cursor_auto_tab_backward(ses, "");
			}
		}
		SET_BIT(flags, TAB_FLAG_BACKWARD);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR TAB {LIST|SCROLLBACK} <BACKWARD|FORWARD>");
	}
}

DO_CURSOR(cursor_tab_forward)
{
	struct listroot *root = ses->list[LIST_COMMAND];
	int tab_found;

	if (!root->list[0])
	{
		gtd->ses->input->tab = cursor_calc_input_now();
	}

	if (gtd->ses->input->tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->ses->input->tab);

	if (!root->list[0])
	{
		create_node_list(root, &gtd->ses->input->buf[gtd->ses->input->tab], "", "", "");
	}
	tab_found = cursor_tab_add(gtd->ses->input->tab, TRUE);

	cursor_show_completion(gtd->ses->input->tab, tab_found);
}

DO_CURSOR(cursor_tab_backward)
{
	struct listroot *root = ses->list[LIST_COMMAND];

	if (!root->list[0])
	{
		gtd->ses->input->tab = cursor_calc_input_now();
	}

	if (gtd->ses->input->tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->ses->input->tab);

	if (root->used)
	{
		delete_index_list(root, root->used - 1);
	}

	if (!root->list[0])
	{
		create_node_list(root, &gtd->ses->input->buf[gtd->ses->input->tab], "", "", "");

		cursor_tab_add(gtd->ses->input->tab, FALSE);
	}
	cursor_show_completion(gtd->ses->input->tab, TRUE);
}

DO_CURSOR(cursor_auto_tab_forward)
{
	struct listroot *root = ses->list[LIST_COMMAND];
	int tab_found;

	if (!root->list[0])
	{
		gtd->ses->input->tab = cursor_calc_input_now();
	}

	if (gtd->ses->input->tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->ses->input->tab);

	if (!root->list[0])
	{
		create_node_list(root, &gtd->ses->input->buf[gtd->ses->input->tab], "", "", "");
	}

	tab_found = cursor_auto_tab_add(gtd->ses->input->tab, TRUE);

	cursor_show_completion(gtd->ses->input->tab, tab_found);
}

DO_CURSOR(cursor_auto_tab_backward)
{
	struct listroot *root = ses->list[LIST_COMMAND];

	if (!root->list[0])
	{
		gtd->ses->input->tab = cursor_calc_input_now();
	}

	if (gtd->ses->input->tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->ses->input->tab);

	if (root->used)
	{
		delete_index_list(root, root->used - 1);
	}

	if (!root->list[0])
	{
		create_node_list(root, &gtd->ses->input->buf[gtd->ses->input->tab], "", "", "");

		cursor_auto_tab_add(gtd->ses->input->tab, FALSE);
	}

	cursor_show_completion(gtd->ses->input->tab, TRUE);
}


DO_CURSOR(cursor_mixed_tab_forward)
{
	struct listroot *root = ses->list[LIST_COMMAND];
	int tab_found;

	if (!root->list[0])
	{
		gtd->ses->input->tab = cursor_calc_input_now();
	}

	if (gtd->ses->input->tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->ses->input->tab);

	if (!root->list[0])
	{
		create_node_list(root, &gtd->ses->input->buf[gtd->ses->input->tab], "", "", "");
	}
	tab_found = cursor_tab_add(gtd->ses->input->tab, TRUE) || cursor_auto_tab_add(gtd->ses->input->tab, TRUE);

	cursor_show_completion(gtd->ses->input->tab, tab_found);
}

DO_CURSOR(cursor_mixed_tab_backward)
{
	struct listroot *root = ses->list[LIST_COMMAND];

	if (!root->list[0])
	{
		gtd->ses->input->tab = cursor_calc_input_now();
	}

	if (gtd->ses->input->tab == -1)
	{
		return;
	}

	cursor_hide_completion(gtd->ses->input->tab);

	if (root->used)
	{
		delete_index_list(root, root->used - 1);
	}

	if (!root->list[0])
	{
		create_node_list(root, &gtd->ses->input->buf[gtd->ses->input->tab], "", "", "");

		cursor_tab_add(gtd->ses->input->tab, FALSE);
		cursor_auto_tab_add(gtd->ses->input->tab, FALSE);
	}

	cursor_show_completion(gtd->ses->input->tab, TRUE);
}

DO_CURSOR(cursor_screen_focus_in)
{
	gtd->screen->focus = 1;

	check_all_events(gtd->ses, SUB_ARG, 0, 1, "SCREEN FOCUS", ntos(gtd->screen->focus));

	msdp_update_all("SCREEN_FOCUS", "%d", gtd->screen->focus);
}

DO_CURSOR(cursor_screen_focus_out)
{
	gtd->screen->focus = 0;

	check_all_events(gtd->ses, SUB_ARG, 0, 1, "SCREEN FOCUS", ntos(gtd->screen->focus));

	msdp_update_all("SCREEN_FOCUS", "%d", gtd->screen->focus);
}
