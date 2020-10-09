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

static char draw_buf[100][10];
static int  draw_cnt;

#include "tintin.h"

DO_COMMAND(do_draw)
{
	char *color, *code1, *code2, *input;
	long long flags;
	int index, top_row, top_col, bot_row, bot_col, rows, cols;

	input = str_alloc_stack(0);

	substitute(ses, arg, input, SUB_VAR|SUB_FUN);

	arg = input;

	draw_cnt = 0;

	if (*arg == 0)
	{
		tintin_header(ses, 80, " DRAW OPTIONS ");

		for (index = 0 ; *draw_table[index].fun ; index++)
		{
			if (*draw_table[index].name)
			{
				tintin_printf2(ses, "  [%-24s] %s", draw_table[index].name, draw_table[index].desc);
			}
		}
		tintin_header(ses, 80, "");

		return ses;
	}

	color = str_alloc_stack(0);
	code1 = str_alloc_stack(0);
	code2 = str_alloc_stack(0);

	flags = HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) ? DRAW_FLAG_UTF8 : 0;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

		if (!HAS_BIT(flags, DRAW_FLAG_COLOR1) && translate_color_names(ses, arg1, code1))
		{
			get_color_names(ses, arg1, color);

			SET_BIT(flags, DRAW_FLAG_COLOR1);

			continue;
		}

		if (!HAS_BIT(flags, DRAW_FLAG_COLOR2) && translate_color_names(ses, arg1, code2))
		{
//			get_color_names(ses, arg1, color);

			SET_BIT(flags, DRAW_FLAG_COLOR2);

			continue;
		}

		switch (*arg1 % 32)
		{
			case CTRL_A:
				if (is_abbrev(arg1, "ASCII"))
				{
					DEL_BIT(flags, DRAW_FLAG_UTF8);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_B:
				if (is_abbrev(arg1, "BLANKED"))
				{
					SET_BIT(flags, DRAW_FLAG_BLANKED);
				}
				else if (is_abbrev(arg1, "BOTTOM"))
				{
					SET_BIT(flags, DRAW_FLAG_BOT);
				}
				else if (!strcasecmp(arg1, "BOXED"))
				{
					SET_BIT(flags, DRAW_FLAG_BOXED|DRAW_FLAG_LEFT|DRAW_FLAG_RIGHT|DRAW_FLAG_TOP|DRAW_FLAG_BOT);
				}
				else if (is_abbrev(arg1, "BUMPED"))
				{
					SET_BIT(flags, DRAW_FLAG_BUMP);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_C:
				if (is_abbrev(arg1, "CALIGN"))
				{
					SET_BIT(flags, DRAW_FLAG_CALIGN);
				}
				else if (is_abbrev(arg1, "CIRCLED"))
				{
					SET_BIT(flags, DRAW_FLAG_CIRCLED);
				}
				else if (is_abbrev(arg1, "CONVERT"))
				{
					SET_BIT(flags, DRAW_FLAG_CONVERT);
				}
				else if (is_abbrev(arg1, "CROSSED"))
				{
					SET_BIT(flags, DRAW_FLAG_CROSSED);
				}
				else if (is_abbrev(arg1, "CURSIVE"))
				{
					SET_BIT(flags, DRAW_FLAG_CURSIVE);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_F:
				if (is_abbrev(arg1, "FAT"))
				{
					SET_BIT(flags, DRAW_FLAG_FAT);
				}
				else if (is_abbrev(arg1, "FILLED"))
				{
					SET_BIT(flags, DRAW_FLAG_FILLED);
				}
				else if (is_abbrev(arg1, "FOREGROUND"))
				{
					SET_BIT(flags, DRAW_FLAG_FOREGROUND);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_G:
				if (is_abbrev(arg1, "GRID"))
				{
					SET_BIT(flags, DRAW_FLAG_GRID);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_H:
				if (is_abbrev(arg1, "HORIZONTAL"))
				{
					SET_BIT(flags, DRAW_FLAG_HOR);
				}
				else if (is_abbrev(arg1, "HUGE"))
				{
					SET_BIT(flags, DRAW_FLAG_HUGE);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_J:
				if (is_abbrev(arg1, "JEWELED"))
				{
					SET_BIT(flags, DRAW_FLAG_JEWELED);
				}
				else if (is_abbrev(arg1, "JOINTED"))
				{
					SET_BIT(flags, DRAW_FLAG_CORNERED);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_L:
				if (is_abbrev(arg1, "LALIGN"))
				{
					SET_BIT(flags, DRAW_FLAG_LALIGN);
				}
				else if (is_abbrev(arg1, "LEFT"))
				{
					SET_BIT(flags, DRAW_FLAG_LEFT);
				}
				else
				{
					goto option;
				}
				continue;

			case CTRL_N:
				if (is_abbrev(arg1, "NUMBERED"))
				{
					SET_BIT(flags, DRAW_FLAG_NUMBERED);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_P:
				if (is_abbrev(arg1, "PRUNED"))
				{
					SET_BIT(flags, DRAW_FLAG_PRUNED);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_R:
				if (is_abbrev(arg1, "RALIGN"))
				{
					SET_BIT(flags, DRAW_FLAG_RALIGN);
				}
				else if (is_abbrev(arg1, "RIGHT"))
				{
					SET_BIT(flags, DRAW_FLAG_RIGHT);
				}
				else if (is_abbrev(arg1, "ROUNDED"))
				{
					SET_BIT(flags, DRAW_FLAG_ROUNDED);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_S:
				if (is_abbrev(arg1, "SANSSERIF"))
				{
					SET_BIT(flags, DRAW_FLAG_SANSSERIF);
				}
				else if (is_abbrev(arg1, "SCALED"))
				{
					SET_BIT(flags, DRAW_FLAG_SCALED);
				}
				else if (is_abbrev(arg1, "SCROLL"))
				{
					SET_BIT(flags, DRAW_FLAG_SCROLL);
				}
				else if (is_abbrev(arg1, "SHADOWED"))
				{
					SET_BIT(flags, DRAW_FLAG_SHADOWED);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_T:
				if (is_abbrev(arg1, "TALIGN"))
				{
					SET_BIT(flags, DRAW_FLAG_TALIGN);
				}
				else if (is_abbrev(arg1, "TEED"))
				{
					SET_BIT(flags, DRAW_FLAG_TEED);
				}
				else if (is_abbrev(arg1, "TOP"))
				{
					SET_BIT(flags, DRAW_FLAG_TOP);
				}
				else if (is_abbrev(arg1, "TRACED"))
				{
					SET_BIT(flags, DRAW_FLAG_TRACED);
				}
				else if (is_abbrev(arg1, "TUBED"))
				{
					SET_BIT(flags, DRAW_FLAG_TUBED);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_U:
				if (is_abbrev(arg1, "UALIGN"))
				{
					SET_BIT(flags, DRAW_FLAG_UALIGN);
				}
				else if (is_abbrev(arg1, "UNICODE"))
				{
					SET_BIT(flags, DRAW_FLAG_UTF8);
				}
				else
				{
					goto option;
				}
				continue;
			case CTRL_V:
				if (is_abbrev(arg1, "VERTICAL"))
				{
					SET_BIT(flags, DRAW_FLAG_VER);
				}
				else
				{
					break;
				}
				continue;

			default:
				goto option;
				continue;
		}
	}

	option:

	for (index = 0 ; *draw_table[index].name ; index++)
	{
		if (is_abbrev(arg1, draw_table[index].name))
		{
			arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
			arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

			top_row = get_row_index_arg(ses, arg1);
			top_col = get_col_index_arg(ses, arg2);

			arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
			arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

			bot_row = get_row_index_arg(ses, arg1);
			bot_col = get_col_index_arg(ses, arg2);

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
			else
			{
				if (!HAS_BIT(flags, DRAW_FLAG_SCROLL) && !HAS_BIT(flags, DRAW_FLAG_FOREGROUND) && ses != gtd->ses)
				{
					show_message(ses, LIST_COMMAND, "#WARNING: #DRAW %s %d %d %d %d: SESSION IS IN THE BACKGROUND.", draw_table[index].name, top_row, top_col, bot_row, bot_col);

					return ses;
				}
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

			if (top_row > bot_row || top_col > bot_col)
			{
				show_error(ses, LIST_COMMAND, "#ERROR: #DRAW INVALID SQUARE: %s {%d %d %d %d} ROWS: %d COLS: %d", draw_table[index].name, top_row, top_col, bot_row, bot_col, 1 + bot_row - top_row, 1 + bot_col - top_col);

				return ses;
			}

			rows = URANGE(1, 1 + bot_row - top_row, gtd->screen->rows);
			cols = URANGE(1, 1 + bot_col - top_col, gtd->screen->cols);

			if (HAS_BIT(flags, DRAW_FLAG_SCALED))
			{
				scale_drawing(ses, &top_row, &top_col, &bot_row, &bot_col, &rows, &cols, index, draw_table[index].flags | flags, arg);
			}

			*arg1 = 0;
			*arg2 = 0;
			*arg3 = 0;
//			*arg4 = 0;

			// not sure of the utility of this
/*
			if (*arg == 0)
			{
				arg = arg4;
			}
*/
			save_pos(ses);

			if (HAS_BIT(flags, DRAW_FLAG_BUMP))
			{
				tintin_printf2(ses, "");
			}

			str_cpy(&arg2, code1);
			str_cpy(&arg3, code2);

			draw_table[index].fun(ses, top_row, top_col, bot_row, bot_col, rows, cols, draw_table[index].flags | flags, color, arg, arg1, arg2, arg3);

			print_stdout(0, 0, "\e[0m");

			restore_pos(ses);

			return ses;
		}
	}
	show_error(ses, LIST_COMMAND, "#ERROR: #DRAW {%s} IS NOT A VALID OPTION.", capitalize(arg1));

	return ses;
}

// utilities

void scale_drawing(struct session *ses, int *top_row, int *top_col, int *bot_row, int *bot_col, int *rows, int *cols, int index, long long flags, char *arg)
{
	char *buf, *out, *tmp;
	int inside;
	int height, bor_height, tot_height, max_height;
	int width, bor_width, tot_width, max_width;

	if (*arg == 0)
	{
		return;
	}

	push_call("scale_drawing(%p,%p,%p,%p,%p,%p,%p,%p,%d,%lld,%p",ses,top_row,top_col,bot_row,bot_col,rows,cols,index,flags,arg);

	buf = str_alloc_stack(0);
	out = str_alloc_stack(0);
	tmp = str_alloc_stack(0);

	height = bor_height = tot_height = max_height = 0;
	width = bor_width = tot_width = max_width = 0;

	if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
	{
		inside = TRUE;
	}
	else
	{
		inside = inside_scroll_region(ses, *top_row, *top_col);
	}

	if (inside)
	{
		max_height = 0 + ses->split->bot_row - ses->split->top_row;
		max_width  = 1 + ses->split->bot_col - ses->split->top_col;
	}
	else
	{
		max_height = gtd->screen->rows;
		max_width  = gtd->screen->cols;
	}

	if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
	{
		max_height = gtd->screen->rows * 10;
	}

	bor_height += HAS_BIT(flags, DRAW_FLAG_TOP) ? 1 : 0;
	bor_height += HAS_BIT(flags, DRAW_FLAG_BOT) ? 1 : 0;

	bor_width += HAS_BIT(flags, DRAW_FLAG_LEFT)  ? 1 : 0;
	bor_width += HAS_BIT(flags, DRAW_FLAG_RIGHT) ? 1 : 0;

	if (HAS_BIT(flags, DRAW_FLAG_UALIGN))
	{
		ualign(ses, arg, tmp, max_width - bor_width);

		arg = tmp;
	}

	while (*arg)
	{
		arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN|SUB_COL|SUB_LIT|SUB_ESC);

		word_wrap_split(ses, buf, out, max_width - bor_width, 0, 0, WRAP_FLAG_NONE, &height, &width);

		tot_width = UMAX(tot_width, width);
		tot_height += height;

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}

	tot_height += bor_height;
	tot_width += bor_width;

	if (tot_height > max_height)
	{
		tot_height = max_height;
	}

	if (tot_height > *rows)
	{
		*bot_row = get_row_index(ses, *top_row + tot_height - 1);

		if (inside)
		{
			if (*bot_row > ses->split->bot_row - 1)
			{
				*bot_row = ses->split->bot_row - 1;
			}
		}
		*rows = URANGE(1, 1 + *bot_row - *top_row, gtd->screen->rows);
	}

	while (tot_height > *rows)
	{
		(*top_row)--;

		*rows = 1 + *bot_row - *top_row;
	}
//	tintin_printf2(ses, "debug4: rows %d top_row: %d bot_row %d", *rows, *top_row, *bot_row);

	if (tot_width > max_width)
	{
		tot_width = max_width;
	}

	if (tot_width > *cols)
	{
		*bot_col = get_col_index(ses, *top_col + tot_width - 1);

		if (inside)
		{
			if (*bot_col > ses->split->bot_col)
			{
				*bot_col = ses->split->bot_col;
			}
		}
		*cols = URANGE(1, 1 + *bot_col - *top_col, gtd->screen->cols);
	}

	while (tot_width > *cols)
	{
		(*top_col)--;

		*cols = 1 + *bot_col - *top_col;
	}
//	tintin_printf2(ses, "debug4: cols %d top_col: %d bot_col %d", *cols, *top_col, *bot_col);

	pop_call();
	return;
}

char *alnum_normal[] =
{
	"\x00",   "\x01",   "\x02",   "\x03",   "\x04",   "\x05",   "\x06",   "\x07",   "\x08",   "\x09",   "\x10",   "\x11",   "\x12",   "\x13",   "\x14",   "\x15",   "\x16",   "\x17",   "\x18",   "\x19",   "\x20",   "\x21",   "\x22",   "\x23",   "\x24",   "\x25",   "\x26",   "\x27",   "\x28",   "\x29",   "\x30",   "\x31",
	" ",      "!",      "\"",     "#",      "$",      "%",      "&",      "'",      "(",      ")",      "*",      "+",      ",",      "-",      ".",      "/",      "0",      "1",      "2",      "3",      "4",      "5",      "6",      "7",      "8",      "9",      ":",      ";",      "<",      "=",      ">",      "?",
	"@",      "A",      "B",      "C",      "D",      "E",      "F",      "G",      "H",      "I",      "J",      "K",      "L",      "M",      "N",      "O",      "P",      "Q",      "R",      "S",      "T",      "U",      "V",      "W",      "X",      "Y",      "Z",      "[",      "\\",     "]",      "^",      "_",
	"`",      "a",      "b",      "c",      "d",      "e",      "f",      "g",      "h",      "i",      "j",      "k",      "l",      "m",      "n",      "o",      "p",      "q",      "r",      "s",      "t",      "u",      "v",      "w",      "x",      "y",      "z",      "{",      "|",      "}",      "~",      "\x7F"
};

char *alnum_normal_fat[] =
{
	"\x00",   "\x01",   "\x02",   "\x03",   "\x04",   "\x05",   "\x06",   "\x07",   "\x08",   "\x09",   "\x10",   "\x11",   "\x12",   "\x13",   "\x14",   "\x15",   "\x16",   "\x17",   "\x18",   "\x19",   "\x20",   "\x21",   "\x22",   "\x23",   "\x24",   "\x25",   "\x26",   "\x27",   "\x28",   "\x29",   "\x30",   "\x31",
	" ",      "!",      "\"",     "#",      "$",      "%",      "&",      "'",      "(",      ")",      "*",      "+",      ",",      "-",      ".",      "/",      "𝟎",      "𝟏",      "𝟐",      "𝟑",      "𝟒",      "𝟓",      "𝟔",      "𝟕",      "𝟖",      "𝟗",      ":",      ";",      "<",      "=",      ">",      "?",
	"@",      "𝐀",      "𝐁",      "𝐂",      "𝐃",      "𝐄",      "𝐅",      "𝐆",      "𝐇",      "𝐈",      "𝐉",      "𝐊",      "𝐋",      "𝐌",      "𝐍",      "𝐎",      "𝐏",      "𝐐",      "𝐑",      "𝐒",      "𝐓",      "𝐔",      "𝐕",      "𝐖",      "𝐗",      "𝐘",      "Z",      "𝐙",      "\\",     "]",      "^",      "_",
	"`",      "𝐚",      "𝐛",      "𝐜",      "𝐝",      "𝐞",      "𝐟",      "𝐠",      "𝐡",      "𝐢",      "𝐣",      "𝐤",      "𝐥",      "𝐦",      "𝐧",      "𝐨",      "𝐩",      "𝐪",      "𝐫",      "𝐬",      "𝐭",      "𝐮",      "𝐯",      "𝐰",      "𝐱",      "𝐲",      "𝐳",      "{",      "|",      "}",      "~",      "\x7F"
};

char *alnum_cursive[] =
{
	"\x00",   "\x01",   "\x02",   "\x03",   "\x04",   "\x05",   "\x06",   "\x07",   "\x08",   "\x09",   "\x10",   "\x11",   "\x12",   "\x13",   "\x14",   "\x15",   "\x16",   "\x17",   "\x18",   "\x19",   "\x20",   "\x21",   "\x22",   "\x23",   "\x24",   "\x25",   "\x26",   "\x27",   "\x28",   "\x29",   "\x30",   "\x31",
	" ",      "!",      "\"",     "#",      "$",      "%",      "&",      "'",      "(",      ")",      "*",      "+",      ",",      "-",      ".",      "/",      "0",      "1",      "2",      "3",      "4",      "5",      "6",      "7",      "8",      "9",      ":",      ";",      "<",      "=",      ">",      "?",
	"@",      "𝒜",      "ℬ",      "𝒞",      "𝒟",      "ℰ",      "ℱ",      "𝒢",      "ℋ",      "ℐ",      "𝒥",      "𝒦",      "ℒ",      "ℳ",      "𝒩",      "𝒪",      "𝒫",      "𝒬",      "ℛ",      "𝒮",      "𝒯",      "𝒰",      "𝒱",      "𝒲",      "𝒳",      "𝒴",      "𝒵",      "[",      "\\",     "]",      "^",      "_",
	"`",      "𝒶",      "𝒷",      "𝒸",      "𝒹",      "ℯ",      "𝒻",      "ℊ",      "𝒽",      "𝒾",      "𝒿",      "𝓀",      "𝓁",      "𝓂",      "𝓃",      "ℴ",      "𝓅",      "𝓆",      "𝓇",      "𝓈",      "𝓉",      "𝓊",      "𝓋",      "𝓌",      "𝓍",      "𝓎",      "𝓏",      "{",      "|",      "}",      "~",      "\x7F"
};

char *alnum_sansserif[] =
{
	"\x00",   "\x01",   "\x02",   "\x03",   "\x04",   "\x05",   "\x06",   "\x07",   "\x08",   "\x09",   "\x10",   "\x11",   "\x12",   "\x13",   "\x14",   "\x15",   "\x16",   "\x17",   "\x18",   "\x19",   "\x20",   "\x21",   "\x22",   "\x23",   "\x24",   "\x25",   "\x26",   "\x27",   "\x28",   "\x29",   "\x30",   "\x31",
	" ",      "!",      "\"",     "#",      "$",      "%",      "&",      "'",      "(",      ")",      "*",      "+",      ",",      "-",      ".",      "/",      "𝟢",      "𝟣",      "𝟤",      "𝟥",      "𝟦",      "𝟧",      "𝟨",      "𝟩",      "𝟪",      "𝟫",      ":",      ";",      "<",      "=",      ">",      "?",
	"@",      "𝖠",      "𝖡",      "𝖢",      "𝖣",      "𝖤",      "𝖥",      "𝖦",      "𝖧",      "𝖨",      "𝖩",      "𝖪",      "𝖫",      "𝖬",      "𝖭",      "𝖮",      "𝖯",      "𝖰",      "𝖱",      "𝖲",      "𝖳",      "𝖴",      "𝖵",      "𝖶",      "𝖷",      "𝖸",      "𝖹",      "[",      "\\",     "]",      "^",      "_",
	"`",      "𝖺",      "𝖻",      "𝖼",      "𝖽",      "𝖾",      "𝖿",      "𝗀",      "𝗁",      "𝗂",      "𝗃",      "𝗄",      "𝗅",      "𝗆",      "𝗇",      "𝗈",      "𝗉",      "𝗊",      "𝗋",      "𝗌",      "𝗍",      "𝗎",      "𝗏",      "𝗐",      "𝗑",      "𝗒",      "𝗓",      "{",      "|",      "}",      "~",      "\x7F"
};

void string_to_font(struct session *ses, long long flags, char *in, char *out)
{
	char buf[BUFFER_SIZE];
	char *pti, *pto;
	int skip;

	push_call("string_to_font(%p,%d,%p,%p)",ses,flags,in,out);

	// DRAW_FLAG_FAT, DRAW_FLAG_ITALIC, DRAW_FLAG_SERIF, DRAW_FLAG_CURSIVE, DRAW_FLAG_GOTHIC, DRAW_FLAG_TRACED, DRAW_FLAG_MONO

	strcpy(buf, in);

	pti = buf;
	pto = out;

	while (*pti)
	{
		skip = skip_vt102_codes(pti);

		if (skip)
		{
			pto += sprintf(pto, "%.*s", skip, pti);

			pti += skip;
			
			continue;
		}

		if (*pti >= 32)
		{
			switch (HAS_BIT(flags, DRAW_FLAG_FAT|DRAW_FLAG_CURSIVE|DRAW_FLAG_SANSSERIF))
			{
				case DRAW_FLAG_CURSIVE:
					pto += sprintf(pto, "%s", alnum_cursive[(int) *pti]);
					break;

				case DRAW_FLAG_FAT:
					pto += sprintf(pto, "%s", alnum_normal_fat[(int) *pti]);
					break;

				case DRAW_FLAG_SANSSERIF:
					pto += sprintf(pto, "%s", alnum_sansserif[(int) *pti]);
					break;

				default:
					*pto++ = *pti;
					break;
			}
			pti++;
		}
		else
		{
			*pto++ = *pti++;
		}
	}
	*pto++ = 0;

	pop_call();
	return;
}

int find_stamp(char *in, char *out)
{
	int cnt;

	for (cnt = 0 ; huge_stamp_table[cnt].name != NULL ; cnt++)
	{
		if (!strcmp(in, huge_stamp_table[cnt].name))
		{
			strcpy(out, huge_stamp_table[cnt].desc);

			return huge_stamp_table[cnt].length;
		}
	}
	tintin_printf2(gtd->ses, "debug: didn't find stamp {%s}", in);

	return 0;
}

void stamp_cat(char *color, long long flags, char *str, char *cat, char *out)
{
	char *pts = str;
	char *ptc = cat;
	char *pto = out;

	while (*pts || *ptc)
	{
		while (*pts && *pts != '\n')
		{
			*pto++ = *pts++;
		}

		pto += sprintf(pto, "%s", color);

		while (*ptc && *ptc != '\n')
		{
			if (HAS_BIT(flags, DRAW_FLAG_SHADOWED))
			{
				*pto++ = *ptc++;
			}
			else if (HAS_BIT(flags, DRAW_FLAG_TRACED))
			{
				if (!strncmp(ptc, "╗", 3))
				{
					pto += sprintf(pto, "┐");
					ptc += strlen("╗");
				}
				else if (!strncmp(ptc, "║", 3))
				{
					pto += sprintf(pto, "│");
					ptc += strlen("║");
				}
				else if (!strncmp(ptc, "╝", 3))
				{
					pto += sprintf(pto, "┘");
					ptc += strlen("╝");
				}
				else if (!strncmp(ptc, "╚", 3))
				{
					pto += sprintf(pto, "└");
					ptc += strlen("╚");
				}
				else if (!strncmp(ptc, "╔", 3))
				{
					pto += sprintf(pto, "┌");
					ptc += strlen("╔");
				}
				else if (!strncmp(ptc, "═", 3))
				{
					pto += sprintf(pto, "─");
					ptc += strlen("═");
				}
				else
				{
					*pto++ = *ptc++;
				}
			}
			else
			{
				if (!strncmp(ptc, "╗", 3))
				{
					pto += sprintf(pto, " ");
					ptc += strlen("╗");
				}
				else if (!strncmp(ptc, "║", 3))
				{
					pto += sprintf(pto, " ");
					ptc += strlen("║");
				}
				else if (!strncmp(ptc, "╝", 3))
				{
					pto += sprintf(pto, " ");
					ptc += strlen("╝");
				}
				else if (!strncmp(ptc, "╚", 3))
				{
					pto += sprintf(pto, " ");
					ptc += strlen("╚");
				}
				else if (!strncmp(ptc, "╔", 3))
				{
					pto += sprintf(pto, " ");
					ptc += strlen("╔");
				}
				else if (!strncmp(ptc, "═", 3))
				{
					pto += sprintf(pto, " ");
					ptc += strlen("═");
				}
				else
				{
					*pto++ = *ptc++;
				}
			}
		}

		if (*pts == '\n' && *ptc == '\n')
		{
			*pto++ = *pts++;
			ptc++;
		}
		else if (*ptc == '\n')
		{
			*pto++ = *ptc++;
		}
	}
	*pto = 0;
}

void string_to_stamp(struct session *ses, long long flags, char *in, char *out)
{
	char *pti, *buf1, *buf2, *buf3, chr1[CHAR_SIZE], color[COLOR_SIZE] = { 0 };
	int skip;

	push_call("string_to_stamp(%p,%d,%p,%p)",ses,flags,in,out);

	buf1 = str_alloc_stack(0);
	buf2 = str_alloc_stack(0);
	buf3 = str_alloc_stack(0);

	sub_arg_in_braces(ses, in, buf1, GET_ALL, SUB_COL|SUB_LIT|SUB_ESC);

	pti = buf1;

	buf3[0] = 0;

	while (*pti)
	{
		skip = skip_vt102_codes(pti);

		if (skip)
		{
			get_color_codes(color, pti, color, GET_ONE);

			pti += skip;
			
			continue;
		}
		pti = get_char(ses, pti, chr1);

		find_stamp(chr1, buf2);

		stamp_cat(color, flags, buf3, buf2, out);

		strcpy(buf3, out);
	}
	strcat(out, "\n");

	pop_call();
	return;
}

char *get_draw_corner(long long flags, char *str)
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
			if (HAS_BIT(flags, DRAW_FLAG_TUBED))
			{
				strcpy(draw_buf[draw_cnt], "╬");
			}
			else
			{
				strcpy(draw_buf[draw_cnt], "┼");
			}
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
						strcpy(draw_buf[draw_cnt], "╩");
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

char *draw_horizontal(long long flags, char *str)
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

char *draw_vertical(long long flags, char *str)
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

	if (!HAS_BIT(flags, DRAW_FLAG_LEFT) && !HAS_BIT(flags, DRAW_FLAG_RIGHT) && !HAS_BIT(flags, DRAW_FLAG_BOT))
	{
		return;
	}

	SET_BIT(flags, HAS_BIT(flags, DRAW_FLAG_VER) ? DRAW_FLAG_VER : DRAW_FLAG_HOR);

	corner = flags;

	DEL_BIT(corner, DRAW_FLAG_RIGHT|DRAW_FLAG_TOP);

	arg = arg1;

	if (HAS_BIT(flags, DRAW_FLAG_LEFT) || HAS_BIT(flags, DRAW_FLAG_BOT))
	{
		SET_BIT(corner, DRAW_FLAG_LEFT|DRAW_FLAG_BOT);

		arg1 += sprintf(arg1, "%s%s", color, get_draw_corner(corner, "└"));
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
			arg1 += sprintf(arg1, "%s", get_draw_corner(corner, "┘"));
		}
	}

	if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
	{

		tintin_printf2(ses, "%s%s", indent_one(top_col - 1), arg);
	}
	else
	{
		goto_pos(ses, bot_row, top_col);

		print_stdout(bot_row, top_col, "%s", arg);
	}
}

DO_DRAW(draw_arg)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	top_row = get_row_index_arg(ses, arg1);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	top_col = get_col_index_arg(ses, arg1);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	bot_row = get_row_index_arg(ses, arg1);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	bot_col = get_col_index_arg(ses, arg1);

	rows = URANGE(1, 1 + bot_row - top_row, gtd->screen->rows);
	cols = URANGE(1, 1 + bot_col - top_col, gtd->screen->cols);

	save_pos(ses);

	draw_text(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg2, arg1, arg, arg3);

	restore_pos(ses);
}

DO_DRAW(draw_box)
{
	if (HAS_BIT(flags, DRAW_FLAG_TOP))
	{
		draw_top_side(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2, arg3);
	}

	draw_vertical_lines(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2, arg3);

	if (HAS_BIT(flags, DRAW_FLAG_BOT))
	{
		draw_bot_side(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2, arg3);
	}
}

DO_DRAW(draw_buffer)
{
	char *buf;
	int cnt, line;

	push_call("draw_buffer(%p,...)",ses);

	buf = str_alloc_stack(0);

	if (ses->scroll->line == -1)
	{
		line = ses->scroll->used - 1;
	}
	else
	{
		line = ses->scroll->line;
	}

	for (cnt = rows ; cnt >= 0 && line - cnt >= 0 ; cnt--)
	{
		str_cat_printf(&buf, "{%s}", ses->scroll->buffer[line - cnt]->str);
	}

	draw_box(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, buf, arg1, arg2, arg3);

	pop_call();
	return;
}

DO_DRAW(draw_corner)
{
	if (*arg)
	{
		strcpy(arg1, arg);
	}
	else
	{
		strcpy(arg1, get_draw_corner(flags, " "));
	}

	if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
	{
		tintin_printf2(ses, "%s%s%s", indent_one(top_col - 1), color, arg1);
	}
	else
	{
		goto_pos(ses, top_row, top_col);
		
		print_stdout(top_row, top_col, "%s%s", color, arg1);
	}
}

DO_DRAW(draw_line_horizontal)
{
	int col, corner;

	if (!HAS_BIT(flags, DRAW_FLAG_VER))
	{
		SET_BIT(flags, DRAW_FLAG_HOR);
	}

	if (HAS_BIT(flags, DRAW_FLAG_APPENDIX))
	{
		if (!HAS_BIT(flags, DRAW_FLAG_LEFT|DRAW_FLAG_RIGHT))
		{
			SET_BIT(flags, DRAW_FLAG_LEFT|DRAW_FLAG_RIGHT);
		}
	}

	corner = flags;

	arg = arg1;

	if (HAS_BIT(flags, DRAW_FLAG_LEFT))
	{
		DEL_BIT(corner, DRAW_FLAG_RIGHT);

		arg1 += sprintf(arg1, "%s", get_draw_corner(corner, "┌"));
	}
	else
	{
		arg1 += sprintf(arg1, "%s", draw_horizontal(flags, "─"));
	}

	if (cols - 2 > 0)
	{
		for (col = top_col + 1 ; col < bot_col ; col++)
		{
			arg1 += sprintf(arg1, "%s", draw_horizontal(flags, "─"));
		}
	}

	if (HAS_BIT(flags, DRAW_FLAG_RIGHT) && cols - 2 > 0)
	{
		SET_BIT(corner, DRAW_FLAG_RIGHT);
		DEL_BIT(corner, DRAW_FLAG_LEFT);

		arg1 += sprintf(arg1, "%s", get_draw_corner(corner, "┐"));
	}
	else
	{
		arg1 += sprintf(arg1, "%s", draw_horizontal(flags, "─"));
	}

	if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
	{
		tintin_printf2(ses, "%*s%s%s", top_col - 1, "", color, arg);
	}
	else
	{
		goto_pos(ses, top_row, top_col);

		print_stdout(top_row, top_col, "%s%s", color, arg);
	}
}

DO_DRAW(draw_line_vertical)
{
	int row, corner;

	if (!HAS_BIT(flags, DRAW_FLAG_HOR))
	{
		SET_BIT(flags, DRAW_FLAG_VER);
	}

	if (HAS_BIT(flags, DRAW_FLAG_APPENDIX))
	{
		if (!HAS_BIT(flags, DRAW_FLAG_TOP|DRAW_FLAG_BOT))
		{
			SET_BIT(flags, DRAW_FLAG_TOP|DRAW_FLAG_BOT);
		}
	}

	corner = flags;

	arg = arg1;

	if (HAS_BIT(flags, DRAW_FLAG_TOP))
	{
		DEL_BIT(corner, DRAW_FLAG_BOT);

		arg1 += sprintf(arg1, "{%s}", get_draw_corner(corner, "┬"));
	}
	else
	{
		arg1 += sprintf(arg1, "{%s}", draw_vertical(flags, "│"));
	}

	if (rows - 2 > 0)
	{
		for (row = top_row + 1 ; row < bot_row ; row++)
		{
			arg1 += sprintf(arg1, "{%s}", draw_vertical(flags, "│"));
		}
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOT))
	{
		SET_BIT(corner, DRAW_FLAG_BOT);
		DEL_BIT(corner, DRAW_FLAG_TOP);

		arg1 += sprintf(arg1, "{%s}", get_draw_corner(corner, "┴"));
	}
	else
	{
		arg1 += sprintf(arg1, "{%s}", draw_vertical(flags, "│"));
	}

	row = top_row;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

		if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
		{
			tintin_printf2(ses, "%*s%s%s", top_col - 1, "", color, arg2);
		}
		else
		{
			goto_pos(ses, row, top_col);

			print_stdout(row, top_col, "%s%s", color, arg2);
		}
		row++;
	}
	return;
}

DO_DRAW(draw_line)
{
	if (top_row == bot_row)
	{
		draw_line_horizontal(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2, arg3);

		return;
	}

	if (top_col == bot_col)
	{
		draw_line_vertical(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2, arg3);
		
		return;
	}

	draw_box(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2, arg3);
}

DO_DRAW(draw_map)
{
	int map_rows = rows;
	int map_cols = cols;

	if (HAS_BIT(flags, DRAW_FLAG_BOXED|DRAW_FLAG_TOP|DRAW_FLAG_PRUNED))
	{
		map_rows--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED|DRAW_FLAG_BOT|DRAW_FLAG_PRUNED))
	{
		map_rows--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED|DRAW_FLAG_LEFT|DRAW_FLAG_PRUNED))
	{
		map_cols--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED|DRAW_FLAG_RIGHT|DRAW_FLAG_PRUNED))
	{
		map_cols--;
	}

	if (ses->map && ses->map->in_room)
	{
		sprintf(arg, "{%d} {%d} {SAVE}", map_rows, map_cols);

		map_map(ses, arg, arg1, arg2);
	}
	else
	{
		str_cpy(&gtd->buf, "{}");
	}

	if (HAS_BIT(flags, DRAW_FLAG_TOP|DRAW_FLAG_BOT))
	{
		draw_box(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, gtd->buf, arg1, arg2, arg3);
	}
	else
	{
		draw_text(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, gtd->buf, arg1, arg2, arg3);
	}
}

/*
DO_DRAW(draw_map)
{
	if (ses->map && ses->map->in_room)
	{
		int map_top_row = ses->map->top_row;
		int map_top_col = ses->map->top_col;
		int map_bot_row = ses->map->bot_row;
		int map_bot_col = ses->map->bot_col;
		int map_rows    = ses->map->rows;
		int map_cols    = ses->map->cols;
		int map_flags   = ses->map->flags;

		ses->map->top_row = top_row;
		ses->map->top_col = top_col;
		ses->map->bot_row = bot_row;
		ses->map->bot_col = bot_col;
		ses->map->rows    = URANGE(1, bot_row - top_row, gtd->screen->rows);
		ses->map->cols    = URANGE(1, bot_col - top_col, gtd->screen->cols);

		SET_BIT(ses->map->flags, MAP_FLAG_VTMAP);
		DEL_BIT(ses->map->flags, MAP_FLAG_RESIZE);

		show_vtmap(ses);

		ses->map->top_row = map_top_row;
		ses->map->top_col = map_top_col;
		ses->map->bot_row = map_bot_row;
		ses->map->bot_col = map_bot_col;
		ses->map->rows    = map_rows;
		ses->map->cols    = map_cols;

		ses->map->flags   = map_flags;

		if (HAS_BIT(flags, DRAW_FLAG_BOXED|DRAW_FLAG_BOT|DRAW_FLAG_TOP|DRAW_FLAG_LEFT|DRAW_FLAG_RIGHT|DRAW_FLAG_PRUNED))
		{
			draw_box(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, "", arg1, arg2, arg3);
		}
	}
	else
	{
		draw_text(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, "{}", arg1, arg2, arg3);
	}
}
*/

char *rain_symbols = "ﾛｦｱｳｴｵｶｷｹｺｻｼｽｾｿﾀﾂﾃﾅﾆﾇﾈﾊﾋﾎﾏﾐﾑﾒﾓﾔﾕﾗﾘﾜ01SԐ45789Z=*+-¦|_ʺ╌";
char *braille_symbols = "⠁⠂⠃⠄⠅⠆⠇⠈⠊⠌⠎⠐⠑⠔⠕⠘⠜⠠⠡⠢⠣⠨⠪⠰⠱⠸⡀⡁⡂⡃⡄⡅⡆⡇⡈⡊⡌⡎⡐⡑⡔⡕⡘⡜⡠⡡⡢⡣⡨⡪⡰⡱⡸⢀⢁⢂⢃⢄⢅⢆⢇⢈⢉⢊⢌⢎⢐⢑⢔⢕⢘⢜⢠⢡⢢⢣⢨⢪⢰⢱";

DO_DRAW(draw_rain)
{
	char code[BUFFER_SIZE], arg4[BUFFER_SIZE], *rain[400];
	struct listnode *node;
	int row, col, len, rand, cnt, size, max, utfs[400];
	long double density, fade;

	strcpy(code, arg2);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);


	if (!valid_variable(ses, arg1))
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #DRAW {<COLOR>} RAIN %d %d %d %d {<VARIABLE>} {[SPAWN]} {[FADE]} {[LEGEND]}.", top_row, top_col, bot_row, bot_col);

		return;
	}

	node = search_nest_node_ses(ses, arg1);

	if (node == NULL || node->root == NULL)
	{
		node = set_nest_node(ses->list[LIST_VARIABLE], arg1, "{0}{}");
	}

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (*arg1 == 0)
	{
		density = 1;
	}
	else
	{
		density = URANGE(0.01, get_number(ses, arg1), 100);
	}

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (*arg1 == 0)
	{
		fade = 10;
	}
	else
	{
		fade = URANGE(1, get_number(ses, arg1), 100);
	}

	arg = get_arg_in_braces(ses, arg, arg4, GET_ONE);

	if (*arg4 == 0)
	{
		if (HAS_BIT(ses->config_flags, CONFIG_FLAG_SCREENREADER))
		{
			for (max = len = 0 ; braille_symbols[len] ; max++)
			{
				rain[max] = &braille_symbols[len];
				utfs[max] = get_utf8_size(&braille_symbols[len]);
				len += utfs[max];
			}
		}
		else
		{
			for (max = len = 0 ; rain_symbols[len] ; max++)
			{
				rain[max] = &rain_symbols[len];
				utfs[max] = get_utf8_size(&rain_symbols[len]);
				len += utfs[max];
			}
		}
	}
	else
	{
		for (max = len = 0 ; arg4[len] && max < 400 ; max++)
		{
			rain[max] = &arg4[len];
			utfs[max] = get_utf8_size(&arg4[len]);
			len += utfs[max];
		}
	}

//	tintin_printf2(ses, "debug: [%s] (%s) <%s>", code, fuzzy_color_code(ses, code), dim_color_code(ses, code, 20));

	for (col = 0 ; col < 1 + bot_col - top_col ; col++)
	{
		if (node->root->used <= col)
		{
			set_nest_node(node->root, ntos(col), "");
		}

		if (node->root->list[col]->val16[0] == 0)
		{
			if (generate_rand(ses) % 10000 / (long double) 100 < density)
			{
				rand = generate_rand(ses) % rows;

				node->root->list[col]->val16[0] = 1;
				node->root->list[col]->val16[1] = rand;
				node->root->list[col]->val16[2] = rand < rows / 2 ? 2 : 0;
				node->root->list[col]->val16[3] = 0;

				str_cpy_printf(&node->root->list[col]->arg2, "%*s", rand, "");
			}
			continue;
		}
		else if (node->root->list[col]->val16[0] == 1)
		{
			if (node->root->list[col]->val16[2])
			{
				node->root->list[col]->val16[3]++;

				if (node->root->list[col]->val16[3] % 2)
				{
					rand = generate_rand(ses) % max;

					sprintf(arg2, "%s%.*s", lit_color_code(ses, code, 10), utfs[rand], rain[rand]);

					substitute(ses, arg2, arg3, SUB_COL);

					goto_pos(ses, top_row + node->root->list[col]->val16[1], top_col + col);

					print_stdout(0, 0, "%s", arg3);

					continue;
				}
			}

			rand = generate_rand(ses) % max;

			str_cat_printf(&node->root->list[col]->arg2, "%.*s", utfs[rand], rain[rand]);

			node->root->list[col]->val16[1]++;

			if (generate_rand(ses) % 10000 / (long double) 100 < fade)
			{
				node->root->list[col]->val16[0] = 2;
			}
			else if (node->root->list[col]->val16[1] > rows - generate_rand(ses) % 8)
			{
				node->root->list[col]->val16[0] = 2;
			}
		}
		else
		{
			node->root->list[col]->val16[0]++;
		}

		len = str_len(node->root->list[col]->arg2);

		row = 0;
		cnt = 0;

		if (node->root->list[col]->val16[0] == 1)
		{
			while (row < len)
			{
				if (node->root->list[col]->arg2[row] == ' ')
				{
					cnt++;
					row++;

					continue;
				}
				goto_pos(ses, top_row + cnt, top_col + col);

				cnt++;

				size = get_utf8_size(&node->root->list[col]->arg2[row]);

				if (cnt == node->root->list[col]->val16[1])
				{
					sprintf(arg2, "%s%.*s", lit_color_code(ses, code, 5), size, &node->root->list[col]->arg2[row]);
				}
				else
				{
					if (node->root->list[col]->val16[1] % 2 == 0)
					{
						row += size;

						continue;
					}

					sprintf(arg2, "%s%.*s", fuzzy_color_code(ses, code), size, &node->root->list[col]->arg2[row]);
				}

				substitute(ses, arg2, arg3, SUB_COL);

				print_stdout(0, 0, "%s", arg3);

				row += size;
			}
		}
		else if (node->root->list[col]->val16[0] > 1)
		{
			while (row < len)
			{
				if (node->root->list[col]->arg2[row] == ' ')
				{
					cnt++;
					row++;
					continue;
				}

				goto_pos(ses, top_row + cnt, top_col + col);

				cnt++;

				size = get_utf8_size(&node->root->list[col]->arg2[row]);

				if (node->root->list[col]->val16[0] - cnt > 15)
				{
					sprintf(arg2, "%s ", dim_color_code(ses, code, node->root->list[col]->val16[0] - cnt));
					substitute(ses, arg2, arg3, SUB_COL);
					print_stdout(0, 0, "%s", arg3);

					print_stdout(0, 0, " ");
				}
				else if (node->root->list[col]->val16[0] - cnt < 0)
				{
					sprintf(arg2, "%s%.*s", fuzzy_color_code(ses, code), size, &node->root->list[col]->arg2[row]);
					substitute(ses, arg2, arg3, SUB_COL);
					print_stdout(0, 0, "%s", arg3);
				}
				else
				{
					sprintf(arg2, "%s%.*s", dim_color_code(ses, code, node->root->list[col]->val16[0] - cnt), size, &node->root->list[col]->arg2[row]);
					substitute(ses, arg2, arg3, SUB_COL);
					print_stdout(0, 0, "%s", arg3);
				}

				row += size;
			}

			if (node->root->list[col]->val16[0] - cnt > 16)
			{
				node->root->list[col]->val16[0] = 0;
				node->root->list[col]->val16[1] = 0;

				str_cpy(&node->root->list[col]->arg2, "");
			}
		}
		else
		{
			tintin_printf2(ses, "debug: problemo");
		}
	}
}

DO_DRAW(draw_side)
{
	push_call("draw_side()");

	draw_box(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2, arg3);

	pop_call();
	return;
}

DO_DRAW(draw_square)
{
	if (HAS_BIT(flags, DRAW_FLAG_TOP))
	{
		draw_top_side(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2, arg3);
	}

	draw_text(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2, arg3);

	if (HAS_BIT(flags, DRAW_FLAG_BOT))
	{
		draw_bot_side(ses, top_row, top_col, bot_row, bot_col, rows, cols, flags, color, arg, arg1, arg2, arg3);
	}
}


DO_DRAW(draw_table_grid)
{
	char buf1[BUFFER_SIZE], *str, buf2[BUFFER_SIZE], buf3[BUFFER_SIZE], row_color[COLOR_SIZE];
	int corner, blank, row, col, max_r, max_c, r, c, top_r, top_c, bot_r, bot_c, tot_r, tot_c;

	row = cnt_arg_all(ses, arg, GET_ALL);

	row = URANGE(0, row, (rows - 1) / 2);

	get_arg_in_braces(ses, arg, buf1, GET_ALL);

	col = cnt_arg_all(ses, buf1, GET_ALL);

	if (row == 0 || col == 0)
	{
		tintin_printf2(ses, "#ERROR: #DRAW TABLE: NEED AT LEAST 1 ROW AND 1 COLUMN.");

		return;
	}

	if (*arg3)
	{
		substitute(ses, arg3, row_color, SUB_COL|SUB_LIT|SUB_ESC);
	}
	else
	{
		row_color[0] = 0;
	}

	corner = flags;

	DEL_BIT(corner, DRAW_FLAG_LEFT | DRAW_FLAG_RIGHT | DRAW_FLAG_TOP | DRAW_FLAG_BOT);

	if (HAS_BIT(flags, DRAW_FLAG_GRID))
	{
		max_r = (rows - 1) / row;
		max_c = (cols - 1) / col;

		tot_r = 1 + max_r * row;
		tot_c = 1 + max_c * col;

		if (max_r <= 0)
		{
			tintin_printf2(ses, "#ERROR: #DRAW TABLE: ROW SIZE TOO SMALL.");
			return;
		}

		if (max_c <= 0)
		{
			tintin_printf2(ses, "#ERROR: #DRAW TABLE: COLUMN SIZE TOO SMALL.");
			return;
		}


		for (r = 0 ; r < tot_r ; r++)
		{
			buf3[0] = 0;

			if (r % max_r == 0)
			{
				arg = get_arg_in_braces(ses, arg, buf1, GET_ALL);
				str = buf1;

				if (*arg == COMMAND_SEPARATOR)
				{
					arg++;
				}
			}
			else
			{
				str = buf1;
			}

			for (c = 0 ; c < tot_c ; c++)
			{
				if (r == 0)
				{
					if (c == 0)
					{
						strcat(buf3, get_draw_corner(corner | DRAW_FLAG_LEFT | DRAW_FLAG_TOP, "┌"));
					}
					else if (c == tot_c - 1)
					{
						strcat(buf3, get_draw_corner(corner | DRAW_FLAG_RIGHT | DRAW_FLAG_TOP, "┐"));
					}
					else if (c % max_c == 0)
					{
						strcat(buf3, get_draw_corner(corner | DRAW_FLAG_TEED | DRAW_FLAG_TOP, "┬"));
					}
					else
					{
						strcat(buf3, draw_horizontal(flags, "?"));
					}
				}
				else if (r == tot_r - 1)
				{
					if (c == 0)
					{
						strcat(buf3, get_draw_corner(corner | DRAW_FLAG_LEFT | DRAW_FLAG_BOT, "?"));
					}
					else if (c == tot_c - 1)
					{
						strcat(buf3, get_draw_corner(corner | DRAW_FLAG_RIGHT | DRAW_FLAG_BOT, "?"));
					}
					else if (c % max_c == 0)
					{
						strcat(buf3, get_draw_corner(corner | DRAW_FLAG_TEED | DRAW_FLAG_BOT, "┬"));
					}
					else
					{
						strcat(buf3, draw_horizontal(flags, "?"));
					}
				}
				else if (r % max_r == 0)
				{
					if (c == 0)
					{
						strcat(buf3, get_draw_corner(corner | DRAW_FLAG_TEED | DRAW_FLAG_HOR | DRAW_FLAG_LEFT, "?"));
					}
					else if (c == tot_c - 1)
					{
						strcat(buf3, get_draw_corner(corner | DRAW_FLAG_TEED | DRAW_FLAG_HOR | DRAW_FLAG_RIGHT, "?"));
					}
					else if (c % max_c == 0)
					{
						strcat(buf3, get_draw_corner(corner | DRAW_FLAG_CROSSED, "?"));
					}
					else
					{
						strcat(buf3, draw_horizontal(flags, "?"));
					}
				}
				else if (r % max_r == 1)
				{
					if (c == tot_c - 1)
					{
						if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
						{
							cat_sprintf(buf3, "%s", draw_vertical(flags, "│"));
						}
						else
						{
							goto_pos(ses, top_row + r, top_col + c);

							print_stdout(top_row + r, top_col + c, "%s%s", color, draw_vertical(flags, "│"));
						}
					}
					else if (c == 0 || c % max_c == 0)
					{
						strcpy(buf2, row_color);

						str = sub_arg_in_braces(ses, str, buf2 + strlen(buf2), GET_ALL, SUB_VAR|SUB_FUN|SUB_LIT|SUB_ESC|SUB_COL);

						get_color_codes(row_color, buf2, row_color, GET_ALL);

						top_r = top_row + r - 1;
						top_c = top_col + c;
						bot_r = top_row + r - 1 + max_r;
						bot_c = top_col + c + max_c;

						draw_vertical_lines(ses, top_r, top_c, top_r, bot_c, 1 + max_r, 1 + max_c, corner | DRAW_FLAG_LEFT, color, buf2, arg1, arg2, arg3);

						if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
						{
							strcat(buf3, arg2);
						}
	
						if (*str == COMMAND_SEPARATOR)
						{
							str++;
						}
					}
				}
				else
				{
					if (c == tot_c - 1)
					{
						if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
						{
							cat_sprintf(buf3, "%s", draw_vertical(flags, "│"));
						}
						else
						{
							goto_pos(ses, top_row + r, top_col + c);

							print_stdout(top_row + r, top_col + c, "%s%s", color, draw_vertical(flags, "│"));
						}
					}
				}
			}

			if (*buf3)
			{
				if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
				{
					tintin_printf2(ses, "%s%s%s", indent_one(top_col - 1), color, buf3);
				}
				else
				{
					goto_pos(ses, top_row + r, top_col);

					print_stdout(top_row + r, top_col, "%s%s", color, buf3);
				}
			}

		}
		return;
	}

	blank = flags;

	DEL_BIT(blank, DRAW_FLAG_BOXED|DRAW_FLAG_LEFT|DRAW_FLAG_RIGHT|DRAW_FLAG_TOP|DRAW_FLAG_BOT);

	max_r = rows / row;
	max_c = cols / col;

	for (r = 0 ; r < row ; r++)
	{
		top_r = top_row + r * max_r;
		bot_r = top_row + r * max_r + max_r - 1;

		arg = get_arg_in_braces(ses, arg, buf1, GET_ALL);

		str = buf1;

		for (c = 0 ; c < col ; c++)
		{
			str = get_arg_in_braces(ses, str, buf2, GET_ALL);

			top_c = top_col + c * max_c;
			bot_c = top_col + c * max_c + max_c - 1;

			draw_box(ses, top_r, top_c, bot_r, bot_c, 1 + bot_r - top_r, 1 + bot_c - top_c, flags, color, buf2, arg1, arg2, arg3);

//			tintin_printf2(ses, "#draw box %d %d %d %d %s", top_row + r * max_r, top_col + c * max_c, top_row + r * max_r + max_r, top_col + c * max_c + max_c, buf1);

			if (*str == COMMAND_SEPARATOR)
			{
				str++;
			}
		}

		if (cols > max_c * col)
		{
			top_c = top_col + c * max_c;
			bot_c = bot_col;

			draw_text(ses, top_r, top_c, bot_r, bot_c, 1 + bot_r - top_r, 1 + bot_c - top_c, blank, color, "", arg1, arg2, arg3);
		}

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}
}

DO_DRAW(draw_text)
{
	char *txt, *buf1, *buf2, *buf3, side1[100], side2[100];
	int row, col, height, width;

	push_call("draw_text(%p,%d,%p,%p,%p)",ses,flags,arg,arg1,arg2);

	buf1 = str_alloc_stack(0);
	buf2 = str_alloc_stack(0);
	buf3 = str_alloc_stack(0);

	side1[0] = side2[0] = arg2[0] = 0;

	txt = buf2;

	substitute(ses, arg, buf3, SUB_VAR|SUB_FUN);

	arg = buf3;

	if (*arg3)
	{
		txt += substitute(ses, arg3, txt, SUB_COL|SUB_LIT|SUB_ESC);
	}

	if (HAS_BIT(flags, DRAW_FLAG_HUGE))
	{
		string_to_stamp(ses, flags, arg, txt);
	}
	else
	{
		while (*arg)
		{
			arg = sub_arg_in_braces(ses, arg, buf1, GET_ALL, SUB_COL|SUB_LIT|SUB_ESC|SUB_VAR|SUB_FUN);

			txt += sprintf(txt, "%s\n", buf1);

			if (*arg == COMMAND_SEPARATOR)
			{
				arg++;
			}
		}
		string_to_font(ses, flags, buf2, buf2);
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED|DRAW_FLAG_TOP|DRAW_FLAG_PRUNED))
	{
		top_row++;
		rows--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED|DRAW_FLAG_BOT|DRAW_FLAG_PRUNED))
	{
		bot_row--;
		rows--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED|DRAW_FLAG_LEFT|DRAW_FLAG_PRUNED))
	{
		strcpy(side1, " ");
		cols--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_BOXED|DRAW_FLAG_RIGHT|DRAW_FLAG_PRUNED))
	{
		if (!HAS_BIT(flags, DRAW_FLAG_GRID) || HAS_BIT(flags, DRAW_FLAG_RIGHT))
		{
			strcpy(side2, " ");
		}
		cols--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_UALIGN))
	{
		ualign(ses, buf2, buf3, cols);

		word_wrap_split(ses, buf3, buf1, cols, 0, 0, FLAG_NONE, &height, &width);
	}
	else
	{
		word_wrap_split(ses, buf2, buf1, cols, 0, 0, FLAG_NONE, &height, &width);
	}

	height--;

	txt = buf1;


	if (HAS_BIT(flags, DRAW_FLAG_TALIGN))
	{
		height = 0;

		while (*txt && height < rows)
		{
			txt = strchr(txt, '\n');
			txt++;
			height++;
		}
		*txt = 0;
		txt = buf1;
	}
	else
	{
		while (*txt && height > rows)
		{
			txt = strchr(txt, '\n');
			txt++;
			height--;
		}
	}

	if (HAS_BIT(flags, DRAW_FLAG_LEFT))
	{
		strcpy(side1, draw_vertical(flags, "│"));
	}

	if (HAS_BIT(flags, DRAW_FLAG_RIGHT))
	{
		strcpy(side2, draw_vertical(flags, "│"));
	}

	row = top_row;
	col = top_col;

	arg = txt;

	while (*arg)
	{
		arg = strchr(arg, '\n');

		if (arg)
		{
			*arg++ = 0;
		}
		else
		{
			break;
		}

		if (HAS_BIT(flags, DRAW_FLAG_CONVERT))
		{
			convert_meta(txt, buf3, FALSE);

			justify_string(ses, buf3, buf2, 0 - cols, cols);
		}
		else if (HAS_BIT(flags, DRAW_FLAG_CALIGN|DRAW_FLAG_LALIGN|DRAW_FLAG_RALIGN))
		{
			if (HAS_BIT(flags, DRAW_FLAG_CALIGN))
			{
				calign(ses, txt, buf3, cols);
			}
			else if (HAS_BIT(flags, DRAW_FLAG_LALIGN))
			{
				lalign(ses, txt, buf3, cols);
			}
			else
			{
				ralign(ses, txt, buf3, cols);
			}
			justify_string(ses, buf3, buf2, 0 - cols, cols);
		}
		else
		{
			justify_string(ses, txt, buf2, 0 - cols, cols);
		}

		if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
		{
			if (HAS_BIT(flags, DRAW_FLAG_GRID))
			{
				cat_sprintf(arg2, "%s%s%s%s%s", color, side1, buf2, color, side2);
			}
			else
			{
				tintin_printf2(ses, "%s%s%s%s%s%s", indent_one(top_col - 1), color, side1, buf2, color, side2);
			}
		}
		else
		{
			goto_pos(ses, row, col);

			print_stdout(row, col, "%s%s%s%s%s", color, side1, buf2, color, side2);
		}
		row++;

		txt = arg;
	}

	while (height < rows)
	{
		if (HAS_BIT(flags, DRAW_FLAG_LEFT))
		{
			strcpy(side1, draw_vertical(flags, "│"));
		}

		if (HAS_BIT(flags, DRAW_FLAG_RIGHT))
		{
			strcpy(side2, draw_vertical(flags, "│"));
		}

		if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
		{
			if (HAS_BIT(flags, DRAW_FLAG_GRID))
			{
				cat_sprintf(arg2, "%s%s%*s%s%s", color, side1, cols, "", color, side2);
			}
			else
			{
				tintin_printf2(ses, "%s%s%s%-*s%s%s", indent_one(top_col - 1), color, side1, cols, "", color, side2);
			}
		}
		else
		{
			goto_pos(ses, row, col);

			print_stdout(row, col, "%s%s%*s%s%s", color, side1, cols, "", color, side2);

			row++;
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
		SET_BIT(corner, DRAW_FLAG_LEFT|DRAW_FLAG_RIGHT);

		arg1 += sprintf(arg1, "%s%s", color, get_draw_corner(corner, "┌"));
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
			SET_BIT(corner, DRAW_FLAG_RIGHT|DRAW_FLAG_TOP);

			arg1 += sprintf(arg1, "%s", get_draw_corner(corner, "┐"));
		}
	}

	if (HAS_BIT(flags, DRAW_FLAG_SCROLL))
	{
		tintin_printf2(ses, "%*s%s", top_col - 1, "", arg);
	}
	else
	{
		goto_pos(ses, top_row, top_col);

		print_stdout(top_row, top_col, "%s", arg);
	}
}

DO_DRAW(draw_vertical_lines)
{
	int row;

	push_call("draw_vertical_lines(%p,%d,%p,%p,%p)",ses,flags,arg,arg1,arg2);

	if (HAS_BIT(flags, DRAW_FLAG_SCROLL) || *arg)
	{
		draw_text(ses, top_row, top_col, bot_row, top_col, rows, cols, flags, color, arg, arg1, arg2, arg3);

		pop_call();
		return;
	}

	arg1[0] = arg2[0] = 0;

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

	if (HAS_BIT(flags, DRAW_FLAG_LEFT))
	{
		strcpy(arg1, draw_vertical(flags, "│"));

		cols--;
	}

	if (HAS_BIT(flags, DRAW_FLAG_RIGHT))
	{
		strcpy(arg2, draw_vertical(flags, "│"));

		cols--;
	}

//	tintin_printf2(ses, "debug: rows = %d", rows);

	if (*arg1)
	{
		row = top_row;

		while (row <= bot_row)
		{
			goto_pos(ses, row, top_col);

			print_stdout(row, top_col, "%s%s", color, arg1);

			row++;
		}
	}

	if (*arg2)
	{
		row = top_row;

		while (row <= bot_row)
		{
			goto_pos(ses, row, bot_col);

			print_stdout(row, bot_col, "%s%s", color, arg2);

			row++;
		}
	}

	pop_call();
	return;
}
