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
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"

#include <limits.h>

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
	root->update = 0;
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

	push_call("copy_list(%p,%p,%s)",ses,sourcelist,list_table[type].name);

	ses->list[type] = init_list(ses, type, sourcelist->size);

	if (HAS_BIT(sourcelist->flags, LIST_FLAG_INHERIT))
	{
		for (i = 0 ; i < sourcelist->used ; i++)
		{
			node = (struct listnode *) calloc(1, sizeof(struct listnode));

			node->arg1  = str_dup_clone(sourcelist->list[i]->arg1);
			node->arg2  = str_dup_clone(sourcelist->list[i]->arg2);
			node->arg3  = str_dup_clone(sourcelist->list[i]->arg3);
			node->arg4  = str_dup_clone(sourcelist->list[i]->arg4);
			node->shots = sourcelist->list[i]->shots;
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
				case LIST_DELAY:
				case LIST_EVENT:
				case LIST_TICKER:
				case LIST_PATHDIR:
					node->val64 = sourcelist->list[i]->val64;
/*
					node->val16[0] = sourcelist->list[i]->val16[0];
					node->val16[1] = sourcelist->list[i]->val16[1];
					node->val16[2] = sourcelist->list[i]->val16[2];
					node->val16[3] = sourcelist->list[i]->val16[3];
*/
					break;

				case LIST_VARIABLE:
					copy_nest_node(ses->list[type], node, sourcelist->list[i]);
					break;

				default:
					if (sourcelist->list[i]->val64)
					{
						printf("copy_list: unhandled val64 (%d).\n", type);
					}
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

struct listnode *create_node_list(struct listroot *root, char *arg1, char *arg2, char *arg3, char *arg4)
{
	struct listnode *node;

	node = (struct listnode *) calloc(1, sizeof(struct listnode));

	if (list_table[root->type].priority_arg == 3 && *arg3 == 0)
	{
		strcpy(arg3, "5");
	}

	if (HAS_BIT(root->flags, LIST_FLAG_NEST) && *arg1 == '\\')
	{
		node->arg1 = str_dup(arg1+1);
	}
	else
	{
		node->arg1 = str_dup(arg1);
	}
	node->arg2 = str_dup(arg2);
	node->arg3 = str_dup(arg3);
	node->arg4 = str_dup(arg4);

//	printf("debug: %p [%p] (%d) (%s) (%s)\n", root, root->ses, root->type, node->arg1, node->arg3);

	switch (root->type)
	{
		case LIST_DELAY:
			node->val64 = (long long) tintoi(node->arg1);

			if (node->val64 < gtd->utime_next_delay)
			{
				gtd->utime_next_delay = node->val64;
			}
			break;

		case LIST_TICKER:
			node->val64 = gtd->utime + (long long) tintoi(arg3) * 1000000.0;

			if (node->val64 < gtd->utime_next_tick)
			{
				gtd->utime_next_tick = node->val64;
			}
			break;
	}

	if (gtd->level->shots)
	{
		node->shots = gtd->level->mshot;
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

	return insert_node_list(root, node);
}

struct listnode *insert_node_list(struct listroot *root, struct listnode *node)
{
	int index;

	index = locate_index_list(root, node->arg1, node->arg3);

	return insert_index_list(root, node, index);
}


struct listnode *update_node_list(struct listroot *root, char *arg1, char *arg2, char *arg3, char *arg4)
{
	int index;
	struct listnode *node;

	index = search_index_list(root, arg1, NULL);

	if (index != -1)
	{
		node = root->list[index];

		if (gtd->level->shots)
		{
			node->shots = gtd->level->mshot;
		}

		if (strcmp(node->arg2, arg2) != 0)
		{
			str_cpy(&node->arg2, arg2);
		}

		switch (root->type)
		{
			case LIST_TICKER:
				node->val64 = gtd->utime + (long long) tintoi(arg3) * 1000000;

				if (node->val64 < gtd->utime_next_tick)
				{
					gtd->utime_next_tick = node->val64;
				}
				break;
		}

		if (list_table[root->type].priority_arg == 3 && *arg3 == 0)
		{
			strcpy(arg3, "5");
		}

		if (list_table[root->type].mode != SORT_PRIORITY)
		{
			if (strcmp(node->arg3, arg3) != 0)
			{
				str_cpy(&node->arg3, arg3);
			}
		}

		if (strcmp(node->arg4, arg4) != 0)
		{
			str_cpy(&node->arg4, arg4);
		}

		switch (list_table[root->type].mode)
		{
			case SORT_PRIORITY:
				if (strcmp(node->arg3, arg3))
				{
					remove_index_list(root, index);
					str_cpy(&node->arg3, arg3);
					insert_node_list(root, node);
				}
				break;

			case SORT_APPEND:
				remove_index_list(root, index);
				insert_node_list(root, node);
				break;

			case SORT_ALPHA:
			case SORT_ALNUM:
			case SORT_STABLE:
				break;

			default:
				tintin_printf2(root->ses, "#BUG: update_node_list: unknown sort: %d", list_table[root->type].mode);
				break;
		}
		return node;
	}
	else
	{
		return create_node_list(root, arg1, arg2, arg3, arg4);
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

	if (index + 1 < root->used)
	{
		memmove(&root->list[index + 1], &root->list[index], (root->used - index) * sizeof(struct listnode *));
	}

	root->list[index] = node;

	return node;
}

void remove_node_list(struct session *ses, int type, struct listnode *node)
{
	int index = search_index_list(ses->list[type], node->arg1, node->arg3);

	remove_index_list(ses->list[type], index);
}

void remove_index_list(struct listroot *root, int index)
{
	if (index <= root->update)
	{
		root->update--;
	}

	memmove(&root->list[index], &root->list[index + 1], (root->used - index) * sizeof(struct listnode *));

	root->used--;

	return;
}

void delete_node_list(struct session *ses, int type, struct listnode *node)
{
	int index = search_index_list(ses->list[type], node->arg1, node->arg3);

	delete_index_list(ses->list[type], index);
}

void delete_index_list(struct listroot *root, int index)
{
	struct listnode *node = root->list[index];

	if (HAS_BIT(list_table[root->type].flags, LIST_FLAG_REGEX))
	{
		if (node->regex)
		{
			free(node->regex);
		}
	}

	switch (root->type)
	{
		case LIST_TERRAIN:
			delete_room_data(node->room);
			free(node->room);
			break;

		case LIST_CLASS:
			if (node->data)
			{
				free(node->data);
			}
			break;

		case LIST_EVENT:
			event_table[node->val32[0]].level--;
			break;
	}

	remove_index_list(root, index);

	// dispose in memory update for one shot handling

	insert_index_list(gtd->dispose_list, node, gtd->dispose_list->used);
}

void dispose_node(struct listnode *node)
{
	if (node->root)
	{
		free_list(node->root);
	}

	str_free(node->arg1);
	str_free(node->arg2);
	str_free(node->arg3);
	str_free(node->arg4);

	free(node->group);

	free(node);
}

struct listnode *search_node_list(struct listroot *root, char *text)
{
	int index;

	push_call("search_node_list(%p,%p)",root,text);

	switch (list_table[root->type].mode)
	{
		case SORT_ALPHA:
		case SORT_STABLE:
			index = bsearch_alpha_list(root, text, 0);
			break;

		case SORT_ALNUM:
			index = bsearch_alnum_list(root, text, 0);
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
	switch (list_table[root->type].mode)
	{
		case SORT_ALPHA:
		case SORT_STABLE:
			return bsearch_alpha_list(root, text, 0);
		
		case SORT_ALNUM:
			return bsearch_alnum_list(root, text, 0);

		case SORT_PRIORITY:
			if (priority)
			{
				bsearch_priority_list(root, text, priority, 0);
			}
			break;
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
			return bsearch_alpha_list(root, text, SEEK_REPLACE);

		case SORT_STABLE:
			return bsearch_alpha_list(root, text, SEEK_APPEND);

		case SORT_ALNUM:
			return bsearch_alnum_list(root, text, SEEK_REPLACE);

		case SORT_PRIORITY:
			return bsearch_priority_list(root, text, priority, SEEK_REPLACE);

		default:
			return root->used;
	}
}

int bsearch_alpha_list(struct listroot *root, char *text, int seek)
{
	int bot, mid, top;

	if (root->used == 0)
	{
		return seek ? 0 : -1;
	}

	bot = 0;
	top = root->used;

	while (top > 1)
	{
		mid = top / 2;

		if (strcmp(text, root->list[bot + mid]->arg1) >= 0)
		{
			bot += mid;
		}
		top -= mid;
	}

	if (strcmp(text, root->list[bot]->arg1) == 0)
	{
		return bot + (seek == SEEK_APPEND);
	}

	if (seek)
	{
		return bot + (strcmp(text, root->list[bot]->arg1) > 0);
	}

	return -1;
}

/*
int bsearch_alpha_list(struct listroot *root, char *text, int seek)
{
	int bot, top, val, srt;

	bot = 0;
	top = root->used - 1;
	val = top;

	while (bot <= top)
	{
		srt = strcmp(text, root->list[val]->arg1);

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
	return -1;
}
*/

int bsearch_alnum_list(struct listroot *root, char *text, int seek)
{
	long double toi, toj;
	int bot, top, val, noi, noj, srt;

	push_call("bsearch_alpha_list(%p,%p,%d)",root,text,seek);

	if (seek == 0 && HAS_BIT(root->flags, LIST_FLAG_NEST))
	{
		switch (*text)
		{
			case '+':
				toi = get_number(root->ses, text);

				if (seek == 0)
				{
					if (toi > 0 && toi <= root->used)
					{
						pop_call();
						return toi - 1;
					}
				}
				else
				{
					if (toi >= 0 && toi <= root->used)
					{
						pop_call();
						return UMAX(0, toi - 1);
					}
				}
				break;

			case '-':
				toi = get_number(root->ses, text);

				if (toi < 0 && root->used + toi >= 0)
				{
					pop_call();
					return root->used + toi;
				}
				break;

			case '\\':
				text++;
				break;
		}
	}

	bot = 0;
	top = root->used - 1;
	val = top;

	noi = is_number(text);

	toi = noi ? tintoi(text) : 0;

	while (bot <= top)
	{
		noj = is_number(root->list[val]->arg1);

		toj = noj ? tintoi(root->list[val]->arg1) : 0;

		if (noi)
		{
			srt = (toi < toj) ? -1 : (toi > toj) ? 1 : 0;
		}
		else if (noj)
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
	long double prt, srt;

	bot = 0;
	top = root->used - 1;
	val = top;
	prt = tintoi(priority);

	while (bot <= top)
	{
		srt = tintoi(root->list[val]->arg3);

		if (srt == prt)
		{
			srt = strcmp(text, root->list[val]->arg1);

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
		}
		else
		{
			if (prt < srt)
			{
				top = val - 1;
			}
			else
			{
				bot = val + 1;
			}
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
	char *str_arg2;

	push_call("show_node(%p,%p,%d)",root,node,level);

	str_arg2 = str_alloc_stack(0);

	show_nest_node(node, &str_arg2, TRUE);

	switch (list_table[root->type].args)
	{
		case 4:
			tintin_printf2(root->ses, "%s" COLOR_TINTIN "#" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}", indent(level), list_table[root->type].name, node->arg1, str_arg2, node->arg3, node->arg4);
			break;
		case 3:
			if (list_table[root->type].priority_arg == 3 && !strcmp(node->arg3, "5"))
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
	pop_call();
	return;
}

/*
	list content of a list on screen
*/

void show_list(struct listroot *root, int level)
{
	int i;

	if (root == root->ses->list[root->type])
	{
		tintin_header(root->ses, 80, " %s ", list_table[root->type].name_multi);
	}

	for (i = 0 ; i < root->used ; i++)
	{
		show_node(root, root->list[i], level);
	}
}


int show_node_with_wild(struct session *ses, char *text, struct listroot *root)
{
	struct listnode *node;
	int index, found = FALSE;

	push_call("show_node_with_wild(%p,%p,%p)",ses,text,root);

	switch (list_table[root->type].mode)
	{
		case SORT_ALPHA:
		case SORT_STABLE:
			index = bsearch_alpha_list(root, text, 0);
			break;

		case SORT_ALNUM:
			index = bsearch_alnum_list(root, text, 0);
			break;

		default:
			index = nsearch_list(root, text);
			break;
	}

	if (index != -1)
	{
		node = root->list[index];

		if (list_table[root->type].script_arg == 2)
		{
			if (list_table[root->type].args == 2)
			{
				tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2));
			}
			else if (list_table[root->type].args == 3)
			{
				if (list_table[root->type].priority_arg == 3)
				{
					if (!strcmp(node->arg3, "5"))
					{
						tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2));
					}
					else
					{
						tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n{" COLOR_STRING "%s" COLOR_BRACE "}\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2), node->arg3);
					}
				}
				else
				{
					tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n{" COLOR_STRING "%s" COLOR_BRACE "}\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2), node->arg3);
				}
			}
		}
		else
		{
			show_node(root, node, 0);
		}

		switch(root->type)
		{
//			case LIST_EVENT:
//			case LIST_FUNCTION:
//			case LIST_MACRO:
//				tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2));
//				break;

//			case LIST_ACTION:
//			case LIST_ALIAS:
//			case LIST_BUTTON:
//				if (!strcmp(node->arg3, "5"))
//				{
//					tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2));
//				}
//				else
//				{
//					tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n{" COLOR_STRING "%s" COLOR_BRACE "}\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2), node->arg3);
//				}
//				break;

//			case LIST_DELAY:
//			case LIST_TICKER:
//				tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s\n" COLOR_BRACE "}\n{" COLOR_STRING "%s" COLOR_BRACE "}\n\n", gtd->tintin_char, list_table[root->type].name, node->arg1, script_viewer(ses, node->arg2), node->arg3);
//				break;

//			default:
//				show_node(root, node, 0);
//				break;
		}
		pop_call();
		return TRUE;
	}

	for (index = 0 ; index < root->used ; index++)
	{
		if (match(ses, root->list[index]->arg1, text, SUB_VAR|SUB_FUN))
		{
			show_node(root, root->list[index], 0);

			found = TRUE;
		}
	}
	pop_call();
	return found;
}

int delete_node_with_wild(struct session *ses, int type, char *text)
{
	struct listroot *root = ses->list[type];
	struct listnode *node;
	char arg1[BUFFER_SIZE];
	int index, found = FALSE;

	sub_arg_in_braces(ses, text, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	switch (list_table[type].mode)
	{
		case SORT_ALPHA:
		case SORT_STABLE:
			index = bsearch_alpha_list(root, arg1, 0);
			break;

		case SORT_ALNUM:
			index = bsearch_alnum_list(root, arg1, 0);
			break;

		default:
			index = nsearch_list(root, arg1);
			break;
	}

	if (index != -1)
	{
		node = root->list[index];

		show_message(ses, type, "#OK. {%s} IS NO LONGER %s %s.", node->arg1, (*list_table[type].name == 'A' || *list_table[type].name == 'E') ? "AN" : "A", list_table[type].name);

		delete_index_list(root, index);

		return TRUE;
	}

	for (index = root->used - 1 ; index >= 0 ; index--)
	{
		if (match(ses, root->list[index]->arg1, arg1, SUB_VAR|SUB_FUN))
		{
			show_message(ses, type, "#OK. {%s} IS NO LONGER %s %s.", root->list[index]->arg1, is_vowel(list_table[type].name) ? "AN" : "A", list_table[type].name);

			delete_index_list(root, index);

			found = TRUE;
		}
	}

	if (found == 0)
	{
		show_message(ses, type, "#KILL: NO MATCHES FOUND FOR %s {%s}.", list_table[type].name, arg1);

		return FALSE;
	}
	return TRUE;
}


DO_COMMAND(do_killall)
{
	tintin_printf2(ses, "\e[1;31m#NOTICE: PLEASE CHANGE #KILLALL TO #KILL ALL.");

	do_kill(ses, arg, arg1, arg2, arg3, arg4);

	return ses;
}

DO_COMMAND(do_kill)
{
	int index;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	      get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 == 0 || !strcasecmp(arg1, "ALL"))
	{
		for (index = 0 ; index < LIST_MAX ; index++)
		{
			if (!HAS_BIT(ses->list[index]->flags, LIST_FLAG_HIDE))
			{
				kill_list(ses->list[index]);
			}
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
	int index, found = FALSE;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " MESSAGES ");

		for (index = 0 ; index < LIST_MAX ; index++)
		{
			if (!HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				tintin_printf2(ses, "  %-20s %3s", list_table[index].name_multi, HAS_BIT(ses->list[index]->flags, LIST_FLAG_MESSAGE) ? "ON" : "OFF");
			}
		}

		tintin_header(ses, 80, "");
	}
	else
	{
		for (index = found = 0 ; index < LIST_MAX ; index++)
		{
			if (HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				continue;
			}

			if (!is_abbrev(arg1, list_table[index].name_multi) && strcasecmp(arg1, "ALL"))
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
			show_message(ses, LIST_COMMAND, "#OK: #MESSAGE STATUS FOR %s HAS BEEN SET TO: %s.", list_table[index].name_multi, HAS_BIT(ses->list[index]->flags, LIST_FLAG_MESSAGE) ? "ON" : "OFF");

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
	int index, found = FALSE;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " IGNORES ");

		for (index = 0 ; index < LIST_MAX ; index++)
		{
			if (!HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				tintin_printf2(ses, "  %-20s %3s", list_table[index].name_multi, HAS_BIT(ses->list[index]->flags, LIST_FLAG_IGNORE) ? "ON" : "OFF");
			}
		}

		tintin_header(ses, 80, "");
	}
	else
	{
		for (index = found = 0 ; index < LIST_MAX ; index++)
		{
			if (HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				continue;
			}

			if (!is_abbrev(arg1, list_table[index].name_multi) && strcasecmp(arg1, "ALL"))
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
			show_message(ses, LIST_COMMAND, "#OK: #IGNORE STATUS FOR %s HAS BEEN SET TO: %s.", list_table[index].name_multi, HAS_BIT(ses->list[index]->flags, LIST_FLAG_IGNORE) ? "ON" : "OFF");

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
	int index, found = FALSE;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " DEBUGS ");

		for (index = 0 ; index < LIST_MAX ; index++)
		{
			if (!HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				tintin_printf2(ses, "  %-20s %3s", list_table[index].name_multi, HAS_BIT(ses->list[index]->flags, LIST_FLAG_DEBUG) ? "ON" : "OFF");
			}
		}

		tintin_header(ses, 80, "");
	}
	else
	{
		for (index = found = 0 ; index < LIST_MAX ; index++)
		{
			if (HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				continue;
			}

			if (!is_abbrev(arg1, list_table[index].name_multi) && strcasecmp(arg1, "ALL"))
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
			show_message(ses, LIST_COMMAND, "#OK: #DEBUG STATUS FOR %s HAS BEEN SET TO: %s.", list_table[index].name_multi, is_abbrev(arg2, "LOG") ? "LOG" : HAS_BIT(ses->list[index]->flags, LIST_FLAG_DEBUG) ? "ON" : "OFF");

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
	char name[BUFFER_SIZE];
	int cnt, index, found = FALSE;
	struct listroot *root;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " INFORMATION ");

		for (index = 0 ; index < LIST_MAX ; index++)
		{
			if (!HAS_BIT(ses->list[index]->flags, LIST_FLAG_HIDE))
			{
				tintin_printf2(ses, "%-15s %5d   IGNORE %3s   MESSAGE %3s   INFO %3s   DEBUG %3s%s",
					list_table[index].name_multi,
					ses->list[index]->used,
					HAS_BIT(ses->list[index]->flags, LIST_FLAG_IGNORE)  ?  "ON" : "OFF",
					HAS_BIT(ses->list[index]->flags, LIST_FLAG_MESSAGE) ?  "ON" : "OFF",
					HAS_BIT(ses->list[index]->flags, LIST_FLAG_INFO)    ?  "ON" : "OFF",
					HAS_BIT(ses->list[index]->flags, LIST_FLAG_DEBUG)   ?  "ON" : "OFF",
					HAS_BIT(ses->list[index]->flags, LIST_FLAG_LOG)     ? " LOG" : "");
			}
		}
		tintin_header(ses, 80, "");
	}
	else
	{
		for (index = found = 0 ; index < LIST_MAX ; index++)
		{
			if (HAS_BIT(list_table[index].flags, LIST_FLAG_HIDE))
			{
				continue;
			}

			if (!is_abbrev(arg1, list_table[index].name_multi) && strcasecmp(arg1, "ALL"))
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
						tintin_printf2(ses, "#INFO %s %4d {arg1}{%s} {arg2}{%s} {arg3}{%s} {arg4}{%s} {class}{%s} {shots}{%u}", list_table[index].name_multi, cnt+1, root->list[cnt]->arg1, root->list[cnt]->arg2, root->list[cnt]->arg3, root->list[cnt]->arg4, root->list[cnt]->group, root->list[cnt]->shots);
					}
				}
				else if (is_abbrev(arg2, "SAVE"))
				{
					sprintf(name, "info[%s]", list_table[index].name_multi);

					set_nest_node_ses(ses, name, "");

					for (cnt = 0 ; cnt < root->used ; cnt++)
					{
						sprintf(name, "info[%s][%d]", list_table[index].name_multi, cnt + 1);

						set_nest_node_ses(ses, name, "{arg1}{%s}{arg2}{%s}{arg3}{%s}{arg4}{%s}{class}{%s}{nest}{%d}{shots}{%u}", root->list[cnt]->arg1, root->list[cnt]->arg2, root->list[cnt]->arg3, root->list[cnt]->arg4, root->list[cnt]->group, root->list[cnt]->root ? root->list[cnt]->root->used : 0, root->list[cnt]->shots);
					}
					show_message(ses, LIST_COMMAND, "#INFO: DATA WRITTEN TO {info[%s]}", list_table[index].name_multi);
				}
				else
				{
					show_error(ses, LIST_COMMAND, "#SYNTAX: #INFO {%s} [ON|OFF|LIST|SAVE|SYSTEM]", arg1);
				}
				return ses;
			}
			show_message(ses, LIST_COMMAND, "#OK: #INFO STATUS FOR %s HAS BEEN SET TO: %s.", list_table[index].name_multi, HAS_BIT(ses->list[index]->flags, LIST_FLAG_INFO) ? "ON" : "OFF");

			if (strcasecmp(arg1, "ALL"))
			{
				return ses;
			}
			found = TRUE;
		}

		if (found)
		{
			return ses;
		}

		*gtd->is_result = 0;

		switch (*arg1 % 32)
		{
			case CTRL_C:
				if (is_abbrev(arg1, "CPU"))
				{
					show_cpu(ses);
				}
				break;

			case CTRL_E:
				if (is_abbrev(arg1, "ENVIRON"))
				{
					char **env, *sep;

					if (is_abbrev(arg2, "SAVE"))
					{
						set_nest_node_ses(ses, "info[ENVIRON]", "");

						for (env = environ ; *env ; env++)
						{
							sep = strchr(*env, '=');

							*sep = 0;

							add_nest_node_ses(ses, "info[ENVIRON]", "{%s}{%s}", *env, sep + 1);

							*sep = '=';
						}
					}
					else
					{
						for (env = environ ; *env ; env++)
						{
							tintin_printf2(ses, "%s", *env);
						}
					}
				}
				break;

			case CTRL_I:
				if (is_abbrev(arg1, "INPUT"))
				{
					if (is_abbrev(arg2, "SAVE"))
					{
						set_nest_node_ses(ses, "info[INPUT]", "{BUFFER}{%s}", gtd->ses->input->buf);
						add_nest_node_ses(ses, "info[INPUT]", "{CUT}{%s}", gtd->ses->input->cut);
						add_nest_node_ses(ses, "info[INPUT]", "{COL}{%d}", inputline_cur_col());
						add_nest_node_ses(ses, "info[INPUT]", "{HEIGHT}{%d}", inputline_max_row());
						add_nest_node_ses(ses, "info[INPUT]", "{LENGTH}{%d}", inputline_cur_str_len());
						add_nest_node_ses(ses, "info[INPUT]", "{NAME}{%s}", gtd->ses->input->line_name);
						add_nest_node_ses(ses, "info[INPUT]", "{OFFSET}{%d}", inputline_cur_off());
						add_nest_node_ses(ses, "info[INPUT]", "{ROW}{%d}", inputline_cur_row());
						add_nest_node_ses(ses, "info[INPUT]", "{WIDTH}{%d}", inputline_max_str_len());
					}
					else
					{
						tintin_printf2(ses, "#INFO INPUT: BUFFER: %s", gtd->ses->input->buf);
						tintin_printf2(ses, "#INFO INPUT: CUT: %s", gtd->ses->input->cut);
						tintin_printf2(ses, "#INFO INPUT: COL: %d", inputline_cur_col());
						tintin_printf2(ses, "#INFO INPUT: HEIGHT: %d", inputline_max_row());
						tintin_printf2(ses, "#INFO INPUT: LENGTH: %d", inputline_cur_str_len());
						tintin_printf2(ses, "#INFO INPUT: NAME: %s", gtd->ses->input->line_name);
						tintin_printf2(ses, "#INFO INPUT: OFFSET: %d", inputline_cur_off());
						tintin_printf2(ses, "#INFO INPUT: ROW: %d", inputline_cur_row());
						tintin_printf2(ses, "#INFO INPUT: WIDTH: %d", inputline_max_str_len());
					}
				}
				break;

			case CTRL_M:
				if (is_abbrev(arg1, "MCCP"))
				{
					if (ses->mccp2)
					{
						tintin_printf2(ses, "#INFO MCCP2: TOTAL IN: %9u TOTAL OUT: %9u PERCENT: %3d", ses->mccp2->total_in, ses->mccp2->total_out, ses->mccp2->total_out ? 100 * ses->mccp2->total_in / ses->mccp2->total_out : 0);
					}
					if (ses->mccp3)
					{
						tintin_printf2(ses, "#INFO MCCP3: TOTAL IN: %9u TOTAL OUT: %9u PERCENT: %3d", ses->mccp3->total_in, ses->mccp3->total_out, ses->mccp3->total_in ? 100 * ses->mccp3->total_out / ses->mccp3->total_in : 0);
					}
				}
				else if (is_abbrev(arg1, "MEMORY"))
				{
					struct str_data *str_ptr;

					long long quan, used, max;

					max  = 0;
					quan = 0;
					used = 0;

					for (index = 0 ; index < gtd->memory->stack_cap ; index++)
					{
						max++;
						quan += gtd->memory->stack[index]->max;
						used += gtd->memory->stack[index]->len;
					}

					tintin_printf2(ses, "#INFO MEMORY: STACK SIZE: %d", quan);

					tintin_printf2(ses, "#INFO MEMORY: STACK  MAX: %d", gtd->memory->stack_max);
					tintin_printf2(ses, "#INFO MEMORY: STACK  CAP: %d", gtd->memory->stack_cap);
					tintin_printf2(ses, "#INFO MEMORY: STACK  LEN: %d", gtd->memory->stack_len);

					tintin_printf2(ses, "");

					max  = 0;
					quan = 0;
					used = 0;

					for (index = 0 ; index < gtd->memory->list_len ; index++)
					{
						str_ptr = gtd->memory->list[index];

						if (str_ptr->max != NAME_SIZE + 1 && strlen(get_str_str(str_ptr)) != str_ptr->len)
						{
							tintin_printf2(ses, "#ERROR: index %d len = %d/%d max = %d flags = %d (%s)", index, strlen(get_str_str(str_ptr)), str_ptr->len, str_ptr->max, str_ptr->flags, get_str_str(str_ptr));
						}

						if (!HAS_BIT(str_ptr->flags, STR_FLAG_FREE))
						{
							max++;
							quan += str_ptr->max;
							used += str_ptr->len;
						}
					}

					tintin_printf2(ses, "#INFO MEMORY: ALLOC SIZE: %d", quan);
					tintin_printf2(ses, "#INFO MEMORY: ALLOC USED: %d", used);

					tintin_printf2(ses, "#INFO MEMORY: ALLOC  MAX: %d", gtd->memory->list_max);
					tintin_printf2(ses, "#INFO MEMORY: ALLOC  LEN: %d", gtd->memory->list_len);

					tintin_printf2(ses, "");

					quan = 0;
					used = 0;

					for (index = 0 ; index < gtd->memory->free_len ; index++)
					{
						str_ptr = gtd->memory->list[gtd->memory->free[index]];

						if (HAS_BIT(str_ptr->flags, STR_FLAG_FREE))
						{
							quan += str_ptr->max;
							used += str_ptr->len;
						}
						else
						{
							tintin_printf2(ses, "error: found freed memory not marked as free.");
						}
					}

					tintin_printf2(ses, "#INFO MEMORY: FREED SIZE: %d", quan);

					tintin_printf2(ses, "#INFO MEMORY: FREED  MAX: %d", gtd->memory->free_max);
					tintin_printf2(ses, "#INFO MEMORY: FREED  LEN: %d", gtd->memory->free_len);

					tintin_printf2(ses, "");

					quan = 0;
					used = 0;

					for (index = 0 ; index < gtd->memory->debug_len ; index++)
					{
						quan += NAME_SIZE;
					}

					tintin_printf2(ses, "#INFO MEMORY: DEBUG SIZE: %d", quan);

					tintin_printf2(ses, "#INFO MEMORY: DEBUG  MAX: %d", gtd->memory->debug_max);
					tintin_printf2(ses, "#INFO MEMORY: DEBUG  LEN: %d", gtd->memory->debug_len);
				}
				break;

			case CTRL_S:
				if (is_abbrev(arg1, "SESSION"))
				{
					if (is_abbrev(arg2, "SAVE"))
					{
						set_nest_node_ses(ses, "info[SESSION]", "{NAME}{%s}", ses->name);
						add_nest_node_ses(ses, "info[SESSION]", "{ACTIVE}{%d}", gtd->ses == ses);
						add_nest_node_ses(ses, "info[SESSION]", "{CLASS}{%s}", ses->group);
						add_nest_node_ses(ses, "info[SESSION]", "{CREATED}{%d}", ses->created);
						add_nest_node_ses(ses, "info[SESSION]", "{HOST} {%s}", ses->session_host);
						add_nest_node_ses(ses, "info[SESSION]", "{IP} {%s}", ses->session_ip);
						add_nest_node_ses(ses, "info[SESSION]", "{PORT} {%s}", ses->session_port);

						show_message(ses, LIST_COMMAND, "#INFO: DATA WRITTEN TO {info[SESSION]}");
					}
					else
					{
						tintin_printf2(ses, "{NAME}{%s}", ses->name);
						tintin_printf2(ses, "{ACTIVE}{%d}", gtd->ses == ses);
						tintin_printf2(ses, "{CLASS}{%s}", ses->group);
						tintin_printf2(ses, "{CREATED}{%d}", ses->created);
						tintin_printf2(ses, "{HOST} {%s}", ses->session_host);
						tintin_printf2(ses, "{IP} {%s}", ses->session_ip);
						tintin_printf2(ses, "{PORT} {%s}", ses->session_port);
					}
				}
				else if (is_abbrev(arg1, "SESSIONS"))
				{
					struct session *sesptr;

					if (is_abbrev(arg2, "SAVE"))
					{
						set_nest_node_ses(ses, "info[SESSIONS]", "");

						for (sesptr = gts ; sesptr ; sesptr = sesptr->next)
						{
							sprintf(name, "info[SESSIONS][%s]", sesptr->name);

							add_nest_node_ses(ses, name, "{NAME}{%s}", sesptr->name);
							add_nest_node_ses(ses, name, "{ACTIVE}{%d}", gtd->ses == sesptr);
							add_nest_node_ses(ses, name, "{CLASS}{%s}", sesptr->group);
							add_nest_node_ses(ses, name, "{CREATED}{%d}", sesptr->created);
							add_nest_node_ses(ses, name, "{HOST} {%s}", sesptr->session_host);
							add_nest_node_ses(ses, name, "{IP} {%s}", sesptr->session_ip);
							add_nest_node_ses(ses, name, "{PORT} {%s}", sesptr->session_port);
						}
						show_message(ses, LIST_COMMAND, "#INFO: DATA WRITTEN TO {info[SESSION]}");
					}
					else
					{
						for (sesptr = gts ; sesptr ; sesptr = sesptr->next)
						{
							tintin_printf2(ses, "{%s}{NAME}{%s}", sesptr->name, sesptr->name);
							tintin_printf2(ses, "{%s}{ACTIVE}{%d}", sesptr->name, gtd->ses == sesptr);
							tintin_printf2(ses, "{%s}{CLASS}{%s}", sesptr->name, sesptr->group);
							tintin_printf2(ses, "{%s}{CREATED}{%d}", sesptr->name, sesptr->created);
							tintin_printf2(ses, "{%s}{HOST} {%s}", sesptr->name, sesptr->session_host);
							tintin_printf2(ses, "{%s}{IP} {%s}", sesptr->name, sesptr->session_ip);
							tintin_printf2(ses, "{%s}{PORT} {%s}", sesptr->name, sesptr->session_port);
						}
					}
				}
				else if (is_abbrev(arg1, "STACK"))
				{
					dump_stack();
				}
				else if (is_abbrev(arg1, "SYSTEM"))
				{
					char cwd[PATH_MAX];

					if (getcwd(cwd, PATH_MAX) == NULL)
					{
						syserr_printf(ses, "do_info: getcwd:");

						cwd[0] = 0;
					}

					if (is_abbrev(arg2, "SAVE"))
					{
						sprintf(name, "info[SYSTEM]");

						set_nest_node_ses(ses, name, "{CLIENT_NAME}{%s}{CLIENT_VERSION}{%s}", CLIENT_NAME, CLIENT_VERSION);
						add_nest_node_ses(ses, name, "{CWD}{%s}{EXEC}{%s}{HOME}{%s}{LANG}{%s}{OS}{%s}{PID}{%d}{TERM}{%s}{TINTIN}{%s}", cwd, gtd->system->exec, gtd->system->home, gtd->system->lang, gtd->system->os, getpid(), gtd->system->term, gtd->system->tt_dir);
						add_nest_node_ses(ses, name, "{DETACH_FILE}{%s}{ATTACH_FILE}{%s}", gtd->detach_port > 0 ? gtd->detach_file : "", gtd->attach_sock > 0 ? gtd->attach_file : "");

						show_message(ses, LIST_COMMAND, "#INFO: DATA WRITTEN TO {info[SYSTEM]}");
					}
					else
					{
						tintin_printf2(ses, "#INFO SYSTEM: CLIENT_NAME    = %s", CLIENT_NAME);
						tintin_printf2(ses, "#INFO SYSTEM: CLIENT_VERSION = %s", CLIENT_VERSION);
						tintin_printf2(ses, "#INFO SYSTEM: CWD            = %s", cwd);
						tintin_printf2(ses, "#INFO SYSTEM: EXEC           = %s", gtd->system->exec);
						tintin_printf2(ses, "#INFO SYSTEM: HOME           = %s", gtd->system->home);
						tintin_printf2(ses, "#INFO SYSTEM: LANG           = %s", gtd->system->lang);
						tintin_printf2(ses, "#INFO SYSTEM: OS             = %s", gtd->system->os);
						tintin_printf2(ses, "#INFO SYSTEM: PID            = %d", getpid());
						tintin_printf2(ses, "#INFO SYSTEM: TERM           = %s", gtd->system->term);
						tintin_printf2(ses, "#INFO SYSTEM: TINTIN         = %s", gtd->system->tt_dir);
						tintin_printf2(ses, "#INFO SYSTEM: DETACH_PORT    = %d", gtd->detach_port);
						tintin_printf2(ses, "#INFO SYSTEM: DETACH_FILE    = %s", gtd->detach_port ? gtd->detach_file : "");
						tintin_printf2(ses, "#INFO SYSTEM: ATTACH_SOCK    = %d", gtd->attach_sock);
						tintin_printf2(ses, "#INFO SYSTEM: ATTACH_FILE    = %s", gtd->attach_sock ? gtd->attach_file : "");
					}
				}
				break;

			case CTRL_U:
				if (is_abbrev(arg1, "UNICODE"))
				{
					int size, width, index;

					size = get_utf8_size(arg2);
					get_utf8_width(arg2, &width);
					get_utf8_index(arg2, &index);

					tintin_printf2(ses, "#INFO UNICODE: %s:  is_utf8_head  = %d (%s)", arg2, is_utf8_head(arg2), is_utf8_head(arg2) ? "true" : "false");
					tintin_printf2(ses, "#INFO UNICODE: %s: get_utf8_size  = %d", arg2, size);
					tintin_printf2(ses, "#INFO UNICODE: %s: get_utf8_width = %d", arg2, width);
					tintin_printf2(ses, "#INFO UNICODE: %s: get_utf8_index = %d (decimal)", arg2, index);
					tintin_printf2(ses, "#INFO UNICODE: %s: get_utf8_index = %x (hexadecimal)", arg2, index);
				}
				break;
		}
		if (*gtd->is_result == 0)
		{
			show_error(ses, LIST_COMMAND, "#INFO {%s} - NO MATCH FOUND.", arg1);
		}
	}
	return ses;

}
