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
*                      coded by Igor van den Hoven 2006                       *
******************************************************************************/


#include "tintin.h"

DO_COMMAND(do_macro)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_MACRO], 0);
	}
	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_MACRO]) == FALSE)
		{
			show_message(ses, LIST_MACRO, "#MACRO: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		unconvert_meta(arg1, arg3);

		update_node_list(ses->list[LIST_MACRO], arg1, arg2, arg3, "");

		show_message(ses, LIST_MACRO, "#OK. MACRO {%s} HAS BEEN SET TO {%s}.", arg1, arg2);
	}
	return ses;
}


DO_COMMAND(do_unmacro)
{
	delete_node_with_wild(ses, LIST_MACRO, arg);

	return ses;
}

