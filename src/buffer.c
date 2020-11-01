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
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"


void init_buffer(struct session *ses, int size)
{
	int cnt;

	push_call("init_buffer(%p,%p)",ses,size);

	if (size && size == ses->scroll->size)
	{
		pop_call();
		return;
	}

	if (ses->scroll->buffer)
	{
		for (cnt = 0 ; cnt < ses->scroll->used ; cnt++)
		{
			free(ses->scroll->buffer[cnt]->str);
			free(ses->scroll->buffer[cnt]);
		}
		free(ses->scroll->buffer);

		str_free(ses->scroll->input);
	}

	if (size)
	{
		ses->scroll->buffer = (struct buffer_data **) calloc(size, sizeof(struct buffer_data *));
		ses->scroll->input  = str_dup("");

		ses->scroll->size  = size;
		ses->scroll->used  = 0;
		ses->scroll->wrap  = get_scroll_cols(ses);

		add_line_buffer(ses, "", 0);

		ses->scroll->line = -1;
	}
	else
	{
		free(ses->scroll);
	}

	pop_call();
	return;
}

void check_buffer(struct session *ses)
{
	struct buffer_data *buffer;
	char temp[STRING_SIZE];
	int index, wrap;

	if (!HAS_BIT(ses->scroll->flags, SCROLL_FLAG_RESIZE))
	{
		return;
	}

	DEL_BIT(ses->scroll->flags, SCROLL_FLAG_RESIZE);

	wrap = get_scroll_cols(ses);

	if (ses->scroll->width)
	{
		if (wrap == ses->scroll->wrap)
		{
			return;
		}

		if (ses->scroll->width < ses->scroll->wrap && ses->scroll->width < wrap)
		{
			goto end;
		}
	}

	ses->scroll->width = 0;

	for (index = ses->scroll->used - 1 ; index >= 0 ; index--)
	{
		buffer = ses->scroll->buffer[index];

		if (buffer->width < wrap && buffer->width < ses->scroll->wrap)
		{
			buffer->height = buffer->lines;
		}
		else
		{
			buffer->lines = word_wrap_split(ses, buffer->str, temp, wrap, 0, 0, FLAG_NONE, &buffer->height, &buffer->width);
		}

		if (ses->scroll->width < buffer->width)
		{
			ses->scroll->width = buffer->width;
		}
	}

	end:

	ses->scroll->wrap = wrap;
	ses->scroll->time = gtd->time;
	ses->scroll->base = 0;
	ses->scroll->line = -1;

	return;
}

void update_scrollbar(struct session *ses)
{
	if (HAS_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLMODE))
	{
		if (ses == gtd->ses)
		{
			int line = gtd->ses->scroll->line >= 0 ? gtd->ses->scroll->line : gtd->ses->scroll->used + 1;

			line = URANGE(1, line - get_scroll_rows(gtd->ses), gtd->ses->scroll->used);

			print_stdout(0, 0, "\e[%d;%d#t", line, gtd->ses->scroll->used);

			check_all_events(ses, EVENT_FLAG_UPDATE, 0, 2, "SCROLLBAR UPDATE", ntos(line), ntos(gtd->ses->scroll->used));

			DEL_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLUPDATE);
		}
	}
}

void add_line_buffer(struct session *ses, char *line, int prompt)
{
	char *temp;
	char *pti, *pto;
	int cnt, purge;
	int skip, cur_row, cur_col, top_row, bot_row;
	struct buffer_data *buffer;

	push_call("add_line_buffer(%p,%s,%d)",ses,line,prompt);

	if (gtd->level->scroll)
	{
		pop_call();
		return;
	}

	temp = str_alloc_stack(0);

	SET_BIT(gtd->flags, TINTIN_FLAG_SESSIONUPDATE);
	SET_BIT(ses->flags, SES_FLAG_BUFFERUPDATE);

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_CONVERTMETA))
	{
		convert_meta(line, temp, TRUE);

		line = temp;
	}

	cur_row = ses->cur_row;
	cur_col = ses->cur_col;
	top_row = ses->split->top_row;
	bot_row = ses->split->bot_row;

	if (str_len(ses->scroll->input) + strlen(line) + 100 >= BUFFER_SIZE)
	{
		str_cat_printf(&ses->scroll->input, "\n\e[1;31m#BUFFER: LINE LENGTH OF (%d) EXEEDS MAXIMUM SIZE OF (%d)%s\n", str_len(ses->scroll->input) + strlen(line), BUFFER_SIZE, COLOR_TEXT);

		prompt = FALSE;
	}
	else
	{
		if (prompt == TRUE)
		{
			pti = strchr(line, '\n');

			while (pti)
			{
				*pti = 0;

				add_line_buffer(ses, line, FALSE);

				*pti++ = '\n';

				line = pti;

				pti = strchr(line, '\n');
			}

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
		skip = skip_vt102_codes_non_graph(pti);

		if (skip)
		{
			interpret_vt102_codes(ses, pti, FALSE);

			pti += skip;

			continue;
		}

		skip = skip_vt102_codes(pti);

		if (SCROLL(ses))
		{
			if (skip)
			{
				while (skip--)
				{
					*pto++ = *pti++;
				}
			}
			else
			{
				*pto++ = *pti++;
			}
		}
		else
		{
			if (skip)
			{
				pti += skip;
			}
			else
			{
				pti++;
			}
		}
	}
	*pto = 0;

	if (HAS_BIT(ses->flags, SES_FLAG_SNOOP) && ses != gtd->ses)
	{
		tintin_printf2(gtd->ses, "%s[%s%s] %s", COLOR_TEXT, ses->name, ses->scroll->input, COLOR_TEXT);
	}

	if (ses->proxy)
	{
		port_socket_printf(ses, ses->proxy, "%s%s", ses->scroll->input, prompt ? "" : "\n");
	}

	ses->scroll->buffer[ses->scroll->used] = calloc(1, sizeof(struct buffer_data));

	buffer = ses->scroll->buffer[ses->scroll->used];

	buffer->lines = word_wrap_split(ses, ses->scroll->input, temp, ses->wrap, 0, 0, FLAG_NONE, &buffer->height, &buffer->width);
	buffer->time  = gtd->time;
	buffer->str   = strdup(ses->scroll->input);

	if (ses->scroll->line == -1)
	{
		add_line_screen(ses, temp, 0);
	}

	if (gtd->level->grep || prompt == -1)
	{
		SET_BIT(buffer->flags, BUFFER_FLAG_GREP);
	}

	ses->scroll->used++;

	str_cpy(&ses->scroll->input, "");

	if (!HAS_BIT(ses->logmode, LOG_FLAG_LOW))
	{
		if (ses->logfile)
		{
			logit(ses, temp, ses->logfile, LOG_FLAG_LINEFEED);
		}
	}

	if (gtd->chat)
	{
		chat_forward_session(ses, temp);
	}

	if (ses->scroll->used == ses->scroll->size)
	{
		purge = URANGE(10, ses->scroll->size / 10, 10000);

		for (cnt = 0 ; cnt < purge ; cnt++)
		{
			free(ses->scroll->buffer[cnt]->str);
			free(ses->scroll->buffer[cnt]);
		}
		memmove(&ses->scroll->buffer[0], &ses->scroll->buffer[purge], (ses->scroll->size - purge) * sizeof(struct buffer_data *));

		ses->scroll->used -= purge;
		ses->scroll->line = URANGE(-1, ses->scroll->line - purge, ses->scroll->used - 1);
	}

	if (HAS_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLMODE))
	{
		SET_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLUPDATE);
	}

	ses->cur_row = cur_row;
	ses->cur_col = cur_col;
	ses->split->top_row = top_row;
	ses->split->bot_row = bot_row;

	pop_call();
	return;
}


void buffer_print(struct session *ses, int index, int start, int end)
{
	struct buffer_data *buffer;
	char *pti, temp[STRING_SIZE];
	int col, swap, str_len, col_len, height, width;

	push_call("buffer_print(%p,%d,%d,%d)",ses,index,start,end);

	col_len = get_scroll_cols(ses);

	// prompt

	if (start == 0 && end == 0)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_PRINTBUFFER) || ses->scroll->line == ses->scroll->used - 1)
		{
			word_wrap_split(ses, ses->scroll->input, temp, ses->wrap, 0, 1, WRAP_FLAG_SPLIT, &height, &width);
		}
		else
		{
			strcpy(temp, "");
		}

		if (ses->cur_row != ses->split->bot_row)
		{
			print_stdout(0, 0, "[1;31m%02d[0m \e[1;32mmisaligned (%d)", ses->cur_row, ses->scroll->line);
		}
		else
		{
			print_stdout(0, 0, "%s", temp);

			add_line_screen(ses, temp, ses->cur_row);

			erase_cols(col_len - width);
		}
	}
	else
	{
		buffer = ses->scroll->buffer[index];

		if (buffer->height == 1)
		{
			word_wrap_split(ses, buffer->str, temp, ses->wrap, start, end, 0, &height, &width);

			print_stdout(0, 0, "%s", temp);

			add_line_screen(ses, temp, ses->cur_row);

			erase_cols(col_len - buffer->width);

			goto_pos(ses, ses->cur_row + 1, ses->split->top_col);
		}
		else
		{
			word_wrap_split(ses, buffer->str, temp, ses->wrap, start, end, WRAP_FLAG_SPLIT, &height, &width);

			pti = temp;
			col = 0;

			while (TRUE)
			{
				if (pti[col] == '\n' || pti[col] == 0)
				{
					swap = pti[col];

					pti[col] = 0;

					str_len = strip_vt102_strlen(ses, pti);

					print_stdout(0, 0, "%s", pti);

					add_line_screen(ses, pti, ses->cur_row);

					erase_cols(col_len - str_len);

					pti += col + 1;
					col = 0;

					goto_pos(ses, ses->cur_row + 1, ses->split->top_col);

					if (swap == 0)
					{
						break;
					}
					continue;
				}
				col++;
			}
		}
	}
	pop_call();
	return;
}


int show_buffer(struct session *ses)
{
	int scroll_size, scroll_cnt, scroll_tmp, scroll_add, scroll_cut, start, end, row;

	if (ses != gtd->ses)
	{
		return TRUE;
	}

	push_call("show_buffer(%p)",ses);

	check_buffer(ses);

	scroll_size = get_scroll_rows(ses);
	scroll_add  = 0;
	scroll_cnt  = URANGE(0, ses->scroll->line, ses->scroll->used - 1);
	scroll_cut  = 0;

	if (ses->scroll->base)
	{
		scroll_add -= ses->scroll->base;
	}

	// scroll_cut is cut from top line

	while (TRUE)
	{
		scroll_tmp = ses->scroll->buffer[scroll_cnt]->height;

		if (scroll_add + scroll_tmp >= scroll_size)
		{
			if (scroll_add + scroll_tmp == scroll_size)
			{
				scroll_add += scroll_tmp;
			}
			else
			{
				scroll_cut = scroll_size - scroll_add;
			}
			break;
		}

		scroll_add += scroll_tmp;

		if (scroll_cnt == 0) // home
		{
			erase_scroll_region(ses);

			break;
		}

		scroll_cnt--;
	}

	save_pos(ses);

	if (scroll_cnt == 0) // home
	{
		row = ses->split->bot_row - scroll_add;
	}
	else
	{
		row = ses->split->top_row;
	}
	goto_pos(ses, row, ses->split->top_col);

	if (IS_SPLIT(ses))
	{
		SET_BIT(ses->flags, SES_FLAG_READMUD);
	}

	// scroll_cut is taken from top line
	// scroll_base is taken from the bot line

	if (scroll_cut)
	{
		scroll_tmp = ses->scroll->buffer[scroll_cnt]->height;

		// bottom

		if (scroll_cut == scroll_size)
		{
			start = scroll_tmp - scroll_cut;
			end   = scroll_tmp;

			buffer_print(ses, scroll_cnt, start, end);
		}

		// middle chunk

		else if (scroll_cut > scroll_size)
		{
			start = scroll_tmp - scroll_cut;
			end   = scroll_tmp - scroll_cut + scroll_size;

			buffer_print(ses, scroll_cnt, start, end);

			goto eof;
		}

		// top chunk

		else if (scroll_add == 0)
		{
			start = ses->scroll->base;
			end   = scroll_tmp - scroll_cut;

			buffer_print(ses, scroll_cnt, start, end);

			goto eof;
		}

		// bot chunk

		else
		{
			start = scroll_tmp - scroll_cut;
			end   = scroll_tmp;

			buffer_print(ses, scroll_cnt, start, end);
		}
		scroll_cnt++;
		scroll_cut = 0;
	}

	while (TRUE)
	{
		if (scroll_cnt == ses->scroll->used)
		{
			break;
		}

		scroll_tmp = ses->scroll->buffer[scroll_cnt]->height;

		if (scroll_add - scroll_tmp < 0)
		{
			break;
		}

		scroll_add -= scroll_tmp;

		start = 0;
		end   = scroll_tmp;

		buffer_print(ses, scroll_cnt, start, end);

		scroll_cnt++;
	}

	if (scroll_cnt < ses->scroll->used && ses->scroll->base)
	{
		scroll_tmp = ses->scroll->buffer[scroll_cnt]->height;

		start = 0;
		end   = scroll_tmp - ses->scroll->base;

		buffer_print(ses, scroll_cnt, start, end);
	}

	eof:

	// prompt

	buffer_print(ses, 0, 0, 0);

	restore_pos(ses);

	if (IS_SPLIT(ses))
	{
		DEL_BIT(ses->flags, SES_FLAG_READMUD);
	}

	if (HAS_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLMODE))
	{
		SET_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLUPDATE);
	}

	pop_call();
	return TRUE;
}


DO_COMMAND(do_buffer)
{
	int cnt;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	check_buffer(ses);

	if (*arg1 == 0)
	{
		info:

		tintin_header(ses, 80, " BUFFER OPTIONS ");

		for (cnt = 0 ; *buffer_table[cnt].name != 0 ; cnt++)
		{
			tintin_printf2(ses, "  [%-13s] %s", buffer_table[cnt].name, buffer_table[cnt].desc);
		}
		tintin_header(ses, 80, "");

		return ses;
	}

	for (cnt = 0 ; *buffer_table[cnt].name ; cnt++)
	{
		if (!is_abbrev(arg1, buffer_table[cnt].name))
		{
			continue;
		}

		buffer_table[cnt].fun(ses, arg, arg1, arg2);

		return ses;
	}

	goto info;

	return ses;
}

DO_BUFFER(buffer_clear)
{
	int cnt;

	for (cnt = 1 ; cnt < ses->scroll->used ; cnt++)
	{
		free(ses->scroll->buffer[cnt]->str);
		free(ses->scroll->buffer[cnt]);
	}

	ses->scroll->used  = 1;
	ses->scroll->line = - 1;
}

DO_BUFFER(buffer_up)
{
	int scroll_size;

	check_buffer(ses);

	if (ses->scroll->line == -1)
	{
		ses->scroll->line = ses->scroll->used - 1;
		ses->scroll->base = 0;
	}

	if (*arg)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	}

	if (is_math(ses, arg1))
	{
		scroll_size = URANGE(1, get_number(ses, arg1), get_scroll_rows(ses));
	}
	else
	{
		scroll_size = get_scroll_rows(ses);
	}

	while (scroll_size)
	{
		scroll_size--;
		ses->scroll->base++;

		if (ses->scroll->base == ses->scroll->buffer[ses->scroll->line]->height)
		{
			if (ses->scroll->line == 0)
			{
				ses->scroll->line = 0;
				ses->scroll->base = 0;

				return;
			}
			ses->scroll->line--;
			ses->scroll->base = 0;
		}
	}

	show_buffer(ses);

	return;
}

DO_BUFFER(buffer_down)
{
	int scroll_size;

	if (ses->scroll->line == -1)
	{
		return;
	}

	check_buffer(ses);

	if (*arg)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	}

	if (is_math(ses, arg1))
	{
		scroll_size = URANGE(1, get_number(ses, arg1), get_scroll_rows(ses));
	}
	else
	{
		scroll_size = get_scroll_rows(ses);
	}

	while (scroll_size)
	{
		scroll_size--;

		if (ses->scroll->base == 0)
		{
			if (++ses->scroll->line >= ses->scroll->used)
			{
				buffer_end(ses, "", "", "");
				return;
			}
			ses->scroll->base = ses->scroll->buffer[ses->scroll->line]->height;
		}
		ses->scroll->base--;
	}
	show_buffer(ses);
}

DO_BUFFER(buffer_home)
{
	check_buffer(ses);

	ses->scroll->line = 0;
	ses->scroll->base = 0;

	buffer_down(ses, "", "", "");
}

DO_BUFFER(buffer_end)
{
	check_buffer(ses);

	ses->scroll->line = ses->scroll->used - 1;

	ses->scroll->base = 0;

	show_buffer(ses);

	ses->scroll->line = -1;
}

DO_BUFFER(buffer_jump)
{
	int line;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	line = get_number(ses, arg1);

	if (line >= 0)
	{
		line = URANGE(0, line, ses->scroll->used - 1);
	}
	else
	{
		line = URANGE(0, ses->scroll->used + line, ses->scroll->used - 1);
	}

	if (line >= ses->scroll->used - 1)
	{
		buffer_end(ses, "", "", "");

		return;
	}

	if (line != ses->scroll->line)
	{
		ses->scroll->line = line;

		ses->scroll->base = 0;

		show_buffer(ses);

		update_scrollbar(ses);
	}
}

DO_BUFFER(buffer_lock)
{
	check_buffer(ses);

	sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (!strcasecmp(arg1, "ON"))
	{
		ses->scroll->line = ses->scroll->used + 1;
	}
	else if (!strcasecmp(arg1, "OFF"))
	{
		 buffer_end(ses, "", "", "");
	}
	else
	{
		if (ses->scroll->line == -1)
		{
			ses->scroll->line = ses->scroll->used + 1;
		}
		else
		{
			buffer_end(ses, "", "", "");
		}
	}
}

DO_BUFFER(buffer_find)
{
	int scroll_cnt, grep_cnt, grep_max, page;

	check_buffer(ses);

	grep_cnt = grep_max = scroll_cnt = 0;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(gtd->ses, LIST_COMMAND, "#BUFFER, FIND WHAT?");

		return;
	}

	if (is_math(ses, arg1))
	{
		page = get_number(ses, arg1);

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

		strcpy(arg2, arg1);
	}

	if (page > 0)
	{
		for (scroll_cnt = ses->scroll->used - 1; scroll_cnt >= 0 ; scroll_cnt--)
		{
			if (HAS_BIT(ses->scroll->buffer[scroll_cnt]->flags, BUFFER_FLAG_GREP))
			{
				continue;
			}

			if (find(ses, ses->scroll->buffer[scroll_cnt]->str, arg2, SUB_NONE, REGEX_FLAG_NONE))
			{
				grep_cnt++;

				if (grep_cnt == page)
				{
					break;
				}
			}
		}
	}
	else
	{
		for (scroll_cnt = 0 ; scroll_cnt < ses->scroll->used ; scroll_cnt++)
		{
			if (HAS_BIT(ses->scroll->buffer[scroll_cnt]->flags, BUFFER_FLAG_GREP))
			{
				continue;
			}

			if (find(ses, ses->scroll->buffer[scroll_cnt]->str, arg2, SUB_NONE, REGEX_FLAG_NONE))
			{
				grep_cnt--;

				if (grep_cnt == page)
				{
					break;
				}

			}
		}
	}

	if (scroll_cnt < 0 || scroll_cnt >= ses->scroll->used)
	{
		show_error(gtd->ses, LIST_COMMAND, "#BUFFER FIND, NO MATCHES FOUND.");

		return;
	}

	ses->scroll->line = scroll_cnt;

	buffer_down(ses, ntos(get_scroll_rows(ses) - 1), arg1, arg2);

	return;
}

DO_BUFFER(buffer_get)
{
	int cnt, min, max;

	check_buffer(ses);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_NONE);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #BUFFER GET <VARIABLE> [LOWER BOUND] [UPPER BOUND]");

		return;
	}

	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	min = get_number(ses, arg2);

	if (min < 0)
	{
		min = ses->scroll->used + min;
	}
	min = URANGE(0, min, ses->scroll->used - 1);

	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg2 == 0)
	{
		set_nest_node_ses(ses, arg1, "%s", ses->scroll->buffer[min]->str);

		return;
	}

	max = get_number(ses, arg2);

	if (max < 0)
	{
		max = ses->scroll->used + max;
	}
	max = URANGE(0, max, ses->scroll->used - 1);

	if (min > max)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #BUFFER GET {%s} {%d} {%d} LOWER BOUND EXCEEDS UPPER BOUND.", arg1, min, max);

		return;
	}

	cnt = 0;

	set_nest_node_ses(ses, arg1, "");

	while (min <= max)
	{
		sprintf(arg2, "%s[%d]", arg1, ++cnt);

		set_nest_node_ses(ses, arg2, "%s", ses->scroll->buffer[min++]->str);
	}

	show_message(ses, LIST_COMMAND, "#BUFFER GET: %d LINES SAVED TO {%s}.", cnt, arg1);

	return;
}

DO_BUFFER(buffer_refresh)
{
	check_buffer(ses);

	if (HAS_BIT(ses->flags, SES_FLAG_PRINTLINE) && ses->check_output == 0)
	{
		if (ses == gtd->ses)
		{
			DEL_BIT(ses->flags, SES_FLAG_PRINTLINE);

			SET_BIT(ses->flags, SES_FLAG_PRINTBUFFER);

			print_scroll_region(ses);

			DEL_BIT(ses->flags, SES_FLAG_PRINTBUFFER);

			fflush(stdout);
		}
		return;
	}

	if (ses->scroll->line == -1)
	{
		buffer_end(ses, "", "", "");
	}
	else
	{
		show_buffer(ses);
	}
	return;
}

DO_BUFFER(buffer_write)
{
	FILE *fp;
	int cnt;

	check_buffer(ses);

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

			loginit(ses, fp, ses->logmode + LOG_FLAG_OVERWRITE);

			for (cnt = 0 ; cnt < ses->scroll->used ; cnt++)
			{
				if (HAS_BIT(ses->logmode, LOG_FLAG_PLAIN))
				{
					strip_vt102_codes(ses->scroll->buffer[cnt]->str, arg2);
				}
				else if (HAS_BIT(ses->logmode, LOG_FLAG_HTML))
				{
					vt102_to_html(ses, ses->scroll->buffer[cnt]->str, arg2);
				}
				else
				{
					strcpy(arg2, ses->scroll->buffer[cnt]->str);
				}
				fprintf(fp, "%s\n", arg2);
			}

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
	int index, memory;

	check_buffer(ses);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);	
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_printf2(ses, "#BUFFER INFO: BASE   = %d", ses->scroll->base);
		tintin_printf2(ses, "#BUFFER INFO: LINE   = %d", ses->scroll->line);
		tintin_printf2(ses, "#BUFFER INFO: SIZE   = %d", ses->scroll->size);
		tintin_printf2(ses, "#BUFFER INFO: USED   = %d", ses->scroll->used);
		tintin_printf2(ses, "#BUFFER INFO: WIDTH  = %d", ses->scroll->width);
		tintin_printf2(ses, "#BUFFER INFO: WRAP   = %d", ses->scroll->wrap);

		tintin_printf2(ses, "");

		memory = 0;

		for (index = 0 ; index < ses->scroll->used ; index++)
		{
			memory += sizeof(struct buffer_data) + strlen(ses->scroll->buffer[index]->str);
		}
		tintin_printf2(ses, "#BUFFER INFO: MEMORY = %d", memory);
	}
	else if (is_abbrev(arg1, "SAVE") && *arg2)
	{
		add_nest_node_ses(ses, arg2, "{BASE}{%d}", ses->scroll->base);
		add_nest_node_ses(ses, arg2, "{LINE}{%d}", ses->scroll->line);
		add_nest_node_ses(ses, arg2, "{SIZE}{%d}", ses->scroll->size);
		add_nest_node_ses(ses, arg2, "{USED}{%d}", ses->scroll->used);
		add_nest_node_ses(ses, arg2, "{WRAP}{%d}", ses->scroll->wrap);

		show_message(ses, LIST_COMMAND, "#BUFFER INFO: SAVED TO {%s}", arg2);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #BUFFER INFO [SAVE] [<VARIABLE>]");
	}
}


DO_COMMAND(do_grep)
{
	int scroll_cnt, grep_cnt, grep_min, grep_max, grep_add, page;

	check_buffer(ses);

	grep_cnt = grep_add = scroll_cnt = grep_min = 0;
	grep_max = ses->split->bot_row - ses->split->top_row - 2;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: GREP [#] <SEARCH TEXT>");

		return ses;
	}

	if (is_math(ses, arg1))
	{
		page = get_number(ses, arg1);

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

		strcpy(arg2, arg1);
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

	gtd->level->grep++;

	tintin_header(ses, 80, " GREPPING PAGE %d FOR %s ", page, arg2);

	if (page > 0)
	{
		for (scroll_cnt = ses->scroll->used - 1 ; scroll_cnt > 0 ; scroll_cnt--)
		{
			if (HAS_BIT(ses->scroll->buffer[scroll_cnt]->flags, BUFFER_FLAG_GREP))
			{
				continue;
			}

			if (find(ses, ses->scroll->buffer[scroll_cnt]->str, arg2, SUB_NONE, REGEX_FLAG_NONE))
			{
				grep_add = ses->scroll->buffer[scroll_cnt]->height;

				if (grep_cnt + grep_add > grep_max)
				{
					break;
				}

				grep_cnt += grep_add;
			}
		}

		if (grep_cnt <= grep_min)
		{
			show_error(ses, LIST_COMMAND, "#NO MATCHES FOUND.");

			gtd->level->grep--;

			return ses;
		}

		while (++scroll_cnt < ses->scroll->used)
		{
			if (HAS_BIT(ses->scroll->buffer[scroll_cnt]->flags, BUFFER_FLAG_GREP))
			{
				continue;
			}

			if (find(ses, ses->scroll->buffer[scroll_cnt]->str, arg2, SUB_NONE, REGEX_FLAG_NONE))
			{
				grep_add = ses->scroll->buffer[scroll_cnt]->height;

				if (grep_cnt - grep_add < grep_min)
				{
					break;
				}

				grep_cnt -= grep_add;

				tintin_puts2(ses, ses->scroll->buffer[scroll_cnt]->str);
			}
		}
	}
	else
	{
		for (scroll_cnt = 0 ; scroll_cnt < ses->scroll->used ; scroll_cnt++)
		{
			if (HAS_BIT(ses->scroll->buffer[scroll_cnt]->flags, BUFFER_FLAG_GREP))
			{
				continue;
			}

			if (find(ses, ses->scroll->buffer[scroll_cnt]->str, arg2, SUB_NONE, REGEX_FLAG_NONE))
			{
				grep_add = ses->scroll->buffer[scroll_cnt]->height;

				if (grep_cnt + grep_add >= grep_min)
				{
					grep_cnt += grep_add;
					
					tintin_puts2(ses, ses->scroll->buffer[scroll_cnt]->str);

					if (grep_cnt + grep_add > grep_max)
					{
						break;
					}
				}
			}
		}

		if (grep_cnt == 0)
		{
			show_error(ses, LIST_COMMAND, "#NO MATCHES FOUND.");

			gtd->level->grep--;

			return ses;
		}
	}
	tintin_header(ses, 80, "");

	gtd->level->grep--;

	return ses;
}
