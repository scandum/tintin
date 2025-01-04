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

struct line_type
{
	char                  * name;
	LINE                  * fun;
	char                  * desc;
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
	{    "JSON",              line_json,           "Execute line with json conversion."             },
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

DO_COMMAND(do_line)
{
	int cnt;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " LINE OPTIONS ");

		for (cnt = 0 ; *line_table[cnt].fun != NULL ; cnt++)
		{
			if (*line_table[cnt].desc)
			{
				tintin_printf2(ses, "  [%-13s] %s", line_table[cnt].name, line_table[cnt].desc);
			}
		}
		tintin_header(ses, 80, "");

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
			show_error(ses, LIST_COMMAND, "#ERROR: #LINE {%s}: INVALID LINE OPTION.", arg1);
		}
		else
		{
			ses = line_table[cnt].fun(ses, arg, arg1, arg2, arg3);
		}
	}
	return ses;
}

DO_LINE(line_background)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {BACKGROUND} <COMMAND>");

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

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {BENCHMARK} <COMMAND>");

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
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE CAPTURE <VARIABLE> <COMMAND>");
	}
	return ses;
}

DO_LINE(line_convert)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {CONVERT} <COMMAND>");

		return ses;
	}

	// May need a clearer flag here.

	gtd->level->convert++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->convert--;

	return ses;
}

DO_LINE(line_debug)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {DEBUG} <COMMAND>");

		return ses;
	}

	gtd->level->debug++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->debug--;

	return ses;
}

DO_LINE(line_gag)
{
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

	if (ses->gagline < 0)
	{
		ses->gagline = 0;
	}
	show_debug(ses, LIST_GAG, COLOR_DEBUG "#DEBUG LINE GAG " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} " COLOR_COMMAND "[" COLOR_STRING "%d" COLOR_COMMAND "]", arg1, ses->gagline);

	return ses;
}

DO_LINE(line_ignore)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {IGNORE} <COMMAND>");
		
		return ses;
	}

	gtd->level->ignore++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->ignore--;

	return ses;
}

DO_LINE(line_local)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {LOCAL} <COMMAND>");

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
	FILE *logfile;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 && *arg2)
	{
		substitute(ses, arg2, arg2, SUB_ESC|SUB_COL|SUB_LNF);

		if (ses->log->file && !strcmp(ses->log->name, arg1))
		{
			logit(ses, arg2, ses->log->file, LOG_FLAG_NONE);
		}
		else if (ses->log->line_time == gtd->time && !strcmp(ses->log->line_name, arg1))
		{
			logit(ses, arg2, ses->log->line_file, LOG_FLAG_NONE);
		}
		else
		{
			if ((logfile = fopen(arg1, "a")))
			{
				if (ses->log->line_file)
				{
					fclose(ses->log->line_file);
				}
				free(ses->log->line_name);

				ses->log->line_name = strdup(arg1);
				ses->log->line_file = logfile;
				ses->log->line_time = gtd->time;

				logheader(ses, ses->log->line_file, LOG_FLAG_APPEND | HAS_BIT(ses->log->mode, LOG_FLAG_HTML));

				logit(ses, arg2, ses->log->line_file, LOG_FLAG_NONE);
			}
			else
			{
				show_error(ses, LIST_COMMAND, "#ERROR: #LINE LOG {%s}: COULDN'T OPEN FILE.", arg1);
			}
		}
	}
	else if (*arg1)
	{
		if (ses->log->next_time == gtd->time && !strcmp(ses->log->next_name, arg1))
		{
			SET_BIT(ses->log->mode, LOG_FLAG_NEXT);
		}
		else if ((logfile = fopen(arg1, "a")))
		{
			if (ses->log->next_file)
			{
				fclose(ses->log->next_file);
			}
			free(ses->log->next_name);

			ses->log->next_name = strdup(arg1);
			ses->log->next_file = logfile;
			ses->log->next_time = gtd->time;

			SET_BIT(ses->log->mode, LOG_FLAG_NEXT);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #LINE LOG {%s}: COULDN'T OPEN FILE.", arg1);
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {LOG} <FILENAME> [TEXT]");
	}
	return ses;
}


DO_LINE(line_logverbatim)
{
	FILE *logfile;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 && *arg2)
	{
		if (ses->log->file && !strcmp(ses->log->name, arg1))
		{
			logit(ses, arg2, ses->log->file, LOG_FLAG_LINEFEED);
		}
		else if (ses->log->line_time == gtd->time && !strcmp(ses->log->line_name, arg1))
		{
			logit(ses, arg2, ses->log->line_file, LOG_FLAG_LINEFEED|LOG_FLAG_PLAIN);
		}
		else
		{
			if ((logfile = fopen(arg1, "a")))
			{
				if (ses->log->line_file)
				{
					fclose(ses->log->line_file);
				}
				free(ses->log->line_name);

				ses->log->line_name = strdup(arg1);
				ses->log->line_file = logfile;
				ses->log->line_time = gtd->time;

				logit(ses, arg2, ses->log->line_file, LOG_FLAG_LINEFEED|LOG_FLAG_PLAIN);
			}
			else
			{
				show_error(ses, LIST_COMMAND, "#ERROR: #LINE LOGVERBATIM {%s}: COULDN'T OPEN FILE.", arg1);
			}
		}
	}
	else if (*arg1)
	{
		if (ses->log->next_time == gtd->time && !strcmp(ses->log->next_name, arg1))
		{
			SET_BIT(ses->log->mode, LOG_FLAG_NEXT);
		}
		else if ((logfile = fopen(arg1, "a")))
		{
			if (ses->log->next_file)
			{
				fclose(ses->log->next_file);
			}
			free(ses->log->next_name);

			ses->log->next_name = strdup(arg1);
			ses->log->next_file = logfile;
			ses->log->next_time = gtd->time;

			SET_BIT(ses->log->mode, LOG_FLAG_NEXT);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #LINE LOGVERBATIM {%s}: COULDN'T OPEN FILE.", arg1);
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {LOGVERBATIM} <FILENAME> <TEXT>");
	}
	return ses;
}

DO_LINE(line_logmode)
{
	struct session *active_ses;

	int old_mode = ses->log->mode;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (is_abbrev(arg1, "HTML"))
	{
		SET_BIT(ses->log->mode, LOG_FLAG_HTML);
		DEL_BIT(ses->log->mode, LOG_FLAG_PLAIN);
		DEL_BIT(ses->log->mode, LOG_FLAG_RAW);
	}
	else if (is_abbrev(arg1, "PLAIN"))
	{
		SET_BIT(ses->log->mode, LOG_FLAG_PLAIN);
		DEL_BIT(ses->log->mode, LOG_FLAG_HTML);
		DEL_BIT(ses->log->mode, LOG_FLAG_RAW);
	}
	else if (is_abbrev(arg1, "RAW"))
	{
		SET_BIT(ses->log->mode, LOG_FLAG_RAW);
		DEL_BIT(ses->log->mode, LOG_FLAG_HTML);
		DEL_BIT(ses->log->mode, LOG_FLAG_PLAIN);
	}
	else if (is_abbrev(arg1, "STAMP"))
	{
		SET_BIT(ses->log->mode, LOG_FLAG_STAMP);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {LOGMODE} {HTML|PLAIN|RAW|STAMP} <COMMAND>");

		return ses;
	}

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	active_ses = script_driver(ses, LIST_COMMAND, arg1);

	ses->log->mode = old_mode;

	return active_ses;
}

DO_LINE(line_msdp)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {MSDP} <COMMAND>");

		return ses;
	}

	tintin2msdp(arg1, arg2);

	ses = script_driver(ses, LIST_COMMAND, arg2);

	return ses;
}

DO_LINE(line_json)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {JSON} <VARIABLE> <COMMAND>");

		return ses;
	}

	char *str_sub = str_alloc_stack(0);

//	struct listroot *root = ses->list[LIST_VARIABLE];
	struct listnode *node = search_nest_node_ses(ses, arg1);

	if (node)
	{
		if (node->root)
		{
			view_nest_node_json(node, &str_sub, 0, TRUE);

//			print_lines(ses, SUB_NONE, "", "%s\n", str_sub);
		}
		else
		{
			sprintf(str_sub, "\"%s\"\n", node->arg2);
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#LINE JSON {%s}: VARIABLE NOT FOUND.", arg1);
	}

	RESTRING(gtd->cmds[0], str_sub);

	substitute(ses, arg2, str_sub, SUB_CMD);

	ses = script_driver(ses, LIST_COMMAND, str_sub);

	return ses;
}
	
DO_LINE(line_multishot)
{
	unsigned int shots;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	shots = (unsigned int) get_number(ses, arg1);

	if (!is_math(ses, arg1) || *arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {MULTISHOT} <NUMBER> <COMMAND>");
		
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
	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {ONESHOT} <COMMAND>");
		
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
	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {QUIET} <COMMAND>");
		
		return ses;
	}

	gtd->level->quiet++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->quiet--;

	return ses;
}

DO_LINE(line_strip)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_ESC|SUB_COL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {STRIP} <COMMAND>");

		return ses;
	}

	strip_vt102_codes(arg1, arg2);

	ses = script_driver(ses, LIST_COMMAND, arg2);

	return ses;
}

DO_LINE(line_substitute)
{
	int i, flags = 0;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg2 == 0)
	{
		for (i = 0 ; *substitution_table[i].name ; i++)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {SUBSTITUTE} {%s} <COMMAND>", substitution_table[i].name);
		}
		return ses;
	}

	arg = arg1;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, arg3, GET_ONE);

		for (i = 0 ; *substitution_table[i].name ; i++)
		{
			if (is_abbrev(arg3, substitution_table[i].name))
			{
				SET_BIT(flags, substitution_table[i].bitvector);
			}
		}

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}

	substitute(ses, arg2, arg3, flags);

	ses = script_driver(ses, LIST_COMMAND, arg3);

	return ses;
}

DO_LINE(line_verbatim)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {VERBATIM} <COMMAND>");
		
		return ses;
	}

	gtd->level->verbatim++;

	ses = parse_input(ses, arg1);

	gtd->level->verbatim--;

	return ses;
}

DO_LINE(line_verbose)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #LINE {VERBOSE} <COMMAND>");
		
		return ses;
	}

	gtd->level->verbose++;

	ses = script_driver(ses, LIST_COMMAND, arg1);

	gtd->level->verbose--;

	return ses;
}

	