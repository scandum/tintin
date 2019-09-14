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

DO_COMMAND(do_history)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int cnt;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
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

		history_table[cnt].fun(ses, arg2);

		return ses;
	}

	do_history(ses, "");

	return ses;
}


void add_line_history(struct session *ses, char *line)
{
	struct listroot *root;

	root = ses->list[LIST_HISTORY];

	if (HAS_BIT(root->flags, LIST_FLAG_IGNORE))
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
	gtd->repeat_char = *arg;

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
	add_line_history(ses, arg);
}

DO_HISTORY(history_list)
{
	struct listroot *root;
	int i, cnt = 1;

	root = ses->list[LIST_HISTORY];

	for (i = 0 ; i < root->used ; i++)
	{
		tintin_printf2(ses, "%6d - %s", cnt++, root->list[i]->arg1);
	}
	return;
}

DO_HISTORY(history_read)
{
	FILE *file;
	char *cptr, buffer[BUFFER_SIZE];

	file = fopen(arg, "r");

	if (file == NULL)
	{
		show_message(ses, LIST_HISTORY, "#HISTORY: COULDN'T OPEN FILE {%s} TO READ.", arg);
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
				insert_node_list(ses->list[LIST_HISTORY], buffer, "", "", "");
			}
		}
	}
	insert_node_list(ses->list[LIST_HISTORY], "", "", "", "");

	fclose(file);

	if (ses->list[LIST_HISTORY]->used > gtd->history_size) 
	{
		char buf[BUFFER_SIZE];

		sprintf(buf, "{HISTORY SIZE} {%d}", UMIN(ses->list[LIST_HISTORY]->used, 9999));

		do_configure(gts, buf);
	}

	return;
}

DO_HISTORY(history_size)
{
	if (atoi(arg) < 1 || atoi(arg) > 100000)
	{
		tintin_printf(ses, "#HISTORY SIZE: PROVIDE A NUMBER BETWEEN 1 and 100,000");
	}
	else
	{
		gtd->history_size = atoi(arg);
	}
	return;
}

DO_HISTORY(history_write)
{
	struct listroot *root = ses->list[LIST_HISTORY];
	FILE *file;
	int i;

	file = fopen(arg, "w");

	if (file == NULL)
	{
		tintin_printf2(ses, "#HISTORY: COULDN'T OPEN FILE {%s} TO WRITE.", arg);

		return;
	}

	for (i = 0 ; i < root->used ; i++)
	{
		fprintf(file, "%s\n", root->list[i]->arg1);
	}

	fclose(file);

	return;
}
