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

DO_COMMAND(do_action)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg3 == 0)
	{
		strcpy(arg3, "5");
	}

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_ACTION], 0);
	}
	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_ACTION]) == FALSE)
		{
			show_message(ses, LIST_ACTION, "#ACTION: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		update_node_list(ses->list[LIST_ACTION], arg1, arg2, arg3, "");

		show_message(ses, LIST_ACTION, "#OK. #ACTION {%s} NOW TRIGGERS {%s} @ {%s}.", arg1, arg2, arg3);
	}
	return ses;
}


DO_COMMAND(do_unaction)
{
	delete_node_with_wild(ses, LIST_ACTION, arg);

	return ses;
}


void check_all_actions(struct session *ses, char *original, char *line)
{
	struct listroot *root = ses->list[LIST_ACTION];
	char buf[BUFFER_SIZE];

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		if (check_one_regexp(ses, root->list[root->update], line, original, 0))
		{
			show_debug(ses, LIST_ACTION, "#DEBUG ACTION {%s}", root->list[root->update]->arg1);

			substitute(ses, root->list[root->update]->arg2, buf, SUB_ARG|SUB_SEC);

			script_driver(ses, LIST_ACTION, buf);

			return;
		}
	}
}
