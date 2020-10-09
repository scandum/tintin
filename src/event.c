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
	struct listroot *root = ses->list[LIST_EVENT];
	struct listnode *node;
	int cnt, found, index;
	char symbol;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " EVENTS ");

		for (cnt = 0 ; *event_table[cnt].name != 0 ; cnt++)
		{
			symbol = ' ';

			if (event_table[cnt].level)
			{
				symbol = '-';

				if (HAS_BIT(ses->event_flags, event_table[cnt].flags))
				{
					found = 0;

					for (index = 0 ; index < root->used ; index++)
					{
						node = root->list[index];

						if (!strncmp(event_table[cnt].name, node->arg1, strlen(event_table[cnt].name)))
						{
							found++;
						}
					}

					switch (found)
					{
						case 0:
							break;
						case 1:
							symbol = '+';
							break;
						default:
							symbol = '*';
							break;
					}
				}
			}
			tintin_printf2(ses, "%c [%-26s] [%-8s] %s", symbol, event_table[cnt].name, event_table[cnt].group, event_table[cnt].desc);
		}
		tintin_header(ses, 80, "");
	}
	else if (*arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, root) == FALSE)
		{
			symbol = 0;

			for (cnt = 0 ; *event_table[cnt].name != 0 ; cnt++)
			{
				if (!strcasecmp(event_table[cnt].group, arg1))
				{
					symbol = ' ';

					if (event_table[cnt].level)
					{
						symbol = '-';

						if (HAS_BIT(ses->event_flags, event_table[cnt].flags))
						{
							found = 0;

							for (index = 0 ; index < root->used ; index++)
							{
								node = root->list[index];

								if (!strncmp(event_table[cnt].name, node->arg1, str_len(node->arg1)))
								{
									found++;
								}
							}

							switch (found)
							{
								case 0:
									break;
								case 1:
									symbol = '+';
									break;
								default:
									symbol = '*';
									break;
							}
						}
					}
					tintin_printf2(ses, "%c [%-26s] [%-8s] %s", symbol, event_table[cnt].name, event_table[cnt].group, event_table[cnt].desc);
				}
			}

			if (symbol == 0)
			{
				show_message(ses, LIST_ALIAS, "#EVENT: NO MATCH(ES) FOUND FOR {%s}.", arg1);
			}
		}
	}
	else
	{
		for (index = 0 ; *event_table[index].name != 0 ; index++)
		{
			if (!strncmp(event_table[index].name, arg1, strlen(event_table[index].name)))
			{
				show_message(ses, LIST_EVENT, "#EVENT {%s} HAS BEEN SET TO {%s}.", arg1, arg2);

				SET_BIT(ses->event_flags, event_table[index].flags);
				SET_BIT(gtd->event_flags, event_table[index].flags);

				node = update_node_list(root, arg1, arg2, "", "");

				node->val32[0] = index;
				node->val32[1] = event_table[index].flags;

				if (node->val32[1] == 0)
				{
					tintin_printf2(gtd->ses, "\e[1;33mdo_event: event_table[index].flags == 0");
				}
				event_table[index].level++;

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
	char *name, *buf;
	va_list list;
	int cnt, sub, found;

	if (gtd->level->ignore)
	{
		return 0;
	}

	sub = HAS_BIT(flags, SUB_SEC) ? SUB_ARG|SUB_SEC : SUB_ARG;

	DEL_BIT(flags, SUB_ARG|SUB_SEC);

	if (flags == 0)
	{
		tintin_printf2(ses, "\e[1;31merror: check_all_events: flags = %d", flags);

		return 0;
	}

	if (gtd->level->info == 0 && !HAS_BIT(gtd->ses->list[LIST_EVENT]->flags, LIST_FLAG_INFO))
	{
		if (!HAS_BIT(gtd->event_flags, flags))
		{
			return 0;
		}

		if (ses && !HAS_BIT(ses->event_flags, flags))
		{
			return 0;
		}
	}

	if (args)
	{
		va_start(list, fmt);

		if (vasprintf(&name, fmt, list) == -1)
		{
			syserr_printf(ses, "check_all_events: vasprintf:");
		}

		va_end(list); 
	}
	else
	{
		name = strdup(fmt);
	}

	push_call("check_all_events(%p,%d,%d,%d,%s, ...)",ses,flags,args,vars,name);

	buf = str_alloc_stack(0);

	found = 0;

	for (ses_ptr = ses ? ses : gts ; ses_ptr ; ses_ptr = ses_ptr->next)
	{
		if (!HAS_BIT(ses_ptr->list[LIST_EVENT]->flags, LIST_FLAG_IGNORE))
		{
			if (!HAS_BIT(flags, EVENT_FLAG_UPDATE))
			{
				show_info(ses_ptr, LIST_EVENT, "#INFO EVENT {%s} ARGUMENTS {%d}", name, vars);
			}

			node = search_node_list(ses_ptr->list[LIST_EVENT], name);

			if (node)
			{
				if (node->val32[1] != flags)
				{
					tintin_printf2(ses, "\e[1;31merror: check_all_events: %s: flags: %d != %d", name, flags, node->val32[1]);
				}

				if (vars > 0 && found == 0)
				{
					found = 1;

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

				substitute(ses_ptr, node->arg2, buf, sub);

				if (!HAS_BIT(flags, EVENT_FLAG_UPDATE) && HAS_BIT(ses_ptr->list[LIST_EVENT]->flags, LIST_FLAG_DEBUG))
				{
					show_debug(ses_ptr, LIST_EVENT, "#DEBUG EVENT {%s} {%s}", node->arg1, node->arg2);
				}

				if (node->shots && --node->shots == 0)
				{
					delete_node_list(ses, LIST_EVENT, node);
				}

				gtd->level->quiet += HAS_BIT(flags, EVENT_FLAG_UPDATE) ? 1 : 0;

				script_driver(ses_ptr, LIST_EVENT, buf);

				gtd->level->quiet -= HAS_BIT(flags, EVENT_FLAG_UPDATE) ? 1 : 0;

				if (ses)
				{
					free(name);

					pop_call();
					return TRUE;
				}
			}
		}

		if (ses)
		{
			goto end;
		}
	}

	end:

	free(name);

	pop_call();
	return 0;
}

void mouse_event_handler(struct session *ses, char *arg1, char *arg2, int row, int col, int rev_row, int rev_col, int pix_row, int pix_col, char *grid, char *word, char *line, char *link, char *name);

void mouse_handler(struct session *ses, int flags, int row, int col)
{
	char *arg1, *arg2, *line, *name, link[NUMBER_SIZE];
	char *grid, *grid_val[] = { "nw", "n", "ne", "w", "c", "e", "sw", "s", "se" };
	static char word[BUFFER_SIZE];
	static char last[100], dir[10];
	static long long click[3];
	static int swipe[10];
	int debug, info, rev_row, rev_col, option, pix_row, pix_col, char_row, char_col, grid_pos = 4;

	push_call("mouse_handler(%p,%d,%d,%d,%c)",ses,flags,row,col);

	arg1 = str_alloc_stack(0);
	arg2 = str_alloc_stack(0);

	line = str_alloc_stack(0);
	name = str_alloc_stack(0);

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_MOUSEPIXELS))
	{
		pix_row = row;
		pix_col = col;

		char_row = (row - 1) % gtd->screen->char_height + 1;
		char_col = (col - 1) % gtd->screen->char_width  + 1;

//		if (flags == 8) tintin_printf2(NULL, "debug: rpx %4d cpx %4d\n", pix_row, pix_col);

		row = 1 + (row - 1) / gtd->screen->char_height;
		col = 1 + (col - 1) / gtd->screen->char_width;

//		if (flags == 8) tintin_printf2(NULL, "debug: row %4d col %4d\n", row, col);

//		if (flags == 8) tintin_printf2(NULL, "debug: rpx %4d/%d cpx %4d/%d\n\n\n", char_row, gtd->screen->char_height, char_col, gtd->screen->char_width);

		grid_pos  = char_row * 3 / gtd->screen->char_height * 3;
		grid_pos += char_col * 3 / gtd->screen->char_width;
	}
	else
	{
		pix_row = 0;
		pix_col = 0;
		char_row = 0;
		char_col = 0;
	}
	grid = grid_val[grid_pos];

/*
	if (!HAS_BIT(ses->event_flags, EVENT_FLAG_MOUSE))
	{
		if (!HAS_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO) && !HAS_BIT(ses->list[LIST_EVENT]->flags, LIST_FLAG_INFO))
		{
			pop_call();
			return;
		}
	}
*/
	if (HAS_BIT(flags, MOUSE_FLAG_MOTION))
	{
		strcpy(arg1, "MOVED");
	}
	else
	{
		if (HAS_BIT(flags, MOUSE_FLAG_RELEASE))
		{
			strcpy(arg1, "RELEASED");
		}
		else
		{
			if (HAS_BIT(flags, MOUSE_FLAG_WHEEL))
			{
				strcpy(arg1, "SCROLLED");
			}
			else
			{
				strcpy(arg1, "PRESSED");
			}
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

	if (row < 1)
	{
		tintin_printf2(ses, "mouse_handler: bad row: (row,col)=(%d,%d)", row, col);
		pop_call();
		return;
	}
	else if (row > gtd->screen->rows)
	{
		tintin_printf2(ses, "mouse_handler: bad col: (row,col)=(%d,%d)", row, col);
		pop_call();
		return;
	}

	get_line_screen(line, row - 1);

	option = get_link_screen(ses, name, word, flags, row, col);

	link[0] = 0;

	switch (option)
	{
		case 1:
			strcpy(link, "LINK");
			break;
		case 2:
			strcpy(link, "SECURE LINK");
			break;
		case 3:
			strcpy(link, "URL LINK");
			break;
		case 4:
			strcpy(link, "JUMP MARK");
			break;
		case 5:
			strcpy(link, "JUMP LINK");
			break;
		case 6:
			if (flags == 0)
			{
				tintin_printf2(ses, "\e[1;32m         %s\n", capitalize(word));

				command(ses, do_help, "{%s}", word);
			}
			break;
	}

	if (HAS_BIT(flags, MOUSE_FLAG_WHEEL))
	{
		switch (HAS_BIT(flags, MOUSE_FLAG_BUTTON_A|MOUSE_FLAG_BUTTON_B))
		{
			case 0:
				strcat(arg2, "MOUSE WHEEL UP");
				break;
			case MOUSE_FLAG_BUTTON_A:
				strcat(arg2, "MOUSE WHEEL DOWN");
				break;
			case MOUSE_FLAG_BUTTON_B:
				strcat(arg2, "MOUSE WHEEL LEFT");
				break;
			case MOUSE_FLAG_BUTTON_A|MOUSE_FLAG_BUTTON_B:
				strcat(arg2, "MOUSE WHEEL RIGHT");
				break;
		}
	}
	else
	{
		switch (HAS_BIT(flags, MOUSE_FLAG_BUTTON_A|MOUSE_FLAG_BUTTON_B))
		{
			case 0:
				strcat(arg2, "MOUSE BUTTON ONE");
				break;
			case MOUSE_FLAG_BUTTON_A:
				strcat(arg2, "MOUSE BUTTON TWO");
				break;
			case MOUSE_FLAG_BUTTON_B:
				strcat(arg2, "MOUSE BUTTON THREE");
				break;
			case MOUSE_FLAG_BUTTON_A|MOUSE_FLAG_BUTTON_B:
				strcat(arg2, "MOUSE");
				break;
		}
	}

	debug = HAS_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG) ? 1 : 0;
	info  = HAS_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO) ? 1 : 0;

	gtd->level->debug += debug;
	gtd->level->info  += info;
/*
	if (!HAS_BIT(ses->list[LIST_BUTTON]->flags, LIST_FLAG_IGNORE))
	{
		check_all_buttons(ses, row, col, arg1, arg2, word, line);
	}
*/
	rev_row = -1 - (gtd->screen->rows - row);
	rev_col = -1 - (gtd->screen->cols - col);


	mouse_event_handler(ses, arg1, arg2, row, col, rev_row, rev_col, pix_row, pix_col, grid, word, line, link, name);

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
					mouse_event_handler(ses, "TRIPLE-CLICKED", arg2, row, col, rev_row, rev_col, pix_row, pix_col, grid, word, line, link, name);

					strcpy(last, "");
				}
				else
				{
					mouse_event_handler(ses, "DOUBLE-CLICKED", arg2, row, col, rev_row, rev_col, pix_row, pix_col, grid, word, line, link, name);
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
			check_all_events(ses, EVENT_FLAG_MOUSE, 0, 12, "SWIPED", dir, arg2, ntos(swipe[0]), ntos(swipe[1]), ntos(swipe[2]), ntos(swipe[3]), ntos(swipe[4]), ntos(swipe[5]), ntos(swipe[6]), ntos(swipe[7]), ntos(swipe[8]), ntos(swipe[9]));			
			check_all_events(ses, EVENT_FLAG_MOUSE, 1, 12, "SWIPED %s", dir, dir, arg2, ntos(swipe[0]), ntos(swipe[1]), ntos(swipe[2]), ntos(swipe[3]), ntos(swipe[4]), ntos(swipe[5]), ntos(swipe[6]), ntos(swipe[7]), ntos(swipe[8]), ntos(swipe[9]));
			check_all_events(ses, EVENT_FLAG_MOUSE, 2, 12, "SWIPED %s %s", arg2, dir, dir, arg2, ntos(swipe[0]), ntos(swipe[1]), ntos(swipe[2]), ntos(swipe[3]), ntos(swipe[4]), ntos(swipe[5]), ntos(swipe[6]), ntos(swipe[7]), ntos(swipe[8]), ntos(swipe[9]));
		}
		else if (utime() - click[0] >= 500000)
		{
			mouse_event_handler(ses, "LONG-CLICKED", arg2, row, col, rev_row, rev_col, pix_row, pix_col, grid, word, line, link, name);
		}
		else if (click[0] - click[1] >= 500000)
		{
			if (abs(swipe[0] - swipe[4]) <= 3 && abs(swipe[1] - swipe[5]) <= 3)
			{
				mouse_event_handler(ses, "SHORT-CLICKED", arg2, row, col, rev_row, rev_col, pix_row, pix_col, grid, word, line, link, name);
			}
		}
	}

	gtd->level->debug -= debug;
	gtd->level->info  -= info;

	pop_call();
	return;
}

void mouse_event_handler(struct session *ses, char *arg1, char *arg2, int row, int col, int rev_row, int rev_col, int pix_row, int pix_col, char *grid, char *word, char *line, char *link, char *name)
{
	if (!HAS_BIT(ses->list[LIST_BUTTON]->flags, LIST_FLAG_IGNORE))
	{
		check_all_buttons(ses, row, col, arg1, arg2, word, line);
	}

	if (*link)
	{
		check_all_events(ses, EVENT_FLAG_MOUSE, 3, 8, "%s %s %s", arg1, link, arg2, ntos(row), ntos(col), ntos(rev_row), ntos(-1 - (gtd->screen->cols - col)), word, line, name, grid);

		if (*name)
		{
			check_all_events(ses, EVENT_FLAG_MOUSE, 4, 8, "%s %s %s %s", arg1, link, name, arg2, ntos(row), ntos(col), ntos(rev_row), ntos(-1 - (gtd->screen->cols - col)), word, line, name, grid);
		}
	}

	if (row >= ses->input->top_row && row <= ses->input->bot_row && col >= ses->input->top_col && row <= ses->input->bot_col)
	{
		check_all_events(ses, EVENT_FLAG_MOUSE, 2, 8, "%s INPUT %s", arg1, arg2, ntos(row), ntos(col - (ses->input->top_col - 1)), ntos(rev_row), ntos(rev_col - (ses->input->top_col - 1)), word, line, name, grid);
	}

	check_all_events(ses, EVENT_FLAG_MOUSE, 2, 8, "%s %s", arg1, arg2, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line, name, grid);

	check_all_events(ses, EVENT_FLAG_MOUSE, 3, 8, "%s %s %d", arg1, arg2, row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line, name, grid);

	check_all_events(ses, EVENT_FLAG_MOUSE, 3, 8, "%s %s %d", arg1, arg2, rev_row, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), word, line, name, grid);

	map_mouse_handler(ses, arg1, arg2, row, col, rev_row, rev_col, pix_row, pix_col);
}
