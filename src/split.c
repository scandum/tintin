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
*                          coded by Bill Reiss 1993                           *
*                     recoded by Igor van den Hoven 2004                      *
******************************************************************************/

#include "tintin.h"

DO_COMMAND(do_split)
{
	int input;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if ((*arg1 && !is_math(ses, arg1)) || (*arg2 && !is_math(ses, arg2)))
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SPLIT [TOP BAR] [BOTTOM BAR] [LEFT BAR] [RIGHT BAR] [INPUT BAR]");

		return ses;
	}

	ses->split->sav_top_row = *arg1 ? get_number(ses, arg1) : 0;
	ses->split->sav_bot_row = *arg2 ? get_number(ses, arg2) : 1;

	if (*arg == 0)
	{
		ses->split->sav_top_col = 0;
		ses->split->sav_bot_col = 0;
	}
	else
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

		if ((*arg1 && !is_math(ses, arg1)) || (*arg2 && !is_math(ses, arg2)))
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #SPLIT {TOP BAR} {BOT BAR} {LEFT BAR} {RIGHT BAR}");

			return ses;
		}

		ses->split->sav_top_col = *arg1 ? get_number(ses, arg1) : 1;
		ses->split->sav_bot_col = *arg2 ? get_number(ses, arg2) : 0;
	}

	if (*arg)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		
		input = get_number(ses, arg1);

		input *= -1;
	}
	else
	{
		input = -1;
	}

	init_input(ses, input, 1, -1, -1);

	ses->split->sav_bot_row += inputline_rows(ses) - 1;

	DEL_BIT(ses->flags, SES_FLAG_SCROLLSPLIT);
	SET_BIT(ses->flags, SES_FLAG_SPLIT);

	ses->input->str_off = 1;

	init_split(ses, ses->split->sav_top_row, ses->split->sav_top_col, ses->split->sav_bot_row, ses->split->sav_bot_col);

	return ses;
}


DO_COMMAND(do_unsplit)
{
	memset(ses->split, 0, sizeof(struct split_data));

	ses->wrap = gtd->screen->cols;

	reset_screen(ses);

	SET_BIT(ses->scroll->flags, SCROLL_FLAG_RESIZE);

	if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
	{
		if (HAS_BIT(ses->telopts, TELOPT_FLAG_NAWS))
		{
			client_send_sb_naws(ses, 0, NULL);
		}
		DEL_BIT(ses->flags, SES_FLAG_SPLIT);
		DEL_BIT(ses->flags, SES_FLAG_SCROLLSPLIT);
	}
	check_all_events(ses, EVENT_FLAG_SCREEN, 0, 4, "SCREEN UNSPLIT", ntos(ses->split->top_row), ntos(ses->split->top_col), ntos(ses->split->bot_row), ntos(ses->split->bot_col));
	return ses;
}

void init_split(struct session *ses, int top_row, int top_col, int bot_row, int bot_col)
{
	push_call("init_split(%p,%d,%d,%d,%d)",ses,top_row,top_col,bot_row,bot_col);

	SET_BIT(ses->scroll->flags, SCROLL_FLAG_RESIZE);

	init_input(ses, ses->input->sav_top_row, ses->input->sav_top_col, ses->input->sav_bot_row, ses->input->sav_bot_col);

	if (!HAS_BIT(ses->flags, SES_FLAG_SPLIT))
	{
		ses->split->top_row = 1;
		ses->split->top_col = 1;
		ses->split->bot_row = gtd->screen->rows;
		ses->split->bot_col = gtd->screen->cols;
		ses->wrap = gtd->screen->cols;

		init_pos(ses, gtd->screen->rows, 1);

		if (ses->map && HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP))
		{
			SET_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);
		}

		pop_call();
		return;
	}

	if (HAS_BIT(ses->flags, SES_FLAG_SCROLLSPLIT))
	{
		ses->split->top_row = top_row > 0 ? top_row : top_row < 0 ? gtd->screen->rows + top_row + 1 : 1;
		ses->split->top_col = top_col > 0 ? top_col : top_col < 0 ? gtd->screen->cols + top_col + 1 : 1;
		ses->split->bot_row = bot_row > 0 ? bot_row : bot_row < 0 ? gtd->screen->rows + bot_row + 1 : gtd->screen->rows - 2;
		ses->split->bot_col = bot_col > 0 ? bot_col : bot_col < 0 ? gtd->screen->cols + bot_col + 1 : gtd->screen->cols;
	}
	else
	{
		ses->split->top_row = top_row > 0 ? top_row + 1 : top_row < 0 ? gtd->screen->rows + top_row + 1 : 1;
		ses->split->top_col = top_col > 0 ? top_col + 1 : top_col < 0 ? gtd->screen->cols + top_col + 1 : 1;
		ses->split->bot_row = bot_row > 0 ? gtd->screen->rows - bot_row - 1 : bot_row < 0 ? bot_row * -1 : gtd->screen->rows - 1;
		ses->split->bot_col = bot_col > 0 ? gtd->screen->cols - bot_col : bot_col < 0 ? bot_col * -1 : gtd->screen->cols;
	}

	ses->split->top_row = URANGE(1, ses->split->top_row, gtd->screen->rows -3);
	ses->split->top_col = URANGE(1, ses->split->top_col, gtd->screen->cols - 2);
	ses->split->bot_row = URANGE(ses->split->top_row + 1,  ses->split->bot_row, gtd->screen->rows - 1);
	ses->split->bot_col = URANGE(ses->split->top_col + 1, ses->split->bot_col, gtd->screen->cols);

	ses->split->sav_top_row = ses->split->top_row - 1;
	ses->split->sav_top_col = ses->split->top_col - 1;
	ses->split->sav_bot_row = gtd->screen->rows - ses->split->bot_row - 1;
	ses->split->sav_bot_col = gtd->screen->cols - ses->split->bot_col;

	ses->wrap = ses->split->bot_col - (ses->split->top_col - 1);

	scroll_region(ses, ses->split->top_row, ses->split->bot_row);

	init_pos(ses, ses->input->top_row, ses->input->top_col);

	if (HAS_BIT(ses->telopts, TELOPT_FLAG_NAWS))
	{
		SET_BIT(ses->telopts, TELOPT_FLAG_UPDATENAWS);
	}

	if (ses->map && HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP))
	{
		SET_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);
	}

	check_all_events(ses, EVENT_FLAG_SCREEN, 0, 4, "SCREEN SPLIT FILL", ntos(ses->split->top_row), ntos(ses->split->top_col), ntos(ses->split->bot_row), ntos(ses->split->bot_col), ntos(ses->split->sav_top_row), ntos(ses->split->sav_bot_row), ntos(ses->split->sav_top_col), ntos(ses->split->sav_bot_col));

	if (!check_all_events(ses, EVENT_FLAG_CATCH, 0, 8, "CATCH SCREEN SPLIT FILL", ntos(ses->split->top_row), ntos(ses->split->top_col), ntos(ses->split->bot_row), ntos(ses->split->bot_col), ntos(ses->split->sav_top_row), ntos(ses->split->sav_bot_row), ntos(ses->split->sav_top_col), ntos(ses->split->sav_bot_col)))
	{
		if (ses == gtd->ses && !HAS_BIT(ses->flags, SES_FLAG_SCROLLSPLIT))
		{
			if (HAS_BIT(ses->config_flags, CONFIG_FLAG_VERBOSE) || gtd->level->verbose || gtd->level->quiet == 0)
			{
				command(ses, do_screen, "FILL DEFAULT");
			}
		}

	}

	check_all_events(ses, EVENT_FLAG_SCREEN, 0, 4, "SCREEN SPLIT", ntos(ses->split->top_row), ntos(ses->split->top_col), ntos(ses->split->bot_row), ntos(ses->split->bot_col));

	pop_call();
	return;
}


/*
	unsplit
*/

void reset_screen(struct session *ses)
{
	reset_scroll_region(ses);

	init_pos(ses, gtd->screen->rows, 1);
}


/*
	refresh
*/

void dirty_screen(struct session *ses)
{
	push_call("dirty_screen(%p)",ses);

	refresh_session_terminal(ses);

	print_stdout(0, 0, "\e=");

	if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
	{
		init_split(ses, ses->split->sav_top_row, ses->split->sav_top_col, ses->split->sav_bot_row, ses->split->sav_bot_col);

		init_input(ses, ses->input->sav_top_row, ses->input->sav_top_col, ses->input->sav_bot_row, ses->input->sav_bot_col);
	}
	else if (IS_SPLIT(ses))
	{
		scroll_region(ses, ses->split->top_row, ses->split->bot_row);
	}
	else
	{
		reset_screen(ses);
	}

	if (IS_SPLIT(ses) && ses == gtd->ses)
	{
		init_pos(ses, ses->input->top_row, ses->input->top_col);
	}

	pop_call();
	return;
}


void split_show(struct session *ses, char *prompt, char *row_str, char *col_str)
{
	char buf1[BUFFER_SIZE];
	int row, col, len, width, clear;

	row = 0;

	if (*row_str)
	{
		row = get_row_index_arg(ses, row_str);
	}

	if (row == 0)
	{
		row = URANGE(1, ses->input->top_row - 1, gtd->screen->rows);
	}

	col = 0;

	if (*col_str)
	{
		col = get_col_index_arg(ses, col_str);
	}

	if (col == 0)
	{
		col = 1;
		clear = 1;
	}
	else
	{
		clear = 0;
	}

	if (row < 1 || row > gtd->screen->rows)
	{
		show_error(ses, LIST_PROMPT, "#ERROR: PROMPT ROW IS OUTSIDE THE SCREEN: {%s} {%s} {%s} [%d].", prompt, row_str, col_str, row);

		return;
	}

	if (col < 0 || col > gtd->screen->cols)
	{
		show_error(ses, LIST_PROMPT, "#ERROR: PROMPT COLUMN IS OUTSIDE THE SCREEN: {%s} {%s} {%s} [%d].", prompt, row_str, col_str, col);

		return;
	}

	if (inside_scroll_region(ses, row, col))
	{
		show_error(ses, LIST_PROMPT, "#ERROR: PROMPT ROW IS INSIDE THE SCROLLING REGION: {%s} {%s} [%d].", prompt, row_str, row);

		return;
	}

	if (ses != gtd->ses)
	{
		return;
	}

	len = strip_vt102_width(ses, prompt, &width);

	if (col - 1 + width <= gtd->screen->cols)
	{
		sprintf(buf1, "%s", prompt);
	}
	else
	{
		show_debug(ses, LIST_PROMPT, "#DEBUG PROMPT {%s}", prompt);

		show_debug(ses, LIST_PROMPT, "#PROMPT WIDTH %d WITH OFFSET %d LONGER THAN ROW SIZE %d.", width, col, gtd->screen->cols);

		sprintf(buf1, "#PROMPT WIDTH %d WITH OFFSET %d LONGER THAN ROW SIZE %d.", len, col, gtd->screen->cols);
	}

	save_pos(ses);

	if (row == gtd->screen->rows)
	{
		gtd->ses->input->str_off = len + 1;

		goto_pos(ses, row, col);

		print_stdout(0, 0, "%s%s", buf1, gtd->ses->input->buf);

		// bit of a hack

		gtd->screen->sav_col[0] = inputline_cur_col();
		gtd->screen->sav_row[0] = inputline_cur_row();
	}
	else
	{
		goto_pos(ses, row, col);

		if (clear)
		{
			erase_cols(gtd->screen->cols);
		}
	
		print_stdout(0, 0, "%s", buf1);
	}

	set_line_screen(ses, buf1, row, col);

	restore_pos(ses);
}
