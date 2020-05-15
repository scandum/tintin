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
*                        coded by Peter Unold 1992                            *
*                    recoded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"
#include <sys/stat.h>


DO_COMMAND(do_read)
{
	FILE *fp;

	sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if ((fp = fopen(arg1, "r")) == NULL)
	{
		check_all_events(ses, SUB_ARG, 0, 2, "READ ERROR", arg1, "FILE NOT FOUND.");

		tintin_printf(ses, "#READ {%s} - FILE NOT FOUND.", arg1);

		return ses;
	}

	return read_file(ses, fp, arg1);
}

struct session *read_file(struct session *ses, FILE *fp, char *filename)
{
	char *bufi, *bufo, temp[INPUT_SIZE], *pti, *pto, last = 0;
	int lvl, cnt, com, lnc, fix, ok, verbose, size;
	int counter[LIST_MAX];

	temp[0] = getc(fp);

	if (!ispunct((int) temp[0]))
	{
		check_all_events(ses, SUB_ARG, 0, 2, "READ ERROR", filename, "INVALID START OF FILE");

		tintin_printf(ses, "#ERROR: #READ {%s} - INVALID START OF FILE '%c'.", filename, temp[0]);

		fclose(fp);

		return ses;
	}

	ungetc(temp[0], fp);

	for (cnt = 0 ; cnt < LIST_MAX ; cnt++)
	{
		if (HAS_BIT(list_table[cnt].flags, LIST_FLAG_READ))
		{
			counter[cnt] = ses->list[cnt]->used;
		}
	}

	fseek(fp, 0, SEEK_END);

	size = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	if ((bufi = (char *) calloc(1, size + 2)) == NULL || (bufo = (char *) calloc(1, size + 2)) == NULL)
	{
		check_all_events(ses, SUB_ARG, 0, 2, "READ ERROR", filename, "FAILED TO ALLOCATE MEMORY");

		tintin_printf(ses, "#ERROR: #READ {%s} - FAILED TO ALLOCATE %d BYTES OF MEMORY.", filename, size + 2);

		fclose(fp);

		return ses;
	}


	fread(bufi, 1, size, fp);

	pti = bufi;
	pto = bufo;

	lvl = com = lnc = fix = ok = 0;

	while (*pti)
	{
		if (com == 0)
		{
			if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
			{
				*pto++ = *pti++;
				*pto++ = *pti++;

				continue;
			}

			switch (*pti)
			{
				case DEFAULT_OPEN:
					*pto++ = *pti++;
					lvl++;
					last = DEFAULT_OPEN;
					break;

				case DEFAULT_CLOSE:
					*pto++ = *pti++;
					lvl--;
					last = DEFAULT_CLOSE;
					break;

				case COMMAND_SEPARATOR:
					*pto++ = *pti++;
					last = COMMAND_SEPARATOR;
					break;

				case ' ':
					*pto++ = *pti++;
					break;

				case '/':
					if (lvl == 0 && pti[1] == '*')
					{
						pti += 2;
						com += 1;
					}
					else
					{
						*pto++ = *pti++;
					}
					break;

				case '\t':
					*pto++ = *pti++;
					break;

				case '\r':
					pti++;
					break;

				case '\n':
					lnc++;

					pto--;

					while (isspace((int) *pto))
					{
						pto--;
					}

					pto++;

					if (fix == 0 && pti[1] == gtd->tintin_char)
					{
						if (lvl == 0)
						{
							ok = lnc + 1;
						}
						else
						{
							fix = lnc;
						}
					}

					if (lvl)
					{
						pti++;

						while (isspace((int) *pti))
						{
							if (*pti == '\n')
							{
								lnc++;

								if (fix == 0 && pti[1] == gtd->tintin_char)
								{
									fix = lnc;
								}
							}
							pti++;

						}

						if (last == 0)
						{
							if (*pti != DEFAULT_OPEN && *pti != DEFAULT_CLOSE)
							{
								show_error(ses, LIST_COMMAND, "#WARNING: #READ {%s} MISSING SEMICOLON ON LINE %d.", filename, lnc - 1);
							}
						}

						if (*pti != DEFAULT_CLOSE && last == 0)
						{
							*pto++ = ' ';
						}
					}
					else for (cnt = 1 ; ; cnt++)
					{
						if (pti[cnt] == 0)
						{
							*pto++ = *pti++;
							break;
						}

						if (pti[cnt] == DEFAULT_OPEN)
						{
							pti++;
							while (isspace((int) *pti))
							{
								pti++;
							}
							*pto++ = ' ';
							break;
						}

						if (!isspace((int) pti[cnt]))
						{
							*pto++ = *pti++;
							break;
						}
					}
					break;

				default:
					*pto++ = *pti++;
					last = 0;
					break;
			}
		}
		else
		{
			switch (*pti)
			{
				case '/':
					if (pti[1] == '*')
					{
						pti += 2;
						com += 1;
					}
					else
					{
						pti += 1;
					}
					break;

				case '*':
					if (pti[1] == '/')
					{
						pti += 2;
						com -= 1;
					}
					else
					{
						pti += 1;
					}
					break;

				case '\n':
					lnc++;
					pti++;
					break;

				default:
					pti++;
					break;
			}
		}
	}
	*pto++ = '\n';
	*pto   = '\0';


	if (lvl)
	{
		check_all_events(ses, SUB_ARG, 0, 2, "READ ERROR", filename, "MISSING BRACE OPEN OR CLOSE");

		tintin_printf(ses, "#ERROR: #READ {%s} - MISSING %d '%c' BETWEEN LINE %d AND %d.", filename, abs(lvl), lvl < 0 ? DEFAULT_OPEN : DEFAULT_CLOSE, fix == 0 ? 1 : ok, fix == 0 ? lnc + 1 : fix);

		fclose(fp);

		free(bufi);
		free(bufo);

		return ses;
	}

	if (com)
	{
		check_all_events(ses, SUB_ARG, 0, 2, "READ ERROR", filename, "MISSING COMMENT OPEN OR CLOSE");

		tintin_printf(ses, "#ERROR: #READ {%s} - MISSING %d '%s'", filename, abs(com), com < 0 ? "/*" : "*/");

		fclose(fp);

		free(bufi);
		free(bufo);

		return ses;
	}

	sprintf(temp, "#CONFIG {TINTIN CHAR} {%c}", bufo[0]);

	if (bufo[0] != gtd->tintin_char)
	{
		gtd->level->verbose++;
		gtd->level->debug++;

		show_error(ses, LIST_COMMAND, "\e[1;5;31mWARNING: SETTING THE COMMAND CHARACTER TO '%c' BECAUSE IT'S THE FIRST CHARACTER IN THE FILE.", bufo[0]);

		gtd->level->debug--;
		gtd->level->verbose--;

		gtd->level->quiet++;

		script_driver(ses, LIST_COMMAND, temp);

		gtd->level->quiet--;
	}

	verbose = HAS_BIT(ses->flags, SES_FLAG_VERBOSE) ? 1 : 0;

	gtd->level->input++;

	gtd->level->verbose += verbose;

	lvl = 0;
	lnc = 0;
	pti = bufo;
	pto = bufi;

	while (*pti)
	{
		if (*pti != '\n')
		{
			*pto++ = *pti++;
			continue;
		}
		lnc++;
		*pto = 0;

		if (pto - bufi >= BUFFER_SIZE)
		{
			show_debug(ses, LIST_COMMAND, "#WARNING: #READ {%s} - POSSIBLE BUFFER OVERFLOW AT COMMAND: %.30s", filename, bufi);
		}

		if (bufi[0])
		{
			ses = script_driver(ses, LIST_COMMAND, bufi);
		}
		pto = bufi;
		pti++;
	}

	gtd->level->verbose -= verbose;

	gtd->level->input--;

	if (!HAS_BIT(ses->flags, SES_FLAG_VERBOSE))
	{
		for (cnt = 0 ; cnt < LIST_MAX ; cnt++)
		{
			if (HAS_BIT(list_table[cnt].flags, LIST_FLAG_READ))
			{
				switch (ses->list[cnt]->used - counter[cnt])
				{
					case 0:
						break;

					case 1:
						show_message(ses, cnt, "#OK: %3d %s LOADED.", ses->list[cnt]->used - counter[cnt], list_table[cnt].name);
						break;

					default:
						show_message(ses, cnt, "#OK: %3d %s LOADED.", ses->list[cnt]->used - counter[cnt], list_table[cnt].name_multi);
						break;
				}
			}
		}
	}
	fclose(fp);

	free(bufi);
	free(bufo);

	return ses;
}


DO_COMMAND(do_write)
{
	FILE *file;
	struct listroot *root;
	struct listnode *node;

	int i, j, fix, cnt = 0;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

	if (*arg1 == 0)
	{
		check_all_events(ses, SUB_ARG, 0, 2, "WRITE ERROR", arg1, "INVALID FILE NAME");

		tintin_printf2(ses, "#SYNTAX: #WRITE {<filename>} {[FORCE]}");

		return ses;
	}
	
	if (!str_suffix(arg1, ".map") && !is_abbrev(arg2, "FORCE"))
	{
		check_all_events(ses, SUB_ARG, 0, 2, "WRITE ERROR", arg1, "INVALID FILE EXTENSION");
		tintin_printf2(ses, "#WRITE {%s}: USE {FORCE} TO OVERWRITE .map FILES.", arg1);

		return ses;
	}

	if ((file = fopen(arg1, "w")) == NULL)
	{
		check_all_events(ses, SUB_ARG, 0, 2, "WRITE ERROR", arg1, "FAILED TO OPEN");

		tintin_printf(ses, "#ERROR: #WRITE: COULDN'T OPEN {%s} TO WRITE.", arg1);

		return ses;
	}

	for (i = 0 ; i < LIST_MAX ; i++)
	{
		root = ses->list[i];

		if (!HAS_BIT(root->flags, LIST_FLAG_WRITE))
		{
			continue;
		}

		fix = 0;

		for (j = 0 ; j < root->used ; j++)
		{
			node = root->list[j];

			if (node->shots)//HAS_BIT(node->flags, NODE_FLAG_ONESHOT))
			{
				continue;
			}

			if (*node->group == 0)
			{
				write_node(ses, i, node, file);

				cnt++;
				fix++;
			}
		}

		if (fix)
		{
			fputs("\n", file);
		}
	}

	fclose(file);

	show_message(ses, LIST_COMMAND, "#WRITE: %d COMMANDS WRITTEN TO {%s}.", cnt, arg1);

	return ses;
}


void write_node(struct session *ses, int list, struct listnode *node, FILE *file)
{
	char *result, *str;

	int llen = UMAX(20, strlen(node->arg1));
	int rlen = UMAX(25, strlen(node->arg2));

	push_call("write_node(%d,%p,%p)",list,node,file);

	switch (list)
	{
		case LIST_EVENT:
		case LIST_FUNCTION:
		case LIST_MACRO:
			asprintf(&result, "%c%s {%s}\n{\n%s\n}\n\n", gtd->tintin_char, list_table[list].name, node->arg1, script_writer(ses, node->arg2));
			break;

		case LIST_ACTION:
		case LIST_ALIAS:
		case LIST_BUTTON:
			if (!strcmp(node->arg3, "5"))
			{
				asprintf(&result, "%c%s {%s}\n{\n%s\n}\n\n", gtd->tintin_char, list_table[list].name, node->arg1, script_writer(ses, node->arg2));
			}
			else
			{
				asprintf(&result, "%c%s {%s}\n{\n%s\n}\n{%s}\n\n", gtd->tintin_char, list_table[list].name, node->arg1, script_writer(ses, node->arg2), node->arg3);
			}
			break;

		case LIST_VARIABLE:
			str = str_dup("");

			show_nest_node(node, &str, 1);

			asprintf(&result, "%c%-16s {%s} %*s {%s}\n", gtd->tintin_char, list_table[list].name, node->arg1, 20 - llen, "", str);

			str_free(str);

			break;

		default:
			switch (list_table[list].args)
			{
				case 0:
					result = strdup("");
					break;
				case 1:
					asprintf(&result, "%c%-16s {%s}\n", gtd->tintin_char, list_table[list].name, node->arg1);
					break;
				case 2:
					asprintf(&result, "%c%-16s {%s} %*s {%s}\n", gtd->tintin_char, list_table[list].name, node->arg1, 20 - llen, "", node->arg2);
					break;
				case 3:
					asprintf(&result, "%c%-16s {%s} %*s {%s} %*s {%s}\n", gtd->tintin_char, list_table[list].name, node->arg1, 20 - llen, "", node->arg2, 25 - rlen, "", node->arg3);
					break;
				case 4:
					asprintf(&result, "%c%-16s {%s} %*s {%s} %*s {%s} {%s}\n", gtd->tintin_char, list_table[list].name, node->arg1, 20 - llen, "", node->arg2, 25 - rlen, "", node->arg3, node->arg4);
					break;
			}
			break;
	}
	fputs(result, file);

	free(result);

	pop_call();
	return;
}

