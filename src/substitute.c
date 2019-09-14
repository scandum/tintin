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
*                         coded by Peter Unold 1992                           *
******************************************************************************/


#include "tintin.h"

DO_COMMAND(do_substitute)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE], *str;

	str = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, str, arg2, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg3 == 0)
	{
		strcpy(arg3, "5");
	}

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_SUBSTITUTE], 0);
	}
	else if (*str == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_SUBSTITUTE]) == FALSE)
		{
			show_message(ses, LIST_SUBSTITUTE, "#SUBSTITUTE: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		update_node_list(ses->list[LIST_SUBSTITUTE], arg1, arg2, arg3, "");

		show_message(ses, LIST_SUBSTITUTE, "#OK. {%s} IS NOW SUBSTITUTED AS {%s} @ {%s}.", arg1, arg2, arg3);
	}
	return ses;
}


DO_COMMAND(do_unsubstitute)
{
	delete_node_with_wild(ses, LIST_SUBSTITUTE, arg);

	return ses;
}

void check_all_substitutions(struct session *ses, char *original, char *line)
{
	char match[BUFFER_SIZE], subst[BUFFER_SIZE], output[BUFFER_SIZE], temp[BUFFER_SIZE], *ptl, *ptm, *pto;
	struct listroot *root = ses->list[LIST_SUBSTITUTE];
	struct listnode *node;
	int len;

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		if (check_one_regexp(ses, root->list[root->update], line, original, 0))
		{
			node = root->list[root->update];

			pto = original;
			ptl = line;

			*output = 0;

			do
			{
				if (*gtd->vars[0] == 0)
				{
					break;
				}

				strcpy(match, gtd->vars[0]);

				substitute(ses, node->arg2, temp, SUB_ARG);
				substitute(ses, temp, subst, SUB_VAR|SUB_FUN|SUB_COL|SUB_ESC);

				if (*node->arg1 == '~')
				{
					ptm = strstr(pto, match);

					len = strlen(match);
				}
				else
				{
					ptm = strip_vt102_strstr(pto, match, &len);

					ptl = strstr(ptl, match) + strlen(match);
				}

				*ptm = 0;

				cat_sprintf(output, "%s%s", pto, subst);

				pto = ptm + len;

				show_debug(ses, LIST_SUBSTITUTE, "#DEBUG SUBSTITUTE {%s} {%s}", node->arg1, match);
			}
			while (check_one_regexp(ses, node, ptl, pto, 0));

			strcat(output, pto);

//			substitute(ses, output, original, SUB_VAR|SUB_FUN|SUB_COL|SUB_ESC);

			strcpy(original, output);

			strip_vt102_codes(original, line);
		}
	}
}

