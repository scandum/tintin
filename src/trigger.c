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

#include "tintin.h"

/******************************************************************************
*               (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                  *
*                                                                             *
*                        coded by Peter Unold 1992                            *
*                   recoded by Igor van den Hoven 2009                        *
******************************************************************************/

DO_COMMAND(do_action)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_ACTION], 0);
	}
	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_ACTION]) == FALSE)
		{
			show_message(ses, LIST_ACTION, "#ACTION: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		if (*arg3 && (atof(arg3) < 1 || atof(arg3) >= 10))
		{
			show_error(ses, LIST_ACTION, "\e[1;31m#WARNING: #ACTION {%s} {..} {%s} SHOULD HAVE A PRIORITY BETWEEN 1.000 and 9.999.", arg1, arg3);
		}

		update_node_list(ses->list[LIST_ACTION], arg1, arg2, arg3, "");

		show_message(ses, LIST_ACTION, "#OK. #ACTION {%s} NOW TRIGGERS {%s} @ {%s}.", arg1, arg2, arg3);
	}
	return ses;
}


DO_COMMAND(do_unaction)
{
	delete_node_with_wild(ses, LIST_ACTION, arg);

	return ses;
}


void check_all_actions(struct session *ses, char *original, char *line, char *buf)
{
	struct listroot *root = ses->list[LIST_ACTION];
	struct listnode *node;

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		node = root->list[root->update];

		if (check_one_regexp(ses, node, line, original, 0))
		{
			show_debug(ses, LIST_ACTION, "#DEBUG ACTION {%s}", node->arg1);

			substitute(ses, node->arg2, buf, SUB_ARG|SUB_SEC);

			if (node->shots && --node->shots == 0)
			{
				delete_node_list(ses, LIST_ACTION, node);
			}

			script_driver(ses, LIST_ACTION, buf);

			return;
		}
	}
}

/******************************************************************************
*               (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                  *
*                                                                             *
*                        coded by Peter Unold 1992                            *
*                   recoded by Igor van den Hoven 2009                        *
******************************************************************************/

DO_COMMAND(do_alias)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_ALIAS], 0);
	}
	else if (*arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_ALIAS]) == FALSE)
		{
			show_message(ses, LIST_ALIAS, "#ALIAS: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		update_node_list(ses->list[LIST_ALIAS], arg1, arg2, arg3, "");

		show_message(ses, LIST_ALIAS, "#ALIAS {%s} NOW TRIGGERS {%s} @ {%s}.", arg1, arg2, arg3);
	}
	return ses;
}


DO_COMMAND(do_unalias)
{
	delete_node_with_wild(ses, LIST_ALIAS, arg);

	return ses;
}

int check_all_aliases(struct session *ses, char *input)
{
	struct listnode *node;
	struct listroot *root;
	char *buf, *line, *arg;
	int i;

	root = ses->list[LIST_ALIAS];

	if (gtd->level->ignore || HAS_BIT(root->flags, LIST_FLAG_IGNORE))
	{
		return FALSE;
	}

	if (push_call_printf("check_all_aliases(%s,%s)",ses->name,input) == 0)
	{
		pop_call();
		return FALSE;
	}

	buf  = str_alloc_stack(0);
	line = str_alloc_stack(0);

	substitute(ses, input, line, SUB_VAR|SUB_FUN);

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		node = root->list[root->update];

		if (check_one_regexp(ses, node, line, line, PCRE_ANCHORED))
		{
			i = strlen(node->arg1);

			if (!strncmp(node->arg1, line, i))
			{
				if (line[i])
				{
					if (line[i] != ' ')
					{
						continue;
					}
					arg = &line[i + 1];
				}
				else
				{
					arg = &line[i];
				}

				RESTRING(gtd->vars[0], arg)

				for (i = 1 ; i < 100 ; i++)
				{
					arg = get_arg_in_braces(ses, arg, buf, GET_ONE);

					RESTRING(gtd->vars[i], buf);

					if (*arg == 0)
					{
						while (++i < 100)
						{
							if (*gtd->vars[i])
							{
								RESTRING(gtd->vars[i], "");
							}
						}
						break;
					}

				}
			}

			substitute(ses, node->arg2, buf, SUB_ARG);

			if (!strncmp(node->arg1, line, strlen(node->arg1)) && !strcmp(node->arg2, buf) && *gtd->vars[0])
			{
				sprintf(input, "%s %s", buf, gtd->vars[0]);
			}
			else
			{
				sprintf(input, "%s", buf);
			}

			show_debug(ses, LIST_ALIAS, "#DEBUG ALIAS {%s} {%s}", node->arg1, gtd->vars[0]);

			if (node->shots && --node->shots == 0)
			{
				delete_node_list(ses, LIST_ALIAS, node);
			}
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}


/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                      coded by Igor van den Hoven 2019                       *
******************************************************************************/


DO_COMMAND(do_button)
{
	struct listnode *node;
	int index;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_BUTTON], 0);
	}
	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_BUTTON]) == FALSE)
		{
			show_message(ses, LIST_BUTTON, "#BUTTON: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		SET_BIT(gtd->event_flags, EVENT_FLAG_MOUSE);
		SET_BIT(ses->event_flags, EVENT_FLAG_MOUSE);

		node = update_node_list(ses->list[LIST_BUTTON], arg1, arg2, arg3, "");

		show_message(ses, LIST_BUTTON, "#OK. BUTTON {%s} NOW TRIGGERS {%s} @ {%s}.", arg1, arg2, arg3);

		arg = arg1;

		for (index = 0 ; index < 4 ; index++)
		{
			arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

			node->val16[index] = (short) get_number(ses, arg2);

			if (*arg == COMMAND_SEPARATOR)
			{
				arg++;
			}
		}

		if (*arg)
		{
			arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

			str_cpy(&node->arg4, arg2);
		}
		else
		{
			str_cpy(&node->arg4, "PRESSED MOUSE BUTTON ONE");
		}
	}
	return ses;
}


DO_COMMAND(do_unbutton)
{
	delete_node_with_wild(ses, LIST_BUTTON, arg);

	return ses;
}

void check_all_buttons(struct session *ses, short row, short col, char *arg1, char *arg2, char *word, char *line)
{
	char *buf, *arg4;
	struct listnode *node;
	struct listroot *root;
	short val[4];

	root = ses->list[LIST_BUTTON];

	if (HAS_BIT(root->flags, LIST_FLAG_IGNORE))
	{
		return;
	}

	push_call("check_all_buttons(%p,%d,%d,%p,%p,%p,%p)",ses,row,col,arg1,arg2,word,line);

	buf  = str_alloc_stack(0);
	arg4 = str_alloc_stack(0);

	sprintf(arg4, "%s %s", arg1, arg2);

	show_info(ses, LIST_BUTTON, "#INFO BUTTON {%d;%d;%d;%d;%s}", row, col, -1 - (gtd->screen->rows - row), -1 - (gtd->screen->cols - col), arg4);

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		node = root->list[root->update];;

		val[0] = node->val16[0] < 0 ? 1 + gtd->screen->rows + node->val16[0] : node->val16[0];
		val[1] = node->val16[1] < 0 ? 1 + gtd->screen->cols + node->val16[1] : node->val16[1];

		if (row < val[0] || col < val[1])
		{
			continue;
		}

		val[2] = node->val16[2] < 0 ? 1 + gtd->screen->rows + node->val16[2] : node->val16[2];
		val[3] = node->val16[3] < 0 ? 1 + gtd->screen->cols + node->val16[3] : node->val16[3];

		if (row > val[2] || col > val[3])
		{
			continue;
		}

		if (!strcmp(arg4, node->arg4))
		{
			show_debug(ses, LIST_BUTTON, "#DEBUG BUTTON {%s}", node->arg1);

			RESTRING(gtd->vars[0], ntos(row));
			RESTRING(gtd->vars[1], ntos(col));
			RESTRING(gtd->vars[2], ntos(-1 - (gtd->screen->rows - row)));
			RESTRING(gtd->vars[3], ntos(-1 - (gtd->screen->cols - col)));
			RESTRING(gtd->vars[4], word);
			RESTRING(gtd->vars[5], line);

			substitute(ses, node->arg2, buf, SUB_ARG|SUB_SEC);

			if (node->shots && --node->shots == 0)
			{
				delete_node_list(ses, LIST_BUTTON, node);
			}
			script_driver(ses, LIST_BUTTON, buf);

			break;
		}
	}
	pop_call();
	return;
}


/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

DO_COMMAND(do_delay)
{
	char time[NUMBER_SIZE];
	long double number;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_DELAY], 0);
	}
	else if (*arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_DELAY]) == FALSE)
		{
			show_message(ses, LIST_DELAY, "#DELAY: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		if (*arg3 == 0)
		{
			number = get_number(ses, arg1);

			sprintf(arg3, "%lld", gtd->utime);

			sprintf(time, "%llu.%010llu", gtd->utime + (unsigned long long) (number * 1000000), (unsigned long long) (number * 10000000000) % 10000000000);

			create_node_list(ses->list[LIST_DELAY], time, arg2, arg1, arg3);

			show_message(ses, LIST_DELAY, "#OK, IN {%s} SECONDS {%s} IS EXECUTED.", arg1, arg2);

		}
		else
		{
			struct listnode *node;

			get_number_string(ses, arg3, time);

			sprintf(arg3, "%lld", gtd->utime);

			node = update_node_list(ses->list[LIST_TICKER], arg1, arg2, time, arg3);

			node->shots = 1;

			show_message(ses, LIST_TICKER, "#ONESHOT: #TICK {%s} WILL EXECUTE {%s} IN {%s} SECONDS.", arg1, arg2, time);
		}
	}
	return ses;
}

DO_COMMAND(do_undelay)
{
	sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (is_alpha(*arg1))
	{
		delete_node_with_wild(ses, LIST_TICKER, arg);
	}
	else
	{
		delete_node_with_wild(ses, LIST_DELAY, arg);
	}

	return ses;
}

// checked in update.c


/******************************************************************************
*               (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                  *
*                                                                             *
*                       coded by Sverre Normann 1999                          *
*                    recoded by Igor van den Hoven 2004                       *
******************************************************************************/

DO_COMMAND(do_function)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_FUNCTION], 0);
	}

	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_FUNCTION]) == FALSE)
		{
			show_message(ses, LIST_FUNCTION, "#FUNCTION: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		update_node_list(ses->list[LIST_FUNCTION], arg1, arg2, "", "");

		show_message(ses, LIST_FUNCTION, "#OK. FUNCTION {%s} NOW TRIGGERS {%s}.", arg1, arg2);
	}
	return ses;
}


DO_COMMAND(do_unfunction)
{
	delete_node_with_wild(ses, LIST_FUNCTION, arg);

	return ses;
}

// checked in tinexp.c


/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                       coded by Igor van den Hoven 2007                      *
******************************************************************************/

DO_COMMAND(do_gag)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_GAG], 0);
	}
	else
	{
		update_node_list(ses->list[LIST_GAG], arg1, "", "", "");

		show_message(ses, LIST_GAG, "#OK. {%s} IS NOW GAGGED.", arg1);
	}
	return ses;
}


DO_COMMAND(do_ungag)
{
	delete_node_with_wild(ses, LIST_GAG, arg);

	return ses;
}

void check_all_gags(struct session *ses, char *original, char *line)
{
	struct listroot *root = ses->list[LIST_GAG];
	struct listnode *node;

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		node = root->list[root->update];

		if (check_one_regexp(ses, node, line, original, 0))
		{
//			show_debug(ses, LIST_GAG, "#DEBUG GAG {%s}", node->arg1);

			if (node->shots && --node->shots == 0)
			{
				delete_node_list(ses, LIST_GAG, node);
			}
			ses->gagline++;

			return;
		}
	}
}


/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                          coded by Bill Reiss 1993                           *
*                     recoded by Igor van den Hoven 2004                      *
******************************************************************************/


DO_COMMAND(do_highlight)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_HIGHLIGHT], 0);
	}
	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_HIGHLIGHT]) == FALSE)
		{
			show_message(ses, LIST_HIGHLIGHT, "#HIGHLIGHT: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		if (!is_color_name(arg2))
		{
			tintin_printf2(ses, "#HIGHLIGHT: VALID COLORS ARE:\n");
			tintin_printf2(ses, "reset, bold, light, faint, dim, dark, underscore, blink, reverse, black, red, green, yellow, blue, magenta, cyan, white, b black, b red, b green, b yellow, b blue, b magenta, b cyan, b white, azure, ebony, jade, lime, orange, pink, silver, tan, violet.");
		}
		else
		{
			update_node_list(ses->list[LIST_HIGHLIGHT], arg1, arg2, arg3, "");

			show_message(ses, LIST_HIGHLIGHT, "#OK. {%s} NOW HIGHLIGHTS {%s} @ {%s}.", arg1, arg2, arg3);
		}
	}
	return ses;
}


DO_COMMAND(do_unhighlight)
{
	delete_node_with_wild(ses, LIST_HIGHLIGHT, arg);

	return ses;
}

void check_all_highlights(struct session *ses, char *original, char *line)
{
	struct listroot *root = ses->list[LIST_HIGHLIGHT];
	struct listnode *node;
	char *pto, *ptl, *ptm;
	char *match, *color, *output, *plain;
	int len;

	push_call("check_all_highlights(%p,%p,%p)",ses,original,line);

	match  = str_alloc_stack(0);
	color  = str_alloc_stack(0);
	output = str_alloc_stack(0);
	plain  = str_alloc_stack(0);

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		node = root->list[root->update];

		if (check_one_regexp(ses, node, line, original, 0))
		{
			get_color_names(ses, node->arg2, color);

			*output = *gtd->color_reset = 0;

			pto = original;
			ptl = line;

			do
			{
				if (*gtd->vars[0] == 0)
				{
					break;
				}

				strcpy(match, gtd->vars[0]);

				strip_vt102_codes(match, plain);

				if (*node->arg1 == '~')
				{
					ptm = strstr(pto, match);

					len = strlen(match);
				}
				else
				{
					ptm = strip_vt102_strstr(pto, match, &len);

					ptl = strstr(ptl, match) + strlen(match);
				}

				*ptm = 0;

				get_color_codes(gtd->color_reset, pto, gtd->color_reset, GET_ALL);

				cat_sprintf(output, "%s%s%s\e[0m%s", pto, color, plain, gtd->color_reset);

				pto = ptm + len;

				show_debug(ses, LIST_HIGHLIGHT, "#DEBUG HIGHLIGHT {%s}", node->arg1);
			}
			while (check_one_regexp(ses, node, ptl, pto, 0));

			if (node->shots && --node->shots == 0)
			{
				delete_node_list(ses, LIST_HIGHLIGHT, node);
			}
			strcat(output, pto);

			strcpy(original, output);
		}
	}
	pop_call();
	return;
}


/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                      coded by Igor van den Hoven 2006                       *
******************************************************************************/


DO_COMMAND(do_macro)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_MACRO], 0);
	}
	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_MACRO]) == FALSE)
		{
			show_message(ses, LIST_MACRO, "#MACRO: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		tintin_macro_compile(arg1, arg3);

		update_node_list(ses->list[LIST_MACRO], arg1, arg2, "", arg3);

		show_message(ses, LIST_MACRO, "#OK. MACRO {%s} NOW TRIGGERS {%s}.", arg1, arg2);
	}
	return ses;
}


DO_COMMAND(do_unmacro)
{
	delete_node_with_wild(ses, LIST_MACRO, arg);

	return ses;
}

// checked in input.c


/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/


DO_COMMAND(do_prompt)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_PROMPT], 0);
	}
	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_PROMPT]) == FALSE)
		{
			show_message(ses, LIST_PROMPT, "#PROMPT: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		arg3 = str_alloc_stack(0);
		arg4 = str_alloc_stack(0);

		arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg4, GET_ONE, SUB_VAR|SUB_FUN);

		update_node_list(ses->list[LIST_PROMPT], arg1, arg2, arg3, arg4);

		show_message(ses, LIST_PROMPT, "#OK. {%s} NOW PROMPTS {%s} @ {%s} {%s}.", arg1, arg2, arg3, arg4);
	}
	return ses;
}


DO_COMMAND(do_unprompt)
{
	delete_node_with_wild(ses, LIST_PROMPT, arg);

	return ses;
}


int check_all_prompts(struct session *ses, char *original, char *line)
{
	struct listroot *root = ses->list[LIST_PROMPT];
	struct listnode *node;

	if (!HAS_BIT(ses->flags, SES_FLAG_SPLIT))
	{
		return 0;
	}

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		node = root->list[root->update];

		if (check_one_regexp(ses, node, line, original, 0))
		{
			if (*node->arg2)
			{
				substitute(ses, node->arg2, line, SUB_ARG);
				substitute(ses, line, original, SUB_VAR|SUB_FUN|SUB_COL|SUB_ESC);

				strip_vt102_codes(original, line);
			}

			show_debug(ses, LIST_PROMPT, "#DEBUG PROMPT {%s}", node->arg1);

			split_show(ses, original, node->arg3, node->arg4);

			if (node->shots && --node->shots == 0)
			{
				delete_node_list(ses, LIST_PROMPT, node);
			}
			ses->gagline = 1;
		}
	}
	return 0;
}


/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                         coded by Peter Unold 1992                           *
*                    recoded by Igor van den Hoven 2004                       *
******************************************************************************/


DO_COMMAND(do_substitute)
{
	char *str;

	str = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, str, arg2, GET_ALL);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_SUBSTITUTE], 0);
	}
	else if (*str == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_SUBSTITUTE]) == FALSE)
		{
			show_message(ses, LIST_SUBSTITUTE, "#SUBSTITUTE: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		update_node_list(ses->list[LIST_SUBSTITUTE], arg1, arg2, arg3, "");

		show_message(ses, LIST_SUBSTITUTE, "#OK. {%s} IS NOW SUBSTITUTED AS {%s} @ {%s}.", arg1, arg2, arg3);
	}
	return ses;
}


DO_COMMAND(do_unsubstitute)
{
	delete_node_with_wild(ses, LIST_SUBSTITUTE, arg);

	return ses;
}

void check_all_substitutions(struct session *ses, char *original, char *line)
{
	char *match, *subst, *result, *temp, *ptl, *ptm, *pto, *ptr;
	struct listroot *root = ses->list[LIST_SUBSTITUTE];
	struct listnode *node;
	int len;

	push_call("check_all_substitutions(%p,%p,%p)",ses,original,line);

	match  = str_alloc_stack(0);
	subst  = str_alloc_stack(0);
	result = str_alloc_stack(0);
	temp   = str_alloc_stack(0);
	
	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		node = root->list[root->update];

		if (check_one_regexp(ses, node, line, original, 0))
		{
			pto = original;
			ptl = line;
			ptr = result;

			*result = *gtd->color_reset = 0;

			do
			{
				if (*gtd->vars[0] == 0)
				{
					break;
				}

				strcpy(match, gtd->vars[0]);

				substitute(ses, node->arg2, temp, SUB_ARG);

				if (*node->arg1 == '~')
				{
					ptm = strstr(pto, match);

					len = strlen(match);
				}
				else
				{
					ptm = strip_vt102_strstr(pto, match, &len);

					ptl = strstr(ptl, match) + strlen(match);
				}

				*ptm = 0;

				get_color_codes(gtd->color_reset, pto, gtd->color_reset, GET_ALL);

				substitute(ses, temp, subst, SUB_VAR|SUB_FUN|SUB_COL|SUB_ESC);

				ptr += sprintf(ptr, "%s%s", pto, subst);

				pto = ptm + len;

				show_debug(ses, LIST_SUBSTITUTE, "#DEBUG SUBSTITUTE {%s} {%s}", node->arg1, match);
			}
			while (*pto && check_one_regexp(ses, node, ptl, pto, 0));

			if (node->shots && --node->shots == 0)
			{
				delete_node_list(ses, LIST_SUBSTITUTE, node);
			}
			strcpy(ptr, pto);

			strcpy(original, result);

			strip_vt102_codes(original, line);
		}
	}
	pop_call();
	return;
}


/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                      coded by Igor van den Hoven 2006                       *
******************************************************************************/


DO_COMMAND(do_tab)
{
	sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_TAB], 0);
	}
	else
	{
		update_node_list(ses->list[LIST_TAB], arg1, "", "", "");

		show_message(ses, LIST_TAB, "#OK. {%s} IS NOW A TAB.", arg1);
	}
	return ses;
}


DO_COMMAND(do_untab)
{
	delete_node_with_wild(ses, LIST_TAB, arg);

	return ses;
}

// checked in cursor.c


/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                         coded by Peter Unold 1992                           *
*                     recoded by Igor van den Hoven 2004                      *
******************************************************************************/


DO_COMMAND(do_tick)
{
	char time[NUMBER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg3 == 0)
	{
		strcpy(time, "60");
	}
	else
	{
		get_number_string(ses, arg3, time);
	}

	sprintf(arg3, "%lld", ++gtd->utime);

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_TICKER], 0);
	}
	else if (*arg1 && *arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_TICKER]) == FALSE) 
		{
			show_message(ses, LIST_TICKER, "#TICK, NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		update_node_list(ses->list[LIST_TICKER], arg1, arg2, time, arg3);

		show_message(ses, LIST_TICKER, "#OK. #TICK {%s} NOW EXECUTES {%s} EVERY {%s} SECONDS.", arg1, arg2, time);
	}
	return ses;
}


DO_COMMAND(do_untick)
{
	delete_node_with_wild(ses, LIST_TICKER, arg);

	return ses;
}


// checked in update.c

