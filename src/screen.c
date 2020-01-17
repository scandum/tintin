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

void screen_osc(char *arg1, char *arg2);
void screen_csi(char *cmd, char *arg1, char *arg2, char *arg3, char *tc);
void screen_csit(struct session *ses, char *arg1, char *arg2, char *tc);

DO_COMMAND(do_screen)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_header(ses, " SCREEN OPTIONS ");

		for (cnt = 0 ; *screen_table[cnt].fun ; cnt++)
		{
			if (*screen_table[cnt].name)
			{
				tintin_printf2(ses, "  [%-18s] [%-6s] %s", screen_table[cnt].name, "", screen_table[cnt].desc);
			}
		}
		tintin_header(ses, "");
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
		print_stdout("\e[2J");
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
	else if (is_abbrev(arg1, "SPLIT REGION"))
	{
		erase_split_region(ses);
	}
	else if (is_abbrev(arg1, "SQUARE"))
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

		top_row = get_row_index(ses, arg1);
		top_col = get_col_index(ses, arg2);

//		tintin_printf2(ses, "debug: (%s) (%s) %d %d", arg1, arg2, top_row, top_col);

		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

		bot_row = get_row_index(ses, arg1);
		bot_col = get_col_index(ses, arg2);

//		tintin_printf2(ses, "debug: (%s) (%s) %d %d", arg1, arg2, bot_row, bot_col);

		if (bot_col == 0)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN CLEAR SQUARE {<ROW>} {<COL>} {<ROW>} {<COL>}");
		}
		else
		{
			erase_square(ses, top_row, top_col, bot_row, bot_col);

//			print_stdout("\e[%d;%d;%d;%d${", top_row, top_col, bot_row, bot_col); VT400 not widely supported
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN CLEAR {ALL|SCROLL|SPLIT|SQUARE}");
	}
}

DO_SCREEN(screen_fill)
{
	char buf[BUFFER_SIZE];

	if (is_abbrev(arg1, "DEFAULT"))
	{
		if (ses->split->sav_top_col || ses->split->sav_bot_col)
		{
			sprintf(buf, "CLEAR SPLIT");

			do_screen(ses, buf);
		}

		if (ses->split->sav_top_row > 0)
		{
			if (ses->split->sav_top_row == 1)
			{
				sprintf(buf, "LINE %d %d %d %d", 1, 1, ses->split->top_row - 1, gtd->screen->cols);
			}
			else
			{
				sprintf(buf, "BOX %d %d %d %d {}", 1, 1, ses->split->top_row - 1, gtd->screen->cols);
			}
			do_draw(ses, buf);
		}

		if (ses->split->sav_bot_row)
		{
			if (ses->split->sav_bot_row == 1)
			{
				sprintf(buf, "LINE %d %d %d %d", ses->split->bot_row + 1, 1, gtd->screen->rows - 1, gtd->screen->cols);
			}
			else
			{
				sprintf(buf, "BOX %d %d %d %d {}", ses->split->bot_row + 1, 1, gtd->screen->rows - 1, gtd->screen->cols);
			}
			do_draw(ses, buf);
		}

		if (ses->split->sav_top_row > 0)
		{
			if (ses->split->sav_top_col)
			{
				sprintf(buf, "TEED VERTICAL LINE %d %d %d %d", ses->split->top_row - 1, ses->split->top_col - 1, ses->split->bot_row + 1, ses->split->top_col - 1);
				do_draw(ses, buf);
			}

			if (ses->split->sav_bot_col)
			{
				sprintf(buf, "TEED VERTICAL LINE %d %d %d %d", ses->split->top_row - 1, ses->split->bot_col + 1, ses->split->bot_row + 1, ses->split->bot_col + 1);
				do_draw(ses, buf);
			}
		}
		else
		{
			if (ses->split->sav_top_col)
			{
				sprintf(buf, "VERTICAL LINE %d %d %d %d", ses->split->top_row, ses->split->top_col - 1, ses->split->bot_row, ses->split->top_col - 1);
				do_draw(ses, buf);
			}

			if (ses->split->sav_bot_col)
			{
				sprintf(buf, "VERTICAL LINE %d %d %d %d", ses->split->top_row, ses->split->bot_col + 1, ses->split->bot_row, ses->split->bot_col + 1);
				do_draw(ses, buf);
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
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {GET} {FOCUS} {<VAR>}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {GET} {ROWS|COLS|HEIGHT|WIDTH} {<VAR>}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {GET} {CHAR_HEIGHT|CHAR_WIDTH}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {GET} {SPLIT_TOP_BAR|SPLIT_BOT_BAR|SPLIT_LEFT_BAR|SPLIT_RIGHT_BAR} {<VAR>}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {GET} {SCROLL_TOP_ROW|SCROLL_TOP_COL|SCROLL_BOT_ROW|SCROLL_BOT_COL} {<VAR>}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {GET} {CUR_ROW|CUR_COL} {<VAR>}");

		return;
	}

	if (is_abbrev(arg1, "FOCUS"))
	{
		set_nest_node_ses(ses, arg2, "%d", gtd->screen->focus);
	}
	else if (is_abbrev(arg1, "CHAR_HEIGHT"))
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
	else if (is_abbrev(arg1, "HEIGHT"))
	{
		set_nest_node_ses(ses, arg2, "%d", gtd->screen->height);
	}
	else if (is_abbrev(arg1, "WIDTH"))
	{
		set_nest_node_ses(ses, arg2, "%d", gtd->screen->width);
	}

	else if (is_abbrev(arg1, "ROWS"))
	{
		set_nest_node_ses(ses, arg2, "%d", gtd->screen->rows);
	}

	else if (is_abbrev(arg1, "SPLIT_TOP_BAR"))
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
		set_nest_node_ses(ses, arg2, "%d", ses->split->bot_row - ses->split->top_row);
	}
	else if (is_abbrev(arg1, "SCROLL_COLS"))
	{
		set_nest_node_ses(ses, arg2, "%d", ses->wrap);
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
	else
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
		sprintf(arg1, "%d", 1 + gtd->screen->desk_height - gtd->screen->tot_height + height);
	}

	if (width < 0)
	{
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
	if (is_abbrev(arg1, "HIDE"))
	{
		screen_csi("?", "30", "", "", "l");
	}
	else if(is_abbrev(arg1, "SHOW"))
	{
		screen_csi("?", "30", "", "", "h");
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {SCROLLBAR} {HIDE|SHOW}");
	}
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
		do_unsplit(ses, "");

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
	SET_BIT(ses->flags, SES_FLAG_PRINTBUFFER);

	buffer_end(ses, "");

	DEL_BIT(ses->flags, SES_FLAG_PRINTBUFFER);
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
	else if (*arg1 == 0 || is_math(ses, arg1))
	{
		if (*arg2 == 0 || is_math(ses, arg2))
		{
			screen_csit(ses, "8", arg1, arg2);
		}
		else
		{
			screen_resize(ses, 0, arg, "SYNTAX", "");
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
		print_stdout("%s", "\e[2;1'z\e['|");
	}
	else if (is_abbrev(arg1, "LOCATION") || is_abbrev(arg1, "POSITION"))
	{
		screen_csit(ses, "13", "", "");
	}
	else if (is_abbrev(arg1, "RESIZE"))
	{
		check_all_events(NULL, SUB_ARG, 0, 4, "SCREEN RESIZE", ntos(gtd->screen->rows), ntos(gtd->screen->cols), ntos(gtd->screen->height), ntos(gtd->screen->width));
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

int get_row_index(struct session *ses, char *arg)
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

int get_col_index(struct session *ses, char *arg)
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
			gtd->screen->minimized = 1;
			check_all_events(NULL, SUB_ARG, 0, 1, "SCREEN MINIMIZED", "1");
			msdp_update_all("SCREEN_MINIMIZED", "1");
			break;

		case 2:
			gtd->screen->minimized = 0;
			check_all_events(NULL, SUB_ARG, 0, 1, "SCREEN MINIMIZED", "0");
			msdp_update_all("SCREEN_MINIMIZED", "0");
			break;

		case 3:
			gtd->screen->pos_height = UMAX(0, var2);
			gtd->screen->pos_width  = UMAX(0, var1);
			check_all_events(NULL, SUB_ARG, 0, 4, "SCREEN LOCATION", ntos(var2 / gtd->screen->char_height), ntos(var1 / gtd->screen->char_width), ntos(var2), ntos(var1)); // swap x y
			msdp_update_all("SCREEN_LOCATION_HEIGHT", "%d", gtd->screen->pos_height);
			msdp_update_all("SCREEN_LOCATION_WIDTH", "%d", gtd->screen->pos_width);
			break;

		case 4:
			gtd->screen->tot_height = UMAX(0, var1);
			gtd->screen->tot_width  = UMAX(0, var2);
			check_all_events(NULL, SUB_ARG, 0, 2, "SCREEN DIMENSIONS", ntos(var1), ntos(var2));
			break;

		case 5:
			gtd->screen->desk_height = var1;
			gtd->screen->desk_width  = var2;
			check_all_events(NULL, SUB_ARG, 0, 2, "SCREEN DESKTOP DIMENSIONS", ntos(var1), ntos(var2));
			break;

		case 6:
			gtd->screen->char_height = var1;
			gtd->screen->char_width  = var2;
			check_all_events(NULL, SUB_ARG, 0, 2, "SCREEN CHARACTER DIMENSIONS", ntos(var1), ntos(var2));
			msdp_update_all("SCREEN_CHARACTER_HEIGHT", "%d", gtd->screen->char_height);
			msdp_update_all("SCREEN_CHARACTER_WIDTH", "%d", gtd->screen->char_width);
			break;

		case 7:
			init_screen(gtd->screen->rows, gtd->screen->cols, gtd->screen->height, gtd->screen->width);
			check_all_events(NULL, SUB_ARG, 0, 4, "SCREEN REFRESH", ntos(gtd->screen->rows), ntos(gtd->screen->cols), ntos(gtd->screen->height), ntos(gtd->screen->width));
			break;

		case 8:
			gtd->screen->rows = var1;
			gtd->screen->cols = var2;

			check_all_events(NULL, SUB_ARG, 0, 2, "SCREEN SIZE", ntos(var1), ntos(var2));
			msdp_update_all("SCREEN_ROWS", "%d", gtd->screen->rows);
			msdp_update_all("SCREEN_COLS", "%d", gtd->screen->cols);
			break;

		case 9:
			gtd->screen->desk_rows = var1;
			gtd->screen->desk_cols = var2;

			check_all_events(NULL, SUB_ARG, 0, 2, "SCREEN DESKTOP SIZE", ntos(var1), ntos(var2));
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

	debug = HAS_BIT(gtd->ses->flags, SES_FLAG_MOUSEDEBUG) ? 1 : 0;
	info  = HAS_BIT(gtd->ses->flags, SES_FLAG_MOUSEINFO) ? 1 : 0;

	gtd->level->debug += debug;
	gtd->level->info  += info;

	check_all_events(gtd->ses, SUB_ARG, 0, 9, "SCREEN MOUSE LOCATION", ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), ntos(char_height), ntos(char_width), ntos(rev_char_height), ntos(rev_char_width), grid[grid_val]);

	map_mouse_handler(gtd->ses, NULL, NULL, col, row, char_height, char_width);

	gtd->level->debug -= debug;
	gtd->level->info  -= info;

//	tintin_printf2(gtd->ses, "rqlp_handler(%d,%d,%d,%d) (%d,%d,%d,%d) (%d,%d,%d,%d)", event,button,height,width, row,col,rev_row,rev_col, char_height,char_width,rev_char_height,rev_char_width);
}

void screen_osc(char *arg1, char *arg2)
{
	print_stdout("\e]%s;%s\a", arg1, arg2);
}

void osc_handler(char ind, char *arg)
{
	tintin_printf2(gtd->ses, "osc debug: [%c] (%s)", ind, arg);
}

void screen_csi(char *cmd, char *num1, char *num2, char *num3, char *tc)
{

	print_stdout("\e[%s%s%s%s%s%s%s",
		cmd,
		*num1 ? XT_S : XT_V, *num1 && *num1 != ' ' ? num1 : "",
		*num2 ? XT_S : XT_V, *num2 && *num2 != ' ' ? num2 : "",
		num3,
		tc);

	SET_BIT(gtd->flags, TINTIN_FLAG_FLUSH);
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

	print_stdout("\e[%s%s%s%s%st", arg1, *num1 ? XT_S : XT_V, *num1 && *num1 != ' ' ? num1 : "", *num2 ? XT_S : XT_V, *num2 && *num2 != ' ' ? num2 : "");

//	convert_meta(buf, debug, FALSE);

//	tintin_printf2(gtd->ses, "\e[1;32m[%s] num1 (%s) num2 (%s) %s %s", num1, num2, debug, buf);


	SET_BIT(gtd->flags, TINTIN_FLAG_FLUSH);
}


DO_SCREEN(screen_info)
{
	int lvl;

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

	if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
	{
		tintin_printf2(ses, "SPLIT mode detected.");
	}
}

/*

	if (gtd->screen == NULL)
	{
		tintin_printf2(ses, "gtd->screen: NULL");

		return ses;
	}

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);


	print_screen(gts);

	return ses;
}
*/

void add_row_screen(int index)
{
	gtd->screen->lines[index] = (struct row_data *) calloc(1, sizeof(struct row_data));
	gtd->screen->lines[index]->str = strdup("");
}

void del_row_screen(int index)
{
	free(gtd->screen->lines[index]->str);
	free(gtd->screen->lines[index]);
}

void init_screen(int rows, int cols, int height, int width)
{
	int cnt;

	gtd->screen->rows        = UMAX(1, rows);
	gtd->screen->cols        = UMAX(1, cols);
	gtd->screen->height      = UMAX(1, height);
	gtd->screen->width       = UMAX(1, width);
	gtd->screen->char_height = UMAX(1, gtd->screen->height / gtd->screen->rows);
	gtd->screen->char_width  = UMAX(1, gtd->screen->width / gtd->screen->cols);

	gtd->screen->focus  = 1;

	return;

//	disabled for now

	push_call("init_screen(%d,%d)",rows,cols);

	if (gtd->screen)
	{
		if (gtd->screen->max_row < rows)
		{
			gtd->screen->lines = (struct row_data **) realloc(gtd->screen->lines, rows * sizeof(struct row_data *));
			
			memset(gtd->screen->lines + gtd->screen->max_row * sizeof(struct row_data *), 0, (rows - gtd->screen->max_row) * sizeof(struct row_data *));

			gtd->screen->max_row = rows;
		}
	}
	else
	{
		gtd->screen = calloc(1, sizeof(struct screen_data));
		gtd->screen->lines = (struct row_data **) calloc(rows, sizeof(struct row_data *));

		gtd->screen->top_row = 1;
		gtd->screen->bot_row = rows;

		gtd->screen->max_row = rows;
	}

	gtd->screen->rows = rows;
	gtd->screen->cols = cols;

	gtd->screen->sav_lev = 0;
	gtd->screen->sav_row[0] = gtd->screen->cur_row = rows;
	gtd->screen->sav_col[0] = gtd->screen->cur_col = 0;

	for (cnt = 0 ; cnt < gtd->screen->max_row ; cnt++)
	{
		if (gtd->screen->lines[cnt] == NULL)
		{
			add_row_screen(cnt);
		}
	}
	pop_call();
	return;
}

void destroy_screen()
{
	int cnt;

//	disabled for now

	return;

	for (cnt = 0 ; cnt < gtd->screen->max_row ; cnt++)
	{
		del_row_screen(cnt);
	}
	free(gtd->screen->lines);
	free(gtd->screen);

	gtd->screen = NULL;
}

void print_screen()
{
	int cnt;

	for (cnt = 0 ; cnt < gtd->screen->rows ; cnt++)
	{
		print_stdout("%2d %s\n", cnt, gtd->screen->lines[cnt]->str);
	}
}


void add_line_screen(char *str)
{
	char *ptr;
	int cnt;

	// disabled for now

	return;

	push_call("add_line_screen(%p)",str);

	if (gtd->screen == NULL)
	{
		print_stdout("screen == NULL!\n");

		pop_call();
		return;
	}

	while (str)
	{
		cnt = gtd->ses->split->top_row - 1;

		free(gtd->screen->lines[cnt]->str);

		while (cnt < gtd->ses->split->bot_row - 2)
		{
			gtd->screen->lines[cnt]->str = gtd->screen->lines[cnt + 1]->str;

			cnt++;
		}

		ptr = strchr(str, '\n');

		if (ptr)
		{
			gtd->screen->lines[cnt]->str = strndup(str, ptr - str);

			str = ptr + 1;
		}
		else
		{
			gtd->screen->lines[cnt]->str = strdup(str);

			str = NULL;
		}
//		gtd->screen->lines[cnt]->raw_len = strlen(gtd->screen->lines[cnt]->str);
//		gtd->screen->lines[cnt]->str_len = strip_vt102_strlen(gts, gtd->screen->lines[cnt]->str);
	}

	pop_call();
	return;
}



void set_line_screen(char *str, int row, int col)
{
	char buf[BUFFER_SIZE];

	// disabled for now
	
	return;

	push_call("set_line_screen(%p,%d)",str,row,col);

	strcpy(buf, gtd->screen->lines[row]->str);

	free(gtd->screen->lines[row]->str);

	strcpy(&buf[col], str);

	gtd->screen->lines[row]->str = strdup(buf);

	pop_call();
	return;
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
		print_stdout("\e[%d;%dH\e[%dX", row, ses->split->top_col, ses->wrap);
	}
	restore_pos(ses);

	pop_call();
	return;
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
			print_stdout("\e[K\n");
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
			print_stdout("\e[K\n");
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
			print_stdout("\e[%d;1H\e[%dX", row, ses->split->top_col - 1);
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
			print_stdout("\e[%d;%dH\e[K", row, ses->split->bot_col + 1);
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
		print_stdout("\e[%dX", bot_col - top_col + 1);
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
			print_stdout("%s", arg);
		}
		print_stdout("\n");
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
			print_stdout("\e[0m");

			for (col = 0 ; col < gtd->screen->cols ; col++)
			{
				print_stdout("%s", arg);
			}
			print_stdout("\n");
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
			print_stdout("\e[0m");
			for (col = 0 ; col < gtd->screen->cols ; col++)
			{
				print_stdout("%s", arg);
			}
			print_stdout("\n");
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
			print_stdout("\e[%d;1H\e[0m", row);

			for (col = 0 ; col < ses->split->top_col - 1 ; col++)
			{
				print_stdout("%s", arg);
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
			print_stdout("\e[%d;%dH\e[0m", row, ses->split->bot_col + 1);

			for (col = ses->split->bot_col + 1 ; col <= gtd->screen->cols ; col++)
			{
				print_stdout("%s", arg);
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

void get_line_screen(char *result, int row)
{
	strcpy(result, gtd->screen->lines[row]->str);
}

void get_word_screen(char *result, int row, int col)
{
	char *ptr;
	int i, j;

	strip_vt102_codes(gtd->screen->lines[row]->str, result);

	ptr = result;

	if (!isalnum((int) ptr[col]) && ptr[col] != '_')
	{
		sprintf(result, "%c", ptr[col]);

		return;
	}

	for (i = col ; i >= 0 ; i--)
	{
		if (!isalnum((int) ptr[i]) && ptr[i] != '_')
		{
			break;
		}
	}
	i++;

	for (j = col ; ptr[j] ; j++)
	{
		if (!isalnum((int) ptr[j]) && ptr[j] != '_')
		{
			break;
		}
	}

	strncpy(result, &ptr[i], j - i);

	result[j - i] = 0;
}
