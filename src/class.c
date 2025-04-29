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

#define DO_CLASS(class) struct session *class(struct session *ses, struct listnode *node, char *arg1, char *arg2)

extern DO_CLASS(class_assign);
extern DO_CLASS(class_clear);
extern DO_CLASS(class_close);
extern DO_CLASS(class_kill);
extern DO_CLASS(class_list);
extern DO_CLASS(class_load);
extern DO_CLASS(class_open);
extern DO_CLASS(class_read);
extern DO_CLASS(class_save);
extern DO_CLASS(class_size);
extern DO_CLASS(class_write);

typedef struct session *CLASS(struct session *ses, struct listnode *node, char *arg1, char *arg2);

struct class_type
{
	char                  * name;
	CLASS                 * fun;
	char                  * desc;
};

struct class_type class_table[] =
{
	{    "ASSIGN",            class_assign,   "Execute command with given class opened."        },
	{    "CLEAR",             class_clear,    "Delete triggers associated with given class."    },
	{    "CLOSE",             class_close,    "Close given class."                              },
	{    "KILL",              class_kill,     "Clear, close, and remove given class."           },
	{    "LIST",              class_list,     "List triggers associated with given class."      },
	{    "LOAD",              class_load,     "Load saved data of given class."                 },
	{    "OPEN",              class_open,     "Open given class."                               },
	{    "READ",              class_read,     "Read file with given class opened."              },
	{    "SAVE",              class_save,     "Save given class to memory."                     },
	{    "SIZE",              class_size,     "Store size of given class to a variable."        },
	{    "WRITE",             class_write,    "Write given class to file."                      },
	{    "",                  NULL,           NULL                                              }
};

int count_class(struct session *ses, struct listnode *group);

DO_COMMAND(do_class)
{
	int i, cnt;
	struct listroot *root;
	struct listnode *node;

	root = ses->list[LIST_CLASS];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " CLASSES ");

		for (root->update = 0 ; root->update < root->used ; root->update++)
		{
			node = root->list[root->update];

			tintin_printf2(ses, "%-20s  %4d  %6s  %6s  %3d", node->arg1, count_class(ses, node), !strcmp(ses->group, node->arg1) ? "ACTIVE" : "", atoi(node->arg3) ? "OPEN" : "CLOSED", atoi(node->arg3));
		}
	}
	else if (*arg2 == 0)
	{
		node = search_node_list(ses->list[LIST_CLASS], arg1);

		class_list(ses, node, arg1, arg2);
	}
	else
	{
		for (i = 0 ; *class_table[i].name ; i++)
		{
			if (is_abbrev(arg2, class_table[i].name))
			{
				break;
			}
		}

		if (*class_table[i].name == 0)
		{
			tintin_header(ses, 80, " CLASS OPTIONS ");

			for (cnt = 0 ; *class_table[cnt].fun != NULL ; cnt++)
			{
				if (*class_table[cnt].desc)
				{
					tintin_printf2(ses, "  [%-13s] %s", class_table[cnt].name, class_table[cnt].desc);
				}
			}
			tintin_header(ses, 80, "");
		}
		else
		{
			node = search_node_list(ses->list[LIST_CLASS], arg1);

			class_table[i].fun(ses, node, arg1, arg3);
		}
	}
	return ses;
}

struct listnode *create_class(struct session *ses, char *arg1, char *arg3)
{
	show_info(ses, LIST_CLASS, "#INFO: CLASS {%s} CREATED", arg1);

	check_all_events(ses, EVENT_FLAG_CLASS, 0, 1, "CLASS CREATED", arg1);
	check_all_events(ses, EVENT_FLAG_CLASS, 1, 1, "CLASS CREATED %s", arg1, arg1);

	return update_node_list(ses->list[LIST_CLASS], arg1, "", arg3, "");
}

int count_class(struct session *ses, struct listnode *group)
{
	int list, cnt, index;

	for (cnt = list = 0 ; list < LIST_MAX ; list++)
	{
		if (ses->list[list] == NULL || !HAS_BIT(ses->list[list]->flags, LIST_FLAG_CLASS))
		{
			continue;
		}

		for (index = 0 ; index < ses->list[list]->used ; index++)
		{
			if (!strcmp(ses->list[list]->list[index]->group, group->arg1))
			{
				cnt++;
			}
		}
	}
	return cnt;
}

void clear_class(struct session *ses, struct listnode *group)
{
	size_t type, index;

	check_all_events(ses, EVENT_FLAG_CLASS, 0, 1, "CLASS CLEAR", group->arg1);
	check_all_events(ses, EVENT_FLAG_CLASS, 1, 1, "CLASS CLEAR %s", group->arg1, group->arg1);

	for (type = 0 ; type < LIST_MAX ; type++)
	{
		if (ses->list[type] == NULL || !HAS_BIT(ses->list[type]->flags, LIST_FLAG_CLASS))
		{
			continue;
		}

		for (index = 0 ; index < ses->list[type]->used ; index++)
		{
			if (!strcmp(ses->list[type]->list[index]->group, group->arg1))
			{
				delete_index_list(ses->list[type], index--);
			}
		}
	}
}

DO_CLASS(class_assign)
{
	if (node == NULL)
	{
		node = create_class(ses, arg1, arg2);
	}
	char *tmp = ses->group;

	ses->group = strdup(arg1);

	script_driver(ses, LIST_COMMAND, arg2);

	free(ses->group);

	ses->group = tmp;

	return ses;
}

DO_CLASS(class_clear)
{
	if (node == NULL)
	{
		show_message(ses, LIST_CLASS, "#CLASS {%s} DOES NOT EXIST.", arg1);

		return ses;
	}

	clear_class(ses, node);

	show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN CLEARED.", arg1);

	if (!strcmp(ses->group, arg1))
	{
		class_close(ses, node, arg1, arg2);
	}

	return ses;
}

DO_CLASS(class_close)
{
	if (node == NULL)
	{
		node = search_node_list(ses->list[LIST_CLASS], arg1);

		if (node == NULL)
		{
			show_message(ses, LIST_CLASS, "#CLASS {%s} DOES NOT EXIST.", arg1);
			return ses;
		}
	}

	if (atoi(node->arg3) == 0)
	{
		show_message(ses, LIST_CLASS, "#CLASS {%s} IS ALREADY CLOSED.", arg1);
	}
	else
	{
		show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN CLOSED.", arg1);

		update_node_list(ses->list[LIST_CLASS], arg1, "", "0", node->arg4);

		if (!strcmp(ses->group, arg1))
		{
			check_all_events(ses, EVENT_FLAG_CLASS, 0, 1, "CLASS DEACTIVATED", ses->group);
			check_all_events(ses, EVENT_FLAG_CLASS, 1, 1, "CLASS DEACTIVATED %s", ses->group, ses->group);

			node = ses->list[LIST_CLASS]->list[0];

			if (atoi(node->arg3))
			{
				RESTRING(ses->group, node->arg1);

				show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN ACTIVATED.", node->arg1);

				check_all_events(ses, EVENT_FLAG_CLASS, 0, 1, "CLASS ACTIVATED", node->arg1);
				check_all_events(ses, EVENT_FLAG_CLASS, 1, 1, "CLASS ACTIVATED %s", arg1, arg1);
			}
			else
			{
				RESTRING(ses->group, "");
			}
		}
	}
	return ses;
}


DO_CLASS(class_list)
{
	int i, j;

	if (node)
	{
		tintin_header(ses, 80, " %s ", arg1);

		for (i = 0 ; i < LIST_MAX ; i++)
		{
			if (!HAS_BIT(ses->list[i]->flags, LIST_FLAG_CLASS))
			{
				continue;
			}

			if (*arg2 && !is_abbrev(arg2, list_table[i].name) && !is_abbrev(arg2, list_table[i].name_multi))
			{
				continue;
			}

			for (j = 0 ; j < ses->list[i]->used ; j++)
			{
				if (!strcmp(ses->list[i]->list[j]->group, arg1))
				{
					show_node(ses->list[i], ses->list[i]->list[j], 0);
				}
			}
		}
	}
	else
	{
		show_error(ses, LIST_CLASS, "#CLASS {%s} DOES NOT EXIST.", arg1);
	}
	return ses;
}


DO_CLASS(class_kill)
{
	if (node == NULL)
	{
		show_message(ses, LIST_CLASS, "#CLASS {%s} DOES NOT EXIST.", arg1);

		return ses;
	}

//	Best to handle this in delete_node so it works with #kill

//	class_clear(ses, node, arg1, arg2);

	delete_node_list(ses, LIST_CLASS, node);

//	group = search_index_list(ses->list[LIST_CLASS], arg1, NULL);

//	delete_index_list(ses->list[LIST_CLASS], group);

//	check_all_events(ses, EVENT_FLAG_CLASS, 0, 1, "CLASS DESTROYED", arg1);
//	check_all_events(ses, EVENT_FLAG_CLASS, 1, 1, "CLASS DESTROYED %s", arg1, arg1);

	show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN KILLED.", arg1);

	return ses;
}

DO_CLASS(class_load)
{
	FILE *file;

	if (node == NULL)
	{
		show_error(ses, LIST_CLASS, "#CLASS {%s} DOES NOT EXIST.", arg1);

		return ses;
	}

	if (node->data == NULL)
	{
		show_error(ses, LIST_CLASS, "#CLASS {%s} DOES NOT HAVE ANY DATA SAVED.", arg1);

		return ses;
	}

	file = fmemopen(node->data, (size_t) atoi(node->arg4), "r");

	read_file(ses, file, arg1);

	fclose(file);

	check_all_events(ses, EVENT_FLAG_CLASS, 0, 1, "CLASS LOAD", arg1);
	check_all_events(ses, EVENT_FLAG_CLASS, 1, 1, "CLASS LOAD %s", arg1, arg1);

	show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN LOADED FROM MEMORY.", arg1);

	return ses;
}

DO_CLASS(class_open)
{
	int count;

	if (node == NULL)
	{
		node = create_class(ses, arg1, arg2);
	}

	if (!strcmp(ses->group, arg1))
	{
		show_message(ses, LIST_CLASS, "#CLASS {%s} IS ALREADY OPENED AND ACTIVATED.", arg1);
	}
	else
	{
		if (*ses->group)
		{
			check_all_events(ses, EVENT_FLAG_CLASS, 0, 1, "CLASS DEACTIVATED", ses->group);
			check_all_events(ses, EVENT_FLAG_CLASS, 1, 1, "CLASS DEACTIVATED %s", ses->group, ses->group);
		}
		RESTRING(ses->group, arg1);

		count = atoi(ses->list[LIST_CLASS]->list[0]->arg3);

		update_node_list(ses->list[LIST_CLASS], arg1, "", ntos(--count), node->arg4);

		show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN OPENED AND ACTIVATED.", arg1);

		check_all_events(ses, EVENT_FLAG_CLASS, 0, 1, "CLASS ACTIVATED", arg1);
		check_all_events(ses, EVENT_FLAG_CLASS, 1, 1, "CLASS ACTIVATED %s", arg1, arg1);
	}

	return ses;
}


DO_CLASS(class_read)
{
	class_open(ses, node, arg1, arg2);

	command(ses, do_read, "{%s}", arg2);

	class_close(ses, node, arg1, arg2);

	return ses;
}

DO_CLASS(class_save)
{
	FILE *file;
	size_t len;
	int list, index;

	if (node == NULL)
	{
		show_error(ses, LIST_CLASS, "#ERROR: #CLASS {%s} DOES NOT EXIST.", arg1);

		return ses;
	}

	file = open_memstream(&node->data, (size_t *) &len);

	fprintf(file, "%cCLASS {%s} OPEN\n\n", gtd->tintin_char, arg1);

	for (list = 0 ; list < LIST_MAX ; list++)
	{
		if (!HAS_BIT(ses->list[list]->flags, LIST_FLAG_CLASS))
		{
			continue;
		}

		for (index = 0 ; index < ses->list[list]->used ; index++)
		{
			if (!strcmp(ses->list[list]->list[index]->group, arg1))
			{
				write_node(ses, list, ses->list[list]->list[index], file);
			}
		}
	}

	fprintf(file, "\n%cCLASS {%s} CLOSE\n", gtd->tintin_char, arg1);

	fclose(file);

	str_cpy_printf(&node->arg4, "%d", len);

	show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN SAVED TO MEMORY.", arg1);

	return ses;
}	

DO_CLASS(class_size)
{
	if (*arg1 == 0 || *arg2 == 0)
	{
		show_error(ses, LIST_CLASS, "#SYNTAX: #CLASS <NAME> SIZE <VARIABLE>");
		
		return ses;
	}

	set_nest_node_ses(ses, arg2, "%d", node ? count_class(ses, node) : 0);

	return ses;
}


DO_CLASS(class_write)
{
	FILE *file;
	int list, index;

	if (node == NULL)
	{
		show_error(ses, LIST_CLASS, "#ERROR: #CLASS {%s} DOES NOT EXIST.", arg1);

		return ses;
	}

	if (*arg2 == 0 || (file = fopen(arg2, "w")) == NULL)
	{
		show_error(ses, LIST_CLASS, "#ERROR: #CLASS WRITE {%s}: COULDN'T OPEN FILE.", arg2);
		
		return ses;
	}

	fprintf(file, "%cCLASS {%s} OPEN\n\n", gtd->tintin_char, arg1);

	for (list = 0 ; list < LIST_MAX ; list++)
	{
		if (!HAS_BIT(ses->list[list]->flags, LIST_FLAG_CLASS))
		{
			continue;
		}

		for (index = 0 ; index < ses->list[list]->used ; index++)
		{
			if (!strcmp(ses->list[list]->list[index]->group, arg1))
			{
				write_node(ses, list, ses->list[list]->list[index], file);
			}
		}
	}

	fprintf(file, "\n%cCLASS {%s} CLOSE\n", gtd->tintin_char, arg1);

	fclose(file);

	show_message(ses, LIST_CLASS, "#CLASS {%s} HAS BEEN WRITTEN TO FILE.", arg1);

	return ses;
}
