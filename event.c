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
*                      coded by Igor van den Hoven 2007                       *
******************************************************************************/


#include "tintin.h"

DO_COMMAND(do_event)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	struct listnode *node;
	int cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 == 0)
	{
		tintin_header(ses, " EVENTS ");

		for (cnt = 0 ; *event_table[cnt].name != 0 ; cnt++)
		{
			tintin_printf2(ses, "%s [%-27s] %s", search_node_list(ses->list[LIST_EVENT], event_table[cnt].name) ? "+" : " ", event_table[cnt].name, event_table[cnt].desc);
		}
		tintin_header(ses, "");
	}
	else if (*arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, ses->list[LIST_EVENT]) == FALSE)
		{
			show_message(ses, LIST_ALIAS, "#EVENT: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		for (cnt = 0 ; *event_table[cnt].name != 0 ; cnt++)
		{
			if (!strncmp(event_table[cnt].name, arg1, strlen(event_table[cnt].name)))
			{
				show_message(ses, LIST_EVENT, "#EVENT {%s} HAS BEEN SET TO {%s}.", arg1, arg2);

				SET_BIT(ses->event_flags, event_table[cnt].flags);

				node = update_node_list(ses->list[LIST_EVENT], arg1, arg2, "", "");

				node->val64 = event_table[cnt].flags;

				return ses;
			}
		}
		show_error(ses, LIST_EVENT, "#EVENT {%s} IS NOT AN EXISTING EVENT.", arg1);
	}
	return ses;
}

DO_COMMAND(do_unevent)
{
	delete_node_with_wild(ses, LIST_EVENT, arg);

	return ses;
}

int check_all_events(struct session *ses, int flags, int args, int vars, char *fmt, ...)
{
	struct session *ses_ptr;
	struct listnode *node;
	char *name, buf[BUFFER_SIZE];
	va_list list;
	int cnt;

	if (gtd->level->ignore)
	{
		return 0;
	}

	if (args)
	{
		va_start(list, fmt);

		vasprintf(&name, fmt, list);

		va_end(list); 
	}
	else
	{
		name = strdup(fmt);
	}

	push_call("check_all_events(%p,%d,%d,%d,%s, ...)",ses,flags,args,vars,name);

	for (ses_ptr = ses ? ses : gts ; ses_ptr ; ses_ptr = ses_ptr->next)
	{
		if (!HAS_BIT(ses_ptr->list[LIST_EVENT]->flags, LIST_FLAG_IGNORE))
		{
			show_info(ses_ptr, LIST_EVENT, "#INFO EVENT {%s}", name);

			node = search_node_list(ses_ptr->list[LIST_EVENT], name);

			if (node)
			{
				if (vars)
				{
					va_start(list, fmt);

					for (cnt = 0 ; cnt < args ; cnt++)
					{
						va_arg(list, char *);
					}

					for (cnt = 0 ; cnt < vars ; cnt++)
					{
						RESTRING(gtd->vars[cnt], va_arg(list, char *));
					}
					va_end(list);
				}

				substitute(ses_ptr, node->arg2, buf, flags);

				if (HAS_BIT(ses_ptr->list[LIST_EVENT]->flags, LIST_FLAG_DEBUG))
				{
					show_debug(ses_ptr, LIST_EVENT, "#DEBUG EVENT {%s} (%s}", node->arg1, node->arg2);
				}

				if (HAS_BIT(node->flags, NODE_FLAG_ONESHOT))
				{
					delete_node_list(ses, LIST_EVENT, node);
				}

				script_driver(ses_ptr, LIST_EVENT, buf);

				if (ses)
				{
					free(name);

					pop_call();
					return 1;
				}
			}
		}

		if (ses)
		{
			free(name);

			pop_call();
			return 0;
		}
	}
	free(name);

	pop_call();
	return 0;
}

void mouse_handler(struct session *ses, int flags, int row, int col, char type)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], line[BUFFER_SIZE], word[BUFFER_SIZE];
	static char last[100], dir[10];
	static long long click[3];
	static int swipe[10];
	int debug, info, rev_row, rev_col;

	push_call("mouse_handler(%p,%d,%d,%d,%c)",ses,flags,row,col,type);

	if (!HAS_BIT(ses->event_flags, EVENT_FLAG_MOUSE))
	{
		if (!HAS_BIT(ses->flags, SES_FLAG_MOUSEINFO) && !HAS_BIT(ses->list[LIST_EVENT]->flags, LIST_FLAG_INFO))
		{
			return;
		}
	}

	if (HAS_BIT(flags, MOUSE_FLAG_MOTION))
	{
		strcpy(arg1, "MOVED");
	}
	else
	{
		switch (type)
		{
			case 'M':
				if (HAS_BIT(flags, MOUSE_FLAG_WHEEL))
				{
					strcpy(arg1, "SCROLLED");
				}
				else
				{
					strcpy(arg1, "PRESSED");
				}
				break;
			case 'm':
				strcpy(arg1, "RELEASED");
				break;
			default:
				strcpy(arg1, "UNKNOWN");
				break;
		}
	}

	arg2[0] = 0;

	if (HAS_BIT(flags, MOUSE_FLAG_CTRL))
	{
		strcat(arg2, "CTRL ");
	}
	if (HAS_BIT(flags, MOUSE_FLAG_ALT))
	{
		strcat(arg2, "ALT ");
	}
	if (HAS_BIT(flags, MOUSE_FLAG_SHIFT))
	{
		strcat(arg2, "SHIFT ");
	}

	if (HAS_BIT(flags, MOUSE_FLAG_EXTRA))
	{
		strcat(arg2, "EXTRA ");
	}

	if (HAS_BIT(flags, MOUSE_FLAG_UNKNOWN))
	{
		strcat(arg2, "256 ");
	}

	if (HAS_BIT(flags, MOUSE_FLAG_WHEEL))
	{
		strcat(arg2, "MOUSE WHEEL ");
	}
	else
	{
		strcat(arg2, "MOUSE BUTTON ");
	}

	if (row - 1 < 0)
	{
		tintin_printf2(ses, "mouse_handler: bad row: (row,col)=(%d,%d)", row, col);
		pop_call();
		return;
	}
	else if (row - 1 > gtd->screen->rows)
	{
		tintin_printf2(ses, "mouse_handler: bad col: (row,col)=(%d,%d)", row, col);
		pop_call();
		return;
	}
	else
	{
		strcpy(line, "under development");
		strcpy(word, "under development");
//		get_line_screen(line, row - 1);
//		get_word_screen(word, row - 1, col - 1);
	}

	if (HAS_BIT(flags, MOUSE_FLAG_WHEEL))
	{
		if (HAS_BIT(flags, MOUSE_FLAG_BUTTON_A) && HAS_BIT(flags, MOUSE_FLAG_BUTTON_B))
		{
			strcat(arg2, "RIGHT");
		}
		else if (HAS_BIT(flags, MOUSE_FLAG_BUTTON_B))
		{
			strcat(arg2, "LEFT");
		}
		else if (HAS_BIT(flags, MOUSE_FLAG_BUTTON_A))
		{
			strcat(arg2, "DOWN");
		}
		else
		{
			strcat(arg2, "UP");
		}
	}
	else
	{
		if (HAS_BIT(flags, MOUSE_FLAG_BUTTON_A) && HAS_BIT(flags, MOUSE_FLAG_BUTTON_B))
		{
			strcat(arg2, "FOUR");
		}
		else if (HAS_BIT(flags, MOUSE_FLAG_BUTTON_B))
		{
			strcat(arg2, "THREE");
		}
		else if (HAS_BIT(flags, MOUSE_FLAG_BUTTON_A))
		{
			strcat(arg2, "TWO");
		}
		else
		{
			strcat(arg2, "ONE");
		}
	}

	debug = HAS_BIT(ses->flags, SES_FLAG_MOUSEDEBUG) ? 1 : 0;
	info  = HAS_BIT(ses->flags, SES_FLAG_MOUSEINFO) ? 1 : 0;

	gtd->level->debug += debug;
	gtd->level->info  += info;

	check_all_buttons(ses, row, col, arg1, arg2, word, line);

	rev_row = -1 - (gtd->screen->rows - row);
	rev_col = -1 - (gtd->screen->cols - col);

	check_all_events(ses, SUB_ARG, 2, 6, "%s %s", arg1, arg2, ntos(row), ntos(col), ntos(rev_row), ntos(-1 - (gtd->screen->cols - col)), word, line);

	check_all_events(ses, SUB_ARG, 3, 6, "%s %s %d", arg1, arg2, row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

	check_all_events(ses, SUB_ARG, 3, 6, "%s %s %d", arg1, arg2, rev_row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

	map_mouse_handler(ses, arg1, arg2, col, row, 0, 0);

	if (!strcmp(arg1, "PRESSED"))
	{
		sprintf(arg1, "PRESSED %s %d %d", arg2, col, row);

		swipe[0] = row;
		swipe[1] = col;
		swipe[2] = rev_row;
		swipe[3] = rev_col;

		if (!strcmp(arg1, last))
		{
			click[2] = click[1];
			click[1] = click[0];
			click[0] = utime();

			if (click[0] - click[1] < 500000)
			{
				if (click[0] - click[2] < 500000)
				{
					check_all_buttons(ses, row, col, "TRIPLE-CLICKED", arg2, word, line);

					check_all_events(ses, SUB_ARG, 1, 6, "TRIPLE-CLICKED %s", arg2, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

					check_all_events(ses, SUB_ARG, 2, 6, "TRIPLE-CLICKED %s %d", arg2, row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

					check_all_events(ses, SUB_ARG, 2, 6, "TRIPLE-CLICKED %s %d", arg2, rev_row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

					map_mouse_handler(ses, "TRIPPLE-CLICKED", arg2, col, row, 0, 0);

					strcpy(last, "");
				}
				else
				{
					check_all_buttons(ses, row, col, "DOUBLE-CLICKED", arg2, word, line);

					check_all_events(ses, SUB_ARG, 1, 6, "DOUBLE-CLICKED %s", arg2, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

					check_all_events(ses, SUB_ARG, 2, 6, "DOUBLE-CLICKED %s %d", arg2, row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

					check_all_events(ses, SUB_ARG, 2, 6, "DOUBLE-CLICKED %s %d", arg2, rev_row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

					map_mouse_handler(ses, "DOUBLE-CLICKED", arg2, col, row, 0, 0);
				}
			}
		}
		else
		{
			click[2] = 0;
			click[1] = 0;
			click[0] = utime();

			sprintf(last, "PRESSED %s %d %d", arg2, col, row);
		}
	}
	else if (!strcmp(arg1, "RELEASED"))
	{
		swipe[4] = row;
		swipe[5] = col;
		swipe[6] = rev_row;
		swipe[7] = rev_col;

		swipe[8] = swipe[4] - swipe[0];
		swipe[9] = swipe[5] - swipe[1];

		if (abs(swipe[8]) > 3 || abs(swipe[9]) > 3)
		{
			if (abs(swipe[8]) <= 3)
			{
				strcpy(dir, swipe[9] > 0 ? "E" : "W");
			}
			else if (abs(swipe[9]) <= 3)
			{
				strcpy(dir, swipe[8] > 0 ? "S" : "N");
			}
			else if (swipe[8] > 0)
			{
				strcpy(dir, swipe[9] > 0 ? "SE" : "SW");
			}
			else
			{
				strcpy(dir, swipe[9] > 0 ? "NE" : "NW");
			}
			check_all_events(ses, SUB_ARG, 0, 12, "SWIPED", dir, arg2, ntos(swipe[0]), ntos(swipe[1]), ntos(swipe[2]), ntos(swipe[3]), ntos(swipe[4]), ntos(swipe[5]), ntos(swipe[6]), ntos(swipe[7]), ntos(swipe[8]), ntos(swipe[9]));			
			check_all_events(ses, SUB_ARG, 1, 12, "SWIPED %s", dir, dir, arg2, ntos(swipe[0]), ntos(swipe[1]), ntos(swipe[2]), ntos(swipe[3]), ntos(swipe[4]), ntos(swipe[5]), ntos(swipe[6]), ntos(swipe[7]), ntos(swipe[8]), ntos(swipe[9]));
			check_all_events(ses, SUB_ARG, 2, 12, "SWIPED %s %s", arg2, dir, dir, arg2, ntos(swipe[0]), ntos(swipe[1]), ntos(swipe[2]), ntos(swipe[3]), ntos(swipe[4]), ntos(swipe[5]), ntos(swipe[6]), ntos(swipe[7]), ntos(swipe[8]), ntos(swipe[9]));
		}
		else if (utime() - click[0] >= 500000)
		{
			check_all_buttons(ses, row, col, "LONG-CLICKED", arg2, word, line);

			check_all_events(ses, SUB_ARG, 1, 6, "LONG-CLICKED %s", arg2, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

			check_all_events(ses, SUB_ARG, 2, 6, "LONG-CLICKED %s %d", arg2, row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

			check_all_events(ses, SUB_ARG, 2, 6, "LONG-CLICKED %s %d", arg2, rev_row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

			map_mouse_handler(ses, "LONG-CLICKED", arg2, col, row, 0, 0);
		}
		else if (click[0] - click[1] >= 500000)
		{
			if (abs(swipe[0] - swipe[4]) <= 3 && abs(swipe[1] - swipe[5]) <= 3)
			{
				check_all_buttons(ses, row, col, "SHORT-CLICKED", arg2, word, line);

				check_all_events(ses, SUB_ARG, 1, 6, "SHORT-CLICKED %s", arg2, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

				check_all_events(ses, SUB_ARG, 2, 6, "SHORT-CLICKED %s %d", arg2, row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

				check_all_events(ses, SUB_ARG, 2, 6, "SHORT-CLICKED %s %d", arg2, rev_row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line);

				map_mouse_handler(ses, "SHORT-CLICKED", arg2, col, row, 0, 0);
			}
		}
	}

	gtd->level->debug -= debug;
	gtd->level->info  -= info;

	pop_call();
	return;
}
