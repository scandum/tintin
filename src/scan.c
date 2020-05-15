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

struct session *scan_bulk_file(struct session *ses, FILE *fp, char *filename, char *arg)
{
	char line[STRING_SIZE], *str_out, *str_rip, *str_sub;
	int cnt = 0;

	str_out = str_dup("");

	while (fgets(line, BUFFER_SIZE - 1, fp))
	{
		cnt++;
		str_cat(&str_out, line);
	}

	str_rip = str_alloc(str_len(str_out));

	strip_vt102_codes(str_out, str_rip);

	RESTRING(gtd->cmds[0], str_out);
	RESTRING(gtd->cmds[1], str_rip);
	RESTRING(gtd->cmds[2], ntos(str_len(str_out)));
	RESTRING(gtd->cmds[3], ntos(strlen(str_rip)));
	RESTRING(gtd->cmds[4], ntos(cnt));

	str_sub = str_alloc(strlen(arg) + STRING_SIZE);

	substitute(ses, arg, str_sub, SUB_CMD);

	show_message(ses, LIST_COMMAND, "#SCAN BULK: FILE {%s} SCANNED.", filename);

	DEL_BIT(ses->flags, SES_FLAG_SCAN);

	ses = script_driver(ses, LIST_COMMAND, str_sub);

	return ses;
}

struct session *scan_csv_file(struct session *ses, FILE *fp, char *filename)
{
	char line[STRING_SIZE], temp[BUFFER_SIZE], *arg;
	int i, header = FALSE;

	SET_BIT(ses->flags, SES_FLAG_SCAN);

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
			arg = get_arg_in_quotes(ses, arg, temp, FALSE);

			RESTRING(gtd->vars[i], temp);

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

			check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "SCAN CSV HEADER");
		}
		else
		{
			check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "SCAN CSV LINE");
		}

		if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
		{
			break;
		}
	}

	DEL_BIT(ses->flags, SES_FLAG_SCAN);

	if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
	{
		DEL_BIT(ses->flags, SES_FLAG_SCANABORT);

		show_message(ses, LIST_COMMAND, "#SCAN CSV: FILE {%s} PARTIALLY SCANNED.", filename);
	}
	else
	{
		show_message(ses, LIST_COMMAND, "#SCAN CSV: FILE {%s} SCANNED.", filename);
	}
	fclose(fp);

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

struct session *scan_tsv_file(struct session *ses, FILE *fp, char *filename)
{
	char line[STRING_SIZE], temp[BUFFER_SIZE], *arg;
	int i, header = FALSE;

	SET_BIT(ses->flags, SES_FLAG_SCAN);

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
			arg = get_arg_stop_tabs(ses, arg, temp, FALSE);

			RESTRING(gtd->vars[i], temp);

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

			check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "SCAN TSV HEADER");
		}
		else
		{
			check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "SCAN TSV LINE");
		}

		if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
		{
			break;
		}
	}

	DEL_BIT(ses->flags, SES_FLAG_SCAN);

	if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
	{
		DEL_BIT(ses->flags, SES_FLAG_SCANABORT);

		show_message(ses, LIST_COMMAND, "#SCAN TSV: FILE {%s} PARTIALLY SCANNED.", filename);
	}
	else
	{
		show_message(ses, LIST_COMMAND, "#SCAN TSV: FILE {%s} SCANNED.", filename);
	}
	return ses;
}

/* support routines for text files */

struct session *scan_txt_file(struct session *ses, FILE *fp, char *filename)
{
	char line[STRING_SIZE], *arg;

	SET_BIT(ses->flags, SES_FLAG_SCAN);

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

	DEL_BIT(ses->flags, SES_FLAG_SCAN);

	if (HAS_BIT(ses->flags, SES_FLAG_SCANABORT))
	{
		DEL_BIT(ses->flags, SES_FLAG_SCANABORT);

		show_message(ses, LIST_COMMAND, "#SCAN TXT: FILE {%s} PARTIALLY SCANNED.", filename);
	}
	else
	{
		show_message(ses, LIST_COMMAND, "#SCAN TXT: FILE {%s} SCANNED.", filename);
	}
	return ses;
}

DO_COMMAND(do_scan)
{
	FILE *fp;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCAN {ABORT|CSV|TXT} {<FILENAME>}");

		return ses;
	}

	if (is_abbrev(arg1, "ABORT"))
	{
		if (!HAS_BIT(ses->flags, SES_FLAG_SCAN))
		{
			show_error(ses, LIST_COMMAND, "#SCAN ABORT: NOT CURRENTLY SCANNING.");
		}
		else
		{
			SET_BIT(ses->flags, SES_FLAG_SCANABORT);
		}
		return ses;
	}

	if (*arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCAN {ABORT|CSV|TXT} {<FILENAME>}");

		return ses;
	}

	if ((fp = fopen(arg2, "r")) == NULL)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #SCAN - FILE {%s} NOT FOUND.", arg2);

		return ses;
	}

	SET_BIT(ses->flags, SES_FLAG_SCAN);

	if (is_abbrev(arg1, "FILE"))
	{
		ses = scan_bulk_file(ses, fp, arg2, arg);
	}
	else if (is_abbrev(arg1, "CSV"))
	{
		ses = scan_csv_file(ses, fp, arg2);
	}
	else if (is_abbrev(arg1, "TSV"))
	{
		ses = scan_tsv_file(ses, fp, arg2);
	}
	else if (is_abbrev(arg1, "TXT"))
	{
		ses = scan_txt_file(ses, fp, arg2);
	}
	else
	{
		DEL_BIT(ses->flags, SES_FLAG_SCAN);

		show_error(ses, LIST_COMMAND, "#SYNTAX: #SCAN {ABORT|CSV|FILE|TSV|TXT} {<FILENAME>}");
	}

	DEL_BIT(ses->flags, SES_FLAG_SCAN);

	fclose(fp);

	return ses;
}
