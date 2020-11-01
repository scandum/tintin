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
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"

struct command_type command_table[];

DO_COMMAND(do_commands)
{
	int cmd;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	tintin_header(ses, 0, " %s ", "COMMANDS");

	for (cmd = 0 ; *command_table[cmd].name != 0 ; cmd++)
	{
		if (*arg1 && !is_abbrev(arg1, command_table[cmd].name))
		{
			continue;
		}

		if (strip_vt102_strlen(ses, arg2) + 20 > get_scroll_cols(ses))
		{
			tintin_puts2(ses, arg2);
			*arg2 = 0;
		}

		if (command_table[cmd].type == TOKEN_TYPE_COMMAND)
		{
			cat_sprintf(arg2, "%s%20s", COLOR_COMMAND, command_table[cmd].name);
		}
		else
		{
			cat_sprintf(arg2, "%s%20s", COLOR_STATEMENT, command_table[cmd].name);
		}
	}

	if (*arg2)
	{
		tintin_puts2(ses, arg2);
	}

	tintin_header(ses, 0, "");

	return ses;
}


void init_commands()
{
	int index, ref;

	for (index = 1 ; *command_table[index].name ; index++)
	{
		if (strcmp(command_table[index - 1].name, command_table[index].name) > 0)
		{
			print_stdout(0, 0, "\e[1;31minit_tintin() unsorted command table %s vs %s.", command_table[index - 1].name, command_table[index].name);
			exit(1);
		}
	}

	for (ref = 0 ; ref < 32 ; ref++)
	{
		for (index = 0 ; *command_table[index].name ; index++)
		{
			if (*command_table[index].name % 32 == ref)
			{
				break;
			}
		}
		gtd->command_ref[ref] = index;
	}
}

// unused

struct session *execute(struct session *ses, char *format, ...)
{
	char *buffer;
	va_list args;

	push_call("execute(%p,%p,...)",ses,format);

	va_start(args, format);

	if (vasprintf(&buffer, format, args) == -1)
	{
		syserr_printf(ses, "execute: vasprintf:");
	}

	va_end(args);

	if (*buffer)
	{
		if (*buffer != gtd->tintin_char)
		{
			*buffer = gtd->tintin_char;
		}
		get_arg_all(ses, buffer, buffer, FALSE);
	}

	ses = script_driver(ses, LIST_COMMAND, buffer);

	free(buffer);

	pop_call();
	return ses;
}

struct session *command(struct session *ses, COMMAND *cmd, char *format, ...)
{
	char *arg1, *arg2, *arg3, *arg4, *buffer;
	va_list args;

	push_call("command(%p,%p,%p,...)",ses,cmd,format);

	va_start(args, format);

	if (vasprintf(&buffer, format, args) == -1)
	{
		syserr_printf(ses, "command: vasprintf:");
	}

	va_end(args);

	arg1 = str_alloc_stack(0);
	arg2 = str_alloc_stack(0);
	arg3 = str_alloc_stack(0);
	arg4 = "";

	ses = cmd(ses, buffer, arg1, arg2, arg3, arg4);

	free(buffer);

	pop_call();
	return ses;
}

extern DO_COMMAND(do_action);
extern DO_COMMAND(do_alias);
extern DO_COMMAND(do_all);
extern DO_COMMAND(do_bell);
extern DO_COMMAND(do_button);
extern DO_COMMAND(do_cat);
extern DO_COMMAND(do_class);
extern DO_COMMAND(do_commands);
extern DO_COMMAND(do_cr);
extern DO_COMMAND(do_debug);
extern DO_COMMAND(do_echo);
extern DO_COMMAND(do_event);
extern DO_COMMAND(do_format);
extern DO_COMMAND(do_function);
extern DO_COMMAND(do_gag);
extern DO_COMMAND(do_highlight);
extern DO_COMMAND(do_ignore);
extern DO_COMMAND(do_info);
extern DO_COMMAND(do_killall);
extern DO_COMMAND(do_log);
extern DO_COMMAND(do_local);
extern DO_COMMAND(do_list);
extern DO_COMMAND(do_macro);
extern DO_COMMAND(do_math);
extern DO_COMMAND(do_message);
extern DO_COMMAND(do_path);
extern DO_COMMAND(do_port);
extern DO_COMMAND(do_prompt);
extern DO_COMMAND(do_replace);
extern DO_COMMAND(do_run);
extern DO_COMMAND(do_scan);
extern DO_COMMAND(do_script);
extern DO_COMMAND(do_send);
extern DO_COMMAND(do_showme);
extern DO_COMMAND(do_ssl);
extern DO_COMMAND(do_substitute);
extern DO_COMMAND(do_system);
extern DO_COMMAND(do_tab);
extern DO_COMMAND(do_textin);
extern DO_COMMAND(do_tick);
extern DO_COMMAND(do_unaction);
extern DO_COMMAND(do_unalias);
extern DO_COMMAND(do_unbutton);
extern DO_COMMAND(do_undelay);
extern DO_COMMAND(do_unevent);
extern DO_COMMAND(do_unfunction);
extern DO_COMMAND(do_ungag);
extern DO_COMMAND(do_unhighlight);
extern DO_COMMAND(do_unlocal);
extern DO_COMMAND(do_unmacro);
extern DO_COMMAND(do_unpathdir);
extern DO_COMMAND(do_unprompt);
extern DO_COMMAND(do_unsubstitute);
extern DO_COMMAND(do_untab);
extern DO_COMMAND(do_untick);
extern DO_COMMAND(do_unvariable);
extern DO_COMMAND(do_variable);

struct command_type command_table[] =
{
	{    "action",            do_action,            3, TOKEN_TYPE_COMMAND },
	{    "alias",             do_alias,             3, TOKEN_TYPE_COMMAND },
	{    "all",               do_all,               1, TOKEN_TYPE_COMMAND },
	{    "banner",            do_banner,            1, TOKEN_TYPE_COMMAND },
	{    "bell",              do_bell,              2, TOKEN_TYPE_COMMAND },
	{    "break",             do_nop,               0, TOKEN_TYPE_BREAK   },
	{    "buffer",            do_buffer,            2, TOKEN_TYPE_COMMAND },
	{    "button",            do_button,            3, TOKEN_TYPE_COMMAND },
	{    "case",              do_nop,               0, TOKEN_TYPE_CASE    },
	{    "cat",               do_cat,               1, TOKEN_TYPE_COMMAND },
	{    "chat",              do_chat,              2, TOKEN_TYPE_COMMAND },
	{    "class",             do_class,             3, TOKEN_TYPE_COMMAND },
	{    "commands",          do_commands,          2, TOKEN_TYPE_COMMAND },
	{    "config",            do_configure,         2, TOKEN_TYPE_COMMAND },
	{    "continue",          do_nop,               0, TOKEN_TYPE_CONTINUE},
	{    "cr",                do_cr,                0, TOKEN_TYPE_COMMAND },
	{    "cursor",            do_cursor,            2, TOKEN_TYPE_COMMAND },
	{    "daemon",            do_daemon,            2, TOKEN_TYPE_COMMAND },
	{    "debug",             do_debug,             2, TOKEN_TYPE_COMMAND },
	{    "default",           do_nop,               0, TOKEN_TYPE_DEFAULT },
	{    "delay",             do_delay,             3, TOKEN_TYPE_COMMAND },
	{    "dictionary",        do_dictionary,        3, TOKEN_TYPE_COMMAND },
	{    "draw",              do_draw,              3, TOKEN_TYPE_COMMAND },
	{    "echo",              do_echo,              3, TOKEN_TYPE_COMMAND },
	{    "edit",              do_edit,              2, TOKEN_TYPE_COMMAND },
	{    "else",              do_nop,               0, TOKEN_TYPE_ELSE    },
	{    "elseif",            do_nop,               0, TOKEN_TYPE_ELSEIF  },
	{    "end",               do_end,               1, TOKEN_TYPE_COMMAND },
	{    "event",             do_event,             2, TOKEN_TYPE_COMMAND },
	{    "foreach",           do_nop,               3, TOKEN_TYPE_FOREACH },
	{    "format",            do_format,            3, TOKEN_TYPE_COMMAND },
	{    "function",          do_function,          2, TOKEN_TYPE_COMMAND },
	{    "gag",               do_gag,               1, TOKEN_TYPE_COMMAND },
	{    "grep",              do_grep,              2, TOKEN_TYPE_COMMAND },
	{    "help",              do_help,              1, TOKEN_TYPE_COMMAND },
	{    "highlight",         do_highlight,         3, TOKEN_TYPE_COMMAND },
	{    "history",           do_history,           3, TOKEN_TYPE_COMMAND },
	{    "if",                do_nop,               0, TOKEN_TYPE_IF      },
	{    "ignore",            do_ignore,            2, TOKEN_TYPE_COMMAND },
	{    "info",              do_info,              2, TOKEN_TYPE_COMMAND },
	{    "kill",              do_kill,              2, TOKEN_TYPE_COMMAND },
	{    "killall",           do_killall,           2, TOKEN_TYPE_COMMAND },
	{    "line",              do_line,              3, TOKEN_TYPE_COMMAND },
	{    "list",              do_list,              3, TOKEN_TYPE_COMMAND },
	{    "local",             do_local,             1, TOKEN_TYPE_COMMAND },
	{    "log",               do_log,               2, TOKEN_TYPE_COMMAND },
	{    "loop",              do_nop,               3, TOKEN_TYPE_LOOP    },
	{    "macro",             do_macro,             3, TOKEN_TYPE_COMMAND },
	{    "map",               do_map,               2, TOKEN_TYPE_COMMAND },
	{    "math",              do_math,              2, TOKEN_TYPE_COMMAND },
	{    "message",           do_message,           2, TOKEN_TYPE_COMMAND },
	{    "nop",               do_nop,               0, TOKEN_TYPE_COMMAND },
	{    "parse",             do_nop,               3, TOKEN_TYPE_PARSE   },
	{    "path",              do_path,              1, TOKEN_TYPE_COMMAND },
	{    "pathdir",           do_pathdir,           3, TOKEN_TYPE_COMMAND },
	{    "port",              do_port,              2, TOKEN_TYPE_COMMAND },
	{    "prompt",            do_prompt,            2, TOKEN_TYPE_COMMAND },
	{    "read",              do_read,              1, TOKEN_TYPE_COMMAND },
	{    "regexp",            do_regexp,            3, TOKEN_TYPE_REGEX   },
	{    "replace",           do_replace,           3, TOKEN_TYPE_COMMAND },
	{    "return",            do_nop,               0, TOKEN_TYPE_RETURN  },
	{    "run",               do_run,               3, TOKEN_TYPE_COMMAND },
	{    "scan",              do_scan,              2, TOKEN_TYPE_COMMAND },
	{    "screen",            do_screen,            2, TOKEN_TYPE_COMMAND },
	{    "script",            do_script,            2, TOKEN_TYPE_COMMAND },
	{    "send",              do_send,              1, TOKEN_TYPE_COMMAND },
	{    "session",           do_session,           1, TOKEN_TYPE_COMMAND },
	{    "showme",            do_showme,            3, TOKEN_TYPE_COMMAND },
	{    "snoop",             do_snoop,             2, TOKEN_TYPE_COMMAND },
	{    "split",             do_split,             2, TOKEN_TYPE_COMMAND },
	{    "ssl",               do_ssl,               3, TOKEN_TYPE_COMMAND },
	{    "substitute",        do_substitute,        3, TOKEN_TYPE_COMMAND },
	{    "switch",            do_nop,               0, TOKEN_TYPE_SWITCH  },
	{    "system",            do_system,            1, TOKEN_TYPE_COMMAND },
	{    "tab",               do_tab,               1, TOKEN_TYPE_COMMAND },
	{    "test",              do_test,              3, TOKEN_TYPE_COMMAND },
	{    "textin",            do_textin,            2, TOKEN_TYPE_COMMAND },
	{    "ticker",            do_tick,              3, TOKEN_TYPE_COMMAND },
	{    "unaction",          do_unaction,          0, TOKEN_TYPE_COMMAND },
	{    "unalias",           do_unalias,           0, TOKEN_TYPE_COMMAND },
	{    "unbutton",          do_unbutton,          0, TOKEN_TYPE_COMMAND },
	{    "undelay",           do_undelay,           1, TOKEN_TYPE_COMMAND },
	{    "unevent",           do_unevent,           0, TOKEN_TYPE_COMMAND },
	{    "unfunction",        do_unfunction,        0, TOKEN_TYPE_COMMAND },
	{    "ungag",             do_ungag,             0, TOKEN_TYPE_COMMAND },
	{    "unhighlight",       do_unhighlight,       0, TOKEN_TYPE_COMMAND },
	{    "unlocal",           do_unlocal,           1, TOKEN_TYPE_COMMAND },
	{    "unmacro",           do_unmacro,           0, TOKEN_TYPE_COMMAND },
	{    "unpathdir",         do_unpathdir,         1, TOKEN_TYPE_COMMAND },
	{    "unprompt",          do_unprompt,          0, TOKEN_TYPE_COMMAND },
	{    "unsplit",           do_unsplit,           0, TOKEN_TYPE_COMMAND },
	{    "unsubstitute",      do_unsubstitute,      0, TOKEN_TYPE_COMMAND },
	{    "untab",             do_untab,             0, TOKEN_TYPE_COMMAND },
	{    "unticker",          do_untick,            0, TOKEN_TYPE_COMMAND },
	{    "unvariable",        do_unvariable,        1, TOKEN_TYPE_COMMAND },
	{    "variable",          do_variable,          1, TOKEN_TYPE_COMMAND },
	{    "while",             do_nop,               0, TOKEN_TYPE_WHILE   },
	{    "write",             do_write,             2, TOKEN_TYPE_COMMAND },
	{    "zap",               do_zap,               1, TOKEN_TYPE_COMMAND },
	{    "",                  NULL,                 0, TOKEN_TYPE_COMMAND }
};
