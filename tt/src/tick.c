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
*                     recoded by Igor van den Hoven 2004                      *
******************************************************************************/


#include "tintin.h"


DO_COMMAND(do_tick)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg3 == 0)
	{
		strcpy(arg3, "60");
	}
	else
	{
		get_number_string(ses, arg3, arg3);
	}

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_TICKER], 0);
	}
	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_TICKER]) == FALSE) 
		{
			show_message(ses, LIST_TICKER, "#TICK, NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		update_node_list(ses->list[LIST_TICKER], arg1, arg2, arg3, "");

		show_message(ses, LIST_TICKER, "#OK. #TICK {%s} NOW EXECUTES {%s} EVERY {%s} SECONDS.", arg1, arg2, arg3);
	}
	return ses;
}


DO_COMMAND(do_untick)
{
	delete_node_with_wild(ses, LIST_TICKER, arg);

	return ses;
}


DO_COMMAND(do_delay)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE], temp[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_DELAY], 0);
	}
	else if (*arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_DELAY]) == FALSE)
		{
			show_message(ses, LIST_DELAY, "#DELAY: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		if (*arg3 == 0)
		{
			sprintf(arg3, "%lld", utime() + (long long) (1000000 * get_number(ses, arg1)));

			get_number_string(ses, arg1, temp);

			update_node_list(ses->list[LIST_DELAY], arg3, arg2, temp, "");

			show_message(ses, LIST_TICKER, "#OK, IN {%s} SECONDS {%s} IS EXECUTED.", temp, arg2);
		}
		else
		{
			get_number_string(ses, arg3, temp);

			update_node_list(ses->list[LIST_DELAY], arg1, arg2, temp, "");

			show_message(ses, LIST_TICKER, "#OK, IN {%s} SECONDS {%s} IS EXECUTED.", temp, arg2);
		}
	}
	return ses;
}

DO_COMMAND(do_undelay)
{
	delete_node_with_wild(ses, LIST_DELAY, arg);

	return ses;
}
