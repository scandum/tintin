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
*                          coded by Bill Reiss 1993                           *
*                     recoded by Igor van den Hoven 2004                      *
******************************************************************************/


#include "tintin.h"


DO_COMMAND(do_highlight)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE], temp[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg3 == 0)
	{
		strcpy(arg3, "5");
	}

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
		if (get_highlight_codes(ses, arg2, temp) == FALSE)
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
	char match[BUFFER_SIZE], color[BUFFER_SIZE], reset[BUFFER_SIZE], output[BUFFER_SIZE], plain[BUFFER_SIZE];
	int len;

	push_call("check_all_highlights(%p,%p,%p)",ses,original,line);

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		if (check_one_regexp(ses, root->list[root->update], line, original, 0))
		{
			node = root->list[root->update];

			get_highlight_codes(ses, node->arg2, color);

			*output = *reset = 0;

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

				get_color_codes(reset, pto, reset);

				cat_sprintf(output, "%s%s%s\e[0m%s", pto, color, plain, reset);

				pto = ptm + len;

				show_debug(ses, LIST_HIGHLIGHT, "#DEBUG HIGHLIGHT {%s}", node->arg1);
			}
			while (check_one_regexp(ses, node, ptl, pto, 0));

			strcat(output, pto);

			strcpy(original, output);
		}
	}
	pop_call();
	return;
}

int get_highlight_codes(struct session *ses, char *string, char *result)
{
	int cnt;

	*result = 0;

	if (*string == '<')
	{
		substitute(ses, string, result, SUB_COL);

		return TRUE;
	}

	if (*string == '\\')
	{
		substitute(ses, string, result, SUB_ESC);

		return TRUE;
	}

	while (*string)
	{
		if (isalpha((int) *string))
		{
			for (cnt = 0 ; *color_table[cnt].name ; cnt++)
			{
				if (is_abbrev(color_table[cnt].name, string))
				{
					substitute(ses, color_table[cnt].code, result, SUB_COL);

					result += strlen(result);

					break;
				}
			}

			if (*color_table[cnt].name == 0)
			{
				return FALSE;
			}

			string += strlen(color_table[cnt].name);
		}

		switch (*string)
		{
			case ' ':
			case ';':
			case ',':
			case '{':
			case '}':
				string++;
				break;

			case 0:
				return TRUE;

			default:
				return FALSE;
		}
	}
	return TRUE;
}
