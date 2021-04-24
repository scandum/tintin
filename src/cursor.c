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
	int cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " CURSOR OPTIONS ");

		for (cnt = 0 ; cursor_table[cnt].fun ; cnt++)
		{
			if (*cursor_table[cnt].desc)
			{
				convert_meta(cursor_table[cnt].code, arg2, FALSE);

				tintin_printf2(ses, "  [%-18s] [%-8s] %s", cursor_table[cnt].name, arg2, cursor_table[cnt].desc);
			}
		}
		tintin_header(ses, 80, "");
	}
	else
	{
		for (cnt = 0 ; *cursor_table[cnt].name ; cnt++)
		{
			if (is_abbrev(arg1, cursor_table[cnt].name))
			{
				cursor_table[cnt].fun(ses, arg);

				return ses;
			}
		}
		show_error(ses, LIST_COMMAND, "#ERROR: #CURSOR {%s} IS NOT A VALID OPTION.", arg1);
	}
	return ses;
}

// strlen with offset.

int inputline_str_str_len(int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt, width;

	raw_cnt = str_cnt = ret_cnt = 0;

	while (raw_cnt < gtd->ses->input->raw_len)
	{
		if (str_cnt >= end)
		{
			break;
		}

		raw_cnt += get_vt102_width(gtd->ses, &gtd->ses->input->buf[raw_cnt], &width);

		if (str_cnt >= start)
		{
			ret_cnt += width;
		}
		str_cnt += width;
	}
	return ret_cnt;
}

// raw range

int inputline_raw_str_len(int raw_start, int raw_end)
{
	int raw_cnt, str_len, size, width;

	raw_cnt = raw_start;
	str_len = 0;

	while (raw_cnt < gtd->ses->input->raw_len)
	{
		if (raw_end >= 0 && raw_cnt >= raw_end)
		{
			break;
		}

		size = get_vt102_width(gtd->ses, &gtd->ses->input->buf[raw_cnt], &width);

		raw_cnt += size;
		str_len += width;
	}
	return str_len;
}

// display range

int inputline_str_raw_len(int str_start, int str_end)
{
	int raw_cnt, str_cnt, ret_cnt, size, width;

	raw_cnt = str_cnt = ret_cnt = 0;

	while (raw_cnt < gtd->ses->input->raw_len)
	{
		if (str_end >= 0 && str_cnt >= str_end)
		{
			break;
		}

		size = get_vt102_width(gtd->ses, &gtd->ses->input->buf[raw_cnt], &width);

		if (str_cnt >= str_start)
		{
			ret_cnt += size;
		}
		raw_cnt += size;
		str_cnt += width;
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

int inputline_cur_off(void)
{
	int off;

	off = gtd->ses->input->str_off;

	off += gtd->ses->input->top_col - 1;

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		off += 10;
	}

	return off;
}

int inputline_rows(struct session *ses)
{
	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		return 1;
	}
	return 1 + ses->input->bot_row - ses->input->top_row;
}

// Get string length of the input area

int inputline_max_str_len(void)
{
	int result = 1 + gtd->ses->input->bot_col - gtd->ses->input->top_col;

	result -= gtd->ses->input->str_off - 1;

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		result -= 20;
	}

	return UMAX(1, result);
}

int inputline_cur_str_len(void)
{
	return inputline_str_str_len(gtd->ses->input->str_hid, gtd->ses->input->str_hid + inputline_max_str_len());
}

int inputline_editor(void)
{
	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		return FALSE;
	}

	if (!HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_EDIT))
	{
		return FALSE;
	}
	return TRUE;
}

int inputline_multiline(void)
{
	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		return FALSE;
	}

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_EDIT))
	{
		return FALSE;
	}

	if (gtd->ses->input->top_row == gtd->ses->input->bot_row)
	{
		return FALSE;
	}

	return TRUE;
}

int inputline_tab_valid(void)
{
	if (gtd->ses->input->raw_len == 0)
	{
		return FALSE;
	}

	if (gtd->ses->input->buf[gtd->ses->input->raw_len - 1] == ' ')
	{
		return FALSE;
	}
	return TRUE;
}

void inputline_set_row(int row)
{
	struct edit_data *edit = gtd->ses->input->edit;

	if (row == gtd->ses->input->cur_row)
	{
		return;
	}

	if (inputline_editor())
	{
		str_cpy(&edit->line[edit->update]->str, gtd->ses->input->buf);

		edit->update = URANGE(0, edit->update + (row - gtd->ses->input->cur_row), edit->used - 1);

		inputline_set(edit->line[edit->update]->str, gtd->ses->input->str_pos);
	}
	gtd->ses->input->cur_row = row;
}

void inputline_position(int col)
{
	int size, width;

	if (col == -1)
	{
		gtd->ses->input->raw_pos = gtd->ses->input->raw_len;
		gtd->ses->input->str_pos = gtd->ses->input->str_len;

		return;
	}

	gtd->ses->input->raw_pos = 0;
	gtd->ses->input->str_pos = 0;

	while (gtd->ses->input->raw_pos < gtd->ses->input->raw_len)
	{
		size = get_vt102_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

		if (gtd->ses->input->str_pos + width > col)
		{
			break;
		}
		gtd->ses->input->raw_pos += size;
		gtd->ses->input->str_pos += width;
	}

	if (inputline_editor() && gtd->ses->input->str_len < col)
	{
		gtd->ses->input->str_pos = col;
	}
}

void inputline_insert(char *arg, int str_pos)
{
	int raw_len, str_len;

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK))
	{
		if (gtd->ses->input->raw_len > INPUT_SIZE)
		{
			tintin_printf2(gtd->ses, "#CONFIG CHILD LOCK: YOU ARE CONFINED TO %d BYTES OF INPUT.", INPUT_SIZE);

			str_cpy(&gtd->ses->input->cut, "");

			inputline_set("", -1);

			return;
		}
	}

	raw_len = strip_vt102_width(gtd->ses, arg, &str_len);

	if (str_len && HAS_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT) && gtd->ses->input->raw_len != gtd->ses->input->raw_pos)
	{
		int cnt, loop;

		cnt = UMAX(gtd->ses->input->str_pos, gtd->ses->input->str_len - str_len);

		while (gtd->ses->input->str_len > cnt)
		{
			loop = gtd->ses->input->raw_len;

			cursor_delete(gtd->ses, NULL);

			if (loop == gtd->ses->input->raw_len)
			{
				tintin_printf2(gtd->ses, "inputline_insert: infinite loop detected.");
				break;
			}
		}
	}
	str_ins(&gtd->ses->input->buf, gtd->ses->input->raw_pos, arg);

	gtd->ses->input->raw_len += raw_len;
	gtd->ses->input->str_len += str_len;

	if (str_pos == -1)
	{
		gtd->ses->input->raw_pos += raw_len;
		gtd->ses->input->str_pos += str_len;
	}
}

void inputline_set(char *arg, int str_pos)
{
	int raw_len, str_len;

	str_cpy(&gtd->ses->input->buf, arg);

	raw_len = strip_vt102_width(gtd->ses, arg, &str_len);

	gtd->ses->input->raw_len = raw_len;
	gtd->ses->input->str_len = str_len;

	gtd->ses->input->raw_pos = raw_len;
	gtd->ses->input->str_pos = str_len;

	inputline_position(str_pos);
}

void inputline_cap(char *arg)
{
	int raw_len, str_len;

	gtd->ses->input->raw_len = gtd->ses->input->raw_pos;
	gtd->ses->input->str_len = gtd->ses->input->str_pos;

	str_cap(&gtd->ses->input->buf, gtd->ses->input->raw_pos, arg);

	raw_len = strip_vt102_width(gtd->ses, arg, &str_len);

	gtd->ses->input->raw_len += raw_len;
	gtd->ses->input->str_len += str_len;

	gtd->ses->input->raw_pos += raw_len;
	gtd->ses->input->str_pos += str_len;
}


// Get the position of the cursor

int inputline_cur_row(void)
{
	return gtd->ses->input->cur_row;
}

int inputline_cur_col(void)
{
	if (inputline_editor())
	{
		return inputline_cur_off() + gtd->ses->input->str_pos - gtd->ses->input->str_hid;
	}

	if (gtd->ses->input->top_row == gtd->ses->input->bot_row)
	{
		return inputline_cur_off() + gtd->ses->input->str_pos - gtd->ses->input->str_hid;
	}
	else
	{
		return inputline_cur_off() + gtd->ses->input->str_pos % inputline_max_str_len();
	}
}

void inputline_erase(void)
{
	input_printf("\e[%dG\e[%dX", inputline_cur_off(), inputline_max_str_len());
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
	if (gtd->ses->input->raw_pos == 0)
	{
		if (inputline_editor())
		{
			if (gtd->ses->input->edit->update > 0)
			{
				cursor_move_up(ses, NULL);
				cursor_end(ses, NULL);
				cursor_delete(ses, NULL);

				cursor_redraw_edit(ses, arg);
			}
		}
		return;
	}

	cursor_move_left(ses, NULL);
	cursor_delete(ses, arg);

	modified_input();
}

DO_CURSOR(cursor_brace)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (is_abbrev(arg1, "OPEN"))
	{
		inputline_insert("{", -1);
	}
	else if (is_abbrev(arg1, "CLOSE"))
	{
		inputline_insert("}", -1);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR {BRACE} {OPEN|CLOSE}");
	}
	cursor_redraw_line(ses, arg);

	modified_input();
}

DO_CURSOR(cursor_buffer_down)
{
	if (inputline_editor())
	{
		int update = gtd->ses->input->edit->update;

		if (update + 1 >= gtd->ses->input->edit->used)
		{
			return;
		}

		str_cpy(&gtd->ses->input->edit->line[update]->str, gtd->ses->input->buf);

		update = UMIN(gtd->ses->input->edit->used - 1, update + UMAX(1, inputline_rows(gtd->ses) / 2));

		inputline_set(gtd->ses->input->edit->line[update]->str, gtd->ses->input->str_pos);

		gtd->ses->input->edit->update = update;

		cursor_redraw_edit(ses, arg);

		return;
	}

	command(ses, do_buffer, "down %s", arg);
}

DO_CURSOR(cursor_buffer_end)
{
	int update;

	if (inputline_editor())
	{
		update = gtd->ses->input->edit->update;

		if (update + 1 >= gtd->ses->input->edit->used && gtd->ses->input->cur_row == gtd->ses->input->bot_row)
		{
			return;
		}

		str_cpy(&gtd->ses->input->edit->line[update]->str, gtd->ses->input->buf);

		update = gtd->ses->input->edit->used - 1;

		inputline_set(gtd->ses->input->edit->line[update]->str, gtd->ses->input->str_pos);

		gtd->ses->input->edit->update = update;

		gtd->ses->input->cur_row = gtd->ses->input->bot_row;

		cursor_redraw_edit(ses, arg);

		return;
	}
	buffer_end(ses, "", "", "");
}

DO_CURSOR(cursor_buffer_home)
{
	if (inputline_editor())
	{
		int update = gtd->ses->input->edit->update;

		if (update <= 0 && gtd->ses->input->cur_row == gtd->ses->input->top_row)
		{
			return;
		}

		str_cpy(&gtd->ses->input->edit->line[update]->str, gtd->ses->input->buf);

		inputline_set(gtd->ses->input->edit->line[0]->str, gtd->ses->input->str_pos);

		gtd->ses->input->edit->update = 0;

		gtd->ses->input->cur_row = gtd->ses->input->top_row;

		cursor_redraw_edit(ses, arg);

		return;
	}

	buffer_home(ses, "", "", "");
}

DO_CURSOR(cursor_buffer_lock)
{
	command(ses, do_buffer, "lock %s", arg);
}

DO_CURSOR(cursor_buffer_up)
{
	if (inputline_editor())
	{
		int update = gtd->ses->input->edit->update;

		if (update <= 0)
		{
			return;
		}

		str_cpy(&gtd->ses->input->edit->line[update]->str, gtd->ses->input->buf);

		update = UMAX(0, update - UMAX(1, inputline_rows(gtd->ses) / 2));

		inputline_set(gtd->ses->input->edit->line[update]->str, gtd->ses->input->str_pos);

		gtd->ses->input->edit->update = update;

		cursor_redraw_edit(ses, arg);

		return;
	}

	command(ses, do_buffer, "up %s", arg);
}

DO_CURSOR(cursor_page)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (is_abbrev(arg1, "DOWN"))
	{
		return cursor_buffer_down(ses, arg);
	}
	if (is_abbrev(arg1, "END"))
	{
		return cursor_buffer_end(ses, arg);
	}
	if (is_abbrev(arg1, "LOCK"))
	{
		return cursor_buffer_lock(ses, arg);
	}
	if (is_abbrev(arg1, "HOME"))
	{
		return cursor_buffer_home(ses, arg);
	}
	if (is_abbrev(arg1, "UP"))
	{
		return cursor_buffer_up(ses, arg);
	}
	show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR PAGE {DOWN|END|LOCK|HOME|UP}");
}

DO_CURSOR(cursor_check_line)
{
	if (arg == NULL)
	{
		return;
	}

	int str_max = inputline_max_str_len();

	if (inputline_multiline())
	{
		if (gtd->ses->input->cur_row != gtd->ses->input->top_row + (gtd->ses->input->str_pos - gtd->ses->input->str_hid) / str_max)
		{
			return cursor_redraw_line(ses, "");
		}

		if (gtd->ses->input->str_hid && gtd->ses->input->str_pos - gtd->ses->input->str_hid < 1)
		{
			return cursor_redraw_line(ses, "");
		}
	}
	else
	{
		if (gtd->ses->input->str_pos - gtd->ses->input->str_hid > str_max - 3)
		{
			return cursor_redraw_line(ses, "");
		}

		if (gtd->ses->input->str_hid && gtd->ses->input->str_pos - gtd->ses->input->str_hid < 3)
		{
			return cursor_redraw_line(ses, "");
		}
	}
	gtd->ses->cur_col = inputline_cur_off() + gtd->ses->input->str_pos - gtd->ses->input->str_hid;
}

DO_CURSOR(cursor_check_line_modified)
{
	int max, width;

	if (gtd->ses->input->raw_len != str_len(gtd->ses->input->buf))
	{
		tintin_printf2(ses, "\e[1;31merror: cursor_check_line_modified1: raw: %d vs %d", gtd->ses->input->raw_len, str_len(gtd->ses->input->buf));
	}

	strip_vt102_width(gtd->ses, gtd->ses->input->buf, &width);

	if (gtd->ses->input->str_len != width)
	{
		tintin_printf2(ses, "\e[1;31merror: cursor_check_line_modified2: str: %d vs %d", gtd->ses->input->str_len, width);
	}

	if (gtd->ses->input->str_pos > gtd->ses->input->str_len)
	{
		return cursor_end(ses, "");
	}

	max = inputline_max_str_len();

	if (inputline_multiline())
	{
		if (gtd->ses->input->str_len / max != gtd->ses->input->str_pos / max)
		{
			return cursor_redraw_line(ses, "");
		}

		if (gtd->ses->input->str_pos % max == 0)
		{
			return cursor_redraw_line(ses, "");
		}
	}
	return cursor_check_line(ses, "");
}

DO_CURSOR(cursor_clear_left)
{
	if (gtd->ses->input->raw_pos == 0)
	{
		if (inputline_editor())
		{
			cursor_backspace(ses, arg);
		}
		return;
	}

	str_cpy_printf(&gtd->ses->input->cut, "%.*s", gtd->ses->input->raw_pos, gtd->ses->input->buf);

	str_mov(&gtd->ses->input->buf, 0, gtd->ses->input->raw_pos);

	gtd->ses->input->raw_len -= gtd->ses->input->raw_pos;
	gtd->ses->input->str_len -= gtd->ses->input->str_pos;

	gtd->ses->input->raw_pos  = 0;
	gtd->ses->input->str_pos  = 0;

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_remove_line)
{
	str_cpy(&gtd->ses->input->cut, gtd->ses->input->buf);

	inputline_set("", -1);

	cursor_delete(ses, arg);
}

DO_CURSOR(cursor_clear_line)
{
	if (inputline_editor())
	{
		return cursor_remove_line(ses, arg);
	}

	if (gtd->ses->input->raw_len == 0)
	{
		return;
	}

	str_cpy(&gtd->ses->input->cut, gtd->ses->input->buf);

	inputline_set("", -1);

	gtd->ses->input->str_hid = 0;

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_clear_right)
{
	if (gtd->ses->input->raw_pos == gtd->ses->input->raw_len)
	{
		if (inputline_editor())
		{
			cursor_delete(ses, arg);
		}
		return;
	}

	str_cpy(&gtd->ses->input->cut, &gtd->ses->input->buf[gtd->ses->input->raw_pos]);

	str_cap(&gtd->ses->input->buf, gtd->ses->input->raw_pos, "");

	gtd->ses->input->raw_len = gtd->ses->input->raw_pos;
	gtd->ses->input->str_len = gtd->ses->input->str_pos;

//	input_printf("\e[%dX", inputline_max_str_len() - inputline_str_str_len(gtd->ses->input->str_hid, gtd->ses->input->str_pos));

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_convert_meta)
{
	SET_BIT(gtd->ses->input->flags, INPUT_FLAG_CONVERTMETACHAR);
}

DO_CURSOR(cursor_delete_or_exit)
{
	if (gtd->ses->input->raw_len == 0)
	{
		if (ses == gts)
		{
			command(ses, do_end, "");
		}
		else
		{
			command(ses, do_zap, "");
		}
	}
	else
	{
		cursor_delete(ses, arg);
	}
}

DO_CURSOR(cursor_delete)
{
	int size, width;

	if (gtd->ses->input->raw_len == 0)
	{
		if (inputline_editor())
		{
			if (gtd->ses->input->edit->update + 1 < gtd->ses->input->edit->used)
			{
				remove_line(gtd->ses->input->edit, gtd->ses->input->edit->update);

				inputline_set(gtd->ses->input->edit->line[gtd->ses->input->edit->update]->str, 0);
				
				cursor_redraw_edit(ses, arg);
				
				modified_input();
			}
		}
		return;
	}

	if (gtd->ses->input->raw_len == gtd->ses->input->raw_pos)
	{
		if (inputline_editor())
		{
			if (gtd->ses->input->edit->update + 1 < gtd->ses->input->edit->used)
			{
				inputline_insert(gtd->ses->input->edit->line[gtd->ses->input->edit->update + 1]->str, 0);

				remove_line(gtd->ses->input->edit, gtd->ses->input->edit->update + 1);

				cursor_redraw_edit(ses, arg);

				modified_input();
			}
		}
		return;
	}

	size = get_vt102_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

	gtd->ses->input->raw_len -= size;
	gtd->ses->input->str_len -= width;

	str_mov(&gtd->ses->input->buf, gtd->ses->input->raw_pos, gtd->ses->input->raw_pos + size);

	if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
	{
		while (gtd->ses->input->raw_len > gtd->ses->input->raw_pos)
		{
			if (!is_utf8_head(&gtd->ses->input->buf[gtd->ses->input->raw_pos]))
			{
				break;
			}
			size = get_utf8_width(&gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

			if (width)
			{
				break;
			}
			gtd->ses->input->raw_len -= size;

			str_mov(&gtd->ses->input->buf, gtd->ses->input->raw_pos, gtd->ses->input->raw_pos + size);
		}
	}

	if (gtd->ses->input->raw_len == gtd->ses->input->raw_pos)
	{
		input_printf("\e[1X");
		cursor_check_line(ses, arg);
	}
	else
	{
		cursor_redraw_line(ses, "");
	}
	modified_input();
}

DO_CURSOR(cursor_delete_word_left)
{
	int index_raw, index_str, span_raw, width;

	if (gtd->ses->input->raw_pos == 0)
	{
		if (inputline_editor())
		{
			return cursor_backspace(ses, arg);
		}
		return;
	}

	index_raw = gtd->ses->input->raw_pos;
	index_str = gtd->ses->input->str_pos;

	while (gtd->ses->input->raw_pos > 0 && gtd->ses->input->buf[gtd->ses->input->raw_pos - 1] == ' ')
	{
		gtd->ses->input->raw_pos--;
	}

	while (gtd->ses->input->raw_pos > 0 && gtd->ses->input->buf[gtd->ses->input->raw_pos - 1] != ' ')
	{
		gtd->ses->input->raw_pos--;
	}

	span_raw = gtd->ses->input->raw_pos;

	while (span_raw < index_raw)
	{
		span_raw += get_vt102_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

		gtd->ses->input->str_pos -= width;
	}

	str_cpy_printf(&gtd->ses->input->cut, "%.*s", index_raw - gtd->ses->input->raw_pos, &gtd->ses->input->buf[gtd->ses->input->raw_pos]);

	str_mov(&gtd->ses->input->buf, gtd->ses->input->raw_pos, index_raw);

	gtd->ses->input->raw_len -= index_raw - gtd->ses->input->raw_pos;
	gtd->ses->input->str_len -= index_str - gtd->ses->input->str_pos;

	cursor_redraw_line(ses, "");

	modified_input();
}


DO_CURSOR(cursor_delete_word_right)
{
	int index_raw, index_str, size, width;

	if (gtd->ses->input->raw_pos == gtd->ses->input->raw_len)
	{
		if (inputline_editor())
		{
			return cursor_delete(ses, arg);
		}
		return;
	}

	index_raw = gtd->ses->input->raw_pos;
	index_str = gtd->ses->input->str_pos;

	while (gtd->ses->input->raw_pos < gtd->ses->input->raw_len && gtd->ses->input->buf[gtd->ses->input->raw_pos] == ' ')
	{
		gtd->ses->input->raw_pos++;
		gtd->ses->input->str_pos++;
	}

	while (gtd->ses->input->raw_pos < gtd->ses->input->raw_len && gtd->ses->input->buf[gtd->ses->input->raw_pos] != ' ')
	{
		size = get_vt102_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

		gtd->ses->input->raw_pos += size;
		gtd->ses->input->str_pos += width;
	}

	str_cpy_printf(&gtd->ses->input->cut, "%.*s", gtd->ses->input->raw_pos - index_raw, &gtd->ses->input->buf[gtd->ses->input->raw_pos]);

	str_mov(&gtd->ses->input->buf, index_raw, gtd->ses->input->raw_pos);

	gtd->ses->input->raw_len -= gtd->ses->input->raw_pos - index_raw;
	gtd->ses->input->str_len -= gtd->ses->input->str_pos - index_str;

	gtd->ses->input->raw_pos = index_raw;
	gtd->ses->input->str_pos = index_str;

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
	gtd->ses->input->raw_pos = gtd->ses->input->raw_len;
	gtd->ses->input->str_pos = inputline_raw_str_len(0, -1);

	cursor_redraw_line(ses, arg);
}

DO_CURSOR(cursor_enter)
{
	int filesize;

	push_call("cursor_enter(%p,%p)",ses,arg);

	gtd->ses->input->str_hid = 0;

	gtd->ses->input->raw_pos = 0;
	gtd->ses->input->str_pos = 0;

	if (inputline_editor())
	{
		char *str1 = str_alloc_stack(0);

		DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_EDIT);

		str_cpy(&gtd->ses->input->edit->line[gtd->ses->input->edit->update]->str, gtd->ses->input->buf);

		filesize = str_save_editor(gtd->ses->input->edit, &str1);

		check_all_events(gtd->ses, EVENT_FLAG_INPUT, 0, 4, "EDIT FINISHED", gtd->ses->input->edit_name, ntos(gtd->ses->input->edit->used), ntos(filesize), str1);

		if (*gtd->ses->input->edit_name)
		{
			check_all_events(gtd->ses, EVENT_FLAG_INPUT, 1, 4, "EDIT FINISHED %s", gtd->ses->input->edit_name, ntos(gtd->ses->input->edit->used), ntos(filesize), gtd->ses->input->edit_name, str1);

			if (check_all_events(gtd->ses, EVENT_FLAG_CATCH, 1, 4, "CATCH EDIT FINISHED %s", gtd->ses->input->edit_name, gtd->ses->input->edit_name, ntos(gtd->ses->input->edit->used), ntos(filesize), str1))
			{
				cursor_enter_finish(ses, "");

				pop_call();
				return;
			}
		}

		if (check_all_events(gtd->ses, EVENT_FLAG_CATCH, 0, 4, "CATCH EDIT FINISHED", gtd->ses->input->edit_name, ntos(gtd->ses->input->edit->used), ntos(filesize), str1))
		{
			cursor_enter_finish(ses, "");

			pop_call();
			return;
		}
	}

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		struct listroot *root = ses->list[LIST_HISTORY];

		if (root->update >= 0 && root->update < root->used)
		{
			inputline_set(root->list[root->update]->arg1, -1);
		}

		DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH);

		gtd->ses->input->str_hid = 0;

		gtd->ses->input->raw_pos = 0;
		gtd->ses->input->str_pos = 0;
	}
	else if (*gtd->ses->input->buf == 0)
	{
		struct listroot *root = ses->list[LIST_HISTORY];

		if (root->used && HAS_BIT(ses->config_flags, CONFIG_FLAG_REPEATENTER))
		{
			inputline_set(root->list[root->used - 1]->arg1, -1);
		}
		gtd->ses->input->str_hid = 0;

		gtd->ses->input->raw_pos = 0;
		gtd->ses->input->str_pos = 0;
	}

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_SPLIT))
	{
		inputline_erase();
	}
	else
	{
		input_printf("\n");
	}

	SET_BIT(gtd->flags, TINTIN_FLAG_PROCESSINPUT);

	modified_input();

	pop_call();
	return;
}

DO_CURSOR(cursor_soft_enter)
{
	if (!inputline_editor())
	{
//		show_error(ses, LIST_COMMAND, "#ERROR: #CURSOR SOFT ENTER: YOU ARE NOT CURRENTLY EDITING.");

		return cursor_enter(ses, arg);
	}

	insert_line(gtd->ses->input->edit, gtd->ses->input->edit->update + 1, &gtd->ses->input->buf[gtd->ses->input->raw_pos]);

	str_cap(&gtd->ses->input->buf, gtd->ses->input->raw_pos, "");

	if (gtd->ses->input->raw_pos != gtd->ses->input->raw_len)
	{
		cursor_clear_right(ses, NULL);
	}

	gtd->ses->input->str_hid = 0;

	gtd->ses->input->raw_pos = 0;
	gtd->ses->input->str_pos = 0;

	cursor_move_down(ses, "");
}

DO_CURSOR(cursor_enter_finish)
{
	if (inputline_editor())
	{
		cursor_redraw_line(gtd->ses, "");
		
		return;
	}

	str_cpy(&ses->input->tmp, "");
	str_cpy(&ses->input->buf, "");

	ses->input->raw_len = 0;
	ses->input->str_len = 0;
	ses->input->raw_pos = 0;
	ses->input->str_pos = 0;

	ses->input->str_hid = 0;

	DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_EDIT);

	if (ses == gtd->ses && HAS_BIT(gtd->ses->flags, SES_FLAG_SPLIT))
	{
		cursor_redraw_line(gtd->ses, "");
	}
}

DO_CURSOR(cursor_flag)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (is_abbrev(arg1, "ECHO"))
	{
		if (*arg2 == 0)
		{
			TOG_BIT(ses->telopts, TELOPT_FLAG_ECHO);
		}
		else if (!strcasecmp(arg2, "ON"))
		{
			SET_BIT(ses->telopts, TELOPT_FLAG_ECHO);
		}
		else if (!strcasecmp(arg2, "OFF"))
		{
			DEL_BIT(ses->telopts, TELOPT_FLAG_ECHO);
		}
		else
		{
			show_error(gtd->ses, LIST_COMMAND, "#SYNTAX: #CURSOR {FLAG} {ECHO} {ON|OFF}.");
		}
		return;
	}

	if (is_abbrev(arg1, "INSERT"))
	{
		if (*arg2 == 0)
		{
			TOG_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
		}
		else if (!strcasecmp(arg2, "ON"))
		{
			SET_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
		}
		else if (!strcasecmp(arg2, "OFF"))
		{
			DEL_BIT(gtd->flags, TINTIN_FLAG_INSERTINPUT);
		}
		else
		{
			show_error(gtd->ses, LIST_COMMAND, "#SYNTAX: #CURSOR {FLAG} {INSERT} {ON|OFF}.");
		}
		return;
	}

	show_error(gtd->ses, LIST_COMMAND, "#SYNTAX: #CURSOR {FLAG} {ECHO|INSERT} {ON|OFF}.");
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

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
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
			input_printf("\e[%dG\e[%dX ]  %.*s",
				inputline_cur_off() + inputline_cur_str_len(),
				inputline_max_str_len() - inputline_cur_off() - inputline_cur_str_len(),
				UMAX(0, inputline_max_str_len() - inputline_cur_off() - inputline_cur_str_len() - 4),
				root->list[root->update]->arg1);
			
			goto_pos(gtd->ses, inputline_cur_row(), inputline_cur_col());
		}
		return;
	}

	if (!HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYBROWSE))
	{
		return;
	}

	if (root->update < root->used)
	{
		for (root->update++ ; root->update < root->used ; root->update++)
		{
			if (!strncmp(gtd->ses->input->tmp, root->list[root->update]->arg1, str_len(gtd->ses->input->tmp)))
			{
				break;
			}
		}
	}

	if (root->update >= root->used)
	{
		DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYBROWSE);

		if (!strcmp(gtd->ses->input->buf, gtd->ses->input->tmp))
		{
			return;
		}
		inputline_set(gtd->ses->input->tmp, -1);
	}
	else
	{
		inputline_set(root->list[root->update]->arg1, -1);
		SET_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYBROWSE);
	}
	cursor_end(ses, "");
}

DO_CURSOR(cursor_history_prev)
{
	struct listroot *root = ses->list[LIST_HISTORY];

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		if (root->update <= 0)
		{
			return;
		}

		if (root->update > root->used)
		{
			tintin_printf2(ses, "debug: cursor_history_prev %d > %d", root->update, root->used);
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
			input_printf("\e[%dG\e[%dX ]  %.*s",
				inputline_cur_off() + inputline_cur_str_len(),
				inputline_max_str_len() - inputline_cur_off() - inputline_cur_str_len(),
				UMAX(0, inputline_max_str_len() - inputline_cur_off() - inputline_cur_str_len() - 4),
				root->list[root->update]->arg1);
			
			goto_pos(gtd->ses, inputline_cur_row(), inputline_cur_col());
		}
		return;
	}

	if (root->update >= root->used || !HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYBROWSE))
	{
		SET_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYBROWSE);

		str_cpy(&gtd->ses->input->tmp, gtd->ses->input->buf);

		root->update = root->used - 1;
	}
	else if (root->update >= 0)
	{
		root->update--;
	}

	while (root->update >= 0)
	{
		if (!strncmp(gtd->ses->input->tmp, root->list[root->update]->arg1, str_len(gtd->ses->input->tmp)))
		{
			break;
		}
		root->update--;
	}

	if (root->update < 0)
	{
		if (!strcmp(gtd->ses->input->buf, gtd->ses->input->tmp))
		{
			return;
		}
		str_cpy(&gtd->ses->input->buf, gtd->ses->input->tmp);
	}
	else
	{
		str_cpy(&gtd->ses->input->buf, root->list[root->update]->arg1);
	}

	gtd->ses->input->raw_len = strip_vt102_width(ses, gtd->ses->input->buf, &gtd->ses->input->str_len);

	cursor_end(ses, "");
}

DO_CURSOR(cursor_history_search)
{
	struct listroot *root = ses->list[LIST_HISTORY];

	if (!HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		str_cpy(&gtd->ses->input->tmp, gtd->ses->input->buf);

		inputline_set("", -1);

		inputline_erase();

		SET_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH);

		root->update = -1;

		input_printf("(search) [ ] \e[3D");
	}
	else
	{
		if (root->update >= 0 && root->update < root->used)
		{
			inputline_set(root->list[root->update]->arg1, -1);
		}
		else
		{
			inputline_set(gtd->ses->input->tmp, -1);
		}

		DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH);

		cursor_redraw_line(ses, "");
	}
}

DO_CURSOR(cursor_history_find)
{
	struct listroot *root = ses->list[LIST_HISTORY];

	push_call("cursor_history_find(%s)", gtd->ses->input->buf);

	if (inputline_str_chk(0, gtd->ses->input->raw_len) == FALSE)
	{
		pop_call();
		return;
	}

	if (*gtd->ses->input->buf == 0)
	{
		root->update = -1;
	}
	else
	{
		gtd->level->quiet++;

		for (root->update = root->used - 1 ; root->update >= 0 ; root->update--)
		{
			if (find(ses, root->list[root->update]->arg1, gtd->ses->input->buf, SUB_NONE, REGEX_FLAG_NONE))
			{
				break;
			}
		}
		gtd->level->quiet--;
	}

	if (root->update >= 0)
	{
		if (0 && gtd->ses->input->str_hid)
		{
			input_printf("\e[%dG\e[%dX ] %.*s",
				inputline_cur_off() + inputline_max_str_len(),
				7,
				7,
				root->list[root->update]->arg1);
		}
		else
		{
			input_printf("\e[%dG\e[%dX ] %.*s",
				inputline_cur_off() + inputline_cur_str_len(),
				inputline_max_str_len() - inputline_cur_off() - inputline_cur_str_len() + 20,
				UMAX(7, inputline_max_str_len() - inputline_cur_off() - inputline_cur_str_len() + 7),
				root->list[root->update]->arg1);
		}
	}
	else
	{
		input_printf("\e[%dG\e[%dX ]",
			inputline_cur_off() + inputline_cur_str_len(),
			inputline_max_str_len() - inputline_cur_off() - inputline_cur_str_len() + 20);
	}
	goto_pos(gtd->ses, inputline_cur_row(), inputline_cur_col());

	pop_call();
	return;
}

DO_CURSOR(cursor_home)
{
	if (gtd->ses->input->raw_pos == 0)
	{
		if (gtd->ses->input->str_pos)
		{
			gtd->ses->input->str_pos = 0;
			
			return cursor_redraw_line(ses, arg);
		}
		return;
	}

	input_printf("\e[%dD", gtd->ses->input->str_pos - gtd->ses->input->str_hid);

	gtd->ses->input->raw_pos = 0;
	gtd->ses->input->str_pos = 0;

	cursor_check_line(ses, arg);
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

DO_CURSOR(cursor_move_page_up)
{
	int rows = UMAX(1, inputline_rows(gtd->ses) / 2);

	DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH);
	
	cursor_move_up(ses, ntos(rows));
}

DO_CURSOR(cursor_move_up)
{
	int str_max, moves, update;

	if (inputline_editor())
	{
		update = gtd->ses->input->edit->update;

		if (update <= 0)
		{
			return;
		}

		moves = arg ? URANGE(1, atoi(arg), update) : 1;

		str_cpy(&gtd->ses->input->edit->line[update]->str, gtd->ses->input->buf);

		inputline_set(gtd->ses->input->edit->line[update - moves]->str, gtd->ses->input->str_pos);

		gtd->ses->input->edit->update -= moves;

		gtd->ses->input->cur_row = UMAX(gtd->ses->input->top_row, gtd->ses->input->cur_row - moves);

		cursor_redraw_edit(ses, arg);

		return;
	}

	if (!inputline_multiline())
	{
		return cursor_history_prev(ses, "");
	}

	if (gtd->ses->input->str_pos == 0)
	{
		return cursor_history_prev(ses, "");
	}

	moves = arg ? UMAX(1, atoi(arg)) : 1;

	str_max = inputline_max_str_len();

	while (moves--)
	{
		gtd->ses->input->str_pos = UMAX(0, gtd->ses->input->str_pos - str_max);
	}

	gtd->ses->input->raw_pos = inputline_str_raw_len(0, gtd->ses->input->str_pos);

	cursor_redraw_line(ses, arg);
}

DO_CURSOR(cursor_move_page_down)
{
	int rows = UMAX(1, inputline_rows(gtd->ses) / 2);

	DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH);
	
	cursor_move_down(ses, ntos(rows));
}

DO_CURSOR(cursor_move_down)
{
	int str_max, moves, update;

	if (inputline_editor())
	{
		update = gtd->ses->input->edit->update;

		if (update + 1 >= gtd->ses->input->edit->used)
		{
			return;
		}

		moves = arg ? URANGE(1, atoi(arg), gtd->ses->input->edit->used - update - 1) : 1;

		str_cpy(&gtd->ses->input->edit->line[update]->str, gtd->ses->input->buf);

		inputline_set(gtd->ses->input->edit->line[update + moves]->str, gtd->ses->input->str_pos);

		gtd->ses->input->edit->update += moves;

		gtd->ses->input->cur_row = UMIN(gtd->ses->input->bot_row, gtd->ses->input->cur_row + moves);

		cursor_redraw_edit(ses, arg);

		return;
	}

	if (!inputline_multiline())
	{
		return cursor_history_next(ses, "");
	}

	if (gtd->ses->input->str_pos == gtd->ses->input->str_len)
	{
		return cursor_history_next(ses, "");
	}

	str_max = inputline_max_str_len();

	moves = arg ? UMAX(1, atoi(arg)) : 1;

	while (moves--)
	{
		gtd->ses->input->str_pos = UMIN(gtd->ses->input->str_pos + str_max, gtd->ses->input->str_len);
	}
	gtd->ses->input->raw_pos = inputline_str_raw_len(0, gtd->ses->input->str_pos);

	cursor_redraw_line(ses, arg);
}

DO_CURSOR(cursor_move_left)
{
	int width;

	if (gtd->ses->input->raw_pos > 0)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC))
		{
			gtd->ses->input->raw_pos--;
			gtd->ses->input->str_pos--;
			input_printf("\e[1D");

			if (inputline_str_chk(0, gtd->ses->input->raw_pos) == FALSE)
			{
				gtd->ses->input->raw_pos--;
				gtd->ses->input->str_pos--;
				input_printf("\e[1D");
			}
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			gtd->ses->input->raw_pos--;

			if (gtd->ses->input->raw_pos > 0 && is_utf8_tail(&gtd->ses->input->buf[gtd->ses->input->raw_pos]))
			{
				do
				{
					gtd->ses->input->raw_pos--;
				}
				while (gtd->ses->input->raw_pos > 0 && is_utf8_tail(&gtd->ses->input->buf[gtd->ses->input->raw_pos]));

				get_vt102_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

				if (width == 0)
				{
					return cursor_move_left(ses, "");
				}
				input_printf("\e[%dD", width);
				gtd->ses->input->str_pos -= width;
			}
			else
			{
				get_vt102_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

				gtd->ses->input->str_pos -= width;

				input_printf("\e[%dD", width);
			}
		}
		else
		{
			do
			{
				gtd->ses->input->raw_pos--;

				get_ascii_width(&gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

				if (width)
				{
					gtd->ses->input->str_pos -= width;
					input_printf("\e[%dD", width);
					break;
				}
			}
			while (gtd->ses->input->raw_pos > 0);
		}

		if (gtd->ses->input->raw_pos != gtd->ses->input->str_pos)
		{
			gtd->ses->input->raw_pos = inputline_str_raw_len(0, gtd->ses->input->str_pos);
		}

		cursor_check_line(ses, "");
	}
}

DO_CURSOR(cursor_move_left_word)
{
	int index_raw, span_raw, width;

	if (gtd->ses->input->raw_pos == 0)
	{
		return;
	}

	index_raw = gtd->ses->input->raw_pos;

	while (gtd->ses->input->raw_pos > 0 && gtd->ses->input->buf[gtd->ses->input->raw_pos - 1] == ' ')
	{
		gtd->ses->input->raw_pos--;
	}

	while (gtd->ses->input->raw_pos > 0 && gtd->ses->input->buf[gtd->ses->input->raw_pos - 1] != ' ')
	{
		gtd->ses->input->raw_pos--;
	}

	span_raw = gtd->ses->input->raw_pos;

	while (span_raw < index_raw)
	{
		span_raw += get_vt102_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

		gtd->ses->input->str_pos -= width;
	}

	cursor_redraw_line(ses, "");
}

DO_CURSOR(cursor_move_right)
{
	int size, width;

	if (gtd->ses->input->raw_pos < gtd->ses->input->raw_len)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC))
		{
			gtd->ses->input->raw_pos += get_euc_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

			input_printf("\e[%dC", width);

			gtd->ses->input->str_pos += width;
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			gtd->ses->input->raw_pos += get_vt102_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

			if (width == 0)
			{
				return cursor_move_right(ses, arg);
			}
			input_printf("\e[%dC", width);

			gtd->ses->input->str_pos += width;

			while (gtd->ses->input->raw_pos < gtd->ses->input->raw_len)
			{
				if (!is_utf8_head(&gtd->ses->input->buf[gtd->ses->input->raw_pos]))
				{
					break;
				}
				size = get_utf8_width(&gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

				if (width)
				{
					break;
				}
				gtd->ses->input->raw_pos += size;
			}
		}
		else
		{
			gtd->ses->input->raw_pos += get_ascii_width(&gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

			input_printf("\e[1C");

			gtd->ses->input->str_pos++;
		}
	}

	cursor_check_line(ses, "");
}

DO_CURSOR(cursor_move_right_word)
{
	int size, width;

	if (gtd->ses->input->raw_pos == gtd->ses->input->raw_len)
	{
		return;
	}

	while (gtd->ses->input->raw_pos < gtd->ses->input->raw_len && gtd->ses->input->buf[gtd->ses->input->raw_pos] == ' ')
	{
		gtd->ses->input->raw_pos++;
		gtd->ses->input->str_pos++;
	}

	while (gtd->ses->input->raw_pos < gtd->ses->input->raw_len && gtd->ses->input->buf[gtd->ses->input->raw_pos] != ' ')
	{
		size = get_vt102_width(gtd->ses, &gtd->ses->input->buf[gtd->ses->input->raw_pos], &width);

		gtd->ses->input->raw_pos += size;
		gtd->ses->input->str_pos += width;
	}

	cursor_redraw_line(ses, "");
}

DO_CURSOR(cursor_paste_buffer)
{
	if (*gtd->ses->input->cut)
	{
		inputline_insert(gtd->ses->input->cut, -1);

		cursor_redraw_line(ses, "");

		modified_input();
	}
}

DO_CURSOR(cursor_position)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int row, col;

	arg = sub_arg_in_braces(gtd->ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(gtd->ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg2 == 0)
	{
		col = gtd->ses->input->str_hid + get_col_index_arg(gtd->ses, arg1) - 1;

		inputline_position(col);

		return cursor_redraw_line(ses, arg);
	}

	row = get_row_index_arg(gtd->ses, arg1);
	col = gtd->ses->input->str_hid + get_col_index_arg(gtd->ses, arg2) - 1;

	if (row >= gtd->ses->input->top_row && row <= gtd->ses->input->bot_row)
	{
		if (inputline_rows(gtd->ses) > 1)
		{
			if (inputline_editor())
			{
				inputline_set_row(row);
				inputline_position(col);

				return cursor_redraw_edit(ses, arg);
			}

			gtd->ses->input->cur_row = gtd->ses->input->top_row;

			while (gtd->ses->input->cur_row < row)
			{
				gtd->ses->input->cur_row++;

				col += inputline_max_str_len();
			}
		}
	}
	inputline_position(col);

	return cursor_redraw_line(ses, arg);
}

DO_CURSOR(cursor_macro)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (is_abbrev(arg1, "PRESERVE"))
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_PRESERVEMACRO);

		return;
	}

	if (is_abbrev(arg1, "RESET"))
	{
		gtd->macro_buf[0] = 0;

		return;
	}

	show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR {MACRO} {PRESERVE|RESET}");
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
	cursor_redraw_line(ses, "");

	fflush(stdout);
}

DO_CURSOR(cursor_redraw_line)
{
	push_call("cursor_redraw_line(%p,%p)",ses,arg);

	if (arg == NULL) // avoid unnecessary redraws
	{
		pop_call();
		return;
	}

	if (inputline_str_chk(0, gtd->ses->input->raw_len) == FALSE)
	{
		tintin_printf2(gtd->ses, "debug: cursor_redraw_line: corrupted utf-8 detected");
		pop_call();
		return;
	}

	if (inputline_editor())
	{
		cursor_redraw_singleline(ses, "");

		pop_call();
		return;
	}

	if (inputline_multiline())
	{
		cursor_redraw_multiline(ses, "");

		pop_call();
		return;
	}

	cursor_redraw_singleline(ses, "");

	pop_call();
	return;
}

DO_CURSOR(cursor_redraw_singleline)
{
	int raw_len, raw_off, str_pos, str_max, str_len;

	str_max = inputline_max_str_len();

	// Center long lines of input

	if (gtd->ses->input->str_pos > str_max - 3)
	{
		if (inputline_editor())
		{
			return cursor_redraw_edit(ses, arg);
		}

		while (gtd->ses->input->str_pos - gtd->ses->input->str_hid > str_max - 3)
		{
			gtd->ses->input->str_hid += URANGE(1, 8, str_max);
		}

		while (gtd->ses->input->str_pos - gtd->ses->input->str_hid < 3)
		{
			gtd->ses->input->str_hid -= URANGE(1, str_max - 8, gtd->ses->input->str_hid);
		}
	}
	else
	{
		if (gtd->ses->input->str_hid && gtd->ses->input->str_pos - gtd->ses->input->str_hid < 3)
		{
			if (inputline_editor())
			{
				return cursor_redraw_edit(ses, arg);
			}

			gtd->ses->input->str_hid = 0;
		}
	}

	inputline_erase();

	// Print the entire thing

	raw_len = inputline_str_raw_len(gtd->ses->input->str_hid, gtd->ses->input->str_hid + str_max - 1);
//	str_len = inputline_raw_str_len(0, -1);
	str_len = gtd->ses->input->str_len;
	raw_off = inputline_str_raw_len(0, gtd->ses->input->str_hid);
	str_pos = inputline_cur_off() + gtd->ses->input->str_pos - gtd->ses->input->str_hid;

	if (inputline_editor() || str_max <= 4)
	{
		input_printf("%.*s", raw_len, &gtd->ses->input->buf[raw_off]);
	}
	else if (gtd->ses->input->str_hid)
	{
		if (gtd->ses->input->str_hid + str_max >= str_len)
		{
			input_printf("%.*s\e[%dG<", raw_len, &gtd->ses->input->buf[raw_off], inputline_cur_off());
		}
		else
		{
			input_printf("%.*s\e[%dG<\e[%dG>", raw_len, &gtd->ses->input->buf[raw_off], inputline_cur_off(), inputline_cur_off() - 1 + str_max);
		}
	}
	else
	{
		if (str_max >= inputline_raw_str_len(0, -1))
		{
			input_printf("%.*s",   raw_len, &gtd->ses->input->buf[raw_off]);
		}
		else
		{
			input_printf("%.*s\e[%dG>",  raw_len, &gtd->ses->input->buf[raw_off], inputline_cur_off() - 1 + str_max);
		}
	}

	goto_pos(gtd->ses, inputline_cur_row(), str_pos);

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		cursor_history_find(gtd->ses, "");
	}
}

DO_CURSOR(cursor_redraw_edit)
{
	int row, start, end, offset, raw_off, raw_width, str_max;

	push_call("cursor_redraw_edit(%p,%p)",ses,arg);

	if (arg == NULL)
	{
		pop_call();
		return;
	}

	str_max = inputline_max_str_len();

	// Center long lines of input

	// Probably needs proper tab handling

	if (gtd->ses->input->str_pos > str_max - 3)
	{
		while (gtd->ses->input->str_pos - gtd->ses->input->str_hid > str_max - 3)
		{
			gtd->ses->input->str_hid += URANGE(1, 8, str_max);
		}

		while (gtd->ses->input->str_pos - gtd->ses->input->str_hid < 3)
		{
			gtd->ses->input->str_hid -= URANGE(1, str_max - 8, gtd->ses->input->str_hid);
		}
	}
	else
	{
		if (gtd->ses->input->str_hid && gtd->ses->input->str_pos - gtd->ses->input->str_hid < 3)
		{
			gtd->ses->input->str_hid = 0;
		}
	}

	end   = gtd->ses->input->bot_row - gtd->ses->input->cur_row;

	while (end >= gtd->ses->input->edit->used - gtd->ses->input->edit->update)
	{
		gtd->ses->input->cur_row++;

		end--;
	}

	offset = gtd->ses->input->cur_row - gtd->ses->input->top_row;

	start = gtd->ses->input->edit->update - offset;

	while (start < 0)
	{
		gtd->ses->input->cur_row--;

		start++;
	}


	for (row = gtd->ses->input->top_row ; row <= gtd->ses->input->bot_row ; row++)
	{
		goto_pos(gtd->ses, row, gtd->ses->input->top_col);

		inputline_erase();

		if (start >= gtd->ses->input->edit->used)
		{
			continue;
		}


		if (row == gtd->ses->input->cur_row)
		{
			raw_off = get_raw_off_str_range_raw_width(gtd->ses, gtd->ses->input->buf, gtd->ses->input->str_hid, gtd->ses->input->str_hid + str_max - 1, &raw_width);

			input_printf("%.*s", raw_width, &gtd->ses->input->buf[raw_off]);
		}
		else
		{
			raw_off = get_raw_off_str_range_raw_width(gtd->ses, gtd->ses->input->edit->line[start]->str, gtd->ses->input->str_hid, gtd->ses->input->str_hid + str_max - 1, &raw_width);

			input_printf("%.*s", raw_width, &gtd->ses->input->edit->line[start]->str[raw_off]);
		}
		start++;
	}
/*
	goto_pos(gtd->ses, 1, 1);

	printf("\e[1;31mcur_row %2d, cur_off %2d, str_hid %3d, str_pos %3d, str_max %3d, str_len %3d\e[0m",
		gtd->ses->input->cur_row,
		inputline_cur_off(),
		gtd->ses->input->str_hid,
		gtd->ses->input->str_pos,
		str_max,
		gtd->ses->input->str_len);
*/
	goto_pos(gtd->ses, inputline_cur_row(), inputline_cur_col());

	pop_call();
	return;
}

DO_CURSOR(cursor_redraw_multiline)
{
	int row, rows, str_max, hid, str_off, raw_off, raw_len;

	str_max  = inputline_max_str_len();
	rows     = inputline_rows(gtd->ses);

	if (gtd->ses->input->str_pos - gtd->ses->input->str_hid > rows * str_max - 1)
	{
		while (gtd->ses->input->str_pos - gtd->ses->input->str_hid > rows * str_max - 1)
		{
			gtd->ses->input->str_hid += rows * str_max;
		}
	}
	else if (gtd->ses->input->str_hid && gtd->ses->input->str_pos - gtd->ses->input->str_hid < 0)
	{
		while (gtd->ses->input->str_pos - gtd->ses->input->str_hid < 0)
		{
			gtd->ses->input->str_hid -= URANGE(1, rows * str_max, gtd->ses->input->str_hid);
		}
	}

	hid = gtd->ses->input->str_hid;

	if (hid)
	{
		str_off = gtd->ses->input->str_hid;
		raw_off = inputline_str_raw_len(0, str_off);
	}
	else
	{
		str_off = 0;
		raw_off = 0;
	}

	for (row = gtd->ses->input->top_row ; row <= gtd->ses->input->bot_row ; row++)
	{
		goto_pos(gtd->ses, row, gtd->ses->input->top_col);

		inputline_erase();

		if (gtd->ses->input->str_len < str_off)
		{
			raw_len = 0;
		}
		else
		{
			raw_len = inputline_str_raw_len(str_off, str_off + str_max);

			input_printf("%.*s", raw_len, &gtd->ses->input->buf[raw_off]);
		}
		str_off += str_max;
		raw_off += raw_len;
	}

	gtd->ses->input->cur_row = gtd->ses->input->top_row + (gtd->ses->input->str_pos - gtd->ses->input->str_hid) / str_max;

/*
	goto_pos(gtd->ses, 1, 1);

	printf("\e[1;31mcur_row %2d, top_row %2d, str_pos %3d, str_hid %3d, str_max %3d, str_len %3d\e[0m",  gtd->ses->input->cur_row, gtd->ses->input->top_row, gtd->ses->input->str_pos, gtd->ses->input->str_hid, str_max, gtd->ses->input->str_len);
*/
	goto_pos(gtd->ses, inputline_cur_row(), inputline_cur_off() + gtd->ses->input->str_pos % str_max);

	if (HAS_BIT(gtd->ses->input->flags, INPUT_FLAG_HISTORYSEARCH))
	{
		cursor_history_find(gtd->ses, "");
	}
}


// like set, but sub escapes, unused

DO_CURSOR(cursor_insert_line)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN|SUB_ESC);

	if (*arg1 == 0)
	{
		return;
	}

	inputline_insert(arg1, -1);

	cursor_redraw_line(ses, "");

	modified_input();
}

DO_CURSOR(cursor_set)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		return;
	}

	inputline_insert(arg1, -1);

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
	tintin_printf2(ses, "inputline_max_str_len():        %10d", inputline_max_str_len());
	tintin_printf2(ses, "inputline_cur_off():            %10d", inputline_cur_off());
	tintin_printf2(ses, "inputline_cur_str_len():        %10d", inputline_cur_str_len());
	tintin_printf2(ses, "inputline_rows():               %10d", inputline_rows(gtd->ses));

	tintin_printf2(ses, "");

	tintin_printf2(ses, "inputline_max_row():            %10d", inputline_max_row());
	tintin_printf2(ses, "inputline_cur_row():            %10d", inputline_cur_row());
	tintin_printf2(ses, "inputline_cur_col():            %10d", inputline_cur_col());
	tintin_printf2(ses, "inputline_cur_hid():            %10d", gtd->ses->input->str_hid);

	tintin_printf2(ses, "");

	tintin_printf2(ses, "input->raw_len:                 %10d", gtd->ses->input->raw_len);
	tintin_printf2(ses, "input->str_len:                 %10d", gtd->ses->input->str_len);
	tintin_printf2(ses, "input->raw_pos:                 %10d", gtd->ses->input->raw_pos);
	tintin_printf2(ses, "input->str_pos:                 %10d", gtd->ses->input->str_pos);

	tintin_printf2(ses, "");

	tintin_printf2(ses, "input->line_name:               %s", gtd->ses->input->line_name);
	tintin_printf2(ses, "");

	tintin_printf2(ses, "input->sav_top_row:             %10d", gtd->ses->input->sav_top_row);
	tintin_printf2(ses, "input->sav_top_col:             %10d", gtd->ses->input->sav_top_col);
	tintin_printf2(ses, "input->sav_bot_row:             %10d", gtd->ses->input->sav_bot_row);
	tintin_printf2(ses, "input->sav_bot_col:             %10d", gtd->ses->input->sav_bot_col);

	tintin_printf2(ses, "");

	tintin_printf2(ses, "input->top_row:                 %10d", gtd->ses->input->top_row);
	tintin_printf2(ses, "input->top_col:                 %10d", gtd->ses->input->top_col);
	tintin_printf2(ses, "input->bot_row:                 %10d", gtd->ses->input->bot_row);
	tintin_printf2(ses, "input->bot_col:                 %10d", gtd->ses->input->bot_col);
}

/*
	original tab cycling by Ben Love
*/


DO_CURSOR(cursor_tab)
{
	char arg1[BUFFER_SIZE];
	int flags = 0;

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
		else if (is_abbrev(arg1, "COMPLETE"))
		{
			SET_BIT(flags, TAB_FLAG_COMPLETE);
		}

		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}

	if (is_abbrev(arg1, "FORWARD"))
	{
		if (!HAS_BIT(flags, TAB_FLAG_COMPLETE) && inputline_editor())
		{
			sprintf(arg1, "%*s", gtd->ses->tab_width, "");

			inputline_insert(arg1, -1);

			return cursor_redraw_line(ses, arg);
		}

		if (!HAS_BIT(flags, TAB_FLAG_LIST|TAB_FLAG_SCROLLBACK))
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR TAB {LIST|SCROLLBACK} FORWARD");
		}
		else
		{
			cursor_tab_forward(ses, ntos(flags));
		}
		SET_BIT(flags, TAB_FLAG_FORWARD);
	}
	else if (is_abbrev(arg1, "BACKWARD"))
	{
		if (!HAS_BIT(flags, TAB_FLAG_COMPLETE) && inputline_editor())
		{
			sprintf(arg1, "%*s", gtd->ses->tab_width, "");
			
			inputline_insert(arg1, 0);

			return cursor_redraw_line(ses, arg);
		}

		if (!HAS_BIT(flags, TAB_FLAG_LIST|TAB_FLAG_SCROLLBACK))
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR TAB {LIST|SCROLLBACK} BACKWARD");
		}
		else
		{
			cursor_tab_backward(ses, ntos(flags));
		}
		SET_BIT(flags, TAB_FLAG_BACKWARD);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #CURSOR TAB {LIST;SCROLLBACK} <BACKWARD|FORWARD>");
	}
}

int cursor_tab_add(int stop_after_first)
{
	struct listroot *tab_root = gtd->ses->list[LIST_TAB];
	struct listroot *cmd_root = gtd->ses->list[LIST_COMMAND];
	struct listnode *node;
	char *tail;
	int tail_len;

	if (!HAS_BIT(tab_root->flags, LIST_FLAG_IGNORE))
	{
		tail     = cmd_root->list[0]->arg1;
		tail_len = str_len(tail);

		for (tab_root->update = 0 ; tab_root->update < tab_root->used ; tab_root->update++)
		{
			node = tab_root->list[tab_root->update];

			if (*node->arg1 == *tail && !strncmp(node->arg1, tail, tail_len))
			{
				if (search_node_list(cmd_root, node->arg1))
				{
					continue;
				}
				create_node_list(cmd_root, node->arg1, "", "", "");

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


int cursor_scrollback_tab_add(int stop_after_first)
{
	char tab[BUFFER_SIZE], buf[BUFFER_SIZE];
	struct listroot *root = gtd->ses->list[LIST_COMMAND];
	struct listnode *node;
	int scroll_cnt, tab_len, tail_len;
	char *ptb, *ptt, *tail;

	tail     = root->list[0]->arg1;
	tail_len = str_len(tail);

	if (root->list[root->used - 1]->val32[0])
	{
		scroll_cnt = UMIN(root->list[root->used - 1]->val32[0], gtd->ses->scroll->used - 1);
	}
	else
	{
		scroll_cnt = gtd->ses->scroll->used - 1;
	}

	for ( ; scroll_cnt > 0 ; scroll_cnt--)
	{
		if (HAS_BIT(gtd->ses->scroll->buffer[scroll_cnt]->flags, BUFFER_FLAG_GREP))
		{
			continue;
		}

		strip_vt102_codes(gtd->ses->scroll->buffer[scroll_cnt]->str, buf);

		ptb = buf;

		while (*ptb)
		{
			while (*ptb && is_space(*ptb))
			{
				ptb++;
			}

			if (*ptb == *tail && !strncmp(ptb, tail, tail_len))
			{
				ptt = tab;

				for (tab_len = 0 ; tab_len < tail_len ; tab_len++)
				{
					*ptt++ = *ptb++;
				}

				while (*ptb && *ptb != ' ')
				{
					switch (*ptb)
					{
						case ';':
						case '.':
						case ',':
						case '!':
						case '?':
						case ':':
						case '"':
							*ptt++ = 0;
							ptb++;
							break;

						default:
							*ptt++ = *ptb++;
							break;
					}
				}
				*ptt = 0;

				if (search_node_list(gtd->ses->list[LIST_COMMAND], tab))
				{
					continue;
				}

				node = create_node_list(gtd->ses->list[LIST_COMMAND], tab, "", "", "");

				node->val32[0] = scroll_cnt;

				if (stop_after_first)
				{
					return TRUE;
				}

				if (root->used > 100)
				{
					return FALSE;
				}
			}

			while (*ptb && !is_space(*ptb))
			{
				ptb++;
			}
		}
	}

	return FALSE;
}

DO_CURSOR(cursor_tab_forward)
{
	struct listroot *root = gtd->ses->list[LIST_COMMAND];
	int tab_found, flags;

	if (inputline_tab_valid() == FALSE)
	{
		return;
	}

	cursor_move_left_word(gtd->ses, "");

	if (root->used == 0)
	{
		create_node_list(root, &gtd->ses->input->buf[gtd->ses->input->raw_pos], "", "", "");
	}

	inputline_cap("");

	tab_found = 0;

	flags = atoi(arg);

	if (tab_found == 0 && HAS_BIT(flags, TAB_FLAG_LIST))
	{
		tab_found = cursor_tab_add(TRUE);
	}

	if (tab_found == 0 && HAS_BIT(flags, TAB_FLAG_SCROLLBACK))
	{
		tab_found = cursor_scrollback_tab_add(TRUE);
	}

	if (tab_found == 0)
	{
		inputline_insert(root->list[0]->arg1, -1);

		kill_list(root);
	}
	else
	{
		inputline_insert(root->list[root->used - 1]->arg1, -1);
	}
	cursor_end(ses, "");
}

DO_CURSOR(cursor_tab_backward)
{
	struct listroot *root = ses->list[LIST_COMMAND];
	int flags;

	if (inputline_tab_valid() == FALSE)
	{
		return;
	}

	if (root->used)
	{
		delete_index_list(root, root->used - 1);
	}

	cursor_move_left_word(gtd->ses, "");

	if (root->used == 0)
	{
		create_node_list(root, &gtd->ses->input->buf[gtd->ses->input->raw_pos], "", "", "");

		inputline_cap("");

		flags = atoi(arg);

		if (HAS_BIT(flags, TAB_FLAG_LIST))
		{
			cursor_tab_add(FALSE);
		}

		if (HAS_BIT(flags, TAB_FLAG_SCROLLBACK))
		{
			cursor_scrollback_tab_add(FALSE);
		}
	}
	else
	{
		inputline_cap("");
	}

	inputline_insert(root->list[root->used - 1]->arg1, -1);

	cursor_end(ses, "");
}
