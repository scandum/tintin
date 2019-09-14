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
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"


DO_COMMAND(do_prompt)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE], arg4[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);


	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_PROMPT], 0);
	}
	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_PROMPT]) == FALSE)
		{
			show_message(ses, LIST_PROMPT, "#PROMPT: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg4, GET_ONE, SUB_VAR|SUB_FUN);

		update_node_list(ses->list[LIST_PROMPT], arg1, arg2, arg3, arg4);

		show_message(ses, LIST_PROMPT, "#OK. {%s} NOW PROMPTS {%s} @ {%s} {%s}.", arg1, arg2, arg3, arg4);
	}
	return ses;
}


DO_COMMAND(do_unprompt)
{
	delete_node_with_wild(ses, LIST_PROMPT, arg);

	return ses;
}


void check_all_prompts(struct session *ses, char *original, char *line)
{
	struct listroot *root = ses->list[LIST_PROMPT];
	struct listnode *node;

	if (!HAS_BIT(ses->flags, SES_FLAG_SPLIT))
	{
		return;
	}

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		if (check_one_regexp(ses, root->list[root->update], line, original, 0))
		{
			node = root->list[root->update];

			if (*node->arg2)
			{
				substitute(ses, node->arg2, original, SUB_ARG);
				substitute(ses, original, original, SUB_VAR|SUB_FUN|SUB_COL|SUB_ESC);
			}

			show_debug(ses, LIST_PROMPT, "#DEBUG PROMPT {%s}", node->arg1);
			show_debug(ses, LIST_GAG, "#DEBUG GAG {%s}", node->arg1);

			do_one_prompt(ses, original, atoi(node->arg3), atoi(node->arg4));

			SET_BIT(ses->flags, SES_FLAG_GAG);
		}
	}
}

void do_one_prompt(struct session *ses, char *prompt, int row, int col)
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
