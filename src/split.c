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
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int top, bot;

	if (arg == NULL)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SPLIT {[TOP ROWS]} {[BOTTOM ROWS]}");

		return ses;
	}

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		if (*arg2 == 0)
		{
			top = 0;
			bot = 1;
		}
		else if (is_math(ses, arg2))
		{
			top = 0;
			bot = get_number(ses, arg2);
		}
		else
		{
			return do_split(ses, NULL);
		}
	}
	else if (*arg2 == 0)
	{
		if (is_math(ses, arg1))
		{
			top = 0;
			bot = get_number(ses, arg1);
		}
		else
		{
			return do_split(ses, NULL);
		}
	}
	else if (!is_math(ses, arg1) || !is_math(ses, arg2))
	{
		return do_split(ses, NULL);
	}
	else
	{
		top = get_number(ses, arg1);
		bot = get_number(ses, arg2);
	}

	if (top < 0 || bot < 0)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: NEGATIVE VALUE(S): #SPLIT {%d} {%d}", top, bot);

		return ses;
	}

	ses->top_split = top;
	ses->bot_split = bot;

	init_split(ses, 1 + ses->top_split, gtd->screen->rows - 1 - ses->bot_split);

	return ses;
}


DO_COMMAND(do_unsplit)
{
	reset_screen(ses);

	if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
	{
		if (HAS_BIT(ses->telopts, TELOPT_FLAG_NAWS))
		{
			client_send_sb_naws(ses, 0, NULL);
		}
		DEL_BIT(ses->flags, SES_FLAG_SPLIT);
	}
	return ses;
}

void init_split(struct session *ses, int top, int bot)
{
	push_call("init_split(%p,%d,%d)",ses,top,bot);

	if (bot > gtd->screen->rows)
	{
		bot = gtd->screen->rows;
	}

	if (bot < 2)
	{
		bot = 2;
	}

	if (top >= bot)
	{
		top = bot - 1;
	}

	if (top < 1)
	{
		top = 1;
	}

	scroll_region(ses, top, bot);

	SET_BIT(ses->flags, SES_FLAG_SPLIT);

	for (bot = 1 ; gtd->screen->rows - bot > ses->bot_row ; bot++)
	{
		split_show(ses, "", gtd->screen->rows - bot, 0);
	}

	set_line_screen("", ses->bot_row - 1, 0);

	for (top = 1 ; top < ses->top_row ; top++)
	{
		split_show(ses, "", top, 0);
	}

	goto_rowcol(ses, gtd->screen->rows, 1);

	if (HAS_BIT(ses->telopts, TELOPT_FLAG_NAWS))
	{
		client_send_sb_naws(ses, 0, NULL);
	}

	if (ses->map && HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP))
	{
		SET_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);
	}

	buffer_end(ses, "");

	pop_call();
	return;
}


/*
	unsplit
*/

void reset_screen(struct session *ses)
{
	reset_scroll_region(ses);

	if (ses == gtd->ses)
	{
		goto_rowcol(ses, gtd->screen->rows, 1);
	}
}


/*
	refresh
*/

void dirty_screen(struct session *ses)
{
	push_call("dirty_screen(%p)",ses);

	refresh_session_terminal(ses);

	printf("\e=");

	if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
	{
		init_split(ses, 1 + ses->top_split, gtd->screen->rows - 1 - ses->bot_split);
	}
	else if (IS_SPLIT(ses))
	{
		scroll_region(ses, ses->top_row, ses->bot_row);
	}
	else
	{
		reset_screen(ses);
	}

	if (IS_SPLIT(ses) && ses == gtd->ses)
	{
		goto_rowcol(ses, gtd->screen->rows, 1);
	}

//	buffer_end(ses, "");

	pop_call();
	return;
}


void split_show(struct session *ses, char *prompt, int row, int col)
{
	char temp[BUFFER_SIZE];
	int original_row, original_col, len, clear;

	original_row = row;
	original_col = col;

	if (row < 0)
	{
		row = 1 + gtd->screen->rows + row;
	}
	else if (row == 0)
	{
		row = gtd->screen->rows - 2;
	}

	clear = 0;

	if (col < 0)
	{
		col = 1 + gtd->screen->cols + col;
	}
	else if (col == 0)
	{
		col = 1;
		clear = 1;
	}

	if (row < 1 || row > gtd->screen->rows)
	{
		show_error(ses, LIST_PROMPT, "#ERROR: PROMPT ROW IS OUTSIDE THE SCREEN: {%s} {%d} {%d}.", prompt, original_row, original_col);

		return;
	}

	if (col < 0 || col > gtd->screen->cols)
	{
		show_error(ses, LIST_PROMPT, "#ERROR: PROMPT COLUMN IS OUTSIDE THE SCREEN: {%s} {%d} {%d}.", prompt, original_row, original_col);

		return;
	}

	if (row > ses->top_row && row < ses->bot_row)
	{
		show_error(ses, LIST_PROMPT, "#ERROR: PROMPT ROW IS INSIDE THE SCROLLING REGION: {%s} {%d}.", prompt, original_row);

		return;
	}

	if (ses != gtd->ses)
	{
		return;
	}

	len = strip_vt102_strlen(ses, prompt);

	if (len == 0)
	{
		sprintf(temp, "%.*s", gtd->screen->cols + 4, "\e[0m--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------");
	}
	else if (col - 1 + len <= gtd->screen->cols)
	{
		sprintf(temp, "%s", prompt);
	}
	else
	{
		show_debug(ses, LIST_PROMPT, "#DEBUG PROMPT {%s}", prompt);

		sprintf(temp, "#PROMPT SIZE (%d) LONGER THAN ROW SIZE (%d)", len, gtd->screen->cols);
	}

	if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
	{
		save_pos(ses);
	}

	if (row == gtd->screen->rows)
	{
		gtd->input_off = len + 1;

		printf("\e[%d;1H\e[%d;1H\e[K%s%s\e[%d;%dH\e7\e[%d;1H", row, row, temp, gtd->input_buf, row, gtd->input_off + gtd->input_cur, ses->bot_row);
	}
	else
	{
		printf("\e[%d;%dH\e[%d;%dH%s%s\e[%d;1H", row, col, row, col, clear ? "\e[2K" : "", temp, ses->bot_row);
	}

	set_line_screen(temp, row - 1, col - 1);

	if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
	{
		restore_pos(ses);
	}

}
