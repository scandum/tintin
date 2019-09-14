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

struct command_type command_table[] =
{
	{    "action",            do_action,            TOKEN_TYPE_COMMAND },
	{    "advertise",         do_advertise,         TOKEN_TYPE_COMMAND },
	{    "alias",             do_alias,             TOKEN_TYPE_COMMAND },
	{    "all",               do_all,               TOKEN_TYPE_COMMAND },
	{    "bell",              do_bell,              TOKEN_TYPE_COMMAND },
	{    "break",             do_nop,               TOKEN_TYPE_BREAK   },
	{    "buffer",            do_buffer,            TOKEN_TYPE_COMMAND },
	{    "case",              do_nop,               TOKEN_TYPE_CASE    },
	{    "chat",              do_chat,              TOKEN_TYPE_COMMAND },
	{    "class",             do_class,             TOKEN_TYPE_COMMAND },
	{    "commands",          do_commands,          TOKEN_TYPE_COMMAND },
	{    "config",            do_configure,         TOKEN_TYPE_COMMAND },
	{    "continue",          do_nop,               TOKEN_TYPE_CONTINUE},
	{    "cr",                do_cr,                TOKEN_TYPE_COMMAND },
	{    "cursor",            do_cursor,            TOKEN_TYPE_COMMAND },
	{    "debug",             do_debug,             TOKEN_TYPE_COMMAND },
	{    "default",           do_nop,               TOKEN_TYPE_DEFAULT },
	{    "delay",             do_delay,             TOKEN_TYPE_COMMAND },
	{    "echo",              do_echo,              TOKEN_TYPE_COMMAND },
	{    "else",              do_nop,               TOKEN_TYPE_ELSE    },
	{    "elseif",            do_nop,               TOKEN_TYPE_ELSEIF  },
	{    "end",               do_end,               TOKEN_TYPE_COMMAND },
	{    "event",             do_event,             TOKEN_TYPE_COMMAND },
	{    "forall",            do_forall,            TOKEN_TYPE_COMMAND },
	{    "foreach",           do_nop,               TOKEN_TYPE_FOREACH },
	{    "format",            do_format,            TOKEN_TYPE_COMMAND },
	{    "function",          do_function,          TOKEN_TYPE_COMMAND },
	{    "gag",               do_gag,               TOKEN_TYPE_COMMAND },
	{    "grep",              do_grep,              TOKEN_TYPE_COMMAND },
	{    "help",              do_help,              TOKEN_TYPE_COMMAND },
	{    "highlight",         do_highlight,         TOKEN_TYPE_COMMAND },
	{    "history",           do_history,           TOKEN_TYPE_COMMAND },
	{    "if",                do_nop,               TOKEN_TYPE_IF      },
	{    "ignore",            do_ignore,            TOKEN_TYPE_COMMAND },
	{    "info",              do_info,              TOKEN_TYPE_COMMAND },
	{    "killall",           do_kill,              TOKEN_TYPE_COMMAND },
	{    "line",              do_line,              TOKEN_TYPE_COMMAND },
	{    "list",              do_list,              TOKEN_TYPE_COMMAND },
	{    "local",             do_local,             TOKEN_TYPE_COMMAND },
	{    "log",               do_log,               TOKEN_TYPE_COMMAND },
	{    "loop",              do_nop,               TOKEN_TYPE_LOOP    },
	{    "macro",             do_macro,             TOKEN_TYPE_COMMAND },
	{    "map",               do_map,               TOKEN_TYPE_COMMAND },
	{    "math",              do_math,              TOKEN_TYPE_COMMAND },
	{    "message",           do_message,           TOKEN_TYPE_COMMAND },
	{    "nop",               do_nop,               TOKEN_TYPE_COMMAND },
	{    "parse",             do_nop,               TOKEN_TYPE_PARSE   },
	{    "path",              do_path,              TOKEN_TYPE_COMMAND },
	{    "pathdir",           do_pathdir,           TOKEN_TYPE_COMMAND },
	{    "port",              do_port,              TOKEN_TYPE_COMMAND },
	{    "prompt",            do_prompt,            TOKEN_TYPE_COMMAND },
	{    "read",              do_read,              TOKEN_TYPE_COMMAND },
	{    "regexp",            do_regexp,            TOKEN_TYPE_REGEX   },
	{    "replace",           do_replace,           TOKEN_TYPE_COMMAND },
	{    "return",            do_nop,               TOKEN_TYPE_RETURN  },
	{    "run",               do_run,               TOKEN_TYPE_COMMAND },
	{    "scan",              do_scan,              TOKEN_TYPE_COMMAND },
	{    "screen",            do_screen,            TOKEN_TYPE_COMMAND },
	{    "script",            do_script,            TOKEN_TYPE_COMMAND },
	{    "send",              do_send,              TOKEN_TYPE_COMMAND },
	{    "session",           do_session,           TOKEN_TYPE_COMMAND },
	{    "showme",            do_showme,            TOKEN_TYPE_COMMAND },
	{    "snoop",             do_snoop,             TOKEN_TYPE_COMMAND },
	{    "split",             do_split,             TOKEN_TYPE_COMMAND },
	{    "ssl",               do_ssl,               TOKEN_TYPE_COMMAND },
	{    "substitute",        do_substitute,        TOKEN_TYPE_COMMAND },
	{    "switch",            do_nop,               TOKEN_TYPE_SWITCH  },
	{    "system",            do_system,            TOKEN_TYPE_COMMAND },
	{    "tab",               do_tab,               TOKEN_TYPE_COMMAND },
	{    "test",              do_test,              TOKEN_TYPE_COMMAND },
	{    "textin",            do_textin,            TOKEN_TYPE_COMMAND },
	{    "ticker",            do_tick,              TOKEN_TYPE_COMMAND },
	{    "unaction",          do_unaction,          TOKEN_TYPE_COMMAND },
	{    "unalias",           do_unalias,           TOKEN_TYPE_COMMAND },
	{    "undelay",           do_undelay,           TOKEN_TYPE_COMMAND },
	{    "unevent",           do_unevent,           TOKEN_TYPE_COMMAND },
	{    "unfunction",        do_unfunction,        TOKEN_TYPE_COMMAND },
	{    "ungag",             do_ungag,             TOKEN_TYPE_COMMAND },
	{    "unhighlight",       do_unhighlight,       TOKEN_TYPE_COMMAND },
	{    "unmacro",           do_unmacro,           TOKEN_TYPE_COMMAND },
	{    "unpathdir",         do_unpathdir,         TOKEN_TYPE_COMMAND },
	{    "unprompt",          do_unprompt,          TOKEN_TYPE_COMMAND },
	{    "unsplit",           do_unsplit,           TOKEN_TYPE_COMMAND },
	{    "unsubstitute",      do_unsubstitute,      TOKEN_TYPE_COMMAND },
	{    "untab",             do_untab,             TOKEN_TYPE_COMMAND },
	{    "unticker",          do_untick,            TOKEN_TYPE_COMMAND },
	{    "unvariable",        do_unvariable,        TOKEN_TYPE_COMMAND },
	{    "variable",          do_variable,          TOKEN_TYPE_COMMAND },
	{    "while",             do_nop,               TOKEN_TYPE_WHILE   },
	{    "write",             do_write,             TOKEN_TYPE_COMMAND },
	{    "zap",               do_zap,               TOKEN_TYPE_COMMAND },
	{    "",                  NULL,                 TOKEN_TYPE_COMMAND }
};


struct list_type list_table[LIST_MAX] =
{
	{    "ACTION",            "ACTIONS",            SORT_PRIORITY,    3,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_REGEX },
	{    "ALIAS",             "ALIASES",            SORT_PRIORITY,    3,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_REGEX },
	{    "CLASS",             "CLASSES",            SORT_PRIORITY,    2,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_INHERIT                                 },
	{    "COMMAND",           "COMMANDS",           SORT_APPEND,      1,  LIST_FLAG_MESSAGE                                                                  },
	{    "CONFIG",            "CONFIGURATIONS",     SORT_ALPHA,       2,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_INHERIT                 },
	{    "DELAY",             "DELAYS",             SORT_DELAY,       3,  LIST_FLAG_MESSAGE|LIST_FLAG_READ                                                   },
	{    "EVENT",             "EVENTS",             SORT_ALPHA,       2,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "FUNCTION",          "FUNCTIONS",          SORT_ALPHA,       2,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "GAG",               "GAGS",               SORT_ALPHA,       1,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "HIGHLIGHT",         "HIGHLIGHTS",         SORT_PRIORITY,    3,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_REGEX },
	{    "HISTORY",           "HISTORIES",          SORT_APPEND,      1,  LIST_FLAG_MESSAGE                                                                  },
	{    "MACRO",             "MACROS",             SORT_ALPHA,       2,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "PATH",              "PATHS",              SORT_APPEND,      2,  LIST_FLAG_MESSAGE                                                                  },
	{    "PATHDIR",           "PATHDIRS",           SORT_ALPHA,       3,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "PROMPT",            "PROMPTS",            SORT_PRIORITY,    4,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_REGEX },
	{    "SUBSTITUTE",        "SUBSTITUTES",        SORT_PRIORITY,    3,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_REGEX },
	{    "TAB",               "TABS",               SORT_ALPHA,       1,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "TICKER",            "TICKERS",            SORT_ALPHA,       3,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "VARIABLE",          "VARIABLES",          SORT_ALPHA,       2,  LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_NEST }
};

struct substitution_type substitution_table[] =
{
	{    "ARGUMENTS",            1  },
	{    "VARIABLES",            2  },
	{    "FUNCTIONS",            4  },
	{    "COLORS",               8  },
	{    "ESCAPES",             16  },
//	{    "COMMANDS",            32  },
	{    "SECURE",              64  },
	{    "EOL",                128  },
	{    "LNF",                256  },
//	{    "FIX",                512  },
        {    "COMPRESS",          1024  },
	{    "",                  0     }
};

struct config_type config_table[] =
{
	{
		"AUTO TAB",
		"",
		"Scroll back buffer lines used for tab completion",
		config_autotab
	},

	{
		"BUFFER SIZE",
		"",
		"The size of the scroll back buffer",
		config_buffersize
	},

	{
		"CHARSET",
		"",
		"The character set encoding used by TinTin++",
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
		"",
		"The color code encoding used by TinTin++",
		config_colormode
	},

	{
		"COLOR PATCH",
		"TinTin++ will properly color the start of each line",
		"TinTin++ will leave color handling to the server",
		config_colorpatch
	},

	{
		"COMMAND COLOR",
		"",
		"The color of echoed commands",
		config_commandcolor
	},

	{
		"COMMAND ECHO",
		"Your commands are echoed in split mode",
		"Your commands are not echoed in split mode",
		config_commandecho
	},

	{
		"CONNECT RETRY",
		"",
		"Seconds TinTin++ sessions try to connect on failure",
		config_connectretry
	},

	{
		"CONVERT META",
		"TinTin++ converts meta prefixed characters",
		"TinTin++ doesn't convert meta prefixed characters",
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
		"",
		"The size of the command history",
		config_historysize
	},

	{
		"INHERITANCE",
		"The startup session is inherited",
		"The startup session is not inherited",
		config_inheritance
	},

	{
		"LOG",
		"",
		"The data format of the log files",
		config_log
	},

	{
		"LOG LEVEL",
		"TinTin++ only logs low level mud data",
		"TinTin++ only logs high level mud data",
		config_loglevel
	},

	{
		"MCCP",
		"MCCP is enabled.",
		"MCCP is disabled.",
		config_mccp
	},

	{
		"MOUSE TRACKING",
		"Your terminal generates mouse events.",
		"Your terminal does not generate mouse events.",
		config_mousetracking
	},

	{
		"PACKET PATCH",
		"",
		"Seconds TinTin++ will try to patch broken packets",
		config_packetpatch
	},

	{
		"RANDOM SEED",
		"",
		"Seed value used for the random number engine",
		config_randomseed
	},

	{
		"REPEAT ENTER",
		"You send the last command on an enter",
		"You send a carriage return on an enter",
		config_repeatenter
	},

	{
		"REPEAT CHAR",
		"",
		"The character used for repeating commands",
		config_repeatchar
	},

	{
		"SCREEN READER",
		"You are using a screen reader",
		"You are not using a screen reader",
		config_screenreader
	},

	{
		"SCROLL LOCK",
		"You do not see mud output while scrolling",
		"You see mud output while scrolling",
		config_scrolllock
	},

	{
		"SPEEDWALK",
		"Your input is scanned for speedwalk directions",
		"Your input is not scanned for speedwalk directions",
		config_speedwalk
	},

	{
		"TINTIN CHAR",
		"",
		"The character used for TinTin++ commands",
		config_tintinchar
	},

	{
		"VERBATIM",
		"Your keyboard input isn't modified by TinTin++",
		"Your keyboard input is parsed by TinTin++",
		config_verbatim
	},

	{
		"VERBATIM CHAR",
		"",
		"The character used for unparsed text",
		config_verbatimchar
	},

	{
		"VERBOSE",
		"Messages while reading in a script file are echoed",
		"Messages while reading in a script file are gagged",
		config_verbose
	},

	{
		"WORDWRAP",
		"Mud output is word wrapped",
		"Mud output is line wrapped",
		config_wordwrap
	},


	{
		"",
		"",
		0,
		0
	}
};

struct color_type color_table[] =
{
	{    "azure",         "<abd>" },
	{    "ebony",         "<g04>" },
	{    "jade",          "<adb>" },
	{    "lime",          "<bda>" },
	{    "orange",        "<dba>" },
	{    "pink",          "<dab>" },
	{    "silver",        "<ccc>" },
	{    "tan",           "<cba>" },
	{    "violet",        "<bad>" },

	{    "light azure",   "<acf>" },
	{    "light ebony",   "<bbb>" },
	{    "light jade",    "<afc>" },
	{    "light lime",    "<cfa>" },
	{    "light orange",  "<fca>" },
	{    "light pink",    "<fac>" },
	{    "light silver",  "<eee>" },
	{    "light tan",     "<eda>" },
	{    "light violet",  "<caf>" },

	{    "reset",         "<088>" },
	{    "light",         "<188>" },
	{    "bold",          "<188>" },
	{    "faint",         "<288>" },
	{    "dim",           "<288>" },
	{    "dark",          "<288>" },
	{    "underscore",    "<488>" },
	{    "blink",         "<588>" },
	{    "reverse",       "<788>" },

	{    "no-underscore", "\e[24m"},
	{    "no-blink",      "\e[25m"},
	{    "no-reverse",    "\e[27m"},
		
	{    "black",         "<808>" },
	{    "red",           "<818>" },
	{    "green",         "<828>" },
	{    "yellow",        "<838>" },
	{    "blue",          "<848>" },
	{    "magenta",       "<858>" },
	{    "cyan",          "<868>" },
	{    "white",         "<878>" },

	{    "b black",       "<880>" },
	{    "b red",         "<881>" },
	{    "b green",       "<882>" },
	{    "b yellow",      "<883>" },
	{    "b blue",        "<884>" },
	{    "b magenta",     "<885>" },
	{    "b cyan",        "<886>" },
	{    "b white",       "<887>" },

	{    "b azure",       "<ABD>" },
	{    "b ebony",       "<G04>" },
	{    "b jade",        "<ADB>" },
	{    "b lime",        "<BDA>" },
	{    "b orange",      "<DBA>" },
	{    "b pink",        "<DAB>" },
	{    "b silver",      "<CCC>" },
	{    "b tan",         "<CBA>" },
	{    "b violet",      "<BAD>" },

	{    "",              "<888>" }
};

struct color_type map_color_table[] =
{
	{     "AVOID",            "<118>" },
	{     "BACKGROUND",       ""      },
	{     "EXITS",            "<278>" },
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
	{    "OPEN",              class_open             },
	{    "CLOSE",             class_close            },
	{    "LIST",              class_list             },
	{    "READ",              class_read             },
	{    "SIZE",              class_size             },
	{    "WRITE",             class_write            },
	{    "KILL",              class_kill             },
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
	{     "FORWARDALL",       chat_forwardall,     1, 0, "Forward all chat/mud messages to a buddy"       },
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
	{     "ADD",              array_add           },
	{     "CLEAR",            array_clear         },
	{     "CLR",              array_clear         },
	{     "CREATE",           array_create        },
	{     "DELETE",           array_delete        },
	{     "FIND",             array_find          },
	{     "FND",              array_find          },
	{     "GET",              array_get           },
	{     "INSERT",           array_insert        },
	{     "LENGTH",           array_size          },
	{     "SET",              array_set           },
	{     "SIMPLIFY",         array_simplify      },
	{     "SIZE",             array_size          },
	{     "SORT",             array_sort          },
	{     "SRT",              array_sort          },
	{     "TOKENIZE",         array_tokenize      },
	{     "",                 NULL                }
};

// 0 no map, 1 has map, 2 is inside map

struct map_type map_table[] =
{
	{     "AT",               map_at,              0,              1    },
	{     "COLOR",            map_color,           MAP_FLAG_VTMAP, 1    },
	{     "CREATE",           map_create,          MAP_FLAG_VTMAP, 0    },
	{     "DEBUG",            map_debug,           0,              2    },
	{     "DELETE",           map_delete,          MAP_FLAG_VTMAP, 1    },
	{     "DESTROY",          map_destroy,         MAP_FLAG_VTMAP, 1    },
	{     "DIG",              map_dig,             MAP_FLAG_VTMAP, 2    },
	{     "EXIT",             map_exit,            MAP_FLAG_VTMAP, 2    },
	{     "EXITFLAG",         map_exitflag,        MAP_FLAG_VTMAP, 2    },
	{     "EXPLORE",          map_explore,         MAP_FLAG_VTMAP, 2    },
	{     "FIND",             map_find,            MAP_FLAG_VTMAP, 2    },
	{     "FLAG",             map_flag,            MAP_FLAG_VTMAP, 1    },
	{     "GET",              map_get,             0,              2    },
	{     "GLOBAL",           map_global,          0,              1    },
	{     "GOTO",             map_goto,            MAP_FLAG_VTMAP, 1    },
	{     "INFO",             map_info,            0,              1    },
	{     "INSERT",           map_insert,          MAP_FLAG_VTMAP, 2    },
	{     "JUMP",             map_jump,            MAP_FLAG_VTMAP, 2    },
	{     "LEAVE",            map_leave,           MAP_FLAG_VTMAP, 2    },
	{     "LEGENDA",          map_legend,          MAP_FLAG_VTMAP, 1    },
	{     "LINK",             map_link,            MAP_FLAG_VTMAP, 2    },
	{     "LIST",             map_list,            0,              2    },
	{     "MAP",              map_map,             0,              2    },
	{     "MOVE",             map_move,            MAP_FLAG_VTMAP, 2    },
	{     "NAME",             map_name,            MAP_FLAG_VTMAP, 2    },
	{     "OFFSET",           map_offset,          MAP_FLAG_VTMAP, 1    },
	{     "READ",             map_read,            MAP_FLAG_VTMAP, 0    },
	{     "RESIZE",           map_resize,          0,              1    },
	{     "RETURN",           map_return,          MAP_FLAG_VTMAP, 1    },
	{     "ROOMFLAG",         map_roomflag,        MAP_FLAG_VTMAP, 2    },
	{     "RUN",              map_run,             MAP_FLAG_VTMAP, 2    },
	{     "SET",              map_set,             MAP_FLAG_VTMAP, 2    },
	{     "TRAVEL",           map_travel,          MAP_FLAG_VTMAP, 2    },
	{     "UNDO",             map_undo,            MAP_FLAG_VTMAP, 2    },
	{     "UNINSERT",         map_uninsert,        MAP_FLAG_VTMAP, 2    },
	{     "UNLINK",           map_unlink,          MAP_FLAG_VTMAP, 2    },
	{     "UPDATE",           map_update,          MAP_FLAG_VTMAP, 2    },
	{     "VNUM",             map_vnum,            MAP_FLAG_VTMAP, 2    },
	{     "WRITE",            map_write,           0,              1    },
	{     "",                 NULL,                0    }
};


struct cursor_type cursor_table[] =
{
	{
		"AUTO TAB BACKWARD",
		"Tab completion from scrollback buffer, backward",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_auto_tab_backward
	},
	{
		"AUTO TAB FORWARD",
		"Tab completion from scrollback buffer, forward",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_auto_tab_forward
	},
	{
		"BACKSPACE",
		"Delete backward character",
		"",
		CURSOR_FLAG_GET_ONE,
		cursor_backspace
	},

	{
		"BRACE OPEN",
		"Insert the { character",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_brace_open
	},

	{
		"BRACE CLOSE",
		"Insert the } character",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_brace_close
	},

	{
		"BACKWARD",
		"Move cursor backward",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_left
	},
	{
		"CLEAR",
		"Delete the input line",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_clear_line
	},
	{
		"CLEAR LEFT",
		"Delete from cursor to start of input",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_clear_left
	},
	{
		"CLEAR LINE", /* obsolete */
		"Delete the input line",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_clear_line
	},
	{
		"CLEAR RIGHT",
		"Delete from cursor to end of input",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_clear_right
	},
	{
		"CONVERT META",
		"Meta convert the next character",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_convert_meta
	},
	{
		"CTRL DELETE",
		"Delete one character, exit on an empty line",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_delete_or_exit
	},
	{
		"DELETE",
		"Delete character at cursor",
		"\e[3~",
		CURSOR_FLAG_GET_ALL,
		cursor_delete
	},
	{
		"DELETE WORD LEFT",
		"Delete backwards till next space",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_delete_word_left
	},
	{
		"DELETE WORD RIGHT",
		"Delete forwards till next space",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_delete_word_right
	},
	{
		"ECHO",
		"Turn local echoing on or off",
		"",
		CURSOR_FLAG_GET_ONE,
		cursor_echo
	},
	{
		"END",
		"Move cursor to end of input",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_end
	},
	{
		"ENTER",
		"Process the input line",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_enter
	},
	{
		"EXIT",
		"Exit current session",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_exit
	},
	{
		"FORWARD",
		"Move cursor forward",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_right
	},
	{
		"GET",
		"Copy input line to given variable",
		"",
		CURSOR_FLAG_GET_ONE,
		cursor_get
	},
	{
		"HISTORY NEXT",
		"Select next command history entry",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_history_next
	},
	{
		"HISTORY PREV",
		"Select previous command history entry",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_history_prev
	},
	{
		"HISTORY SEARCH",
		"Search command history",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_history_search
	},
	{
		"HOME",
		"Move the cursor to start of input",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_home
	},
	{
		"INFO",
		"Print debugging information",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_info
	},
	{
		"INSERT",
		"Turn insert mode on or off",
		"",
		CURSOR_FLAG_GET_ONE,
		cursor_insert
	},
	{
		"MIXED TAB BACKWARD",
		"Tab completion on last word, search backward",
		"\e[Z", // shift-tab
		CURSOR_FLAG_GET_ALL,
		cursor_mixed_tab_backward
	},
	{
		"MIXED TAB FORWARD",
		"Tab completion on last word, search forward",
		"\t",
		CURSOR_FLAG_GET_ALL,
		cursor_mixed_tab_forward
	},
	{
		"NEXT WORD",
		"Move cursor to the next word",
		"\ef",
		CURSOR_FLAG_GET_ALL,
		cursor_right_word
	},
	{
		"PASTE BUFFER",
		"Paste the previously deleted input text",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_paste_buffer
	},
	{
		"PREV WORD",
		"Move cursor to the previous word",
		"\eb",
		CURSOR_FLAG_GET_ALL,
		cursor_left_word
	},
	{
		"REDRAW INPUT",
		"Redraw the input line",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_redraw_input
	},
	{
		"SCREEN FOCUS IN",
		"Window is focussed in event",
		"\e[I",
		CURSOR_FLAG_GET_ALL|CURSOR_FLAG_ALWAYS,
		cursor_screen_focus_in
	},
	{
		"SCREEN FOCUS OUT",
		"Window is focussed out event",
		"\e[O",
		CURSOR_FLAG_GET_ALL|CURSOR_FLAG_ALWAYS,
		cursor_screen_focus_out
	},
	{
		"SET",
		"Copy given string to input line",
		"",
		CURSOR_FLAG_GET_ONE,
		cursor_set
	},
	{
		"SUSPEND",
		"Suspend program, return with fg",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_suspend
	},
	{
		"TAB BACKWARD",
		"Tab completion from tab list, backward",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_tab_backward
	},
	{
		"TAB FORWARD",
		"Tab completion from tab list, forward",
		"",
		CURSOR_FLAG_GET_ALL,
		cursor_tab_forward
	},

	{
		"", "", "\e[5~",  0, cursor_buffer_up
	},
	{
		"", "", "\e[6~",  0, cursor_buffer_down
	},
	{
		"", "", "",      0, cursor_buffer_lock
	},
	{
		"", "", "\eOM",   0, cursor_enter
	},
	{
		"", "", "\e[7~",  0, cursor_home
	},
	{
		"", "", "\e[1~",  0, cursor_home
	},
	{
		"", "", "\eOH",   0, cursor_home
	},
	{
		"", "", "\e[H",   0, cursor_home
	},
	{
		"", "", "\eOD",   0, cursor_left
	},
	{
		"", "", "\e[D",   0, cursor_left
	},
	{
		"", "", "\e[8~",  0, cursor_end
	},
	{
		"", "", "\e[4~",  0, cursor_end
	},
	{
		"", "", "\eOF",   0, cursor_end
	},
	{
		"", "", "\e[F",   0, cursor_end
	},
	{
		"", "", "\eOC",   0, cursor_right
	},
	{
		"", "", "\e[C",   0, cursor_right
	},
	{
		"", "", "\x7F",   0, cursor_backspace
	},
	{
		"", "", "\eOB",   0, cursor_history_next
	},
	{
		"", "", "\e[B",   0, cursor_history_next
	},
	{
		"", "", "\eOA",   0, cursor_history_prev
	},
	{
		"", "", "\e[A",   0, cursor_history_prev
	},
	{
		"", "", "\e\x7F", 0, cursor_delete_word_left
	},
	{
		"", "", "\ed",    0, cursor_delete_word_right
	},
	{
		"", "", "",       0, NULL
	}
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
		SCREEN_FLAG_GET_ONE,
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
		"RAISE",
		"Raise a screen event.",
		SCREEN_FLAG_GET_ALL,
		SCREEN_FLAG_GET_ALL,
		SCREEN_FLAG_CSIP,
		screen_raise
	},
	{
		"REFRESH",
		"Refresh the screen. (may not do much)",
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
		"Scrollbar",
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
		"", "", 0, 0, 0, NULL
	}
};


struct timer_type timer_table[] =
{
	{    "Poll Stdin"                  },
	{    "Poll Sessions"               },
	{    "Poll Chat Server"            },
	{    "Poll Port Sessions"          },
	{    "Update Tickers"              },
	{    "Update Delays"               },
	{    "Update Packet Patcher"       },
	{    "Update Chat Server"          },
	{    "Update Terminal"             },
	{    "Update Time Events"          },
	{    "Update Memory"               },
	{    "Stall Program"               }
};

struct event_type event_table[] =
{
	{    "CATCH ",                                 "Triggers on catch events."               },
	{    "CHAT MESSAGE",                           "Triggers on any chat related message."   },
	{    "CLASS ACTIVATED",                        "Triggers on class activations."          },
	{    "CLASS DEACTIVATED",                      "Triggers on class deactivations."        },
	{    "DATE",                                   "Triggers on the given date."             },
	{    "DAY",                                    "Triggers each day or given day."         },
	{    "DOUBLE-CLICKED ",                        "Triggers when mouse is double-clicked"   },
	{    "END OF PATH",                            "Triggers when walking the last room."    },
	{    "HOUR",                                   "Triggers each hour or given hour."       },
	{    "IAC ",                                   "Triggers on telopt negotiation."         },
	{    "LONG-CLICKED ",                          "Triggers when mouse is long-clicked."    },
	{    "MAP DOUBLE-CLICKED ",                    "Triggers on vt map click."               },
	{    "MAP ENTER MAP",                          "Triggers when entering the map."         },
	{    "MAP ENTER ROOM",                         "Triggers when entering a map room."      },
	{    "MAP EXIT MAP",                           "Triggers when exiting the map."          },
	{    "MAP EXIT ROOM",                          "Triggers when exiting a map room."       },
	{    "MAP FOLLOW MAP",                         "Triggers when moving to a map room."     },
	{    "MAP LONG-CLICKED ",                      "Triggers on vt map click."               },
	{    "MAP PRESSED ",                           "Triggers on vt map click."               },
	{    "MAP RELEASED ",                          "Triggers on vt map click."               },
	{    "MAP SHORT-CLICKED ",                     "Triggers on vt map click."               },
	{    "MAP TRIPLE-CLICKED ",                    "Triggers on vt map click."               },
	{    "MAP UPDATED VTMAP",                      "Triggers on vt map update."              },
	{    "MINUTE",                                 "Triggers each minute or given minute."   },
	{    "MONTH",                                  "Triggers each month or given month."     },
	{    "MOVED ",                                 "Triggers when mouse is moved."           },
	{    "PORT CONNECTION",                        "Triggers when socket connects."          },
	{    "PORT DISCONNECTION",                     "Triggers when socket disconnects."       },
	{    "PORT LOG MESSAGE",                       "Triggers on local port log messages."    },
	{    "PORT MESSAGE",                           "Triggers on local port messages."        },
	{    "PORT RECEIVED MESSAGE",                  "Triggers when socket data is received."  },
	{    "PRESSED ",                               "Triggers when mouse button is pressed."  },
	{    "PROGRAM START",                          "Triggers when main session starts."      },
	{    "PROGRAM TERMINATION",                    "Triggers when main session exists."      },
	{    "RECEIVED INPUT",                         "Triggers when new input is received."    },
	{    "RECEIVED KEYPRESS",                      "Triggers when a keypress is received."   },
	{    "RECEIVED LINE",                          "Triggers when a new line is received."   },
	{    "RECEIVED OUTPUT",                        "Triggers when new output is received."   },
	{    "RECEIVED PROMPT",                        "Triggers when a prompt is received."     },
	{    "RELEASED ",                              "Triggers when mouse button is released." },
	{    "SCAN CSV HEADER",                        "Triggers when scanning a csv file."      },
	{    "SCAN CSV LINE",                          "Triggers when scanning a csv file."      },
	{    "SCAN TSV HEADER",                        "Triggers when scanning a tsv file."      },
	{    "SCAN TSV LINE",                          "Triggers when scanning a tsv file."      },
	{    "SCREEN CHARACTER DIMENSIONS",            "Triggers when called by #screen raise."  },
	{    "SCREEN DESKTOP DIMENSIONS",              "Triggers when called by #screen raise."  },
	{    "SCREEN DIMENSIONS",                      "Triggers when called by #screen raise."  },
	{    "SCREEN MINIMIZED",                       "Triggers when called by #screen raise."  },
	{    "SCREEN POSITION",                        "Triggers when called by #screen raise."  },
	{    "SCREEN RESIZE",                          "Triggers when the screen is resized."    },
	{    "SCROLLED ",                              "Triggers when mouse wheel is scrolled."  },
	{    "SECOND",                                 "Triggers each second or given second."   },
	{    "SEND OUTPUT",                            "Triggers before sending output."         },
	{    "SENT OUTPUT",                            "Triggers after sending output."          },
	{    "SESSION ACTIVATED",                      "Triggers when a session is activated."   },
	{    "SESSION CONNECTED",                      "Triggers when a new session connects."   },
	{    "SESSION CREATED",                        "Triggers when a new session is created." },
	{    "SESSION DEACTIVATED",                    "Triggers when a session is deactivated." },
	{    "SESSION DISCONNECTED",                   "Triggers when a session disconnects."    },
	{    "SESSION TIMED OUT",                      "Triggers when a session doesn't connect."},
	{    "SHORT-CLICKED",                          "Triggers when mouse is short-clicked."   },
	{    "SYSTEM ERROR",                           "Triggers on system errors."              },
	{    "TIME",                                   "Triggers on the given time."             },
	{    "TRIPLE-CLICKED",                         "Triggers when mouse is triple-clicked."  },
	{    "UNKNOWN COMMAND",                        "Triggers on unknown tintin command."     },
	{    "VARIABLE UPDATE ",                       "Triggers on a variable update."          },
	{    "VT100 CPR",                              "Triggers on an ESC [ 6 n call."          },
	{    "VT100 DA",                               "Triggers on an ESC [ c call."            },
	{    "VT100 DECID",                            "Triggers on an ESC Z call."              },
	{    "VT100 DSR",                              "Triggers on an ESC [ 5 n call."          },
	{    "VT100 ENQ",                              "Triggers on an \\x05 call."              },
	{    "VT100 SCROLL REGION",                    "Triggers on vt100 scroll region change." },
	{    "WEEK",                                   "Triggers each week or given week."       },
	{    "WINDOW FOCUS IN",                        "Triggers on window focussing in."        },
	{    "WINDOW FOCUS OUT",                       "Triggers on window focussing out."       },
	{    "YEAR",                                   "Triggers each year or given year."       },
	{    "",                                       ""                                        }
};

struct path_type path_table[] =
{
	{    "CREATE",            path_create,         "Clear the path and start path mapping."         },
	{    "DELETE",            path_delete,         "Delete the last command from the path."         },
	{    "DESCRIBE",          path_describe,       "Describe the path and current position."        },
	{    "DESTROY",           path_destroy,        "Clear the path and stop path mapping."          },
	{    "END",               path_end,            ""                                               },
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
	{    "GAG",               line_gag               },
	{    "IGNORE",            line_ignore            },
	{    "LOG",               line_log               },
	{    "LOGVERBATIM",       line_logverbatim       },
	{    "QUIET",             line_quiet             },
	{    "STRIP",             line_strip             },
	{    "SUBSTITUTE",        line_substitute        },
	{    "VERBATIM",          line_verbatim          },
	{    "VERBOSE",           line_verbose           },
	{    "",                  NULL                   }
};

struct history_type history_table[] =
{
/*	{    "CHARACTER",         history_character,   "Set the character used for repeating commands." }, */
	{    "DELETE",            history_delete,      "Delete last command history entry."             },
	{    "INSERT",            history_insert,      "Insert a new command history entry."            },
	{    "LIST",              history_list,        "Display command history list."                  },
	{    "READ",              history_read,        "Read a command history list from file."         },
/*	{    "SIZE",              history_size,        "The size of the command history."               }, */
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
	{    "LOCK",              buffer_lock,         "Toggle the locking state of the buffer."        },
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
	{    "NEW-ENVIRON",       TEL_N,               ANNOUNCE_DO },
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

	{ NULL,			NULL,			NULL,	NULL	}
};

struct map_group_type map_group_table[] =
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

	{ NULL,				NULL,			0, 0,	0, 0,	0, 0,	"" }
};
