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
	struct listnode *node;
	int index, cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		info:

		tintin_header(ses, " LIST OPTIONS ");

		for (index = 0 ; *array_table[index].fun ; index++)
		{
			if (array_table[index].desc && *array_table[index].name)
			{
				tintin_printf2(ses, "  [%-24s] %s", array_table[index].name, array_table[index].desc);
			}
		}
		tintin_header(ses, "");
	}
	else if (*arg2 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #LIST {variable} {option} {argument}");
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
			goto info;
		}
		else
		{
			if ((node = search_nest_node_ses(ses, arg1)) == NULL)
			{
				node = set_nest_node_ses(ses, arg1, "");
			}
			array_table[cnt].fun(ses, node, arg, arg1);
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

DO_ARRAY(array_clear)
{
	if (list->root)
	{
		free_list(list->root);

		list->root = NULL;
	}

	set_nest_node_ses(ses, var, "");

	return ses;
}

DO_ARRAY(array_collapse)
{
	int index;

	if (list->root)
	{
		str_cpy(&list->arg2, "");

		for (index = 0 ; index < list->root->used ; index++)
		{
			str_cat(&list->arg2, list->root->list[index]->arg2);
		}
		free_list(list->root);

		list->root = NULL;
	}
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
				str_cpy_printf(&list->root->list[cnt]->arg1, "%d", cnt);
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

DO_ARRAY(array_explode)
{
	char buf[BUFFER_SIZE], tmp[BUFFER_SIZE], *pti;
	int index = 1;

	if (list->root)
	{
		array_collapse(ses, list, "", "");
	}

	list->root = init_list(ses, LIST_VARIABLE, LIST_SIZE);

	pti = list->arg2;

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			pti += sprintf(tmp, "%.*s", get_euc_size(ses, pti), pti);
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(pti))
		{
			pti += sprintf(tmp, "%.*s", get_utf8_size(pti), pti);
		}
		else
		{
			pti += sprintf(tmp, "%c", *pti);
		}

		set_nest_node(list->root, ntos(index++), "%s", tmp);
	}
	sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN);

	pti = buf;

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			pti += sprintf(tmp, "%.*s", get_euc_size(ses, pti), pti);
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(pti))
		{
			pti += sprintf(tmp, "%.*s", get_utf8_size(pti), pti);
		}
		else
		{
			pti += sprintf(tmp, "%c", *pti);
		}

		set_nest_node(list->root, ntos(index++), "%s", tmp);
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
			set_nest_node_ses(ses, arg2, "%d", index + 1);
		}
		else
		{
			set_nest_node_ses(ses, arg2, "0");
		}
		return ses;
	}
	else
	{
		set_nest_node_ses(ses, arg2, "0");
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
			set_nest_node_ses(ses, arg2, "0");
		}
		else
		{
			set_nest_node_ses(ses, arg2, "%s", list->root->list[index]->arg2);
		}
		return ses;
	}
	else
	{
		set_nest_node_ses(ses, arg2, "0");
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
		str_cpy_printf(&list->root->list[cnt]->arg1, "%d", cnt + 2);
	}

	set_nest_node(list->root, ntos(index + 1), "%s", arg2);

	return ses;
}

DO_ARRAY(array_order)
{
	int cnt;
	char **buffer;

	array_add(ses, list, arg, var);

	buffer = malloc(list->root->used * sizeof(char *));

	for (cnt = 0 ; cnt < list->root->used ; cnt++)
	{
		buffer[cnt] = list->root->list[cnt]->arg2;
	}

	quadsort(buffer, list->root->used, sizeof(char *), cmp_num);

	for (cnt = 0 ; cnt < list->root->used ; cnt++)
	{
		list->root->list[cnt]->arg2 = buffer[cnt];
	}

	free(buffer);

	return ses;
}

DO_ARRAY(array_reverse)
{
	char *swap;
	int cnt, rev;

	array_add(ses, list, arg, var);

	for (cnt = 0 ; cnt < list->root->used / 2 ; cnt++)
	{
		rev = list->root->used - 1 - cnt;

		swap = list->root->list[cnt]->arg2;
		list->root->list[cnt]->arg2 = list->root->list[rev]->arg2;
		list->root->list[rev]->arg2 = swap;
	}
	return ses;
}

DO_ARRAY(array_simplify)
{
	char arg1[BUFFER_SIZE], *str;
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);
/*
	if (*arg1 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #LIST {variable} SIMPLIFY {variable}");
		
		return ses;
	}
*/
	if (list->root)
	{
		for (index = 0 ; index < list->root->used ; index++)
		{
			if (index == 0)
			{
				str = str_dup(list->root->list[index]->arg2);
			}
			else
			{
				str_cat_printf(&str, ";%s", list->root->list[index]->arg2);
			}
		}
		if (*arg1 == 0)
		{
			set_nest_node_ses(ses, list->arg1, "%s", str);
		}
		else
		{
			set_nest_node_ses(ses, arg1, "%s", str);
		}

		str_free(str);

		return ses;
	}
	else
	{
		show_error(ses, LIST_VARIABLE, "#LIST SIMPLIFY: {%s} is not a list.", list->arg1);
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
		set_nest_node_ses(ses, arg1, "%d", list->root->used);
	}
	else
	{
		set_nest_node_ses(ses, arg1, "0");
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

//		set_nest_node(list->root, ntos(index + 1), "%s", arg2);

		str_cpy(&list->root->list[index]->arg2, arg2);

		return ses;
	}

	show_error(ses, LIST_VARIABLE, "#LIST SET: {%s} is not a list.", var);

	return ses;
}

DO_ARRAY(array_shuffle)
{
	char *swap;
	int cnt, rnd;

	array_add(ses, list, arg, var);

	for (cnt = 0 ; cnt < list->root->used ; cnt++)
	{
		rnd = generate_rand(ses) % list->root->used;

		swap = list->root->list[cnt]->arg2;
		list->root->list[cnt]->arg2 = list->root->list[rnd]->arg2;
		list->root->list[rnd]->arg2 = swap;
	}
	return ses;
}

DO_ARRAY(array_sort)
{
	int cnt;
	char **buffer;

	array_add(ses, list, arg, var);

	buffer = malloc(list->root->used * sizeof(char *));

	for (cnt = 0 ; cnt < list->root->used ; cnt++)
	{
		buffer[cnt] = list->root->list[cnt]->arg2;
	}

	quadsort(buffer, list->root->used, sizeof(char *), cmp_str);

	for (cnt = 0 ; cnt < list->root->used ; cnt++)
	{
		list->root->list[cnt]->arg2 = buffer[cnt];
	}

	free(buffer);

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
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, &buf[i]))
		{
			i += sprintf(tmp, "%.*s", get_euc_size(ses, &buf[i]), &buf[i]);
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&buf[i]))
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
