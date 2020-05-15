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


DO_COMMAND(do_path)
{
	int cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		info:

		tintin_header(ses, " PATH OPTIONS ");

		for (cnt = 0 ; *path_table[cnt].fun != NULL ; cnt++)
		{
			if (*path_table[cnt].desc)
			{
				tintin_printf2(ses, "  [%-13s] %s", path_table[cnt].name, path_table[cnt].desc);
			}
		}
		tintin_header(ses, "");

		return ses;
	}
	else
	{
		for (cnt = 0 ; *path_table[cnt].name ; cnt++)
		{
			if (is_abbrev(arg1, path_table[cnt].name))
			{
				break;
			}
		}

		if (*path_table[cnt].name == 0)
		{
			goto info;
		}
		else
		{
			path_table[cnt].fun(ses, arg);
		}
	}
	return ses;
}


DO_PATH(path_create)
{
	struct listroot *root = ses->list[LIST_PATH];

	kill_list(root);

	root->update = 0;

	show_message(ses, LIST_COMMAND, "#PATH CREATE: YOU START MAPPING A NEW PATH.");

	SET_BIT(ses->flags, SES_FLAG_PATHMAPPING);
}


DO_PATH(path_destroy)
{
	struct listroot *root = ses->list[LIST_PATH];

	kill_list(root);

	root->update = 0;

	DEL_BIT(ses->flags, SES_FLAG_PATHMAPPING);

	show_message(ses, LIST_COMMAND, "#PATH DESTROY: PATH DESTROYED.");
}


DO_PATH(path_start)
{
	if (HAS_BIT(ses->flags, SES_FLAG_PATHMAPPING))
	{
		show_message(ses, LIST_COMMAND, "#PATH START: ERROR: YOU ARE ALREADY MAPPING A PATH.");
	}
	else
	{
		SET_BIT(ses->flags, SES_FLAG_PATHMAPPING);

		show_message(ses, LIST_COMMAND, "#PATH START: YOU START MAPPING A PATH.");
	}
}

DO_PATH(path_stop)
{
	int index;
	struct listroot *root = ses->list[LIST_PATH];

	if (HAS_BIT(ses->flags, SES_FLAG_PATHMAPPING))
	{
		show_message(ses, LIST_COMMAND, "#PATH STOP: YOU STOP MAPPING A PATH.");

		DEL_BIT(ses->flags, SES_FLAG_PATHMAPPING);
	}
	else
	{
		if (root->list[root->update]->val64)
		{
			for (index = 0 ; index < root->used ; index++)
			{
				root->list[index]->val64 = 0;
			}
			show_message(ses, LIST_COMMAND, "#PATH STOP: YOU STOP RUNNING A PATH.");
		}
		else
		{
			show_message(ses, LIST_COMMAND, "#PATH STOP: YOU ARE NOT MAPPING OR RUNNING A PATH.");
		}
	}
}


DO_PATH(path_describe)
{
	char *dirs[] = {
		"west", "west-southwest", "southwest", "south-southwest",
		"south", "south-southeast", "southeast", "east-southeast",
		"east", "east-northeast", "northeast", "north-northeast",
		"north", "north-northwest", "northwest", "west-northwest",
		"west" };

	char *slopes[] = {
		"a steep upward slope", "a steady upward slope", "a slight upward slope",
		"the same level",
		"a slight downward slope", "a steady downward slope", "a steep downward slope" };

	struct listroot *root = ses->list[LIST_PATH];
	struct listnode *node;
	int a, d, i, s, x, y, z;

	i = x = y = z = 0;

	i = root->update;

	while (i < root->used)
	{
		node = search_node_list(ses->list[LIST_PATHDIR], root->list[i++]->arg1);

		if (node)
		{
			x += (HAS_BIT(atoi(node->arg3), MAP_EXIT_E) ? 1 : HAS_BIT(atoi(node->arg3), MAP_EXIT_W) ? -1 : 0);
			y += (HAS_BIT(atoi(node->arg3), MAP_EXIT_N) ? 1 : HAS_BIT(atoi(node->arg3), MAP_EXIT_S) ? -1 : 0);
			z += (HAS_BIT(atoi(node->arg3), MAP_EXIT_U) ? 1 : HAS_BIT(atoi(node->arg3), MAP_EXIT_D) ? -1 : 0);
		}
	}

	a = sqrt(x * x + y * y);

	a = sqrt(a * a + z * z);

	d = round(16 * (atan2(y, x) + M_PI) / (M_PI * 2));

	s = round(12 * (atan2(a, z) - M_PI / 4) / M_PI);

	if (x == 0 && y == 0)
	{
		if (z == 0)
		{
			if (root->used > 2)
			{
				tintin_printf2(ses, "The path is %d rooms long and leads back to where you are at.", root->used);
			}
			else
			{
				tintin_printf2(ses, "The path is %d rooms long and the destination is right where you are at.", root->used);
			}
		}
		else
		{
			tintin_printf2(ses, "The path is %d rooms long and the destination lies %d rooms %s of you.", root->used, abs(z), z < 0 ? "below" : "above", s);
		}
	}
	else
	{
		tintin_printf2(ses, "The path is %d rooms long and the destination lies %d rooms to the %s of you at %s.", root->used, a, dirs[d], slopes[s]);
	}

	if (root->update == 0)
	{
		tintin_printf2(ses, "You are at the start of the path.");
	}
	else if (root->update == root->used)
	{
		tintin_printf2(ses, "You are at the end of the path.");
	}
	else
	{
		tintin_printf2(ses, "You've traversed %d out of %d steps of the path.", root->update, root->used);
	}
}

DO_PATH(path_map)
{
	struct listroot *root = ses->list[LIST_PATH];
	char buf[BUFFER_SIZE];
	int i = 0;

	if (root->used == 0)
	{
		show_message(ses, LIST_COMMAND, "#PATH MAP: EMPTY PATH.");
	}
	else
	{
		sprintf(buf, "%-7s", "#PATH:");

		for (i = 0 ; i < root->update ; i++)
		{
			cat_sprintf(buf, " %s", root->list[i]->arg1);
		}

		if (i != root->used)
		{
			cat_sprintf(buf, " [%s]", root->list[i++]->arg1);

			for (i = root->update + 1 ; i < root->used ; i++)
			{
				cat_sprintf(buf, " %s", root->list[i]->arg1);
			}
		}

		if (root->update == root->used)
		{
			cat_sprintf(buf, " [ ]");
		}

		tintin_puts2(ses, buf);
	}
}

DO_PATH(path_get)
{
	struct listroot *root = ses->list[LIST_PATH];
	char result[STRING_SIZE], arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg2 == 0)
	{
		tintin_printf2(ses, "  length: %d", root->used);
		tintin_printf2(ses, "position: %d", root->update + 1);
	}
	else if (is_abbrev(arg1, "LENGTH"))
	{
		sprintf(result, "%d", root->used);

		set_nest_node_ses(ses, arg2, "%s", result);

		show_message(ses, LIST_COMMAND, "#PATH GET: PATH LENGTH {%s} SAVED TO {%s}", result, arg2);
	}
	else if (is_abbrev(arg1, "POSITION"))
	{
		sprintf(result, "%d", root->update + 1);

		set_nest_node_ses(ses, arg2, "%s", result);

		show_message(ses, LIST_COMMAND, "#PATH GET: PATH POSITION {%s} SAVED TO {%s}", result, arg2);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #PATH GET <LENGTH|POSITION> <VARIABLE NAME>");
	}
}

DO_PATH(path_save)
{
	struct listroot *root = ses->list[LIST_PATH];
	char result[STRING_SIZE], arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int i;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (root->used == 0)
	{
		tintin_puts2(ses, "#PATH SAVE: LOAD OR CREATE A PATH FIRST.");
	}
	else if (*arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #PATH SAVE <BACKWARD|FORWARD> <VARIABLE NAME>");
	}
	else if (is_abbrev(arg1, "BACKWARDS"))
	{
		result[0] = 0;

		for (i = root->used - 1 ; i >= 0 ; i--)
		{
			strcat(result, root->list[i]->arg2);

			if (i != 0)
			{
				cat_sprintf(result, "%c", COMMAND_SEPARATOR);
			}
		}
		set_nest_node_ses(ses, arg2, "%s", result);

		show_message(ses, LIST_COMMAND, "#PATH SAVE: BACKWARD PATH SAVED TO {%s}", arg2);
	}
	else if (is_abbrev(arg1, "FORWARDS"))
	{
		result[0] = 0;

		for (i = 0 ; i < root->used ; i++)
		{
			strcat(result, root->list[i]->arg1);

			if (i != root->used - 1)
			{
				cat_sprintf(result, "%c", COMMAND_SEPARATOR);
			}
		}
		set_nest_node_ses(ses, arg2, "%s", result);

		show_message(ses, LIST_COMMAND, "#PATH SAVE: FORWARD PATH SAVED TO {%s}", arg2);
	}
	else if (is_abbrev(arg1, "LENGTH"))
	{
		sprintf(result, "%d", root->used);

		set_nest_node_ses(ses, arg2, "%s", result);

		show_message(ses, LIST_COMMAND, "#PATH SAVE: PATH LENGTH SAVED TO {%s}", arg2);
	}
	else if (is_abbrev(arg1, "POSITION"))
	{
		sprintf(result, "%d", root->update + 1);

		set_nest_node_ses(ses, arg2, "%s", result);

		show_message(ses, LIST_COMMAND, "#PATH SAVE: PATH POSITION SAVED TO {%s}", arg2);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #PATH SAVE <BACKWARD|FORWARD> <VARIABLE NAME>");
	}
}


DO_PATH(path_load)
{
	struct listroot *root = ses->list[LIST_PATH];
	char arg1[BUFFER_SIZE], temp[BUFFER_SIZE];
	struct listnode *node;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if ((node = search_nest_node_ses(ses, arg1)) == NULL)
	{
		arg = arg1;
	}
	else
	{
		arg = node->arg2;
	}

	kill_list(root);

	root->update = 0;

	while (*arg)
	{
		if (*arg == ';')
		{
			arg++;
		}

		arg = get_arg_in_braces(ses, arg, temp, GET_ALL);

		if ((node = search_node_list(root, temp)))
		{
			create_node_list(root, node->arg1, node->arg2, "0", "");
		}
		else
		{
			create_node_list(root, temp, temp, "0", "");
		}
	}
	show_message(ses, LIST_COMMAND, "#PATH LOAD: PATH WITH %d NODES LOADED.", root->used);
}

DO_PATH(path_delete)
{
	struct listroot *root = ses->list[LIST_PATH];

	if (root->used)
	{
		show_message(ses, LIST_COMMAND, "#PATH DELETE: DELETED MOVE {%s}.", root->list[root->used - 1]->arg1);

		delete_index_list(root, root->used - 1);

		if (root->update >= root->used)
		{
			root->update--;
		}
	}
	else
	{
		tintin_puts(ses, "#PATH DELETE: NO MOVES LEFT.");
	}

}

DO_PATH(path_insert)
{
	struct listroot *root = ses->list[LIST_PATH];
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 && *arg2 == 0)
	{
		show_message(ses, LIST_COMMAND, "#PATH INSERT: ERROR: YOU MUST GIVE A COMMAND TO INSERT");
	}
	else
	{
		create_node_list(root, arg1, arg2, "0", "");

		show_message(ses, LIST_COMMAND, "#PATH INSERT: FORWARD {%s} BACKWARD {%s}.", arg1, arg2);

		if (HAS_BIT(ses->flags, SES_FLAG_PATHMAPPING))
		{
			root->update = root->used;
		}
	}
}


DO_PATH(path_run)
{
	int index;
	struct listroot *root = ses->list[LIST_PATH];
	char arg1[BUFFER_SIZE];
	long long total, delay;

	push_call("path_run(%p,%p)",ses,arg);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (root->update == root->used)
	{
		tintin_puts(ses, "#PATH RUN: #END OF PATH.");
	}
	else
	{
		if (*arg1)
		{
		
			delay = (long long) (get_number(ses, arg1) * 1000000.0);
			total = 0;

			DEL_BIT(ses->flags, SES_FLAG_PATHMAPPING);

			for (index = root->update ; index < root->used ; index++)
			{
				root->list[index]->val64 = gtd->utime + total;

				total += delay;
			}
		}
		else
		{
			while (root->update < root->used)
			{
				script_driver(ses, LIST_COMMAND, root->list[root->update++]->arg1);
			}
		}
	}

	pop_call();
	return;
}


DO_PATH(path_walk)
{
	struct listroot *root = ses->list[LIST_PATH];
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	DEL_BIT(ses->flags, SES_FLAG_PATHMAPPING);

	if (is_abbrev(arg1, "BACKWARDS") || is_abbrev(arg1, "-1"))
	{
		if (root->update == 0)
		{
			tintin_puts(ses, "#PATH WALK: #START OF PATH.");
		}
		else
		{
			script_driver(ses, LIST_COMMAND, root->list[--root->update]->arg2);

			if (root->update == 0)
			{
				check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "START OF PATH");
			}
		}
	}
	else if (*arg1 == 0 || is_abbrev(arg1, "FORWARDS") || is_abbrev(arg1, "+1"))
	{
		if (root->update == root->used)
		{
			tintin_puts(ses, "#PATH WALK: #END OF PATH.");
		}
		else
		{
			script_driver(ses, LIST_COMMAND, root->list[root->update++]->arg1);

			if (root->update == root->used)
			{
				check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "END OF PATH");
			}
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #PATH WALK {FORWARD|BACKWARD}.");
	}
}


DO_PATH(path_swap)
{
	struct listroot *root = ses->list[LIST_PATH];
	struct listnode *node;
	int a, z;

	if (root->used == 0)
	{
		show_error(ses, LIST_COMMAND, "#PATH SWAP: ERROR: PATH IS EMPTY.");

		return;
	}

	a = 0;
	z = root->used - 1;

	if (root->update)
	{
		root->update = root->used - root->update;
	}

	while (z > a)
	{
		arg = root->list[z]->arg1;
		root->list[z]->arg1 = root->list[z]->arg2;
		root->list[z]->arg2 = arg;

		arg = root->list[a]->arg1;
		root->list[a]->arg1 = root->list[a]->arg2;
		root->list[a]->arg2 = arg;

		node = root->list[z];
		root->list[z--] = root->list[a];
		root->list[a++] = node;
	}

	if (z == a)
	{
		arg = root->list[z]->arg1;
		root->list[z]->arg1 = root->list[z]->arg2;
		root->list[z]->arg2 = arg;
	}

	show_message(ses, LIST_COMMAND, "#PATH SWAP: PATH HAS BEEN SWAPPED.");
}


DO_PATH(path_zip)
{
	struct listroot *root = ses->list[LIST_PATH];
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int i, cnt;

	cnt   =  1;

	*arg1 =  0;
	*arg2 = 0;

	for (i = 0 ; i < root->used ; i++)
	{
		if (search_node_list(root, root->list[i]->arg1) == NULL || strlen(root->list[i]->arg1) != 1)
		{
			if (i && search_node_list(root, root->list[i - 1]->arg1) != NULL && strlen(root->list[i - 1]->arg1) == 1)
			{
				cat_sprintf(arg1, "%c", COMMAND_SEPARATOR);
			}
			cat_sprintf(arg1, "%s", root->list[i]->arg1);

			if (i < root->used - 1)
			{
				cat_sprintf(arg1, "%c", COMMAND_SEPARATOR);
			}
			continue;
		}

		if (i < root->used - 1 && !strcmp(root->list[i]->arg1, root->list[i + 1]->arg1))
		{
			cnt++;
		}
		else
		{
			cat_sprintf(arg1, "%d%s", cnt, root->list[i]->arg1);

			cnt = 1;
		}
	}

	for (i = root->used - 1 ; i >= 0 ; i--)
	{
		if (search_node_list(root, root->list[i]->arg2) == NULL || strlen(root->list[i]->arg2) != 1)
		{
			if (i != root->used - 1 && search_node_list(root, root->list[i + 1]->arg2) != NULL && strlen(root->list[i + 1]->arg2) == 1)
			{
				cat_sprintf(arg2, "%c", COMMAND_SEPARATOR);
			}
			cat_sprintf(arg2, "%s", root->list[i]->arg2);

			if (i > 0)
			{
				cat_sprintf(arg2, "%c", COMMAND_SEPARATOR);
			}
			continue;
		}

		if (i > 0 && !strcmp(root->list[i]->arg2, root->list[i - 1]->arg2))
		{
			cnt++;
		}
		else
		{
			cat_sprintf(arg2, "%d%s", cnt, root->list[i]->arg2);
			cnt = 1;
		}
	}

	root->update = root->used;

	kill_list(root);

	create_node_list(root, arg1, arg2, "0", "");

	show_message(ses, LIST_COMMAND, "#PATH ZIP: THE PATH HAS BEEN ZIPPED TO {%s} {%s}.", arg1, arg2);
}

DO_PATH(path_unzip)
{
	struct listroot *root = ses->list[LIST_PATH];
	struct listnode *node;
	char name[BUFFER_SIZE], num[NUMBER_SIZE], *ptn;
	char arg1[BUFFER_SIZE];
	int cnt, max;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if ((node = search_nest_node_ses(ses, arg1)) == NULL)
	{
		arg = arg1;
	}
	else
	{
		arg = node->arg2;
	}

	kill_list(root);

	root->update = 0;

	while (*arg)
	{
		switch (*arg)
		{
			case ';':
			case ' ':
				arg++;
				continue;
		}

		if (isdigit((int) *arg))
		{
			ptn = num;

			while (isdigit((int) *arg))
			{
				if (ptn - num < 5)
				{
					*ptn++ = *arg++;
				}
				else
				{
					arg++;
				}
			}
			*ptn = 0;

			max = atoi(num);
		}
		else
		{
			max = 1;
		}

		arg = get_arg_stop_digits(ses, arg, name, GET_ONE);

		if (*name == 0)
		{
			break;
		}

		node = search_node_list(ses->list[LIST_PATHDIR], name);

		if (node)
		{
			for (cnt = 0 ; cnt < max ; cnt++)
			{
				create_node_list(root, node->arg1, node->arg2, "0", "");
			}
		}
		else
		{
			for (cnt = 0 ; cnt < max ; cnt++)
			{
				create_node_list(root, name, name, "0", "");
			}
		}
	}
/*
		arg = get_arg_in_braces(ses, arg, temp, GET_ALL);

		if (is_speedwalk(ses, temp))
		{
			char dir[2];
			int cnt, i;

			str = temp;

			for (dir[1] = 0 ; *str ; str++)
			{
				if (isdigit((int) *str))
				{
					sscanf(str, "%d%c", &cnt, dir);

					while (*str != dir[0])
					{
						str++;
					}
				}
				else
				{
					cnt = 1;
					dir[0] = *str;
				}

				for (i = 0 ; i < cnt ; i++)
				{
					if ((node = search_node_list(ses->list[LIST_PATHDIR], dir)))
					{
						create_node_list(root, node->arg1, node->arg2, "0", "");
					}
					else
					{
						create_node_list(root, dir, dir, "0", "");
					}
				}
			}
		}
		else
		{
			if ((node = search_node_list(ses->list[LIST_PATHDIR], temp)))
			{
				create_node_list(root, node->arg1, node->arg2, "0", "");
			}
			else
			{
				create_node_list(root, temp, temp, "0", "");
			}
		}
	}
*/
	show_message(ses, LIST_COMMAND, "#PATH UNZIP: PATH WITH %d NODES UNZIPPED.", root->used);
}


DO_PATH(path_goto)
{
	struct listroot *root = ses->list[LIST_PATH];
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (is_abbrev(arg1, "END"))
	{
		root->update = root->used;

		show_message(ses, LIST_COMMAND, "#PATH GOTO: POSITION SET TO {%d}.", root->update + 1);
	}
	else if (is_abbrev(arg1, "START"))
	{
		root->update = 0;

		show_message(ses, LIST_COMMAND, "#PATH GOTO: POSITION SET TO %d.", root->update + 1);
	}
	else if (is_math(ses, arg1))
	{
		if (get_number(ses, arg1) < 1 || get_number(ses, arg1) > root->used + 1)
		{
			show_message(ses, LIST_COMMAND, "#PATH GOTO: POSITION MUST BE BETWEEN 1 AND %d.", root->used + 1);
		}
		else
		{
			root->update = get_number(ses, arg1) - 1;

			show_message(ses, LIST_COMMAND, "#PATH GOTO: POSITION SET TO %d.", root->update + 1);
		}
	}
}


DO_PATH(path_move)
{
	struct listroot *root = ses->list[LIST_PATH];
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (is_abbrev(arg1, "BACKWARD"))
	{
		if (root->update == 0)
		{
			show_message(ses, LIST_COMMAND, "#PATH GOTO: ALREADY AT START OF PATH.");
		}
		else
		{
			root->update--;

			show_message(ses, LIST_COMMAND, "#PATH MOVE: POSITION SET TO %d.", root->update + 1);
		}
	}
	else if (is_abbrev(arg1, "FORWARD"))
	{
		if (root->update == root->used)
		{
			show_message(ses, LIST_COMMAND, "#PATH MOVE: ALREADY AT END OF PATH.");
		}
		else
		{
			root->update++;

			show_message(ses, LIST_COMMAND, "#PATH MOVE: POSITION SET TO %d.", root->update + 1);
		}
	}
	else if (is_math(ses, arg1))
	{
		int last = root->update;

		root->update = URANGE(0, root->update + get_number(ses, arg1), root->used);

		show_message(ses, LIST_COMMAND, "#PATH MOVE: POSITION MOVED FROM %d TO %d.", last + 1, root->update + 1);
	}
}

DO_PATH(path_undo)
{
	struct listroot *root = ses->list[LIST_PATH];

	if (root->used == 0)
	{
		show_message(ses, LIST_COMMAND, "#PATH UNDO: ERROR: PATH IS EMPTY.");

		return;
	}

	if (root->update != root->used)
	{
		show_message(ses, LIST_COMMAND, "#PATH UNDO: ERROR: YOUR POSITION IS NOT AT END OF PATH.");
	
		return;
	}

	if (!HAS_BIT(ses->flags, SES_FLAG_PATHMAPPING))
	{
		show_message(ses, LIST_COMMAND, "#PATH UNDO: ERROR: YOU ARE NOT CURRENTLY MAPPING A PATH.");

		return;
	}

	DEL_BIT(ses->flags, SES_FLAG_PATHMAPPING);

	script_driver(ses, LIST_COMMAND, root->list[root->used - 1]->arg2);

	SET_BIT(ses->flags, SES_FLAG_PATHMAPPING);

	delete_index_list(root, root->used - 1);

	root->update = root->used;


	show_message(ses, LIST_COMMAND, "#PATH MOVE: POSITION SET TO %d.", root->update);
}

void check_append_path(struct session *ses, char *forward, char *backward, int follow)
{
	struct listroot *root = ses->list[LIST_PATH];
	struct listnode *node;

	if (follow)
	{
		if ((node = search_node_list(ses->list[LIST_PATHDIR], forward)))
		{
			create_node_list(root, node->arg1, node->arg2, "0", "");

			root->update = root->used;
		}
	}
	else
	{
		if ((node = search_node_list(ses->list[LIST_PATHDIR], forward)))
		{
			create_node_list(root, node->arg1, node->arg2, "0", "");
		}
		else
		{
			create_node_list(root, forward, backward, "0", "");
		}
	}
}


DO_COMMAND(do_pathdir)
{
	struct listnode *node;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_PATHDIR], 0);
	}
	else if (*arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_PATHDIR]) == FALSE)
		{
			show_message(ses, LIST_PATHDIR, "#NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		if (*arg3 == 0)
		{
			if ((node = search_node_list(ses->list[LIST_PATHDIR], arg1)) != NULL)
			{
				strcpy(arg3, node->arg3);
			}
			else
			{
				strcpy(arg3, "0");
			}
		}
		else
		{
			if (!is_math(ses, arg3) || get_number(ses, arg3) < 0 || get_number(ses, arg3) >= 64)
			{
				show_message(ses, LIST_PATHDIR, "#PATHDIR: THE THIRD ARGUMENT MUST BE A NUMBER BETWEEN 0 and 63.");
				return ses;
			}
			get_number_string(ses, arg3, arg3);
		}
		node = update_node_list(ses->list[LIST_PATHDIR], arg1, arg2, arg3, "");

		node->val32[0] = atoi(arg3);

		show_message(ses, LIST_PATHDIR, "#OK: DIRECTION {%s} WILL BE REVERSED AS {%s} @ {%d}.", arg1, arg2, atoi(arg3));
	}
	return ses;
}

DO_COMMAND(do_unpathdir)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	do
	{
		if (delete_nest_node(ses->list[LIST_PATHDIR], arg1))
		{
			show_message(ses, LIST_PATHDIR, "#OK. {%s} IS NO LONGER A PATHDIR.", arg1);
		}
		else
		{
			delete_node_with_wild(ses, LIST_PATHDIR, arg1);
		}
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	}
	while (*arg1);

	return ses;
}

int exit_to_dir(struct session *ses, char *name)
{
	struct listnode *node;

	node = search_node_list(ses->list[LIST_PATHDIR], name);

	if (node)
	{
		return atoi(node->arg3);
	}
	else
	{
		return 0;
	}
}

char *dir_to_exit(struct session *ses, int dir)
{
	struct listroot *root;
	struct listnode *node;

	if (dir <= 0 || dir >= 64)
	{
		return "";
	}

	root = ses->list[LIST_PATHDIR];

	root->update = 0;

	while (root->update < root->used)
	{
		node = root->list[root->update];

		if (node->val32[0] == dir)
		{
			return node->arg1;
		}
		root->update++;
	}
	return "";
}

// Old commands, left for backward compatibility

DO_PATH(path_new)
{
	struct listroot *root = ses->list[LIST_PATH];

	if (HAS_BIT(ses->flags, SES_FLAG_PATHMAPPING))
	{
		show_message(ses, LIST_COMMAND, "#PATH NEW: YOU ARE ALREADY MAPPING A PATH.");
	}
	else
	{
		kill_list(root);

		root->update = 0;

		show_message(ses, LIST_COMMAND, "#PATH NEW: YOU ARE NOW MAPPING A PATH.");

		SET_BIT(ses->flags, SES_FLAG_PATHMAPPING);
	}
}

DO_PATH(path_end)
{
	if (HAS_BIT(ses->flags, SES_FLAG_PATHMAPPING))
	{
		show_message(ses, LIST_COMMAND, "#PATH END: YOU ARE NO LONGER MAPPING A PATH.");

		DEL_BIT(ses->flags, SES_FLAG_PATHMAPPING);
	}
	else
	{
		show_message(ses, LIST_COMMAND, "#PATH: YOU ARE NOT MAPPING A PATH.");
	}
}
	