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
*                     coded by Igor van den Hoven 2019                        *
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
				if (HAS_BIT(screen_table[cnt].get1, SCREEN_FLAG_GET_ONE))
				{
					arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
				}
				else
				{
					arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);
				}
				if (HAS_BIT(screen_table[cnt].get2, SCREEN_FLAG_GET_ONE))
				{
					arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
				}
				else
				{
					arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);
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
		screen_csi("?", "25", "", "", "l");
	}
	else if (is_abbrev(arg1, "SHOW"))
	{
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
		printf("\e[2J");
	}
	else if (is_abbrev(arg1, "SCROLL REGION"))
	{
//		erase_scroll_region(ses);

		printf("\e[%d;%d;%d;%d${", ses->top_row, 1, ses->bot_row, gtd->screen->cols);
	}
	else if (is_abbrev(arg1, "SQUARE"))
	{
		strcpy(arg1, arg2);
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

		top_row = get_row_index(ses, arg1);
		top_col = get_col_index(ses, arg2);

		tintin_printf2(ses, "debug: (%s) (%s) %d %d", arg1, arg2, top_row, top_col);

		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

		bot_row = get_row_index(ses, arg1);
		bot_col = get_col_index(ses, arg2);

		tintin_printf2(ses, "debug: (%s) (%s) %d %d", arg1, arg2, bot_row, bot_col);

		if (bot_col == 0)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN CLEAR SQUARE {<ROW>} {<COL>} {<ROW>} {<COL>}");
		}
		else
		{
			printf("\e[%d;%d;%d;%d${", top_row, top_col, bot_row, bot_col);
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN CLEAR {ALL|SCROLL REGION|SQUARE}");
	}
}

DO_SCREEN(screen_focus)
{
	screen_csit(ses, "5", "", "");
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

	height = (int) get_number(ses, arg2);
	width  = (int) get_number(ses, arg1);

	if (height < 0)
	{
		strcpy(arg2, "0");
	}

	if (width < 0)
	{
		strcpy(arg1, "0");
	}

	screen_csit(ses, "3", arg2, arg1); // reverse x,y to row,col
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


DO_SCREEN(screen_load)
{
	if (is_abbrev(arg1, "LABEL"))
	{
		screen_csit(ses, "23", "1", "");
	}
	else if (is_abbrev(arg1, "BOTH"))
	{
		screen_csit(ses, "23", "0", "");
	}
	else if (is_abbrev(arg1, "TITLE"))
	{
		screen_csit(ses, "23", "2", "");
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {LOAD} {BOTH|LABEL|TITLE}");
	}
}

DO_SCREEN(screen_refresh)
{
	screen_csit(ses, "7", "", "");
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
	if (is_abbrev(arg1, "LABEL"))
	{
		screen_osc("1", arg2);
	}
	else if (is_abbrev(arg1, "BOTH"))
	{
		screen_osc("0", arg2);
	}
	else if (is_abbrev(arg1, "TITLE"))
	{
		screen_osc("2", arg2);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {SET} {BOTH|LABEL|TITLE}");
	}
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
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {GET} {FOCUS|ROWS|COLS|HEIGHT|WIDTH} {<VAR>}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {GET} {TOP_ROW|BOT_ROW|TOP_SPLIT|BOT_SPLIT} {<VAR>}");

		return;
	}

	if (is_abbrev(arg1, "FOCUS"))
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", gtd->screen->focus);
	}
	else if (is_abbrev(arg1, "ROWS"))
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", gtd->screen->rows);
	}
	else if (is_abbrev(arg1, "COLS"))
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", gtd->screen->cols);
	}
	else if (is_abbrev(arg1, "HEIGHT"))
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", gtd->screen->height);
	}
	else if (is_abbrev(arg1, "WIDTH"))
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", gtd->screen->width);
	}
	else if (is_abbrev(arg1, "TOP_ROW"))
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", ses->top_row);
	}
	else if (is_abbrev(arg1, "TOP_SPLIT"))
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", ses->top_split);
	}
	else if (is_abbrev(arg1, "BOT_ROW"))
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", ses->bot_row);
	}
	else if (is_abbrev(arg1, "BOT_SPLIT"))
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", ses->bot_split);
	}
	else
	{
		screen_get(ses, 0, "", "", "");
	}
}

DO_SCREEN(screen_raise)
{
	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN CHARACTER DIMENSIONS}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN POSITION}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN MINIMIZED}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN RESIZE}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN DIMENSIONS}");
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCREEN {RAISE} {SCREEN DESKTOP DIMENSIONS}");

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
	else if (is_abbrev(arg1, "POSITION"))
	{
		screen_csit(ses, "13", "", "");
	}
	else if (is_abbrev(arg1, "RESIZE"))
	{
		check_all_events(NULL, SUB_ARG, 0, 4, "SCREEN RESIZE", ntos(gtd->screen->rows), ntos(gtd->screen->cols), ntos(gtd->screen->height), ntos(gtd->screen->width));
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
			gtd->screen->pos_height = var2;
			gtd->screen->pos_width  = var1;
			check_all_events(NULL, SUB_ARG, 0, 2, "SCREEN POSITION", ntos(var2), ntos(var1)); // swap x y
			msdp_update_all("SCREEN_POSITION_HEIGHT", "%d", gtd->screen->pos_height);
			msdp_update_all("SCREEN_POSITION_WIDTH", "%d", gtd->screen->pos_width);
			break;

		case 4:
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
	}
}

void screen_osc(char *arg1, char *arg2)
{
	printf("\e]%s;%s\a", arg1, arg2);
}

void osc_handler(char ind, char *arg)
{
	tintin_printf2(gtd->ses, "osc debug: [%c] (%s)", ind, arg);
}

void screen_csi(char *cmd, char *num1, char *num2, char *num3, char *tc)
{

	printf("\e[%s%s%s%s%s%s%s",
		cmd,
		*num1 ? XT_S : XT_V, *num1 && *num1 != ' ' ? num1 : "",
		*num2 ? XT_S : XT_V, *num2 && *num2 != ' ' ? num2 : "",
		num3,
		tc);

	fflush(NULL);
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

	printf("\e[%s%s%s%s%st", arg1, *num1 ? XT_S : XT_V, *num1 && *num1 != ' ' ? num1 : "", *num2 ? XT_S : XT_V, *num2 && *num2 != ' ' ? num2 : "");

//	convert_meta(buf, debug, FALSE);

//	tintin_printf2(gtd->ses, "\e[1;32m[%s] num1 (%s) num2 (%s) %s %s", num1, num2, debug, buf);


	fflush(NULL);
}


DO_SCREEN(screen_info)
{
	tintin_printf2(ses, "gtd->ses->top_row:     %4d", gtd->ses->top_row);
	tintin_printf2(ses, "gtd->ses->bot_row:     %4d", gtd->ses->bot_row);
	tintin_printf2(ses, "gtd->ses->cur_row:     %4d", gtd->ses->cur_row);
	tintin_printf2(ses, "gtd->ses->cur_col:     %4d", gtd->ses->cur_col);

	tintin_printf2(ses, "");

	tintin_printf2(ses, "gtd->screen->rows:     %4d", gtd->screen->rows);
	tintin_printf2(ses, "gtd->screen->cols:     %4d", gtd->screen->cols);
	tintin_printf2(ses, "gtd->screen->height: %4d", gtd->screen->height);
	tintin_printf2(ses, "gtd->screen->pix_cows: %4d", gtd->screen->width);

	tintin_printf2(ses, "");

	tintin_printf2(ses, "gtd->screen->top_row:  %4d", gtd->screen->top_row);
	tintin_printf2(ses, "gtd->screen->bot_row:  %4d", gtd->screen->bot_row);
	tintin_printf2(ses, "gtd->screen->sav_row:  %4d", gtd->screen->sav_row);
	tintin_printf2(ses, "gtd->screen->sav_col:  %4d", gtd->screen->sav_col);
	tintin_printf2(ses, "gtd->screen->cur_row:  %4d", gtd->screen->cur_row);
	tintin_printf2(ses, "gtd->screen->cur_col:  %4d", gtd->screen->cur_col);
	tintin_printf2(ses, "gtd->screen->max_row:  %4d", gtd->screen->max_row);
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

	if (gtd->screen == NULL)
	{
		gtd->screen = calloc(1, sizeof(struct screen_data));
	}

	gtd->screen->rows   = rows;
	gtd->screen->cols   = cols;
	gtd->screen->height = height;
	gtd->screen->width  = width;
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
	gtd->screen->sav_row = gtd->screen->cur_row = rows;

	gtd->screen->cols = cols;
	gtd->screen->sav_col = gtd->screen->cur_col = 0;

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
		printf("%2d %s\n", cnt, gtd->screen->lines[cnt]->str);
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
		printf("screen == NULL!\n");

		pop_call();
		return;
	}

	while (str)
	{
		cnt = gtd->ses->top_row - 1;

		free(gtd->screen->lines[cnt]->str);

		while (cnt < gtd->ses->bot_row - 2)
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
	push_call("erase_scroll_region(%p) (%d,%d)",ses,ses->top_row,ses->bot_row);

//	printf("\e[%d;%d;%d;%d${", ses->top_row, 1, ses->bot_row, gtd->screen->cols); VT400

	printf("\e7\e[%d;1H\e[%dM\e8", ses->top_row, ses->bot_row - ses->top_row);

	ses->sav_row = ses->cur_row;
	ses->sav_col = ses->cur_col;

	pop_call();
	return;
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
