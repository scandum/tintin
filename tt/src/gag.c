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
*                       coded by Igor van den Hoven 2007                      *
******************************************************************************/


#include "tintin.h"

DO_COMMAND(do_gag)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_GAG], 0);
	}
	else
	{
		update_node_list(ses->list[LIST_GAG], arg1, "", "", "");

		show_message(ses, LIST_GAG, "#OK. {%s} IS NOW GAGGED.", arg1);
	}
	return ses;
}


DO_COMMAND(do_ungag)
{
	delete_node_with_wild(ses, LIST_GAG, arg);

	return ses;
}

void check_all_gags(struct session *ses, char *original, char *line)
{
	struct listroot *root = ses->list[LIST_GAG];

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		if (check_one_regexp(ses, root->list[root->update], line, original, 0))
		{
			show_debug(ses, LIST_GAG, "#DEBUG GAG {%s}", root->list[root->update]->arg1);

			SET_BIT(ses->flags, SES_FLAG_GAG);

			return;
		}
	}
}
