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
*                       coded by Igor van den Hoven 2004                      *
******************************************************************************/

#include "tintin.h"


struct listroot *init_list(struct session *ses, int type, int size)
{
	struct listroot *listhead;

	if ((listhead = (struct listroot *) calloc(1, sizeof(struct listroot))) == NULL)
	{
		syserr_fatal(-1, "init_list: calloc");
	}

	listhead->ses  = ses;
	listhead->list = (struct listnode **) calloc(size, sizeof(struct listnode *));
	listhead->size = size;
	listhead->type = type;

	listhead->flags = list_table[type].flags;

	return listhead;
}


void kill_list(struct listroot *root)
{
	while (root->used)
	{
		delete_index_list(root, root->used - 1);
	}
}

void free_list(struct listroot *root)
{
	kill_list(root);

	free(root->list);

	free(root);
}

struct listroot *copy_list(struct session *ses, struct listroot *sourcelist, int type)
{
	int i;
	struct listnode *node;

	push_call("copy_list(%p,%p,%p)",ses,sourcelist,type);

	ses->list[type] = init_list(ses, type, sourcelist->size);

	if (HAS_BIT(sourcelist->flags, LIST_FLAG_INHERIT))
	{
		for (i = 0 ; i < sourcelist->used ; i++)
		{
			node = (struct listnode *) calloc(1, sizeof(struct listnode));

			node->arg1  = strdup(sourcelist->list[i]->arg1);
			node->arg2  = strdup(sourcelist->list[i]->arg2);
			node->arg3  = strdup(sourcelist->list[i]->arg3);
			node->arg4  = strdup(sourcelist->list[i]->arg4);
			node->flags = sourcelist->list[i]->flags;
			node->group = strdup(sourcelist->list[i]->group);

			switch (type)
			{
				case LIST_ALIAS:
					node->regex = tintin_regexp_compile(ses, node, node->arg1, PCRE_ANCHORED);
					break;

				case LIST_ACTION:
				case LIST_GAG:
				case LIST_HIGHLIGHT:
				case LIST_PROMPT:
				case LIST_SUBSTITUTE:
					node->regex = tintin_regexp_compile(ses, node, node->arg1, 0);
					break;

				case LIST_BUTTON:
					node->val16[0] = sourcelist->list[i]->val16[0];
					node->val16[1] = sourcelist->list[i]->val16[1];
					node->val16[2] = sourcelist->list[i]->val16[2];
					node->val16[3] = sourcelist->list[i]->val16[3];
					break;

				case LIST_VARIABLE:
					copy_nest_node(ses->list[type], node, sourcelist->list[i]);
					break;
			}
			ses->list[type]->list[i] = node;
		}
		ses->list[type]->used = sourcelist->used;
	}
	ses->list[type]->flags = sourcelist->flags;

	pop_call();
	return ses->list[type];
}


struct listnode *insert_node_list(struct listroot *root, char *arg1, char *arg2, char *arg3, char *arg4)
{
	int index;
	struct listnode *node;

	node = (struct listnode *) calloc(1, sizeof(struct listnode));

	if (HAS_BIT(root->flags, LIST_FLAG_PRIORITY) && *arg3 == 0)
	{
		strcpy(arg3, "5");
	}

	node->arg1 = strdup(arg1);
	node->arg2 = strdup(arg2);
	node->arg3 = strdup(arg3);
	node->arg4 = strdup(arg4);

	if (gtd->level->oneshot)
	{
		SET_BIT(node->flags, NODE_FLAG_ONESHOT);
	}

	node->group = HAS_BIT(root->flags, LIST_FLAG_CLASS) ? strdup(root->ses->group) : strdup("");

	switch (root->type)
	{
		case LIST_ALIAS:
			node->regex = tintin_regexp_compile(root->ses, node, node->arg1, PCRE_ANCHORED);
			break;

		case LIST_ACTION:
		case LIST_GAG:
		case LIST_HIGHLIGHT:
		case LIST_PROMPT:
		case LIST_SUBSTITUTE:
			node->regex = tintin_regexp_compile(root->ses, node, node->arg1, 0);
			break;
	}

	index = locate_index_list(root, arg1, arg3);

	return insert_index_list(root, node, index);
}


struct listnode *update_node_list(struct listroot *root, char *arg1, char *arg2, char *arg3, char *arg4)
{
	int index;
	struct listnode *node;

	index = search_index_list(root, arg1, NULL);

	if (index != -1)
	{
		if (list_table[root->type].mode == SORT_DELAY && is_number(arg1))
		{
			return insert_node_list(root, arg1, arg2, arg3, arg4);
		}

		node = root->list[index];

		if (gtd->level->oneshot)
		{
			SET_BIT(node->flags, NODE_FLAG_ONESHOT);
		}

		if (strcmp(node->arg2, arg2) != 0)
		{
			free(node->arg2);
			node->arg2 = strdup(arg2);
		}

		switch (root->type)
		{
			case LIST_DELAY:
			case LIST_TICKER:
				node->data = 0;
				break;
		}

		if (HAS_BIT(root->flags, LIST_FLAG_PRIORITY) && *arg3 == 0)
		{
			strcpy(arg3, "5");
		}

		switch (list_table[root->type].mode)
		{
			case SORT_PRIORITY:
				if (atof(node->arg3) != atof(arg3))
				{
					delete_index_list(root, index);
					return insert_node_list(root, arg1, arg2, arg3, arg4);
				}
				break;

			case SORT_APPEND:
				delete_index_list(root, index);
				return insert_node_list(root, arg1, arg2, arg3, arg4);
				break;

			case SORT_ALPHA:
			case SORT_DELAY:
				if (strcmp(node->arg3, arg3) != 0)
				{
					free(node->arg3);
					node->arg3 = strdup(arg3);
				}
				break;

			default:
				tintin_printf2(root->ses, "#BUG: update_node_list: unknown mode: %d", list_table[root->type].mode);
				break;
		}
		return node;
	}
	else
	{
		return insert_node_list(root, arg1, arg2, arg3, arg4);
	}
}

struct listnode *insert_index_list(struct listroot *root, struct listnode *node, int index)
{
	root->used++;

	if (root->used == root->size)
	{
		root->size *= 2;

		root->list = (struct listnode **) realloc(root->list, (root->size) * sizeof(struct listnode *));
	}

	memmove(&root->list[index + 1], &root->list[index], (root->used - index) * sizeof(struct listnode *));

	root->list[index] = node;

	return node;
}

void delete_node_list(struct session *ses, int type, struct listnode *node)
{
	int index = search_index_list(ses->list[type], node->arg1, node->arg3);

	delete_index_list(ses->list[type], index);
}

void delete_index_list(struct listroot *root, int index)
{
	struct listnode *node = root->list[index];

	if (node->root)
	{
		free_list(node->root);
	}

	if (index <= root->update)
	{
		root->update--;
	}

	free(node->arg1);
	free(node->arg2);
	free(node->arg3);
	free(node->arg4);
	free(node->group);

	if (HAS_BIT(list_table[root->type].flags, LIST_FLAG_REGEX))
	{
		if (node->regex)
		{
			free(node->regex);
		}
	}
	free(node);

	memmove(&root->list[index], &root->list[index + 1], (root->used - index) * sizeof(struct listnode *));

	root->used--;

	return;
}

struct listnode *search_node_list(struct listroot *root, char *text)
{
	int index;

	push_call("search_node_list(%p,%p)",root,text);

	switch (list_table[root->type].mode)
	{
		case SORT_ALPHA:
		case SORT_DELAY:
			index = bsearch_alpha_list(root, text, 0);
			break;

		default:
			index = nsearch_list(root, text);
			break;
	}

	if (index != -1)
	{
		pop_call();
		return root->list[index];
	}
	pop_call();
	return NULL;
}

int search_index_list(struct listroot *root, char *text, char *priority)
{
	if (list_table[root->type].mode == SORT_ALPHA || list_table[root->type].mode == SORT_DELAY)
	{
		return bsearch_alpha_list(root, text, 0);
	}

	if (list_table[root->type].mode == SORT_PRIORITY && priority)
	{
		return bsearch_priority_list(root, text, priority, 0);
	}

	return nsearch_list(root, text);
}
 
/*
	Return insertion index.
*/

int locate_index_list(struct listroot *root, char *text, char *priority)
{
	switch (list_table[root->type].mode)
	{
		case SORT_ALPHA:
		case SORT_DELAY:
			return bsearch_alpha_list(root, text, 1);

		case SORT_PRIORITY:
			return bsearch_priority_list(root, text, priority, 1);

		default:
			return root->used;
	}
}

/*
	binary search on alphabetically sorted list
*/

int bsearch_alpha_list(struct listroot *root, char *text, int seek)
{
	long long bot, top, val;
	long double toi, toj, srt;

	push_call("bsearch_alpha_list(%p,%p,%d)",root,text,seek);

	bot = 0;
	top = root->used - 1;
	val = top;

//	toi = get_number(root->ses, text);

	if (seek == 0 && (*text == '+' || *text == '-') && is_math(root->ses, text) && HAS_BIT(root->flags, LIST_FLAG_NEST))
	{
		toi = get_number(root->ses, text);

		if (toi > 0 && toi <= root->used)
		{
			pop_call();
			return toi - 1;
		}
		if (toi < 0 && toi + root->used >= 0)
		{
			pop_call();
			return root->used + toi;
		}
		else
		{
			pop_call();
			return -1;
		}
	}

	toi = is_number(text) ? tintoi(text) : 0;

	while (bot <= top)
	{
		toj = is_number(root->list[val]->arg1) ? tintoi(root->list[val]->arg1) : 0;

		if (toi)
		{
			srt = toi - toj;
		}
		else if (toj)
		{
			srt = -1;
		}
		else
		{
			srt = strcmp(text, root->list[val]->arg1);
		}

		if (srt == 0)
		{
			pop_call();
			return val;
		}

		if (srt < 0)
		{
			top = val - 1;
		}
		else
		{
			bot = val + 1;
		}

		val = bot + (top - bot) / 2;
	}

	if (seek)
	{
		pop_call();
		return UMAX(0, val);
	}
	pop_call();
	return -1;
}

/*
	Binary search on priorially and alphabetically sorted list
*/

int bsearch_priority_list(struct listroot *root, char *text, char *priority, int seek)
{
	int bot, top, val;
	double srt;

	bot = 0;
	top = root->used - 1;
	val = top;

	while (bot <= top)
	{
		srt = atof(priority) - atof(root->list[val]->arg3);

		if (!srt)
		{
			srt = strcmp(text, root->list[val]->arg1);
		}

		if (srt == 0)
		{
			return val;
		}

		if (srt < 0)
		{
			top = val - 1;
		}
		else
		{
			bot = val + 1;
		}

		val = bot + (top - bot) / 2;
	}

	if (seek)
	{
		return UMAX(0, val);
	}
	else
	{
		return -1;
	}
}

/*
	Linear search
*/

int nsearch_list(struct listroot *root, char *text)
{
	int i;

	for (i = 0 ; i < root->used ; i++)
	{
		if (!strcmp(text, root->list[i]->arg1))
		{
			return i;
		}
	}
	return -1;
}

/*
	show content of a node on screen
*/

void show_node(struct listroot *root, struct listnode *node, int level)
{
	char *str_arg2 = str_dup("");

	show_nest_node(node, &str_arg2, TRUE);

	switch (list_table[root->type].args)
	{
		case 4:
			tintin_printf2(root->ses, "%s" COLOR_TINTIN "#" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}", indent(level), list_table[root->type].name, node->arg1, str_arg2, node->arg3, node->arg4);
			break;
		case 3:
			if (HAS_BIT(list_table[root->type].flags, LIST_FLAG_PRIORITY) && !strcmp(node->arg3, "5"))
			{
				tintin_printf2(root->ses, "%s" COLOR_TINTIN "#" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}", indent(level), list_table[root->type].name, node->arg1, str_arg2);
			}
			else
			{
				tintin_printf2(root->ses, "%s" COLOR_TINTIN "#" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}", indent(level), list_table[root->type].name, node->arg1, str_arg2, node->arg3);
			}
			break;
		case 2:
			tintin_printf2(root->ses, "%s" COLOR_TINTIN "#" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}", indent(level), list_table[root->type].name, node->arg1, str_arg2);
			break;
		case 1:
			tintin_printf2(root->ses, "%s" COLOR_TINTIN "#" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}", indent(level), list_table[root->type].name, node->arg1);
			break;
	}
	str_free(str_arg2);
}

/*
	list content of a list on screen
*/

void show_list(struct listroot *root, int level)
{
	int i;

	if (root == root->ses->list[root->type])
	{
		tintin_header(root->ses, " %s ", list_table[root->type].name_multi);
	}

	for (i = 0 ; i < root->used ; i++)
	{
		show_node(root, root->list[i], level);
	}
}


int show_node_with_wild(struct session *ses, char *text, struct listroot *root)
{
	struct listnode *node;
	int i, found = FALSE;

	push_call("show_node_with_wild(%p,%p,%p)",ses,text,root);

	node = search_node_list(root, text);

	if (node)
	{
		switch(root->type)
		{
			case LIST_EVENT:
			case LIST_FUNCTION:
			case LIST_MACRO:
				tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2));
				break;

			case LIST_ACTION:
			case LIST_ALIAS:
			case LIST_BUTTON:
				if (!strcmp(node->arg3, "5"))
				{
					tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2));
				}
				else
				{
					tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n{" COLOR_STRING "%s" COLOR_BRACE "}\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2), node->arg3);
				}
				break;

			case LIST_DELAY:
			case LIST_TICKER:
				tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n{" COLOR_STRING "%s" COLOR_BRACE "}\n\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2), node->arg3);
				break;

			default:
				show_node(root, node, 0);
				break;
		}
		pop_call();
		return TRUE;
	}

	for (i = 0 ; i < root->used ; i++)
	{
		if (match(ses, root->list[i]->arg1, text, SUB_VAR|SUB_FUN))
		{
			show_node(root, root->list[i], 0);

			found = TRUE;
		}
	}
	pop_call();
	return found;
}

void delete_node_with_wild(struct session *ses, int type, char *text)
{
	struct listroot *root = ses->list[type];
	struct listnode *node;
	char arg1[BUFFER_SIZE];
	int i, found = FALSE;

	sub_arg_in_braces(ses, text, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	node = search_node_list(root, arg1);

	if (node)
	{
		show_message(ses, type, "#OK. {%s} IS NO LONGER %s %s.", node->arg1, (*list_table[type].name == 'A' || *list_table[type].name == 'E') ? "AN" : "A", list_table[type].name);

		delete_node_list(ses, type, node);

		return;
	}

	for (i = root->used - 1 ; i >= 0 ; i--)
	{
		if (match(ses, root->list[i]->arg1, arg1, SUB_VAR|SUB_FUN))
		{
			show_message(ses, type, "#OK. {%s} IS NO LONGER %s %s.", root->list[i]->arg1, (*list_table[type].name == 'A' || *list_table[type].name == 'E') ? "AN" : "A", list_table[type].name);

			delete_index_list(root, i);

			found = TRUE;
		}
	}

	if (found == 0)
	{
		show_message(ses, type, "#KILL: NO MATCHES FOUND FOR %s {%s}.", list_table[type].name, arg1);
	}
}


DO_COMMAND(do_killall)
{
	tintin_printf2(ses, "\e[1;31m#NOTICE: PLEASE CHANGE #KILLALL TO #KILL ALL.");

	do_kill(ses, arg);

	return ses;
}

DO_COMMAND(do_kill)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int index;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	      get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 == 0 || !strcasecmp(arg1, "ALL"))
	{
		for (index = 0 ; index < LIST_MAX ; index++)
		{
			kill_list(ses->list[index]);
		}
		show_message(ses, LIST_COMMAND, "#KILL - ALL LISTS CLEARED.");

		return ses;
	}

	for (index = 0 ; index < LIST_MAX ; index++)
	{
		if (!is_abbrev(arg1, list_table[index].name) && !is_abbrev(arg1, list_table[index].name_multi))
		{
			continue;
		}

		if (*arg2 == 0 || !strcasecmp(arg2, "ALL"))
		{
			kill_list(ses->list[index]);

			show_message(ses, LIST_COMMAND, "#OK: #%s LIST CLEARED.", list_table[index].name);
		}
		else
		{
			delete_node_with_wild(ses, index, arg);
		}
		break;
	}

	if (index == LIST_MAX)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #KILL {%s} {%s} - NO MATCH FOUND.", arg1, arg2);
	}
	return ses;
}


DO_COMMAND(do_message)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int index, found = FALSE;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_header(ses, " MESSAGES ");

		for (index = 0 ; index < LIST_MAX ; index++)
		{
			if (!HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				tintin_printf2(ses, "  %-20s %3s", list_table[index].name_multi, HAS_BIT(ses->list[index]->flags, LIST_FLAG_MESSAGE) ? "ON" : "OFF");
			}
		}

		tintin_header(ses, "");
	}
	else
	{
		for (index = found = 0 ; index < LIST_MAX ; index++)
		{
			if (HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				continue;
			}

			if (!is_abbrev(arg1, list_table[index].name) && !is_abbrev(arg1, list_table[index].name_multi) && strcasecmp(arg1, "ALL"))
			{
				continue;
			}

			if (*arg2 == 0)
			{
				TOG_BIT(ses->list[index]->flags, LIST_FLAG_MESSAGE);
			}
			else if (is_abbrev(arg2, "ON"))
			{
				SET_BIT(ses->list[index]->flags, LIST_FLAG_MESSAGE);
			}
			else if (is_abbrev(arg2, "OFF"))
			{
				DEL_BIT(ses->list[index]->flags, LIST_FLAG_MESSAGE);
			}
			else
			{
				show_error(ses, LIST_COMMAND, "#SYNTAX: #MESSAGE {%s} [ON|OFF]",  arg1);
				
				return ses;
			}
			show_message(ses, LIST_COMMAND, "#OK: #%s MESSAGES HAVE BEEN SET TO: %s.", list_table[index].name, HAS_BIT(ses->list[index]->flags, LIST_FLAG_MESSAGE) ? "ON" : "OFF");

			found = TRUE;
		}

		if (found == FALSE)
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #MESSAGE {%s} - NO MATCH FOUND.", arg1);
		}
	}
	return ses;
}


DO_COMMAND(do_ignore)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int index, found = FALSE;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_header(ses, " IGNORES ");

		for (index = 0 ; index < LIST_MAX ; index++)
		{
			if (!HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				tintin_printf2(ses, "  %-20s %3s", list_table[index].name_multi, HAS_BIT(ses->list[index]->flags, LIST_FLAG_IGNORE) ? "ON" : "OFF");
			}
		}

		tintin_header(ses, "");
	}
	else
	{
		for (index = found = 0 ; index < LIST_MAX ; index++)
		{
			if (HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				continue;
			}

			if (!is_abbrev(arg1, list_table[index].name) && !is_abbrev(arg1, list_table[index].name_multi) && strcasecmp(arg1, "ALL"))
			{
				continue;
			}

			if (*arg2 == 0)
			{
				TOG_BIT(ses->list[index]->flags, LIST_FLAG_IGNORE);
			}
			else if (is_abbrev(arg2, "ON"))
			{
				SET_BIT(ses->list[index]->flags, LIST_FLAG_IGNORE);
			}
			else if (is_abbrev(arg2, "OFF"))
			{
				DEL_BIT(ses->list[index]->flags, LIST_FLAG_IGNORE);
			}
			else
			{
				show_error(ses, LIST_COMMAND, "#SYNTAX: #IGNORE {%s} [ON|OFF]", arg1);
				
				return ses;
			}
			show_message(ses, LIST_COMMAND, "#OK: #%s IGNORE STATUS HAS BEEN SET TO: %s.", list_table[index].name, HAS_BIT(ses->list[index]->flags, LIST_FLAG_IGNORE) ? "ON" : "OFF");

			found = TRUE;
		}

		if (found == FALSE)
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #IGNORE {%s} - NO MATCH FOUND.", arg1);
		}
	}
	return ses;
}


DO_COMMAND(do_debug)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int index, found = FALSE;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_header(ses, " DEBUGS ");

		for (index = 0 ; index < LIST_MAX ; index++)
		{
			if (!HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				tintin_printf2(ses, "  %-20s %3s", list_table[index].name_multi, HAS_BIT(ses->list[index]->flags, LIST_FLAG_DEBUG) ? "ON" : "OFF");
			}
		}

		tintin_header(ses, "");
	}
	else
	{
		for (index = found = 0 ; index < LIST_MAX ; index++)
		{
			if (HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				continue;
			}

			if (!is_abbrev(arg1, list_table[index].name) && !is_abbrev(arg1, list_table[index].name_multi) && strcasecmp(arg1, "ALL"))
			{
				continue;
			}

			if (*arg2 == 0)
			{
				TOG_BIT(ses->list[index]->flags, LIST_FLAG_DEBUG);
			}
			else if (is_abbrev(arg2, "ON"))
			{
				SET_BIT(ses->list[index]->flags, LIST_FLAG_DEBUG);
			}
			else if (is_abbrev(arg2, "OFF"))
			{
				DEL_BIT(ses->list[index]->flags, LIST_FLAG_DEBUG);
				DEL_BIT(ses->list[index]->flags, LIST_FLAG_LOG);
			}
			else if (is_abbrev(arg2, "LOG"))
			{
				SET_BIT(ses->list[index]->flags, LIST_FLAG_LOG);
			}
			else
			{
				show_error(ses, LIST_COMMAND, "#SYNTAX: #DEBUG {%s} [ON|OFF|LOG]", arg1);
				
				return ses;
			}
			show_message(ses, LIST_COMMAND, "#OK: #%s DEBUG STATUS HAS BEEN SET TO: %s.", list_table[index].name, is_abbrev(arg2, "LOG") ? "LOG" : HAS_BIT(ses->list[index]->flags, LIST_FLAG_DEBUG) ? "ON" : "OFF");

			found = TRUE;
		}

		if (found == FALSE)
		{
			show_error(ses, LIST_COMMAND, "#DEBUG {%s} - NO MATCH FOUND.", arg1);
		}
	}
	return ses;
}

DO_COMMAND(do_info)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], name[BUFFER_SIZE];
	int cnt, index, found = FALSE;
	struct listroot *root;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_header(ses, " INFORMATION ");

		for (index = 0 ; index < LIST_MAX ; index++)
		{
			if (!HAS_BIT(ses->list[index]->flags, LIST_FLAG_HIDE))
			{
				tintin_printf2(ses, "%-15s %5d   IGNORE %3s   MESSAGE %3s   INFO %3s   DEBUG %3s %3s",
					list_table[index].name_multi,
					ses->list[index]->used,
					HAS_BIT(ses->list[index]->flags, LIST_FLAG_IGNORE)  ?  "ON" : "OFF",
					HAS_BIT(ses->list[index]->flags, LIST_FLAG_MESSAGE) ?  "ON" : "OFF",
					HAS_BIT(ses->list[index]->flags, LIST_FLAG_INFO)    ?  "ON" : "OFF",
					HAS_BIT(ses->list[index]->flags, LIST_FLAG_DEBUG)   ?  "ON" : "OFF",
					HAS_BIT(ses->list[index]->flags, LIST_FLAG_LOG)     ? "LOG" : "   ");
			}
		}
		tintin_header(ses, "");
	}
	else
	{
		for (index = found = 0 ; index < LIST_MAX ; index++)
		{
			if (HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				continue;
			}

			if (!is_abbrev(arg1, list_table[index].name) && !is_abbrev(arg1, list_table[index].name_multi) && strcasecmp(arg1, "ALL"))
			{
				continue;
			}

			if (*arg2 == 0)
			{
				TOG_BIT(ses->list[index]->flags, LIST_FLAG_INFO);
			}
			else if (is_abbrev(arg2, "ON"))
			{
				SET_BIT(ses->list[index]->flags, LIST_FLAG_INFO);
			}
			else if (is_abbrev(arg2, "OFF"))
			{
				DEL_BIT(ses->list[index]->flags, LIST_FLAG_INFO);
			}
			else
			{
				root = ses->list[index];

				if (is_abbrev(arg2, "LIST"))
				{
					for (cnt = 0 ; cnt < root->used ; cnt++)
					{
						tintin_printf2(ses, "#INFO %s %4d {arg1}{%s} {arg2}{%s} {arg3}{%s} {arg4}{%s} {class}{%s} {flags}{%d}", list_table[index].name, cnt, root->list[cnt]->arg1, root->list[cnt]->arg2, root->list[cnt]->arg3, root->list[cnt]->arg4, root->list[cnt]->group, root->list[cnt]->flags);
					}
				}
				else if (is_abbrev(arg2, "SAVE"))
				{
					sprintf(name, "info[%s]", list_table[index].name);
					delete_nest_node(ses->list[LIST_VARIABLE], name);

					for (cnt = 0 ; cnt < root->used ; cnt++)
					{
						sprintf(name, "info[%s][%d]", list_table[index].name, cnt);

						set_nest_node(ses->list[LIST_VARIABLE], name, "{arg1}{%s}{arg2}{%s}{arg3}{%s}{arg4}{%s}{class}{%s}{flags}{%d}", root->list[cnt]->arg1, root->list[cnt]->arg2, root->list[cnt]->arg3, root->list[cnt]->arg4, root->list[cnt]->group, root->list[cnt]->flags);
					}
					show_message(ses, LIST_COMMAND, "#INFO: DATA WRITTEN TO {info[%s]}", list_table[index].name);
				}
				else
				{
					show_error(ses, LIST_COMMAND, "#SYNTAX: #INFO {%s} [ON|OFF|LIST|SAVE|SYSTEM]", arg1);
				}
				return ses;
			}
			show_message(ses, LIST_COMMAND, "#OK: #%s INFO STATUS HAS BEEN SET TO: %s.", list_table[index].name, HAS_BIT(ses->list[index]->flags, LIST_FLAG_INFO) ? "ON" : "OFF");

			found = TRUE;
		}

		if (found == FALSE)
		{
			if (is_abbrev(arg1, "CPU"))
			{
				show_cpu(ses);
			}
			else if (is_abbrev(arg1, "MCCP"))
			{
				if (ses->mccp2)
				{
					tintin_printf2(ses, "#INFO MCCP2: TOTAL IN: %9ull TOTAL OUT: %9ull RATIO: %3d", ses->mccp2->total_in, ses->mccp2->total_out, 100 * ses->mccp2->total_out / ses->mccp2->total_in);
				}
				if (ses->mccp3)
				{
					tintin_printf2(ses, "#INFO MCCP3: TOTAL IN: %9ull TOTAL OUT: %9ull RATIO: %3d", ses->mccp3->total_in, ses->mccp3->total_out, 100 * ses->mccp3->total_out / ses->mccp3->total_in);
				}
			}
			else if (is_abbrev(arg1, "STACK"))
			{
				dump_stack();
			}
			else if (is_abbrev(arg1, "SYSTEM"))
			{
				if (is_abbrev(arg2, "SAVE"))
				{
					sprintf(name, "info[SYSTEM]");

					set_nest_node(ses->list[LIST_VARIABLE], name, "{CLIENT_NAME}{%s}{CLIENT_VERSION}{%-3s}", CLIENT_NAME, CLIENT_VERSION);

					add_nest_node(ses->list[LIST_VARIABLE], name, "{CLIENT}{{NAME}{%s}{VERSION}{%-3s}}", CLIENT_NAME, CLIENT_VERSION);

					add_nest_node(ses->list[LIST_VARIABLE], name, "{EXEC}{%s}{HOME}{%s}{LANG}{%s}{OS}{%s}{TERM}{%s}", gtd->exec, gtd->home, gtd->lang, gtd->os, gtd->term);

					set_nest_node(ses->list[LIST_VARIABLE], name, "{DETACH_FILE}{%s}{ATTACH_FILE}{%-3s}", gtd->detach_port ? gtd->detach_file : "", gtd->attach_pid  ? gtd->attach_file : "");

					show_message(ses, LIST_COMMAND, "#INFO: DATA WRITTEN TO {info[SYSTEM]}");
				}
				else
				{
					tintin_printf2(ses, "#INFO SYSTEM: CLIENT_NAME    = %s", CLIENT_NAME);
					tintin_printf2(ses, "#INFO SYSTEM: CLIENT_VERSION = %s", CLIENT_VERSION);
					tintin_printf2(ses, "#INFO SYSTEM: EXEC           = %s", gtd->exec);
					tintin_printf2(ses, "#INFO SYSTEM: HOME           = %s", gtd->home);
					tintin_printf2(ses, "#INFO SYSTEM: LANG           = %s", gtd->lang);
					tintin_printf2(ses, "#INFO SYSTEM: OS             = %s", gtd->os);
					tintin_printf2(ses, "#INFO SYSTEM: TERM           = %s", gtd->term);
					tintin_printf2(ses, "#INFO SYSTEM: DETACH_FILE    = %s", gtd->detach_port ? gtd->detach_file : "");
					tintin_printf2(ses, "#INFO SYSTEM: ATTACH_FILE    = %s", gtd->attach_pid  ? gtd->attach_file : "");
				}
			}
			else
			{
				show_error(ses, LIST_COMMAND, "#INFO {%s} - NO MATCH FOUND.", arg1);
			}
		}
	}
	return ses;

}
