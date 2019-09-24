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
*                     coded by Igor van den Hoven 2004                        *
******************************************************************************/

#include "tintin.h"


void init_buffer(struct session *ses, int size)
{
	int cnt;

	push_call("init_buffer(%p,%p)",ses,size);

	if (ses->scroll == NULL)
	{
		ses->scroll = (struct scroll_data *) calloc(1, sizeof(struct scroll_data));
	}

	if (size == -1)
	{
		size = gts->scroll->max;
	}

	if (ses->scroll->max == size)
	{
		pop_call();
		return;
	}

	if (ses->scroll->buffer)
	{
		cnt = ses->scroll->row;

		do
		{
			if (++cnt == ses->scroll->max)
			{
				cnt = 0;
			}

			if (ses->scroll->buffer[cnt] == NULL)
			{
				break;
			}

			str_unhash(ses->scroll->buffer[cnt]);
		}
		while (cnt != ses->scroll->row);
	}

	if (ses->scroll->buffer)
	{
		free(ses->scroll->buffer);
	}

	if (ses->scroll->input)
	{
		str_free(ses->scroll->input);
	}

	if (size)
	{
		ses->scroll->buffer = (char **) calloc(size, sizeof(char *));
		ses->scroll->input  = str_alloc(1);

		ses->scroll->max  = size;
		ses->scroll->row  = size - 1;
		ses->scroll->line = - 1;
	}
	else
	{
		free(ses->scroll);
	}

	pop_call();
	return;
}

int get_scroll_home(struct session *ses)
{
	if (ses->scroll->buffer[0])
	{
		return ses->scroll->row - 1;
	}
	else
	{
		return ses->scroll->max - 1;
	}
}

void add_line_buffer(struct session *ses, char *line, int prompt)
{
	char *linebuf;
	char *pti, *pto;
	int lines;
	int sav_row, sav_col, cur_row, cur_col, top_row, bot_row;

	push_call("add_line_buffer(%p,%s,%d)",ses,line,prompt);

	if (gtd->scroll_level || ses->scroll == NULL)
	{
		pop_call();
		return;
	}

	sav_row = ses->sav_row;
	sav_col = ses->sav_col;
	cur_row = ses->cur_row;
	cur_col = ses->cur_col;
	top_row = ses->top_row;
	bot_row = ses->bot_row;

	if (str_len(ses->scroll->input) + strlen(line) + 100 >= BUFFER_SIZE)
	{
		str_cat_printf(&ses->scroll->input, "\n\e[1;31m#BUFFER: LINE LENGTH OF (%d) EXEEDS MAXIMUM SIZE OF (%d)%s\n", str_len(ses->scroll->input) + strlen(line), BUFFER_SIZE, COLOR_TEXT);
	}
	else
	{
		if (prompt == TRUE)
		{
			str_cat(&ses->scroll->input, line);

			pop_call();
			return;
		}
		else
		{
			str_cat(&ses->scroll->input, line);
		}
	}

	pti = pto = ses->scroll->input;

	while (*pti != 0)
	{
		while (skip_vt102_codes_non_graph(pti))
		{
			interpret_vt102_codes(ses, pti, FALSE);

			pti += skip_vt102_codes_non_graph(pti);
		}

		if (*pti == 0)
		{
			break;
		}

		if (SCROLL(ses))
		{
			*pto++ = *pti++;
		}
		else
		{
			pti++;
		}
	}
	*pto = 0;

	if (HAS_BIT(ses->flags, SES_FLAG_SNOOP) && ses != gtd->ses)
	{
		gtd->scroll_level++;

		tintin_printf2(gtd->ses, "%s[%s%s] %s", COLOR_TEXT, ses->name, ses->scroll->input, COLOR_TEXT);

		gtd->scroll_level--;
	}

	linebuf = str_alloc(str_len(ses->scroll->input) * 2);

	lines = word_wrap(ses, ses->scroll->input, linebuf, FALSE);

	str_cpy(&ses->scroll->input, "");

	ses->scroll->buffer[ses->scroll->row] = str_hash(linebuf, lines);

	if (prompt == -1)
	{
		str_hash_grep(ses->scroll->buffer[ses->scroll->row], TRUE);
	}

	if (!HAS_BIT(ses->logmode, LOG_FLAG_LOW))
	{
		if (ses->logfile)
		{
			logit(ses, linebuf, ses->logfile, LOG_FLAG_LINEFEED);
		}
	}

	if (gtd->chat)
	{
		chat_forward_session(ses, linebuf);
	}

	if (--ses->scroll->row < 0)
	{
		ses->scroll->row = ses->scroll->max -1;
	}

	if (ses->scroll->buffer[ses->scroll->row])
	{
		ses->scroll->buffer[ses->scroll->row] = str_unhash(ses->scroll->buffer[ses->scroll->row]);
	}

	ses->sav_row = sav_row;
	ses->sav_col = sav_col;
	ses->cur_row = cur_row;
	ses->cur_col = cur_col;
	ses->top_row = top_row;
	ses->bot_row = bot_row;

	str_free(linebuf);

	pop_call();
	return;
}

DO_COMMAND(do_grep)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int scroll_cnt, grep_cnt, grep_min, grep_max, grep_add, page;

	grep_cnt = grep_add = scroll_cnt = grep_min = 0;
	grep_max = ses->bot_row - ses->top_row - 2;

	sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (ses->scroll == NULL)
	{
		show_error(ses, LIST_COMMAND, "#GREP, NO SCROLL BUFFER AVAILABLE.");

		return ses;
	}

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: GREP [#] <SEARCH TEXT>");

		return ses;
	}

	page = get_number(ses, arg1);

	if (page)
	{
		arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
		arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

		if (*arg2 == 0)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #grep {%s} <SEARCH TEXT>", arg1);

			return ses;
		}
	}
	else
	{
		page = 1;

		arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);
	}

	if (page > 0)
	{
		grep_min = grep_max * page - grep_max;
		grep_max = grep_max * page;
	}
	else
	{
		grep_min = grep_max * (page * -1) - grep_max;
		grep_max = grep_max * (page * -1);
	}

	gtd->scroll_level++;

	tintin_header(ses, " GREPPING PAGE %d FOR %s ", page, arg2);

	if (page > 0)
	{
		scroll_cnt = ses->scroll->row;

		do
		{
			if (scroll_cnt == ses->scroll->max - 1)
			{
				scroll_cnt = 0;
			}
			else
			{
				scroll_cnt++;
			}

			if (ses->scroll->buffer[scroll_cnt] == NULL)
			{
				break;
			}

			if (str_hash_grep(ses->scroll->buffer[scroll_cnt], FALSE))
			{
				continue;
			}

			if (find(ses, ses->scroll->buffer[scroll_cnt], arg2, SUB_NONE, SUB_NONE))
			{
				grep_add = str_hash_lines(ses->scroll->buffer[scroll_cnt]);

				if (grep_cnt + grep_add > grep_max)
				{
					break;
				}

				grep_cnt += grep_add;
			}
		}
		while (scroll_cnt != ses->scroll->row);

		if (grep_cnt <= grep_min)
		{
			show_error(ses, LIST_COMMAND, "#NO MATCHES FOUND.");
			
			return ses;
		}

		do
		{
			if (scroll_cnt == 0)
			{
				scroll_cnt = ses->scroll->max - 1;
			}
			else
			{
				scroll_cnt--;
			}

			if (ses->scroll->buffer[scroll_cnt] == NULL)
			{
				break;
			}

			if (str_hash_grep(ses->scroll->buffer[scroll_cnt], FALSE))
			{
				continue;
			}

			if (find(ses, ses->scroll->buffer[scroll_cnt], arg2, SUB_NONE, SUB_NONE))
			{
				grep_add = str_hash_lines(ses->scroll->buffer[scroll_cnt]);

				if (grep_cnt - grep_add < grep_min)
				{
					break;
				}

				grep_cnt -= grep_add;

				tintin_puts2(ses, ses->scroll->buffer[scroll_cnt]);
			}
		}
		while (scroll_cnt != ses->scroll->row);
	}
	else
	{
		scroll_cnt = get_scroll_home(ses);

		do
		{
			if (scroll_cnt == -1)
			{
				scroll_cnt = 0;
			}
			else
			{
				scroll_cnt--;
			}

			if (ses->scroll->buffer[scroll_cnt] == NULL)
			{
				break;
			}

			if (str_hash_grep(ses->scroll->buffer[scroll_cnt], FALSE))
			{
				continue;
			}

			if (find(ses, ses->scroll->buffer[scroll_cnt], arg2, SUB_NONE, SUB_NONE))
			{
				grep_add = str_hash_lines(ses->scroll->buffer[scroll_cnt]);

				if (grep_cnt >= grep_min)
				{
					tintin_puts2(ses, ses->scroll->buffer[scroll_cnt]);
				}

				if (grep_cnt + grep_add >= grep_max)
				{
					break;
				}

				grep_cnt += grep_add;
			}
		}
		while (scroll_cnt != ses->scroll->row);

		if (grep_cnt <= grep_min)
		{
			show_error(ses, LIST_COMMAND, "#NO MATCHES FOUND.");
			
			return ses;
		}
	}
	tintin_header(ses, "");

	gtd->scroll_level--;

	return ses;
}

int show_buffer(struct session *ses)
{
	char temp[STRING_SIZE];
	int scroll_size, scroll_cnt, scroll_tmp, scroll_add, scroll_cut;

	if (ses != gtd->ses)
	{
		return TRUE;
	}

	scroll_size = get_scroll_size(ses);
	scroll_add  = 0 - ses->scroll->base;
	scroll_cnt  = ses->scroll->line;
	scroll_cut  = 0;

	while (TRUE)
	{
		if (ses->scroll->buffer[scroll_cnt] == NULL)
		{
			break;
		}

		scroll_tmp = str_hash_lines(ses->scroll->buffer[scroll_cnt]);

		if (scroll_add + scroll_tmp > scroll_size)
		{
			if (scroll_add == scroll_size)
			{
				scroll_cut = 0;
			}
			else
			{
				scroll_cut = scroll_tmp - (scroll_size - scroll_add);
			}
			break;
		}
		ses->scroll->base = 0;

		scroll_add += scroll_tmp;

		if (scroll_cnt == ses->scroll->max - 1)
		{
			scroll_cnt = 0;
		}
		else
		{
			scroll_cnt++;
		}
	}

	erase_scroll_region(ses);

	if (ses->scroll->buffer[scroll_cnt] == NULL)
	{
		save_pos(ses);
		goto_rowcol(ses, ses->bot_row, 1);
	}
	else
	{
		save_pos(ses);
		goto_rowcol(ses, ses->top_row, 1);
	}

	if (IS_SPLIT(ses))
	{
		SET_BIT(ses->flags, SES_FLAG_READMUD);
	}

	if (ses->scroll->buffer[scroll_cnt] && scroll_cut)
	{
		word_wrap_split(ses, ses->scroll->buffer[scroll_cnt], temp, ses->wrap, scroll_cut, scroll_tmp - ses->scroll->base);

		printf("%s\n", temp);

		if (ses->scroll->base)
		{
			goto eof;
		}
	}

	scroll_cut = 0;

	while (TRUE)
	{
		if (scroll_cnt == 0)
		{
			scroll_cnt = ses->scroll->max - 1;
		}
		else
		{
			scroll_cnt--;
		}

		if (ses->scroll->buffer[scroll_cnt] == NULL)
		{
			break;
		}

		scroll_tmp = word_wrap(ses, ses->scroll->buffer[scroll_cnt], temp, TRUE);

		if (scroll_add - scroll_tmp < 0)
		{
			scroll_cut = scroll_add;
			break;
		}

		scroll_add -= scroll_tmp;

		printf("%s\n", temp);
	}

	if (ses->scroll->buffer[scroll_cnt] && scroll_cut)
	{
		word_wrap_split(ses, ses->scroll->buffer[scroll_cnt], temp, ses->wrap, 0, scroll_cut);

		printf("%s\n", temp);

		ses->scroll->base = scroll_tmp - scroll_cut;
	}
	else
	{
		ses->scroll->base = 0;
	}

	eof:

	restore_pos(ses);

	if (IS_SPLIT(ses))
	{
		DEL_BIT(ses->flags, SES_FLAG_READMUD);
	}
	return TRUE;
}


DO_COMMAND(do_buffer)
{
	char arg1[BUFFER_SIZE];
	int cnt;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_RESETBUFFER))
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_RESETBUFFER);

		reset_hash_table();
	}

	if (*arg1 == 0)
	{
		tintin_header(ses, " BUFFER COMMANDS ");

		for (cnt = 0 ; *buffer_table[cnt].name != 0 ; cnt++)
		{
			tintin_printf2(ses, "  [%-13s] %s", buffer_table[cnt].name, buffer_table[cnt].desc);
		}
		tintin_header(ses, "");

		return ses;
	}

	for (cnt = 0 ; *buffer_table[cnt].name ; cnt++)
	{
		if (!is_abbrev(arg1, buffer_table[cnt].name))
		{
			continue;
		}

		buffer_table[cnt].fun(ses, arg);

		return ses;
	}

	do_buffer(ses, "");

	return ses;
}

DO_BUFFER(buffer_clear)
{
	int cnt;

	cnt = ses->scroll->row;
		
	do
	{
		if (++cnt == ses->scroll->max)
		{
			cnt = 0;
		}

		if (ses->scroll->buffer[cnt] == NULL)
		{
			break;
		}
		ses->scroll->buffer[cnt] = str_unhash(ses->scroll->buffer[cnt]);
	}
	while (cnt != ses->scroll->row);

	ses->scroll->row  = ses->scroll->max - 1;
	ses->scroll->line = - 1;
}

DO_BUFFER(buffer_up)
{
	char arg1[BUFFER_SIZE];
	int scroll_size, scroll_cnt, buffer_add, buffer_tmp;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (ses->scroll->line == -1)
	{
		ses->scroll->line = ses->scroll->row + 1;
	}

	scroll_size = get_scroll_size(ses);
	scroll_cnt  = ses->scroll->line;
	buffer_add  = 0 - ses->scroll->base;

	if (is_math(ses, arg1))
	{
		scroll_size = URANGE(1, get_number(ses, arg1), get_scroll_size(ses));
	}

	while (TRUE)
	{
		if (ses->scroll->buffer[scroll_cnt] == NULL)
		{
			return;
		}

		buffer_tmp = str_hash_lines(ses->scroll->buffer[scroll_cnt]);

		if (scroll_size < buffer_add + buffer_tmp)
		{
			ses->scroll->line = scroll_cnt;
			ses->scroll->base = scroll_size - buffer_add;

			break;
		}

		buffer_add += buffer_tmp;

		if (scroll_cnt == ses->scroll->max - 1)
		{
			scroll_cnt = 0;
		}
		else
		{
			scroll_cnt++;
		}
	}

	show_buffer(ses);

	return;
}

DO_BUFFER(buffer_down)
{
	char arg1[BUFFER_SIZE];
	int scroll_size, scroll_cnt, buffer_add, buffer_tmp;

	if (ses->scroll->line == -1)
	{
		return;
	}

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	scroll_size = get_scroll_size(ses);
	scroll_cnt  = ses->scroll->line;
	buffer_add  = 0;

	if (is_math(ses, arg1))
	{
		scroll_size = URANGE(1, get_number(ses, arg1), get_scroll_size(ses));
	}

	if (ses->scroll->base)
	{
		if (ses->scroll->base >= scroll_size)
		{
			ses->scroll->base -= scroll_size;

			goto eof;
		}
		else
		{
			buffer_add = ses->scroll->base;
			ses->scroll->base = 0;
		}
	}

	if (scroll_cnt == 0)
	{
		scroll_cnt = ses->scroll->max - 1;
	}
	else
	{
		scroll_cnt--;
	}

	while (TRUE)
	{
		if (ses->scroll->buffer[scroll_cnt] == NULL)
		{
			buffer_end(ses, "");
			return;
		}

		buffer_tmp = str_hash_lines(ses->scroll->buffer[scroll_cnt]);

//              size=50 tmp=200 base=100 add=100

		if (scroll_size <= buffer_add + buffer_tmp)
		{
			ses->scroll->line = scroll_cnt;
			ses->scroll->base = buffer_tmp - (scroll_size - buffer_add);

			break;
		}

		buffer_add += buffer_tmp;

		if (scroll_cnt == 0)
		{
			scroll_cnt = ses->scroll->max - 1;
		}
		else
		{
			scroll_cnt--;
		}
	}

	eof:

	show_buffer(ses);
}

DO_BUFFER(buffer_home)
{
	if (ses->scroll == NULL)
	{
		show_error(ses, LIST_COMMAND, "#BUFFER: NO SCROLL BUFFER AVAILABLE.");

		return;
	}

	ses->scroll->line = get_scroll_home(ses);

	ses->scroll->base = str_hash_lines(ses->scroll->buffer[ses->scroll->line]);
	ses->scroll->line++;

	buffer_down(ses, "");
}

DO_BUFFER(buffer_end)
{
	if (ses->scroll == NULL)
	{
		show_error(ses, LIST_COMMAND, "#BUFFER: NO SCROLL BUFFER AVAILABLE.");

		return;
	}

	if (ses->scroll->row == ses->scroll->max - 1)
	{
		ses->scroll->line = 0;
	}
	else
	{
		ses->scroll->line = ses->scroll->row + 1;
	}

	ses->scroll->base = 0;

	show_buffer(ses);

	ses->scroll->line = -1;
	ses->scroll->base =  0;
}

DO_BUFFER(buffer_lock)
{
	char arg1[BUFFER_SIZE];

	if (ses->scroll->buffer == NULL)
	{
		show_error(ses, LIST_COMMAND, "#BUFFER: NO SCROLL BUFFER AVAILABLE.");

		return;
	}

	sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (!strcasecmp(arg1, "ON"))
	{
		ses->scroll->line = ses->scroll->row + 1;
	}
	else if (!strcasecmp(arg1, "OFF"))
	{
		 buffer_end(ses, "");
	}
	else
	{
		if (ses->scroll->line == -1)
		{
			ses->scroll->line = ses->scroll->row + 1;
		}
		else
		{
			buffer_end(ses, "");
		}
	}
}

DO_BUFFER(buffer_find)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int scroll_cnt, grep_cnt, grep_max, page;

	grep_cnt = grep_max = scroll_cnt = 0;

	sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (ses->scroll->buffer == NULL)
	{
		show_error(gtd->ses, LIST_COMMAND, "#BUFFER: NO SCROLL BUFFER AVAILABLE.");

		return;
	}

	if (*arg1 == 0)
	{
		show_error(gtd->ses, LIST_COMMAND, "#BUFFER, FIND WHAT?");

		return;
	}

	page = get_number(ses, arg1);

	if (page)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

		if (*arg2 == 0)
		{
			show_error(gtd->ses, LIST_COMMAND, "#BUFFER, FIND OCCURANCE %d OF WHAT?", page);

			return;
		}
	}
	else
	{
		page = 1;

		arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);
	}

	if (page > 0)
	{
		scroll_cnt = ses->scroll->row;

		do
		{
			if (scroll_cnt == ses->scroll->max -1)
			{
				scroll_cnt = 0;
			}
			else
			{
				scroll_cnt++;
			}

			if (ses->scroll->buffer[scroll_cnt] == NULL)
			{
				break;
			}

			if (str_hash_grep(ses->scroll->buffer[scroll_cnt], FALSE))
			{
				continue;
			}

			if (find(ses, ses->scroll->buffer[scroll_cnt], arg2, SUB_NONE, SUB_NONE))
			{
				grep_cnt++;

				if (grep_cnt == page)
				{
					break;
				}
			}
		}
		while (scroll_cnt != ses->scroll->row);
	}
	else
	{
		scroll_cnt = get_scroll_home(ses);

		do
		{
			if (scroll_cnt == 0)
			{
				scroll_cnt = ses->scroll->max -1;
			}
			else
			{
				scroll_cnt--;
			}

			if (ses->scroll->buffer[scroll_cnt] == NULL)
			{
				break;
			}

			if (str_hash_grep(ses->scroll->buffer[scroll_cnt], FALSE))
			{
				continue;
			}

			if (find(ses, ses->scroll->buffer[scroll_cnt], arg2, SUB_NONE, SUB_NONE))
			{
				grep_cnt--;

				if (grep_cnt == page)
				{
					break;
				}

			}
		}
		while (scroll_cnt != ses->scroll->row);
	}

	if (ses->scroll->buffer[scroll_cnt] == NULL || scroll_cnt == ses->scroll->row)
	{
		show_error(gtd->ses, LIST_COMMAND, "#BUFFER FIND, NO MATCHES FOUND.");

		return;
	}

	ses->scroll->line = scroll_cnt;

	show_buffer(ses);

	return;
}

DO_BUFFER(buffer_get)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];
	int min, max, cur, cnt, add;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_NONE);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

	min = get_number(ses, arg2);
	max = get_number(ses, arg3);

	if (*arg1 == 0 || *arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #BUFFER GET <VARIABLE> <LOWER BOUND> [UPPER BOUND]");

		return;
	}

	if (*arg3 == 0)
	{
		cur = UMAX(0, (ses->scroll->row + min) % ses->scroll->max);

		if (ses->scroll->buffer[cur] == NULL)
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg1, "");
		}
		else
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg1, "%s", ses->scroll->buffer[cur]);
		}
		return;
	}

	cnt = 0;
	add = (min < max) ? 1 : -1;

	set_nest_node(ses->list[LIST_VARIABLE], arg1, "");

	while ((add == 1 && max >= min) || (add == -1 && max <= min))
	{
		sprintf(arg2, "%s[%d]", arg1, ++cnt);

		cur = (ses->scroll->row + min) % ses->scroll->max;

		if (ses->scroll->buffer[cur] == NULL)
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "");
		}
		else
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", ses->scroll->buffer[cur]);
		}

		min = min + add;
	}
	return;
}

DO_BUFFER(buffer_write)
{
	FILE *fp;
	char arg1[BUFFER_SIZE], out[STRING_SIZE];
	int cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #BUFFER WRITE <FILENAME>]");
	}
	else
	{
		if ((fp = fopen(arg1, "w")))
		{
			show_message(ses, LIST_COMMAND, "#OK: WRITING BUFFER TO '%s'.", arg1);

			loginit(ses, fp, LOG_FLAG_OVERWRITE | HAS_BIT(ses->logmode, LOG_FLAG_HTML));

			cnt = ses->scroll->row;

			do
			{
				if (cnt == 0)
				{
					cnt = ses->scroll->max - 1;
				}
				else
				{
					cnt--;
				}

				if (ses->scroll->buffer[cnt] == NULL)
				{
					continue;
				}

				if (HAS_BIT(ses->logmode, LOG_FLAG_PLAIN))
				{
					strip_vt102_codes(ses->scroll->buffer[cnt], out);
				}
				else if (HAS_BIT(ses->logmode, LOG_FLAG_HTML))
				{
					vt102_to_html(ses, ses->scroll->buffer[cnt], out);
				}
				else
				{
					strcpy(out, ses->scroll->buffer[cnt]);
				}
				strcat(out, "\n");

				fputs(out, fp);
			}
			while (cnt != ses->scroll->row);

			fclose(fp);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #BUFFER WRITE {%s} - FAILED TO OPEN FILE.", arg1);
		}
	}
	return;
}

DO_BUFFER(buffer_info)
{
	tintin_printf2(ses, "Scroll row:  %d", ses->scroll->row);
	tintin_printf2(ses, "Scroll max:  %d", ses->scroll->max);
	tintin_printf2(ses, "Scroll line: %d", ses->scroll->line);
	tintin_printf2(ses, "Scroll base: %d", ses->scroll->base);

	tintin_printf2(ses, "");

	str_hash_info(ses);
}
