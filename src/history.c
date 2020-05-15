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
*                      coded by Igor van den Hoven 2006                       *
******************************************************************************/

#include "tintin.h"

DO_COMMAND(do_history)
{
	int cnt;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg4, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		info:

		tintin_header(ses, " HISTORY COMMANDS ");

		for (cnt = 0 ; *history_table[cnt].name != 0 ; cnt++)
		{
			tintin_printf2(ses, "  [%-13s] %s", history_table[cnt].name, history_table[cnt].desc);
		}
		tintin_header(ses, "");

		return ses;
	}

	for (cnt = 0 ; *history_table[cnt].name ; cnt++)
	{
		if (!is_abbrev(arg1, history_table[cnt].name))
		{
			continue;
		}

		history_table[cnt].fun(ses, arg2, arg3, arg4);

		return ses;
	}

	goto info;

	return ses;
}


void add_line_history(struct session *ses, char *line)
{
	struct listroot *root;

	root = ses->list[LIST_HISTORY];

	if (HAS_BIT(root->flags, LIST_FLAG_IGNORE) || gtd->level->ignore)
	{
		return;
	}

	if (*line == 0)
	{
		if (root->used && HAS_BIT(ses->flags, SES_FLAG_REPEATENTER))
		{
			strcpy(line, root->list[root->used - 1]->arg1);
		}
		return;
	}

	if (*line == gtd->repeat_char)
	{
		search_line_history(ses, line);
	}

	update_node_list(ses->list[LIST_HISTORY], line, "", "", "");

	while (root->used > gtd->history_size)
	{
		delete_index_list(ses->list[LIST_HISTORY], 0);
	}

	return;
}

void search_line_history(struct session *ses, char *line)
{
	struct listroot *root;
	int i;

	root = ses->list[LIST_HISTORY];

	for (i = root->used - 1 ; i >= 0 ; i--)
	{
		if (!strncmp(root->list[i]->arg1, &line[1], strlen(&line[1])))
		{
			strcpy(line, root->list[i]->arg1);

			return;
		}
	}
	tintin_printf2(ses, "#REPEAT: NO MATCH FOUND FOR '%s'", line);
}

DO_HISTORY(history_character)
{
	gtd->repeat_char = *arg1;

	show_message(ses, LIST_HISTORY, "#HISTORY CHARACTER SET TO {%c}.", gtd->repeat_char);
}

DO_HISTORY(history_delete)
{
	if (ses->list[LIST_HISTORY]->used)
	{
		delete_index_list(ses->list[LIST_HISTORY], ses->list[LIST_HISTORY]->used - 1);
	}

	return;
}

DO_HISTORY(history_insert)
{
	add_line_history(ses, arg1);
}

DO_HISTORY(history_get)
{
	struct listroot *root = ses->list[LIST_HISTORY];
	int cnt, min, max;

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #HISTORY GET <VARIABLE> [LOWER BOUND] [UPPER BOUND]");

		return;
	}

	min = get_number(ses, arg2);

	if (min < 0)
	{
		min = root->used + min;
	}

	min = URANGE(0, min, root->used - 1);

	if (*arg3 == 0)
	{
		set_nest_node_ses(ses, arg1, "%s", root->list[min]->arg1);

		return;
	}

	max = get_number(ses, arg3);

	if (max < 0)
	{
		max = root->used + max;
	}
	max = URANGE(0, max, root->used - 1);

	if (min > max)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #HISTORY GET {%s} {%d} {%d} LOWER BOUND EXCEEDS UPPER BOUND.", arg1, min, max);

		return;
	}

	cnt = 0;

	set_nest_node_ses(ses, arg1, "");

	while (min <= max)
	{
		sprintf(arg2, "%s[%d]", arg1, ++cnt);

		set_nest_node_ses(ses, arg2, "%s", root->list[min++]->arg1);
	}

	show_message(ses, LIST_COMMAND, "#HISTORY GET: %d LINES SAVED TO {%s}.", cnt, arg1);

	return;
}

DO_HISTORY(history_list)
{
	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_HISTORY], 0);
	}
	else
	{
		show_node_with_wild(ses, arg1, ses->list[LIST_HISTORY]);
	}
	return;
}

DO_HISTORY(history_read)
{
	FILE *file;
	char *cptr, buffer[BUFFER_SIZE];

	file = fopen(arg1, "r");

	if (file == NULL)
	{
		show_message(ses, LIST_HISTORY, "#HISTORY: COULDN'T OPEN FILE {%s} TO READ.", arg1);
		return;
	}

	kill_list(ses->list[LIST_HISTORY]);

	while (fgets(buffer, BUFFER_SIZE-1, file))
	{
		cptr = strchr(buffer, '\n');

		if (cptr)
		{
			*cptr = 0;

			if (*buffer)
			{
				create_node_list(ses->list[LIST_HISTORY], buffer, "", "", "");
			}
		}
	}
	create_node_list(ses->list[LIST_HISTORY], "", "", "", "");

	fclose(file);

	if (ses->list[LIST_HISTORY]->used > gtd->history_size) 
	{
		execute(gts, "#CONFIG {HISTORY SIZE} {%d}", UMIN(ses->list[LIST_HISTORY]->used, 9999));
	}

	return;
}

DO_HISTORY(history_size)
{
	if (atoi(arg1) < 1 || atoi(arg1) > 100000)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #HISTORY SIZE: PROVIDE A NUMBER BETWEEN 1 and 100,000");
	}
	else
	{
		gtd->history_size = atoi(arg1);
	}
	return;
}

DO_HISTORY(history_write)
{
	struct listroot *root = ses->list[LIST_HISTORY];
	FILE *file;
	int i;

	file = fopen(arg1, "w");

	if (file == NULL)
	{
		tintin_printf2(ses, "#HISTORY: COULDN'T OPEN FILE {%s} TO WRITE.", arg1);

		return;
	}

	for (i = 0 ; i < root->used ; i++)
	{
		fprintf(file, "%s\n", root->list[i]->arg1);
	}

	fclose(file);

	return;
}
