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

static char draw_buf[100][10];
static int  draw_cnt;

#include "tintin.h"

DO_COMMAND(do_draw)
{
	char *sub_arg, arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], color[BUFFER_SIZE], arg3[BUFFER_SIZE], input[BUFFER_SIZE];
	int index, flags;
	int top_row, top_col, bot_row, bot_col, rows, cols;

	substitute(ses, arg, input, SUB_VAR|SUB_FUN);

	arg = input;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	draw_cnt = 0;

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
		flags = HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) ? DRAW_FLAG_UTF8 : 0;

		while (*arg)
		{
			if (!HAS_BIT(flags, DRAW_FLAG_COLOR) && get_color_names(ses, arg1, color))
			{
				SET_BIT(flags, DRAW_FLAG_COLOR);
			}
			else if (!HAS_BIT(flags, DRAW_FLAG_COLOR) && strip_vt102_strlen(ses, arg1) == 0)
			{
				strcpy(color, arg1);
				SET_BIT(flags, DRAW_FLAG_COLOR);
			}
			else if (is_abbrev(arg1, "ASCII"))
			{
				DEL_BIT(flags, DRAW_FLAG_UTF8);
			}
			else if (is_abbrev(arg1, "BLANKED"))
			{
				SET_BIT(flags, DRAW_FLAG_BLANKED|DRAW_FLAG_BOXED);
			}
			else if (is_abbrev(arg1, "BOTTOM"))
			{
				SET_BIT(flags, DRAW_FLAG_BOT);
			}
			else if (is_abbrev(arg1, "BUMPED"))
			{
				SET_BIT(flags, DRAW_FLAG_BUMP);
			}
			else if (is_abbrev(arg1, "CIRCLED"))
			{
				SET_BIT(flags, DRAW_FLAG_CIRCLED);
			}
			else if (is_abbrev(arg1, "CONVERT"))
			{
				SET_BIT(flags, DRAW_FLAG_CONVERT);
			}
			else if (is_abbrev(arg1, "CORNERED"))
			{
				SET_BIT(flags, DRAW_FLAG_CORNERED);
			}
			else if (is_abbrev(arg1, "CROSSED"))
			{
				SET_BIT(flags, DRAW_FLAG_CROSSED|DRAW_FLAG_BOXED);
			}
			else if (is_abbrev(arg1, "FILLED"))
			{
				SET_BIT(flags, DRAW_FLAG_FILLED);
			}
			else if (is_abbrev(arg1, "HORIZONTAL"))
			{
				SET_BIT(flags, DRAW_FLAG_HOR);
			}
			else if (is_abbrev(arg1, "JEWELED"))
			{
				SET_BIT(flags, DRAW_FLAG_JEWELED);
			}
			else if (is_abbrev(arg1, "LEFT"))
			{
				SET_BIT(flags, DRAW_FLAG_LEFT);
			}
			else if (is_abbrev(arg1, "NUMBERED"))
			{
				SET_BIT(flags, DRAW_FLAG_NUMBERED);
			}
			else if (is_abbrev(arg1, "PRUNED"))
			{
				SET_BIT(flags, DRAW_FLAG_PRUNED|DRAW_FLAG_BOXED);
			}
			else if (is_abbrev(arg1, "RIGHT"))
			{
				SET_BIT(flags, DRAW_FLAG_RIGHT);
			}
			else if (is_abbrev(arg1, "ROUNDED"))
			{
				SET_BIT(flags, DRAW_FLAG_ROUNDED|DRAW_FLAG_BOXED);
			}
			else if (is_abbrev(arg1, "TEED"))
			{
				SET_BIT(flags, DRAW_FLAG_TEED|DRAW_FLAG_BOXED);
			}
			else if (is_abbrev(arg1, "TOP"))
			{
				SET_BIT(flags, DRAW_FLAG_TOP);
			}
			else if (is_abbrev(arg1, "TUBED"))
			{
				SET_BIT(flags, DRAW_FLAG_TUBED);
			}
			else if (is_abbrev(arg1, "UNICODE"))
			{
				SET_BIT(flags, DRAW_FLAG_UTF8);
			}
			else if (is_abbrev(arg1, "VERTICAL"))
			{
				SET_BIT(flags, DRAW_FLAG_VER);
			}
			else
			{
				break;
			}
			arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
		}

		for (index = 0 ; *draw_table[index].name ; index++)
		{
			if (is_abbrev(arg1, draw_table[index].name))
			{
				arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

				sub_arg = get_arg_in_braces(ses, arg1, arg3, GET_ONE);

				if (*sub_arg)
				{
					strcpy(arg1, arg3);
					sub_arg = get_arg_in_braces(ses, sub_arg, arg2, GET_ONE);
				}
				else
				{
					arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);
				}

				top_row = get_row_index(ses, arg1);
				top_col = get_col_index(ses, arg2);

				if (*sub_arg)
				{
					sub_arg = get_arg_in_braces(ses, sub_arg, arg1, GET_ONE);
					sub_arg = get_arg_in_braces(ses, sub_arg, arg2, GET_ONE);
				}
				else
				{
					arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
					arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);
				}
				bot_row = get_row_index(ses, arg1);
				bot_col = get_col_index(ses, arg2);

				if (top_row == 0 && top_col == 0)
				{
					show_error(ses, LIST_COMMAND, "#SYNTAX: #DRAW [COLOR] [OPTIONS] {%s} <TOP_ROW> <TOP_COL> <BOT_ROW> <BOT_COL> [TEXT]", draw_table[index].name);

					return ses;
				}

				if (top_row == 0)
				{
					top_row = 1;

					SET_BIT(flags, DRAW_FLAG_SCROLL);
				}

				if (top_col == 0)
				{
					top_col = 1;
				}

				if (bot_row == 0)
				{
					bot_row = 1;
				}

				if (bot_col == 0)
				{
					bot_col = 1;
				}

				rows = URANGE(1, 1 + bot_row - top_row, gtd->screen->rows);
				cols = URANGE(1, 1 + bot_col - top_col, gtd->screen->cols);

				*arg1 = 0;
				*arg2 = 0;
				*arg3 = 0;

				if (*arg == 0)
				{
					arg = arg3;
				}

				save_pos(ses);

				if (HAS_BIT(flags, DRAW_FLAG_BUMP))
				{
					tintin_printf2(ses, "");
				}

				draw_table[index].fun(ses, top_row, top_col, bot_row, bot_col, rows, cols, draw_table[index].flags | flags, color, arg, arg1, arg2);

				print_stdout("\e[0m");

				restore_pos(ses);

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
	draw_cnt = (draw_cnt + 1) % 100;

	if (HAS_BIT(flags, DRAW_FLAG_BLANKED))
	{
		strcpy(draw_buf[draw_cnt], " ");
	}
	else if (HAS_BIT(flags, DRAW_FLAG_NUMBERED))
	{
		sprintf(draw_buf[draw_cnt], "%d", draw_cnt % 10);
	}
	else if (HAS_BIT(flags, DRAW_FLAG_UTF8))
	{
		if (HAS_BIT(flags, DRAW_FLAG_PRUNED))
		{
			if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
			{
				strcpy(draw_buf[draw_cnt], " ");
			}
			else
			{
				strcpy(draw_buf[draw_cnt], "\e[C");
			}
		}
		else if (HAS_BIT(flags, DRAW_FLAG_CIRCLED))
		{
			if (HAS_BIT(flags, DRAW_FLAG_CROSSED))
			{
				strcpy(draw_buf[draw_cnt], "ϴ");
			}
			else if (HAS_BIT(flags, DRAW_FLAG_FILLED))
			{
				strcpy(draw_buf[draw_cnt], "⬤");
			}
			else if (HAS_BIT(flags, DRAW_FLAG_VER))
			{
				strcpy(draw_buf[draw_cnt], "O");
			}
			else
			{
				strcpy(draw_buf[draw_cnt], "○");
			}
		}
		else if (HAS_BIT(flags, DRAW_FLAG_CROSSED))
		{
			strcpy(draw_buf[draw_cnt], "┼");
		}
		else if (HAS_BIT(flags, DRAW_FLAG_JEWELED))
		{
			if (HAS_BIT(flags, DRAW_FLAG_FILLED))
			{
				strcpy(draw_buf[draw_cnt], "⧫");
			}
			else
			{
				strcpy(draw_buf[draw_cnt], "◊");
			}
		}
		else if (HAS_BIT(flags, DRAW_FLAG_TEED))
		{
			if (HAS_BIT(flags, DRAW_FLAG_HOR))
			{
				if (HAS_BIT(flags, DRAW_FLAG_LEFT))
				{
					if (HAS_BIT(flags, DRAW_FLAG_TUBED))
					{
						strcpy(draw_buf[draw_cnt], "╠");
					}
					else
					{
						strcpy(draw_buf[draw_cnt], "├");
					}
				}
				else
				{
					if (HAS_BIT(flags, DRAW_FLAG_TUBED))
					{
						strcpy(draw_buf[draw_cnt], "╣");
					}
					else
					{
						strcpy(draw_buf[draw_cnt], "┤");
					}
				}
			}
			else
			{
				if (HAS_BIT(flags, DRAW_FLAG_TOP))
				{
					if (HAS_BIT(flags, DRAW_FLAG_TUBED))
					{
						strcpy(draw_buf[draw_cnt], "╦");
					}
					else
					{
						strcpy(draw_buf[draw_cnt], "┬");
					}
				}
				else
				{
					if (HAS_BIT(flags, DRAW_FLAG_TUBED))
					{
						strcpy(draw_buf[draw_cnt], "╧");
					}
					else
					{
						strcpy(draw_buf[draw_cnt], "┴");
					}
				}
			}
		}
		else if (HAS_BIT(flags, DRAW_FLAG_BOXED) || HAS_BIT(flags, DRAW_FLAG_CORNERED))
		{
			if (HAS_BIT(flags, DRAW_FLAG_LEFT))
			{
				if (HAS_BIT(flags, DRAW_FLAG_TOP))
				{
					if (HAS_BIT(flags, DRAW_FLAG_ROUNDED))
					{
						strcpy(draw_buf[draw_cnt], "╭");
					}
					else
					{
						if (HAS_BIT(flags, DRAW_FLAG_TUBED))
						{
							strcpy(draw_buf[draw_cnt], "╔");
						}
						else
						{
							strcpy(draw_buf[draw_cnt], "┌");
						}
					}
				}
				else if (HAS_BIT(flags, DRAW_FLAG_BOT))
				{
					if (HAS_BIT(flags, DRAW_FLAG_ROUNDED))
					{
						strcpy(draw_buf[draw_cnt], "╰");
					}
					else
					{
						if (HAS_BIT(flags, DRAW_FLAG_TUBED))
						{
							strcpy(draw_buf[draw_cnt], "╚");
						}
						else
						{
							strcpy(draw_buf[draw_cnt], "└");
						}
					}
				}
				else
				{
					if (HAS_BIT(flags, DRAW_FLAG_HOR))
					{
						if (HAS_BIT(flags, DRAW_FLAG_TUBED))
						{
							strcpy(draw_buf[draw_cnt], "═");
						}
						else
						{
							strcpy(draw_buf[draw_cnt], "─");
						}
					}
					else
					{
						if (HAS_BIT(flags, DRAW_FLAG_TUBED))
						{
							strcpy(draw_buf[draw_cnt], "║");
						}
						else
						{
							strcpy(draw_buf[draw_cnt], "│");
						}
					}
				}
			}
			else if (HAS_BIT(flags, DRAW_FLAG_RIGHT))
			{
				if (HAS_BIT(flags, DRAW_FLAG_TOP))
				{
					if (HAS_BIT(flags, DRAW_FLAG_ROUNDED))
					{
						strcpy(draw_buf[draw_cnt], "╮");
					}
					else
					{
						if (HAS_BIT(flags, DRAW_FLAG_TUBED))
						{
							strcpy(draw_buf[draw_cnt], "╗");
						}
						else
						{
							strcpy(draw_buf[draw_cnt], "┐");
						}
					}
				}
				else if (HAS_BIT(flags, DRAW_FLAG_BOT))
				{
					if (HAS_BIT(flags, DRAW_FLAG_ROUNDED))
					{
						strcpy(draw_buf[draw_cnt], "╯");
					}
					else
					{
						if (HAS_BIT(flags, DRAW_FLAG_TUBED))
						{
							strcpy(draw_buf[draw_cnt], "╝");
						}
						else
						{
							strcpy(draw_buf[draw_cnt], "┘");
						}
					}
				}
				else
				{
					if (HAS_BIT(flags, DRAW_FLAG_HOR))
					{
						if (HAS_BIT(flags, DRAW_FLAG_TUBED))
						{
							strcpy(draw_buf[draw_cnt], "═");
						}
						else
						{
							strcpy(draw_buf[draw_cnt], "─");
						}
					}
					else
					{
						if (HAS_BIT(flags, DRAW_FLAG_TUBED))
						{
							strcpy(draw_buf[draw_cnt], "║");
						}
						else
						{
							strcpy(draw_buf[draw_cnt], "│");
						}
					}
				}
			}
			else
			{
				if (HAS_BIT(flags, DRAW_FLAG_HOR))
				{
					if (HAS_BIT(flags, DRAW_FLAG_TUBED))
					{
						strcpy(draw_buf[draw_cnt], "═");
					}
					else
					{
						strcpy(draw_buf[draw_cnt], "─");
					}
				}
				else
				{
					if (HAS_BIT(flags, DRAW_FLAG_TUBED))
					{
						strcpy(draw_buf[draw_cnt], "║");
					}
					else
					{
						strcpy(draw_buf[draw_cnt], "│");
					}
				}
			}
		}
		else
		{
			if (HAS_BIT(flags, DRAW_FLAG_HOR))
			{
				if (HAS_BIT(flags, DRAW_FLAG_TUBED))
				{
					strcpy(draw_buf[draw_cnt], "═");
				}
				else
				{
					strcpy(draw_buf[draw_cnt], "─");
				}
			}
			else if (HAS_BIT(flags, DRAW_FLAG_VER))
			{
				if (HAS_BIT(flags, DRAW_FLAG_TUBED))
				{
					strcpy(draw_buf[draw_cnt], "║");
				}
				else
				{
					strcpy(draw_buf[draw_cnt], "│");
				}
			}
			else
			{
				strcpy(draw_buf[draw_cnt], "?");
			}
		}
	}
	else
	{
		if (HAS_BIT(flags, DRAW_FLAG_PRUNED))
		{
			strcpy(draw_buf[draw_cnt], "\e[C");
		}
		else if (HAS_BIT(flags, DRAW_FLAG_CIRCLED) || HAS_BIT(flags, DRAW_FLAG_ROUNDED))
		{
			strcpy(draw_buf[draw_cnt], "o");
		}
		else if (HAS_BIT(flags, DRAW_FLAG_CROSSED))
		{
			strcpy(draw_buf[draw_cnt], "+");
		}
		else
		{
			strcpy(draw_buf[draw_cnt], "+");
		}
	}
	return draw_buf[draw_cnt];
}

char *draw_horizontal(int flags, char *str)
{
	draw_cnt = (draw_cnt + 1) % 100;

	if (HAS_BIT(flags, DRAW_FLAG_BLANKED))
	{
		strcpy(draw_buf[draw_cnt], " ");
	}
	else if (HAS_BIT(flags, DRAW_FLAG_NUMBERED))
	{
		sprintf(draw_buf[draw_cnt], "%d", draw_cnt % 10);
	}
	else if (HAS_BIT(flags, DRAW_FLAG_UTF8))
	{
		if (HAS_BIT(flags, DRAW_FLAG_TUBED))
		{
			strcpy(draw_buf[draw_cnt], "═");
		}
		else
		{
			strcpy(draw_buf[draw_cnt], "─");
		}
	}
	else
	{
		strcpy(draw_buf[draw_cnt], "-");
	}
	return draw_buf[draw_cnt];
}

char *draw_vertical(int flags, char *str)
{
	draw_cnt = (draw_cnt + 1) % 100;

	if (HAS_BIT(flags, DRAW_FLAG_BLANKED))
	{
		strcpy(draw_buf[draw_cnt], " ");
	}
	else if (HAS_BIT(flags, DRAW_FLAG_NUMBERED))
	{
		sprintf(draw_buf[draw_cnt], "%d", draw_cnt % 10);
	}
	else if (HAS_BIT(flags, DRAW_FLAG_UTF8))
	{
		if (HAS_BIT(flags, DRAW_FLAG_TUBED))
		{
			strcpy(draw_buf[draw_cnt], "║");
		}
		else
		{
			strcpy(draw_buf[draw_cnt], "│");
		}
	}
	else
	{
		strcpy(draw_buf[draw_cnt], "|");
	}
	return draw_buf[draw_cnt];
}

// options

DO_DRAW(draw_bot_side)
{
	int col, corner;

	SET_BIT(flags, HAS_BIT(flags, DRAW_FLAG_VER) ? DRAW_FLAG_VER : DRAW_FLAG_HOR);

	corner = flags;

	DEL_BIT(corner, DRAW_FLAG_RIGHT|DRAW_FLAG_TOP);

	arg = arg1;

	if (HAS_BIT(flags, DRAW_FLAG_LEFT) || HAS_BIT(flags, DRAW_FLAG_BOT))
	{
		arg1 += sprintf(arg1, "%s%s", color, draw_corner(corner, "└"));
	}

	if (cols - 2 >= 0)
	{
		if (HAS_BIT(flags, DRAW_FLAG_BOT))
		{
			for (col = top_col + 1 ; col < bot_col ; col++)
			{
				arg1 += sprintf(arg1, "%s", draw_horizontal(flags, "─"));
			}
		}
		else if (HAS_BIT(flags, DRAW_FLAG_RIGHT) && cols - 2 > 0)
		{
			arg1 += sprintf(arg1, "\e[%dC", cols - 2);
		}

		corner = flags;

		DEL_BIT(corner, DRAW_FLAG_LEFT|DRAW_FLAG_TOP);

		if (HAS_BIT(flags, DRAW_FLAG_RIGHT) || HAS_BIT(flags, DRAW_FLAG_BOT))
		{
			arg1 += sprintf(arg1, "%s", draw_corner(corner, "┘"));
		}
	}

	if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
	{
		tintin_printf2(ses, "%s%s", indent_one(top_col - 1), arg);
	}
	else
	{
		goto_pos(ses, bot_row, top_col);

		print_stdout("%s", arg);
	}
}

DO_DRAW(draw_box)
{
	draw_top_side(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2);

	draw_vertical_lines(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2);

	draw_bot_side(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2);
}

DO_DRAW(draw_line)
{
	if (top_row == bot_row)
	{
		SET_BIT(flags, DRAW_FLAG_HOR|DRAW_FLAG_LEFT|DRAW_FLAG_RIGHT);

		if (!HAS_BIT(flags, DRAW_FLAG_BOT))
		{
			SET_BIT(flags, DRAW_FLAG_TOP);
		}
	}
	if (top_col == bot_col)
	{
		SET_BIT(flags, DRAW_FLAG_VER|DRAW_FLAG_TOP|DRAW_FLAG_BOT);

		if (!HAS_BIT(flags, DRAW_FLAG_RIGHT))
		{
			SET_BIT(flags, DRAW_FLAG_LEFT);
		}
	}
	draw_box(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2);
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

	draw_text(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg2, arg1, arg);

	restore_pos(ses);
}

DO_DRAW(draw_side)
{
	push_call("draw_side()");

	draw_box(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2);

	pop_call();
	return;
}

DO_DRAW(draw_square)
{
	draw_text(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2);
}

DO_DRAW(draw_text)
{
	char *txt, buf1[BUFFER_SIZE], buf2[BUFFER_SIZE], buf3[BUFFER_SIZE];
	int row, col, height, width;

	push_call("draw_text(%p,%d,%p,%p,%p)",ses,flags,arg,arg1,arg2);

	buf1[0] = buf2[0] = arg1[0] = arg2[0] = 0;

	txt = buf2;

	substitute(ses, arg, buf3, SUB_VAR|SUB_FUN);

	arg = buf3;

	while (*arg)
	{
		arg = sub_arg_in_braces(ses, arg, buf1, GET_ALL, SUB_COL|SUB_ESC|SUB_VAR|SUB_FUN);

		txt += sprintf(txt, "%s\n", buf1);

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED) || HAS_BIT(flags, DRAW_FLAG_TOP) || HAS_BIT(flags, DRAW_FLAG_PRUNED))
	{
		top_row++;
		rows--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED) || HAS_BIT(flags, DRAW_FLAG_BOT) || HAS_BIT(flags, DRAW_FLAG_PRUNED))
	{
		bot_row--;
		rows--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED) || HAS_BIT(flags, DRAW_FLAG_LEFT) || HAS_BIT(flags, DRAW_FLAG_PRUNED))
	{
		strcpy(arg1, " ");
		cols--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED) || HAS_BIT(flags, DRAW_FLAG_RIGHT) || HAS_BIT(flags, DRAW_FLAG_PRUNED))
	{
		strcpy(arg2, " ");
		cols--;
	}

	word_wrap_split(ses, buf2, buf1, cols, 0, 0, FLAG_NONE, &height, &width);

	height--;

	txt = buf1;

	while (*txt && height > rows)
	{
		txt = strchr(txt, '\n');
		txt++;
		height--;
	}

	arg = txt;
	row = top_row;
	col = top_col;

	while (*arg)
	{
		arg = strchr(arg, '\n');

		*arg++ = 0;

		justify_string(ses, txt, buf2, 0 - cols, cols);

		if (HAS_BIT(flags, DRAW_FLAG_LEFT))
		{
			strcpy(arg1, draw_vertical(flags, "│"));
		}

		if (HAS_BIT(flags, DRAW_FLAG_RIGHT))
		{
			strcpy(arg2, draw_vertical(flags, "│"));
		}

		if (HAS_BIT(flags, DRAW_FLAG_CONVERT))
		{
			convert_meta(buf2, buf3, FALSE);
			strcpy(buf2, buf3);
		}

		if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
		{
			tintin_printf2(ses, "%s%s%s%s%s%s", indent_one(top_col - 1), color, arg1, buf2, color, arg2);
		}
		else
		{
			goto_pos(ses, row++, col);

			print_stdout("%s%s%s%s%s", color, arg1, buf2, color, arg2);
		}

		txt = arg;
	}

	while (height < rows)
	{
		if (HAS_BIT(flags, DRAW_FLAG_LEFT))
		{
			strcpy(arg1, draw_vertical(flags, "│"));
		}

		if (HAS_BIT(flags, DRAW_FLAG_RIGHT))
		{
			strcpy(arg2, draw_vertical(flags, "│"));
		}

		if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
		{
			
			tintin_printf2(ses, "%s%s%s%-*s%s%s", indent_one(top_col - 1), color, arg1, cols, "", color, arg2);
		}
		else
		{
			goto_pos(ses, row++, col);

			print_stdout("%s%s%*s%s%s", color, arg1, cols, "", color, arg2);
		}
		height++;
	}
	pop_call();
	return;
}

DO_DRAW(draw_top_side)
{
	int col, corner;

	SET_BIT(flags, HAS_BIT(flags, DRAW_FLAG_VER) ? DRAW_FLAG_VER : DRAW_FLAG_HOR);

	corner = flags;

	DEL_BIT(corner, DRAW_FLAG_RIGHT|DRAW_FLAG_BOT);

	arg = arg1;

	if (HAS_BIT(flags, DRAW_FLAG_LEFT) || HAS_BIT(flags, DRAW_FLAG_TOP))
	{
		arg1 += sprintf(arg1, "%s%s", color, draw_corner(corner, "┌"));
	}

	if (cols - 2 >= 0)
	{
		if (HAS_BIT(flags, DRAW_FLAG_TOP))
		{
			for (col = top_col + 1 ; col < bot_col ; col++)
			{
				arg1 += sprintf(arg1, "%s", draw_horizontal(flags, "─"));
			}
		}
		else if (HAS_BIT(flags, DRAW_FLAG_RIGHT) && cols - 2 > 0)
		{
			arg1 += sprintf(arg1, "\e[%dC", cols - 2);
		}

		corner = flags;

		DEL_BIT(corner, DRAW_FLAG_LEFT|DRAW_FLAG_BOT);

		if (HAS_BIT(flags, DRAW_FLAG_TOP) || HAS_BIT(flags, DRAW_FLAG_RIGHT))
		{
			arg1 += sprintf(arg1, "%s", draw_corner(corner, "┐"));
		}
	}

	if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
	{
		tintin_printf2(ses, "%*s%s", top_col - 1, "", arg);
	}
	else
	{
		goto_pos(ses, top_row, top_col);

		print_stdout("%s", arg);
	}
}

DO_DRAW(draw_vertical_lines)
{
	int row, col, lines;

	push_call("draw_vertical_lines(%p,%d,%p,%p,%p)",ses,flags,arg,arg1,arg2);

	if (HAS_BIT(flags, DRAW_FLAG_SCROLL) || *arg)
	{
		draw_text(ses, top_row, top_col, bot_row, top_col, rows, cols, flags, color, arg, arg1, arg2);

		pop_call();
		return;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED) || HAS_BIT(flags, DRAW_FLAG_TOP))
	{
		top_row++;
		rows--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED) || HAS_BIT(flags, DRAW_FLAG_BOT))
	{
		bot_row--;
		rows--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED) || HAS_BIT(flags, DRAW_FLAG_LEFT))
	{
		cols--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED) || HAS_BIT(flags, DRAW_FLAG_RIGHT))
	{
		cols--;
	}

	lines = 0;

	row = top_row;
	col = top_col;

	strcpy(arg1, "");
	strcpy(arg2, "");

	while (lines < rows)
	{
		if (HAS_BIT(flags, DRAW_FLAG_LEFT))
		{
			strcpy(arg1, draw_vertical(flags, "│"));
		}

		if (HAS_BIT(flags, DRAW_FLAG_RIGHT))
		{
			strcpy(arg2, draw_vertical(flags, "│"));
		}

		goto_pos(ses, row++, col);

		print_stdout("%s%s\e[%dC%s%s", color, arg1, cols, color, arg2);

		lines++;
	}
	pop_call();
	return;
}
