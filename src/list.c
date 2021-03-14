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

#define DO_ARRAY(array) struct session *array (struct session *ses, struct listnode *list, char *arg, char *var, char *arg1, char *arg2)

extern DO_ARRAY(array_add);
extern DO_ARRAY(array_clear);
extern DO_ARRAY(array_collapse);
extern DO_ARRAY(array_create);
extern DO_ARRAY(array_delete);
extern DO_ARRAY(array_explode);
extern DO_ARRAY(array_filter);
extern DO_ARRAY(array_find);
extern DO_ARRAY(array_get);
extern DO_ARRAY(array_index);
extern DO_ARRAY(array_insert);
extern DO_ARRAY(array_order);
extern DO_ARRAY(array_reverse);
extern DO_ARRAY(array_set);
extern DO_ARRAY(array_shuffle);
extern DO_ARRAY(array_simplify);
extern DO_ARRAY(array_size);
extern DO_ARRAY(array_sort);
extern DO_ARRAY(array_tokenize);

typedef struct session *ARRAY(struct session *ses, struct listnode *list, char *arg, char *var, char *arg1, char *arg2);

struct array_type
{
	char                  * name;
	ARRAY                 * fun;
	char                  * desc;
};

struct array_type array_table[] =
{
	{     "ADD",              array_add,         "Add an item to a list table"             },
	{     "CLEAR",            array_clear,       "Clear a list"                            },
	{     "CLR",              array_clear,       NULL                                      },
	{     "COLLAPSE",         array_collapse,    "Collapse the list into a variable"       },
	{     "CREATE",           array_create,      "Create a list table with given items"    },
	{     "DELETE",           array_delete,      "Delete a list item with given index"     },
	{     "EXPLODE",          array_explode,     "Explode the variable into a list"        },
	{     "FILTER",           array_filter,      "Filter a list with given regex"          },
	{     "FIND",             array_find,        "Find a list item with given regex"       },
	{     "FND",              array_find,        NULL                                      },
	{     "GET",              array_get,         "Retrieve a list item with given index"   },
	{     "INDEXATE",         array_index,       "Indexate a list table for sorting"       },
	{     "INSERT",           array_insert,      "Insert a list item at given index"       },
	{     "ORDER",            array_order,       "Sort a list table numerically"           },
	{     "LENGTH",           array_size,        NULL                                      },
	{     "REVERSE",          array_reverse,     "Sort a list table in reverse order"      },
	{     "SET",              array_set,         "Change a list item at given index"       },
	{     "SHUFFLE",          array_shuffle,     "Sort a list table in random order"       },
	{     "SIMPLIFY",         array_simplify,    "Turn a list table into a simple list"    },
	{     "SIZE",             array_size,        NULL                                      },
	{     "SORT",             array_sort,        "Sort a list table alphabetically"        },
	{     "SRT",              array_sort,        NULL                                      },
	{     "TOKENIZE",         array_tokenize,    "Create a list with given characters"     },
	{     "",                 NULL,              ""                                        }
};

DO_COMMAND(do_list)
{
	struct listnode *node;
	int index, cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		info:

		tintin_header(ses, 80, " LIST OPTIONS ");

		for (index = 0 ; *array_table[index].fun ; index++)
		{
			if (array_table[index].desc && *array_table[index].name)
			{
				tintin_printf2(ses, "  [%-24s] %s", array_table[index].name, array_table[index].desc);
			}
		}
		tintin_header(ses, 80, "");
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
			if (!valid_variable(ses, arg1))
			{
				show_error(ses, LIST_VARIABLE, "#LIST: INVALID VARIABLE NAME {%s}.", arg1);

				return ses;
			}

			if ((node = search_nest_node_ses(ses, arg1)) == NULL)
			{
				node = set_nest_node_ses(ses, arg1, "");
			}
			array_table[cnt].fun(ses, node, arg, arg1, arg2, arg3);
		}
	}
	return ses;
}

int get_list_index(struct session *ses, struct listroot *root, char *arg)
{
	int toi;

	toi = get_number(ses, arg);

	if (toi > 0)
	{
		if (toi <= root->used)
		{
			return toi - 1;
		}
		return -1;
	}

	if (toi < 0)
	{
		if (root->used + toi >= 0)
		{
			return root->used + toi;
		}
		return -1;
	}
	return -1;
}

DO_ARRAY(array_add)
{
	char *str;
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
	char *buf, *str;
	int index = 1;

	buf = str_alloc_stack(0);

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
	int index, cnt, loop;

	if (list->root)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

		index = get_list_index(ses, list->root, arg1);

		if (*arg2)
		{
			loop = URANGE(1, (int) get_number(ses, arg2), list->root->used - index);
		}
		else
		{
			loop = 1;
		}

		if (index == -1)
		{
			show_error(ses, LIST_VARIABLE, "#LIST {%s} DEL: Invalid index: %s", var, arg1);

			return ses;
		}

		for (cnt = index + loop ; cnt < list->root->used ; cnt++)
		{
			str_cpy_printf(&list->root->list[cnt]->arg1, "%d", cnt + 1 - loop);
		}

		while (loop--)
		{
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
	char *pti;
	int index = 1;

	if (list->root)
	{
		array_collapse(ses, list, arg, var, arg1, arg2);
	}

	list->root = init_list(ses, LIST_VARIABLE, LIST_SIZE);

	pti = list->arg2;

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			pti += sprintf(arg2, "%.*s", get_euc_size(ses, pti), pti);
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(pti))
		{
			pti += sprintf(arg2, "%.*s", get_utf8_size(pti), pti);
		}
		else
		{
			pti += sprintf(arg2, "%c", *pti);
		}

		set_nest_node(list->root, ntos(index++), "%s", arg2);
	}
	sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	pti = arg1;

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			pti += sprintf(arg2, "%.*s", get_euc_size(ses, pti), pti);
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(pti))
		{
			pti += sprintf(arg2, "%.*s", get_utf8_size(pti), pti);
		}
		else
		{
			pti += sprintf(arg2, "%c", *pti);
		}

		set_nest_node(list->root, ntos(index++), "%s", arg2);
	}
	return ses;
}

DO_ARRAY(array_filter)
{
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 && *arg2 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #LIST {variable} FILTER {keep} {remove}");

		return ses;
	}

	if (list->root)
	{
		if (*arg1)
		{
			for (index = 0 ; index < list->root->used ; index++)
			{
				if (!match(ses, list->root->list[index]->arg2, arg1, SUB_NONE))
				{
					delete_index_list(list->root, index--);
				}
			}
		}

		if (*arg2)
		{
			for (index = 0 ; index < list->root->used ; index++)
			{
				if (match(ses, list->root->list[index]->arg2, arg2, SUB_NONE))
				{
					delete_index_list(list->root, index--);
				}
			}
		}
	}

	return ses;
}

DO_ARRAY(array_find)
{
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

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
				set_nest_node_ses(ses, arg2, "%d", index + 1);

				return ses;
			}
		}
	}
	set_nest_node_ses(ses, arg2, "0");

	return ses;
}


DO_ARRAY(array_get)
{
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg2 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #LIST {variable} GET {index} {variable}");
		
		return ses;
	}

	if (list->root)
	{
		index = get_list_index(ses, list->root, arg1);

		if (index != -1)
		{
			set_nest_node_ses(ses, arg2, "%s", list->root->list[index]->arg2);

			return ses;
		}
	}
	set_nest_node_ses(ses, arg2, "0");

	return ses;
}

DO_ARRAY(array_index)
{
	int cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (list->root == NULL || list->root->list[0]->root == NULL)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #LIST {%s} INDEX: NOT AN INDEXABLE LIST TABLE.", var);

		return ses;
	}

	if (list->root->used > 1)
	{
		int index = search_index_list(list->root->list[0]->root, arg1, "");

		if (index == -1)
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #LIST {%s} INDEX {%s}: FAILED TO FIND NEST.", var, arg1);

			return ses;
		}

		for (cnt = 0 ; cnt < list->root->used ; cnt++)
		{
			if (list->root->list[cnt]->root && list->root->list[cnt]->root->used > index)
			{
				str_cpy(&list->root->list[cnt]->arg2, list->root->list[cnt]->root->list[index]->arg2);
			}
			else
			{
				show_error(ses, LIST_COMMAND, "#ERROR: #LIST {%s} INDEX: FAILED TO POPULATE INDEX {%s}.", var, list->root->list[cnt]->arg1);
				break;
			}
		}
	}
	return ses;
}

DO_ARRAY(array_insert)
{
	int cnt, toi, index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (!list->root)
	{
		list->root = init_list(ses, LIST_VARIABLE, LIST_SIZE);
	}

	toi = get_number(ses, arg1);

	if (toi == 0)
	{
		show_error(ses, LIST_VARIABLE, "#LIST INS: Invalid index: %s", arg1);

		return ses;
	}

	index = get_list_index(ses, list->root, arg1);

	if (index == -1 || toi < 0)
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
	int cnt, val, len;
	char **arg2_buffer;

	array_add(ses, list, arg, var, arg1, arg2);

	if (list->root->used > 1)
	{
/*		if (*list->root->list[0]->arg2 == 0)
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #LIST {%s} ORDER: LIST IS NOT INDEXED.", var);

			return ses;
		}
*/
		if (list->root->list[0]->root)
		{
			struct listroot **root_buffer;

			root_buffer = malloc(list->root->used * sizeof(struct listroot *));
			arg2_buffer = malloc(list->root->used * sizeof(char *));

			for (cnt = 0 ; cnt < list->root->used ; cnt++)
			{
				len = str_len(list->root->list[cnt]->arg2);

				root_buffer[cnt] = list->root->list[cnt]->root;
				arg2_buffer[cnt] = list->root->list[cnt]->arg2;

				str_resize(&arg2_buffer[cnt], 10);

				sprintf(arg2_buffer[cnt] + len + 1, "%x", cnt);
			}

			quadsort(arg2_buffer, list->root->used, sizeof(char *), cmp_num);

			for (cnt = 0 ; cnt < list->root->used ; cnt++)
			{
				val = hex_number_32bit(arg2_buffer[cnt] + str_len(arg2_buffer[cnt]) + 1);

				list->root->list[cnt]->root = root_buffer[val];
				list->root->list[cnt]->arg2 = arg2_buffer[cnt];
			}

			free(arg2_buffer);
			free(root_buffer);
		}
		else
		{
			arg2_buffer = malloc(list->root->used * sizeof(char *));

			for (cnt = 0 ; cnt < list->root->used ; cnt++)
			{
				arg2_buffer[cnt] = list->root->list[cnt]->arg2;
			}

			quadsort(arg2_buffer, list->root->used, sizeof(char *), cmp_num);

			for (cnt = 0 ; cnt < list->root->used ; cnt++)
			{
				list->root->list[cnt]->arg2 = arg2_buffer[cnt];
			}

			free(arg2_buffer);
		}
	}
	return ses;
}

DO_ARRAY(array_reverse)
{
	struct listroot *toor;
	char *swap;
	int cnt, rev;

	array_add(ses, list, arg, var, arg1, arg2);

	for (cnt = 0 ; cnt < list->root->used / 2 ; cnt++)
	{
		rev = list->root->used - 1 - cnt;

		swap = list->root->list[cnt]->arg2;
		list->root->list[cnt]->arg2 = list->root->list[rev]->arg2;
		list->root->list[rev]->arg2 = swap;

		toor = list->root->list[cnt]->root;
		list->root->list[cnt]->root = list->root->list[rev]->root;
		list->root->list[rev]->root = toor;
	}
	return ses;
}

DO_ARRAY(array_simplify)
{
	char *str;
	int index;

	array_add(ses, list, arg, var, arg1, arg2);

	str = str_alloc_stack(0);

	if (list->root)
	{
		if (list->root->used)
		{
			str_cpy(&str, list->root->list[0]->arg2);
		}

		for (index = 1 ; index < list->root->used ; index++)
		{
			str_cat_printf(&str, ";%s", list->root->list[index]->arg2);
		}

		set_nest_node_ses(ses, var, "%s", str);

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
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (list->root)
	{
		index = get_list_index(ses, list->root, arg1);

		if (index == -1)
		{
			show_error(ses, LIST_VARIABLE, "#LIST {%s} SET: Invalid index: %s", var, arg1);

			return ses;
		}
		str_cpy(&list->root->list[index]->arg2, arg2);

		return ses;
	}

	show_error(ses, LIST_VARIABLE, "#LIST SET: {%s} is not a list.", var);

	return ses;
}

DO_ARRAY(array_shuffle)
{
	struct listroot *toor;
	char *swap;
	int cnt, rnd;

	array_add(ses, list, arg, var, arg1, arg2);

	for (cnt = 0 ; cnt < list->root->used ; cnt++)
	{
		rnd = generate_rand(ses) % list->root->used;

		swap = list->root->list[cnt]->arg2;
		list->root->list[cnt]->arg2 = list->root->list[rnd]->arg2;
		list->root->list[rnd]->arg2 = swap;

		toor = list->root->list[cnt]->root;
		list->root->list[cnt]->root = list->root->list[rnd]->root;
		list->root->list[rnd]->root = toor;
	}
	return ses;
}

DO_ARRAY(array_sort)
{
	int cnt, val, len;
	char **arg2_buffer;

	array_add(ses, list, arg, var, arg1, arg2);

	if (list->root->used > 1)
	{
		if (*list->root->list[0]->arg2 == 0)
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #LIST {%s} ORDER: LIST IS NOT INDEXED.", var);

			return ses;
		}

		if (list->root->list[0]->root)
		{
			struct listroot **root_buffer;

			root_buffer = malloc(list->root->used * sizeof(struct listroot *));
			arg2_buffer = malloc(list->root->used * sizeof(char *));

			for (cnt = 0 ; cnt < list->root->used ; cnt++)
			{
				len = str_len(list->root->list[cnt]->arg2);

				root_buffer[cnt] = list->root->list[cnt]->root;
				arg2_buffer[cnt] = list->root->list[cnt]->arg2;

				str_resize(&arg2_buffer[cnt], 10);

				sprintf(arg2_buffer[cnt] + len + 1, "%x", cnt);
			}

			quadsort(arg2_buffer, list->root->used, sizeof(char *), cmp_str);

			for (cnt = 0 ; cnt < list->root->used ; cnt++)
			{
				val = hex_number_32bit(arg2_buffer[cnt] + str_len(arg2_buffer[cnt]) + 1);

				list->root->list[cnt]->root = root_buffer[val];
				list->root->list[cnt]->arg2 = arg2_buffer[cnt];
			}

			free(arg2_buffer);
			free(root_buffer);
		}
		else
		{
			arg2_buffer = malloc(list->root->used * sizeof(char *));

			for (cnt = 0 ; cnt < list->root->used ; cnt++)
			{
				arg2_buffer[cnt] = list->root->list[cnt]->arg2;
			}

			quadsort(arg2_buffer, list->root->used, sizeof(char *), cmp_str);

			for (cnt = 0 ; cnt < list->root->used ; cnt++)
			{
				list->root->list[cnt]->arg2 = arg2_buffer[cnt];
			}

			free(arg2_buffer);
		}
	}
	return ses;
}

DO_ARRAY(array_tokenize)
{
	int index = 1, i;

	sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (list->root)
	{
		free_list(list->root);
	}

	list->root = init_list(ses, LIST_VARIABLE, LIST_SIZE);

	i = 0;

	while (arg1[i] != 0)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, &arg1[i]))
		{
			i += sprintf(arg2, "%.*s", get_euc_size(ses, &arg1[i]), &arg1[i]);
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&arg1[i]))
		{
			i += sprintf(arg2, "%.*s", get_utf8_size(&arg1[i]), &arg1[i]);
		}
		else
		{
			i += sprintf(arg2, "%c", arg1[i]);
		}

		set_nest_node(list->root, ntos(index++), "%s", arg2);
	}
	return ses;
}
