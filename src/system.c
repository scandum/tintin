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
*                     coded by Igor van den Hoven 2007                        *
******************************************************************************/

#include "tintin.h"

#ifdef HAVE_PTY_H
#include <pty.h>
#else
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#endif

DO_COMMAND(do_run)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], temp[BUFFER_SIZE], file[BUFFER_SIZE];
	int desc, pid;
	struct winsize size;
	struct termios run_terminal;

	char *argv[4] = {"sh", "-c", "", NULL};

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, file, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 || *arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #RUN {NAME} {SYSTEM SHELL COMMAND}");
		
		return ses;
	}

	size.ws_row = get_scroll_size(ses);
	size.ws_col = gtd->screen->cols;

	pid = forkpty(&desc, temp, &gtd->old_terminal, &size);

	switch (pid)
	{
		case -1:
			syserr_printf(ses, "do_run: forkpty");
			break;

		case 0:
			sprintf(temp, "exec %s", arg2);
			argv[2] = temp;
			execv("/bin/sh", argv);
			tcgetattr(0, &run_terminal);
			break;

		default:
			sprintf(temp, "{%s} {%d} {%s}", arg2, pid, file);

			ses = new_session(ses, arg1, temp, desc, 0);

//			memcpy(&ses->cur_terminal, &run_terminal, sizeof(run_terminal));

//			refresh_session_terminal(ses);
			break;
	}
	return ses;
}

DO_COMMAND(do_script)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], *cptr, buf[BUFFER_SIZE], var[BUFFER_SIZE], tmp[BUFFER_SIZE];
	FILE *script;
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SCRIPT: ONE ARGUMENT REQUIRED.");
	}
	else if (*arg2 == 0)
	{
		script = popen(arg1, "r");

		if (script)
		{
			while (fgets(buf, BUFFER_SIZE - 1, script))
			{
				cptr = strchr(buf, '\n');

				if (cptr)
				{
					*cptr = 0;
				}

				ses = script_driver(ses, LIST_COMMAND, buf);
			}

			pclose(script);
		}
		else
		{
			syserr_printf(ses, "do_script: popen 1");
		}
	}
	else
	{
		index = 1;

		script = popen(arg2, "r");

		if (script)
		{
			var[0] = 0;

			while (fgets(buf, BUFFER_SIZE - 1, script))
			{
				cptr = strchr(buf, '\n');

				if (cptr)
				{
					*cptr = 0;
				}

				substitute(ses, buf, tmp, SUB_SEC);

				cat_sprintf(var, "{%d}{%s}", index++, tmp);
			}

			set_nest_node(ses->list[LIST_VARIABLE], arg1, "%s", var);

			pclose(script);
		}
		else
		{
			syserr_printf(ses, "do_script: popen 2");
		}
	}
	refresh_session_terminal(ses);

	return ses;
}


DO_COMMAND(do_suspend)
{
	suspend_handler(0);

	return ses;
}


DO_COMMAND(do_system)
{
	char arg1[BUFFER_SIZE];

	sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SYSTEM {COMMAND}.");
		
		return ses;
	}

	show_message(ses, LIST_COMMAND, "#OK: EXECUTING '%s'", arg1);

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_READMUD) && IS_SPLIT(gtd->ses))
	{
		save_pos(gtd->ses);
		goto_rowcol(gtd->ses, gtd->ses->bot_row, 1);
	}
	fflush(stdout);

	system(arg1);

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_READMUD) && IS_SPLIT(gtd->ses))
	{
		restore_pos(gtd->ses);
	}
	fflush(stdout);

	refresh_session_terminal(gtd->ses);

	return ses;
}


DO_COMMAND(do_textin)
{
	FILE *fp;
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], buffer[BUFFER_SIZE], *cptr;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if ((fp = fopen(arg1, "r")) == NULL)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #TEXTIN {%s} - FILE NOT FOUND.", arg1);
		
		return ses;
	}

	while (fgets(buffer, BUFFER_SIZE - 1, fp))
	{
		cptr = strchr(buffer, '\n');

		if (cptr)
		{
			*cptr = 0;
		}

		write_mud(ses, buffer, SUB_EOL);

		if (*arg2)
		{
			usleep((long long) (get_number(ses, arg2) * 1000000));
		}
	}
	fclose(fp);

	show_message(ses, LIST_COMMAND, "#TEXTIN {%s} - FILE READ.", arg1);

	return ses;
}
