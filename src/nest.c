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
*                      coded by Igor van den Hoven 2009                       *
******************************************************************************/

#include "tintin.h"

struct listroot *search_nest_root(struct listroot *root, char *arg)
{
	struct listnode *node;

	node = search_node_list(root, arg);

	if (node == NULL || node->root == NULL)
	{
		return NULL;
	}
	return node->root;
}

struct listroot *search_nest_base_ses(struct session *ses, char *arg)
{
	struct listnode *node;
	struct listroot *root;

	int index;

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_LOCAL))
	{
		for (index = gtd->script_index ; index >= 0 ; index--)
		{
			root = gtd->script_stack[index]->local;

			if (root->used)
			{
				node = search_node_list(root, arg);

				if (node)
				{
					return root;
				}
			}
		}
	}

	node = search_node_list(ses->list[LIST_VARIABLE], arg);

	if (node == NULL)
	{
		return NULL;
	}
	return ses->list[LIST_VARIABLE];
}

struct listnode *search_base_node(struct listroot *root, char *variable)
{
	char name[BUFFER_SIZE];

	get_arg_to_brackets(root->ses, variable, name);

	return search_node_list(root, name);
}

struct listnode *search_nest_node_ses(struct session *ses, char *variable)
{
	char name[BUFFER_SIZE], *arg;
	struct listroot *root;
	struct listnode *node;
	int index;

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_LOCAL))
	{
		for (index = gtd->script_index ; index >= 0 ; index--)
		{
			root = gtd->script_stack[index]->local;

			if (root->used)
			{
				arg = get_arg_to_brackets(root->ses, variable, name);

				while (root && *arg)
				{
					root = search_nest_root(root, name);

					if (root)
					{
						arg = get_arg_in_brackets(root->ses, arg, name);
					}
				}

				if (root)
				{
					node = search_node_list(root, name);

					if (node)
					{
						return node;
					}
				}
			}
		}
	}

	root = ses->list[LIST_VARIABLE];

	arg = get_arg_to_brackets(ses, variable, name);

	while (root && *arg)
	{
		root = search_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	if (root)
	{
		return search_node_list(root, name);
	}

	return NULL;
}

	
struct listnode *search_nest_node(struct listroot *root, char *variable)
{
	char name[BUFFER_SIZE], *arg;

	arg = get_arg_to_brackets(root->ses, variable, name);

	while (root && *arg)
	{
		root = search_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	if (root)
	{
		return search_node_list(root, name);
	}

	return NULL;
}

int search_nest_index(struct listroot *root, char *variable)
{
	char name[BUFFER_SIZE], *arg;

	arg = get_arg_to_brackets(root->ses, variable, name);

	while (root && *arg)
	{
		root = search_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	if (root)
	{
		return search_index_list(root, name, NULL);
	}

	return -1;
}

struct listroot *update_nest_root(struct listroot *root, char *arg)
{
	struct listnode *node;

	node = search_node_list(root, arg);

	if (node == NULL)
	{
		node = update_node_list(root, arg, "", "", "");
	}

	if (node->root == NULL)
	{
		node->root = init_list(root->ses, root->type, LIST_SIZE);
	}

	return node->root;
}

void update_nest_node(struct listroot *root, char *arg)
{
//	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	char *arg1, *arg2;

	arg1 = str_mim(arg);
	arg2 = str_mim(arg);

	while (*arg)
	{
		arg = get_arg_in_braces(root->ses, arg, arg1, GET_ONE);
		arg = get_arg_in_braces(root->ses, arg, arg2, GET_ONE);

		if (*arg2 == DEFAULT_OPEN)
		{
			update_nest_node(update_nest_root(root, arg1), arg2);
		}
		else if (*arg1)
		{
			update_node_list(root, arg1, arg2, "", "");
		}

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}
	str_free(arg1);
	str_free(arg2);

	return;
}

int delete_nest_node(struct listroot *root, char *variable)
{
	char name[BUFFER_SIZE], *arg;
	int index;

	arg = get_arg_to_brackets(root->ses, variable, name);

	while (root && *arg)
	{
		root = search_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	if (root)
	{
		index = search_index_list(root, name, NULL);

		if (index != -1)
		{
			delete_index_list(root, index);

			return TRUE;
		}
	}

	return FALSE;
}

// Return the number of indices of a node.

int get_nest_size(struct listroot *root, char *variable)
{
	char name[BUFFER_SIZE], *arg;
	int index, count;
	arg = get_arg_to_brackets(root->ses, variable, name);

	if (!strcmp(arg, "[]"))
	{
		if (*name == 0)
		{
			return root->used + 1;
		}

		if (search_nest_root(root, name) == NULL)
		{
			if (search_node_list(root, name))
			{
				return 1;
			}
		}
	}

	while (root && *name)
	{
		// Handle regex queries

		if (search_nest_root(root, name) == NULL)
		{
			if (search_node_list(root, name) == NULL)
			{
				if (tintin_regexp_check(root->ses, name))
				{
					for (index = count = 0 ; index < root->used ; index++)
					{
						if (match(root->ses, root->list[index]->arg1, name, SUB_NONE))
						{
							count++;
						}
					}
					return count + 1;
				}
				else if (strstr(name, "..") && is_math(root->ses, name))
				{
					int min, max, range;

					if (root->used)
					{
						range = get_ellipsis(root, name, &min, &max);

						return range + 1;
					}
					else
					{
						return 1;
					}
				}
				else
				{
					return 0;
				}
			}
		}

		root = search_nest_root(root, name);

		if (root)
		{
			if (!strcmp(arg, "[]"))
			{
				return root->used + 1;
			}
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	return 0;
}

int get_nest_size_index(struct listroot *root, char *variable, char **result)
{
	char name[BUFFER_SIZE], *arg;
	int index, count;

	arg = get_arg_to_brackets(root->ses, variable, name);

	str_cpy(result, "");

	if (!strcmp(arg, "[]"))
	{
		if (*name == 0)
		{
			return root->used + 1;
		}

		if (search_nest_root(root, name) == NULL)
		{
			if (search_node_list(root, name))
			{
				return 1;
			}
		}
	}

	while (root && *name)
	{
		// Handle regex queries

		if (search_nest_root(root, name) == NULL)
		{
			if (search_node_list(root, name) == NULL)
			{
				if (tintin_regexp_check(root->ses, name))
				{
					for (index = count = 0 ; index < root->used ; index++)
					{
						if (match(root->ses, root->list[index]->arg1, name, SUB_NONE))
						{
							count++;
						}
					}
					return count + 1;
				}
				else if (strstr(name, "..") && is_math(root->ses, name))
				{
					int min, max, range;

					if (root->used)
					{
						range = get_ellipsis(root, name, &min, &max);

						return range + 1;
					}
					else
					{
						return 1;
					}
				}
				else
				{
					return 0;
				}
			}
		}

		root = search_nest_root(root, name);

		if (root)
		{
			if (!strcmp(arg, "[]"))
			{
				return root->used + 1;
			}
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	return 0;
}

int get_nest_size_key(struct listroot *root, char *variable, char **result)
{
	char name[BUFFER_SIZE], *arg;
	int index, count;

	arg = get_arg_to_brackets(root->ses, variable, name);

	str_cpy(result, "");

	if (!strcmp(arg, "[]"))
	{
		if (*name == 0)
		{
			for (index = 0 ; index < root->used ; index++)
			{
				str_cat_printf(result, "{%s}", root->list[index]->arg1);
			}
			return root->used + 1;
		}

		if (search_nest_root(root, name) == NULL)
		{
			if (search_node_list(root, name))
			{
				return 1;
			}
		}
	}

	while (root && *name)
	{
		// Handle regex queries

		if (search_nest_root(root, name) == NULL)
		{
			if (search_node_list(root, name) == NULL)
			{
				if (tintin_regexp_check(root->ses, name))
				{
					for (index = count = 0 ; index < root->used ; index++)
					{
						if (match(root->ses, root->list[index]->arg1, name, SUB_NONE))
						{
							str_cat_printf(result, "{%s}", root->list[index]->arg1);
							count++;
						}
					}
					return count + 1;
				}
				else if (strstr(name, "..") && is_math(root->ses, name))
				{
					int min, max, range;

					if (root->used)
					{
						range = get_ellipsis(root, name, &min, &max);

						if (min < max)
						{
							while (min <= max)
							{
								str_cat_printf(result, "{%s}", root->list[min++]->arg1);
							}
						}
						else
						{
							while (min >= max)
							{
								str_cat_printf(result, "{%s}", root->list[min--]->arg1);
							}
						}
						return range + 1;
					}
					else
					{
						return 1;
					}
				}
				else
				{
					return 0;
				}
			}
		}

		root = search_nest_root(root, name);

		if (root)
		{
			if (!strcmp(arg, "[]"))
			{
				for (index = 0 ; index < root->used ; index++)
				{
					str_cat_printf(result, "{%s}", root->list[index]->arg1);
				}
				return root->used + 1;
			}
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	return 0;
}

int get_nest_size_val(struct listroot *root, char *variable, char **result)
{
	char name[BUFFER_SIZE], *arg;
	int index, count;
	static int warning;

	arg = get_arg_to_brackets(root->ses, variable, name);

	str_cpy(result, "");

	if (!strcmp(arg, "[]"))
	{
		if (*name == 0)
		{
			for (index = 0 ; index < root->used ; index++)
			{
				str_cat_printf(result, "{%s}", root->list[index]->arg1);
			}
			return root->used + 1;
		}

		if (search_nest_root(root, name) == NULL)
		{
			if (search_node_list(root, name))
			{
				return 1;
			}
		}
	}

	while (root && *name)
	{
		// Handle regex queries

		if (search_nest_root(root, name) == NULL)
		{
			if (search_node_list(root, name) == NULL)
			{
				if (tintin_regexp_check(root->ses, name))
				{
					for (index = count = 0 ; index < root->used ; index++)
					{
						if (match(root->ses, root->list[index]->arg1, name, SUB_NONE))
						{
							show_nest_node(root->list[index], result, FALSE); // behaves like strcat
							count++;
						}
					}
					return count + 1;
				}
				else if (strstr(name, "..") && is_math(root->ses, name))
				{
					int min, max, range;

					if (root->used)
					{
						range = get_ellipsis(root, name, &min, &max);

						if (min < max)
						{
							while (min <= max)
							{
								show_nest_node(root->list[min++], result, FALSE);
							}
						}
						else
						{
							while (min >= max)
							{
								show_nest_node(root->list[min--], result, FALSE);
							}
						}
						return range + 1;
					}
					else
					{
						return 1;
					}
				}
				else
				{
					return 0;
				}
			}
		}

		root = search_nest_root(root, name);

		if (root)
		{
			if (!strcmp(arg, "[]"))
			{
				if (++warning < 100)
				{
					tintin_printf2(root->ses, "\n\e[1;5;31mdebug: please use *%s instead of $%s.\n", variable, variable);
				}

				for (index = 0 ; index < root->used ; index++)
				{
					str_cat_printf(result, "{%s}", root->list[index]->arg1);
				}
				return root->used + 1;
			}
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	return 0;
}


struct listnode *get_nest_node_key(struct listroot *root, char *variable, char **result, int def)
{
	struct listnode *node;
	int size;

	size = get_nest_size_key(root, variable, result); // will copy keys to result

	if (size)
	{
		return NULL;
	}

	node = search_nest_node(root, variable);

	if (node)
	{
		str_cpy_printf(result, "%s", node->arg1);

		if (node->shots && --node->shots == 0)
		{
			delete_nest_node(root, variable);
		}
		return node;
	}

	node = search_base_node(root, variable);

	if (node || def)
	{
		str_cpy(result, "");
	}
	else
	{
		str_cpy_printf(result, "*%s", variable);
	}
	return NULL;
}

struct listnode *get_nest_node_val(struct listroot *root, char *variable, char **result, int def)
{
	struct listnode *node;
	int size;

	size = get_nest_size_val(root, variable, result);

	if (size)
	{
		return NULL;
	}

	node = search_nest_node(root, variable);

	if (node)
	{
		show_nest_node(node, result, TRUE);

		if (node->shots && --node->shots == 0)
		{
			delete_nest_node(root, variable);
		}
		return node;
	}

	node = search_base_node(root, variable);

	if (node || def)
	{
		str_cpy(result, "");
	}
	else
	{
		str_cpy_printf(result, "$%s", variable);
	}
	return NULL;
}


int get_nest_index(struct listroot *root, char *variable, char **result, int def)
{
	struct listnode *node;
	int index, size;

	size = get_nest_size_index(root, variable, result);

	if (size)
	{
		str_cpy_printf(result, "%d", size - 1);

		return -1;
	}

	node = search_nest_node(root, variable);
	index = search_nest_index(root, variable);

	if (node && index >= 0)
	{
		str_cpy_printf(result, "%d", index + 1);

		if (node->shots && --node->shots == 0)
		{
			delete_index_list(root, index);
		}
		return index;
	}

	node = search_base_node(root, variable);

	if (node || def)
	{
		str_cpy(result, "0");
	}
	else
	{
		str_cpy_printf(result, "&%s", variable);
	}
	return -1;
}

// cats to result when initialize is 0

void show_nest_node(struct listnode *node, char **str_result, int initialize)
{
	if (initialize)
	{
		str_cpy(str_result, "");
	}

	if (node->root == NULL)
	{
		if (initialize)
		{
			str_cat(str_result, node->arg2);
		}
		else
		{
			str_cat_printf(str_result, "{%s}", node->arg2);
		}
	}
	else
	{
		struct listroot *root = node->root;
		int i;

		if (!initialize)
		{
			str_cat(str_result, "{");
		}

		for (i = 0 ; i < root->used ; i++)
		{
			str_cat_printf(str_result, "{%s}", root->list[i]->arg1);

			show_nest_node(root->list[i], str_result, FALSE);
		}

		if (!initialize)
		{
			str_cat(str_result, "}");
		}
	}
}

void view_nest_node(struct listnode *node, char **str_result, int nest, int initialize)
{
	if (initialize == TRUE)
	{
		str_cpy(str_result, "");
	}

	if (node->root == NULL)
	{
		if (initialize)
		{
			str_cat(str_result, node->arg2);
		}
		else
		{
			str_cat_printf(str_result, COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n", node->arg2);
		}
	}
	else
	{
		struct listroot *root = node->root;
		int i;

		if (initialize == FALSE)
		{
			str_cat_printf(str_result, "\n" COLOR_BRACE "%s{\n", indent(nest));
		}

		nest++;

		for (i = 0 ; i < root->used ; i++)
		{
			str_cat_printf(str_result, COLOR_BRACE "%s{" COLOR_STRING "%s" COLOR_BRACE "} ", indent(nest), root->list[i]->arg1);

			view_nest_node(root->list[i], str_result, nest, FALSE);
		}

		nest--;

		if (initialize == FALSE)
		{
			str_cat_printf(str_result, COLOR_BRACE "%s}\n", indent(nest), "");
		}
	}
}

struct listnode *set_nest_node_ses(struct session *ses, char *arg1, char *format, ...)
{
	struct listnode *node;
	struct listroot *root;
	char *arg, *arg2, name[BUFFER_SIZE];
	va_list args;

	push_call("set_nest_node_ses(%p,%s,%p,...)",ses,arg1,format);

	va_start(args, format);

	if (vasprintf(&arg2, format, args) == -1)
	{
		syserr_printf(ses, "set_nest_node_ses: vasprintf");
	}

	va_end(args);

	arg = get_arg_to_brackets(ses, arg1, name);

	if (HAS_BIT(ses->event_flags, EVENT_FLAG_VARIABLE))
	{
		check_all_events(ses, EVENT_FLAG_VARIABLE, 1, 2, "VARIABLE UPDATE %s", name, name, arg2);
	}

	root = search_nest_base_ses(ses, name);

	if (root == NULL)
	{
		if (gtd->level->local)
		{
			root = local_list(ses);
		}
		else
		{
			root = ses->list[LIST_VARIABLE];
		}
		node = NULL;
	}
	else
	{
		node = search_nest_node(root, arg1);
	}

	while (*arg)
	{
		root = update_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	node = search_node_list(root, name);

	if (node && node->root)
	{
		free_list(node->root);

		node->root = NULL;
	}

	if (*space_out(arg2) == DEFAULT_OPEN)
	{
		update_nest_node(update_nest_root(root, name), arg2);

		node = search_node_list(root, name);
	}
	else if (node)
	{
		str_cpy(&node->arg2, arg2);
	}
	else
	{
		node = update_node_list(root, name, arg2, "", "");
	}

	if (gtd->level->shots)
	{
		node->shots = gtd->level->mshot;
	}

	if (HAS_BIT(root->ses->event_flags, EVENT_FLAG_VARIABLE))
	{
		check_all_events(root->ses, EVENT_FLAG_VARIABLE, 1, 1, "VARIABLE UPDATED %s", name, name, arg2);
	}
	free(arg2);

	pop_call();
	return node;
}

// like set, but we're adding here.

struct listnode *add_nest_node_ses(struct session *ses, char *arg1, char *format, ...)
{
	struct listnode *node;
	struct listroot *root;
	char *arg, *arg2, name[BUFFER_SIZE];
	va_list args;

	push_call("add_nest_node_ses(%p,%s,%p,...)",ses,arg1,format);

	va_start(args, format);
	if (vasprintf(&arg2, format, args) == -1)
	{
		syserr_printf(ses, "add_nest_node_ses: vasprintf");
	}

	va_end(args);

	arg = get_arg_to_brackets(ses, arg1, name);

	if (HAS_BIT(ses->event_flags, EVENT_FLAG_VARIABLE))
	{
		check_all_events(ses, EVENT_FLAG_VARIABLE, 1, 2, "VARIABLE UPDATE %s", name, name, arg2);
	}

	root = search_nest_base_ses(ses, name);

	if (root == NULL)
	{
		root = ses->list[LIST_VARIABLE];
		node = NULL;
	}
	else
	{
		node = search_nest_node(root, arg1);
	}

	while (*arg)
	{
		root = update_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	node = search_node_list(root, name);

/*
	if (node && node->root)
	{
		free_list(node->root);

		node->root = NULL;
	}
*/

	if (*space_out(arg2) == DEFAULT_OPEN)
	{
		update_nest_node(update_nest_root(root, name), arg2);

		node = search_node_list(root, name);
	}
	else if (node)
	{
		str_cpy(&node->arg2, arg2);
	}
	else
	{
		node = update_node_list(root, name, arg2, "", "");
	}

	if (gtd->level->shots)
	{
		node->shots = gtd->level->mshot;
	}

	if (HAS_BIT(root->ses->event_flags, EVENT_FLAG_VARIABLE))
	{
		check_all_events(root->ses, EVENT_FLAG_VARIABLE, 1, 1, "VARIABLE UPDATED %s", name, name, arg2);
	}
	free(arg2);

	pop_call();
	return node;
}


struct listnode *set_nest_node(struct listroot *root, char *arg1, char *format, ...)
{
	struct listroot *base;
	struct listnode *node;
	char *arg, *arg2, name[BUFFER_SIZE];
	va_list args;

	push_call("set_nest_node(%p,%s,%p,...)",root,arg1,format);

	va_start(args, format);
	if (vasprintf(&arg2, format, args) == -1)
	{
		syserr_printf(root->ses, "set_nest_node: vasprintf");
	}

	va_end(args);

	arg = get_arg_to_brackets(root->ses, arg1, name);

	if (HAS_BIT(root->ses->event_flags, EVENT_FLAG_VARIABLE))
	{
		check_all_events(root->ses, EVENT_FLAG_VARIABLE, 1, 2, "VARIABLE UPDATE %s", name, name, arg2);
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_LOCAL))
	{
		base = search_nest_base_ses(root->ses, name);

		if (base)
		{
			root = base;
		}
	}

	while (*arg)
	{
		root = update_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	node = search_node_list(root, name);

	if (node && node->root)
	{
		free_list(node->root);

		node->root = NULL;
	}

	if (*space_out(arg2) == DEFAULT_OPEN)
	{
		update_nest_node(update_nest_root(root, name), arg2);

		node = search_node_list(root, name);
	}
	else if (node)
	{
		str_cpy(&node->arg2, arg2);
	}
	else
	{
		node = update_node_list(root, name, arg2, "", "");
	}

	if (gtd->level->shots)
	{
		node->shots = gtd->level->mshot;
	}

	if (HAS_BIT(root->ses->event_flags, EVENT_FLAG_VARIABLE))
	{
		check_all_events(root->ses, EVENT_FLAG_VARIABLE, 1, 1, "VARIABLE UPDATED %s", name, name, arg2);
	}

	free(arg2);

	pop_call();
	return node;
}

// Like set, but don't erase old data.

struct listnode *add_nest_node(struct listroot *root, char *arg1, char *format, ...)
{
	struct listroot *base;
	struct listnode *node;
	char *arg, *arg2, name[BUFFER_SIZE];
	va_list args;

	push_call("add_nest_node(%p,%s,%p,...)",root,arg1,format);

	va_start(args, format);
	if (vasprintf(&arg2, format, args) == -1)
	{
		syserr_printf(root->ses, "add_nest_node: vasprintf");
	}

	va_end(args);

	arg = get_arg_to_brackets(root->ses, arg1, name);

	if (HAS_BIT(root->ses->event_flags, EVENT_FLAG_VARIABLE))
	{
		check_all_events(root->ses, EVENT_FLAG_VARIABLE, 1, 2, "VARIABLE UPDATE %s", name, name, arg2);
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_LOCAL))
	{
		base = search_nest_base_ses(root->ses, name);

		if (base)
		{
			root = base;
		}
	}

	while (*arg)
	{
		root = update_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	node = search_node_list(root, name);
/*
	if (node && node->root)
	{
		free_list(node->root);

		node->root = NULL;
	}
*/

	if (*space_out(arg2) == DEFAULT_OPEN)
	{
		root = update_nest_root(root, name);

		update_nest_node(root, arg2);

		node = search_node_list(root, name);
	}
	else if (node)
	{
		str_cat(&node->arg2, arg2);
	}
	else
	{
		node = update_node_list(root, name, arg2, "", "");
	}

	if (gtd->level->shots)
	{
		node->shots = gtd->level->mshot;
	}

	if (HAS_BIT(root->ses->event_flags, EVENT_FLAG_VARIABLE))
	{
		check_all_events(root->ses, EVENT_FLAG_VARIABLE, 1, 1, "VARIABLE UPDATED %s", name, name, arg2);
	}

	free(arg2);

	pop_call();
	return node;
}


void copy_nest_node(struct listroot *dst_root, struct listnode *dst, struct listnode *src)
{
	int index;

	if (src->root == NULL)
	{
		return;
	}

	dst_root = dst->root = init_list(dst_root->ses, dst_root->type, src->root->size);

	for (index = 0 ; index < src->root->used ; index++)
	{
		dst = create_node_list(dst_root, src->root->list[index]->arg1, src->root->list[index]->arg2, src->root->list[index]->arg3, src->root->list[index]->arg4);

		if (src->root->list[index]->root)
		{
			copy_nest_node(dst_root, dst, src->root->list[index]);
		}
	}
}
