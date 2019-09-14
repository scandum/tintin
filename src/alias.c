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
*                   recoded by Igor van den Hoven 2009                        *
******************************************************************************/

#include "tintin.h"


DO_COMMAND(do_alias)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg3 == 0)
	{
		strcpy(arg3, "5");
	}

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_ALIAS], 0);
	}
	else if (*arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_ALIAS]) == FALSE)
		{
			show_message(ses, LIST_ALIAS, "#ALIAS: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		update_node_list(ses->list[LIST_ALIAS], arg1, arg2, arg3, "");

		show_message(ses, LIST_ALIAS, "#ALIAS {%s} NOW TRIGGERS {%s} @ {%s}.", arg1, arg2, arg3);
	}
	return ses;
}


DO_COMMAND(do_unalias)
{
	delete_node_with_wild(ses, LIST_ALIAS, arg);

	return ses;
}

int check_all_aliases(struct session *ses, char *input)
{
	struct listnode *node;
	struct listroot *root;
	char tmp[BUFFER_SIZE], line[BUFFER_SIZE], *arg;
	int i;

	root = ses->list[LIST_ALIAS];

	if (HAS_BIT(root->flags, LIST_FLAG_IGNORE))
	{
		return FALSE;
	}

	substitute(ses, input, line, SUB_VAR|SUB_FUN);

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		if (check_one_regexp(ses, root->list[root->update], line, line, PCRE_ANCHORED))
		{
			node = root->list[root->update];

			i = strlen(node->arg1);

			if (!strncmp(node->arg1, line, i))
			{
				if (line[i])
				{
					if (line[i] != ' ')
					{
						continue;
					}
					arg = &line[i + 1];
				}
				else
				{
					arg = &line[i];
				}

				RESTRING(gtd->vars[0], arg)

				for (i = 1 ; i < 100 ; i++)
				{
					arg = get_arg_in_braces(ses, arg, tmp, GET_ONE);

					RESTRING(gtd->vars[i], tmp);

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
			}

			substitute(ses, node->arg2, tmp, SUB_ARG);

			if (!strncmp(node->arg1, line, strlen(node->arg1)) && !strcmp(node->arg2, tmp) && *gtd->vars[0])
			{
				sprintf(input, "%s %s", tmp, gtd->vars[0]);
			}
			else
			{
				sprintf(input, "%s", tmp);
			}

			show_debug(ses, LIST_ALIAS, "#DEBUG ALIAS {%s} {%s}", node->arg1, gtd->vars[0]);

			return TRUE;
		}
	}
	return FALSE;
}
