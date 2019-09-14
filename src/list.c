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
*                     coded by Igor van den Hoven 2004                        *
******************************************************************************/


#include "tintin.h"


DO_COMMAND(do_list)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	struct listroot *root;
	struct listnode *node;
	int cnt;

	root = ses->list[LIST_VARIABLE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 || *arg2 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #LIST {variable} {ADD|CLE|CRE|DEL|FIN|GET|INS|SET|SIM|SIZ|SOR} {argument}");
	}
	else
	{
		for (cnt = 0 ; *array_table[cnt].name ; cnt++)
		{
			if (is_abbrev(arg2, array_table[cnt].name))
			{
				break;
			}
		}

		if (*array_table[cnt].name == 0)
		{
			return do_list(ses, "");
		}
		else
		{
			if ((node = search_nest_node(root, arg1)) == NULL)
			{
				node = set_nest_node(root, arg1, "");
			}
			array_table[cnt].array(ses, node, arg, arg1);
		}
	}
	return ses;
}

DO_ARRAY(array_add)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], *str;
	int index;

	if (!list->root)
	{
		list->root = init_list(ses, LIST_VARIABLE, LIST_SIZE);
	}

	index = list->root->used + 1;

	while (*arg)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

		str = arg1;

		while (*str)
		{
			str = get_arg_in_braces(ses, str, arg2, GET_ALL);

			set_nest_node(list->root, ntos(index++), "%s", arg2);

//			insert_node_list(list->root, ntos(index++), arg2, "");

			if (*str == COMMAND_SEPARATOR)
			{
				str++;
			}
		}
	}
	return ses;
}

DO_ARRAY(array_clear)
{
	if (list->root)
	{
		free_list(list->root);

		list->root = NULL;
	}

	set_nest_node(ses->list[LIST_VARIABLE], var, "");

	return ses;
}

DO_ARRAY(array_create)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], buf[BUFFER_SIZE], *str;

	int index = 1;

	substitute(ses, arg, buf, SUB_VAR|SUB_FUN);

	arg = buf;

	if (list->root)
	{
		free_list(list->root);
	}

	list->root = init_list(ses, LIST_VARIABLE, LIST_SIZE);

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

		str = arg1;

		while (*str)
		{
			str = get_arg_in_braces(ses, str, arg2, GET_ALL);

			set_nest_node(list->root, ntos(index++), "%s", arg2);

			if (*str == COMMAND_SEPARATOR)
			{
				str++;
			}
		}

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}
	return ses;
}

DO_ARRAY(array_delete)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int index, cnt, loop;

	if (list->root)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

		loop = *arg2 ? (int) get_number(ses, arg2) : 1;

		while (loop--)
		{
			index = search_nest_index(list->root, arg1);

			if (atoi(arg1) == 0 || index == -1)
			{
				show_error(ses, LIST_VARIABLE, "#LIST DEL: Invalid index: %s", arg1);

				return ses;
			}

			for (cnt = index + 1 ; cnt < list->root->used ; cnt++)
			{
				list->root->list[cnt]->arg1 = restringf(list->root->list[cnt]->arg1, "%d", cnt);
			}

			delete_index_list(list->root, index);
		}
	}
	else
	{
		show_error(ses, LIST_VARIABLE, "#LIST DEL: {%s} is not a list.", var);
	}
	return ses;
}

DO_ARRAY(array_find)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg2 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #LIST {variable} FIND {string} {variable}");
		
		return ses;
	}

	if (list->root)
	{
		for (index = 0 ; index < list->root->used ; index++)
		{
			if (match(ses, list->root->list[index]->arg2, arg1, SUB_NONE))
			{
				break;
			}
		}
		if (index < list->root->used)
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", index + 1);
		}
		else
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "0");
		}
		return ses;
	}
	else
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "0");
	}

	return ses;
}


DO_ARRAY(array_get)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg2 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #LIST {variable} GET {index} {variable}");
		
		return ses;
	}

	if (list->root)
	{
		int index = search_nest_index(list->root, arg1);

		if (atoi(arg1) == 0 || index == -1)
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "0");
		}
		else
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", list->root->list[index]->arg2);
		}
		return ses;
	}
	else
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "0");
	}

	return ses;
}

DO_ARRAY(array_insert)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int cnt, index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (!list->root)
	{
		list->root = init_list(ses, LIST_VARIABLE, LIST_SIZE);
	}

	index = search_nest_index(list->root, arg1);

	if (atoi(arg1) == 0)
	{
		show_error(ses, LIST_VARIABLE, "#LIST INS: Invalid index: %s", arg1);
		
		return ses;
	}

	if (index == -1 || atoi(arg1) < 0)
	{
		index++;
	}

	for (cnt = index ; cnt < list->root->used ; cnt++)
	{
		list->root->list[cnt]->arg1 = restringf(list->root->list[cnt]->arg1, "%d", cnt + 2);
	}

	set_nest_node(list->root, ntos(index + 1), "%s", arg2);

//	insert_node_list(list->root, ntos(index + 1), arg2, "");

	return ses;
}

DO_ARRAY(array_simplify)
{
	char arg1[BUFFER_SIZE], tmp[BUFFER_SIZE];
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #LIST {variable} SIMPLIFY {variable}");
		
		return ses;
	}

	if (list->root)
	{
		for (index = 0 ; index < list->root->used ; index++)
		{
			if (index == 0)
			{
				strcpy(tmp, list->root->list[index]->arg2);
			}
			else
			{
				cat_sprintf(tmp, ";%s", list->root->list[index]->arg2);
			}
		}
		set_nest_node(ses->list[LIST_VARIABLE], arg1, "%s", tmp);

		return ses;
	}
	else
	{
		show_error(ses, LIST_VARIABLE, "#LIST SIMPLIFY: {%s} is not a list.", arg1);
	}

	return ses;
}

DO_ARRAY(array_size)
{
	char arg1[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #LIST {variable} SIZE {variable}");
		
		return ses;
	}

	if (list->root)
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg1, "%d", list->root->used);
	}
	else
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg1, "0");
	}
	return ses;
}

DO_ARRAY(array_set)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (list->root)
	{
		int index = search_nest_index(list->root, arg1);

		if (atoi(arg1) == 0 || index == -1)
		{
			show_error(ses, LIST_VARIABLE, "#LIST SET: Invalid index: %s", arg1);
			
			return ses;
		}

		RESTRING(list->root->list[index]->arg2, arg2);

		return ses;
	}

	show_error(ses, LIST_VARIABLE, "#LIST SET: {%s} is not a list.", var);

	return ses;
}

DO_ARRAY(array_sort)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE], *str;
	int cnt;

	if (!list->root)
	{
		list->root = init_list(ses, LIST_VARIABLE, LIST_SIZE);
	}

	while (*arg)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

		str = arg1;

		while (*str)
		{
			str = get_arg_in_braces(ses, str, arg2, GET_ALL);

			for (cnt = 0 ; cnt < list->root->used ; cnt++)
			{
				if (strcmp(arg2, list->root->list[cnt]->arg2) <= 0)
				{
					break;
				}
			}

			if (cnt == list->root->used)
			{
				sprintf(arg3, "{%d} {%s}", -1, arg2);
			}
			else
			{
				sprintf(arg3, "{%d} {%s}", cnt + 1, arg2);
			}

			array_insert(ses, list, arg3, var);

			if (*str == COMMAND_SEPARATOR)
			{
				str++;
			}
		}
		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}
	return ses;
}

DO_ARRAY(array_tokenize)
{
	char buf[BUFFER_SIZE], tmp[BUFFER_SIZE];
	int index = 1, i;

	sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN);

	if (list->root)
	{
		free_list(list->root);
	}

	list->root = init_list(ses, LIST_VARIABLE, LIST_SIZE);

	i = 0;

	while (buf[i] != 0)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && buf[i] & 128 && buf[i+1] != 0)
		{
			i += sprintf(tmp, "%c%c", buf[i], buf[i+1]);
		}
		else if (HAS_BIT(ses->flags, SES_FLAG_UTF8) && is_utf8_head(&buf[i]))
		{
			i += sprintf(tmp, "%.*s", get_utf8_size(&buf[i]), &buf[i]);
		}
		else
		{
			i += sprintf(tmp, "%c", buf[i]);
		}

		set_nest_node(list->root, ntos(index++), "%s", tmp);
	}
	return ses;
}
