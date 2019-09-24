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
*               (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                  *
*                                                                             *
*                     coded by Igor van den Hoven 2019                        *
******************************************************************************/

#include "tintin.h"

DO_COMMAND(do_draw)
{
	char *nst_arg;
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE], arg4[BUFFER_SIZE];
	int index, flags;
	int top_row, top_col, bot_row, bot_col, rows, cols;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_header(ses, " DRAW OPTIONS ");

		for (index = 0 ; *draw_table[index].fun ; index++)
		{
			if (*draw_table[index].name)
			{
				tintin_printf2(ses, "  [%-24s] %s", draw_table[index].name, draw_table[index].desc);
			}
		}
		tintin_header(ses, "");
	}
	else
	{
		flags = HAS_BIT(ses->flags, SES_FLAG_UTF8) ? DRAW_FLAG_UTF8 : 0;

		nst_arg = get_arg_in_braces(ses, arg1, arg2, GET_ONE);

/*		if (is_abbrev(arg2, "ESCAPED"))
		{
			SET_BIT(flags, DRAW_FLAG_ESCAPED);
		}
		else */if (is_abbrev(arg2, "PRUNED"))
		{
			SET_BIT(flags, DRAW_FLAG_PRUNED);
		}
		else if (is_abbrev(arg2, "ROUNDED"))
		{
			SET_BIT(flags, DRAW_FLAG_ROUNDED);
		}
		else if (is_abbrev(arg2, "CROSSED"))
		{
			SET_BIT(flags, DRAW_FLAG_CROSSED);
		}

		if (HAS_BIT(flags, DRAW_FLAG_PRUNED|DRAW_FLAG_ROUNDED|DRAW_FLAG_CROSSED))
		{
			if (*nst_arg)
			{
				get_arg_in_braces(ses, nst_arg, arg1, GET_ALL);
			}
			else
			{
				arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
			}
		}

		for (index = 0 ; *draw_table[index].name ; index++)
		{
			if (is_abbrev(arg1, draw_table[index].name))
			{
				arg = sub_arg_in_braces(ses, arg, arg4, GET_ALL, SUB_VAR|SUB_FUN);

				nst_arg = arg4;

				nst_arg = get_arg_in_braces(ses, nst_arg, arg2, GET_ONE);
				nst_arg = get_arg_in_braces(ses, nst_arg, arg3, GET_ONE);

				top_row = get_row_index(ses, arg2);
				top_col = get_col_index(ses, arg3);

				nst_arg = get_arg_in_braces(ses, nst_arg, arg2, GET_ONE);
				nst_arg = get_arg_in_braces(ses, nst_arg, arg3, GET_ONE);

				bot_row = get_row_index(ses, arg2);
				bot_col = get_col_index(ses, arg3);

				rows = URANGE(1, 1 + bot_row - top_row, gtd->screen->rows);
				cols = URANGE(1, 1 + bot_col - top_col, gtd->screen->cols);

				if (*arg == 0)
				{
					arg = nst_arg;
				}

				*arg1 = 0;
				*arg2 = 0;

				save_pos(ses);

				draw_table[index].fun(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, arg, arg1, arg2);

				load_pos(ses);

				return ses;
			}
		}
		show_error(ses, LIST_COMMAND, "#ERROR: #DRAW {%s} IS NOT A VALID OPTION.", capitalize(arg1));
	}

	return ses;
}

// utilities

char *draw_corner(int flags, char *str)
{
	static char result[10][10];
	static int cnt;

	cnt = (cnt + 1) % 10;

	if (HAS_BIT(flags, DRAW_FLAG_UTF8))
	{
		if (HAS_BIT(flags, DRAW_FLAG_PRUNED))
		{
			strcpy(result[cnt], "\e[C");
		}
		else if (HAS_BIT(flags, DRAW_FLAG_ROUNDED))
		{
			strcpy(result[cnt], "○");
		}
		else if (HAS_BIT(flags, DRAW_FLAG_CROSSED))
		{
			strcpy(result[cnt], "┼");
		}
		else
		{
			strcpy(result[cnt], str);
		}
	}
	else
	{
		if (HAS_BIT(flags, DRAW_FLAG_PRUNED))
		{
			strcpy(result[cnt], "\e[C");
		}
		else if (HAS_BIT(flags, DRAW_FLAG_ROUNDED))
		{
			strcpy(result[cnt], "o");
		}
		else if (HAS_BIT(flags, DRAW_FLAG_CROSSED))
		{
			strcpy(result[cnt], "+");
		}
		else
		{
			strcpy(result[cnt], "+");
		}
	}
	return result[cnt];
}

char *draw_horizontal(int flags, char *str)
{
	static char result[10][10];
	static int cnt;

	cnt = (cnt + 1) % 10;

	if (HAS_BIT(flags, DRAW_FLAG_UTF8))
	{
		strcpy(result[cnt], "─");
	}
	else
	{
		strcpy(result[cnt], "-");
	}
	return result[cnt];
}

char *draw_vertical(int flags, char *str)
{
	static char result[10][10];
	static int cnt;

	cnt = (cnt + 1) % 10;

	if (HAS_BIT(flags, DRAW_FLAG_UTF8))
	{
		strcpy(result[cnt], "│");
	}
	else
	{
		strcpy(result[cnt], "|");
	}
	return result[cnt];
}

// subcommands

DO_DRAW(draw_blank)
{
	int row;

	arg = arg1;

	arg1 += sprintf(arg1, "\e[%d;%dH", top_row, top_col);

	for (row = 0 ; row < rows ; row++)
	{
		arg1 += sprintf(arg1, "\e[%dX\v", bot_col - top_col + 1);
	}

	printf("%s", arg);
}


DO_DRAW(draw_bot_line)
{
	int col;

	goto_pos(ses, bot_row, top_col);

	arg = arg1;

	arg1 += sprintf(arg1, "%s", draw_corner(flags, "└"));

	for (col = top_col + 1 ; col < bot_col ; col++)
	{
		arg1 += sprintf(arg1, "%s", draw_horizontal(flags, "─"));
	}

	arg1 += sprintf(arg1, "%s", draw_corner(flags, "┘"));

	printf("%s", arg);
}

DO_DRAW(draw_box)
{
	draw_top_line(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, arg, arg1, arg2);
	draw_bot_line(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, arg, arg1, arg2);

	draw_left_line (ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, arg, arg1, arg2);
	draw_right_line(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, arg, arg1, arg2);
}

DO_DRAW(draw_box_text)
{
	draw_box (ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, arg, arg1, arg2);

	draw_text(ses, top_row + 1, top_col + 1, bot_row - 1, bot_col - 1, rows - 2, cols - 2, flags, arg, arg1, arg2);
}

DO_DRAW(draw_center_left_line)
{
	int row = top_row;

	arg = arg1;

	arg1 += sprintf(arg1, "\e[%d;%dH%s", top_row, top_col, draw_corner(flags, "┬"));

	while (++row < bot_row)
	{
		arg1 += sprintf(arg1, "\e[%d;%dH%s", row, top_col, draw_vertical(flags, "│"));
	}

	arg1 += sprintf(arg1, "\e[%d;%dH%s", bot_row, top_col, draw_corner(flags, "┴"));

	printf("%s", arg);
}

DO_DRAW(draw_center_right_line)
{
	int row = top_row;

	arg = arg1;

	arg1 += sprintf(arg1, "\e[%d;%dH%s", top_row, bot_col, draw_corner(flags, "┬"));

	while (++row < bot_row)
	{
		arg1 += sprintf(arg1, "\e[%d;%dH%s", row, bot_col, draw_vertical(flags, "│"));
	}

	arg1 += sprintf(arg1, "\e[%d;%dH%s", bot_row, bot_col, draw_corner(flags, "┴"));

	printf("%s", arg);
}

DO_DRAW(draw_horizontal_line)
{
	int col;

	arg = arg1;

	arg1 += sprintf(arg1, "\e[%d;%dH%s", top_row, top_col, draw_horizontal(flags, "─"));

	for (col = top_col + 1 ; col <= bot_col ; col++)
	{
		arg1 += sprintf(arg1, "%s", draw_horizontal(flags, "─"));
	}

	printf("%s", arg);
}

DO_DRAW(draw_left_line)
{
	int row = top_row;

	arg = arg1;

	arg1 += sprintf(arg1, "\e[%d;%dH%s", top_row, top_col, draw_corner(flags, "┌"));

	while (++row < bot_row)
	{
		arg1 += sprintf(arg1, "\e[%d;%dH%s", row, top_col, draw_vertical(flags, "│"));
	}

	arg1 += sprintf(arg1, "\e[%d;%dH%s", bot_row, top_col, draw_corner(flags, "└"));

	printf("%s", arg);
}

DO_DRAW(draw_map)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	top_row = get_row_index(ses, arg1);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	top_col = get_col_index(ses, arg1);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	bot_row = get_row_index(ses, arg1);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	bot_col = get_col_index(ses, arg1);

	rows = URANGE(1, 1 + bot_row - top_row, gtd->screen->rows);
	cols = URANGE(1, 1 + bot_col - top_col, gtd->screen->cols);

	save_pos(ses);

	draw_text(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, arg2, arg1, arg);

	load_pos(ses);
}

DO_DRAW(draw_middle_top_line)
{
	int col;

	goto_pos(ses, top_row, top_col);

	arg = arg1;

	arg1 += sprintf(arg1, "%s", draw_corner(flags, "├"));

	for (col = top_col + 1 ; col < bot_col ; col++)
	{
		arg1 += sprintf(arg1, "%s", draw_horizontal(flags, "─"));
	}
	
	arg1 += sprintf(arg1, "%s", draw_corner(flags, "┤"));

	printf("%s", arg);
}

DO_DRAW(draw_middle_bot_line)
{
	int col;

	goto_pos(ses, bot_row, top_col);

	arg = arg1;

	arg1 += sprintf(arg1, "%s", draw_corner(flags, "├"));

	for (col = top_col + 1 ; col < bot_col ; col++)
	{
		arg1 += sprintf(arg1, "%s", draw_horizontal(flags, "─"));
	}

	arg1 += sprintf(arg1, "%s", draw_corner(flags, "┤"));

	printf("%s", arg);
}

DO_DRAW(draw_right_line)
{
	int row = top_row;

	arg = arg1;

	arg1 += sprintf(arg1, "\e[%d;%dH%s", top_row, bot_col, draw_corner(flags, "┐"));

	while (++row < bot_row)
	{
		arg1 += sprintf(arg1, "\e[%d;%dH%s", row, bot_col, draw_vertical(flags, "│"));
	}

	arg1 += sprintf(arg1, "\e[%d;%dH%s", bot_row, bot_col, draw_corner(flags, "┘"));

	printf("%s", arg);
}

DO_DRAW(draw_side_lines)
{
	draw_vertical_line(ses, top_row, top_col, bot_row, top_col, rows, cols, flags, arg, arg1, arg2);
	draw_vertical_line(ses, top_row, bot_col, bot_row, bot_col, rows, cols, flags, arg, arg1, arg2);
}

DO_DRAW(draw_side_lines_text)
{
	draw_side_lines(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, arg, arg1, arg2);

	draw_text(ses, top_row, top_col + 1, bot_row, bot_col - 1, rows, cols - 2, flags, arg, arg1, arg2);
}

DO_DRAW(draw_top_line)
{
	int col;

	goto_pos(ses, top_row, top_col);

	arg = arg1;

	arg1 += sprintf(arg1, "%s", draw_corner(flags, "┌"));

	for (col = top_col + 1 ; col < bot_col ; col++)
	{
		arg1 += sprintf(arg1, "%s", draw_horizontal(flags, "─"));
	}
	
	arg1 += sprintf(arg1, "%s", draw_corner(flags, "┐"));

	printf("%s", arg);
}


DO_DRAW(draw_text)
{
	char *txt;
	int row, col, lines;

	push_call("draw_text(%p,%d,%p,%p,%p)",ses,flags,arg,arg1,arg2);

	draw_blank(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, arg, arg1, arg2);

	txt = arg2;

	while (*arg)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, SUB_VAR|SUB_FUN|SUB_COL|SUB_ESC, GET_ALL);

		txt += sprintf(txt, "%s\n", arg1);

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}

	lines = -1 + word_wrap_split(ses, arg2, arg1, cols, 0, BUFFER_SIZE / cols);

	txt = arg1;

	while (*txt && lines > rows)
	{
		txt = strchr(txt, '\n');
		txt++;
		lines--;
	}

	arg = txt;
	row = top_row;
	col = top_col;

	while (*arg)
	{
		arg = strchr(arg, '\n');

		*arg++ = 0;

		goto_pos(ses, row++, col);

		printf("%s", txt);

		txt = arg;
	}
	pop_call();
	return;
}

DO_DRAW(draw_vertical_line)
{
	int row = top_row;

	arg = arg1;

	arg1 += sprintf(arg1, "\e[%d;%dH%s", top_row, top_col, draw_vertical(flags, "│"));

	for (row = top_row + 1 ; row <= bot_row ; row++)
	{
		arg1 += sprintf(arg1, "\e[%d;%dH%s", row, top_col, draw_vertical(flags, "│"));
	}

	printf("%s", arg);
}
