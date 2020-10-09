/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright (C) 2004-2020 Igor van den Hoven                                *
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
*                      coded by Igor van den Hoven 2020                       *
******************************************************************************/

#include "tintin.h"

DO_COMMAND(do_edit)
{
	int cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " EDIT OPTIONS ");

		for (cnt = 0 ; edit_table[cnt].fun ; cnt++)
		{
			if (*edit_table[cnt].desc)
			{
				tintin_printf2(ses, "  [%-18s] %s", edit_table[cnt].name, edit_table[cnt].desc);
			}
		}
		tintin_header(ses, 80, "");
	}
	else
	{
		for (cnt = 0 ; edit_table[cnt].fun ; cnt++)
		{
			if (is_abbrev(arg1, edit_table[cnt].name))
			{
				edit_table[cnt].fun(ses, arg, arg1, arg2);

				return ses;
			}
		}
		show_error(ses, LIST_COMMAND, "#ERROR: #EDIT {%s} IS NOT A VALID OPTION.", arg1);
	}
	return ses;
}

DO_EDIT(edit_create)
{
	struct edit_data *edit = gtd->ses->input->edit;
	char *pta, *ptn;
	int index;

	clear_editor(edit);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	str_cpy(&gtd->ses->input->edit_name, arg1);

	index = 0;

	while (*arg)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

		pta = arg1;
		ptn = strchr(pta, '\n');

		while (ptn)
		{
			*ptn++ = 0;

			if (*ptn)
			{
				insert_line(edit, index++, pta);

				pta = ptn;
			}
			ptn = strchr(pta, '\n');
		}
		insert_line(edit, index++, pta);
	}

	show_message(ses, LIST_COMMAND, "#EDIT CREATE: CREATED %d LINES.", edit->used);

	enable_editor(edit);

	return ses;
}

DO_EDIT(edit_load)
{
	struct edit_data *edit = gtd->ses->input->edit;
	struct listnode *node;
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if ((node = search_nest_node_ses(ses, arg1)) == NULL)
	{
		show_error(ses, LIST_COMMAND, "#EDIT LOAD: VARIABLE {%s} NOT FOUND.", arg1);
		
		return ses;
	}

	if (node->root == NULL)
	{
		return edit_create(ses, node->arg1, arg1, arg2);
	}

	clear_editor(edit);

	for (index = 0 ; index < node->root->used ; index++)
	{
		substitute(ses, node->root->list[index]->arg1, arg2, SUB_ESC);

		insert_line(edit, index, arg2);
	}

	show_message(ses, LIST_COMMAND, "#EDIT LOAD: LOADED %d LINES FROM {%s}.", edit->used, arg1);

	enable_editor(edit);

	return ses;
}

DO_EDIT(edit_save)
{
	struct edit_data *edit = gtd->ses->input->edit;
	struct listnode *node;
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (!valid_variable(ses, arg1))
	{
		show_error(ses, LIST_VARIABLE, "#EDIT SAVE: INVALID VARIABLE NAME {%s}.", arg1);

		return ses;
	}

	node = set_nest_node_ses(ses, arg1, "");

	node->root = init_list(gtd->ses, LIST_VARIABLE, LIST_SIZE);

	for (index = 0 ; index < edit->used ; index++)
	{
		substitute(ses, edit->line[index]->str, arg2, SUB_SEC);

		create_node_list(node->root, ntos(index), arg2, "", "");
	}

	show_message(ses, LIST_COMMAND, "#EDIT SAVE: SAVED %d LINES TO {%s}.", edit->used, arg1);

	return ses;
}

DO_EDIT(edit_read)
{
	struct edit_data *edit = gtd->ses->input->edit;
	FILE *file;
	int index, size;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #EDIT READ {<FILENAME>}");

		return ses;
	}

	if ((file = fopen(arg1, "r")) == NULL)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #EDIT READ: FILE {%s} NOT FOUND.", arg1);

		return ses;
	}

	clear_editor(edit);

	str_cpy(&gtd->ses->input->edit_name, arg1);

	index = 0;
	size  = 0;

	while (fread_one_line(&arg2, file))
	{
		size += str_len(arg2);

		insert_line(edit, index++, arg2);
	}

	show_message(ses, LIST_COMMAND, "#EDIT READ: READ %d LINES FROM {%s}.", edit->used, arg1);

	enable_editor(edit);

	fclose(file);

	return ses;
}

DO_EDIT(edit_write)
{
	struct edit_data *edit = gtd->ses->input->edit;
	FILE *file;
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		str_cpy(&arg1, gtd->ses->input->edit_name);
	}

	if ((file = fopen(arg1, "w")) == NULL)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #EDIT WRITE: COULDN'T OPEN {%s} TO WRITE.", arg1);

		return ses;
	}

	for (index = 0 ; index < edit->used ; index++)
	{
		fputs(edit->line[index]->str, file);

		putc(ASCII_LF, file);
	}

	show_message(ses, LIST_COMMAND, "#EDIT WRITE: WROTE %d LINES TO {%s}.", index, arg1);

	return ses;
}

DO_EDIT(edit_resume)
{
	struct edit_data *edit = gtd->ses->input->edit;

	SET_BIT(gtd->ses->input->flags, INPUT_FLAG_EDIT);

	if (edit->size == edit->update)
	{
		insert_line(edit, edit->update, "");
	}

	inputline_set(edit->line[edit->update]->str, 0);

	cursor_redraw_edit(ses, "");

	return ses;
}

DO_EDIT(edit_suspend)
{
	struct edit_data *edit = gtd->ses->input->edit;

	DEL_BIT(gtd->ses->input->flags, INPUT_FLAG_EDIT);

	str_cpy(&edit->line[edit->update]->str, gtd->ses->input->buf);

	cursor_clear_line(ses, "");

	cursor_redraw_edit(ses, "");

	return ses;
}

struct edit_data *create_editor(void)
{
	return calloc(1, sizeof(struct edit_data));
}

void delete_editor(struct edit_data *edit)
{
	if (edit == NULL)
	{
		tintin_printf2(gtd->ses, "destroy_edit: NULL");

		return;
	}

	clear_editor(edit);

	free(edit);
}

void resize_editor(struct edit_data *edit, int size)
{
	if (edit->size < size)
	{
		edit->size = size;

		edit->line = (struct row_data **) realloc(edit->line, edit->size * sizeof(struct row_data *));
	}
}

void clear_editor(struct edit_data *edit)
{
	int index;

	edit->update = 0;

	for (index = edit->used - 1 ; index >= 0 ; index--)
	{
		delete_line(edit, index);
	}

}

void enable_editor(struct edit_data *edit)
{
	char *str1;
	int filesize;

	SET_BIT(gtd->ses->input->flags, INPUT_FLAG_EDIT);

	if (edit->used == 0)
	{
		create_line(edit, 0, "");
	}

	inputline_set(edit->line[edit->update]->str, 0);

	str1 = str_alloc_stack(0);

	filesize = str_save_editor(gtd->ses->input->edit, &str1);

	cursor_redraw_edit(gtd->ses, "");

	check_all_events(gtd->ses, EVENT_FLAG_INPUT, 0, 4, "STARTED EDITING", gtd->ses->input->edit_name, ntos(edit->used), ntos(filesize), str1);

	if (*gtd->ses->input->edit_name)
	{
		check_all_events(gtd->ses, EVENT_FLAG_INPUT, 1, 4, "STARTED EDITING %s", gtd->ses->input->edit_name, gtd->ses->input->edit_name, ntos(edit->used), ntos(filesize), str1);
	}
}

int str_save_editor(struct edit_data *edit, char **str)
{
	int index, size;

	size = 0;

	str_cpy(str, "");

	for (index = 0 ; index < edit->used ; index++)
	{
		str_cat_printf(str, "%s\n", edit->line[index]->str);

		size += str_len(edit->line[index]->str);
	}
	return size;
}

int var_save_editor(struct edit_data *edit, char **str)
{
	int index, size;

	size = 0;

	for (index = 0 ; index < edit->used ; index++)
	{
		str_cat_printf(str, "{%d}{%s}", index + 1, edit->line[index]->str);
	}
	return size;
}

void create_line(struct edit_data *edit, int index, char *str)
{
	resize_editor(edit, index + 2);

	edit->line[index]      = (struct row_data *) calloc(1, sizeof(struct row_data));
	edit->line[index]->str = str_dup(str);

	edit->used++;
}

void delete_line(struct edit_data *edit, int index)
{
	str_free(edit->line[index]->str);

	free(edit->line[index]);

	edit->used--;
}

void insert_line(struct edit_data *edit, int index, char *str)
{
	int cnt;

	if (index >= edit->used)
	{
		while (edit->used < index)
		{
			create_line(edit, edit->used, "");
		}
		create_line(edit, edit->used, str);

		return;
	}

	resize_editor(edit, edit->used + 2);

	for (cnt = edit->used ; cnt > index ; cnt--)
	{
		edit->line[cnt] = edit->line[cnt - 1];
	}

	create_line(edit, index, str);
}

void remove_line(struct edit_data *edit, int index)
{
	int cnt;

	delete_line(edit, index);

	for (cnt = index ; cnt < edit->used ; cnt++)
	{
		edit->line[cnt] = edit->line[cnt + 1];
	}
}
