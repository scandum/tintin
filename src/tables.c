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
#include "telnet.h"

struct list_type list_table[LIST_MAX] =
{
	{    "ACTION",            "ACTIONS",            SORT_PRIORITY,    3, 2, 3, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_REGEX|LIST_FLAG_PRIORITY },
	{    "ALIAS",             "ALIASES",            SORT_PRIORITY,    3, 2, 3, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_REGEX|LIST_FLAG_PRIORITY },
	{    "BUTTON",            "BUTTONS",            SORT_PRIORITY,    3, 2, 3, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_PRIORITY },
	{    "CLASS",             "CLASSES",            SORT_PRIORITY,    2, 0, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_INHERIT                                 },
	{    "COMMAND",           "COMMANDS",           SORT_APPEND,      1, 0, 0, LIST_FLAG_MESSAGE                                                                  },
	{    "CONFIG",            "CONFIGS",            SORT_ALPHA,       2, 0, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "DELAY",             "DELAYS",             SORT_DELAY,       2, 2, 3, LIST_FLAG_MESSAGE|LIST_FLAG_READ                                                   },
	{    "EVENT",             "EVENTS",             SORT_ALPHA,       2, 2, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "FUNCTION",          "FUNCTIONS",          SORT_ALPHA,       2, 2, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "GAG",               "GAGS",               SORT_ALPHA,       1, 0, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "HIGHLIGHT",         "HIGHLIGHTS",         SORT_PRIORITY,    3, 0, 3, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_REGEX|LIST_FLAG_PRIORITY },
	{    "HISTORY",           "HISTORIES",          SORT_APPEND,      1, 0, 0, LIST_FLAG_MESSAGE|LIST_FLAG_HIDE },
	{    "LANDMARK",          "LANDMARKS",          SORT_ALPHA,       4, 0, 0, LIST_FLAG_MESSAGE|LIST_FLAG_HIDE },
	{    "MACRO",             "MACROS",             SORT_ALPHA,       2, 2, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "PATH",              "PATHS",              SORT_APPEND,      2, 0, 0, LIST_FLAG_MESSAGE },
	{    "PATHDIR",           "PATHDIRS",           SORT_ALPHA,       3, 0, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "PROMPT",            "PROMPTS",            SORT_ALPHA,       4, 0, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_REGEX },
	{    "SUBSTITUTE",        "SUBSTITUTES",        SORT_PRIORITY,    3, 0, 3, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_REGEX|LIST_FLAG_PRIORITY },
	{    "TAB",               "TABS",               SORT_ALPHA,       1, 0, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "TERRAIN",           "TERRAINS",           SORT_ALPHA,       2, 0, 0, LIST_FLAG_MESSAGE|LIST_FLAG_HIDE },
	{    "TICKER",            "TICKERS",            SORT_ALPHA,       3, 2, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "VARIABLE",          "VARIABLES",          SORT_ALNUM,       2, 0, 0, LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_NEST }
};

struct substitution_type substitution_table[] =
{
	{    "ARGUMENTS",            SUB_ARG },
	{    "VARIABLES",            SUB_VAR },
	{    "FUNCTIONS",            SUB_FUN },
	{    "COLORS",               SUB_COL },
	{    "ESCAPES",              SUB_ESC },
//	{    "COMMANDS",             SUB_CMD },
	{    "SECURE",               SUB_SEC },
	{    "EOL",                  SUB_EOL },
	{    "LNF",                  SUB_LNF },
        {    "LITERAL",              SUB_LIT },
	{    "",                     0       }
};

struct config_type config_table[] =
{
	{
		"AUTO TAB",
		"Buffer lines used for tab completion",
		"",
		config_autotab
	},

	{
		"BUFFER SIZE",
		"The size of the scrollback buffer",
		"",
		config_buffersize
	},

	{
		"CHARSET",
		"The character set encoding used",
		"",
		config_charset
	},

	{
		"CHILD LOCK",
		"TinTin++ is child locked.",
		"TinTin++ is not child locked.",
		config_childlock
	},

	{
		"COLOR MODE",
		"The color code encoding used",
		"",
		config_colormode
	},

	{
		"COLOR PATCH",
		"Color the start of each line",
		"Leave color handling up to the server",
		config_colorpatch
	},

	{
		"COMMAND COLOR",
		"The color of echoed commands",
		"",
		config_commandcolor
	},

	{
		"COMMAND ECHO",
		"Commands are echoed in split mode",
		"Commands are not echoed in split mode",
		config_commandecho
	},

	{
		"CONNECT RETRY",
		"Seconds sessions try to connect on failure",
		"",
		config_connectretry
	},

	{
		"CONVERT META",
		"TinTin++ converts meta characters",
		"TinTin++ doesn't convert meta characters",
		config_convertmeta
	},

	{
		"DEBUG TELNET",
		"You see telnet negotiations",
		"You do not see telnet negotatiations",
		config_debugtelnet
	},

	{
		"HISTORY SIZE",
		"The size of the command history",
		"",
		config_historysize
	},

	{
		"INHERITANCE",
		"The startup session is inherited",
		"The startup session is not inherited",
		config_inheritance
	},

	{
		"LOG MODE",
		"The data type mode of log files",
		"",
		config_logmode
	},

	{
		"LOG LEVEL",
		"TinTin++ only logs low level server data",
		"TinTin++ only logs high level server data",
		config_loglevel
	},

	{
		"MCCP",
		"MCCP is enabled.",
		"MCCP is disabled.",
		config_mccp
	},

	{
		"MOUSE",
		"Generate mouse tracking events.",
		"Do not generate mouse events.",
		config_mousetracking
	},

	{
		"PACKET PATCH",
		"Seconds to try to patch broken packets",
		"",
		config_packetpatch
	},

	{
		"RANDOM SEED",
		"Seed value used for random numbers",
		"",
		config_randomseed
	},

	{
		"REPEAT CHAR",
		"Character used for repeating commands",
		"",
		config_repeatchar
	},

	{
		"REPEAT ENTER",
		"You send the last command on an enter",
		"You send a carriage return on an enter",
		config_repeatenter
	},

	{
		"SCREEN READER",
		"You are using a screen reader",
		"You are not using a screen reader",
		config_screenreader
	},

	{
		"SCROLL LOCK",
		"You do not see server output while scrolling",
		"You see server output while scrolling",
		config_scrolllock
	},

	{
		"SPEEDWALK",
		"Your input is scanned for speedwalks",
		"Your input is not scanned for speedwalks",
		config_speedwalk
	},

	{
		"TAB WIDTH",
		"Number of spaces used for a tab",
		"",
		config_tabwidth
	},

	{
		"TELNET",
		"TELNET support is enabled.",
		"TELNET support is disabled.",
		config_telnet
	},

	{
		"TINTIN CHAR",
		"Character used for TinTin++ commands",
		"",
		config_tintinchar
	},

	{
		"VERBATIM",
		"Keyboard input is send as is",
		"Keyboard input is parsed by TinTin++",
		config_verbatim
	},

	{
		"VERBATIM CHAR",
		"Character used for verbatim lines",
		"",
		config_verbatimchar
	},

	{
		"VERBOSE",
		"Read script files verbosely",
		"Read script files quietly",
		config_verbose
	},

	{
		"WORDWRAP",
		"Server output is word wrapped",
		"Server output is line wrapped",
		config_wordwrap
	},


	{
		"",
		"",
		0,
		0
	}
};

char character_table[256] =
{
	0, // 0
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	CHAR_FLAG_SPACE, // \t
	CHAR_FLAG_SPACE, // \n
	CHAR_FLAG_SPACE, // \v
	CHAR_FLAG_SPACE, // \f
	CHAR_FLAG_SPACE, // \r
	0,
	0,
	0,

	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	CHAR_FLAG_PRINT|CHAR_FLAG_SPACE, // 32 space
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT, // 40 (
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_HEX|CHAR_FLAG_DIGIT, // 48 0
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_HEX|CHAR_FLAG_DIGIT,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_HEX|CHAR_FLAG_DIGIT,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_HEX|CHAR_FLAG_DIGIT,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_HEX|CHAR_FLAG_DIGIT,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_HEX|CHAR_FLAG_DIGIT,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_HEX|CHAR_FLAG_DIGIT,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_HEX|CHAR_FLAG_DIGIT,

	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_HEX|CHAR_FLAG_DIGIT, // 56 8
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_HEX|CHAR_FLAG_DIGIT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT|CHAR_FLAG_CSI, // 64 @
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX, // 65 A
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,

	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA, // 72 H
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,

	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA, // 80 P
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,

	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA, // 88 X
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA, // 90 Z
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT|CHAR_FLAG_CSI, // 93 ]
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR, // 95 _

	CHAR_FLAG_PRINT|CHAR_FLAG_CSI, // 96 `
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX, // 97 a
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA|CHAR_FLAG_HEX,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,

	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA, // 104 h
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA, 
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,

	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA, // 112 p
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,

	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA, // 120 x
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA,
	CHAR_FLAG_PRINT|CHAR_FLAG_VAR|CHAR_FLAG_CSI|CHAR_FLAG_ALPHA, // 122 z
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	0, // 127 del

	CHAR_FLAG_PRINT, // 128
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,

	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT,
	CHAR_FLAG_PRINT
};

	
struct color_type color_table[] =
{
	{    "azure",         "<abd>",  5 },
	{    "ebony",         "<aaa>",  5 },
	{    "jade",          "<adb>",  4 },
	{    "lime",          "<bda>",  4 },
	{    "orange",        "<dba>",  6 },
	{    "pink",          "<dab>",  4 },
	{    "silver",        "<ccc>",  6 },
	{    "tan",           "<cba>",  3 },
	{    "violet",        "<bad>",  6 },

	{    "light azure",   "<acf>", 11 },
	{    "light ebony",   "<bbb>", 11 },
	{    "light jade",    "<afc>", 10 },
	{    "light lime",    "<cfa>", 10 },
	{    "light orange",  "<fca>", 12 },
	{    "light pink",    "<fac>", 10 },
	{    "light silver",  "<eee>", 12 },
	{    "light tan",     "<eda>",  9 },
	{    "light violet",  "<caf>", 12 },

	{    "light black",   "<108>", 11 },
	{    "light red",     "<118>",  9 },
	{    "light green",   "<128>", 11 },
	{    "light yellow",  "<138>", 12 },
	{    "light blue",    "<148>", 10 },
	{    "light magenta", "<158>", 13 },
	{    "light cyan",    "<168>", 10 },
	{    "light white",   "<178>", 11 },

	{    "dark black",    "<208>",  5 },
	{    "dark red",      "<218>",  4 },
	{    "dark green",    "<228>",  5 },
	{    "dark yellow",   "<238>",  6 },
	{    "dark blue",     "<248>",  4 },
	{    "dark magenta",  "<258>",  7 },
	{    "dark cyan",     "<268>",  4 },
	{    "dark white",    "<278>",  5 },

	{    "Azure",         "<acf>",  5 },
	{    "Ebony",         "<bbb>",  5 },
	{    "Jade",          "<afc>",  4 },
	{    "Lime",          "<cfa>",  4 },
	{    "Orange",        "<fca>",  6 },
	{    "Pink",          "<fac>",  4 },
	{    "Silver",        "<eee>",  6 },
	{    "Tan",           "<eda>",  3 },
	{    "Violet",        "<caf>",  6 },

	{    "reset",         "<088>",  5 },
	{    "light",         "<188>",  5 },
	{    "bold",          "<188>",  4 },
	{    "faint",         "<288>",  5 },
	{    "dim",           "<288>",  3 },
	{    "dark",          "<288>",  4 },
	{    "italic",        "<388>",  6 },
	{    "underscore",    "<488>", 10 },
	{    "blink",         "<588>",  5 },
	{    "reverse",       "<788>",  7 },

	{    "unitalic",     "\e[23m",  8 },
	{    "ununderscore", "\e[24m", 13 },
	{    "unblink",      "\e[25m",  8 },
	{    "unreverse",    "\e[27m", 10 },

	{    "black",         "<aaa>",  5 },
	{    "red",           "<daa>",  4 },
	{    "green",         "<ada>",  5 },
	{    "yellow",        "<dda>",  6 },
	{    "blue",          "<aad>",  4 },
	{    "magenta",       "<dad>",  7 },
	{    "cyan",          "<add>",  4 },
	{    "white",         "<ddd>",  5 },

	{    "Black",         "<bbb>",  5 },
	{    "Red",           "<faa>",  3 },
	{    "Green",         "<afa>",  5 },
	{    "Yellow",        "<ffa>",  6 },
	{    "Blue",          "<aaf>",  4 },
	{    "Magenta",       "<faf>",  7 },
	{    "Cyan",          "<aff>",  4 },
	{    "White",         "<fff>",  5 },

	{    "b black",       "<AAA>",  7 },
	{    "b red",         "<DAA>",  5 },
	{    "b green",       "<ADA>",  7 },
	{    "b yellow",      "<DDA>",  8 },
	{    "b blue",        "<AAD>",  6 },
	{    "b magenta",     "<DAD>",  9 },
	{    "b cyan",        "<ADD>",  6 },
	{    "b white",       "<DDD>",  7 },

	{    "b azure",       "<ABD>",  7 },
	{    "b ebony",       "<AAA>",  7 },
	{    "b jade",        "<ADB>",  6 },
	{    "b lime",        "<BDA>",  6 },
	{    "b orange",      "<DBA>",  8 },
	{    "b pink",        "<DAB>",  6 },
	{    "b silver",      "<CCC>",  8 },
	{    "b tan",         "<CBA>",  5 },
	{    "b violet",      "<BAD>",  8 },

	{    "b Azure",       "<ACF>",  7 },
	{    "b Black",       "<BBB>",  7 },
	{    "b Blue",        "<AAF>",  6 },
	{    "b Cyan",        "<AFF>",  6 },
	{    "b Ebony",       "<BBB>",  7 },
	{    "b Green",       "<AFA>",  7 },
	{    "b Jade",        "<AFC>",  6 },
	{    "b Lime",        "<CFA>",  6 },
	{    "b Magenta",     "<FAF>",  9 },
	{    "b Orange",      "<FCA>",  8 },
	{    "b Pink",        "<FAC>",  6 },
	{    "b Red",         "<FAA>",  5 },
	{    "b Silver",      "<EEE>",  8 },
	{    "b Tan",         "<EDA>",  5 },
	{    "b Violet",      "<CAF>",  8 },
	{    "b White",       "<FFF>",  7 },
	{    "b Yellow",      "<FFA>",  8 },
	{    "",              "<888>",  0 }
};

struct color_type map_color_table[] =
{
	{     "AVOID",            "<118>" },
	{     "BACKGROUND",       ""      },
	{     "BLOCK",            "<218>" },
	{     "EXITS",            "<278>" },
	{     "FOG",              "<148>" },
	{     "HIDE",             "<168>" },
	{     "INVISIBLE",        "<208>" },
	{     "PATHS",            "<138>" },
	{     "ROOMS",            "<178>" },
	{     "SYMBOLS",          "<128>" },
	{     "USER",             "<258>" },
	{     NULL,               "<888>" }
};

struct class_type class_table[] =
{
	{    "ASSIGN",            class_assign           },
	{    "CLEAR",             class_clear            },
	{    "CLOSE",             class_close            },
	{    "KILL",              class_kill             },
	{    "LIST",              class_list             },
	{    "LOAD",              class_load             },
	{    "OPEN",              class_open             },
	{    "READ",              class_read             },
	{    "SAVE",              class_save             },
	{    "SIZE",              class_size             },
	{    "WRITE",             class_write            },

	{    "",                  NULL                   },
};

struct chat_type chat_table[] =
{
	{     "ACCEPT",           chat_accept,         0, 1, "Accept a file transfer"		              },
	{     "CALL",             chat_call,           0, 0, "Call a buddy"                                   },
	{     "CANCELFILE",       chat_cancelfile,     1, 0, "Cancel a file transfer"                         },
	{     "COLOR",            chat_color,          1, 0, "Set the default chat color"                     },
	{     "DECLINE",          chat_decline,        1, 0, "Decline a file transfer"                        },
	{     "DND",              chat_dnd,            0, 0, "Decline new connections"                        },
	{     "DOWNLOADDIR",      chat_downloaddir,    1, 0, "Set the download directory"                     },
	{     "EMOTE",            chat_emote,          0, 1, "Send an emoted chat message"                    },
	{     "FORWARD",          chat_forward,        1, 0, "Forward all chat messages to a buddy"           },
	{     "FORWARDALL",       chat_forwardall,     1, 0, "Forward all chat/server messages to a buddy"    },
	{     "FILESTAT",         chat_filestat,       1, 0, "Show file transfer data"                        },
	{     "GROUP",            chat_group,          0, 1, "Assign a group to a buddy"                      },
	{     "IGNORE",           chat_ignore,         1, 0, "Ignore all messages from a buddy"               },
	{     "INITIALIZE",       chat_initialize,     1, 0, "Initialize chat with an optional port number"   },
	{     "INFO",             chat_info,           0, 0, "Display the chat settings"                      },
	{     "IP",               chat_ip,             1, 0, "Change the IP address, unset by default"        },
	{     "MESSAGE",          chat_message,        0, 1, "Send a private message to a buddy"              },
	{     "NAME",             chat_name,           1, 0, "Change the chat name"                           },
	{     "PASTE",            chat_paste,          0, 1, "Paste a block of text to a buddy"               },
	{     "PEEK",             chat_peek,           1, 0, "Show a buddy's public connections"              },
	{     "PING",             chat_ping,           1, 0, "Display a buddy's response time"                },
	{     "PREFIX",           chat_prefix,         1, 0, "Prefix before each chat message"                },
	{     "PRIVATE",          chat_private,        1, 0, "Do not share a buddy's IP address"              },
	{     "PUBLIC",           chat_public,         1, 0, "Share a buddy's IP address"                     },
	{     "REPLY",            chat_reply,          1, 0, "Reply to last private message"                  },
	{     "REQUEST",          chat_request,        1, 0, "Request a buddy's public connections"           },
	{     "SEND",             chat_send,           0, 1, "Send a raw data message to a buddy"             },
	{     "SENDFILE",         chat_sendfile,       0, 1, "Send a file to a buddy"                         },
	{     "SERVE",            chat_serve,          1, 0, "Forward all public chat messages to a buddy"    },
	{     "UNINITIALIZE",     chat_uninitialize,   0, 0, "Uninitializes the chat server"                  },
	{     "WHO",              chat_who,            0, 0, "Show all connections"                           },
	{     "ZAP",              chat_zap,            1, 0, "Close the connection to a buddy"                },
	{     "",                 NULL,                0, 0, ""                                               }
};

struct daemon_type daemon_table[] =
{
	{    "ATTACH",            daemon_attach,             "Attach to a daemon"                             },
	{    "DETACH",            daemon_detach,             "Turn into a daemon and detach"                  },
	{    "INPUT",             daemon_input,              "Send input to an attached daemon"               },
	{    "KILL",              daemon_kill,               "Kill a daemon"                                  },
	{    "LIST",              daemon_list,               "List a daemon"                                  },
	{    "",                  NULL,                      ""                                               }
};

struct port_type port_table[] =
{
	{     "CALL",             port_call,           0, 0, "Create outgoing socket connection"              },
	{     "COLOR",            port_color,          1, 0, "Set the default port message color"             },
	{     "FLAG",             port_flag,           0, 0, "Set various flags."                             },
	{     "GROUP",            port_group,          0, 1, "Assign a group to a socket"                     },
	{     "IGNORE",           port_ignore,         1, 0, "Ignore all messages from a socket"              },
	{     "INITIALIZE",       port_initialize,     0, 0, "Initialize port with optional file name"        },
	{     "INFO",             port_info,           0, 0, "Display the port settings"                      },
	{     "NAME",             port_name,           0, 0, "Change a socket name"                           },
	{     "PREFIX",           port_prefix,         1, 0, "Prefix before each port message"                },
	{     "PROXY",            port_proxy,          0, 0, "Assign a proxy session to a socket"             },
	{     "RANK",             port_rank,           0, 0, "Assign a rank to a socket"                      },
	{     "SEND",             port_send,           0, 1, "Send a message to a socket"                     },
	{     "UNINITIALIZE",     port_uninitialize,   0, 0, "Uninitializes the port"                         },
	{     "WHO",              port_who,            0, 0, "Show all socket connections"                    },
	{     "ZAP",              port_zap,            1, 0, "Close the connection to a socket"               },
	{     "",                 NULL,                0, 0, ""                                               }
};

struct rank_type rank_table[] =
{
	{     "SPY",              0                   },
	{     "SCOUT",            RANK_FLAG_SCOUT     }
};

struct array_type array_table[] =
{
	{     "ADD",              array_add,         "Add an item to a list table"             },
	{     "CLEAR",            array_clear,       "Clear a list"                            },
	{     "CLR",              array_clear,       NULL                                      },
	{     "COLLAPSE",         array_collapse,    "Collapse the list into a variable"       },
	{     "CREATE",           array_create,      "Create a list table with given items"    },
	{     "DELETE",           array_delete,      "Delete a list item with given index"     },
	{     "EXPLODE",          array_explode,     "Explode the variable into a list"        },
	{     "FIND",             array_find,        "Find a list item with given regex"       },
	{     "FND",              array_find,        NULL                                      },
	{     "GET",              array_get,         "Retrieve a list item with given index"   },
	{     "INDEX",            array_index,       "Index a list table for sorting"          },
	{     "INSERT",           array_insert,      "Insert a list item at given index"       },
	{     "ORDER",            array_order,       "Sort a list table numerically"           },
	{     "LENGTH",           array_size,        NULL                                      },
	{     "REVERSE",          array_reverse,     "Sort a list table in reverse order"      },
	{     "SET",              array_set,         "Change a list item at given index"       },
	{     "SHUFFLE",          array_shuffle,     "Sort a list table in random order"       },
	{     "SIMPLIFY",         array_simplify,    "Turn a list table into a simple list"    },
	{     "SIZE",             array_size,        NULL                                      },
	{     "SORT",             array_sort,        "Sort a list table alphabetically"        },
	{     "SRT",              array_sort,        NULL                                      },
	{     "TOKENIZE",         array_tokenize,    "Create a list with given characters"     },
	{     "",                 NULL,                                                        }
};

// 0 no map, 1 has map, 2 is inside map

struct map_type map_table[] =
{
	{     "AT",               map_at,              0,              1, "Execute command at given location"    },
	{     "CENTER",           map_center,          MAP_FLAG_VTMAP, 2, "Set the center of the map display"    },
	{     "COLOR",            map_color,           MAP_FLAG_VTMAP, 1, "Set the color for given field"        },
	{     "CREATE",           map_create,          MAP_FLAG_VTMAP, 0, "Creates the initial map"              },
	{     "DEBUG",            map_debug,           0,              2, "Obscure debug information"            },
	{     "DELETE",           map_delete,          MAP_FLAG_VTMAP, 1, "Delete the room at given direction"   },
	{     "DESTROY",          map_destroy,         MAP_FLAG_VTMAP, 1, "Destroy area or map"                  },
	{     "DIG",              map_dig,             MAP_FLAG_VTMAP, 2, "Create new room at given direction"   },
	{     "ENTRANCE",         map_entrance,        MAP_FLAG_VTMAP, 2, "Change the given exit's entrance"     },
	{     "EXIT",             map_exit,            MAP_FLAG_VTMAP, 2, "Change the given exit"                },
	{     "EXITFLAG",         map_exitflag,        MAP_FLAG_VTMAP, 2, "Change the given exit's flags"        },
	{     "EXPLORE",          map_explore,         MAP_FLAG_VTMAP, 2, "Save explored path to #path"          },
	{     "FIND",             map_find,            MAP_FLAG_VTMAP, 2, "Save found path to #path"             },
	{     "FLAG",             map_flag,            MAP_FLAG_VTMAP, 1, "Change the map's flags"               },
	{     "GET",              map_get,             0,              2, "Get various room values"              },
	{     "GLOBAL",           map_global,          0,              1, "Set the global exit room"             },
	{     "GOTO",             map_goto,            MAP_FLAG_VTMAP, 1, "Move to the given room"               },
	{     "INFO",             map_info,            0,              1, "Display map and room information"     },
	{     "INSERT",           map_insert,          MAP_FLAG_VTMAP, 2, "Insert a room at given direction"     },
	{     "JUMP",             map_jump,            MAP_FLAG_VTMAP, 2, "Move to the given coordinate"         },
	{     "LANDMARK",         map_landmark,        0,              1, "Create a global room reference"       },
	{     "LEAVE",            map_leave,           MAP_FLAG_VTMAP, 2, "Leave the map"                        },
	{     "LEGEND" ,          map_legend,          MAP_FLAG_VTMAP, 1, "Manipulate the map legend"            },
	{     "LINK",             map_link,            MAP_FLAG_VTMAP, 2, "Link room at given direction"         },
	{     "LIST",             map_list,            0,              2, "List matching rooms"                  },
	{     "MAP",              map_map,             0,              2, "Display the map"                      },
	{     "MOVE",             map_move,            MAP_FLAG_VTMAP, 2, "Move to the given direction"          },
	{     "NAME",             map_name,            MAP_FLAG_VTMAP, 2, "(obsolete) Use SET ROOMNAME instead"  },
	{     "OFFSET",           map_offset,          MAP_FLAG_VTMAP, 1, "Set the offset of the vt map"         },
	{     "READ",             map_read,            MAP_FLAG_VTMAP, 0, "Read a map file"                      },
	{     "RESIZE",           map_resize,          0,              1, "Resize the map room vnum range"       },
	{     "RETURN",           map_return,          MAP_FLAG_VTMAP, 1, "Return to last known room"            },
	{     "ROOMFLAG",         map_roomflag,        MAP_FLAG_VTMAP, 2, "Change the room's flags"              },
	{     "RUN",              map_run,             MAP_FLAG_VTMAP, 2, "Save found path to #path and run it"  },
	{     "SET",              map_set,             MAP_FLAG_VTMAP, 2, "Set various room values"              },
	{     "SYNC",             map_sync,            MAP_FLAG_VTMAP, 0, "Read a map file without overwriting"  },
	{     "TERRAIN",          map_terrain,         MAP_FLAG_VTMAP, 1, "Create a terrain type"                },
	{     "TRAVEL",           map_travel,          MAP_FLAG_VTMAP, 2, "Save explored path to #path and run it" },
	{     "UNDO",             map_undo,            MAP_FLAG_VTMAP, 2, "Undo last map action"                 },
	{     "UNINSERT",         map_uninsert,        MAP_FLAG_VTMAP, 2, "Uninsert room in given direction"     },
	{     "UNLANDMARK",       map_unlandmark,      MAP_FLAG_VTMAP, 1, "Remove a landmark"                    },
	{     "UNLINK",           map_unlink,          MAP_FLAG_VTMAP, 2, "Remove given exit"                    },
	{     "UNTERRAIN",        map_unterrain,       MAP_FLAG_VTMAP, 1, "Remove a terrain type"                },
	{     "UPDATE",           map_update,          0,              0, "Mark vt map for an auto update"       },
	{     "VNUM",             map_vnum,            MAP_FLAG_VTMAP, 2, "Change the room vnum to given vnum"   },
	{     "WRITE",            map_write,           0,              1, "Save the map to given file"           },
	{     "",                 NULL,                0,              0, ""                                     }
};


struct cursor_type cursor_table[] =
{
	{     "BACKSPACE",          "Delete backward character",                      "",           CURSOR_FLAG_GET_ONE,     cursor_backspace,             ""          },
	{     "BRACE",              "<OPEN|CLOSE> Insert a bracing character.",       "",            CURSOR_FLAG_GET_ONE,     cursor_brace,                 ""          },
	{     "BACKWARD",           "Move cursor backward",                           "",           CURSOR_FLAG_GET_ALL,     cursor_move_left,             ""          },
	{     "CLEAR",              "Delete the input line",                          "",           CURSOR_FLAG_GET_ALL,     cursor_clear_line,            ""          },
	{     "CLEAR LEFT",         "Delete from cursor to start of input",           "",           CURSOR_FLAG_GET_ALL,     cursor_clear_left,            ""          },
	{     "CLEAR LINE",         "",                                               "",            CURSOR_FLAG_GET_ALL,     cursor_clear_line,            ""          }, // obsolete
	{     "CLEAR RIGHT",        "Delete from cursor to end of input",             "",           CURSOR_FLAG_GET_ALL,     cursor_clear_right,           ""          },
	{     "CONVERT META",       "Meta convert the next character",                "",           CURSOR_FLAG_GET_ALL,     cursor_convert_meta,          ""          },
	{     "CTRL DELETE",        "Delete one character, exit on an empty line",    "",           CURSOR_FLAG_GET_ALL,     cursor_delete_or_exit,        ""          },
	{     "DELETE",             "Delete character at cursor",                     "\e[3~",       CURSOR_FLAG_GET_ALL,     cursor_delete,                ""          },
	{     "DELETE WORD LEFT",   "Delete backwards till next space",               "",           CURSOR_FLAG_GET_ALL,     cursor_delete_word_left,      ""          },
	{     "DELETE WORD RIGHT",  "Delete forwards till next space",                "\e[3;5~",     CURSOR_FLAG_GET_ALL,     cursor_delete_word_right,     ""          },
	{     "DOWN",               "Move cursor down",                               "\e[B",        CURSOR_FLAG_GET_ALL,     cursor_move_down,             ""          },
	{     "ECHO",               "",                                               "",            CURSOR_FLAG_GET_ONE,     cursor_echo,                  ""          },
//	{     "EDIT",               "",                                               "",            CURSOR_FLAG_GET_ONE,     cursor_edit,                  ""          },
	{     "END",                "Move cursor to end of input",                    "",           CURSOR_FLAG_GET_ALL,     cursor_end,                   ""          },
	{     "ENTER",              "Process the input line",                         "",            CURSOR_FLAG_GET_ALL,     cursor_enter,                 ""          },
	{     "FLAG",               "Set input echo/insert flags",                    "",            CURSOR_FLAG_GET_ONE,     cursor_flag,                  ""          },
	{     "FORWARD",            "Move cursor forward",                            "",           CURSOR_FLAG_GET_ALL,     cursor_move_right,            ""          },
	{     "GET",                "Copy input line to given variable",              "",            CURSOR_FLAG_GET_ONE,     cursor_get,                   ""          },
	{     "HISTORY NEXT",       "Select next command history entry",              "",           CURSOR_FLAG_GET_ALL,     cursor_history_next,          ""          },
	{     "HISTORY PREV",       "Select previous command history entry",          "",           CURSOR_FLAG_GET_ALL,     cursor_history_prev,          ""          },
	{     "HISTORY SEARCH",     "Search command history",                         "",           CURSOR_FLAG_GET_ALL,     cursor_history_search,        ""          },
	{     "HOME",               "Move the cursor to start of input",              "",           CURSOR_FLAG_GET_ALL,     cursor_home,                  ""          },
	{     "INFO",               "",                                               "",            CURSOR_FLAG_GET_ALL,     cursor_info,                  ""          },
//	{     "INSERT",             "",                                               "",            CURSOR_FLAG_GET_ONE,     NULL,                         ""          },
	{     "NEXT WORD",          "Move cursor to the next word",                   "\ef",         CURSOR_FLAG_GET_ALL,     cursor_move_right_word,       ""          },
	{     "MACRO",              "<PRESERVE|RESET>",                               "",            CURSOR_FLAG_GET_ONE,     cursor_macro,                 ""          },
	{     "PAGE",               "<DOWN|END|HOME|LOCK|UP>",                        "",                               0,    cursor_page,                  ""          },
	{     "PASTE BUFFER",       "Paste the previously deleted input text",        "",           CURSOR_FLAG_GET_ALL,     cursor_paste_buffer,          ""          },
	{     "POSITION",           "Move cursor position to given column.",          "",            CURSOR_FLAG_GET_ONE,     cursor_position,              ""          },
	{     "PRESERVE MACRO",     "",                                               "",            CURSOR_FLAG_GET_ALL,     cursor_macro,                 "PRESERVE"  }, // obsolete
	{     "PREV WORD",          "Move cursor to the previous word",               "\eb",         CURSOR_FLAG_GET_ALL,     cursor_move_left_word,        ""          },
	{     "REDRAW INPUT",       "Redraw the input line",                          "",           CURSOR_FLAG_GET_ALL,     cursor_redraw_input,          ""          },
	{     "RESET MACRO",        "",                                               "",            CURSOR_FLAG_GET_ALL,     cursor_macro,                 "RESET"     }, // obsolete
	{     "SET",                "Copy given string to input line",                "",            CURSOR_FLAG_GET_ONE,     cursor_set,                   ""          },
	{     "SOFT ENTER",         "Create a new line in edit mode.",                "\e[13;2u",    CURSOR_FLAG_GET_ALL,     cursor_soft_enter,            ""          },
	{     "SUSPEND",            "Suspend program, return with fg",                "",           CURSOR_FLAG_GET_ALL,     cursor_suspend,               ""          },
	{     "TAB",                "<LIST|SCROLLBACK> <BACKWARD|FORWARD>",           "",            CURSOR_FLAG_GET_ONE,     cursor_tab,                   ""          },
	{     "TAB L S BACKWARD",   "",                                               "\e[Z",        CURSOR_FLAG_GET_ALL,     cursor_tab,                   "L S B"     }, // shift tab
	{     "TAB L S FORWARD",    "",                                               "\t",          CURSOR_FLAG_GET_ALL,     cursor_tab,                   "L S F"     },
	{     "UP",                 "Move cursor up",                                 "\e[A",        CURSOR_FLAG_GET_ALL,     cursor_move_up,               ""          },
	{     "",                   "",                                               "\e[6~",                          0,    cursor_buffer_down,           ""          },
	{     "",                   "",                                               "\e[1;5F",                        0,    cursor_buffer_end,            ""          },
	{     "",                   "",                                               "\e[1;5H",                        0,    cursor_buffer_home,           ""          },
	{     "",                   "",                                               "\e[5~",                          0,    cursor_buffer_up,             ""          },
	{     "",                   "",                                               "\eOa",                           0,    cursor_buffer_up,             "1"         },
	{     "",                   "",                                               "\eOb",                           0,    cursor_buffer_down,           "1"         },
	{     "",                   "",                                               "\e[1;2a",                        0,    cursor_buffer_up,             ""          },
	{     "",                   "",                                               "\e[1;2b",                        0,    cursor_buffer_down,           ""          },
	{     "",                   "",                                               "",                              0,    cursor_buffer_lock,           ""          },
//	{     "",                   "",                                               "\e[13;2u",                       0,    cursor_enter,                 ""          },
	{     "",                   "",                                               "\eOM",                           0,    cursor_enter,                 ""          },
	{     "",                   "",                                               "\e[7~",                          0,    cursor_home,                  ""          },
	{     "",                   "",                                               "\e[1~",                          0,    cursor_home,                  ""          },
	{     "",                   "",                                               "\eOH",                           0,    cursor_home,                  ""          },
	{     "",                   "",                                               "\e[H",                           0,    cursor_home,                  ""          },
	{     "",                   "",                                               "\eOD",                           0,    cursor_move_left,             ""          },
	{     "",                   "",                                               "\e[D",                           0,    cursor_move_left,             ""          },
	{     "",                   "",                                               "\e[8~",                          0,    cursor_end,                   ""          },
	{     "",                   "",                                               "\e[4~",                          0,    cursor_end,                   ""          },
	{     "",                   "",                                               "\eOF",                           0,    cursor_end,                   ""          },
	{     "",                   "",                                               "\e[F",                           0,    cursor_end,                   ""          },
	{     "",                   "",                                               "\eOC",                           0,    cursor_move_right,            ""          },
	{     "",                   "",                                               "\e[C",                           0,    cursor_move_right,            ""          },
	{     "",                   "",                                               "\x7F",                           0,    cursor_backspace,             ""          },
	{     "",                   "",                                               "\eOB",                           0,    cursor_move_down,             ""          },
	{     "",                   "",                                               "\eOA",                           0,    cursor_move_up,               ""          },
	{     "",                   "",                                               "\e[1;5D",                        0,    cursor_move_left_word,        ""          },
	{     "",                   "",                                               "\e[1;5C",                        0,    cursor_move_right_word,       ""          },
	{     "",                   "",                                               "\e[1;5B",                        0,    cursor_move_page_down,        ""          },
	{     "",                   "",                                               "\e[1;5A",                        0,    cursor_move_page_up,          ""          },
	{     "",                   "",                                               "\e[127;5u",                      0,    cursor_clear_line,            ""          },
	{     "",                   "",                                               "\e\x7F",                         0,    cursor_delete_word_left,      ""          },
	{     "",                   "",                                               "\ed",                            0,    cursor_delete_word_right,     ""          },
	{     "",                   "",                                               "",                               0,    NULL,                         ""          }
};

struct draw_type draw_table[] =
{
	{
		"BOX",
		"Draw four sides of a box.",
		DRAW_FLAG_BOXED|DRAW_FLAG_LEFT|DRAW_FLAG_RIGHT|DRAW_FLAG_TOP|DRAW_FLAG_BOT,
		draw_box
	},

	{
		"BUFFER",
		"Draw the scrollback buffer.",
		0,
		draw_buffer
	},

	{
		"CORNER",
		"Draw a corner",
		DRAW_FLAG_CORNERED,
		draw_corner
	},

	{
		"LINE",
		"Draw a line.",
		DRAW_FLAG_NONE,
		draw_line
	},

	{
		"MAP",
		"Draw the map.",
		0,
		draw_map
	},

	{
		"RAIN",
		"Draw digital rain.",
		0,
		draw_rain
	},

	{
		"SIDE",
		"Draw a line with corners.",
		DRAW_FLAG_BOXED,
		draw_side
	},

	{
		"TABLE",
		"Draw a table.",
		DRAW_FLAG_BOXED|DRAW_FLAG_LEFT|DRAW_FLAG_RIGHT|DRAW_FLAG_TOP|DRAW_FLAG_BOT,
		draw_table_grid
	},

	{
		"TILE",
		"Draw a tile.",
		0,
		draw_square
	},

	{
		"",
		"",
		0,
		NULL
	}
};


struct edit_type edit_table[] =
{
	{      "CREATE",            edit_create,         "Create an editor"                  },
	{      "LOAD",              edit_load,           "Create an editor from a variable"  },
	{      "READ",              edit_read,           "Create an editor from file"        },
	{      "RESUME",            edit_resume,         "Resume editing after suspending"   },
	{      "SAVE",              edit_save,           "Save editor to a list variable"    },
	{      "SUSPEND",           edit_suspend,        "Suspend editing"                   },
	{      "WRITE",             edit_write,          "Write editor to file"              },
	{      "",                  NULL,                ""                                  },
};


struct scan_type scan_table[] =
{
	{       "ABORT",            scan_abort,   SCAN_FLAG_NONE,                "Abort a scan currently in progress."},
	{       "CSV",              scan_csv,     SCAN_FLAG_FILE|SCAN_FLAG_SCAN, "Scan a comma separated value file." },
	{       "DIR",              scan_dir,     SCAN_FLAG_FILE,                "Scan a directory to a variable."    },
	{       "FILE",             scan_file,    SCAN_FLAG_FILE,                "Scan a file all at once."           },
	{       "FORWARD",          scan_forward, SCAN_FLAG_FILE,                "Scan a file and send each line."    },
	{       "TSV",              scan_tsv,     SCAN_FLAG_FILE|SCAN_FLAG_SCAN, "Scan a tab separated value file."   },
	{       "TXT",              scan_txt,     SCAN_FLAG_FILE|SCAN_FLAG_SCAN, "Scan a text file line by line."     },
	{       "",                 NULL,         0,                             ""                                   }
};

struct screen_type screen_table[] =
{
	{
		"BLUR",
		"Shuffle the screen to the back of the desktop.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_blur
	},
	{
		"CLEAR",
		"Clear the screen.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_NONE,
		SCREEN_FLAG_CSIP,
		screen_clear
	},
	{
		"CURSOR",
		"Cursor settings.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_cursor
	},

	{
		"DUMP",
		"Dump the screen buffer.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_dump
	},

	{
		"FILL",
		"Fill given region with given character.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_fill
	},
	{
		"FOCUS",
		"Shuffle the screen to the front of the desktop.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_focus
	},
	{
		"FULLSCREEN",
		"Toggle fullscreen mode.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_fullscreen
	},
	{
		"GET",
		"Save screen information to given variable.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_get
	},
	{
		"INFO",
		"Show some debugging information.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_info
	},
	{
		"INPUTREGION",
		"Set the input region to {square}.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_inputregion
	},

	{
		"LOAD",
		"Load screen information from memory.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_OSCT,
		screen_load
	},
	{
		"MAXIMIZE",
		"Maximize or restore the screen.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_maximize
	},

	{
		"MINIMIZE",
		"Minimize or restore the screen.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_minimize
	},
	{
		"MOVE",
		"Move the screen to the given position.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_move
	},
	{
		"PRINT",
		"Print the screen dump to the screen.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_print
	},
	{
		"RAISE",
		"Raise a screen event.",
		SCREEN_FLAG_GET_ALL,
		SCREEN_FLAG_GET_ALL,
		SCREEN_FLAG_CSIP,
		screen_raise
	},
	{
		"REFRESH",
		"Force a refresh of the screen.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_refresh
	},
	{
		"RESCALE",
		"Rescale the screen to {height} {width} pixels.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_rescale
	},
	{
		"RESIZE",
		"Resize the screen to {rows} {cols} characters.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_resize
	},
	{
		"SAVE",
		"Save screen information to memory.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_save
	},

	{
		"SCROLLREGION",
		"Set the scroll region to {square}.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_scrollregion
	},

	{
		"SCROLLBAR",
		"Scrollbar settings.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_scrollbar
	},


	{
		"SET",
		"Set screen information.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_OSCT,
		screen_set
	},
	{
		"SWAP",
		"Swap the input and scroll region.",
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_GET_ONE,
		SCREEN_FLAG_CSIP,
		screen_swap
	},
	{
		"", "", 0, 0, 0, NULL
	}
};


struct timer_type timer_table[] =
{
	{    "Update Input"                },
	{    "Update Sessions"             },
	{    "Update Delays"               },
	{    "Update Daemon"               },
	{    "Update Chat"                 },
	{    "Update Port"                 },
	{    "Update Tickers"              },
	{    "Update Paths"                },
	{    "Update Packet Patcher"       },
	{    "Update Terminal"             },
	{    "Update Time Events"          },
	{    "Update Memory"               },
	{    "Stall Program"               }
};

struct event_type event_table[] =
{
	{    "BUFFER UPDATE",                          0, EVENT_FLAG_UPDATE,   "UPDATE",    "scrollback buffer update"   },	
	{    "CATCH ",                                 0, EVENT_FLAG_CATCH,    "CATCH",     "prefix for catch events"    },
	{    "CHAT MESSAGE",                           0, EVENT_FLAG_PORT,     "PORT",      "any chat related message"   },
	{    "CLASS ACTIVATED",                        0, EVENT_FLAG_CLASS,    "CLASS",     "class activations"          },
	{    "CLASS CREATED",                          0, EVENT_FLAG_CLASS,    "CLASS",     "class creation"             },
	{    "CLASS DEACTIVATED",                      0, EVENT_FLAG_CLASS,    "CLASS",     "class deactivations"        },
	{    "CLASS DESTROYED",                        0, EVENT_FLAG_CLASS,    "CLASS",     "class destruction"          },
	{    "DAEMON ATTACHED",                        0, EVENT_FLAG_SYSTEM,   "SYSTEM",    "daemon attachment"          },
	{    "DAEMON DETACHED",                        0, EVENT_FLAG_SYSTEM,   "SYSTEM",    "daemon detachment"          },
	{    "DATE",                                   0, EVENT_FLAG_TIME,     "TIME",      "the given date"             },
	{    "DAY",                                    0, EVENT_FLAG_TIME,     "TIME",      "each day or given day"      },
	{    "DISPLAY UPDATE",                         0, EVENT_FLAG_UPDATE,   "UPDATE",    "when display is updated"    },
	{    "DOUBLE-CLICKED ",                        0, EVENT_FLAG_MOUSE,    "MOUSE",     "mouse is double-clicked"    },
	{    "EDIT FINISHED",                          0, EVENT_FLAG_INPUT,    "INPUT",     "when editing is finished"   },
	{    "EDIT STARTED",                           0, EVENT_FLAG_INPUT,    "INPUT",     "when editing is started"    },
	{    "END OF PATH",                            0, EVENT_FLAG_MAP,      "MAP",       "walking the last room"      },
	{    "END OF RUN",                             0, EVENT_FLAG_MAP,      "MAP",       "running the last room"      },
	{    "GAG ",                                   0, EVENT_FLAG_GAG,      "GAG",       "prefix for gag events"      },
	{    "HOUR",                                   0, EVENT_FLAG_TIME,     "TIME",      "every hour or given hour"   },
	{    "IAC ",                                   0, EVENT_FLAG_TELNET,   "TELNET",    "prefix for telnet events"   },
	{    "LONG-CLICKED ",                          0, EVENT_FLAG_MOUSE,    "MOUSE",     "mouse is long-clicked"      },
	{    "MAP CREATE ROOM",                        0, EVENT_FLAG_MAP,      "MAP",       "a room is created"          },
	{    "MAP DELETE ROOM",                        0, EVENT_FLAG_MAP,      "MAP",       "a room is deleted"          },
	{    "MAP DOUBLE-CLICKED ",                    0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map click"               },
	{    "MAP ENTER MAP",                          0, EVENT_FLAG_MAP,      "MAP",       "entering the map"           },
	{    "MAP ENTER ROOM",                         0, EVENT_FLAG_MAP,      "MAP",       "entering a map room"        },
	{    "MAP EXIT MAP",                           0, EVENT_FLAG_MAP,      "MAP",       "exiting the map"            },
	{    "MAP EXIT ROOM",                          0, EVENT_FLAG_MAP,      "MAP",       "exiting a map room"         },
	{    "MAP FOLLOW MAP",                         0, EVENT_FLAG_MAP,      "MAP",       "moving to a map room"       },
	{    "MAP LOCATION",                           0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map click"               },
	{    "MAP LONG-CLICKED ",                      0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map click"               },
	{    "MAP MOUSE LOCATION",                     0, EVENT_FLAG_MOUSE,    "MOUSE",     "called by #screen raise"    }, 
	{    "MAP MOVED ",                             0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map mouse move"          },
	{    "MAP PRESSED ",                           0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map click"               },
	{    "MAP REGION ",                            0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map mouse events"        },
	{    "MAP RELEASED ",                          0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map click"               },
	{    "MAP ROOM ",                              0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map room mouse events"   },
	{    "MAP SCROLLED ",                          0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map scroll"              },
	{    "MAP SHORT-CLICKED ",                     0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map click"               },
	{    "MAP TRIPLE-CLICKED ",                    0, EVENT_FLAG_MOUSE,    "MOUSE",     "vt map click"               },
	{    "MAP UPDATED VTMAP",                      0, EVENT_FLAG_MAP,      "MAP",       "vt map update"              },
	{    "MINUTE",                                 0, EVENT_FLAG_TIME,     "TIME",      "minute or given minute"     },
	{    "MONTH",                                  0, EVENT_FLAG_TIME,     "TIME",      "month or given month"       },
	{    "MOVED ",                                 0, EVENT_FLAG_MOUSE,    "MOUSE",     "mouse is moved"             },
	{    "PORT CONNECTION",                        0, EVENT_FLAG_PORT,     "PORT",      "socket connects"            },
	{    "PORT DISCONNECTION",                     0, EVENT_FLAG_PORT,     "PORT",      "socket disconnects"         },
	{    "PORT INITIALIZED",                       0, EVENT_FLAG_PORT,     "PORT",      "port is initialized"        },
	{    "PORT LOG MESSAGE",                       0, EVENT_FLAG_PORT,     "PORT",      "local port log messages"    },
	{    "PORT MESSAGE",                           0, EVENT_FLAG_PORT,     "PORT",      "local port messages"        },
	{    "PORT RECEIVED MESSAGE",                  0, EVENT_FLAG_PORT,     "PORT",      "socket data is received"    },
	{    "PORT UNINITIALIZED",                     0, EVENT_FLAG_PORT,     "PORT",      "port is uninitialized"      },
	{    "PRESSED ",                               0, EVENT_FLAG_MOUSE,    "MOUSE",     "mouse button is pressed"    },
	{    "PROCESSED KEYPRESS",                     0, EVENT_FLAG_INPUT,    "INPUT",     "after a regular keypress"   },
	{    "PROGRAM START",                          0, EVENT_FLAG_SYSTEM,   "SYSTEM",    "main session starts"        },

	{    "PROGRAM TERMINATION",                    0, EVENT_FLAG_SYSTEM,   "SYSTEM",    "main session exists"        },
	{    "READ ERROR",                             0, EVENT_FLAG_SYSTEM,   "SYSTEM",    "the read command fails"     },
	{    "RECEIVED ERROR",                         0, EVENT_FLAG_SYSTEM,   "SYSTEM",    "an error is received"       },
	{    "RECEIVED INPUT",                         0, EVENT_FLAG_INPUT,    "INPUT",     "keyboard input is received" },
	{    "RECEIVED KEYPRESS",                      0, EVENT_FLAG_INPUT,    "INPUT",     "a keypress is received"     },
	{    "RECEIVED LINE",                          0, EVENT_FLAG_OUTPUT,   "OUTPUT",    "a new line is received"     },
	{    "RECEIVED OUTPUT",                        0, EVENT_FLAG_OUTPUT,   "OUTPUT",    "bulk output is received"    },
	{    "RECEIVED PROMPT",                        0, EVENT_FLAG_OUTPUT,   "OUTPUT",    "a prompt is received"       },
	{    "RELEASED ",                              0, EVENT_FLAG_MOUSE,    "MOUSE",     "mouse button is released"   },
	{    "SCAN CSV HEADER",                        0, EVENT_FLAG_SCAN,     "SCAN",      "scanning a csv file"        },
	{    "SCAN CSV LINE",                          0, EVENT_FLAG_SCAN,     "SCAN",      "scanning a csv file"        },
	{    "SCAN TSV HEADER",                        0, EVENT_FLAG_SCAN,     "SCAN",      "scanning a tsv file"        },
	{    "SCAN TSV LINE",                          0, EVENT_FLAG_SCAN,     "SCAN",      "scanning a tsv file"        },
	{    "SCREEN DESKTOP DIMENSIONS",              0, EVENT_FLAG_SCREEN,   "SCREEN",    "called by #screen raise"    },
	{    "SCREEN DESKTOP SIZE",                    0, EVENT_FLAG_SCREEN,   "SCREEN",    "called by #screen raise"    },
	{    "SCREEN DIMENSIONS",                      0, EVENT_FLAG_SCREEN,   "SCREEN",    "called by #screen raise"    },
	{    "SCREEN FILL",                            0, EVENT_FLAG_SCREEN,   "SCREEN",    "split bars are filled"      },
	{    "SCREEN FOCUS",                           0, EVENT_FLAG_MOUSE,    "MOUSE",     "focus changes",             },
	{    "SCREEN LOCATION",                        0, EVENT_FLAG_SCREEN,   "SCREEN",    "called by #screen raise"    },
	{    "SCREEN MINIMIZED",                       0, EVENT_FLAG_SCREEN,   "SCREEN",    "called by #screen raise"    },
	{    "SCREEN MOUSE LOCATION",                  0, EVENT_FLAG_MOUSE,    "MOUSE",     "called by #screen raise"    },
	{    "SCREEN REFRESH",                         0, EVENT_FLAG_SCREEN,   "SCREEN",    "the screen is refreshed"    },
	{    "SCREEN RESIZE",                          0, EVENT_FLAG_SCREEN,   "SCREEN",    "the screen is resized"      },
	{    "SCREEN ROTATE LANDSCAPE",                0, EVENT_FLAG_SCREEN,   "SCREEN",    "the screen is rotated"      },
	{    "SCREEN ROTATE PORTRAIT",                 0, EVENT_FLAG_SCREEN,   "SCREEN",    "the screen is rotated"      },
	{    "SCREEN SIZE",                            0, EVENT_FLAG_SCREEN,   "SCREEN",    "called by #screen raise"    },
	{    "SCREEN SPLIT",                           0, EVENT_FLAG_SCREEN,   "SCREEN",    "the screen is split"        },
	{    "SCREEN SPLIT FILL",                      0, EVENT_FLAG_SCREEN,   "SCREEN",    "split region is filled"     },
	{    "SCREEN UNSPLIT",                         0, EVENT_FLAG_SCREEN,   "SCREEN",    "the screen is unsplit"      },
	{    "SCROLLBAR MOVE",                         0, EVENT_FLAG_SCREEN,   "SCREEN",    "scrollbar movement"         },
	{    "SCROLLBAR POSITION",                     0, EVENT_FLAG_SCREEN,   "SCREEN",    "scrollbar position"         },
	{    "SCROLLBAR UPDATE",                       0, EVENT_FLAG_UPDATE,   "UPDATE",    "scrollbar update"           },
	{    "SCROLLED ",                              0, EVENT_FLAG_MOUSE,    "MOUSE",     "mouse wheel is scrolled"    },
	{    "SECOND",                                 0, EVENT_FLAG_TIME,     "TIME",      "second or given second"     },
	{    "SEND OUTPUT",                            0, EVENT_FLAG_INPUT,    "INPUT",     "before sending output"      },
	{    "SENT OUTPUT",                            0, EVENT_FLAG_INPUT,    "INPUT",     "after sending output"       },
	{    "SESSION ACTIVATED",                      0, EVENT_FLAG_SESSION,  "SESSION",   "a session is activated"     },
	{    "SESSION CONNECTED",                      0, EVENT_FLAG_SESSION,  "SESSION",   "a new session connects"     },
	{    "SESSION CREATED",                        0, EVENT_FLAG_SESSION,  "SESSION",   "a new session is created"   },
	{    "SESSION DEACTIVATED",                    0, EVENT_FLAG_SESSION,  "SESSION",   "a session is deactivated"   },
	{    "SESSION DESTROYED",                      0, EVENT_FLAG_SESSION,  "SESSION",   "a session is destroyed"     },
	{    "SESSION DISCONNECTED",                   0, EVENT_FLAG_SESSION,  "SESSION",   "a session disconnects"      },
	{    "SESSION TIMED OUT",                      0, EVENT_FLAG_SESSION,  "SESSION",   "a session doesn't connect"  },
	{    "SHORT-CLICKED",                          0, EVENT_FLAG_MOUSE,    "MOUSE",     "mouse is short-clicked"     },
	{    "SWIPED",                                 0, EVENT_FLAG_MOUSE,    "MOUSE",     "mouse swipe"                },
	{    "SYSTEM CRASH",                           0, EVENT_FLAG_SYSTEM,   "SYSTEM",    "system crash"               },
	{    "SYSTEM ERROR",                           0, EVENT_FLAG_SYSTEM,   "SYSTEM",    "system errors"              },
	{    "TIME",                                   0, EVENT_FLAG_TIME,     "TIME",      "the given time"             },
	{    "TRIPLE-CLICKED",                         0, EVENT_FLAG_MOUSE,    "MOUSE",     "mouse is triple-clicked"    },
	{    "UNKNOWN COMMAND",                        0, EVENT_FLAG_SYSTEM,   "SYSTEM",    "unknown tintin command"     },
	{    "VARIABLE UPDATE ",                       0, EVENT_FLAG_VARIABLE, "VARIABLE",  "before a variable updates"  },
	{    "VARIABLE UPDATED ",                      0, EVENT_FLAG_VARIABLE, "VARIABLE",  "after a variable updates"   },
	{    "VT100 CPR",                              0, EVENT_FLAG_VT100,    "VT100",     "an ESC [ 6 n call"          },
	{    "VT100 DA",                               0, EVENT_FLAG_VT100,    "VT100",     "an ESC [ c call"            },
	{    "VT100 DECID",                            0, EVENT_FLAG_VT100,    "VT100",     "an ESC Z call"              },
	{    "VT100 DSR",                              0, EVENT_FLAG_VT100,    "VT100",     "an ESC [ 5 n call"          },
	{    "VT100 ENQ",                              0, EVENT_FLAG_VT100,    "VT100",     "an \\x05 call"              },
	{    "VT100 SCROLL REGION",                    0, EVENT_FLAG_VT100,    "VT100",     "vt100 scroll region change" },
	{    "WEEK",                                   0, EVENT_FLAG_TIME,     "TIME",      "each week or given week"    },
	{    "WRITE ERROR",                            0, EVENT_FLAG_SYSTEM,   "SYSTEM",    "the write command fails"    },
	{    "YEAR",                                   0, EVENT_FLAG_TIME,     "TIME",      "each year or given year"    },
	{    "",                                       0, 0,                   "",          ""                           }
};

struct path_type path_table[] =
{
	{    "CREATE",            path_create,         "Clear the path and start path mapping."         },
	{    "DELETE",            path_delete,         "Delete the last command from the path."         },
	{    "DESCRIBE",          path_describe,       "Describe the path and current position."        },
	{    "DESTROY",           path_destroy,        "Clear the path and stop path mapping."          },
	{    "END",               path_end,            ""                                               },
	{    "GET",               path_get,            "Store a path value into a variable."            },
	{    "GOTO",              path_goto,           "Move position to given index."                  },
	{    "INSERT",            path_insert,         "Insert a command to the end of the path."       },
	{    "LOAD",              path_load,           "Load a path from a variable."                   },
	{    "MAP",               path_map,            "Display the path and current position."         },
	{    "MOVE",              path_move,           "Move one position forward or backward."         },
	{    "NEW",               path_new,            ""                                               },
	{    "RUN",               path_run,            "Execute the current path with optional delay."  },
	{    "SAVE",              path_save,           "Save the current path to the given variable."   },
	{    "SHOW",              path_map,            ""                                               },
	{    "START",             path_start,          "Start path mapping."                            },
	{    "STOP",              path_stop,           "Stop path mapping."                             },
	{    "SWAP",              path_swap,           "Reverse the path, forward becoming backward."   },
	{    "UNDO",              path_undo,           "Undo last step."                                },
	{    "UNZIP",             path_unzip,          "Turn speedwalk into a path."                    },
	{    "WALK",              path_walk,           "Walk one step forward or backward."             },
	{    "ZIP",               path_zip,            "Turn path into a speedwalk."                    },
	{    "",                  NULL,                ""                                               }
};

struct line_type line_table[] =
{
	{    "BACKGROUND",        line_background,     "Execute line without stealing session focus."   },
	{    "BENCHMARK",         line_benchmark,      "Execute line and provide timing information."   },
	{    "CAPTURE",           line_capture,        "Capture output in the given variable."          },
	{    "CONVERT",           line_convert,        "Execute line in convert meta data mode."        },
	{    "DEBUG",             line_debug,          "Execute line in debug mode."                    },
	{    "GAG",               line_gag,            "Gag the next line."                             },
	{    "IGNORE",            line_ignore,         "Execute line with triggers ignored."            },
	{    "LOCAL",             line_local,          "Execute line with local scope."                 },
	{    "LOG",               line_log,            "Log the next line or given line."               },
	{    "LOGMODE",           line_logmode,        "Execute line with given log mode."              },
	{    "LOGVERBATIM",       line_logverbatim,    "Log the line as plain text verbatim."           },
	{    "MSDP",              line_msdp,           "Execute line with msdp conversion."             },
	{    "MULTISHOT",         line_multishot,      "Execute line creating multishot triggers."      },
	{    "ONESHOT",           line_oneshot,        "Execute line creating oneshot triggers."        },
	{    "QUIET",             line_quiet,          "Execute line with all system messages off."     },
	{    "STRIP",             line_strip,          "Execute line with escape codes stripped."       },
	{    "SUBSTITUTE",        line_substitute,     "Execute line with given substitution."          },
	{    "VERBATIM",          line_verbatim,       "Execute line as plain text."                    },
	{    "VERBOSE",           line_verbose,        "Execute line with all system messages on."      },
	{    "",                  NULL,                ""                                               }
};

struct log_type log_table[] =
{
	{    "APPEND",            log_append,          "Start logging, appending to give file."         },
	{    "INFO",              log_info,            "Some logging related info."                     },
	{    "OFF",               log_off,             "Stop logging."                                  },
	{    "OVERWRITE",         log_overwrite,       "Start logging, overwriting the given file."     },
	{    "",                  NULL,                ""                                               }
};

struct history_type history_table[] =
{
//	{    "CHARACTER",         history_character,   "Set the character used for repeating commands." },
	{    "DELETE",            history_delete,      "Delete last command history entry."             },
	{    "GET",               history_get,         "Store in given variable a given index or range." },
	{    "INSERT",            history_insert,      "Insert a new command history entry."            },
	{    "LIST",              history_list,        "Display command history list."                  },
	{    "READ",              history_read,        "Read a command history list from file."         },
//	{    "SIZE",              history_size,        "The size of the command history."               },
	{    "WRITE",             history_write,       "Write a command history list to file."          },
	{    "",                  NULL,                ""                                               }
};

struct buffer_type buffer_table[] =
{
	{    "CLEAR",             buffer_clear,        "Clear buffer."                                  },
	{    "DOWN",              buffer_down,         "Scroll down one page."                          },
	{    "END",               buffer_end,          "Scroll down to the end of the buffer."          },
	{    "FIND",              buffer_find,         "Move to the given string in the buffer."        },
	{    "GET",               buffer_get,          "Store in given variable a given line or range." },
	{    "HOME",              buffer_home,         "Scroll up to the start of the buffer."          },
	{    "INFO",              buffer_info,         "Display statistics about the buffer."           },
	{    "JUMP",              buffer_jump,         "Jump to given line."                            },
	{    "LOCK",              buffer_lock,         "Toggle the locking state of the buffer."        },
	{    "REFRESH",           buffer_refresh,      "Refresh the buffer display."                    },
	{    "UP",                buffer_up,           "Scroll up one page."                            },
	{    "WRITE",             buffer_write,        "Write the buffer to file."                      },
	{    "",                  NULL,                ""                                               }
};

char *telcmds[] =
{
        "EOF",    "SUSP",   "ABORT",  "EOR",    "SE",
        "NOP",    "DMARK",  "BRK",    "IP",     "AO",
        "AYT",    "EC",     "EL",     "GA",     "SB",
        "WILL",   "WONT",   "DO",     "DONT",   "IAC",
};

struct telopt_type telopt_table[] =
{
	{    "BINARY",            TEL_N,               0 },
	{    "ECHO",              TEL_Y,               0 },
	{    "RCP",               TEL_N,               0 },
	{    "SGA",               TEL_Y,               0 },
	{    "NAME",              TEL_N,               0 },
	{    "STATUS",            TEL_N,               0 },
	{    "TIMING MARK",       TEL_N,               0 },
	{    "RCTE",              TEL_N,               0 },
	{    "NAOL",              TEL_N,               0 },
	{    "NAOP",              TEL_N,               0 },
	{    "NAORCD",            TEL_N,               0 }, /* 10 */
	{    "NAOHTS",            TEL_N,               0 },
	{    "NAOHTD",            TEL_N,               0 },
	{    "NAOFFD",            TEL_N,               0 },
	{    "NAOVTS",            TEL_N,               0 },
	{    "NAOVTD",            TEL_N,               0 },
	{    "NAOLFD",            TEL_N,               0 },
	{    "EXTEND ASCII",      TEL_N,               0 },
	{    "LOGOUT",            TEL_N,               0 },
	{    "BYTE MACRO",        TEL_N,               0 },
	{    "DATA ENTRY TERML",  TEL_N,               0 }, /* 20 */
	{    "SUPDUP",            TEL_N,               0 },
	{    "SUPDUP OUTPUT",     TEL_N,               0 },
	{    "SEND LOCATION",     TEL_N,               0 },
	{    "TERMINAL TYPE",     TEL_Y,               ANNOUNCE_DO },
	{    "EOR",               TEL_Y,               0 },
	{    "TACACS UID",        TEL_N,               0 },
	{    "OUTPUT MARKING",    TEL_N,               0 },
	{    "TTYLOC",            TEL_N,               0 },
	{    "3270 REGIME",       TEL_N,               0 },
	{    "X.3 PAD",           TEL_N,               0 }, /* 30 */
	{    "NAWS",              TEL_Y,               ANNOUNCE_DO },
	{    "TSPEED",            TEL_Y,               0 },
	{    "LFLOW",             TEL_N,               0 },
	{    "LINEMODE",          TEL_N,               0 },
	{    "XDISPLOC",          TEL_N,               0 },
	{    "OLD-ENVIRON",       TEL_N,               0 },
	{    "AUTH",              TEL_N,               0 },
	{    "ENCRYPT",           TEL_N,               0 },
	{    "NEW-ENVIRON",       TEL_Y,               ANNOUNCE_DO },
	{    "TN3270E",           TEL_N,               0 }, /* 40 */
	{    "XAUTH",             TEL_N,               0 },
	{    "CHARSET",           TEL_Y,               ANNOUNCE_WILL },
	{    "RSP",               TEL_N,               0 },
	{    "COM PORT",          TEL_N,               0 },
	{    "SLE",               TEL_N,               0 },
	{    "STARTTLS",          TEL_N,               0 },
	{    "KERMIT",            TEL_N,               0 },
	{    "SEND-URL",          TEL_N,               0 },
	{    "FORWARD_X",         TEL_N,               0 },
	{    "50",                TEL_N,               0 }, /* 50 */
	{    "51",                TEL_N,               0 },
	{    "52",                TEL_N,               0 },
	{    "53",                TEL_N,               0 },
	{    "54",                TEL_N,               0 },
	{    "55",                TEL_N,               0 },
	{    "56",                TEL_N,               0 },
	{    "57",                TEL_N,               0 },
	{    "58",                TEL_N,               0 },
	{    "59",                TEL_N,               0 },
	{    "60",                TEL_N,               0 }, /* 60 */
	{    "61",                TEL_N,               0 },
	{    "62",                TEL_N,               0 },
	{    "63",                TEL_N,               0 },
	{    "64",                TEL_N,               0 },
	{    "65",                TEL_N,               0 },
	{    "66",                TEL_N,               0 },
	{    "67",                TEL_N,               0 },
	{    "68",                TEL_N,               0 },
	{    "MSDP",              TEL_N,               ANNOUNCE_WILL }, /* Mud Server Data Protocol */
	{    "MSSP",              TEL_N,               ANNOUNCE_WILL }, /* Mud Server Status Protocol */
	{    "71",                TEL_N,               0 },
	{    "72",                TEL_N,               0 },
	{    "73",                TEL_N,               0 },
	{    "74",                TEL_N,               0 },
	{    "75",                TEL_N,               0 },
	{    "76",                TEL_N,               0 },
	{    "77",                TEL_N,               0 },
	{    "78",                TEL_N,               0 },
	{    "79",                TEL_N,               0 },
	{    "80",                TEL_N,               0 }, /* 80 */
	{    "81",                TEL_N,               0 },
	{    "82",                TEL_N,               0 },
	{    "83",                TEL_N,               0 },
	{    "84",                TEL_N,               0 },
	{    "MCCP1",             TEL_N,               0 }, /* Obsolete */
	{    "MCCP2",             TEL_Y,               ANNOUNCE_WILL }, /* Mud Client Compression Protocol v2 */
	{    "MCCP3",             TEL_N,               ANNOUNCE_WILL }, /* Mud Client Compression Protocol v3 */
	{    "88",                TEL_N,               0 },
	{    "89",                TEL_N,               0 },
	{    "MSP",               TEL_N,               0 }, /* Mud Sound Protocl */
	{    "MXP",               TEL_N,               0 }, /* Mud eXtension Protocol */
	{    "92",                TEL_N,               0 }, /* Unadopted - MSP2 draft */
	{    "ZMP",               TEL_N,               0 }, /* Unadopted - Zenith Mud Protocl draft */
	{    "94",                TEL_N,               0 },
	{    "95",                TEL_N,               0 },
	{    "96",                TEL_N,               0 },
	{    "97",                TEL_N,               0 },
	{    "98",                TEL_N,               0 },
	{    "99",                TEL_N,               0 },
	{    "100",               TEL_N,               0 },
	{    "101",               TEL_N,               0 },
	{    "102",               TEL_N,               0 }, /* Obsolete - Aardwolf */
	{    "103",               TEL_N,               0 },
	{    "104",               TEL_N,               0 },
	{    "105",               TEL_N,               0 },
	{    "106",               TEL_N,               0 },
	{    "107",               TEL_N,               0 },
	{    "108",               TEL_N,               0 },
	{    "109",               TEL_N,               0 },
	{    "110",               TEL_N,               0 },
	{    "111",               TEL_N,               0 },
	{    "112",               TEL_N,               0 },
	{    "113",               TEL_N,               0 },
	{    "114",               TEL_N,               0 },
	{    "115",               TEL_N,               0 },
	{    "116",               TEL_N,               0 },
	{    "117",               TEL_N,               0 },
	{    "118",               TEL_N,               0 },
	{    "119",               TEL_N,               0 },
	{    "120",               TEL_N,               0 },
	{    "121",               TEL_N,               0 },
	{    "122",               TEL_N,               0 },
	{    "123",               TEL_N,               0 },
	{    "124",               TEL_N,               0 },
	{    "125",               TEL_N,               0 },
	{    "126",               TEL_N,               0 },
	{    "127",               TEL_N,               0 },
	{    "128",               TEL_N,               0 },
	{    "129",               TEL_N,               0 },
	{    "130",               TEL_N,               0 },
	{    "131",               TEL_N,               0 },
	{    "132",               TEL_N,               0 },
	{    "133",               TEL_N,               0 },
	{    "134",               TEL_N,               0 },
	{    "135",               TEL_N,               0 },
	{    "136",               TEL_N,               0 },
	{    "137",               TEL_N,               0 },
	{    "138",               TEL_N,               0 },
	{    "139",               TEL_N,               0 },
	{    "140",               TEL_N,               0 },
	{    "141",               TEL_N,               0 },
	{    "142",               TEL_N,               0 },
	{    "143",               TEL_N,               0 },
	{    "144",               TEL_N,               0 },
	{    "145",               TEL_N,               0 },
	{    "146",               TEL_N,               0 },
	{    "147",               TEL_N,               0 },
	{    "148",               TEL_N,               0 },
	{    "149",               TEL_N,               0 },
	{    "150",               TEL_N,               0 },
	{    "151",               TEL_N,               0 },
	{    "152",               TEL_N,               0 },
	{    "153",               TEL_N,               0 },
	{    "154",               TEL_N,               0 },
	{    "155",               TEL_N,               0 },
	{    "156",               TEL_N,               0 },
	{    "157",               TEL_N,               0 },
	{    "158",               TEL_N,               0 },
	{    "159",               TEL_N,               0 },
	{    "160",               TEL_N,               0 },
	{    "161",               TEL_N,               0 },
	{    "162",               TEL_N,               0 },
	{    "163",               TEL_N,               0 },
	{    "164",               TEL_N,               0 },
	{    "165",               TEL_N,               0 },
	{    "166",               TEL_N,               0 },
	{    "167",               TEL_N,               0 },
	{    "168",               TEL_N,               0 },
	{    "169",               TEL_N,               0 },
	{    "170",               TEL_N,               0 },
	{    "171",               TEL_N,               0 },
	{    "172",               TEL_N,               0 },
	{    "173",               TEL_N,               0 },
	{    "174",               TEL_N,               0 },
	{    "175",               TEL_N,               0 },
	{    "176",               TEL_N,               0 },
	{    "177",               TEL_N,               0 },
	{    "178",               TEL_N,               0 },
	{    "179",               TEL_N,               0 },
	{    "180",               TEL_N,               0 },
	{    "181",               TEL_N,               0 },
	{    "182",               TEL_N,               0 },
	{    "183",               TEL_N,               0 },
	{    "184",               TEL_N,               0 },
	{    "185",               TEL_N,               0 },
	{    "186",               TEL_N,               0 },
	{    "187",               TEL_N,               0 },
	{    "188",               TEL_N,               0 },
	{    "189",               TEL_N,               0 },
	{    "190",               TEL_N,               0 },
	{    "191",               TEL_N,               0 },
	{    "192",               TEL_N,               0 },
	{    "193",               TEL_N,               0 },
	{    "194",               TEL_N,               0 },
	{    "195",               TEL_N,               0 },
	{    "196",               TEL_N,               0 },
	{    "197",               TEL_N,               0 },
	{    "198",               TEL_N,               0 },
	{    "199",               TEL_N,               0 },
	{    "ATCP",              TEL_N,               0 }, /* Obsolete - Achaea Telnet Communication Protocol */
	{    "GMCP",              TEL_N,               ANNOUNCE_WILL }, /* MSDP over GMCP */
	{    "202",               TEL_N,               0 },
	{    "203",               TEL_N,               0 },
	{    "204",               TEL_N,               0 },
	{    "205",               TEL_N,               0 },
	{    "206",               TEL_N,               0 },
	{    "207",               TEL_N,               0 },
	{    "208",               TEL_N,               0 },
	{    "209",               TEL_N,               0 },
	{    "210",               TEL_N,               0 },
	{    "211",               TEL_N,               0 },
	{    "212",               TEL_N,               0 },
	{    "213",               TEL_N,               0 },
	{    "214",               TEL_N,               0 },
	{    "215",               TEL_N,               0 },
	{    "216",               TEL_N,               0 },
	{    "217",               TEL_N,               0 },
	{    "218",               TEL_N,               0 },
	{    "219",               TEL_N,               0 },
	{    "220",               TEL_N,               0 },
	{    "221",               TEL_N,               0 },
	{    "222",               TEL_N,               0 },
	{    "223",               TEL_N,               0 },
	{    "224",               TEL_N,               0 },
	{    "225",               TEL_N,               0 },
	{    "226",               TEL_N,               0 },
	{    "227",               TEL_N,               0 },
	{    "228",               TEL_N,               0 },
	{    "229",               TEL_N,               0 },
	{    "230",               TEL_N,               0 },
	{    "231",               TEL_N,               0 },
	{    "232",               TEL_N,               0 },
	{    "233",               TEL_N,               0 },
	{    "234",               TEL_N,               0 },
	{    "235",               TEL_N,               0 },
	{    "236",               TEL_N,               0 },
	{    "237",               TEL_N,               0 },
	{    "238",               TEL_N,               0 },
	{    "239",               TEL_N,               0 },
	{    "240",               TEL_N,               0 },
	{    "NOP",               TEL_N,               0 },
	{    "242",               TEL_N,               0 },
	{    "243",               TEL_N,               0 },
	{    "244",               TEL_N,               0 },
	{    "245",               TEL_N,               0 },
	{    "246",               TEL_N,               0 },
	{    "247",               TEL_N,               0 },
	{    "248",               TEL_N,               0 },
	{    "249",               TEL_N,               0 },
	{    "250",               TEL_N,               0 },
	{    "251",               TEL_N,               0 },
	{    "252",               TEL_N,               0 },
	{    "253",               TEL_N,               0 },
	{    "254",               TEL_N,               0 },
	{    "255",               TEL_N,               0 }
};


struct map_legend_type map_legend_table[] =
{
	{ "NO EXITS",		"ASCII NESW LINE",	"x",	"1x1"	},
	{ "N",			"ASCII NESW LINE",	"o",	"1x1"	},
	{ "  E",		"ASCII NESW LINE",	"o",	"1x1"	},
	{ "N E",		"ASCII NESW LINE",	"+",	"1x1"	},
	{ "    S",		"ASCII NESW LINE",	"o",	"1x1"	},
	{ "N   S",		"ASCII NESW LINE",	"|",	"1x1"	},
	{ "  E S",		"ASCII NESW LINE",	"+",	"1x1"	},
	{ "N E S",		"ASCII NESW LINE",	"+",	"1x1"	},
	{ "      W",		"ASCII NESW LINE",	"o",	"1x1"	},
	{ "N     W",		"ASCII NESW LINE",	"+",	"1x1"	},
	{ "  E   W",		"ASCII NESW LINE",	"-",	"1x1"	},
	{ "N E   W",		"ASCII NESW LINE",	"+",	"1x1"	},
	{ "    S W",		"ASCII NESW LINE",	"+",	"1x1"	},
	{ "N   S W",		"ASCII NESW LINE",	"+",	"1x1"	},
	{ "  E S W",		"ASCII NESW LINE",	"+",	"1x1"	},
	{ "N E S W",		"ASCII NESW LINE",	"+",	"1x1"	},

	{ "USER",		"ASCII NESW MISC",	"x",	"1x1"	},
	{ "DIR UNKNOWN",	"ASCII NESW MISC",	"+",	"1x1"	},
	{ "N S VOID",		"ASCII NESW MISC",	"|",	"1x1"	},
	{ "E W VOID",		"ASCII NESW MISC",	"-",	"1x1"	},

	{ "N E CURVED",		"ASCII NESW CURVED",	"+",	"1x1"	},
	{ "S E CURVED",		"ASCII NESW CURVED",	"+",	"1x1"	},
	{ "S W CURVED",		"ASCII NESW CURVED",	"+",	"1x1"	},
	{ "N W CURVED",		"ASCII NESW CURVED",	"+",	"1x1"	},

	{ "DIR N",		"ASCII NESW DIRS",	"|",	"1x1"	},
	{ "DIR NE",		"ASCII NESW DIRS",	"/",	"1x1"	},
	{ "DIR E",		"ASCII NESW DIRS",	"-",	"1x1"	},
	{ "DIR SE",		"ASCII NESW DIRS",	"\\",	"1x1"	},
	{ "DIR S",		"ASCII NESW DIRS",	"|",	"1x1"	},
	{ "DIR SW",		"ASCII NESW DIRS",	"/",	"1x1"	},
	{ "DIR W",		"ASCII NESW DIRS",	"-",	"1x1"	},
	{ "DIR NW",		"ASCII NESW DIRS",	"\\",	"1x1"	},

	{ "NO EXITS",		"NESW LINE",		"1x1",	"1x1"	},
	{ "N",			"NESW LINE",		"1x1",	"1x1"	},
	{ "  E",		"NESW LINE",		"1x1",	"1x1"	},
	{ "N E",		"NESW LINE",		"1x1",	"1x1"	},
	{ "    S",		"NESW LINE",		"1x1",	"1x1"	},
	{ "N   S",		"NESW LINE",		"1x1",	"1x1"	},
	{ "  E S",		"NESW LINE",		"1x1",	"1x1"	},
	{ "N E S",		"NESW LINE",		"1x1",	"1x1"	},
	{ "      W",		"NESW LINE",		"1x1",	"1x1"	},
	{ "N     W",		"NESW LINE",		"1x1",	"1x1"	},
	{ "  E   W",		"NESW LINE",		"1x1",	"1x1"	},
	{ "N E   W",		"NESW LINE",		"1x1",	"1x1"	},
	{ "    S W",		"NESW LINE",		"1x1",	"1x1"	},
	{ "N   S W",		"NESW LINE",		"1x1",	"1x1"	},
	{ "  E S W",		"NESW LINE",		"1x1",	"1x1"	},
	{ "N E S W",		"NESW LINE",		"1x1",	"1x1"	},

	{ "USER",		"NESW MISC",		"1x1",	"1x1"	},
	{ "DIR UNKNOWN",	"NESW MISC",		"1x1",	"1x1"	},
	{ "N S VOID",		"NESW MISC",		"1x1",	"1x1"	},
	{ "E W VOID",		"NESW MISC",		"1x1",	"1x1"	},

	{ "N E CURVED",		"NESW CURVED",		"1x1",	"1x1"	},
	{ "S E CURVED",		"NESW CURVED",		"1x1",	"1x1"	},
	{ "S W CURVED",		"NESW CURVED",		"1x1",	"1x1"	},
	{ "N W CURVED",		"NESW CURVED",		"1x1",	"1x1"	},

	{ "DIR N",		"NESW DIRS",		"1x1",	"1x1"	},
	{ "DIR NE",		"NESW DIRS",		"1x1",	"1x1"	},
	{ "DIR E",		"NESW DIRS",		"1x1",	"1x1"	},
	{ "DIR SE",		"NESW DIRS",		"1x1",	"1x1"	},
	{ "DIR S",		"NESW DIRS",		"1x1",	"1x1"	},
	{ "DIR SW",		"NESW DIRS",		"1x1",	"1x1"	},
	{ "DIR W",		"NESW DIRS",		"1x1",	"1x1"	},
	{ "DIR NW",		"NESW DIRS",		"1x1",	"1x1"	},

	{ "NO EXITS",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N",			"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "  NW",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N NW",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "     W",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N    W",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "  NW W",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N NW W",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "       SW",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N      SW",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "  NW   SW",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N NW   SW",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "     W SW",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N    W SW",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "  NW W SW",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N NW W SW",		"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "          S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N         S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "  NW      S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N NW      S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "     W    S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N    W    S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "  NW W    S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N NW W    S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "       SW S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N      SW S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "  NW   SW S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N NW   SW S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "     W SW S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N    W SW S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "  NW W SW S",	"MUDFONT NWS",		"1x2",	"1x2"	},
	{ "N NW W SW S",	"MUDFONT NWS",		"1x2",	"1x2"	},

  	{ "NO EXITS",  		"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N",         		"MUDFONT NES",		"1x2",	"1x2"	},
	{ "  NE",       	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N NE",       	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "     W",     	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N    W",     	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "  NE W",     	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N NE W",     	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "       SE",  	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N      SE",  	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "  NE   SE",  	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N NE   SE",  	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "     W SE",  	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N    W SE",  	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "  NE W SE",  	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N NE W SE",  	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "          S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N         S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "  NE      S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N NE      S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "     W    S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N    W    S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "  NE W    S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N NE W    S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "       SE S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N      SE S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "  NE   SE S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N NE   SE S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "     W SE S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N    W SE S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "  NE W SE S",	"MUDFONT NES",		"1x2",	"1x2"	},
	{ "N NE W SE S",	"MUDFONT NES",		"1x2",	"1x2"	},

	{ "NO EXITS",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N",  		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "  NW",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N NW",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "     W",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N    W",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "  NW W",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N NW W",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "       SW",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N      SW",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "  NW   SW",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N NW   SW",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "     W SW",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N    W SW",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "  NW W SW",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N NW W SW",		"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "          S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N         S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "  NW      S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N NW      S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "     W    S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N    W    S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "  NW W    S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N NW W    S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "       SW S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N      SW S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "  NW   SW S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N NW   SW S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "     W SW S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N    W SW S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "  NW W SW S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},
	{ "N NW W SW S",	"MUDFONT VOID NWS",	"1x2",	"1x2"	},

	{ "NO EXITS",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N",			"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "  NE",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N NE",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "     W",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N    W",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "  NE W",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N NE W",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "       SE",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N      SE",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "  NE   SE",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N NE   SE",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "     W SE",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N    W SE",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "  NE W SE",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N NE W SE",		"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "          S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N         S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "  NE      S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N NE      S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "     W    S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N    W    S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "  NE W    S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N NE W    S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "       SE S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N      SE S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "  NE   SE S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N NE   SE S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "     W SE S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N    W SE S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "  NE W SE S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},
	{ "N NE W SE S",	"MUDFONT VOID NES",	"1x2",	"1x2"	},

	{ "N E",		"MUDFONT CURVED",	"1x2",	"1x2"	},
	{ "S E",		"MUDFONT CURVED",	"1x2",	"1x2"	},
	{ "S W",		"MUDFONT CURVED",	"1x2",	"1x2"	},
	{ "N W",		"MUDFONT CURVED",	"1x2",	"1x2"	},

        { "NO DIAGONAL",        "UNICODE GRAPHICS",     "2x5",  "2x5"   },
	{ "SE",                 "UNICODE GRAPHICS",     "⸌",    "2x5"   },
	{ "NE",                 "UNICODE GRAPHICS",     "⸝",    "2x5"   }, 
	{ "SE NE",              "UNICODE GRAPHICS",     ">",    "2x5"   }, 
	{ "SW",                 "UNICODE GRAPHICS",     "⸍",    "2x5"   }, 
	{ "SE SW",              "UNICODE GRAPHICS",     "⸌⸍",   "2x5"   }, 
	{ "NE SW",              "UNICODE GRAPHICS",     "／",   "2x5"   }, 
	{ "SE NE SW",           "UNICODE GRAPHICS",     ">⸍",   "2x5"   }, 
	{ "NW",                 "UNICODE GRAPHICS",     "⸜",    "2x5"   }, 
	{ "SE NW",              "UNICODE GRAPHICS",     "＼",   "2x5"   }, 
	{ "NE NW",              "UNICODE GRAPHICS",     "⸝⸜",   "2x5"   }, 
	{ "SE NE NW",           "UNICODE GRAPHICS",     ">⸜",   "2x5"   }, 
	{ "SW NW",              "UNICODE GRAPHICS",     "<",    "2x5"   },
	{ "SE SW NW",           "UNICODE GRAPHICS",     "⸌<",   "2x5"   }, 
	{ "NE SW NW",           "UNICODE GRAPHICS",     "⸝<",   "2x5"   }, 
	{ "SE NE SW NW",        "UNICODE GRAPHICS",     "><",   "2x5"   }, 
	{ "D",                  "UNICODE GRAPHICS",     "-",    "2x5"   }, 
	{ "N",                  "UNICODE GRAPHICS",     "↑",    "2x5"   }, 
	{ "S",                  "UNICODE GRAPHICS",     "↓",    "2x5"   }, 
	{ "N S",                "UNICODE GRAPHICS",     "│",    "2x5"   }, 
	{ "U",                  "UNICODE GRAPHICS",     "+",    "2x5"   }, 
	{ "E",                  "UNICODE GRAPHICS",     "🠆",    "2x5"   }, 
	{ "W",                  "UNICODE GRAPHICS",     "🠄",    "2x5"   }, 
	{ "E W",                "UNICODE GRAPHICS",     "─",    "2x5"   }, 
	{ "ROOM L",             "UNICODE GRAPHICS",     "[",    "2x5"   }, 
	{ "ROOM L CURVED",      "UNICODE GRAPHICS",     "(",    "2x5"   }, 
	{ "ROOM R",             "UNICODE GRAPHICS",     "]",    "2x5"   }, 
	{ "ROOM R CURVED",      "UNICODE GRAPHICS",     ")",    "2x5"   },

	{ NULL,			NULL,			NULL,	NULL	}
};

struct map_legend_group_type map_legend_group_table[] =
{
	{ "ALL",                        "",                     1, 1,   1, 2,   0, 0,   "" },

	{ "ASCII NESW",			"ASCII NESW",		1, 1,	1, 1,	0, 0,	"{x} {o} {o} {+} {o} {|} {+} {+} {o} {+} {-} {+} {+} {+} {+} {+} {x} {+} {|} {-} {+} {+} {+} {+} {|} {/} {-} {\\\\} {|} {/} {-} {\\\\}" },
	{ "ASCII NESW LINE",		"ASCII NESW LINE",	1, 1,	1, 1,	0, 0,	"{x} {o} {o} {+} {o} {|} {+} {+} {o} {+} {-} {+} {+} {+} {+} {+}" },
	{ "ASCII NESW MISC",		"ASCII NESW MISC",	1, 1,	1, 1,	0, 0,	"{x} {+} {|} {-}" },
	{ "ASCII NESW CURVED",		"ASCII NESW CURVED",	1, 1,	1, 1,	0, 0,	"{+} {+} {+} {+}" },
	{ "ASCII NESW DIRS",		"ASCII NESW DIRS",	1, 1,	1, 1,	0, 0,	"{|} {/} {-} {\\\\} {|} {/} {-} {\\\\}" },

	{ "NESW",			"NESW",			1, 1,	1, 1,	0, 0,	"{\\u2B51} {\\u2579} {\\u257A} {\\u2517} {\\u257B} {\\u2503} {\\u250F} {\\u2523} {\\u2578} {\\u251B} {\\u2501} {\\u253B} {\\u2513} {\\u252B} {\\u2533} {\\u254B} {\\u2B51} {\\u2012} {\\u2507} {\\u254D} {\\u2570} {\\u256D} {\\u256E} {\\u256F} {\\u2191} {\\u2B08} {\\u2B95} {\\u2b0A} {\\u2193} {\\u2B0B} {\\u2B05} {\\u2B09}" },
	{ "NESW TUBE",			"NESW",			1, 1,	1, 1,	0, 0,	"{\\u25AB}{\\U01F791}{\\U01F791}{\\u255A}{\\U01F791}{\\u2551}{\\u2554} {\\u2560}{\\U01F791}{\\u255D} {\\u2550} {\\u2569} {\\u2557} {\\u2563} {\\u2566} {\\u256C} {\\u25CF} {}        {\\u2502} {\\u2500} {\\u2570} {\\u256D} {\\u256E} {\\u256F} {\\u2191} {\\u2B08} {\\u2B95} {\\u2b0A} {\\u2193} {\\u2B0B} {\\u2B05} {\\u2B09}" },
	{ "NESW LINE",			"NESW LINE",		1, 1,	1, 1,	0, 0,	"{\\u25AA} {\\u2579} {\\u257A} {\\u2517} {\\u257B} {\\u2503} {\\u250F} {\\u2523} {\\u2578} {\\u251B} {\\u2501} {\\u253B} {\\u2513} {\\u252B} {\\u2533} {\\u254B}" },
	{ "NESW MISC",			"NESW MISC",		1, 1,	1, 1,	0, 0,	"{\\u2B51} {\\u2012} {\\u2507} {\\u254D}" },
	{ "NESW CURVED",		"NESW CURVED",		1, 1,	1, 1,	0, 0,	"{\\u2570} {\\u256D} {\\u256E} {\\u256F}" },
	{ "NESW DIRS",			"NESW DIRS",		1, 1,	1, 1,	0, 0,	"{\\u2191} {\\u2B08} {\\u2B95} {\\u2b0A} {\\u2193} {\\u2B0B} {\\u2B05} {\\u2B09}" },
//	{ "NESW DIRS",			"NESW DIRS",		1, 1,	1, 1,	0, 0,	"{\\U01F805}{\\u2B08}{\\U01F806}{\\u2b0A}{\\U01F807}{\\u2B0B}{\\U01F804}{\\u2B09}" }, poor cross-platform support.

	{ "MUDFONT",			"MUDFONT",		1, 2,	1, 2,	0, 0,	"" },
	{ "MUDFONT PRIVATE",		"MUDFONT",		1, 2,	1, 2,	0, 0,	"{\\uE000} {\\uE001} {\\uE002} {\\uE003} {\\uE004} {\\uE005} {\\uE006} {\\uE007} {\\uE008} {\\uE009} {\\uE00A} {\\uE00B} {\\uE00C} {\\uE00D} {\\uE00E} {\\uE00F} {\\uE010} {\\uE011} {\\uE012} {\\uE013} {\\uE014} {\\uE015} {\\uE016} {\\uE017} {\\uE018} {\\uE019} {\\uE01A} {\\uE01B} {\\uE01C} {\\uE01D} {\\uE01E} {\\uE01F} {\\uE020} {\\uE021} {\\uE022} {\\uE023} {\\uE024} {\\uE025} {\\uE026} {\\uE027} {\\uE028} {\\uE029} {\\uE02A} {\\uE02B} {\\uE02C} {\\uE02D} {\\uE02E} {\\uE02F} {\\uE030} {\\uE031} {\\uE032} {\\uE033} {\\uE034} {\\uE035} {\\uE036} {\\uE037} {\\uE038} {\\uE039} {\\uE03A} {\\uE03B} {\\uE03C} {\\uE03D} {\\uE03E} {\\uE03F} {\\uE040} {\\uE041} {\\uE042} {\\uE043} {\\uE044} {\\uE045} {\\uE046} {\\uE047} {\\uE048} {\\uE049} {\\uE04A} {\\uE04B} {\\uE04C} {\\uE04D} {\\uE04E} {\\uE04F} {\\uE050} {\\uE051} {\\uE052} {\\uE053} {\\uE054} {\\uE055} {\\uE056} {\\uE057} {\\uE058} {\\uE059} {\\uE05A} {\\uE05B} {\\uE05C} {\\uE05D} {\\uE05E} {\\uE05F} {\\uE060} {\\uE061} {\\uE062} {\\uE063} {\\uE064} {\\uE065} {\\uE066} {\\uE067} {\\uE068} {\\uE069} {\\uE06A} {\\uE06B} {\\uE06C} {\\uE06D} {\\uE06E} {\\uE06F} {\\uE070} {\\uE071} {\\uE072} {\\uE073} {\\uE074} {\\uE075} {\\uE076} {\\uE077} {\\uE078} {\\uE079} {\\uE07A} {\\uE07B} {\\uE07C} {\\uE07D} {\\uE07E} {\\uE07F} {\\uE080} {\\uE081} {\\uE082} {\\uE083} {\\uE084} {\\uE085} {\\uE086} {\\uE087}" },
	{ "MUDFONT BRAILLE TUBE",	"MUDFONT",		1, 2,	1, 2,	0, 0,	"{\\u28CF} {\\u28C7} {\\u28CE} {\\u28C6} {\\u28C9} {\\u28C1} {\\u28C8} {\\u28C0} {\\u288F} {\\u2887} {\\u288E} {\\u2886} {\\u2889} {\\u2881} {\\u2888} {\\u2880} {\\u284F} {\\u2847} {\\u284E} {\\u2846} {\\u2849} {\\u2841} {\\u2848} {\\u2880} {\\u280F} {\\u2807} {\\u280E} {\\u2806} {\\u2809} {\\u2801} {\\u2808} {\\u2800} {\\u28F9} {\\u28F8} {\\u28F1} {\\u28F0} {\\u28C9} {\\u28C8} {\\u28C2} {\\u28C0} {\\u2879} {\\u2878} {\\u2871} {\\u2870} {\\u2849} {\\u2848} {\\u2842} {\\u2840} {\\u28B9} {\\u28B8} {\\u28B1} {\\u28B0} {\\u2889} {\\u2888} {\\u2882} {\\u2880} {\\u2839} {\\u2838} {\\u2831} {\\u2830} {\\u2809} {\\u2808} {\\u2802} {\\u2800} {\\u2830} {\\u2838} {\\u2831} {\\u2839} {\\u2836} {\\u283E} {\\u2837} {\\u283F} {\\u2870} {\\u2878} {\\u2871} {\\u2879} {\\u2876} {\\u287E} {\\u2877} {\\u287F} {\\u28B0} {\\u28B8} {\\u28B1} {\\u28B9} {\\u28B6} {\\u28BE} {\\u28B7} {\\u28BF} {\\u28F0} {\\u28F8} {\\u28F1} {\\u28F9} {\\u28F6} {\\u28FE} {\\u28F7} {\\u28FF} {\\u2806} {\\u2807} {\\u280E} {\\u280F} {\\u2836} {\\u2837} {\\u283E} {\\u283F} {\\u2886} {\\u2887} {\\u288E} {\\u288F} {\\u28B6} {\\u28B7} {\\u28BE} {\\u28BF} {\\u2846} {\\u2847} {\\u284E} {\\u284F} {\\u2876} {\\u2877} {\\u287E} {\\u287F} {\\u28C6} {\\u28C7} {\\u28CE} {\\u28CF} {\\u28F6} {\\u28F7} {\\u28FE} {\\u28FF} {\\u2818} {\\u28A0} {\\u2844} {\\u2803} {} {} {} {}" },
	{ "MUDFONT BRAILLE LINE",	"MUDFONT",		1, 2,	1, 2,	0, 0,	"{\\u2830} {\\u2838} {\\u2831} {\\u2839} {\\u2836} {\\u283E} {\\u2837} {\\u283F} {\\u2870} {\\u2878} {\\u2871} {\\u2879} {\\u2876} {\\u287E} {\\u2877} {\\u287F} {\\u28B0} {\\u28B8} {\\u28B1} {\\u28B9} {\\u28B6} {\\u28BE} {\\u28B7} {\\u28BF} {\\u28F0} {\\u28F8} {\\u28F1} {\\u28F9} {\\u28F6} {\\u28FE} {\\u28F7} {\\u28FF} {\\u2806} {\\u2807} {\\u280E} {\\u280F} {\\u2836} {\\u2837} {\\u283E} {\\u283F} {\\u2886} {\\u2887} {\\u288E} {\\u288F} {\\u28B6} {\\u28B7} {\\u28BE} {\\u28BF} {\\u2846} {\\u2847} {\\u284E} {\\u284F} {\\u2876} {\\u2877} {\\u287E} {\\u287F} {\\u28C6} {\\u28C7} {\\u28CE} {\\u28CF} {\\u28F6} {\\u28F7} {\\u28FE} {\\u28FF} {\\u2800} {\\u2808} {\\u2801} {\\u2809} {\\u2806} {\\u280E} {\\u2807} {\\u280F} {\\u2840} {\\u2848} {\\u2841} {\\u2849} {\\u2846} {\\u284E} {\\u2847} {\\u284F} {\\u2880} {\\u2888} {\\u2881} {\\u2889} {\\u2886} {\\u288E} {\\u2887} {\\u288F} {\\u28C0} {\\u28C8} {\\u28C1} {\\u28C9} {\\u28C6} {\\u28CE} {\\u28C7} {\\u28CF} {\\u2800} {\\u2801} {\\u2808} {\\u2809} {\\u2830} {\\u2831} {\\u2838} {\\u2839} {\\u2880} {\\u2881} {\\u2888} {\\u2889} {\\u28B0} {\\u28B1} {\\u28B8} {\\u28B9} {\\u2840} {\\u2841} {\\u2848} {\\u2849} {\\u2870} {\\u2871} {\\u2878} {\\u2879} {\\u28C0} {\\u28C1} {\\u28C8} {\\u28C9} {\\u28F0} {\\u28F1} {\\u28F8} {\\u28F9} {\\u2818} {\\u28A0} {\\u2844} {\\u2803} {} {} {} {}" },

	{ "MUDFONT NWS",		"MUDFONT NWS",		1, 2,	1, 2,	0, 0,	"" },
  	{ "MUDFONT NES",		"MUDFONT NES",		1, 2,	1, 2,	0, 0,	"" },

	{ "MUDFONT VOID NWS",		"MUDFONT VOID NWS",	1, 2,	1, 2,	0, 0,	"" },
	{ "MUDFONT VOID NES",		"MUDFONT VOID NES",	1, 2,	1, 2,	0, 0,	"" },

	{ "MUDFONT CURVED",		"MUDFONT CURVED",	1, 2,	1, 2,	0, 0,	"" },
	{ "MUDFONT CURVED BRAILLE",	"MUDFONT CURVED",	1, 2,	1, 2,	0, 0,	"{\\u2818} {\\u28A0} {\\u2844} {\\u2803} {} {} {} {}" },

	{ "UNICODE GRAPHICS",           "UNICODE GRAPHICS",     2, 5,   2, 5,   0, 0,   "{ } {\\u2E0C} {\\u2E1D} {>} {\\u2E0D} {\\u2E0C\\u2E0D} {\\uFF0F} {>\\u2E0D} {\\u2E1C} {\\uFF3C} {\\u2E1D\\u2E1C} {>\\u2E1C} {<} {\\u2E0C<} {\\u2E1D<} {><} {-} {\\u2191} {\\u2193} {\\u2502} {+} {\\U01F806} {\\U01F804} {\\u2500} {[} {(} {]} {)}" },
//	{ "UNICODE GRAPHICS",           "UNICODE GRAPHICS",     2, 5,   2, 5,   0, 0,   "{ } {\\u2E0C} {\\u2E1D} {>} {\\u2E0D} {\\u2E0C\\u2E0D} {\\uFF0F} {>\\u2E0D} {\\u2E1C} {\\uFF3C} {\\u2E1D\\u2E1C} {>\\u2E1C} {<} {\\u2E0C<} {\\u2E1D<} {><} {-} {\\u2191} {\\u2193} {\\u2502} {+} {\\u2192} {\\u2190} {\\u2500} {[} {(} {]} {)}" },

	{ NULL,				NULL,			0, 0,	0, 0,	0, 0,	"" }
};

struct stamp_type huge_stamp_table[] =
{
	{ " ",  23, "   \n   \n   \n   \n   \n   " },
	{ "!",  23, "██╗\n██║\n██║\n╚═╝\n██╗\n╚═╝" },
	{ "\"", 41, "██╗██╗\n2██║██║\n╚═╝╚═╝\n   \n   \n   " },
	{ "#",  59, " ██╗ ██╗ \n████████╗\n╚██╔═██╔╝\n████████╗\n╚██╔═██╔╝\n ╚═╝ ╚═╝ " },
	{ "&",  59, "  ████╗  \n ██╔═██╗ \n ╚████╔╝ \n██╔══██═╗\n╚█████╔█║\n ╚════╝╚╝" },
	{ "%",  47, "██╗ ██╗\n╚═╝██╔╝\n  ██╔╝ \n ██╔╝  \n██╔╝██╗\n╚═╝ ╚═╝" },
	{ "'",  23, "╗██╗\n██║\n╚═╝\n   \n   \n   " },
	{ "(",  29, " ██╗\n██╔╝\n██║ \n██║ \n╚██╗\n ╚═╝" },
	{ ")",  29, "██╗ \n╚██╗\n ██║\n ██║\n██╔╝\n╚═╝ " },
	{ "*",  47, "▄  █  ▄\n █▄█▄█ \n  ▐█▌  \n █▀█▀█ \n▀  █  ▀\n       " },
	{ "+",  59, "   ██╗   \n   ██║   \n████████╗\n╚══██╔══╝\n   ██║   \n   ╚═╝   " },
// ,
// -
// .
// /

	{ "0",  53, " █████╗ \n██╔══██╗\n██║  ██║\n██║  ██║\n╚█████╔╝\n ╚════╝ " },
	{ "1",  53, "  ▄██╗  \n ████║  \n ╚═██║  \n   ██║  \n ██████╗\n ╚═════╝" },
	{ "2",  53, "██████╗ \n╚════██╗\n █████╔╝\n██╔═══╝ \n███████╗\n╚══════╝" },
	{ "3",  53, "██████╗ \n╚════██╗\n █████╔╝\n ╚═══██╗\n██████╔╝\n╚═════╝ " },
	{ "4",  53, "██╗  ██╗\n██║  ██║\n███████║\n╚════██║\n     ██║\n     ╚═╝" },
	{ "5",  53, "███████╗\n██╔════╝\n███████╗\n╚════██║\n███████║\n╚══════╝" },
	{ "6",  53, " █████╗ \n██╔═══╝ \n██████╗ \n██╔══██╗\n╚█████╔╝\n ╚════╝ " },
	{ "7",  53, "███████╗\n╚════██║\n    ██╔╝\n   ██╔╝ \n   ██║  \n   ╚═╝  " },
	{ "8",  53, " █████╗ \n██╔══██╗\n╚█████╔╝\n██╔══██╗\n╚█████╔╝\n ╚════╝ " },
	{ "9",  53, " █████╗ \n██╔══██╗\n╚██████║\n ╚═══██║\n █████╔╝\n ╚════╝ " },

	{ ":",  53, "        \n   ██╗  \n   ╚═╝  \n        \n   ██╗  \n   ╚═╝  " },
// ;
// <
// =
// >
// ?


	{ "@",  59, " ██████╗ \n██╔═══██╗\n██║██╗██║\n██║██║██║\n╚█║████╔╝\n ╚╝╚═══╝ " },
	{ "A",  53, " █████╗ \n██╔══██╗\n███████║\n██╔══██║\n██║  ██║\n╚═╝  ╚═╝" },
	{ "B",  53, "██████╗ \n██╔══██╗\n██████╔╝\n██╔══██╗\n██████╔╝\n╚═════╝ " },
	{ "C",  53, " ██████╗\n██╔════╝\n██║     \n██║     \n╚██████╗\n ╚═════╝" },
	{ "D",  53, "██████╗ \n██╔══██╗\n██║  ██║\n██║  ██║\n██████╔╝\n╚═════╝ " },
	{ "E",  53, "███████╗\n██╔════╝\n█████╗  \n██╔══╝  \n███████╗\n╚══════╝" },
	{ "F",  53, "███████╗\n██╔════╝\n█████╗  \n██╔══╝  \n██║     \n╚═╝     " },
	{ "G",  59, " ██████╗ \n██╔════╝ \n██║  ███╗\n██║   ██║\n╚██████╔╝\n ╚═════╝ " },
	{ "H",  53, "██╗  ██╗\n██║  ██║\n███████║\n██╔══██║\n██║  ██║\n╚═╝  ╚═╝" },
	{ "I",  47, "██████╗\n╚═██╔═╝\n  ██║  \n  ██║  \n██████╗\n╚═════╝" },
	{ "J",  53, "     ██╗\n     ██║\n     ██║\n██   ██║\n╚█████╔╝\n ╚════╝ " },
	{ "K",  53, "██╗  ██╗\n██║ ██╔╝\n█████╔╝ \n██╔═██╗ \n██║  ██╗\n╚═╝  ╚═╝" },
	{ "L",  53, "██╗     \n██║     \n██║     \n██║     \n███████╗\n╚══════╝" },
	{ "M",  71, "███╗  ███╗\n████╗████║\n██╔███╔██║\n██║╚█╔╝██║\n██║ ╚╝ ██║\n╚═╝    ╚═╝" },
	{ "N",  65, "███╗   ██╗\n████╗  ██║\n██╔██╗ ██║\n██║╚██╗██║\n██║ ╚████║\n╚═╝  ╚═══╝" },
	{ "O",  59, " ██████╗ \n██╔═══██╗\n██║   ██║\n██║   ██║\n╚██████╔╝\n ╚═════╝ " },
	{ "P",  53, "██████╗ \n██╔══██╗\n██████╔╝\n██╔═══╝ \n██║     \n╚═╝     " },
	{ "Q",  59, " ██████╗ \n██╔═══██╗\n██║   ██║\n██║▄▄ ██║\n╚██████╔╝\n ╚══▀▀═╝ " },
	{ "R",  53, "██████╗ \n██╔══██╗\n██████╔╝\n██╔══██╗\n██║  ██║\n╚═╝  ╚═╝" },
	{ "S",  53, "███████╗\n██╔════╝\n███████╗\n╚════██║\n███████║\n╚══════╝" },
	{ "T",  59, "████████╗\n╚══██╔══╝\n   ██║   \n   ██║   \n   ██║   \n   ╚═╝   " },
	{ "U",  59, "██╗   ██╗\n██║   ██║\n██║   ██║\n██║   ██║\n╚██████╔╝\n ╚═════╝ " },
	{ "V",  59, "██╗   ██╗\n██║   ██║\n██║   ██║\n╚██╗ ██╔╝\n ╚████╔╝ \n  ╚═══╝  " },
	{ "W",  65, "██╗    ██╗\n██║    ██║\n██║ █╗ ██║\n██║███╗██║\n╚███╔███╔╝\n ╚══╝╚══╝ " },
	{ "X",  53, "██╗  ██╗\n╚██╗██╔╝\n ╚███╔╝ \n ██╔██╗ \n██╔╝ ██╗\n╚═╝  ╚═╝" },
	{ "Y",  59, "██╗   ██╗\n╚██╗ ██╔╝\n ╚████╔╝ \n  ╚██╔╝  \n   ██║   \n   ╚═╝   " },
	{ "Z",  53, "███████╗\n╚══███╔╝\n  ███╔╝ \n ███╔╝  \n███████╗\n╚══════╝" },

// [

// ]
	{ "^",  41, " ███╗ \n██╔██╗\n╚═╝╚═╝\n      \n      \n" },
	{ "_",  53, "        \n        \n        \n        \n███████╗\n╚══════╝" },
// `

	{ "i",  23, "██╗\n╚═╝\n██╗\n██║\n██║\n╚═╝" },
	{ "n",  47, "       \n       \n██▟███╗\n██║ ██║\n██║ ██║\n╚═╝ ╚═╝" },

// {
// }
// ~
// DEL

	{ NULL, 0, NULL }
};

