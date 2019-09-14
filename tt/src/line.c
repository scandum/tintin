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
*                      coded by Igor van den Hoven 2008                       *
******************************************************************************/

#include "tintin.h"

DO_COMMAND(do_line)
{
	char arg1[BUFFER_SIZE];
	int cnt;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {<OPTION>} {argument}.");
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
			do_line(ses, "");
		}
		else
		{
			ses = line_table[cnt].fun(ses, arg);
		}
	}
	return ses;
}


DO_LINE(line_gag)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	show_debug(ses, LIST_GAG, "#DEBUG LINE GAG");

	SET_BIT(ses->flags, SES_FLAG_GAG);

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
			logit(ses, arg2, ses->logline_file, FALSE);
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

				if (HAS_BIT(ses->flags, SES_FLAG_LOGHTML))
				{
					fseek(logfile, 0, SEEK_END);

					if (ftell(logfile) == 0)
					{
						write_html_header(ses, logfile);
					}
				}

				logit(ses, arg2, ses->logline_file, FALSE);
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
			SET_BIT(ses->flags, SES_FLAG_LOGNEXT);
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

			SET_BIT(ses->flags, SES_FLAG_LOGNEXT);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #LINE LOG {%s} - COULDN'T OPEN FILE.", arg1);
		}
	}
	return ses;
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
			logit(ses, arg2, ses->logline_file, TRUE);
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

				if (HAS_BIT(ses->flags, SES_FLAG_LOGHTML))
				{
					fseek(logfile, 0, SEEK_END);

					if (ftell(logfile) == 0)
					{
						write_html_header(ses, logfile);
					}
				}

				logit(ses, arg2, ses->logline_file, TRUE);
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
			SET_BIT(ses->flags, SES_FLAG_LOGNEXT);
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

			SET_BIT(ses->flags, SES_FLAG_LOGNEXT);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #LINE LOGVERBATIM {%s} - COULDN'T OPEN FILE.", arg1);
		}
	}
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

	gtd->verbatim_level++;

	ses = parse_input(ses, arg1);

	gtd->verbatim_level--;

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

	gtd->verbose_level++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->verbose_level--;

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

	gtd->ignore_level++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->ignore_level--;

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

	gtd->quiet++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->quiet--;

	return ses;
}
	