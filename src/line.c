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
*                      coded by Igor van den Hoven 2008                       *
******************************************************************************/

#include "tintin.h"

DO_COMMAND(do_line)
{
	int cnt;

	push_call("do_line(%p,%p)",ses,arg);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (*arg1 == 0)
	{
		info:

		tintin_header(ses, " LINE OPTIONS ");

		for (cnt = 0 ; *line_table[cnt].fun != NULL ; cnt++)
		{
			if (*line_table[cnt].desc)
			{
				tintin_printf2(ses, "  [%-13s] %s", line_table[cnt].name, line_table[cnt].desc);
			}
		}
		tintin_header(ses, "");

		pop_call();
		return ses;
	}
	else
	{
		for (cnt = 0 ; *line_table[cnt].name ; cnt++)
		{
			if (is_abbrev(arg1, line_table[cnt].name))
			{
				break;
			}
		}

		if (*line_table[cnt].name == 0)
		{
			goto info;
		}
		else
		{
			ses = line_table[cnt].fun(ses, arg);
		}
	}
	pop_call();
	return ses;
}

DO_LINE(line_background)
{
	char arg1[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {BACKGROUND} {command}.");

		return ses;
	}

	gtd->level->background++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->background--;

	return ses;
}

DO_LINE(line_benchmark)
{
	long long start, end;
	char arg1[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {BENCHMARK} {command}.");

		return ses;
	}

	start = utime();

	ses = script_driver(ses, LIST_COMMAND, arg1);

	end = utime();

	tintin_printf2(ses, "#LINE BENCHMARK: %lld USEC.", end - start);

	return ses;
}

DO_LINE(line_capture)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 && *arg2)
	{
		if (ses->line_capturefile)
		{
			free(ses->line_capturefile);
		}
		ses->line_capturefile  = strdup(arg1);
		ses->line_captureindex = 1;

		ses = script_driver(ses, LIST_COMMAND, arg2);

		if (ses->line_capturefile)
		{
			free(ses->line_capturefile);
			ses->line_capturefile = NULL;
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE CAPTURE {VARIABLE} {command}.");
	}
	return ses;
}

DO_LINE(line_convert)
{
	char arg1[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {CONVERT} {command}.");

		return ses;
	}

	gtd->level->convert++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->convert--;

	return ses;
}

DO_LINE(line_debug)
{
	char arg1[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {DEBUG} {command}.");

		return ses;
	}

	gtd->level->debug++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->debug--;

	return ses;
}

DO_LINE(line_gag)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	switch (*arg1)
	{
		case '-':
			ses->gagline -= get_number(ses, arg1+1);
			break;
		case '+':
			ses->gagline += get_number(ses, arg1+1);
			break;
		case 0:
			ses->gagline = 1;
			break;
		default:
			ses->gagline = get_number(ses, arg1);
			break;
	}

	show_debug(ses, LIST_GAG, "#DEBUG LINE GAG {%s}", arg1);

//	SET_BIT(ses->flags, SES_FLAG_GAG);

	return ses;
}

DO_LINE(line_ignore)
{
	char arg1[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {IGNORE} {command}.");
		
		return ses;
	}

	gtd->level->ignore++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->ignore--;

	return ses;
}

DO_LINE(line_local)
{
	char arg1[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {LOCAL} {command}.");

		return ses;
	}

	gtd->level->local++;

	SET_BIT(gtd->flags, TINTIN_FLAG_LOCAL);

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->local--;

	return ses;
}

// Without an argument mark next line to be logged, otherwise log the given line to file.

DO_LINE(line_log)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	FILE *logfile;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 && *arg2)
	{
		substitute(ses, arg2, arg2, SUB_ESC|SUB_COL|SUB_LNF);

		if (ses->logline_time == gtd->time && !strcmp(ses->logline_name, arg1))
		{
			logit(ses, arg2, ses->logline_file, LOG_FLAG_NONE);
		}
		else
		{
			if ((logfile = fopen(arg1, "a")))
			{
				if (ses->logline_file)
				{
					fclose(ses->logline_file);
				}
				free(ses->logline_name);

				ses->logline_name = strdup(arg1);
				ses->logline_file = logfile;
				ses->logline_time = gtd->time;

				loginit(ses, ses->logline_file, LOG_FLAG_APPEND | HAS_BIT(ses->logmode, LOG_FLAG_HTML));

				logit(ses, arg2, ses->logline_file, LOG_FLAG_NONE);
			}
			else
			{
				show_error(ses, LIST_COMMAND, "#ERROR: #LINE LOG {%s} - COULDN'T OPEN FILE.", arg1);
			}
		}
	}
	else
	{
		if (ses->lognext_time == gtd->time && !strcmp(ses->lognext_name, arg1))
		{
			SET_BIT(ses->logmode, LOG_FLAG_NEXT);
		}
		else if ((logfile = fopen(arg1, "a")))
		{
			if (ses->lognext_file)
			{
				fclose(ses->lognext_file);
			}
			free(ses->lognext_name);

			ses->lognext_name = strdup(arg1);
			ses->lognext_file = logfile;
			ses->lognext_time = gtd->time;

			SET_BIT(ses->logmode, LOG_FLAG_NEXT);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #LINE LOG {%s} - COULDN'T OPEN FILE.", arg1);
		}
	}
	return ses;
}

DO_LINE(line_logmode)
{
	struct session *active_ses;

	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	DEL_BIT(ses->logmode, LOG_FLAG_OLD_HTML|LOG_FLAG_OLD_PLAIN|LOG_FLAG_OLD_RAW);

	switch (HAS_BIT(ses->logmode, LOG_FLAG_HTML|LOG_FLAG_PLAIN|LOG_FLAG_RAW))
	{
		case LOG_FLAG_HTML:
			SET_BIT(ses->logmode, LOG_FLAG_OLD_HTML);
			break;
		case LOG_FLAG_PLAIN:
			SET_BIT(ses->logmode, LOG_FLAG_OLD_PLAIN);
			break;
		case LOG_FLAG_RAW:
			SET_BIT(ses->logmode, LOG_FLAG_OLD_RAW);
			break;
	}

	if (is_abbrev(arg1, "HTML"))
	{
		SET_BIT(ses->logmode, LOG_FLAG_HTML);
		DEL_BIT(ses->logmode, LOG_FLAG_PLAIN);
		DEL_BIT(ses->logmode, LOG_FLAG_RAW);
	}
	else if (is_abbrev(arg1, "PLAIN"))
	{
		SET_BIT(ses->logmode, LOG_FLAG_PLAIN);
		DEL_BIT(ses->logmode, LOG_FLAG_HTML);
		DEL_BIT(ses->logmode, LOG_FLAG_RAW);
	}
	else if (is_abbrev(arg1, "RAW"))
	{
		SET_BIT(ses->logmode, LOG_FLAG_RAW);
		DEL_BIT(ses->logmode, LOG_FLAG_HTML);
		DEL_BIT(ses->logmode, LOG_FLAG_PLAIN);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {LOGMODE} {HTML|PLAIN|RAW} {command}.");

		return ses;
	}

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	active_ses = script_driver(ses, LIST_COMMAND, arg1);

	DEL_BIT(ses->logmode, LOG_FLAG_HTML|LOG_FLAG_PLAIN|LOG_FLAG_RAW);

	switch (HAS_BIT(ses->logmode, LOG_FLAG_OLD_HTML|LOG_FLAG_OLD_PLAIN|LOG_FLAG_OLD_RAW))
	{
		case LOG_FLAG_OLD_HTML:
			SET_BIT(ses->logmode, LOG_FLAG_HTML);
			break;
		case LOG_FLAG_OLD_PLAIN:
			SET_BIT(ses->logmode, LOG_FLAG_PLAIN);
			break;
		case LOG_FLAG_OLD_RAW:
			SET_BIT(ses->logmode, LOG_FLAG_RAW);
			break;
	}

	return ses = active_ses;
}

DO_LINE(line_logverbatim)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	FILE *logfile;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 && *arg2)
	{
		if (!strcmp(ses->logline_name, arg1))
		{
			logit(ses, arg2, ses->logline_file, LOG_FLAG_LINEFEED);
		}
		else
		{
			if ((logfile = fopen(arg1, "a")))
			{
				if (ses->logline_file)
				{
					fclose(ses->logline_file);
				}
				free(ses->logline_name);
				ses->logline_name = strdup(arg1);
				ses->logline_file = logfile;

				loginit(ses, ses->logline_file, LOG_FLAG_APPEND | HAS_BIT(ses->logmode, LOG_FLAG_HTML));

				logit(ses, arg2, ses->logline_file, LOG_FLAG_LINEFEED);
			}
			else
			{
				show_error(ses, LIST_COMMAND, "#ERROR: #LINE LOGVERBATIM {%s} - COULDN'T OPEN FILE.", arg1);
			}
		}
	}
	else
	{
		if (!strcmp(ses->lognext_name, arg1))
		{
			SET_BIT(ses->logmode, LOG_FLAG_NEXT);
		}
		else if ((logfile = fopen(arg1, "a")))
		{
			if (ses->lognext_file)
			{
				fclose(ses->lognext_file);
			}
			free(ses->lognext_name);
			ses->lognext_name = strdup(arg1);
			ses->lognext_file = logfile;

			SET_BIT(ses->logmode, LOG_FLAG_NEXT);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #LINE LOGVERBATIM {%s} - COULDN'T OPEN FILE.", arg1);
		}
	}
	return ses;
}

DO_LINE(line_multishot)
{
	unsigned int shots;
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	shots = (unsigned int) get_number(ses, arg1);

	if (!is_math(ses, arg1) || *arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {MULTISHOT} {number} {command}.");
		
		return ses;
	}

	gtd->level->shots++;

	gtd->level->mshot = shots;

	ses = script_driver(ses, LIST_COMMAND, arg2);

	gtd->level->mshot = 1;

	gtd->level->shots--;

	return ses;
}

DO_LINE(line_oneshot)
{
	char arg1[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {ONESHOT} {command}.");
		
		return ses;
	}

	gtd->level->shots++;

	gtd->level->mshot = 1;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->shots--;

	return ses;
}

DO_LINE(line_quiet)
{
	char arg1[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {QUIET} {command}.");
		
		return ses;
	}

	gtd->level->quiet++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->quiet--;

	return ses;
}

DO_LINE(line_strip)
{
	char arg1[BUFFER_SIZE], strip[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_ESC|SUB_COL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {STRIP} {command}.");

		return ses;
	}

	strip_vt102_codes(arg1, strip);

	ses = script_driver(ses, LIST_COMMAND, strip);

	return ses;
}

DO_LINE(line_substitute)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], subs[BUFFER_SIZE];
	int i, flags = 0;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {SUBSTITUTE} {argument} {command}.");
		
		return ses;
	}

	arg = arg1;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, subs, GET_ONE);

		for (i = 0 ; *substitution_table[i].name ; i++)
		{
			if (is_abbrev(subs, substitution_table[i].name))
			{
				SET_BIT(flags, substitution_table[i].bitvector);
			}
		}

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}

	substitute(ses, arg2, subs, flags);

	ses = script_driver(ses, LIST_COMMAND, subs);

	return ses;
}

DO_LINE(line_verbatim)
{
	char arg1[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {VERBATIM} {command}.");
		
		return ses;
	}

	gtd->level->verbatim++;

	ses = parse_input(ses, arg1);

	gtd->level->verbatim--;

	return ses;
}

DO_LINE(line_verbose)
{
	char arg1[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {VERBOSE} {command}.");
		
		return ses;
	}

	gtd->level->verbose++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->verbose--;

	return ses;
}

	