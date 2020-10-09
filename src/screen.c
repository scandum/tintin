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
*                      coded by Igor van den Hoven 2019                       *
******************************************************************************/

#include "tintin.h"
#include "telnet.h"

void screen_osc(char *arg1, char *arg2);
void screen_csi(char *cmd, char *arg1, char *arg2, char *arg3, char *tc);
void screen_csit(struct session *ses, char *arg1, char *arg2, char *tc);

DO_COMMAND(do_screen)
{
	int cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " SCREEN OPTIONS ");

		for (cnt = 0 ; *screen_table[cnt].fun ; cnt++)
		{
			if (*screen_table[cnt].name)
			{
				tintin_printf2(ses, "  [%-18s] [%-6s] %s", screen_table[cnt].name, "", screen_table[cnt].desc);
			}
		}
		tintin_header(ses, 80, "");
	}
	else
	{
		for (cnt = 0 ; *screen_table[cnt].name ; cnt++)
		{
			if (is_abbrev(arg1, screen_table[cnt].name))
			{
				if (!HAS_BIT(screen_table[cnt].get1, SCREEN_FLAG_GET_NONE))
				{
					if (HAS_BIT(screen_table[cnt].get1, SCREEN_FLAG_GET_ONE))
					{
						arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
					}
					else
					{
						arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);
					}
				}

				if (!HAS_BIT(screen_table[cnt].get2, SCREEN_FLAG_GET_NONE))
				{
					if (HAS_BIT(screen_table[cnt].get2, SCREEN_FLAG_GET_ONE))
					{
						arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
					}
					else
					{
						arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);
					}
				}
				screen_table[cnt].fun(ses, cnt, arg, arg1, arg2);

				return ses;
			}
		}
		tintin_printf(ses, "#ERROR: #SCREEN {%s} IS NOT A VALID OPTION.", capitalize(arg1));
	}
	return ses;
}



DO_SCREEN(screen_blur)
{
	screen_csit(ses, "6", "", "");
}


DO_SCREEN(screen_cursor)
{
	if (is_abbrev(arg1, "HIDE"))
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_HIDDENCURSOR);
		screen_csi("?", "25", "", "", "l");
	}
	else if (is_abbrev(arg1, "SHOW"))
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_HIDDENCURSOR);
		screen_csi("?", "25", "", "", "h");
	}
	else if (is_abbrev(arg1, "BAR"))
	{
		screen_csi("", "6", "", " ", "q");
	}
	else if (is_abbrev(arg1, "BLOCK"))
	{
		screen_csi("", "2", "", " ", "q");
	}
	else if (is_abbrev(arg1, "UNDERLINE"))
	{
		screen_csi("", "4", "", " ", "q");
	}
	else if (is_abbrev(arg1, "BLINK"))
	{
		screen_csi("?", "12", "", "", "h");
	}
	else if (is_abbrev(arg1, "STEADY"))
	{
		screen_csi("?", "12", "", "", "l");
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {CURSOR} {HIDE|SHOW|BLINK|STEADY}");
	}	
}

DO_SCREEN(screen_clear)
{
	int top_row, top_col, bot_row, bot_col;

	if (is_abbrev(arg1, "ALL"))
	{
		print_stdout(0, 0, "\e[2J");
	}
	else if (is_abbrev(arg1, "BOTTOM SPLIT"))
	{
		erase_bot_region(ses);
	}
	else if (is_abbrev(arg1, "TOP SPLIT"))
	{
		erase_top_region(ses);
	}
	else if (is_abbrev(arg1, "LEFT SPLIT"))
	{
		erase_left_region(ses);
	}
	else if (is_abbrev(arg1, "RIGHT SPLIT"))
	{
		erase_right_region(ses);
	}
	else if (is_abbrev(arg1, "SCROLL REGION"))
	{
		erase_scroll_region(ses);
	}
	else if (is_abbrev(arg1, "INPUT REGION"))
	{
		erase_input_region(ses);
	}
	else if (is_abbrev(arg1, "SPLIT REGION"))
	{
		erase_split_region(ses);
	}
	else if (is_abbrev(arg1, "SQUARE"))
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

		top_row = get_row_index_arg(ses, arg1);
		top_col = get_col_index_arg(ses, arg2);

//		tintin_printf2(ses, "debug: (%s) (%s) %d %d", arg1, arg2, top_row, top_col);

		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

		bot_row = get_row_index_arg(ses, arg1);
		bot_col = get_col_index_arg(ses, arg2);

//		tintin_printf2(ses, "debug: (%s) (%s) %d %d", arg1, arg2, bot_row, bot_col);

		if (bot_col == 0)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN CLEAR SQUARE {<ROW>} {<COL>} {<ROW>} {<COL>}");
		}
		else
		{
			erase_square(ses, top_row, top_col, bot_row, bot_col);

//			print_stdout(0, 0, "\e[%d;%d;%d;%d${", top_row, top_col, bot_row, bot_col); VT400 not widely supported
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN CLEAR {ALL|BOT|TOP|LEFT|RIGHT|SCROLL|INPUT|SPLIT|SQUARE}");
	}
}

DO_SCREEN(screen_dump)
{
	int cnt;

	save_pos(ses);

	printf("\e[2J");

	for (cnt = 0 ; cnt < gtd->screen->max_row ; cnt++)
	{
		goto_pos(ses, cnt + 1, 1);

//		printf("%02d %s", cnt + 1, gtd->screen->grid[cnt]->str);
		printf("%s", gtd->screen->grid[cnt]->str);
	}
	restore_pos(ses);
}

DO_SCREEN(screen_fill)
{
	if (is_abbrev(arg1, "DEFAULT"))
	{
		if (ses->split->sav_top_col || ses->split->sav_bot_col)
		{
			command(ses, do_screen, "CLEAR SPLIT");
		}

		if (ses->split->sav_top_row > 0)
		{
			if (ses->split->sav_top_row == 1)
			{
				command(ses, do_draw, "%s LINE %d %d %d %d", arg2, 1, 1, ses->split->top_row - 1, gtd->screen->cols);
			}
			else
			{
				command(ses, do_draw, "%s BOX %d %d %d %d {}", arg2, 1, 1, ses->split->top_row - 1, gtd->screen->cols);
			}
		}

		// bottom split

		if (ses->split->sav_bot_row)
		{
			if (ses->split->sav_bot_row - inputline_max_row() >= 0)
			{
				if (ses->split->sav_bot_row - inputline_max_row() == 0)
				{
					command(ses, do_draw, "%s LINE %d %d %d %d", arg2, ses->split->bot_row + 1, 1, gtd->screen->rows - inputline_max_row(), gtd->screen->cols);
				}
				else
				{
					command(ses, do_draw, "%s BOX %d %d %d %d {}", arg2, ses->split->bot_row + 1, 1, gtd->screen->rows - inputline_max_row(), gtd->screen->cols);
				}
			}
		}

		if (ses->split->sav_top_row > 0)
		{
			if (ses->split->sav_top_col)
			{
				command(ses, do_draw, "%s VERTICAL TEED LINE %d %d %d %d", arg2, ses->split->top_row - 1, ses->split->top_col - 1, ses->split->bot_row + 1, ses->split->top_col - 1);

				if (ses->split->sav_top_col == 1)
				{
					if (ses->split->sav_top_row > 1)
					{
						command(ses, do_draw, "%s HORIZONTAL TEED TOP LEFT CORNER %d %d %d %d", arg2, ses->split->top_row - 1, ses->split->top_col - 1, ses->split->bot_row + 1, ses->split->bot_col + 1);
					}
					if (ses->split->sav_bot_row > 1)
					{
						command(ses, do_draw, "%s HORIZONTAL TEED BOTTOM LEFT CORNER %d %d %d %d", arg2, ses->split->bot_row + 1, ses->split->top_col - 1, ses->split->bot_row + 1, ses->split->top_col - 1);
					}
				}
			}

			if (ses->split->sav_bot_col)
			{
				command(ses, do_draw, "%s VERTICAL TEED LINE %d %d %d %d", arg2, ses->split->top_row - 1, ses->split->bot_col + 1, ses->split->bot_row + 1, ses->split->bot_col + 1);

				if (ses->split->sav_bot_col == 1)
				{
					if (ses->split->sav_top_row > 1)
					{
						command(ses, do_draw, "%s HORIZONTAL TEED TOP RIGHT CORNER %d %d %d %d", arg2, ses->split->top_row - 1, ses->split->bot_col + 1, ses->split->bot_row + 1, ses->split->bot_col + 1);						
					}
					if (ses->split->sav_bot_row > 1)
					{
						command(ses, do_draw, "%s HORIZONTAL TEED BOTTOM RIGHT CORNER %d %d %d %d", arg2, ses->split->bot_row + 1, ses->split->bot_col + 1, ses->split->bot_row + 1, ses->split->bot_col + 1);
					}
				}
			}
		}
		else
		{
			if (ses->split->sav_top_col)
			{
				command(ses, do_draw, "%s VERTICAL LINE %d %d %d %d", arg2, ses->split->top_row, ses->split->top_col - 1, ses->split->bot_row, ses->split->top_col - 1);
			}

			if (ses->split->sav_bot_col)
			{
				command(ses, do_draw, "%s VERTICAL LINE %d %d %d %d", arg2, ses->split->top_row, ses->split->bot_col + 1, ses->split->bot_row, ses->split->bot_col + 1);
			}
		}
	}
	else if (is_abbrev(arg1, "SPLIT"))
	{
		fill_split_region(ses, arg2);
	}
	else if (is_abbrev(arg1, "BOTTOM SPLIT"))
	{
		fill_bot_region(ses, arg2);
	}
	else if (is_abbrev(arg1, "TOP SPLIT"))
	{
		fill_top_region(ses, arg2);
	}
	else if (is_abbrev(arg1, "LEFT SPLIT"))
	{
		fill_left_region(ses, arg2);
	}
	else if (is_abbrev(arg1, "RIGHT SPLIT"))
	{
		fill_right_region(ses, arg2);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN FILL {TOP|BOT|LEFT|RIGHT|SCROLL|SPLIT|DEFAULT} {arg}");
	}
}

DO_SCREEN(screen_focus)
{
	screen_csit(ses, "5", "", "");
}

DO_SCREEN(screen_fullscreen)
{
	if (*arg1 == 0)
	{
		screen_csit(ses, "10", "2", "");
	}
	else if (is_abbrev(arg1, "ON"))
	{
		screen_csit(ses, "10", "1", "");
	}
	else if (is_abbrev(arg1, "OFF"))
	{
		screen_csit(ses, "10", "0", "");
	}
}

DO_SCREEN(screen_get)
{
	if (*arg1 == 0 || *arg2 == 0)
	{
		tintin_printf2(ses, "#SYNTAX: #SCREEN {GET} {FOCUS} {<VAR>}");
		tintin_printf2(ses, "#SYNTAX: #SCREEN {GET} {ROWS|COLS|HEIGHT|WIDTH} {<VAR>}");
		
		tintin_printf2(ses, "#SYNTAX: #SCREEN {GET} {CHAR_HEIGHT|CHAR_WIDTH}");
		tintin_printf2(ses, "#SYNTAX: #SCREEN {GET} {SPLIT_TOP_BAR|SPLIT_BOT_BAR|SPLIT_LEFT_BAR|SPLIT_RIGHT_BAR} {<VAR>}");
		tintin_printf2(ses, "#SYNTAX: #SCREEN {GET} {SCROLL_TOP_ROW|SCROLL_TOP_COL|SCROLL_BOT_ROW|SCROLL_BOT_COL} {<VAR>}");
		tintin_printf2(ses, "#SYNTAX: #SCREEN {GET} {INPUT_TOP_ROW|INPUT_TOP_COL|INPUT_BOT_ROW|INPUT_BOT_COL} {<VAR>}");
		tintin_printf2(ses, "#SYNTAX: #SCREEN {GET} {SCROLL_ROWS|SCROLL_COLS|INPUT_ROWS|INPUT_COLS} {<VAR>}");
		tintin_printf2(ses, "#SYNTAX: #SCREEN {GET} {INPUT_NAME} {<VAR>}");
		tintin_printf2(ses, "#SYNTAX: #SCREEN {GET} {CUR_ROW|CUR_COL} {<VAR>}");

		return;
	}

	*gtd->is_result = 0;

	switch (*arg1 % 32)
	{
		case CTRL_C:
			if (is_abbrev(arg1, "CHAR_HEIGHT"))
			{
				set_nest_node_ses(ses, arg2, "%d", gtd->screen->char_height);
			}
			else if (is_abbrev(arg1, "CHAR_WIDTH"))
			{
				set_nest_node_ses(ses, arg2, "%d", gtd->screen->char_width);
			}
			else if (is_abbrev(arg1, "COLS"))
			{
				set_nest_node_ses(ses, arg2, "%d", gtd->screen->cols);
			}
			else if (is_abbrev(arg1, "CUR_COL"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->cur_col);
			}
			else if (is_abbrev(arg1, "CUR_ROW"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->cur_row);
			}
			break;

		case CTRL_F:
			if (is_abbrev(arg1, "FOCUS"))
			{
				set_nest_node_ses(ses, arg2, "%d", gtd->screen->focus);
			}
			break;

		case CTRL_H:
			if (is_abbrev(arg1, "HEIGHT"))
			{
				set_nest_node_ses(ses, arg2, "%d", gtd->screen->height);
			}
			break;

		case CTRL_I:
			if (is_abbrev(arg1, "INPUT_ROWS"))
			{
				set_nest_node_ses(ses, arg2, "%d", 1 + ses->input->bot_row - ses->input->top_row);
			}
			else if (is_abbrev(arg1, "INPUT_COLS"))
			{
				set_nest_node_ses(ses, arg2, "%d", 1 + ses->input->bot_col - ses->input->top_col);
			}
			else if (is_abbrev(arg1, "INPUT_NAME"))
			{
				set_nest_node_ses(ses, arg2, "%s", ses->input->line_name);
			}
			else if (is_abbrev(arg1, "INPUT_TOP_ROW"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->input->top_row);
			}
			else if (is_abbrev(arg1, "INPUT_TOP_COL"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->input->top_col);
			}
			else if (is_abbrev(arg1, "INPUT_BOT_ROW"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->input->bot_row);
			}
			else if (is_abbrev(arg1, "INPUT_BOT_COL"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->input->bot_col);
			}
			break;

		case CTRL_R:
			if (is_abbrev(arg1, "ROWS"))
			{
				set_nest_node_ses(ses, arg2, "%d", gtd->screen->rows);
			}
			break;

		case CTRL_S:
			if (is_abbrev(arg1, "SPLIT_TOP_BAR"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->split->sav_top_row);
			}
			else if (is_abbrev(arg1, "SPLIT_LEFT_BAR"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->split->sav_top_col);
			}
			else if (is_abbrev(arg1, "SPLIT_BOT_BAR"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->split->sav_bot_row);
			}
			else if (is_abbrev(arg1, "SPLIT_RIGHT_BAR"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->split->sav_bot_col);
			}
			else if (is_abbrev(arg1, "SCROLL_ROWS"))
			{
				set_nest_node_ses(ses, arg2, "%d", get_scroll_rows(ses));
			}
			else if (is_abbrev(arg1, "SCROLL_COLS"))
			{
				set_nest_node_ses(ses, arg2, "%d", get_scroll_cols(ses));
			}
			else if (is_abbrev(arg1, "SCROLL_TOP_ROW"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->split->top_row);
			}
			else if (is_abbrev(arg1, "SCROLL_TOP_COL"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->split->top_col);
			}
			else if (is_abbrev(arg1, "SCROLL_BOT_ROW"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->split->bot_row);
			}
			else if (is_abbrev(arg1, "SCROLL_BOT_COL"))
			{
				set_nest_node_ses(ses, arg2, "%d", ses->split->bot_col);
			}
			break;

		case CTRL_W:
			if (is_abbrev(arg1, "WIDTH"))
			{
				set_nest_node_ses(ses, arg2, "%d", gtd->screen->width);
			}
			break;
	}

	if (*gtd->is_result == 0)
	{
		screen_get(ses, 0, "", "", "");
	}
}

DO_SCREEN(screen_maximize)
{
	if (*arg1 == 0 || is_abbrev(arg1, "ON"))
	{
		screen_csit(ses, "9", "1", "");
	}
	else if (is_abbrev(arg1, "OFF"))
	{
		screen_csit(ses, "9", "0", "");
	}
	else if (is_abbrev(arg1, "HORIZONTALLY"))
	{
		screen_csit(ses, "9", "3", "");
	}
	else if (is_abbrev(arg1, "VERTICALLY"))
	{
		screen_csit(ses, "9", "2", "");
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {MAXIMIZE} {ON|OFF|VERTICAL|HORIZONTAL}");
	}
}

DO_SCREEN(screen_minimize)
{
	if (*arg1 == 0 || is_abbrev(arg1, "ON"))
	{
		screen_csit(ses, "2", arg2, arg2);
	}
	else if (is_abbrev(arg1, "OFF"))
	{
		screen_csit(ses, "1", arg2, arg2);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {MINIMIZE} {ON|OFF}");
	}
}

DO_SCREEN(screen_move)
{
	int height, width;

	if (!is_math(ses, arg1) || !is_math(ses, arg2))
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {MOVE} {HEIGHT} {WIDTH}");

		return;
	}

	height = (int) get_number(ses, arg1);
	width  = (int) get_number(ses, arg2);

	if (height < 0)
	{
		if (gtd->screen->desk_height == 0)
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #SCREEN MOVE %d %d: USE #SCREEN RAISE DESKTOP DIMENSIONS FIRST.", height, width);

			return;
		}
		if (gtd->screen->tot_height == 0)
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #SCREEN MOVE %d %d: USE #SCREEN RAISE SCREEN DIMENSIONS FIRST.", height, width);

			return;
		}
		sprintf(arg1, "%d", 1 + gtd->screen->desk_height - gtd->screen->tot_height + height);
	}

	if (width < 0)
	{
		if (gtd->screen->desk_width == 0)
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #SCREEN MOVE %d %d: USE #SCREEN RAISE DESKTOP DIMENSIONS FIRST.", height, width);

			return;
		}
		if (gtd->screen->tot_width == 0)
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #SCREEN MOVE %d %d: USE #SCREEN RAISE SCREEN DIMENSIONS FIRST.", height, width);

			return;
		}
		sprintf(arg2, "%d", 1 + gtd->screen->desk_width - gtd->screen->tot_width + width);
	}

	screen_csit(ses, "3", arg2, arg1); // reverse x,y to row,col
}

DO_SCREEN(screen_rescale)
{
	if (is_abbrev(arg1, "HORIZONTALLY"))
	{
		if (*arg2 == 0)
		{
			screen_csit(ses, "4", "", arg2);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RESCALE} {HORIZONTALLY} {[WIDTH]}");
		}
	}
	else if (is_abbrev(arg1, "VERTICALLY"))
	{
		if (*arg2 == 0)
		{
			screen_csit(ses, "4", arg2, "");
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RESCALE} {VERTICALLY} {[HEIGHT]}");
		}
	}
	else if (*arg1 == 0 || is_math(ses, arg1))
	{
		if (*arg2 == 0 || is_math(ses, arg2))
		{
			screen_csit(ses, "4", arg1, arg2);
		}
		else
		{
			screen_rescale(ses, 0, arg, "SYNTAX", "");
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RESCALE} {[HEIGHT]} {[WIDTH]}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RESCALE} {VERTICALLY} {<HEIGHT>}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RESCALE} {HORIZONTALLY} {<WIDTH>}");
	}
}

DO_SCREEN(screen_pixel_resize)
{
	screen_csit(ses, "4", arg1, arg2);
}

DO_SCREEN(screen_restore)
{
	screen_csit(ses, "10", "2", "");
}

DO_SCREEN(screen_scrollbar)
{
	if (is_abbrev(arg1, "ON"))
	{
		SET_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLMODE);
		printf("\e[%d;%d;%d#t", ses->scroll->used, ses->scroll->used, get_scroll_rows(ses));
	}
	else if (is_abbrev(arg1, "OFF"))
	{
		DEL_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLMODE);

		printf("\e[0#t");
	}
	else if (is_abbrev(arg1, "HIDE"))
	{
		screen_csi("?", "30", "", "", "l");
	}
	else if(is_abbrev(arg1, "SHOW"))
	{
		screen_csi("?", "30", "", "", "h");
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {SCROLLBAR} {ON|OFF|HIDE|SHOW}");
	}
}

DO_SCREEN(screen_swap)
{
	command(ses, do_split, "%d %d %d %d %d", ses->split->sav_top_row, UMAX(0, gtd->screen->rows - inputline_rows(ses) - get_scroll_rows(ses) - 1), ses->split->sav_top_col, ses->split->sav_bot_col, get_scroll_rows(ses));
}

DO_SCREEN(screen_scrollregion)
{
	int top_row, top_col, bot_row, bot_col;

	if ((*arg1 && !is_math(ses, arg1)) || (*arg2 && !is_math(ses, arg2)))
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN SCROLLREGION {TOP ROW} {TOP COL} {BOT ROW} {BOT COL}");

		return;
	}

	top_row = get_number(ses, arg1);
	top_col = get_number(ses, arg2);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if ((*arg1 && !is_math(ses, arg1)) || (*arg2 && !is_math(ses, arg2)))
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN SCROLL {TOP ROW} {TOP COL} {BOT ROW} {BOT COL}");

		return;
	}

	bot_row = get_number(ses, arg1);
	bot_col = get_number(ses, arg2);

	if ((top_row|top_col|bot_row|bot_col) == 0)
	{
		command(ses, do_unsplit, "");

		return;
	}

	ses->split->sav_top_row = top_row;
	ses->split->sav_top_col = top_col;
	ses->split->sav_bot_row = bot_row;
	ses->split->sav_bot_col = bot_col;

	SET_BIT(ses->flags, SES_FLAG_SPLIT);
	SET_BIT(ses->flags, SES_FLAG_SCROLLSPLIT);

	init_split(ses, ses->split->sav_top_row, ses->split->sav_top_col, ses->split->sav_bot_row, ses->split->sav_bot_col);

	return;
}

DO_SCREEN(screen_inputregion)
{
	int top_row, top_col, bot_row, bot_col;

	if (is_abbrev(arg1, "RESET"))
	{
		init_input(ses, 0, 0, 0, 0);

		return;
	}

	if (*arg1 == 0 || *arg2 == 0 || !is_math(ses, arg1) || !is_math(ses, arg2))
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN INPUTREGION {TOP ROW} {TOP COL} {BOT ROW} {BOT COL}");

		return;
	}

	top_row = get_number(ses, arg1);
	top_col = get_number(ses, arg2);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if ((*arg1 && !is_math(ses, arg1)) || (*arg2 && !is_math(ses, arg2)))
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN INPUT {TOP ROW} {TOP COL} {BOT ROW} {BOT COL}");

		return;
	}

	bot_row = get_number(ses, arg1);
	bot_col = get_number(ses, arg2);

	init_input(ses, top_row, top_col, bot_row, bot_col);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	str_cpy(&ses->input->line_name, arg1);

	goto_pos(ses, ses->input->top_row, ses->input->top_col);

	cursor_redraw_input(ses, "");

	return;
}

DO_SCREEN(screen_load)
{
	if (is_abbrev(arg1, "LABEL"))
	{
		screen_csit(ses, "23", "1", "");
	}
	else if (is_abbrev(arg1, "BOTH") || is_abbrev(arg1, "NAME"))
	{
		screen_csit(ses, "23", "0", "");
	}
	else if (is_abbrev(arg1, "TITLE"))
	{
		screen_csit(ses, "23", "2", "");
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {LOAD} {LABEL|NAME|TITLE}");
	}
}

DO_SCREEN(screen_refresh)
{
	if (HAS_BIT(ses->flags, SES_FLAG_SPLIT) && ses->wrap != gtd->screen->cols)
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_SESSIONUPDATE);

		SET_BIT(ses->flags, SES_FLAG_PRINTLINE);
	}
}

DO_SCREEN(screen_resize)
{
	if (is_abbrev(arg1, "HORIZONTALLY"))
	{
		if (*arg2 == 0 || is_math(ses,arg2))
		{
			screen_csit(ses, "8", " ", arg2);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {SIZE} {HORIZONTALLY} {[COLS]}");
		}
	}
	else if (is_abbrev(arg1, "VERTICALLY"))
	{
		if (*arg2 == 0 || is_math(ses, arg2))
		{
			screen_csit(ses, "8", arg2, " ");
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {SIZE} {VERTICALLY} {[ROWS]}");
		}
	}
	else if (*arg1 != 0 || *arg2 != 0)
	{
		if (*arg1 != 0 && *arg2 != 0 && is_math(ses, arg1) && is_math(ses, arg2))
		{
			screen_csit(ses, "8", arg1, arg2);
		}
		else if (*arg1 == 0 && is_math(ses, arg2))
		{
			screen_csit(ses, "8", " ", arg2);
		}
		else if (*arg2 == 0 && is_math(ses, arg1))
		{
			screen_csit(ses, "8", arg1, " ");
		}
		else
		{
			screen_resize(ses, 0, arg, "", "");
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {SIZE} {[ROWS]} {[COLS]}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {SIZE} {VERTICALLY} {<ROWS>}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {SIZE} {HORIZONTALLY} {<COLS>}");
	}
}

DO_SCREEN(screen_save)
{
	if (is_abbrev(arg1, "LABEL"))
	{
		screen_csit(ses, "22", "1", "");
	}
	else if (is_abbrev(arg1, "BOTH"))
	{
		screen_csit(ses, "22", "0", "");
	}
	else if (is_abbrev(arg1, "TITLE"))
	{
		screen_csit(ses, "22", "2", "");
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {SAVE} {BOTH|LABEL|TITLE}");
	}
}

DO_SCREEN(screen_set)
{
	if (is_abbrev(arg1, "BOTH") || is_abbrev(arg1, "NAME"))
	{
		screen_osc("0", arg2);
	}
	else if (is_abbrev(arg1, "COLS"))
	{
		gtd->screen->cols = get_number(ses, arg2);
	}
	else if (is_abbrev(arg1, "LABEL"))
	{
		screen_osc("1", arg2);
	}
	else if (is_abbrev(arg1, "ROWS"))
	{
		gtd->screen->rows = get_number(ses, arg2);
	}
	else if (is_abbrev(arg1, "TITLE"))
	{
		screen_osc("2", arg2);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {SET} {COLS|ROWS|LABEL|NAME|TITLE}");
	}
}

DO_SCREEN(screen_raise)
{
	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN CHARACTER DIMENSIONS}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN DESKTOP DIMENSIONS}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN DIMENSIONS}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN MINIMIZED}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN MOUSE LOCATION}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN LOCATION}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN RESIZE}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN SIZE}");

		return;
	}

	if (is_abbrev("SCREEN ", arg1))
	{
		arg1 += 7;
	}

	if (is_abbrev(arg1, "CHARACTER DIMENSIONS"))
	{
		screen_csit(ses, "16", "", "");
	}
	else if (is_abbrev(arg1, "DESKTOP DIMENSIONS"))
	{
		screen_csit(ses, "15", "", "");
	}
	else if (is_abbrev(arg1, "MINIMIZED"))
	{
		screen_csit(ses, "11", "", "");
	}
	else if (is_abbrev(arg1, "MOUSE LOCATION"))
	{
		print_stdout(0, 0, "%s", "\e[2;1'z\e['|");
	}
	else if (is_abbrev(arg1, "LOCATION") || is_abbrev(arg1, "POSITION"))
	{
		screen_csit(ses, "13", "", "");
	}
	else if (is_abbrev(arg1, "RESIZE"))
	{
		check_all_events(NULL, EVENT_FLAG_SCREEN, 0, 4, "SCREEN RESIZE", ntos(gtd->screen->rows), ntos(gtd->screen->cols), ntos(gtd->screen->height), ntos(gtd->screen->width));
	}
	else if (is_abbrev(arg1, "SIZE"))
	{
		screen_csit(ses, "18", "", "");
	}
	else if (is_abbrev(arg1, "DIMENSIONS"))
	{
		screen_csit(ses, "14", "2", "");
	}
	else
	{
		screen_raise(ses, 0, "", "", "");
	}
}

int get_row_index_arg(struct session *ses, char *arg)
{
	int val;

	if (*arg && is_math(ses, arg))
	{
		val = get_number(ses, arg);
	}
	else
	{
		val = 0;
	}

	return get_row_index(ses, val);
}

int get_row_index(struct session *ses, int val)
{
	if (val < 0)
	{
		val = 1 + gtd->screen->rows + val;
	}

	if (val > gtd->screen->rows)
	{
		val = gtd->screen->rows;
	}

	return val;
}


int get_col_index_arg(struct session *ses, char *arg)
{
	int val;

	if (*arg && is_math(ses, arg))
	{
		val = get_number(ses, arg);
	}
	else
	{
		val = 0;
	}
	return get_col_index(ses, val);
}

int get_col_index(struct session *ses, int val)
{
	if (val < 0)
	{
		val = 1 + gtd->screen->cols + val;
	}

	if (val > gtd->screen->cols)
	{
		val = gtd->screen->cols;
	}

	return val;
}



void csit_handler(int ind, int var1, int var2)
{
//	tintin_printf2(gtd->ses, "csit_handler(%d,%d,%d)",ind,var1,var2);

	switch (ind)
	{
		case 1:
			gtd->screen->minimized = 0;
			check_all_events(NULL, EVENT_FLAG_SCREEN, 0, 1, "SCREEN MINIMIZED", "0");
			msdp_update_all("SCREEN_MINIMIZED", "0");
			break;

		case 2:
			gtd->screen->minimized = 1;
			check_all_events(NULL, EVENT_FLAG_SCREEN, 0, 1, "SCREEN MINIMIZED", "1");
			msdp_update_all("SCREEN_MINIMIZED", "1");
			break;

		case 3:
			gtd->screen->pos_height = UMAX(0, var2);
			gtd->screen->pos_width  = UMAX(0, var1);
			check_all_events(NULL, EVENT_FLAG_SCREEN, 0, 4, "SCREEN LOCATION", ntos(var2 / gtd->screen->char_height), ntos(var1 / gtd->screen->char_width), ntos(var2), ntos(var1)); // swap x y
			msdp_update_all("SCREEN_LOCATION_HEIGHT", "%d", gtd->screen->pos_height);
			msdp_update_all("SCREEN_LOCATION_WIDTH", "%d", gtd->screen->pos_width);
			break;

		case 4:
			gtd->screen->tot_height = UMAX(0, var1);
			gtd->screen->tot_width  = UMAX(0, var2);
			check_all_events(NULL, EVENT_FLAG_SCREEN, 0, 2, "SCREEN DIMENSIONS", ntos(var1), ntos(var2));
			break;

		case 5:
			gtd->screen->desk_height = var1;
			gtd->screen->desk_width  = var2;
			check_all_events(NULL, EVENT_FLAG_SCREEN, 0, 2, "SCREEN DESKTOP DIMENSIONS", ntos(var1), ntos(var2));
			break;

		case 6:
			gtd->screen->char_height = var1;
			gtd->screen->char_width  = var2;
			check_all_events(NULL, EVENT_FLAG_SCREEN, 0, 2, "SCREEN CHARACTER DIMENSIONS", ntos(var1), ntos(var2));
			msdp_update_all("SCREEN_CHARACTER_HEIGHT", "%d", gtd->screen->char_height);
			msdp_update_all("SCREEN_CHARACTER_WIDTH", "%d", gtd->screen->char_width);
			break;

		case 7:
			init_screen(gtd->screen->rows, gtd->screen->cols, gtd->screen->height, gtd->screen->width);
			check_all_events(NULL, EVENT_FLAG_SCREEN, 0, 4, "SCREEN REFRESH", ntos(gtd->screen->rows), ntos(gtd->screen->cols), ntos(gtd->screen->height), ntos(gtd->screen->width));
			break;

		case 8:
			gtd->screen->rows = var1;
			gtd->screen->cols = var2;

			check_all_events(NULL, EVENT_FLAG_SCREEN, 0, 2, "SCREEN SIZE", ntos(var1), ntos(var2));
			msdp_update_all("SCREEN_ROWS", "%d", gtd->screen->rows);
			msdp_update_all("SCREEN_COLS", "%d", gtd->screen->cols);
			break;

		case 9:
			gtd->screen->desk_rows = var1;
			gtd->screen->desk_cols = var2;

			check_all_events(NULL, EVENT_FLAG_SCREEN, 0, 2, "SCREEN DESKTOP SIZE", ntos(var1), ntos(var2));
			msdp_update_all("SCREEN_DESKTOP_ROWS", "%d", gtd->screen->desk_rows);
			msdp_update_all("SCREEN_DESKTOP_COLS", "%d", gtd->screen->desk_cols);
			break;
	}
}

void rqlp_handler(int event, int button, int height, int width)
{
	int row, col, rev_row, rev_col, char_height, char_width, rev_char_height, rev_char_width, debug, info, grid_val;
	char *grid[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};

	row = 1 + height / UMAX(1, gtd->screen->char_height);
	col = 1 + width / UMAX(1, gtd->screen->char_width);

	char_height = 1 + height % UMAX(1, gtd->screen->char_height);
	char_width  = 1 + width % UMAX(1, gtd->screen->char_width);

	rev_row = -1 - (gtd->screen->rows - row);
	rev_col = -1 - (gtd->screen->cols - col);

	rev_char_height = -1 - (gtd->screen->char_height - char_height);
	rev_char_width  = -1 - (gtd->screen->char_width - char_width);

	grid_val = URANGE(0, char_height * 3 / gtd->screen->char_height, 2) * 3 + URANGE(0, char_width * 3 / gtd->screen->char_width, 2);

	debug = HAS_BIT(gtd->ses->config_flags, CONFIG_FLAG_MOUSEDEBUG) ? 1 : 0;
	info  = HAS_BIT(gtd->ses->config_flags, CONFIG_FLAG_MOUSEINFO) ? 1 : 0;

	gtd->level->debug += debug;
	gtd->level->info  += info;

	check_all_events(gtd->ses, EVENT_FLAG_SCREEN, 0, 9, "SCREEN MOUSE LOCATION", ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), ntos(char_height), ntos(char_width), ntos(rev_char_height), ntos(rev_char_width), grid[grid_val]);

	map_mouse_handler(gtd->ses, NULL, NULL, row, col, -1 - (gtd->screen->rows - row), -1 - (gtd->screen->cols - col), char_height, char_width);

	gtd->level->debug -= debug;
	gtd->level->info  -= info;

//	tintin_printf2(gtd->ses, "rqlp_handler(%d,%d,%d,%d) (%d,%d,%d,%d) (%d,%d,%d,%d)", event,button,height,width, row,col,rev_row,rev_col, char_height,char_width,rev_char_height,rev_char_width);
}

void screen_osc(char *arg1, char *arg2)
{
	print_stdout(0, 0, "\e]%s;%s\a", arg1, arg2);
}

void osc_handler(char ind, char *arg)
{
	tintin_printf2(gtd->ses, "osc debug: [%c] (%s)", ind, arg);
}

void screen_csi(char *cmd, char *num1, char *num2, char *num3, char *tc)
{

	print_stdout(0, 0, "\e[%s%s%s%s%s%s%s",
		cmd,
		*num1 ? XT_S : XT_V, *num1 && *num1 != ' ' ? num1 : "",
		*num2 ? XT_S : XT_V, *num2 && *num2 != ' ' ? num2 : "",
		num3,
		tc);
}

void screen_csit(struct session *ses, char *arg1, char *arg2, char *arg3)
{
	char num1[NUMBER_SIZE], num2[NUMBER_SIZE];

	if (*arg2 && *arg2 != ' ' && is_math(ses, arg2))
	{
		get_number_string(ses, arg2, num1);
	}
	else
	{
		strcpy(num1, arg2);
	}

	if (*arg3 && *arg3 != ' ' && is_math(ses, arg3))
	{
		get_number_string(ses, arg3, num2);
	}
	else
	{
		strcpy(num2, arg3);
	}

	print_stdout(0, 0, "\e[%s%s%s%s%st", arg1, *num1 ? XT_S : XT_V, *num1 && *num1 != ' ' ? num1 : "", *num2 ? XT_S : XT_V, *num2 && *num2 != ' ' ? num2 : "");

//	convert_meta(buf, debug, FALSE);

//	tintin_printf2(gtd->ses, "\e[1;32m[%s] num1 (%s) num2 (%s) %s %s", num1, num2, debug, buf);
}





/*
	save cursor, goto top row, delete (bot - top) rows, restore cursor
*/

void erase_scroll_region(struct session *ses)
{
	int row;

	push_call("erase_scroll_region(%p) [%d,%d]",ses,ses->split->top_row,ses->split->bot_row);

	save_pos(ses);

	for (row = ses->split->top_row ; row <= ses->split->bot_row ; row++)
	{
		if (row > 0 && row <= gtd->ses->split->bot_row)
		{
			str_cpy(&gtd->screen->line[row - 1]->str, "");
		}

		print_stdout(0, 0, "\e[%d;%dH\e[%dX", row, ses->split->top_col, ses->wrap);
	}
	restore_pos(ses);

	pop_call();
	return;
}

void erase_input_region(struct session *ses)
{
	int row;

	save_pos(ses);

	for (row = ses->input->top_row ; row <= ses->input->bot_row ; row++)
	{
		print_stdout(0, 0, "\e[%d;%dH\e[%dX", row, ses->input->top_col, ses->input->bot_col - ses->input->top_col);
	}
	restore_pos(ses);
}

void erase_split_region(struct session *ses)
{
	erase_top_region(ses);

	erase_bot_region(ses);

	erase_left_region(ses);

	erase_right_region(ses);
}

void erase_top_region(struct session *ses)
{
	int row;

	if (ses->split->top_row > 1)
	{
		save_pos(ses);
		goto_pos(ses, 1, 1);

		for (row = 1 ; row < ses->split->top_row ; row++)
		{
			print_stdout(0, 0, "\e[K\n");
		}
		restore_pos(ses);
	}
}

void erase_bot_region(struct session *ses)
{
	int row;

	if (ses->split->bot_row < gtd->screen->rows)
	{
		save_pos(ses);
		goto_pos(ses, ses->split->bot_row + 1, 1);

		for (row = ses->split->bot_row + 1 ; row < gtd->screen->rows ; row++)
		{
			print_stdout(0, 0, "\e[K\n");
		}
		restore_pos(ses);
	}
}

void erase_left_region(struct session *ses)
{
	int row;

	if (ses->split->top_col > 1)
	{
		save_pos(ses);

		for (row = ses->split->top_row ; row <= ses->split->bot_row ; row++)
		{
			print_stdout(0, 0, "\e[%d;1H\e[%dX", row, ses->split->top_col - 1);
		}
		restore_pos(ses);
	}
}

void erase_right_region(struct session *ses)
{
	int row;

	if (ses->split->bot_col < gtd->screen->cols)
	{
		save_pos(ses);

		for (row = ses->split->top_row ; row <= ses->split->bot_row ; row++)
		{
			print_stdout(0, 0, "\e[%d;%dH\e[K", row, ses->split->bot_col + 1);
		}
		restore_pos(ses);
	}
}


void erase_square(struct session *ses, int top_row, int top_col, int bot_row, int bot_col)
{
	int row;

	push_call("erase_square(%p,%d,%d,%d,%d)",ses,top_row,top_col,bot_row,bot_col);

	save_pos(ses);

	for (row = top_row ; row <= bot_row ; row++)
	{
		goto_pos(ses, row, top_col);
		print_stdout(0, 0, "\e[%dX", bot_col - top_col + 1);
	}
	restore_pos(ses);

	pop_call();
	return;
}

void fill_scroll_region(struct session *ses, char *arg)
{
	int row, col;

	save_pos(ses);

	for (row = ses->split->top_row ; row < ses->split->bot_row ; row++)
	{
		goto_pos(ses, row, ses->split->top_col);

		for (col = ses->split->top_col ; col < ses->split->bot_col ; col++)
		{
			print_stdout(0, 0, "%s", arg);
		}
		print_stdout(0, 0, "\n");
	}
	restore_pos(ses);
}

void fill_top_region(struct session *ses, char *arg)
{
	int row, col;

	if (ses->split->top_row > 1)
	{
		save_pos(ses);
		goto_pos(ses, 1, 1);

		for (row = 1 ; row < ses->split->top_row ; row++)
		{
			print_stdout(0, 0, "\e[0m");

			for (col = 0 ; col < gtd->screen->cols ; col++)
			{
				print_stdout(0, 0, "%s", arg);
			}
			print_stdout(0, 0, "\n");
		}
		restore_pos(ses);
	}
}

void fill_bot_region(struct session *ses, char *arg)
{
	int row, col;

	if (ses->split->bot_row < gtd->screen->rows)
	{
		save_pos(ses);
		goto_pos(ses, ses->split->bot_row + 1, 1);

		for (row = ses->split->bot_row + 1 ; row < gtd->screen->rows ; row++)
		{
			print_stdout(0, 0, "\e[0m");
			for (col = 0 ; col < gtd->screen->cols ; col++)
			{
				print_stdout(0, 0, "%s", arg);
			}
			print_stdout(0, 0, "\n");
		}
		restore_pos(ses);
	}
}

void fill_left_region(struct session *ses, char *arg)
{
	int row, col;

	if (ses->split->top_col > 1)
	{
		save_pos(ses);

		for (row = ses->split->top_row ; row <= ses->split->bot_row ; row++)
		{
			print_stdout(0, 0, "\e[%d;1H\e[0m", row);

			for (col = 0 ; col < ses->split->top_col - 1 ; col++)
			{
				print_stdout(0, 0, "%s", arg);
			}
		}
		restore_pos(ses);
	}
}

void fill_right_region(struct session *ses, char *arg)
{
	int row, col;

	if (ses->split->bot_col < gtd->screen->cols)
	{
		save_pos(ses);

		for (row = ses->split->top_row ; row <= ses->split->bot_row ; row++)
		{
			print_stdout(0, 0, "\e[%d;%dH\e[0m", row, ses->split->bot_col + 1);

			for (col = ses->split->bot_col + 1 ; col <= gtd->screen->cols ; col++)
			{
				print_stdout(0, 0, "%s", arg);
			}
		}
		restore_pos(ses);
	}
}

void fill_split_region(struct session *ses, char *arg)
{
	fill_top_region(ses, arg);

	fill_bot_region(ses, arg);

	fill_left_region(ses, arg);

	fill_right_region(ses, arg);
}

// VT SCREEN TECH

DO_SCREEN(screen_info)
{
	int lvl;

	if (is_abbrev(arg1, "SAVE"))
	{
		strcpy(arg2, "info[SCREEN]");

		set_nest_node_ses(ses, arg2, "{SCROLLMODE}{%d}", HAS_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLMODE));

		show_message(ses, LIST_COMMAND, "#INFO: DATA WRITTEN TO {info[SCREEN]}");
		
		return;
	}
	if (*arg1)
	{
		print_stdout(0, 0, "\e[2J");

		print_screen();

		return;
	}

	tintin_printf2(ses, "gtd->ses->split->sav_top_row: %4d", gtd->ses->split->sav_top_row);
	tintin_printf2(ses, "gtd->ses->split->sav_top_col: %4d", gtd->ses->split->sav_top_col);
	tintin_printf2(ses, "gtd->ses->split->sav_bot_row: %4d", gtd->ses->split->sav_bot_row);
	tintin_printf2(ses, "gtd->ses->split->sav_bot_col: %4d", gtd->ses->split->sav_bot_col);

	tintin_printf2(ses, "gtd->ses->split->top_row:     %4d", gtd->ses->split->top_row);
	tintin_printf2(ses, "gtd->ses->split->top_col:     %4d", gtd->ses->split->top_col);
	tintin_printf2(ses, "gtd->ses->split->bot_row:     %4d", gtd->ses->split->bot_row);
	tintin_printf2(ses, "gtd->ses->split->bot_col:     %4d", gtd->ses->split->bot_col);

	tintin_printf2(ses, "");

	tintin_printf2(ses, "gtd->ses->wrap:           %4d", gtd->ses->wrap);
	tintin_printf2(ses, "gtd->ses->cur_row:        %4d", gtd->ses->cur_row);
	tintin_printf2(ses, "gtd->ses->cur_col:        %4d", gtd->ses->cur_col);

	for (lvl = 0 ; lvl < gtd->screen->sav_lev ; lvl++)
	{
		tintin_printf2(ses, "gtd->screen->sav_row[%2d]: %4d", lvl, gtd->screen->sav_row[lvl]);
		tintin_printf2(ses, "gtd->screen->sav_col[%2d]: %4d", lvl, gtd->screen->sav_col[lvl]);
	}

	tintin_printf2(ses, "");

	tintin_printf2(ses, "gtd->screen->rows:        %4d", gtd->screen->rows);
	tintin_printf2(ses, "gtd->screen->cols:        %4d", gtd->screen->cols);
	tintin_printf2(ses, "gtd->screen->height:      %4d", gtd->screen->height);
	tintin_printf2(ses, "gtd->screen->width:       %4d", gtd->screen->width);
	tintin_printf2(ses, "gtd->screen->tot_height:  %4d", gtd->screen->tot_height);
	tintin_printf2(ses, "gtd->screen->tot_width:   %4d", gtd->screen->tot_width);
	tintin_printf2(ses, "gtd->screen->char_height: %4d", gtd->screen->char_height);
	tintin_printf2(ses, "gtd->screen->char_width:  %4d", gtd->screen->char_width);

	tintin_printf2(ses, "");

	tintin_printf2(ses, "gtd->screen->top_row:     %4d", gtd->screen->top_row);
	tintin_printf2(ses, "gtd->screen->bot_row:     %4d", gtd->screen->bot_row);
	tintin_printf2(ses, "gtd->screen->cur_row:     %4d", gtd->screen->cur_row);
	tintin_printf2(ses, "gtd->screen->cur_col:     %4d", gtd->screen->cur_col);
	tintin_printf2(ses, "gtd->screen->max_row:     %4d", gtd->screen->max_row);

	tintin_printf2(ses, "");

	tintin_printf2(ses, "gtd->screen->desk_rows:   %4d", gtd->screen->desk_rows);
	tintin_printf2(ses, "gtd->screen->desk_cols:   %4d", gtd->screen->desk_cols);
	tintin_printf2(ses, "gtd->screen->desk_height: %4d", gtd->screen->desk_height);
	tintin_printf2(ses, "gtd->screen->desk_width:  %4d", gtd->screen->desk_width);

	tintin_printf2(ses, "");

	tintin_printf2(ses, "gtd->screen->minimized:   %4d", gtd->screen->minimized);

	if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
	{
		tintin_printf2(ses, "SPLIT mode detected.");
	}


	return;
}

DO_SCREEN(screen_print)
{
	print_screen();
}

int inside_scroll_region(struct session *ses, int row, int col)
{
	if (row < ses->split->top_row)
	{
		return 0;
	}
	if (row > ses->split->bot_row)
	{
		return 0;
	}
	if (col < ses->split->top_col)
	{
		return 0;
	}
	if (col > ses->split->bot_col)
	{
		return 0;
	}
	return 1;
}

void add_row_index(struct row_data **row, int index)
{
	row[index] = (struct row_data *) calloc(1, sizeof(struct row_data));

	row[index]->str = str_dup("");
}

void del_row_index(struct row_data **row, int index)
{
	str_free(row[index]->str);

	free(row[index]);
}

void init_screen(int rows, int cols, int height, int width)
{
	int cnt;

	push_call("init_screen(%d,%d)",rows,cols);

	gtd->screen->rows        = UMAX(1, rows);
	gtd->screen->cols        = UMAX(1, cols);
	gtd->screen->height      = UMAX(1, height);
	gtd->screen->width       = UMAX(1, width);
	gtd->screen->char_height = UMAX(1, gtd->screen->height / gtd->screen->rows);
	gtd->screen->char_width  = UMAX(1, gtd->screen->width / gtd->screen->cols);

	gtd->screen->focus  = 1;

	gts->input->top_row = rows;
	gts->input->top_col = 1;
	gts->input->bot_row = rows;
	gts->input->bot_col = cols;

	gtd->screen->sav_lev    = 0;
	gtd->screen->sav_row[0] = gtd->screen->cur_row = rows;
	gtd->screen->sav_col[0] = gtd->screen->cur_col = 1;

	if (gtd->screen->max_row < rows)
	{
		gtd->screen->line = (struct row_data **) realloc(gtd->screen->line, rows * sizeof(struct row_data *));
		gtd->screen->grid = (struct row_data **) realloc(gtd->screen->grid,  rows * sizeof(struct row_data *));

		for (cnt = gtd->screen->max_row ; cnt < rows ; cnt++)
		{
			gtd->screen->line[cnt]      = (struct row_data *) calloc(1, sizeof(struct row_data));
			gtd->screen->line[cnt]->str = str_dup("");

			gtd->screen->grid[cnt]       = (struct row_data *) calloc(1, sizeof(struct row_data));
			gtd->screen->grid[cnt]->str  = str_dup("");
		}
		gtd->screen->max_row = rows;
	}

	pop_call();
	return;
}


void get_line_screen(char *result, int row)
{
	strcpy(result, gtd->screen->line[row]->str);
}

void get_word_screen(char *result, int row, int col)
{
	char *ptr;
	int i, j;

	strip_vt102_codes(gtd->screen->line[row]->str, result);

	ptr = result;

	if (!is_alnum(ptr[col]) && ptr[col] != '_')
	{
		sprintf(result, "%c", ptr[col]);

		return;
	}

	for (i = col ; i >= 0 ; i--)
	{
		if (!is_alnum(ptr[i]) && ptr[i] != '_')
		{
			break;
		}
	}
	i++;

	for (j = col ; ptr[j] ; j++)
	{
		if (!is_alnum(ptr[j]) && ptr[j] != '_')
		{
			break;
		}
	}

	strncpy(result, &ptr[i], j - i);

	result[j - i] = 0;
}

int get_link_screen(struct session *ses, char *var, char *val, int flags, int row, int col)
{
	char *pts, *ptl, *ptw;
	int skip, width, len, start, opt, level;

	ptl     = NULL;

	*var = *val = start = len = opt = level = 0;

	if (inside_scroll_region(ses, row, col))
	{
		col -= ses->split->top_col;
		pts = gtd->screen->line[row - 1]->str;
		ptw = pts;
	}
	else
	{
		col -= 1;
		pts = gtd->screen->grid[row - 1]->str;
		ptw = pts;
	}

	while (*pts)
	{
		start:

		if (*pts == ASCII_ESC)
		{
//			tintin_printf2(gtd->ses, "link debug: %3d %c %c", *pts, pts[1], pts[2]);

			if (pts[1] == ']' && pts[2] == '6' && pts[3] == '8' && pts[4] == ';')
			{
				char *pto;
				int nest, state[100], last;

				var[0] = val[0] = state[0] = nest = last = opt = 0;

				pts += 5;

				opt = 0;

				while (is_digit(*pts))
				{
					opt = opt * 10 + (*pts++ - '0');
				}

				if (opt == 0 || *pts != ';')
				{
					show_error(ses, LIST_EVENT, "get_link_screen: invalid link: invalid option or missing semicolon.");

					return FALSE;
				}
				pts++;

				pto = var;

				while (is_varchar(*pts))
				{
					*pto++ = *pts++;
				}

				*pto = 0;

				if (*pts++ != ';')
				{
					show_error(ses, LIST_EVENT, "get_link_screen: invalid link: missing semicolon.");

					return FALSE;
				}

				pto = val;

				if (opt != 2 && *pts == gtd->tintin_char)
				{
					pto += sprintf(pto, "\\x%x", *pts++);
				}

				while (*pts)
				{
					switch (*pts)
					{
						case DEFAULT_OPEN:
							level++;
							*pto++ = *pts++;
							break;
						case DEFAULT_CLOSE:
							level--;
							*pto++ = *pts++;
							break;

						case ASCII_BEL:
							*pto = 0;

							if (level)
							{
								if (level < 0)
								{
									show_error(ses, LIST_EVENT, "get_link_screen: invalid link: missing %d opening braces.", abs(level));
								}
								else
								{
									show_error(ses, LIST_EVENT, "get_link_screen: invalid link: missing %d closing braces.", abs(level));
								}
								return FALSE;
							}
							pts++;
//							tintin_printf2(gtd->ses, "link osc: %s opt: %d", result, opt);
							goto start;
							break;

						case '\\':
						case '$':
						case '*':
						case '@':
						case COMMAND_SEPARATOR:
							if (opt != 2)
							{
								*pto++ = '\\';
								*pto++ = *pts++;
							}
							else
							{
								*pto++ = *pts++;
							}
							break;

						default:
							*pto++ = *pts++;
							break;
					}
				}
				show_error(ses, LIST_EVENT, "get_link_screen: invalid link: missing string terminator");

				return 0;
			}
			else if (pts[1] == '[' && pts[2] == '4')
			{
				if (pts[3] == 'm')
				{
					ptl   = &pts[4];
					start = len;
				}
				else if (pts[3] == ';' && pts[4] == '2' && pts[5] == '4' && pts[6] == 'm')
				{
					ptl   = &pts[7];
					start = len;
				}
			}
			else if (pts[1] == '[' && pts[2] == '2' && pts[3] == '4' && pts[4] == 'm')
			{
//				tintin_printf2(gtd->ses, "\e[1;32mfound link: (%d,%d,%d) [%s]", start,col, len, val);

				if (ptl && col >= start && col < len)
				{
					if (*val == 0)
					{
						sprintf(val, "%.*s", (int) (pts - ptl), ptl);
					}
					return opt ? opt : 1;
				}
				else
				{
					ptl = NULL;
					*val = 0;
				}
			}
		}
		else if (*pts == ' ')
		{
			if (ptl == NULL && len >= col)
			{
				break;
			}
			ptw = pts;
		}

		skip = get_vt102_width(gtd->ses, pts, &width);

		len += width;
		pts += skip;
	}

	if (ptl && col >= start && col < len)
	{
		if (*val == 0)
		{
			sprintf(val, "%.*s", (int) (pts - ptl), ptl);
		}
		return opt ? opt : 1;
	}

	sprintf(val, "%.*s", (int) (pts - ptw), ptw);

	return FALSE;
}

void set_grid_screen(struct session *ses, char *ins, int row, int col)
{
	str_ins_str(ses, &gtd->screen->grid[row]->str, ins, col, col + strip_vt102_strlen(ses, ins));
}


void destroy_screen()
{
	int cnt;

//	disabled for now

	return;

	for (cnt = 0 ; cnt < gtd->screen->max_row ; cnt++)
	{
		del_row_index(gtd->screen->line, cnt);
	}
	free(gtd->screen->line);
	free(gtd->screen);

	gtd->screen = NULL;
}

void print_scroll_region(struct session *ses)
{
	int cnt;

	save_pos(ses);

	for (cnt = ses->split->top_row ; cnt < ses->split->bot_row ; cnt++)
	{
		print_stdout(0, 0, "\e[%d;%dH\e[%dX%s", cnt, ses->split->top_col, ses->wrap, gtd->screen->line[cnt - 1]->str);
	}
	print_stdout(0, 0, "\e[%d;%dH\e[%dX%s", cnt, ses->split->top_col, ses->wrap, ses->scroll->input);

	restore_pos(ses);
}

void print_screen()
{
	int cnt;

	for (cnt = 0 ; cnt < gtd->screen->rows ; cnt++)
	{
		print_stdout(0, 0, "\e[%dH%02d%s", cnt + 1, cnt + 1, gtd->screen->line[cnt]->str);
	}
}

void set_line_screen(struct session *ses, char *ins, int row, int col)
{
	push_call("set_line_screen(%p,%p,%d,%d)",ses,ins,row,col);

	if (row <= 0 || row > gtd->screen->rows)
	{
		tintin_printf2(ses, "set_line_screen debug: row = %d (%d)", row, gtd->screen->rows);

		pop_call();
		return;
	}

	if (col <= 0 || col > gtd->screen->cols)
	{
		tintin_printf2(ses, "set_line_screen debug: col = %d (%d)", col, gtd->screen->cols);

		dump_stack();

		pop_call();
		return;
	}

	if (inside_scroll_region(ses, row, col))
	{
		col -= ses->split->top_col;

		str_ins_str(ses, &gtd->screen->line[row - 1]->str, ins, col, -1);
	}
	else
	{
		col -= 1;

		str_ins_str(ses, &gtd->screen->grid[row - 1]->str, ins, col, -1);
	}
	pop_call();
	return;
}

void add_line_screen(struct session *ses, char *str, int row)
{
	char *ptr, *tmp;
	int cnt;

	push_call("add_line_screen(%p,%p,%d)",ses,str,row);

	if (gtd->screen == NULL)
	{
		print_stdout(0, 0, "screen == NULL!\n");

		pop_call();
		return;
	}

	if (ses != gtd->ses)
	{
		pop_call();
		return;
	}

	if (row)
	{
		str_cpy(&gtd->screen->line[row - 1]->str, str);

		pop_call();
		return;
	}

	while (str)
	{
		cnt = gtd->ses->split->top_row - 1;

		if (cnt < 0 || cnt >= gtd->ses->split->bot_row)
		{
			tintin_printf2(gtd->ses, "add_line_screen debug: cnt = %d", cnt);
		}

		tmp = gtd->screen->line[cnt]->str;

		while (cnt < gtd->ses->split->bot_row - 2)
		{
			gtd->screen->line[cnt]->str = gtd->screen->line[cnt + 1]->str;

			cnt++;
		}

		gtd->screen->line[cnt]->str = tmp;

		ptr = strchr(str, '\n');

		if (ptr)
		{
			str_ncpy(&gtd->screen->line[cnt]->str, str, ptr - str);

			str = ptr + 1;
		}
		else
		{
			str_cpy(&gtd->screen->line[cnt]->str, str);

			str = NULL;
		}
//		gtd->screen->line[cnt]->raw_len = strlen(gtd->screen->line[cnt]->str);
//		gtd->screen->line[cnt]->str_len = strip_vt102_strlen(gts, gtd->screen->line[cnt]->str);
	}

	pop_call();
	return;
}
