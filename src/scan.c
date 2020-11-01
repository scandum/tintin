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
*                      coded by Igor van den Hoven 2005                       *
******************************************************************************/

#include "tintin.h"

#ifdef HAVE_PTY_H
  #include <pty.h>
#else
  #ifdef HAVE_UTIL_H
    #include <util.h>
  #endif
#endif
#include <dirent.h>

#define DO_SCAN(scan) struct session *scan(struct session *ses, FILE *fp, char *arg, char *arg1, char *arg2)

DO_SCAN(scan_abort);
DO_SCAN(scan_csv);
DO_SCAN(scan_dir);
DO_SCAN(scan_file);
DO_SCAN(scan_forward);
DO_SCAN(scan_tsv);
DO_SCAN(scan_txt);

#define SCAN_FLAG_NONE                0
#define SCAN_FLAG_FILE                BV01
#define SCAN_FLAG_SCAN                BV02

typedef struct session *SCAN(struct session *ses, FILE *fp, char *arg, char *arg1, char *arg2);

struct scan_type
{
	char                  * name;
	SCAN                  * fun;
	int                     flags;
	char                  * desc;
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

DO_COMMAND(do_scan)
{
	char cmd[BUFFER_SIZE];
	FILE *fp = NULL;
	int cnt;

	arg = sub_arg_in_braces(ses, arg, cmd, GET_ONE, SUB_VAR|SUB_FUN);

	if (*cmd == 0)
	{
		tintin_header(ses, 80, " SCAN OPTIONS ");

		for (cnt = 0 ; *scan_table[cnt].name != 0 ; cnt++)
		{
			tintin_printf2(ses, "  [%-13s] %s", scan_table[cnt].name, scan_table[cnt].desc);
		}
		tintin_header(ses, 80, "");

		return ses;
	}

	for (cnt = 0 ; *scan_table[cnt].name != 0 ; cnt++)
	{
		if (!is_abbrev(cmd, scan_table[cnt].name))
		{
			continue;
		}

		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

		if (HAS_BIT(scan_table[cnt].flags, SCAN_FLAG_FILE))
		{
			if (*arg1 == 0)
			{
				show_error(ses, LIST_COMMAND, "#SYNTAX: #SCAN {%s} {<FILENAME>}", scan_table[cnt].name);

				return ses;
			}

			if ((fp = fopen(arg1, "r")) == NULL)
			{
				show_error(ses, LIST_COMMAND, "#ERROR: #SCAN {%s} FILE {%s} NOT FOUND.", scan_table[cnt].name, arg1);

				return ses;
			}
		}

		if (HAS_BIT(scan_table[cnt].flags, SCAN_FLAG_SCAN))
		{
			gtd->level->scan++;
		}

		ses = scan_table[cnt].fun(ses, fp, arg, arg1, arg2);

		if (HAS_BIT(scan_table[cnt].flags, SCAN_FLAG_SCAN))
		{
			gtd->level->scan--;
		}

		if (HAS_BIT(scan_table[cnt].flags, SCAN_FLAG_FILE))
		{
			fclose(fp);
		}
		return ses;
	}

	show_error(ses, LIST_COMMAND, "\e[1;31mTHE SCAN COMMAND HAS CHANGED, EXECUTING #SCAN {TXT} {%s} INSTEAD.", cmd);

	ses = command(ses, do_scan, "{TXT} {%s}", cmd);

	return ses;
}

DO_SCAN(scan_abort)
{
	if (gtd->level->scan)
	{
		SET_BIT(ses->flags, SES_FLAG_SCANABORT);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SCAN ABORT: NOT CURRENTLY SCANNING.");
	}
	return ses;
}

/* support routines for comma separated value files */

char *get_arg_in_quotes(struct session *ses, char *string, char *result, int flag)
{
	char *pti, *pto;
	int nest = 0;

	pti = space_out(string);
	pto = result;

	if (*pti == '"')
	{
		nest = TRUE;
		pti++;
	}

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '"')
		{
			if (pti[1] == '"')
			{
				*pto++ = *pti++;
			}
			else if (nest)
			{
				nest = FALSE;
			}
			pti++;
			continue;
		}
		else if (nest == TRUE)
		{
			*pto++ = *pti++;
		}
		else if (*pti == ' ' || *pti == '\t')
		{
			pti++;
		}
		else if (*pti == ',')
		{
			pti++;
			break;
		}
		else
		{
			*pto++ = *pti++;
		}
	}

	if (nest)
	{
		tintin_printf2(ses, "#SCAN CSV: GET QUOTED ARGUMENT: UNMATCHED QUOTE.");
	}
	*pto = 0;

	return pti;
}


DO_SCAN(scan_csv)
{
	char line[STRING_SIZE];
	int i, header = FALSE;

	while (fgets(line, BUFFER_SIZE, fp))
	{
		arg = strchr(line, '\r');

		if (arg)
		{
			*arg = 0;
		}
		else
		{
		
			arg = strchr(line, '\n');

			if (arg)
			{
				*arg = 0;
			}
		}

		RESTRING(gtd->vars[0], line);

		arg = line;

		for (i = 1 ; i < 100 ; i++)
		{
			arg = get_arg_in_quotes(ses, arg, arg2, FALSE);

			RESTRING(gtd->vars[i], arg2);

			if (*arg == 0)
			{
				while (++i < 100)
				{
					if (*gtd->vars[i])
					{
						RESTRING(gtd->vars[i], "");
					}
				}
				break;
			}
		}

		if (header == FALSE)
		{
			header = TRUE;

			check_all_events(ses, SUB_SEC|EVENT_FLAG_SCAN, 0, 0, "SCAN CSV HEADER");
		}
		else
		{
			check_all_events(ses, SUB_SEC|EVENT_FLAG_SCAN, 0, 0, "SCAN CSV LINE");
		}

		if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
		{
			break;
		}
	}

	if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
	{
		DEL_BIT(ses->flags, SES_FLAG_SCANABORT);

		show_message(ses, LIST_COMMAND, "#SCAN CSV: FILE {%s} PARTIALLY SCANNED.", arg1);
	}
	else
	{
		show_message(ses, LIST_COMMAND, "#SCAN CSV: FILE {%s} SCANNED.", arg1);
	}

	return ses;
}

DO_SCAN(scan_dir)
{
	char filename[PATH_SIZE];
	struct dirent **dirlist;
	struct stat info;
	int size, index;

	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "SYNTAX: #SCAN DIR {%s} <VARIABLE NAME>", arg1);

		return ses;
	}

	set_nest_node_ses(ses, arg2, "");

	size = scandir(arg1, &dirlist, 0, NULL);

	if (size == -1)
	{
		if (stat(arg1, &info) == -1)
		{
			syserr_printf(ses, "scan_dir: stat:");
			
			return ses;
		}

		arg = arg1;

		while (strchr(arg, '\\'))
		{
			arg = strchr(arg, '\\');
		}

		add_nest_node_ses(ses, arg2, "{%s}{{FILE}{%d}{MODE}{%u}{SIZE}{%u}{TIME}{%lld}}",
			arg,
			!S_ISDIR(info.st_mode),
			info.st_mode,
			info.st_size,
			info.st_mtime);

		return ses;
	}

	for (index = 0 ; index < size ; index++)
	{
		sprintf(filename, "%s%s%s", arg1, is_suffix(arg1, "/") ? "" : "/", dirlist[index]->d_name);

		if (stat(filename, &info) == -1)
		{
			syserr_printf(ses, "scan_dir: stat:");
			
			return ses;
		}

		add_nest_node_ses(ses, arg2, "{%s}{{FILE}{%d}{MODE}{%u}{SIZE}{%u}{TIME}{%lld}}",
			dirlist[index]->d_name,
			!S_ISDIR(info.st_mode),
			info.st_mode,
			info.st_size,
			info.st_mtime);
	}

	for (index = 0 ; index < size ; index++)
	{
		free(dirlist[index]);
	}
	free(dirlist);

	show_message(ses, LIST_COMMAND, "#SCAN DIR: DIRECTORY {%s} SAVED TO {%s}.", arg1, arg2);

	return ses;
}

DO_SCAN(scan_file)
{
	char line[STRING_SIZE], *str_out, *str_rip, *str_sub;
	int cnt = 0;

	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	str_out = str_alloc_stack(0);

	while (fgets(line, BUFFER_SIZE - 1, fp))
	{
		cnt++;
		str_cat(&str_out, line);
	}

	str_rip = str_alloc_stack(str_len(str_out));

	strip_vt102_codes(str_out, str_rip);

	RESTRING(gtd->cmds[0], str_out);
	RESTRING(gtd->cmds[1], str_rip);
	RESTRING(gtd->cmds[2], ntos(str_len(str_out)));
	RESTRING(gtd->cmds[3], ntos(strlen(str_rip)));
	RESTRING(gtd->cmds[4], ntos(cnt));

	str_sub = str_alloc_stack(strlen(arg) * 2);

	substitute(ses, arg2, str_sub, SUB_CMD);

	show_message(ses, LIST_COMMAND, "#SCAN FILE: FILE {%s} SCANNED.", arg1);

	ses = script_driver(ses, LIST_COMMAND, str_sub);

	return ses;
}

DO_SCAN(scan_forward)
{
	char line[STRING_SIZE], *lnf;
	float delay = 0;

	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (!HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
	{
		show_error(ses, LIST_COMMAND, "#SCAN FORWARD: SESSION {%s} IS NOT CONNECTED.", ses->name);

		return ses;
	}

	while (fgets(line, BUFFER_SIZE - 1, fp))
	{
		lnf = strchr(line, '\n');

		if (lnf)
		{
			*lnf = 0;
		}

		if (*arg2)
		{
			delay += get_number(ses, arg2);

			command(ses, do_delay, "%.3f #send {%s}", delay, line);
		}
		else
		{
			write_mud(ses, line, SUB_EOL);
		}
	}

	show_message(ses, LIST_COMMAND, "#SCAN FORWARD: FILE {%s} FORWARDED.", arg1);

	return ses;
}

/* support routines for tab separated value files */

char *get_arg_stop_tabs(struct session *ses, char *string, char *result, int flag)
{
	char *pti, *pto;

	pti = string;
	pto = result;

	while (*pti)
	{
		if (*pti == '\t')
		{
			pti++;
			break;
		}
		*pto++ = *pti++;
	}
	*pto = 0;

	return pti;
}

DO_SCAN(scan_tsv)
{
	char line[STRING_SIZE];
	int i, header = FALSE;

	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	while (fgets(line, BUFFER_SIZE, fp))
	{
		arg = strchr(line, '\r');

		if (arg)
		{
			*arg = 0;
		}
		else
		{
		
			arg = strchr(line, '\n');

			if (arg)
			{
				*arg = 0;
			}
		}

		RESTRING(gtd->vars[0], line);

		arg = line;

		for (i = 1 ; i < 100 ; i++)
		{
			arg = get_arg_stop_tabs(ses, arg, arg2, FALSE);

			RESTRING(gtd->vars[i], arg2);

			if (*arg == 0)
			{
				while (++i < 100)
				{
					if (*gtd->vars[i])
					{
						RESTRING(gtd->vars[i], "");
					}
				}
				break;
			}
		}

		if (header == FALSE)
		{
			header = TRUE;

			check_all_events(ses, SUB_SEC|EVENT_FLAG_SCAN, 0, 0, "SCAN TSV HEADER");
		}
		else
		{
			check_all_events(ses, SUB_SEC|EVENT_FLAG_SCAN, 0, 0, "SCAN TSV LINE");
		}

		if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
		{
			break;
		}
	}

	if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
	{
		DEL_BIT(ses->flags, SES_FLAG_SCANABORT);

		show_message(ses, LIST_COMMAND, "#SCAN TSV: FILE {%s} PARTIALLY SCANNED.", arg1);
	}
	else
	{
		show_message(ses, LIST_COMMAND, "#SCAN TSV: FILE {%s} SCANNED.", arg1);
	}
	return ses;
}

/* support routines for text files */

DO_SCAN(scan_txt)
{
	char line[STRING_SIZE];

	while (fgets(line, BUFFER_SIZE - 1, fp))
	{
		arg = strchr(line, '\r');

		if (arg)
		{
			*arg = 0;
		}
		else
		{
		
			arg = strchr(line, '\n');

			if (arg)
			{
				*arg = 0;
			}
		}

		process_mud_output(ses, line, FALSE);

		if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
		{
			break;
		}
	}

	if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
	{
		DEL_BIT(ses->flags, SES_FLAG_SCANABORT);

		show_message(ses, LIST_COMMAND, "#SCAN TXT: FILE {%s} PARTIALLY SCANNED.", arg1);
	}
	else
	{
		show_message(ses, LIST_COMMAND, "#SCAN TXT: FILE {%s} SCANNED.", arg1);
	}
	return ses;
}

