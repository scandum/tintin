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

/*
	todo:
*/

int                 map_grid_x;
int                 map_grid_y;

#define             MAP_SEARCH_DIST 1000
#define             MAP_BF_SIZE 10000

extern  int dir_flags(struct session *ses, int room, int dir);
extern void create_map(struct session *ses, char *arg, int flags);
extern struct room_data *create_room(struct session *ses, char *format, ...);
extern void delete_room(struct session *ses, int room, int exits);
extern struct exit_data *create_exit(struct session *ses, int room, char *format, ...);
extern void delete_exit(struct session *ses, int room, struct exit_data *exit);
extern void search_keywords(struct session *ses, char *arg, char *out, char *var);
extern void map_search_compile(struct session *ses, char *arg, char *var);
extern  int match_room(struct session *ses, int room, struct search_data *search);
extern  int find_location(struct session *ses, char *arg);
extern  int find_path(struct session *ses, char *arg);
extern  int find_room(struct session *ses, char *arg);
extern void goto_room(struct session *ses, int room);
extern  int find_new_room(struct session *ses);
extern struct exit_data *find_exit(struct session *ses, int room, char *arg);
extern struct exit_data *find_exit_vnum(struct session *ses, int room, int vnum);
extern  int get_exit_dir(struct session *ses, char *arg);
extern  int get_exit_length(struct session *ses, struct exit_data *exit);
extern char *get_exit_color(struct session *ses, int room, struct exit_data *exit);
extern  int dir_to_grid(int dir);
extern  int revdir_to_grid(int dir);
extern void set_room_exits(struct session *ses, int room);
extern  int get_room_exits(struct session *ses, int room);
extern  int get_terrain_vnum(struct session *ses, struct room_data *room, int x, int y);
extern char *draw_terrain_symbol(struct session *ses, struct room_data *room, int line, int col, int x, int y, int flags);
extern void displaygrid_build(struct session *ses, int room, int x, int y, int z);
extern void add_undo(struct session *ses, char *format, ...);
extern void del_undo(struct session *ses, struct link_data *link);
extern char *draw_room(struct session *ses, struct room_data *room, int line, int x, int y);
extern  int searchgrid_find(struct session *ses, int from, struct search_data *search);
extern  int searchgrid_walk(struct session *ses, int offset, int from, int dest);
extern void shortest_path(struct session *ses, int run, char *delay, char *arg);
extern void explore_path(struct session *ses, int run, char *left, char *right);
extern  int tunnel_void(struct session *ses, int from, int room, int dir);
extern  int check_global(struct session *ses, int room);
extern  int find_coord(struct session *ses, char *arg);
extern  int spatialgrid_find(struct session *ses, int vnum, int x, int y, int z);
extern void update_terrain(struct session *ses);

DO_COMMAND(do_map)
{
	int cnt;

	push_call("do_map(%p,%p)",ses,arg);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (*arg1 == 0)
	{
		info:

		tintin_header(ses, 80, " MAP OPTIONS ");

		for (cnt = 0 ; *map_table[cnt].fun != NULL ; cnt++)
		{
			if (*map_table[cnt].desc)
			{
				tintin_printf2(ses, "  [%-13s] %s", map_table[cnt].name, map_table[cnt].desc);
			}
		}
		tintin_header(ses, 80, "");

		pop_call();
		return ses;
	}
	else
	{
		for (cnt = 0 ; *map_table[cnt].name ; cnt++)
		{
			if (is_abbrev(arg1, map_table[cnt].name))
			{
				if (map_table[cnt].check > 0 && ses->map == NULL)
				{
					show_error(ses, LIST_COMMAND, "#MAP %s: This session has no map data. Use #map create to create one.", map_table[cnt].name);

					pop_call();
					return ses;
				}
				if (map_table[cnt].check > 1 && ses->map->room_list[ses->map->in_room] == NULL)
				{
					show_error(ses, LIST_COMMAND, "#MAP %s: You are not inside the map. Use #map goto to enter it.", map_table[cnt].name);

					pop_call();
					return ses;
				}
				*arg1 = 0;
				*arg2 = 0;

				if (gtd->level->ignore == 0)
				{
					if (HAS_BIT(map_table[cnt].flags, MAP_FLAG_VTMAP))
					{
						SET_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);
					}
				}
				map_table[cnt].fun (ses, arg, arg1, arg2);

				pop_call();
				return ses;
			}
		}

		goto info;
	}
	pop_call();
	return ses;
}

/*
	Utility functions
*/

void create_map(struct session *ses, char *arg, int flags)
{
	int group, legend;

	push_call("create_map(%p,%p)",ses,arg);

	if (ses->map)
	{
		delete_map(ses);
	}

	ses->map = (struct map_data *) calloc(1, sizeof(struct map_data));
	ses->map->size = atoi(arg) > 0 ? atoi(arg) : 50000;

	ses->map->room_list = (struct room_data **) calloc(ses->map->size, sizeof(struct room_data *));

	ses->map->max_grid_x = 255;
	ses->map->max_grid_y = 101;

	ses->map->grid_rooms = (struct room_data **) calloc(ses->map->max_grid_x * ses->map->max_grid_y, sizeof(struct room_data *));
	ses->map->grid_vnums = (int *) calloc(ses->map->max_grid_x * ses->map->max_grid_y, sizeof(int));

	ses->map->search = calloc(1, sizeof(struct search_data));

	ses->map->flags = MAP_FLAG_ASCIIGRAPHICS|MAP_FLAG_DIRECTION|MAP_FLAG_TERRAIN | flags;

	ses->map->global_exit         = (struct exit_data *) calloc(1, sizeof(struct exit_data));
	ses->map->global_exit->vnum   = ses->map->global_vnum;
	ses->map->global_exit->name   = restringf(ses->map->global_exit->name, "%cnop global", gtd->tintin_char);
	ses->map->global_exit->cmd    = restringf(ses->map->global_exit->cmd, "%cnop global", gtd->tintin_char);
	ses->map->global_exit->data   = strdup("");
	ses->map->global_exit->weight = 1;
	ses->map->global_exit->delay  = 0;
	ses->map->global_exit->color  = strdup("");

	command(ses, do_map, "{COLOR} {RESET}");

	ses->map->display_stamp = 1;
	ses->map->search->stamp = 1;

	command(ses, do_map, "{TERRAIN} {} { }");
/*
	do_map(ses, "TERRAIN BEACH        <eea>~");
	do_map(ses, "TERRAIN CITY         <ebf>-");
	do_map(ses, "TERRAIN DESERT       <ffa>.");
	do_map(ses, "TERRAIN FIELD        <228>.");
	do_map(ses, "TERRAIN FOREST       <128>^");
	do_map(ses, "TERRAIN HILL         <ddd>^");
	do_map(ses, "TERRAIN LAKE         <248>~");
	do_map(ses, "TERRAIN MOUNTAIN     <acf>^");
	do_map(ses, "TERRAIN OCEAN        <148>@");
	do_map(ses, "TERRAIN SWAMP        <bda>.");
	do_map(ses, "TERRAIN UNDERGROUND  <baa>-");
*/

	create_room(ses, "%s", "{1} {0} {} {} { } {} {} {} {} {} {1.0} {}");

	strcpy(arg, "");

	for (group = 0 ; map_legend_group_table[group].name ; group++)
	{
		for (legend = 0 ; map_legend_table[legend].group ; legend++)
		{
			if (*map_legend_group_table[group].group == 0 || is_abbrev(map_legend_group_table[group].group, map_legend_table[legend].group))
			{
				break;
			}
		}

		if (map_legend_table[legend].group)
		{
			map_legend_group_table[group].start = legend;
		}
		else
		{
			show_error(ses, LIST_COMMAND, "create_map: unknown legend group: %s, %s", map_legend_group_table[group].name, map_legend_group_table[group].group);

			continue;
		}

		while (map_legend_table[++legend].group)
		{
			if (*map_legend_group_table[group].group && !is_abbrev(map_legend_group_table[group].group, map_legend_table[legend].group))
			{
				break;
			}
		}
		map_legend_group_table[group].end = legend;
	}

	gtd->level->quiet++;

	command(ses, do_map, "LEGEND RESET");

	gtd->level->quiet--;

	pop_call();
	return;
}

int delete_map(struct session *ses)
{
	int index, cnt;

	for (index = cnt = 0 ; index < ses->map->size ; index++)
	{
		if (ses->map->room_list[index])
		{
			cnt++;

			delete_room(ses, index, FALSE);
		}
	}
	free(ses->map->room_list);

	while (ses->map->undo_head)
	{
		del_undo(ses, ses->map->undo_head);
	}

//	delete_exit(ses, 0, ses->map->global_exit);

	free(ses->map->global_exit->name);
	free(ses->map->global_exit->cmd);
	free(ses->map->global_exit->data);
	free(ses->map->global_exit->color);

	free(ses->map->grid_rooms);
	free(ses->map->grid_vnums);
	free(ses->map->global_exit);
	free(ses->map->search);

	free(ses->map);

	ses->map = NULL;

	kill_list(ses->list[LIST_LANDMARK]);
	kill_list(ses->list[LIST_TERRAIN]);

	return cnt;
}

struct room_data *create_room(struct session *ses, char *format, ...)
{
	char *arg, buf[BUFFER_SIZE], arg1[BUFFER_SIZE];
	struct room_data *newroom;
	va_list args;

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	newroom = (struct room_data *) calloc(1, sizeof(struct room_data));

	arg = get_arg_in_braces(ses, buf, arg1, GET_ONE);

	newroom->vnum = atoi(arg1);

	if (HAS_BIT(ses->map->flags, MAP_FLAG_SYNC) && ses->map->room_list[newroom->vnum] != NULL)
	{
		int vnum = newroom->vnum;

		free(newroom);

		return ses->map->room_list[vnum];
	}

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->flags   = atoi(arg1);
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->color   = strdup(arg1);
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->name    = strdup(arg1);
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->symbol  = strdup(arg1);
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->desc    = strdup(arg1);
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->area    = strdup(arg1);
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->note    = strdup(arg1);
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->terrain = strdup(arg1);
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->data    = strdup(arg1);
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->weight  = atof(arg1);
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE); newroom->id      = strdup(arg1);

	if (HAS_BIT(newroom->flags, ROOM_FLAG_AVOID))
	{
		SET_BIT(newroom->flags, ROOM_FLAG_AVOID_TMP);
	}
	if (HAS_BIT(newroom->flags, ROOM_FLAG_HIDE))
	{
		SET_BIT(newroom->flags, ROOM_FLAG_HIDE_TMP);
	}
	if (HAS_BIT(newroom->flags, ROOM_FLAG_LEAVE))
	{
		SET_BIT(newroom->flags, ROOM_FLAG_LEAVE_TMP);
	}
	if (HAS_BIT(newroom->flags, ROOM_FLAG_VOID))
	{
		SET_BIT(newroom->flags, ROOM_FLAG_VOID_TMP);
	}
	if (HAS_BIT(newroom->flags, ROOM_FLAG_CURVED))
	{
		SET_BIT(newroom->flags, ROOM_FLAG_CURVED_TMP);
	}

	if (newroom->weight <= 0)
	{
		newroom->weight = 1;
	}

	if (newroom->vnum)
	{
		ses->map->room_list[newroom->vnum] = newroom;
	}

	if (!HAS_BIT(ses->map->flags, MAP_FLAG_READ))
	{
		show_message(ses, LIST_PATH, "#MAP CREATE ROOM %5d {%s}.", newroom->vnum, newroom->name);

		check_all_events(ses, EVENT_FLAG_MAP, 0, 2, "MAP CREATE ROOM", ntos(newroom->vnum), newroom->name);
	}

	return newroom;
}

void delete_room_data(struct room_data *room)
{
	free(room->area);
	free(room->color);
	free(room->id);
	free(room->name);
	free(room->symbol);
	free(room->desc);
	free(room->note);
	free(room->terrain);
	free(room->data); 

	return;
}
	
void delete_room(struct session *ses, int room, int exits)
{
	struct exit_data *exit, *exit_next;
	int cnt;

	check_all_events(ses, EVENT_FLAG_MAP, 0, 2, "MAP DELETE ROOM", ntos(ses->map->room_list[room]->vnum), ses->map->room_list[room]->name);

	while (ses->map->room_list[room]->f_exit)
	{
		delete_exit(ses, room, ses->map->room_list[room]->f_exit);
	}

	delete_room_data(ses->map->room_list[room]);

	free(ses->map->room_list[room]);

	ses->map->room_list[room] = NULL;

	if (exits)
	{
		for (cnt = 0 ; cnt < ses->map->size ; cnt++)
		{
			if (ses->map->room_list[cnt])
			{
				for (exit = ses->map->room_list[cnt]->f_exit ; exit ; exit = exit_next)
				{
					exit_next = exit->next;

					if (exit->vnum == room)
					{
						delete_exit(ses, cnt, exit);
					}
				}
			}
		}
	}
}

struct exit_data *create_exit(struct session *ses, int vnum, char *format, ...)
{
	struct exit_data *newexit;
	struct room_data *room;
	va_list args;
	char *arg, buf[BUFFER_SIZE];

	push_call("create_exit(%p,%d,%p)",ses,vnum,format);

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	newexit = (struct exit_data *) calloc(1, sizeof(struct exit_data));

	room = ses->map->room_list[vnum];

	arg = buf;

	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);	newexit->vnum   = atoi(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);

	if (HAS_BIT(ses->map->flags, MAP_FLAG_SYNC) && find_exit(ses, vnum, buf))
	{
		free(newexit);

		pop_call();
		return find_exit(ses, vnum, buf);
	}
	newexit->name = strdup(buf);

	arg = get_arg_in_braces(ses, arg, buf, GET_ALL);	newexit->cmd    = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);	newexit->dir    = atoi(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);	newexit->flags  = atoi(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ALL);	newexit->data   = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);	newexit->weight = atof(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);        newexit->color  = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);        newexit->delay  = atof(buf);

	if (newexit->dir == 0)
	{
		newexit->dir = get_exit_dir(ses, newexit->name);
	}

	newexit->grid = dir_to_grid(newexit->dir);

	if (room->exit_grid[newexit->grid] == NULL)
	{
		room->exit_grid[newexit->grid] = newexit;
	}

	if (newexit->weight <= 0)
	{
		newexit->weight = 1;
	}

	LINK(newexit, room->f_exit, room->l_exit);

	room->exit_size++;

	SET_BIT(room->exit_dirs, (1LL << newexit->dir));

	if (!HAS_BIT(ses->map->flags, MAP_FLAG_READ))
	{
		show_message(ses, LIST_PATH, "#MAP CREATE EXIT {%s} {%s} TO ROOM %d.", newexit->name, newexit->cmd, newexit->vnum);

		check_all_events(ses, EVENT_FLAG_MAP, 0, 4, "MAP CREATE EXIT", ntos(room->vnum), newexit->name,newexit->cmd, ntos(newexit->vnum));
	}
	pop_call();
	return newexit;
}

void delete_exit(struct session *ses, int room, struct exit_data *exit)
{
	check_all_events(ses, EVENT_FLAG_MAP, 0, 4, "MAP DELETE EXIT", ntos(room), exit->name, exit->cmd, ntos(exit->vnum));

	free(exit->name);
	free(exit->cmd);
	free(exit->data);
	free(exit->color);

	UNLINK(exit, ses->map->room_list[room]->f_exit, ses->map->room_list[room]->l_exit)

	set_room_exits(ses, room);

	free(exit);
}

int get_exit_dir(struct session *ses, char *arg)
{
	struct listnode *dir;

	dir = search_node_list(ses->list[LIST_PATHDIR], arg);

	if (dir)
	{
		return pdir(dir);
	}
	else
	{
		return 0;
	}
}

int get_exit_length(struct session *ses, struct exit_data *exit)
{
	return (int) exit->weight + ses->map->room_list[exit->vnum]->length;
}

char *get_exit_color(struct session *ses, int room, struct exit_data *exit)
{
	push_call("get_exit_color(%p,%d,%p)",ses,room,exit);

	if (exit)
	{
		if (*exit->color)
		{
			pop_call();
			return exit->color;
		}

		if (room)
		{
			struct exit_data *rev_exit = ses->map->room_list[exit->vnum]->exit_grid[revdir_to_grid(exit->dir)];

			if (rev_exit && rev_exit->vnum == room)
			{
				if (ses->map->room_list[exit->vnum]->length < ses->map->room_list[rev_exit->vnum]->length)
				{
					exit = rev_exit;
				}
			}
		}

		if (HAS_BIT(exit->flags, EXIT_FLAG_AVOID) && *ses->map->color[MAP_COLOR_AVOID])
		{
			pop_call();
			return ses->map->color[MAP_COLOR_AVOID];
		}
		if (HAS_BIT(exit->flags, EXIT_FLAG_BLOCK) && *ses->map->color[MAP_COLOR_BLOCK])
		{
			pop_call();
			return ses->map->color[MAP_COLOR_BLOCK];
		}
		if (HAS_BIT(exit->flags, EXIT_FLAG_HIDE) && *ses->map->color[MAP_COLOR_HIDE])
		{
			pop_call();
			return ses->map->color[MAP_COLOR_HIDE];
		}
		if (HAS_BIT(exit->flags, EXIT_FLAG_INVIS) && *ses->map->color[MAP_COLOR_INVIS])
		{
			pop_call();
			return ses->map->color[MAP_COLOR_INVIS];
		}
		pop_call();
		return ses->map->color[MAP_COLOR_EXIT];
	}
	pop_call();
	return "";
}

int revdir_to_grid(int dir)
{
	switch (dir)
	{
		case 0:
			return EXIT_GRID_0;
		case MAP_EXIT_N:
			return EXIT_GRID_S;
		case MAP_EXIT_E:
			return EXIT_GRID_W;
		case MAP_EXIT_S:
			return EXIT_GRID_N;
		case MAP_EXIT_W:
			return EXIT_GRID_E;
		case MAP_EXIT_N|MAP_EXIT_E:
			return EXIT_GRID_SW;
		case MAP_EXIT_N|MAP_EXIT_W:
			return EXIT_GRID_SE;
		case MAP_EXIT_S|MAP_EXIT_E:
			return EXIT_GRID_NW;
		case MAP_EXIT_S|MAP_EXIT_W:
			return EXIT_GRID_NE;
	}

	if (HAS_BIT(dir, MAP_EXIT_D))
	{
		return EXIT_GRID_U;
	}

	if (HAS_BIT(dir, MAP_EXIT_U))
	{
		return EXIT_GRID_D;
	}

	return EXIT_GRID_0;
}

int dir_to_grid(int dir)
{
	switch (dir)
	{
		case 0:
			return EXIT_GRID_0;
		case MAP_EXIT_N:
			return EXIT_GRID_N;
		case MAP_EXIT_E:
			return EXIT_GRID_E;
		case MAP_EXIT_S:
			return EXIT_GRID_S;
		case MAP_EXIT_W:
			return EXIT_GRID_W;
		case MAP_EXIT_N|MAP_EXIT_E:
			return EXIT_GRID_NE;
		case MAP_EXIT_N|MAP_EXIT_W:
			return EXIT_GRID_NW;
		case MAP_EXIT_S|MAP_EXIT_E:
			return EXIT_GRID_SE;
		case MAP_EXIT_S|MAP_EXIT_W:
			return EXIT_GRID_SW;
	}

	if (HAS_BIT(dir, MAP_EXIT_D))
	{
		return EXIT_GRID_D;
	}

	if (HAS_BIT(dir, MAP_EXIT_U))
	{
		return EXIT_GRID_U;
	}

	return EXIT_GRID_0;
}

int get_room_exits(struct session *ses, int room)
{
	return ses->map->room_list[room]->exit_size;
}

void set_room_exits(struct session *ses, int vnum)
{
	struct exit_data *exit;
	struct room_data *room;

	room = ses->map->room_list[vnum];

	room->exit_dirs = 0;
	room->exit_size = 0;

	memset(room->exit_grid, 0, sizeof(room->exit_grid));

	for (exit = room->f_exit ; exit ; exit = exit->next)
	{
		SET_BIT(room->exit_dirs, 1LL << exit->dir);

		if (room->exit_grid[exit->grid] == NULL)
		{
			room->exit_grid[exit->grid] = exit;
		}
		room->exit_size++;
	}
}

int get_terrain_vnum(struct session *ses, struct room_data *room, int x, int y)
{
	struct listroot *root = ses->list[LIST_TERRAIN];
	struct room_data *wide_grid[11], *vast_grid[11];
	int terrain;

	if (room)
	{
		if (room->terrain_index != -1)
		{
			return room->vnum;
		}
		return -1;
	}

	wide_grid[EXIT_GRID_N]  = ses->map->grid_rooms[x     + map_grid_x * (y + 1)];
	wide_grid[EXIT_GRID_NE] = ses->map->grid_rooms[x + 1 + map_grid_x * (y + 1)];
	wide_grid[EXIT_GRID_E]  = ses->map->grid_rooms[x + 1 + map_grid_x * (y    )];
	wide_grid[EXIT_GRID_SE] = ses->map->grid_rooms[x + 1 + map_grid_x * (y - 1)];
	wide_grid[EXIT_GRID_S]  = ses->map->grid_rooms[x     + map_grid_x * (y - 1)];
	wide_grid[EXIT_GRID_SW] = ses->map->grid_rooms[x - 1 + map_grid_x * (y - 1)];
	wide_grid[EXIT_GRID_W]  = ses->map->grid_rooms[x - 1 + map_grid_x * (y    )];
	wide_grid[EXIT_GRID_NW]	= ses->map->grid_rooms[x - 1 + map_grid_x * (y + 1)];

	if (wide_grid[EXIT_GRID_N] && wide_grid[EXIT_GRID_N]->terrain_index != -1 && wide_grid[EXIT_GRID_N]->vnum && HAS_BIT(root->list[wide_grid[EXIT_GRID_N]->terrain_index]->room->terrain_flags, TERRAIN_FLAG_WIDE))
	{
		terrain = wide_grid[EXIT_GRID_N]->terrain_index;

		if ((wide_grid[EXIT_GRID_E] == NULL || wide_grid[EXIT_GRID_E]->terrain_index == terrain) && (wide_grid[EXIT_GRID_S] == NULL || wide_grid[EXIT_GRID_S]->terrain_index == terrain) && (wide_grid[EXIT_GRID_W] == NULL || wide_grid[EXIT_GRID_W]->terrain_index == terrain))
		{
			return wide_grid[EXIT_GRID_N]->vnum;
		}
	}

	if (wide_grid[EXIT_GRID_E] && wide_grid[EXIT_GRID_E]->terrain_index != -1 && wide_grid[EXIT_GRID_E]->vnum && HAS_BIT(root->list[wide_grid[EXIT_GRID_E]->terrain_index]->room->terrain_flags, TERRAIN_FLAG_WIDE))
	{
		terrain = wide_grid[EXIT_GRID_E]->terrain_index;

		if ((wide_grid[EXIT_GRID_S] == NULL || wide_grid[EXIT_GRID_S]->terrain_index == terrain) && (wide_grid[EXIT_GRID_W] == NULL || wide_grid[EXIT_GRID_W]->terrain_index == terrain) && (wide_grid[EXIT_GRID_N] == NULL || wide_grid[EXIT_GRID_N]->terrain_index == terrain))
		{
			return wide_grid[EXIT_GRID_E]->vnum;
		}
	}

	if (wide_grid[EXIT_GRID_S] && wide_grid[EXIT_GRID_S]->terrain_index != -1 && wide_grid[EXIT_GRID_S]->vnum && HAS_BIT(root->list[wide_grid[EXIT_GRID_S]->terrain_index]->room->terrain_flags, TERRAIN_FLAG_WIDE))
	{
		terrain = wide_grid[EXIT_GRID_S]->terrain_index;

		if ((wide_grid[EXIT_GRID_W] == NULL || wide_grid[EXIT_GRID_W]->terrain_index == terrain) && (wide_grid[EXIT_GRID_N] == NULL || wide_grid[EXIT_GRID_N]->terrain_index == terrain) && (wide_grid[EXIT_GRID_E] == NULL || wide_grid[EXIT_GRID_E]->terrain_index == terrain))
		{
			return wide_grid[EXIT_GRID_S]->vnum;
		}
	}

	if (wide_grid[EXIT_GRID_W] && wide_grid[EXIT_GRID_W]->terrain_index != -1 && wide_grid[EXIT_GRID_W]->vnum && HAS_BIT(root->list[wide_grid[EXIT_GRID_W]->terrain_index]->room->terrain_flags, TERRAIN_FLAG_WIDE))
	{
		terrain = wide_grid[EXIT_GRID_W]->terrain_index;

		if ((wide_grid[EXIT_GRID_N] == NULL || wide_grid[EXIT_GRID_N]->terrain_index == terrain) && (wide_grid[EXIT_GRID_E] == NULL || wide_grid[EXIT_GRID_E]->terrain_index == terrain) && (wide_grid[EXIT_GRID_S] == NULL || wide_grid[EXIT_GRID_S]->terrain_index == terrain))
		{
			return wide_grid[EXIT_GRID_W]->vnum;
		}
	}

	if (x < 2 || y < 2 || x > map_grid_x - 3 || y > map_grid_y - 3)
	{
		return -1;
	}

	vast_grid[EXIT_GRID_N]  = ses->map->grid_rooms[x     + map_grid_x * (y + 2)];
	vast_grid[EXIT_GRID_NE] = ses->map->grid_rooms[x + 2 + map_grid_x * (y + 2)];
	vast_grid[EXIT_GRID_E]  = ses->map->grid_rooms[x + 2 + map_grid_x * (y    )];
	vast_grid[EXIT_GRID_SE] = ses->map->grid_rooms[x + 2 + map_grid_x * (y - 2)];
	vast_grid[EXIT_GRID_S]  = ses->map->grid_rooms[x     + map_grid_x * (y - 2)];
	vast_grid[EXIT_GRID_SW] = ses->map->grid_rooms[x - 2 + map_grid_x * (y - 2)];
	vast_grid[EXIT_GRID_W]  = ses->map->grid_rooms[x - 2 + map_grid_x * (y    )];
	vast_grid[EXIT_GRID_NW]	= ses->map->grid_rooms[x - 2 + map_grid_x * (y + 2)];

	if (vast_grid[EXIT_GRID_N] && vast_grid[EXIT_GRID_N]->terrain_index != -1 && vast_grid[EXIT_GRID_N]->vnum && HAS_BIT(root->list[vast_grid[EXIT_GRID_N]->terrain_index]->room->terrain_flags, TERRAIN_FLAG_VAST))
	{
		terrain = vast_grid[EXIT_GRID_N]->terrain_index;

		if ((wide_grid[EXIT_GRID_E] == NULL || wide_grid[EXIT_GRID_E]->terrain_index == terrain) && (wide_grid[EXIT_GRID_S] == NULL || wide_grid[EXIT_GRID_S]->terrain_index == terrain) && (wide_grid[EXIT_GRID_W] == NULL || wide_grid[EXIT_GRID_W]->terrain_index == terrain))
		{
			return vast_grid[EXIT_GRID_N]->vnum;
		}
	}

	if (vast_grid[EXIT_GRID_E] && vast_grid[EXIT_GRID_E]->terrain_index != -1 && vast_grid[EXIT_GRID_E]->vnum && HAS_BIT(root->list[vast_grid[EXIT_GRID_E]->terrain_index]->room->terrain_flags, TERRAIN_FLAG_VAST))
	{
		terrain = vast_grid[EXIT_GRID_E]->terrain_index;

		if ((wide_grid[EXIT_GRID_S] == NULL || wide_grid[EXIT_GRID_S]->terrain_index == terrain) && (wide_grid[EXIT_GRID_W] == NULL || wide_grid[EXIT_GRID_W]->terrain_index == terrain) && (wide_grid[EXIT_GRID_N] == NULL || wide_grid[EXIT_GRID_N]->terrain_index == terrain))
		{
			return vast_grid[EXIT_GRID_E]->vnum;
		}
	}

	if (vast_grid[EXIT_GRID_S] && vast_grid[EXIT_GRID_S]->terrain_index != -1 && vast_grid[EXIT_GRID_S]->vnum && HAS_BIT(root->list[vast_grid[EXIT_GRID_S]->terrain_index]->room->terrain_flags, TERRAIN_FLAG_VAST))
	{
		terrain = vast_grid[EXIT_GRID_S]->terrain_index;

		if ((wide_grid[EXIT_GRID_W] == NULL || wide_grid[EXIT_GRID_W]->terrain_index == terrain) && (wide_grid[EXIT_GRID_N] == NULL || wide_grid[EXIT_GRID_N]->terrain_index == terrain) && (wide_grid[EXIT_GRID_E] == NULL || wide_grid[EXIT_GRID_E]->terrain_index == terrain))
		{
			return vast_grid[EXIT_GRID_S]->vnum;
		}
	}

	if (vast_grid[EXIT_GRID_W] && vast_grid[EXIT_GRID_W]->terrain_index != -1 && vast_grid[EXIT_GRID_W]->vnum && HAS_BIT(root->list[vast_grid[EXIT_GRID_W]->terrain_index]->room->terrain_flags, TERRAIN_FLAG_VAST))
	{
		terrain = vast_grid[EXIT_GRID_W]->terrain_index;

		if ((wide_grid[EXIT_GRID_N] == NULL || wide_grid[EXIT_GRID_N]->terrain_index == terrain) && (wide_grid[EXIT_GRID_E] == NULL || wide_grid[EXIT_GRID_E]->terrain_index == terrain) && (wide_grid[EXIT_GRID_S] == NULL || wide_grid[EXIT_GRID_S]->terrain_index == terrain))
		{
			return vast_grid[EXIT_GRID_W]->vnum;
		}
	}

	return -1;
}

int get_terrain_density(struct room_data *room, int width)
{
	int flag = 0;

	switch (width)
	{
		case 0:
			if (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_FADEIN))
			{
				switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_DENSE|TERRAIN_FLAG_AMPLE|TERRAIN_FLAG_SPARSE|TERRAIN_FLAG_SCANT))
				{
					case TERRAIN_FLAG_DENSE:
						switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_NARROW|TERRAIN_FLAG_STANDARD|TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST))
						{
							case TERRAIN_FLAG_NARROW:
								flag = TERRAIN_FLAG_DENSE;
								break;
							case TERRAIN_FLAG_STANDARD:
								flag = TERRAIN_FLAG_AMPLE;
								break;
							case TERRAIN_FLAG_WIDE:
								flag = TERRAIN_FLAG_SPARSE;
								break;
							case TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST:
								flag = TERRAIN_FLAG_SCANT;
								break;
						}
						break;
					case TERRAIN_FLAG_AMPLE:
						switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_NARROW|TERRAIN_FLAG_STANDARD|TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST))
						{
							case TERRAIN_FLAG_NARROW:
								flag = TERRAIN_FLAG_AMPLE;
								break;
							case TERRAIN_FLAG_STANDARD:
								flag = TERRAIN_FLAG_SPARSE;
								break;
							case TERRAIN_FLAG_WIDE:
								flag = TERRAIN_FLAG_SCANT;
								break;
							case TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST:
								flag = TERRAIN_FLAG_SCANT;
								break;
						}
						break;

					case TERRAIN_FLAG_SPARSE:
						switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_NARROW|TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST))
						{
							case TERRAIN_FLAG_NARROW:
								flag = TERRAIN_FLAG_SPARSE;
								break;
							case TERRAIN_FLAG_STANDARD:
							case TERRAIN_FLAG_WIDE:
							case TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST:
								flag = TERRAIN_FLAG_SCANT;
								break;
						}
						break;

					case TERRAIN_FLAG_SCANT:
						flag = TERRAIN_FLAG_SCANT;
						break;
				}
			}
			else if (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_FADEOUT))
			{
				switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_DENSE|TERRAIN_FLAG_AMPLE|TERRAIN_FLAG_SPARSE|TERRAIN_FLAG_SCANT))
				{
					case TERRAIN_FLAG_DENSE:
						flag = TERRAIN_FLAG_DENSE;
						break;
					case TERRAIN_FLAG_AMPLE:
						flag = TERRAIN_FLAG_AMPLE;
						break;
					case TERRAIN_FLAG_SPARSE:
						flag = TERRAIN_FLAG_SPARSE;
						break;
					case TERRAIN_FLAG_SCANT:
						flag = TERRAIN_FLAG_SCANT;
						break;
				}
			}
			else
			{
				flag = room->terrain_flags;
			}
			break;

		case 1:
			if (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_FADEIN))
			{
				switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_DENSE|TERRAIN_FLAG_SPARSE|TERRAIN_FLAG_SCANT))
				{
					case TERRAIN_FLAG_DENSE:
						switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_NARROW|TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST))
						{
							case TERRAIN_FLAG_NARROW:
							case TERRAIN_FLAG_STANDARD:
								flag = TERRAIN_FLAG_DENSE;
								break;

							case TERRAIN_FLAG_WIDE:
								flag = TERRAIN_FLAG_AMPLE;
								break;
								
							case TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST:
								flag = TERRAIN_FLAG_SPARSE;
								break;
						}
						break;

					case TERRAIN_FLAG_AMPLE:
						switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_NARROW|TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST))
						{
							case TERRAIN_FLAG_NARROW:
							case TERRAIN_FLAG_STANDARD:
								flag = TERRAIN_FLAG_AMPLE;
								break;
							case TERRAIN_FLAG_WIDE:
								flag = TERRAIN_FLAG_SPARSE;
								break;
							case TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST:
								flag = TERRAIN_FLAG_SCANT;
								break;
						}
						break;

					case TERRAIN_FLAG_SPARSE:
						switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_NARROW|TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST))
						{
							case TERRAIN_FLAG_NARROW:
							case TERRAIN_FLAG_STANDARD:
							case TERRAIN_FLAG_WIDE:
								flag = TERRAIN_FLAG_SPARSE;
								break;
							case TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST:
								flag = TERRAIN_FLAG_SCANT;
								break;
						}
						break;

					case TERRAIN_FLAG_SCANT:
						flag = TERRAIN_FLAG_SCANT;
						break;
				}
			}
			else if (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_FADEOUT))
			{
				switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_DENSE|TERRAIN_FLAG_SPARSE|TERRAIN_FLAG_SCANT))
				{
					case TERRAIN_FLAG_DENSE:
						flag = TERRAIN_FLAG_AMPLE;
						break;

					case TERRAIN_FLAG_AMPLE:
						flag = TERRAIN_FLAG_SPARSE;
						break;

					case TERRAIN_FLAG_SPARSE:
					case TERRAIN_FLAG_SCANT:
						flag = TERRAIN_FLAG_SCANT;
						break;
				}
			}
			else
			{
				flag = room->terrain_flags;
			}
			break;

		case 2:
			if (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_FADEIN))
			{
				switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_DENSE|TERRAIN_FLAG_SPARSE|TERRAIN_FLAG_SCANT))
				{
					case TERRAIN_FLAG_DENSE:
						switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_NARROW|TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST))
						{
							case TERRAIN_FLAG_NARROW:
							case TERRAIN_FLAG_STANDARD:
							case TERRAIN_FLAG_WIDE:
								flag = TERRAIN_FLAG_DENSE;
								break;

							case TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST:
								flag = TERRAIN_FLAG_AMPLE;
								break;
						}
						break;

					case TERRAIN_FLAG_AMPLE:
						switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_NARROW|TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST))
						{
							case TERRAIN_FLAG_NARROW:
							case TERRAIN_FLAG_STANDARD:
							case TERRAIN_FLAG_WIDE:
								flag = TERRAIN_FLAG_AMPLE;
								break;

							case TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST:
								flag = TERRAIN_FLAG_SPARSE;
								break;
						}
						break;

					case TERRAIN_FLAG_SPARSE:
						switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_NARROW|TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST))
						{
							case TERRAIN_FLAG_NARROW:
							case TERRAIN_FLAG_STANDARD:
							case TERRAIN_FLAG_WIDE:
								flag = TERRAIN_FLAG_SPARSE;
								break;
							case TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST:
								flag = TERRAIN_FLAG_SCANT;
								break;
						}
						break;

					case TERRAIN_FLAG_SCANT:
						flag = TERRAIN_FLAG_SCANT;
						break;
				}
			}
			else if (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_FADEOUT))
			{
				switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_DENSE|TERRAIN_FLAG_SPARSE|TERRAIN_FLAG_SCANT))
				{
					case TERRAIN_FLAG_DENSE:
						flag = TERRAIN_FLAG_SPARSE;
						break;

					default:
						flag = TERRAIN_FLAG_SCANT;
						break;
				}
			}
			else
			{
				flag = room->terrain_flags;
			}
			break;

		default:
			if (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_FADEIN))
			{
				switch (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_DENSE|TERRAIN_FLAG_SPARSE|TERRAIN_FLAG_SCANT))
				{
					case TERRAIN_FLAG_DENSE:
						flag = TERRAIN_FLAG_DENSE;
						break;
					default:
						flag = 0;
						break;

					case TERRAIN_FLAG_SPARSE:
						flag = TERRAIN_FLAG_SPARSE;
						break;

					case TERRAIN_FLAG_SCANT:
						flag = TERRAIN_FLAG_SCANT;
						break;
				}
			}
			else if (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_FADEOUT))
			{
				flag = TERRAIN_FLAG_SCANT;
			}
			else
			{
				flag = room->terrain_flags;
			}
			break;
	}

	if (HAS_BIT(room->terrain_flags, TERRAIN_FLAG_DOUBLE))
	{
		SET_BIT(flag, TERRAIN_FLAG_DOUBLE);
	}
	return flag;
}

char *blank_terrain_symbol(struct session *ses, struct room_data *room, int index, int flags)
{
	if (HAS_BIT(flags, TERRAIN_FLAG_DOUBLE))
	{
		if (room && room->terrain_index != -1 && HAS_BIT(ses->list[LIST_TERRAIN]->list[room->terrain_index]->room->terrain_flags, TERRAIN_FLAG_DOUBLE))
		{
			if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
			{
				if (index % 2 == 1)
				{
					SET_BIT(ses->map->flags, MAP_FLAG_DOUBLED);

					return "  ";
				}
				else
				{
					DEL_BIT(ses->map->flags, MAP_FLAG_DOUBLED);
					return "";
				}
			}
			else if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS))
			{
				switch (index)
				{
					case 1:
					case 3:
						SET_BIT(ses->map->flags, MAP_FLAG_DOUBLED);
						return "  ";
					case 2:
					case 4:
						DEL_BIT(ses->map->flags, MAP_FLAG_DOUBLED);
						return "";
					case 5:
						return "\e[1;31m5";
				}
			}
		}
//		DEL_BIT(ses->map->flags, MAP_FLAG_DOUBLED);
		return " ";
	}
	return " ";
}

char *draw_terrain_symbol(struct session *ses, struct room_data *room, int line, int index, int x, int y, int flags)
{
	struct room_data *room_grid[11], *terrain_room;
	int vnum_grid[11];
	int terrain, width = 0, density, hash;

	if (!HAS_BIT(ses->map->flags, MAP_FLAG_TERRAIN))
	{
		return " ";
	}

	if (HAS_BIT(ses->map->flags, MAP_FLAG_DOUBLED))
	{
		DEL_BIT(ses->map->flags, MAP_FLAG_DOUBLED);

		if (HAS_BIT(flags, TERRAIN_FLAG_DOUBLE) && index != 1)
		{
			return "";
		}
//		return "\e[1;31m?";
	}

	room_grid[EXIT_GRID_0]  = ses->map->grid_rooms[x     + map_grid_x * (y    )];
	room_grid[EXIT_GRID_N]  = ses->map->grid_rooms[x     + map_grid_x * (y + 1)];
	room_grid[EXIT_GRID_NE] = ses->map->grid_rooms[x + 1 + map_grid_x * (y + 1)];
	room_grid[EXIT_GRID_E]  = ses->map->grid_rooms[x + 1 + map_grid_x * (y    )];
	room_grid[EXIT_GRID_SE] = ses->map->grid_rooms[x + 1 + map_grid_x * (y - 1)];
	room_grid[EXIT_GRID_S]  = ses->map->grid_rooms[x     + map_grid_x * (y - 1)];
	room_grid[EXIT_GRID_SW] = ses->map->grid_rooms[x - 1 + map_grid_x * (y - 1)];
	room_grid[EXIT_GRID_W]  = ses->map->grid_rooms[x - 1 + map_grid_x * (y    )];
	room_grid[EXIT_GRID_NW]	= ses->map->grid_rooms[x - 1 + map_grid_x * (y + 1)];

	vnum_grid[EXIT_GRID_0]  = ses->map->grid_vnums[x     + map_grid_x * (y    )];
	vnum_grid[EXIT_GRID_N]  = ses->map->grid_vnums[x     + map_grid_x * (y + 1)];
	vnum_grid[EXIT_GRID_NE] = ses->map->grid_vnums[x + 1 + map_grid_x * (y + 1)];
	vnum_grid[EXIT_GRID_E]  = ses->map->grid_vnums[x + 1 + map_grid_x * (y    )];
	vnum_grid[EXIT_GRID_SE] = ses->map->grid_vnums[x + 1 + map_grid_x * (y - 1)];
	vnum_grid[EXIT_GRID_S]  = ses->map->grid_vnums[x     + map_grid_x * (y - 1)];
	vnum_grid[EXIT_GRID_SW] = ses->map->grid_vnums[x - 1 + map_grid_x * (y - 1)];
	vnum_grid[EXIT_GRID_W]  = ses->map->grid_vnums[x - 1 + map_grid_x * (y    )];
	vnum_grid[EXIT_GRID_NW]	= ses->map->grid_vnums[x - 1 + map_grid_x * (y + 1)];

	hash = index + vnum_grid[EXIT_GRID_0];

	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		if (room == NULL)
		{
			width++;

			switch (line)
			{
				case 1:
					switch (index)
					{
						case 1:
						case 2:
							room = room_grid[EXIT_GRID_N] ? room_grid[EXIT_GRID_N] : room_grid[EXIT_GRID_W] ? room_grid[EXIT_GRID_W] : room_grid[EXIT_GRID_NW] ? room_grid[EXIT_GRID_NW] : NULL;
							break;
						case 3:
						case 4:
							room = room_grid[EXIT_GRID_N] ? room_grid[EXIT_GRID_N] : NULL;
							break;
						case 5:
						case 6:
							room = room_grid[EXIT_GRID_N] ? room_grid[EXIT_GRID_N] : room_grid[EXIT_GRID_E] ? room_grid[EXIT_GRID_E] : room_grid[EXIT_GRID_NE] ? room_grid[EXIT_GRID_NE] : NULL;
							break;
					}
					break;

				case 2:
					switch (index)
					{
						case 1:
						case 2:
							room = room_grid[EXIT_GRID_W] ? room_grid[EXIT_GRID_W] : room_grid[EXIT_GRID_NW] ? room_grid[EXIT_GRID_NW] : room_grid[EXIT_GRID_SW] ? room_grid[EXIT_GRID_SW] : NULL;
							break;
						case 3:
						case 4:
							room = room_grid[EXIT_GRID_N] ? room_grid[EXIT_GRID_N] : room_grid[EXIT_GRID_S] ? room_grid[EXIT_GRID_S] : NULL;
							break;
						case 5:
						case 6:
							room = room_grid[EXIT_GRID_E] ? room_grid[EXIT_GRID_E] : room_grid[EXIT_GRID_NE] ? room_grid[EXIT_GRID_NE] : room_grid[EXIT_GRID_SE] ? room_grid[EXIT_GRID_SE] : NULL;
							break;
					}
					break;

				case 3:
					switch (index)
					{
						case 1:
						case 2:
							room = room_grid[EXIT_GRID_S] ? room_grid[EXIT_GRID_S] : room_grid[EXIT_GRID_W] ? room_grid[EXIT_GRID_W] : room_grid[EXIT_GRID_SW] ? room_grid[EXIT_GRID_SW] : NULL;
							break;
						case 3:
						case 4:
							room = room_grid[EXIT_GRID_S] ? room_grid[EXIT_GRID_S] : NULL;
							break;
						case 5:
						case 6:
							room = room_grid[EXIT_GRID_S] ? room_grid[EXIT_GRID_S] : room_grid[EXIT_GRID_E] ? room_grid[EXIT_GRID_E] : room_grid[EXIT_GRID_SE] ? room_grid[EXIT_GRID_SE] : NULL;
							break;
					}
					break;
			}

			if (room == NULL || room->terrain_index == -1 || HAS_BIT(ses->list[LIST_TERRAIN]->list[room->terrain_index]->room->terrain_flags, TERRAIN_FLAG_NARROW))
			{
				return blank_terrain_symbol(ses, room, index, flags);
			}

			hash += vnum_grid[EXIT_GRID_N] ? vnum_grid[EXIT_GRID_N] :
				vnum_grid[EXIT_GRID_E] ? vnum_grid[EXIT_GRID_E] :
				vnum_grid[EXIT_GRID_S] ? vnum_grid[EXIT_GRID_S] :
				vnum_grid[EXIT_GRID_W] ? vnum_grid[EXIT_GRID_W] :
				vnum_grid[EXIT_GRID_NE] ? vnum_grid[EXIT_GRID_NE] :
				vnum_grid[EXIT_GRID_SE] ? vnum_grid[EXIT_GRID_SE] :
				vnum_grid[EXIT_GRID_SW] ? vnum_grid[EXIT_GRID_SW] :
				vnum_grid[EXIT_GRID_NW] ? vnum_grid[EXIT_GRID_NW] : 0;
		}
		else
		{
			hash += vnum_grid[EXIT_GRID_N] ? 1 :
				vnum_grid[EXIT_GRID_E] ? 2 :
				vnum_grid[EXIT_GRID_S] ? 3 :
				vnum_grid[EXIT_GRID_W] ? 4 :
				vnum_grid[EXIT_GRID_NE] ? 5 :
				vnum_grid[EXIT_GRID_SE] ? 6 :
				vnum_grid[EXIT_GRID_SW] ? 7 :
				vnum_grid[EXIT_GRID_NW] ? 8 : 9;
		}

		terrain = room->terrain_index;

		if (terrain == -1)
		{
			return blank_terrain_symbol(ses, room, index, flags);
		}

		terrain_room = ses->list[LIST_TERRAIN]->list[terrain]->room;

		if (HAS_BIT(ses->list[LIST_TERRAIN]->list[room->terrain_index]->room->terrain_flags, TERRAIN_FLAG_DOUBLE))
		{
			if (HAS_BIT(flags, TERRAIN_FLAG_DOUBLE))
			{
				if (index % 2 == 0)
				{
					return "\e[1;36m?";
				}
				SET_BIT(ses->map->flags, MAP_FLAG_DOUBLED);
			}
			else
			{
				return " ";
			}
		}

		if (HAS_BIT(terrain_room->terrain_flags, TERRAIN_FLAG_NARROW) && room->exit_grid[EXIT_GRID_E] == NULL)
		{
			switch (line * 10 + index)
			{
				case 16:
				case 26:
				case 36:
					return blank_terrain_symbol(ses, room, index, flags);
			}
		}

		if (room->vnum == 0)
		{
			width++;

			density = 0;

			density += (room_grid[EXIT_GRID_N] && room_grid[EXIT_GRID_N]->vnum);
			density += (room_grid[EXIT_GRID_NE] && room_grid[EXIT_GRID_NE]->vnum);
			density += (room_grid[EXIT_GRID_E] && room_grid[EXIT_GRID_E]->vnum);
			density += (room_grid[EXIT_GRID_SE] && room_grid[EXIT_GRID_SE]->vnum);
			density += (room_grid[EXIT_GRID_S] && room_grid[EXIT_GRID_S]->vnum);
			density += (room_grid[EXIT_GRID_SW] && room_grid[EXIT_GRID_SW]->vnum);
			density += (room_grid[EXIT_GRID_W] && room_grid[EXIT_GRID_W]->vnum);
			density += (room_grid[EXIT_GRID_NW] && room_grid[EXIT_GRID_NW]->vnum);

			if (density == 0)
			{
				width++;
			}
		}

		density = get_terrain_density(terrain_room, width);

		if (HAS_BIT(density, TERRAIN_FLAG_DENSE))
		{
			return ses->list[LIST_TERRAIN]->list[terrain]->arg2;
		}

		if (HAS_BIT(density, TERRAIN_FLAG_SPARSE))
		{
			switch (line * 10 + index)
			{
				case 11:
				case 23:
				case 35:
					return ses->list[LIST_TERRAIN]->list[terrain]->arg2;
			}
			return blank_terrain_symbol(ses, room, index, flags);
		}

		if (HAS_BIT(density, TERRAIN_FLAG_SCANT))
		{
			switch (line * 10 + index)
			{
				case 11:
					return hash % 3 == 0 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				case 23:
					return hash % 3 == 1 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				case 35:
					return hash % 3 == 2 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
			}
			return blank_terrain_symbol(ses, room, index, flags);
		}

		if (HAS_BIT(density, TERRAIN_FLAG_DOUBLE))
		{
			switch (y % 2 * 30 + line * 10 + index)
			{
				case 11:
				case 15:
				case 23:
				case 31:
				case 35:
				case 43:
				case 51:
				case 55:
				case 63:
					return ses->list[LIST_TERRAIN]->list[terrain]->arg2;
			}
			return blank_terrain_symbol(ses, room, index, flags);
		}

		switch (y % 2 * 30 + line * 10 + index)
		{
			case 11:
			case 13:
			case 15:
			case 22:
			case 24:
			case 26:
			case 31:
			case 33:
			case 35:
			case 42:
			case 44:
			case 46:
			case 51:
			case 53:
			case 55:
			case 62:
			case 64:
			case 66:
				return ses->list[LIST_TERRAIN]->list[terrain]->arg2;
		}
		return blank_terrain_symbol(ses, room, index, flags);
	}

	// UNICODE

	if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS))
	{
		if (room == NULL)
		{
			width++;

			switch (line)
			{
				case 1:
					switch (index)
					{
						case 1:
						case 2:
							room = room_grid[EXIT_GRID_W] ? room_grid[EXIT_GRID_W] : room_grid[EXIT_GRID_NW] ? room_grid[EXIT_GRID_NW] : NULL;
							break;
						case 3:
						case 4:
							room = room_grid[EXIT_GRID_N] ? room_grid[EXIT_GRID_N] : NULL;
							break;
						case 5:
							room = room_grid[EXIT_GRID_N] ? room_grid[EXIT_GRID_N] : NULL;
							break;
					}
					break;

				case 2:
					switch (index)
					{
						case 1:
						case 2:
							room = room_grid[EXIT_GRID_W] ? room_grid[EXIT_GRID_W] : NULL;
							break;
						case 3:
						case 4:
							if (room_grid[EXIT_GRID_S])
							{
								room = room_grid[EXIT_GRID_S];
							}
							else if (room_grid[EXIT_GRID_N])
							{
								room = room_grid[EXIT_GRID_N];
							}
							break;
						case 5:
							room = room_grid[EXIT_GRID_S] ? room_grid[EXIT_GRID_S] : NULL;
							break;
					}
					break;
			}

			if (room == NULL || room->terrain_index == -1)
			{
				return blank_terrain_symbol(ses, room, index, flags);
			}

			if (HAS_BIT(ses->list[LIST_TERRAIN]->list[room->terrain_index]->room->terrain_flags, TERRAIN_FLAG_NARROW))
			{
				switch (line * 10 + index)
				{
					case 23:
					case 24:
					case 25:
						return blank_terrain_symbol(ses, room, index, flags);
				}

				if (room_grid[EXIT_GRID_N] == NULL)
				{
					switch (line * 10 + index)
					{
						case 13:
						case 14:
						case 15:
							return blank_terrain_symbol(ses, room, index, flags);
					}
				}

				if (room_grid[EXIT_GRID_W] == NULL && room_grid[EXIT_GRID_NW] == NULL)
				{
					switch (line * 10 + index)
					{
						case 11:
						case 12:
							return blank_terrain_symbol(ses, room, index, flags);
					}
				}

				if (room_grid[EXIT_GRID_W] == NULL)
				{
					switch (line * 10 + index)
					{
						case 21:
						case 22:
							return blank_terrain_symbol(ses, room, index, flags);
					}
				}
			}

			hash += vnum_grid[EXIT_GRID_N] ? vnum_grid[EXIT_GRID_N] :
				vnum_grid[EXIT_GRID_E] ? vnum_grid[EXIT_GRID_E] :
				vnum_grid[EXIT_GRID_S] ? vnum_grid[EXIT_GRID_S] :
				vnum_grid[EXIT_GRID_W] ? vnum_grid[EXIT_GRID_W] :
				vnum_grid[EXIT_GRID_NE] ? vnum_grid[EXIT_GRID_NE] :
				vnum_grid[EXIT_GRID_SE] ? vnum_grid[EXIT_GRID_SE] :
				vnum_grid[EXIT_GRID_SW] ? vnum_grid[EXIT_GRID_SW] :
				vnum_grid[EXIT_GRID_NW] ? vnum_grid[EXIT_GRID_NW] : 0;
		}
		else
		{
			switch (line * 10 + index)
			{
				case 11:
				case 12:
					if (room_grid[EXIT_GRID_N] && hash % 4 == 0)
					{
						room = room_grid[EXIT_GRID_N];
					}
					else if (room_grid[EXIT_GRID_W] && hash % 4 == 1)
					{
						room = room_grid[EXIT_GRID_W];
					}
					else if (room_grid[EXIT_GRID_NW] && hash % 4 == 2)
					{
						room = room_grid[EXIT_GRID_NW];
					}
					break;
				case 13:
				case 14:
					if (room_grid[EXIT_GRID_N] && hash % 2 == 0)
					{
						room = room_grid[EXIT_GRID_N];
					}
					break;
				case 21:
				case 22:
					if (room_grid[EXIT_GRID_W] && hash % 2 == 0)
					{
						room = room_grid[EXIT_GRID_W];
					}
					break;
			}

			if (room == NULL)
			{
				return blank_terrain_symbol(ses, room, index, flags);
			}

			hash += vnum_grid[EXIT_GRID_N] ? 1 :
				vnum_grid[EXIT_GRID_E] ? 2 :
				vnum_grid[EXIT_GRID_S] ? 3 :
				vnum_grid[EXIT_GRID_W] ? 4 :
				vnum_grid[EXIT_GRID_NE] ? 5 :
				vnum_grid[EXIT_GRID_SE] ? 6 :
				vnum_grid[EXIT_GRID_SW] ? 7 :
				vnum_grid[EXIT_GRID_NW] ? 8 : 9;
		}

		terrain = room->terrain_index;

		if (terrain == -1)
		{
			return blank_terrain_symbol(ses, room, index, flags);
		}

		terrain_room = ses->list[LIST_TERRAIN]->list[terrain]->room;

		if (HAS_BIT(terrain_room->terrain_flags, TERRAIN_FLAG_DOUBLE))
		{
			if (HAS_BIT(flags, TERRAIN_FLAG_DOUBLE))
			{
				if (index == 5)
				{
					return "\e[1;32m5";
				}
				if (index % 2 == 0)
				{
					DEL_BIT(ses->map->flags, MAP_FLAG_DOUBLED);
					if (index == 2)
					{
						return " ";
					}
					if (index == 4)
					{
						return " ";
					}
					return "\e[1;35m?";
				}
				SET_BIT(ses->map->flags, MAP_FLAG_DOUBLED);
			}
			else
			{
				return blank_terrain_symbol(ses, room, index, flags);
			}
		}

		if (room->vnum == 0)
		{
			width++;

			density = 0;

			density += (room_grid[EXIT_GRID_N] && room_grid[EXIT_GRID_N]->vnum);
			density += (room_grid[EXIT_GRID_NE] && room_grid[EXIT_GRID_NE]->vnum);
			density += (room_grid[EXIT_GRID_E] && room_grid[EXIT_GRID_E]->vnum);
			density += (room_grid[EXIT_GRID_SE] && room_grid[EXIT_GRID_SE]->vnum);
			density += (room_grid[EXIT_GRID_S] && room_grid[EXIT_GRID_S]->vnum);
			density += (room_grid[EXIT_GRID_SW] && room_grid[EXIT_GRID_SW]->vnum);
			density += (room_grid[EXIT_GRID_W] && room_grid[EXIT_GRID_W]->vnum);
			density += (room_grid[EXIT_GRID_NW] && room_grid[EXIT_GRID_NW]->vnum);

			if (density == 0)
			{
				width++;
			}
		}

		density = get_terrain_density(terrain_room, width);

		if (HAS_BIT(density, TERRAIN_FLAG_DENSE))
		{
			return ses->list[LIST_TERRAIN]->list[terrain]->arg2;
		}

		if (HAS_BIT(density, TERRAIN_FLAG_SPARSE))
		{
			if (HAS_BIT(flags, TERRAIN_FLAG_DOUBLE) && HAS_BIT(terrain_room->terrain_flags, TERRAIN_FLAG_DOUBLE))
			{
				if (room_grid[EXIT_GRID_0] && room_grid[EXIT_GRID_N] && room_grid[EXIT_GRID_0]->terrain_index != room_grid[EXIT_GRID_N]->terrain_index)
				{
					switch (line * 10 + index)
					{
						case 13:
							return hash % 2 == 0 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
					}
				}

				switch (line * 10 + index)
				{
					case 11:
						return hash % 2 == 1 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);

					case 23:
						return hash % 2 == 0 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				}
				return blank_terrain_symbol(ses, room, index, flags);
			}

			if (room_grid[EXIT_GRID_0] && room_grid[EXIT_GRID_N] && room_grid[EXIT_GRID_0]->terrain_index != room_grid[EXIT_GRID_N]->terrain_index)
			{
				switch (line * 10 + index)
				{
					case 12:
						return hash % 2 == 0 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);

					case 14:
						return hash % 2 == 1 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				}
			}

			switch (line * 10 + index)
			{
				case 11:
					return hash % 5 == 0 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				case 13:
					return hash % 5 == 2 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				case 15:
					return hash % 5 == 4 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);

				case 22:
					return hash % 2 == 0 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				case 24:
					return hash % 2 == 1 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
			}
			return blank_terrain_symbol(ses, room, index, flags);
		}

		if (HAS_BIT(density, TERRAIN_FLAG_SCANT))
		{
			if (HAS_BIT(flags, TERRAIN_FLAG_DOUBLE) && HAS_BIT(terrain_room->terrain_flags, TERRAIN_FLAG_DOUBLE))
			{
				if (room_grid[EXIT_GRID_0] && room_grid[EXIT_GRID_N] && room_grid[EXIT_GRID_0]->terrain_index != room_grid[EXIT_GRID_N]->terrain_index)
				{
					switch (line * 10 + index)
					{
						case 13:
							return hash % 4 == 2 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
					}
				}

				switch (line * 10 + index)
				{
					case 11:
						return hash % 4 == 0 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
//					case 13:
//						return hash % 8 == 1 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
//					case 21:
//						return hash % 8 == 2 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
					case 23:
						return hash % 4 == 1 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				}
				return blank_terrain_symbol(ses, room, index, flags);
			}

			if (room_grid[EXIT_GRID_0] && room_grid[EXIT_GRID_N] && room_grid[EXIT_GRID_0]->terrain_index != room_grid[EXIT_GRID_N]->terrain_index)
			{
				switch (line * 10 + index)
				{
					case 12:
						return hash % 7 == 5 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
					case 14:
						return hash % 7 == 6 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				}
			}

			switch (line * 10 + index)
			{
				case 11:
					return hash % 7 == 0 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				case 13:
					return hash % 7 == 1 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				case 15:
					return hash % 7 == 2 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				case 22:
					return hash % 7 == 3 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
				case 24:
					return hash % 7 == 4 ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);
			}
			return blank_terrain_symbol(ses, room, index, flags);
		}

		if (HAS_BIT(flags, TERRAIN_FLAG_DOUBLE) && HAS_BIT(terrain_room->terrain_flags, TERRAIN_FLAG_DOUBLE))
		{
			switch (line * 10 + index)
			{
				case 13:
					return room_grid[EXIT_GRID_0] && room_grid[EXIT_GRID_N] && room_grid[EXIT_GRID_0]->terrain_index != room_grid[EXIT_GRID_N]->terrain_index ? ses->list[LIST_TERRAIN]->list[terrain]->arg2 : blank_terrain_symbol(ses, room, index, flags);

				case 11:
//					return "\e[1;31m1 ";
				case 23:
//					return "\e[1;34m3 ";
					return ses->list[LIST_TERRAIN]->list[terrain]->arg2;
			}
			return blank_terrain_symbol(ses, room, index, flags);
		}

		switch (line * 10 + index)
		{
			case 12:
			case 14:
				if (room_grid[EXIT_GRID_0] && room_grid[EXIT_GRID_N] && room_grid[EXIT_GRID_0]->terrain_index != -1 && room_grid[EXIT_GRID_N]->terrain_index != -1 && room_grid[EXIT_GRID_0]->terrain_index != room_grid[EXIT_GRID_N]->terrain_index)
				{
					if (!HAS_BIT(ses->list[LIST_TERRAIN]->list[room_grid[EXIT_GRID_N]->terrain_index]->room->terrain_flags, TERRAIN_FLAG_DOUBLE))
					{
						return ses->list[LIST_TERRAIN]->list[room_grid[EXIT_GRID_N]->terrain_index]->arg2;
					}
				}
				return blank_terrain_symbol(ses, room, index, flags);
			case 11:
			case 13:
			case 15:
			case 22:
			case 24:
				return ses->list[LIST_TERRAIN]->list[terrain]->arg2;
		}
		return blank_terrain_symbol(ses, room, index, flags);
	}
	return blank_terrain_symbol(ses, room, index, flags);
}

int follow_map(struct session *ses, char *argument)
{
	struct room_data *room;
	struct exit_data *exit;;
	int in_room, vnum;

	push_call("follow_map(%p,%p)",ses,argument);

	room = ses->map->room_list[ses->map->in_room];

	if (HAS_BIT(ses->map->flags, MAP_FLAG_NOFOLLOW))
	{
		if (check_global(ses, room->vnum) && find_exit(ses, ses->map->global_vnum, argument))
		{
			in_room = ses->map->global_vnum;
		}
		else
		{
			in_room = ses->map->in_room;
		}
		exit = find_exit(ses, in_room, argument);

		if (exit)
		{
			ses->map->dir = exit->dir;

			vnum = tunnel_void(ses, in_room, exit->vnum, exit->dir);

			check_all_events(ses, EVENT_FLAG_MAP, 0, 4, "MAP FOLLOW MAP", ntos(in_room), ntos(vnum), exit->name, ntos(ses->map->nofollow));
		}
		pop_call();
		return 0;
	}

	if (check_global(ses, room->vnum))
	{
		if (find_exit(ses, ses->map->global_vnum, argument))
		{
			goto_room(ses, ses->map->global_vnum);
		}
	}

	exit = find_exit(ses, ses->map->in_room, argument);

	if (exit)
	{
		ses->map->dir = exit->dir;

		in_room = ses->map->in_room;

		vnum = tunnel_void(ses, in_room, exit->vnum, exit->dir);

		if (ses->map->nofollow == 0)
		{
			if (HAS_BIT(exit->flags, EXIT_FLAG_BLOCK) || HAS_BIT(ses->map->room_list[vnum]->flags, ROOM_FLAG_BLOCK))
			{
				show_error(ses, LIST_COMMAND, "#MAP FOLLOW: %s {%d} HAS THE BLOCK FLAG SET.", HAS_BIT(exit->flags, EXIT_FLAG_BLOCK) ? "EXIT" : "ROOM", vnum);

				pop_call();
				return 1;
			}
			else
			{
				ses->map->nofollow++;
				script_driver(ses, LIST_COMMAND, exit->cmd);
				ses->map->nofollow--;
			}
		}

		check_all_events(ses, EVENT_FLAG_MAP, 0, 4, "MAP FOLLOW MAP", ntos(in_room), ntos(vnum), exit->name, ntos(ses->map->nofollow));

		add_undo(ses, "%d %d %d", vnum, in_room, MAP_UNDO_MOVE);

		goto_room(ses, vnum);

		if (HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_LEAVE))
		{
			show_message(ses, LIST_COMMAND, "#MAP: LEAVE FLAG FOUND IN ROOM {%d}. LEAVING MAP.", ses->map->in_room);

			map_leave(ses, "", "", "");
		}

		if (HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP))
		{
			SET_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);
		}

		pop_call();
		return 1;
	}

	if (!HAS_BIT(ses->map->flags, MAP_FLAG_STATIC) && !HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_STATIC))
	{
		struct listnode *dir;

		if ((dir = search_node_list(ses->list[LIST_PATHDIR], argument)) == NULL)
		{
			pop_call();
			return 0;
		}

		in_room = find_coord(ses, argument);

		if (in_room)
		{
			show_message(ses, LIST_PATH, "#MAP CREATE LINK %5d {%s}.", in_room, ses->map->room_list[in_room]->name);

			add_undo(ses, "%d %d %d", in_room, ses->map->in_room, MAP_UNDO_MOVE|MAP_UNDO_LINK);
		}
		else
		{
			for (in_room = 1 ; in_room < ses->map->size ; in_room++)
			{
				if (ses->map->room_list[in_room] == NULL)
				{
					break;
				}
			}

			if (in_room == ses->map->size)
			{
				show_error(ses, LIST_COMMAND, "#MAP: Maximum amount of rooms of %d reached. Use #MAP RESIZE to increase the maximum.", ses->map->size);

				pop_call();
				return 1;
			}
			add_undo(ses, "%d %d %d", in_room, ses->map->in_room, MAP_UNDO_MOVE|MAP_UNDO_CREATE|MAP_UNDO_LINK);

			create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0} {}", in_room);
		}

		exit = create_exit(ses, ses->map->in_room, "{%d} {%s} {%s}", in_room, dir->arg1, dir->arg1);

		ses->map->dir = exit->dir;

		if (find_exit(ses, in_room, dir->arg2) == NULL)
		{
			create_exit(ses, in_room, "{%d} {%s} {%s}", ses->map->in_room, dir->arg2, dir->arg2);
		}

		if (ses->map->nofollow == 0)
		{
			ses->map->nofollow++;

			script_driver(ses, LIST_COMMAND, argument);

			ses->map->nofollow--;
		}
		goto_room(ses, in_room);

		if (HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP))
		{
			SET_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);
		}

		pop_call();
		return 1;
	}
	pop_call();
	return 0;
}

void add_undo(struct session *ses, char *format, ...)
{
	struct link_data *link;
	char *arg, *buf, *dir, *rev, *val;
	va_list args;

	push_call("add_undo(%s,%s)",ses->name, format);

	buf = str_alloc_stack(0);
	dir = str_alloc_stack(0);
	rev = str_alloc_stack(0);
	val = str_alloc_stack(0);

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	arg = get_arg_in_braces(ses, buf, dir, GET_ONE);
	arg = get_arg_in_braces(ses, arg, rev, GET_ONE);
	arg = get_arg_in_braces(ses, arg, val, GET_ONE);

	link = (struct link_data *) calloc(1, sizeof(struct link_data));

	link->str1 = strdup(dir);
	link->str2 = strdup(rev);
	link->str3 = strdup(val);

	LINK(link, ses->map->undo_head, ses->map->undo_tail);

	ses->map->undo_size++;

	if (ses->map->undo_size > 100)
	{
		del_undo(ses, ses->map->undo_head);
	}
	pop_call();
	return;
}

void del_undo(struct session *ses, struct link_data *link)
{
	UNLINK(link, ses->map->undo_head, ses->map->undo_tail);

	free(link->str1);
	free(link->str2);
	free(link->str3);

	free(link);

	ses->map->undo_size--;
}

/*
	Draws a map on a grid, original breadth first improvements by Bryan Turner
*/



struct grid_node
{
	int vnum;
	int from;
	int flags;
	float length;
	int dir;
	int w;
	int x;
	int y;
	int z;
};

void displaygrid_build(struct session *ses, int vnum, int x, int y, int z)
{
	int head, tail;
	struct grid_node *node, *temp, list[MAP_BF_SIZE];
	struct exit_data *exit;
	struct room_data *room, *toroom;

	push_call("displaygrid_build(%p,%d,%d,%d,%d)",ses,vnum,x,y,z);

	map_grid_x = x;
	map_grid_y = y;

	head = 0;
	tail = 1;

	node = &list[head];

	node->vnum   = vnum;
	node->length = ses->map->room_list[vnum]->weight;
	node->flags  = 0;

	if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS))
	{
		node->x      = (x - 2) / 2 + ses->map->center_x;
	}
	else
	{
		node->x      = x / 2 + ses->map->center_x;
	}
	node->y      = y / 2 + ses->map->center_y;
	node->z      = z / 2 + ses->map->center_z;



	ses->map->display_stamp++;

	for (vnum = 0 ; vnum < x * y ; vnum++)
	{
		ses->map->grid_rooms[vnum] = NULL;
		ses->map->grid_vnums[vnum] = 0;
	}

	while (head != tail)
	{
		node = &list[head];

		head = (head + 1) % MAP_BF_SIZE;

		room = ses->map->room_list[node->vnum];

		if (ses->map->display_stamp != room->display_stamp)
		{
			room->display_stamp = ses->map->display_stamp;
		}
		else if (room->length <= node->length /*&& !HAS_BIT(room->flags, ROOM_FLAG_VOID)*/)
		{
			continue;
		}

		room->length = node->length;

		if (node->x >= 0 && node->x < map_grid_x && node->y >= 0 && node->y < map_grid_y && node->z == 0)
		{
			if (ses->map->grid_rooms[node->x + map_grid_x * node->y] == NULL)
			{
				ses->map->grid_rooms[node->x + map_grid_x * node->y] = room;
				ses->map->grid_vnums[node->x + map_grid_x * node->y] = room->vnum;
			}
			else if (!HAS_BIT(room->flags, ROOM_FLAG_VOID))
			{
				continue;
			}
		}

		if (HAS_BIT(room->flags, ROOM_FLAG_FOG) && ses->map->in_room != room->vnum)
		{
			continue;
		}

		for (exit = room->f_exit ; exit ; exit = exit->next)
		{
			if (exit->dir == 0)
			{
				continue;
			}

			toroom = ses->map->room_list[exit->vnum];

			if (ses->map->display_stamp == toroom->display_stamp)
			{
				if (HAS_BIT(room->flags, ROOM_FLAG_VOID) || !HAS_BIT(toroom->flags, ROOM_FLAG_VOID))
				{
					if (room->length >= ses->map->room_list[exit->vnum]->length)
					{
						continue;
					}
				}
			}

			if (HAS_BIT(exit->flags, EXIT_FLAG_HIDE) || HAS_BIT(ses->map->room_list[exit->vnum]->flags, ROOM_FLAG_HIDE))
			{
				continue;
			}

			if (head == (tail + 1) % MAP_BF_SIZE)
			{
				break;
			}

			if (HAS_BIT(room->flags, ROOM_FLAG_VOID) && ses->map->in_room != room->vnum && get_room_exits(ses, room->vnum) % 2 == 0)
			{
				exit = room->exit_grid[dir_to_grid(node->dir)];
			
				if (exit == NULL)
				{
					if (ses->map->room_list[room->f_exit->vnum]->display_stamp != ses->map->display_stamp)
					{
						exit = room->f_exit;
					}
					else if (ses->map->room_list[room->l_exit->vnum]->display_stamp != ses->map->display_stamp)
					{
						exit = room->l_exit;
					}
					else if (ses->map->room_list[room->f_exit->vnum]->length <= ses->map->room_list[room->l_exit->vnum]->length)
					{
						exit = room->f_exit;
					}
					else
					{
						exit = room->l_exit;
					}
				}
			}

			temp = &list[tail];

			temp->vnum   = exit->vnum;
			temp->dir    = exit->dir;
			temp->x      = node->x + (HAS_BIT(exit->dir, MAP_EXIT_E) ?  1 : HAS_BIT(exit->dir, MAP_EXIT_W) ? -1 : 0);
			temp->y      = node->y + (HAS_BIT(exit->dir, MAP_EXIT_N) ?  1 : HAS_BIT(exit->dir, MAP_EXIT_S) ? -1 : 0);
			temp->z      = node->z + (HAS_BIT(exit->dir, MAP_EXIT_U) ?  1 : HAS_BIT(exit->dir, MAP_EXIT_D) ? -1 : 0);
			temp->length = node->length + exit->weight + ses->map->room_list[exit->vnum]->weight;

			temp->flags  = 0;

			tail = (tail + 1) % MAP_BF_SIZE;

			if (HAS_BIT(room->flags, ROOM_FLAG_VOID) && ses->map->in_room != room->vnum && get_room_exits(ses, room->vnum) % 2 == 0)
			{
				temp->length = node->length + 0.000001;
				break;
			}
		}
	}

	if (HAS_BIT(ses->map->flags, MAP_FLAG_UPDATETERRAIN))
	{
		update_terrain(ses);
	}

	{
		int terrain;

		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (x = 1 ; x < map_grid_x - 1 ; x++)
			{
				if (ses->map->grid_rooms[x + map_grid_x * y] == NULL)
				{
					vnum = get_terrain_vnum(ses, NULL, x, y);

					if (vnum != -1)
					{
						terrain = ses->map->room_list[vnum]->terrain_index;

						ses->map->grid_rooms[x + map_grid_x * y] = ses->list[LIST_TERRAIN]->list[terrain]->room;
						ses->map->grid_vnums[x + map_grid_x * y] = vnum;
					}
				}
			}
		}
	}

	pop_call();
	return;
}


void show_vtmap(struct session *ses)
{
	char buf[BUFFER_SIZE], out[BUFFER_SIZE], tmp[BUFFER_SIZE];
	char *ptb;
	int x, y, line;
	int top_row, top_col, bot_row, bot_col, rows, cols, row;

	push_call("show_vtmap(%p)",ses);

	if (ses->map == NULL || !HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP))
	{
		pop_call();
		return;
	}

	if (ses->map->room_list[ses->map->in_room] == NULL)
	{
		pop_call();
		return;
	}

	if (ses != gtd->ses || HAS_BIT(gtd->ses->flags, SES_FLAG_READMUD))
	{
		pop_call();
		return;
	}

	if (HAS_BIT(ses->map->flags, MAP_FLAG_RESIZE))
	{
		DEL_BIT(ses->map->flags, MAP_FLAG_RESIZE);

		map_offset(ses, NULL, "", "");
	}

	if (ses->map->rows > 1 && ses->map->cols > 1)
	{
		top_row = ses->map->top_row;
		top_col = ses->map->top_col;
		bot_row = ses->map->bot_row;
		bot_col = ses->map->bot_col;

		rows    = ses->map->rows;
		cols    = ses->map->cols;
	}
	else
	{
		top_row = 1;
		top_col = 1;
		bot_row = UMAX(1, ses->split->top_row - 2);
		bot_col = gtd->screen->cols;

		rows    = UMAX(1, ses->split->top_row - 2);
		cols    = gtd->screen->cols;
	}

//	print_stdout(0, 0, "\e[%d;%d;%d;%d${", top_row, top_col, bot_row, bot_col);

	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		erase_square(ses, top_row, top_col, bot_row, bot_col);

		map_grid_y = 2 + rows / 3;
		map_grid_x = 2 + cols / 6;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS))
	{
		erase_square(ses, top_row, top_col, bot_row - 1, bot_col);

		map_grid_y = 2 + (rows + 2) / 2;
		map_grid_x = 2 + (cols + 4) / 5;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS))
	{
		erase_square(ses, top_row, top_col, bot_row, bot_col);

		map_grid_y = 2 + rows / 2;
		map_grid_x = 2 + cols / 5;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_MUDFONT))
	{
		erase_square(ses, top_row, top_col, bot_row, bot_col);

		map_grid_y = 2 + rows;
		map_grid_x = 2 + cols / 2;
	}
	else
	{
		erase_square(ses, top_row, top_col, bot_row, bot_col);

		map_grid_y = 2 + rows;
		map_grid_x = 2 + cols;
	}

	displaygrid_build(ses, ses->map->in_room, map_grid_x, map_grid_y, 0);

	save_pos(ses);

	goto_pos(ses, top_row, 1);

	row = top_row;

	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (line = 1 ; line <= 3 ; line++)
			{
				ptb = buf;

				ptb += sprintf(ptb, "%s", ses->map->color[MAP_COLOR_BACK]);

				for (x = 1 ; x < map_grid_x - 1 ; x++)
				{
					ptb += sprintf(ptb, "%s", draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));
				}
				substitute(ses, buf, out, SUB_COL|SUB_LIT);

				print_stdout(0, 0, "\e[%d;%dH%s\e[0m", row++, top_col, out);
			}
		}
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS))
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (line = 1 ; line <= 2 ; line++)
			{
				if (line == 2 && y == 1)
				{
					continue;
				}
				ptb = buf;

				ptb += sprintf(ptb, "%s", ses->map->color[MAP_COLOR_BACK]);

				for (x = 1 ; x < map_grid_x - 1 ; x++)
				{
					if (x == map_grid_x - 2)
					{
						strcpy(tmp, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));

						ptb += sprintf(ptb, "%.*s", string_str_raw_len(ses, tmp, 0, 2), tmp);
					}
					else
					{
						ptb += sprintf(ptb, "%s", draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));
					}
				}
				substitute(ses, buf, out, SUB_COL|SUB_LIT);

				print_stdout(0, 0, "\e[%d;%dH%s\e[0m", row++, top_col, out);
			}
		}
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS))
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (line = 1 ; line <= 2 ; line++)
			{
				ptb = buf;

				ptb += sprintf(ptb, "%s", ses->map->color[MAP_COLOR_BACK]);

				for (x = 1 ; x < map_grid_x - 1 ; x++)
				{
					ptb += sprintf(ptb, "%s", draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));
				}
				substitute(ses, buf, out, SUB_COL|SUB_LIT);

				print_stdout(0, 0, "\e[%d;%dH%s\e[0m", row++, top_col, out);
			}
		}
	}
	else
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			ptb = buf;

			ptb += sprintf(ptb, "%s", ses->map->color[MAP_COLOR_BACK]);

			for (x = 1 ; x < map_grid_x - 1 ; x++)
			{
				ptb += sprintf(ptb, "%s", draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], 0, x, y));
			}
			substitute(ses, buf, out, SUB_COL|SUB_LIT);

			print_stdout(0, 0, "\e[%d;%dH%s\e[0m", row++, top_col, out);
		}
	}

	restore_pos(ses);

	pop_call();
	return;
}

// http://shapecatcher.com/unicode/block/Mathematical_Operators diagonal #⊥⊤#

char *draw_room(struct session *ses, struct room_data *room, int line, int x, int y)
{
	static char buf[201], *room_color, room_left[101], room_right[101], room_symbol[101];
	int index, symsize = 1, exits, exit1, exit2, room1, room2, offset, flags = 0;
//	struct room_data * room_grid[11];

	push_call("draw_room(%p,%p,%d,%d,%d)",ses,room,line,x,y);

	offset = HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) ? LEGEND_UNICODE : LEGEND_ASCII;

	room_color = ses->map->color[MAP_COLOR_ROOM];

	index = LEGEND_ASCII_MISC;

	if (room && room->vnum)
	{
		// experimental

		substitute(ses, room->symbol, room_symbol, SUB_VAR|SUB_FUN);

		symsize = strip_color_strlen(ses, room_symbol);

		if (HAS_BIT(room->flags, ROOM_FLAG_PATH) && room->search_stamp == ses->map->search->stamp)
		{
			room_color = ses->map->color[MAP_COLOR_PATH];
			if (symsize > 1)
			{
				strcpy(room_symbol, " ");
				symsize = 1;
			}
		}
		else if (*room->color)
		{
			room_color = room->color;
		}
		else
		{
			if (HAS_BIT(room->flags, ROOM_FLAG_INVIS))
			{
				room_color = ses->map->color[MAP_COLOR_INVIS];
			}
			else if (HAS_BIT(room->flags, ROOM_FLAG_HIDE))
			{
				room_color = ses->map->color[MAP_COLOR_HIDE];
			}
			else if (HAS_BIT(room->flags, ROOM_FLAG_AVOID))
			{
				room_color = ses->map->color[MAP_COLOR_AVOID];
			}
			else if (HAS_BIT(room->flags, ROOM_FLAG_BLOCK))
			{
				room_color = ses->map->color[MAP_COLOR_BLOCK];
			}
			else if (HAS_BIT(room->flags, ROOM_FLAG_FOG))
			{
				room_color = ses->map->color[MAP_COLOR_FOG];
			}
			else if (HAS_BIT(ses->map->flags, MAP_FLAG_SYMBOLGRAPHICS))
			{
				room_color = ses->map->color[MAP_COLOR_SYMBOL];
			}
		}

		if (room->vnum == ses->map->in_room)
		{
			if (HAS_BIT(ses->map->flags, MAP_FLAG_DIRECTION))
			{
				exits = ses->map->dir;

				DEL_BIT(exits, MAP_EXIT_U|MAP_EXIT_D);

				switch (exits)
				{
					case MAP_EXIT_N:
						index = LEGEND_ASCII_DIRS + 0;
						break;
					case MAP_EXIT_N+MAP_EXIT_E:
						index = LEGEND_ASCII_DIRS + 1;
						break;
					case MAP_EXIT_E:
						index = LEGEND_ASCII_DIRS + 2;
						break;
					case MAP_EXIT_S+MAP_EXIT_E:
						index = LEGEND_ASCII_DIRS + 3;
						break;
					case MAP_EXIT_S:
						index = LEGEND_ASCII_DIRS + 4;
						break;
					case MAP_EXIT_W+MAP_EXIT_S:
						index = LEGEND_ASCII_DIRS + 5;
						break;
					case MAP_EXIT_W:
						index = LEGEND_ASCII_DIRS + 6;
						break;
					case MAP_EXIT_W+MAP_EXIT_N:
						index = LEGEND_ASCII_DIRS + 7;
						break;
					default:
						index = LEGEND_ASCII_MISC + 1;
						break;
				}
			}
			else
			{
				index = LEGEND_ASCII_MISC + 0;
			}
		}
	}

	if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS))
	{
		struct room_data *room_n, *room_nw, *room_w;
		long long exit_n, exit_nw, exit_w;

		if (room)
		{
			if (HAS_BIT(room->flags, ROOM_FLAG_CURVED))
			{
				sprintf(room_left,  "%s%s", room_color, ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_RL_CURVED]);
				sprintf(room_right, "%s%s", room_color, ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_RR_CURVED]);
			}
			else
			{
				sprintf(room_left,  "%s%s", room_color, ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_RL]);
				sprintf(room_right, "%s%s", room_color, ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_RR]);
			}
		}
		room_n  = ses->map->grid_rooms[x + 0 + map_grid_x * (y + 1)];
		room_nw = ses->map->grid_rooms[x - 1 + map_grid_x * (y + 1)];
		room_w  = ses->map->grid_rooms[x - 1 + map_grid_x * (y + 0)];

		exit_n = exit_nw = exit_w = 0;

		if (room_nw && room_nw->exit_grid[EXIT_GRID_SE])
		{
			SET_BIT(exit_nw, UNICODE_DIR_SE);
		}

		if (room_w && room_w->exit_grid[EXIT_GRID_NE])
		{
			SET_BIT(exit_nw, UNICODE_DIR_NE);
		}

		if (room_n && room_n->exit_grid[EXIT_GRID_SW])
		{
			SET_BIT(exit_nw, UNICODE_DIR_SW);
		}

		if (room && HAS_BIT(room->exit_dirs, MAP_DIR_NW))
		{
			SET_BIT(exit_nw, UNICODE_DIR_NW);
		}

		if (room_n && HAS_BIT(room_n->exit_dirs, MAP_DIR_D))
		{
			SET_BIT(exit_n, MAP_DIR_D);
		}

		if (room && room->exit_grid[EXIT_GRID_N])
		{
			SET_BIT(exit_n, MAP_DIR_N);
		}

		if (room_n && room_n->exit_grid[EXIT_GRID_S])
		{
			SET_BIT(exit_n, MAP_DIR_S);
		}

		if (room && room->exit_grid[EXIT_GRID_W])
		{
			SET_BIT(exit_w, MAP_DIR_W);
		}

		if (room_w && room_w->exit_grid[EXIT_GRID_E])
		{
			SET_BIT(exit_w, MAP_DIR_E);
		}

		sprintf(buf, "%s", ses->map->color[MAP_COLOR_EXIT]);

		switch (line)
		{
			case 1:
				switch (exit_nw)
				{
					case 0:
						strcat(buf, draw_terrain_symbol(ses, room, line, 1, x, y, TERRAIN_FLAG_DOUBLE));
						strcat(buf, draw_terrain_symbol(ses, room, line, 2, x, y, TERRAIN_FLAG_DOUBLE));
						break;

					case UNICODE_DIR_SE:
						cat_sprintf(buf, "%s%s", get_exit_color(ses, 0, room_nw->exit_grid[EXIT_GRID_SE]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SE]);
						strcat(buf, draw_terrain_symbol(ses, room, line, 2, x, y, flags));
						break;
					case UNICODE_DIR_NE:
						cat_sprintf(buf, "%s%s", get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_NE]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NE]);
						strcat(buf, draw_terrain_symbol(ses, room, line, 2, x, y, flags));
						break;
					case UNICODE_DIR_SE|UNICODE_DIR_NE:
						cat_sprintf(buf, "%s%s", room_nw->length < room_w->length ? get_exit_color(ses, 0, room_nw->exit_grid[EXIT_GRID_SE]) : get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_NE]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SE + UNICODE_DIR_NE]);
						strcat(buf, draw_terrain_symbol(ses, room, line, 2, x, y, flags));
						break;

					case UNICODE_DIR_SE|UNICODE_DIR_NW:
						cat_sprintf(buf, "%s%s", room_nw->length < room->length ? get_exit_color(ses, 0, room_nw->exit_grid[EXIT_GRID_SE]) : get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_NW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SE + UNICODE_DIR_NW]);
						break;

					case UNICODE_DIR_NE|UNICODE_DIR_SW:
						cat_sprintf(buf, "%s%s", room_w->length < room_n->length ? get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_NE]) : get_exit_color(ses, 0, room_n->exit_grid[EXIT_GRID_SW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NE + UNICODE_DIR_SW]);
						break;
//
					case UNICODE_DIR_NE|UNICODE_DIR_NW:
						cat_sprintf(buf, "%s%s%s%s", get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_NE]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NE], get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_NW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NW]);
						break;

					case UNICODE_DIR_NE|UNICODE_DIR_SW|UNICODE_DIR_NW:
						cat_sprintf(buf, "%s%s%s%s", get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_NE]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NE], room_n->length < room->length ? get_exit_color(ses, 0, room_n->exit_grid[EXIT_GRID_SW]) : get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_NW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NW + UNICODE_DIR_SW]);
						break;

					case UNICODE_DIR_SE|UNICODE_DIR_SW:
						cat_sprintf(buf, "%s%s%s%s", get_exit_color(ses, 0, room_nw->exit_grid[EXIT_GRID_SE]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SE], get_exit_color(ses, 0, room_n->exit_grid[EXIT_GRID_SW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SW]);
						break;

					case UNICODE_DIR_SE|UNICODE_DIR_SW|UNICODE_DIR_NW:
						cat_sprintf(buf, "%s%s%s%s", get_exit_color(ses, 0, room_nw->exit_grid[EXIT_GRID_SE]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SE], room_n->length < room->length ? get_exit_color(ses, 0, room_n->exit_grid[EXIT_GRID_SW]) : get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_NW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NW + UNICODE_DIR_SW]);
						break;

					case UNICODE_DIR_SE|UNICODE_DIR_NE|UNICODE_DIR_SW:
						cat_sprintf(buf, "%s%s%s%s", room_nw->length < room_w->length ? get_exit_color(ses, 0, room_nw->exit_grid[EXIT_GRID_SE]) : get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_NE]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SE + UNICODE_DIR_NE], get_exit_color(ses, 0, room_n->exit_grid[EXIT_GRID_SW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SW]);
						break;

					case UNICODE_DIR_SE|UNICODE_DIR_NE|UNICODE_DIR_NW:
						cat_sprintf(buf, "%s%s%s%s", room_nw->length < room_w->length ? get_exit_color(ses, 0, room_nw->exit_grid[EXIT_GRID_SE]) : get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_NE]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SE + UNICODE_DIR_NE], get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_NW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NW]);
						break;

					case UNICODE_DIR_SW:
						strcat(buf, draw_terrain_symbol(ses, room, line, 1, x, y, flags));
						cat_sprintf(buf, "%s%s", get_exit_color(ses, 0, room_n->exit_grid[EXIT_GRID_SW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SW]);
						break;

					case UNICODE_DIR_NW:
						strcat(buf, draw_terrain_symbol(ses, room, line, 1, x, y, flags));
						cat_sprintf(buf, "%s%s", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_NW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NW]);
						break;

					case UNICODE_DIR_SW|UNICODE_DIR_NW:
						strcat(buf, draw_terrain_symbol(ses, room, line, 1, x, y, flags));
						cat_sprintf(buf, "%s%s", room_n->length < room->length ? get_exit_color(ses, 0, room_n->exit_grid[EXIT_GRID_SW]) : get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_NW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SW + UNICODE_DIR_NW]);
						break;

					case UNICODE_DIR_NW|UNICODE_DIR_SE|UNICODE_DIR_NE|UNICODE_DIR_SW:
//						cat_sprintf(buf, "ᐳᐸ");
						cat_sprintf(buf, "%s%s%s%s", room_nw->length < room_w->length ? get_exit_color(ses, 0, room_nw->exit_grid[EXIT_GRID_SE]) : get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_NE]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_SE + UNICODE_DIR_NE], room_n->length < room->length ? get_exit_color(ses, 0, room_n->exit_grid[EXIT_GRID_SW]) : get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_NW]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NW + UNICODE_DIR_SW]);
						break;
					default:
						cat_sprintf(buf, "??");
						break;
				}

				if (!HAS_BIT(exit_n, MAP_DIR_D) && !HAS_BIT(exit_n, MAP_DIR_N|MAP_DIR_S))
				{
					strcat(buf, draw_terrain_symbol(ses, room, line, 3, x, y, TERRAIN_FLAG_DOUBLE));
					strcat(buf, draw_terrain_symbol(ses, room, line, 4, x, y, TERRAIN_FLAG_DOUBLE));
				}
				else
				{
					if (HAS_BIT(exit_n, MAP_DIR_D))
					{
						cat_sprintf(buf, "%s%s", get_exit_color(ses, 0, room_n->exit_grid[EXIT_GRID_D]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_D]);
					}
					else
					{
						strcat(buf, draw_terrain_symbol(ses, room, line, 3, x, y, flags));
					}

					switch (HAS_BIT(exit_n, MAP_DIR_N|MAP_DIR_S))
					{
						case MAP_DIR_N:
							cat_sprintf(buf, "%s%s", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_N]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_N]);
							break;
						case MAP_DIR_S:
							cat_sprintf(buf, "%s%s", get_exit_color(ses, 0, room_n->exit_grid[EXIT_GRID_S]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_S]);
							break;
						case MAP_DIR_N|MAP_DIR_S:
							cat_sprintf(buf, "%s%s", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_N]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_NS]);
							break;
						default:
							strcat(buf, draw_terrain_symbol(ses, room, line, 4, x, y, flags));
							break;
					}
				}

				if (room && room->exit_grid[EXIT_GRID_U])
				{
					cat_sprintf(buf, "%s%s", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_U]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_U]);
				}
				else
				{
					strcat(buf, draw_terrain_symbol(ses, room, line, 5, x, y, flags));
				}
				break;

			case 2:
				buf[0] = 0;

				if (room == NULL || room->vnum == 0)
				{
					if (HAS_BIT(exit_w, MAP_DIR_E))
					{
						sprintf(buf, "%s%s%s",
							get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_E]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_E],
							draw_terrain_symbol(ses, room, line, 2, x, y, flags));
					}
					else
					{
						strcat(buf, draw_terrain_symbol(ses, room, line, 1, x, y, TERRAIN_FLAG_DOUBLE));
						strcat(buf, draw_terrain_symbol(ses, room, line, 2, x, y, TERRAIN_FLAG_DOUBLE));
					}
					strcat(buf, draw_terrain_symbol(ses, room, line, 3, x, y, TERRAIN_FLAG_DOUBLE));
					strcat(buf, draw_terrain_symbol(ses, room, line, 4, x, y, TERRAIN_FLAG_DOUBLE));

					strcat(buf, draw_terrain_symbol(ses, room, line, 5, x, y, flags));

					pop_call();
					return buf;
				}

				switch (exit_w)
				{
					case 0:
						strcat(buf, draw_terrain_symbol(ses, room, line, 1, x, y, TERRAIN_FLAG_DOUBLE));
						strcat(buf, draw_terrain_symbol(ses, room, line, 2, x, y, TERRAIN_FLAG_DOUBLE));
						break;
					case MAP_DIR_E:
						sprintf(buf, "%s%s%s", get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_E]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_E], draw_terrain_symbol(ses, room, line, 2, x, y, flags));
						break;
					case MAP_DIR_W:
						sprintf(buf, "%s%s%s", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_W]), draw_terrain_symbol(ses, room, line, 1, x, y, flags), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_W]);
						break;
					case MAP_DIR_E|MAP_DIR_W:
						if (room->exit_grid[EXIT_GRID_W]->vnum == room_w->vnum && room_w->exit_grid[EXIT_GRID_E]->vnum == room->vnum)
						{
							// ‒‒
							sprintf(buf, "%s%s%s%s", get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_E]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_EW], get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_W]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_EW]);
						}
						else
						{
							sprintf(buf, "%s%s%s%s", get_exit_color(ses, 0, room_w->exit_grid[EXIT_GRID_E]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_E], get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_W]), ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_W]);
						}
						break;
					default:
						strcat(buf, "??");
						break;
				}

				if (room->vnum == ses->map->in_room)
				{
					cat_sprintf(buf, "%s%s%s%s", room_left, ses->map->color[MAP_COLOR_USER], ses->map->legend[offset + index], room_right);
				}
				else if (symsize > 1)
				{
					cat_sprintf(buf, "%s%-3s", ses->map->color[MAP_COLOR_SYMBOL], room_symbol);
				}
				else
				{
					if (HAS_BIT(room->flags, ROOM_FLAG_VOID))
					{
						if (HAS_BIT(room->exit_dirs, MAP_DIR_W))
						{
							cat_sprintf(buf, "%s─", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_W]));
						}
						else
						{
							cat_sprintf(buf, " ");
						}

						if (*room_symbol != ' ' && symsize == 1)
						{
							cat_sprintf(buf, "%s%-1s", ses->map->color[MAP_COLOR_SYMBOL], room_symbol);
						}
						else
						{
							if (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_E|MAP_DIR_S|MAP_DIR_W) == (MAP_DIR_N|MAP_DIR_E|MAP_DIR_S|MAP_DIR_W))
							{
								if (get_exit_length(ses, room->exit_grid[EXIT_GRID_N]) < get_exit_length(ses, room->exit_grid[EXIT_GRID_W]))
								{
									cat_sprintf(buf, "%s│", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_N]));
								}
								else
								{
									cat_sprintf(buf, "%s─", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_W]));
								}
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_E|MAP_DIR_W) == (MAP_DIR_E|MAP_DIR_W))
							{
								cat_sprintf(buf, "%s─", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_W]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_S) == (MAP_DIR_N|MAP_DIR_S))
							{
								cat_sprintf(buf, "%s│", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_N]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_E|MAP_DIR_S|MAP_DIR_W|MAP_DIR_NW|MAP_DIR_NE|MAP_DIR_SE|MAP_DIR_SW) == (MAP_DIR_S))
							{
								cat_sprintf(buf, "%s│", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_S]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_E|MAP_DIR_S|MAP_DIR_W|MAP_DIR_NW|MAP_DIR_NE|MAP_DIR_SE|MAP_DIR_SW) == (MAP_DIR_N))
							{
								cat_sprintf(buf, "%s│", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_N]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_E|MAP_DIR_S|MAP_DIR_W|MAP_DIR_NW|MAP_DIR_NE|MAP_DIR_SE|MAP_DIR_SW) == (MAP_DIR_E))
							{
								cat_sprintf(buf, "%s─", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_E]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_E|MAP_DIR_S|MAP_DIR_W|MAP_DIR_NW|MAP_DIR_NE|MAP_DIR_SE|MAP_DIR_SW) == (MAP_DIR_W))
							{
								cat_sprintf(buf, "%s─", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_W]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_E|MAP_DIR_S|MAP_DIR_W|MAP_DIR_NW|MAP_DIR_NE|MAP_DIR_SE|MAP_DIR_SW) == (MAP_DIR_N|MAP_DIR_E|MAP_DIR_W))
							{
								cat_sprintf(buf, "%s├", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_N]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_E|MAP_DIR_S|MAP_DIR_W|MAP_DIR_NW|MAP_DIR_NE|MAP_DIR_SE|MAP_DIR_SW) == (MAP_DIR_S|MAP_DIR_E|MAP_DIR_W))
							{
								cat_sprintf(buf, "%s┬", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_E]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_NW|MAP_DIR_SW) && HAS_BIT(room->exit_dirs, MAP_DIR_E))
							{
								cat_sprintf(buf, "%s─", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_E]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_NE))
							{
								cat_sprintf(buf, "%s╴", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_NE]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_SE))
							{
								cat_sprintf(buf, "%s╴", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_SE]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_SW))
							{
								cat_sprintf(buf, "%s╴", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_SW]));
							}
							else if (HAS_BIT(room->exit_dirs, MAP_DIR_NW))
							{
								cat_sprintf(buf, "%s╴", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_NW]));
							}
							else
							{
								cat_sprintf(buf, " ");
							}
						}

						if (HAS_BIT(room->exit_dirs, MAP_DIR_E))
						{
							cat_sprintf(buf, "%s─", get_exit_color(ses, 0, room->exit_grid[EXIT_GRID_E]));
						}
						else
						{
							cat_sprintf(buf, " ");
						}
					}
					else
					{
						if (symsize == 1)
						{
							cat_sprintf(buf, "%s%s%-1s%s", room_left, ses->map->color[MAP_COLOR_SYMBOL], room_symbol, room_right);
						}
						else
						{
							cat_sprintf(buf, "%s%s%-1s%s", room_left, ses->map->color[MAP_COLOR_SYMBOL], "", room_right);
						}
					}
				}
				cat_sprintf(buf, "%s", ses->map->color[MAP_COLOR_BACK]);

				break;
		}
		pop_call();
		return buf;
	}

	if (HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS))
	{
		struct room_data *room_w;
		long long exit_w;

		room_w = ses->map->grid_rooms[x - 1 + map_grid_x * (y + 0)];
		exit_w = 0;

		if (room_w)
		{
			if (HAS_BIT(room_w->exit_dirs, MAP_DIR_E))
			{
				SET_BIT(exit_w, MAP_DIR_E);
			}
			if (HAS_BIT(room_w->exit_dirs, MAP_DIR_U))
			{
				SET_BIT(exit_w, MAP_DIR_U);
			}
		}

		sprintf(buf, "%s", room_color);

		switch (line)
		{
			case 1:
				switch (exit_w)
				{
					case 0:
						strcat(buf, " ");
						break;
					case MAP_DIR_E:
						strcat(buf, "═");
						break;
					case MAP_DIR_E|MAP_DIR_U:
						strcat(buf, "═̂");
						break;
					case MAP_DIR_U:
						strcat(buf, " ̂");
						break;
					default:
						strcat(buf, "?");
						break;
				}

				if (room == NULL)
				{
					strcat(buf, "    ");
				}
				else
				{
					strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_W) ? "═" : " ");

					if (room->vnum == ses->map->in_room)
					{
						cat_sprintf(buf, "%s", ses->map->color[MAP_COLOR_USER]);
					}

					switch (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_W))
					{
						case MAP_DIR_N:
							strcat(buf, "║");
							break;
						case MAP_DIR_W:
							strcat(buf, "═");
							break;
						case MAP_DIR_N|MAP_DIR_W:
							strcat(buf, "╝");
							break;
						default:
							strcat(buf, "╔");
							break;
					}


					strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_N) ? " " : "═");


					switch (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_E))
					{
						case MAP_DIR_N:
							strcat(buf, "║");
							break;
						case MAP_DIR_E:
							strcat(buf, "═");
							break;
						case MAP_DIR_N|MAP_DIR_E:
							strcat(buf, "╚");
							break;
						default:
							strcat(buf, "╗");
							break;
					}
				}
				break;

			case 2:
				switch (exit_w)
				{
					case 0:
						strcat(buf, " ");
						break;
					case MAP_DIR_E:
						strcat(buf, "═");
						break;
					case MAP_DIR_E|MAP_DIR_U:
						strcat(buf, "═");
						break;
					default:
						strcat(buf, " ");
						break;
				}

				if (room == NULL)
				{
					strcat(buf, " ");
				}
				else
				{
					switch (HAS_BIT(room->exit_dirs, MAP_DIR_W|MAP_DIR_D))
					{
						case MAP_DIR_W:
							strcat(buf, "═");
							break;
						case MAP_DIR_W|MAP_DIR_D:
							strcat(buf, "═̬");
							break;
						case MAP_DIR_D:
							strcat(buf, " ̬");
							break;
						default:
							strcat(buf, " ");
							break;
					}
				}

				if (room == NULL)
				{
					strcat(buf, "   ");
				}
				else
				{
					if (room->vnum == ses->map->in_room)
					{
						cat_sprintf(buf, "%s", ses->map->color[MAP_COLOR_USER]);
					}

					switch (HAS_BIT(room->exit_dirs, MAP_DIR_S|MAP_DIR_W))
					{
						case MAP_DIR_S:
							strcat(buf, "║");
							break;
						case MAP_DIR_W:
							strcat(buf, "═");
							break;
						case MAP_DIR_S|MAP_DIR_W:
							strcat(buf, "╗");
								break;
						default:
							strcat(buf, "╚");
							break;
					}

					strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_S) ? " " : "═");

					switch (HAS_BIT(room->exit_dirs, MAP_DIR_S|MAP_DIR_E))
					{
						case MAP_DIR_S:
							strcat(buf, "║");
							break;
						case MAP_DIR_E:
							strcat(buf, "═");
							break;
						case MAP_DIR_S|MAP_DIR_E:
							strcat(buf, "╔");
							break;
						default:
							strcat(buf, "╝");
							break;
					}
				}
				break;
		}
		pop_call();
		return buf;
	}

	if (room == NULL || room->vnum == 0)
	{
		if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
		{
			strcpy(buf, "");

			for (index = 1 ; index <= 6 ; index++)
			{
				strcat(buf, draw_terrain_symbol(ses, room, line, index, x, y, TERRAIN_FLAG_DOUBLE));
			}
		}
		else if (HAS_BIT(ses->map->flags, MAP_FLAG_MUDFONT))
		{
			sprintf(buf, "  ");
		}
		else
		{
			sprintf(buf, " ");
		}
		pop_call();
		return buf;
	}


	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		strcpy(buf, "");

		switch (line)
		{
			case 1:
				if (room->exit_grid[EXIT_GRID_NW])
				{
					strcat(buf, get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_NW]));
					strcat(buf, "\\");
					strcat(buf, draw_terrain_symbol(ses, room, line, 2, x, y, flags));
				}
				else
				{
					strcat(buf, draw_terrain_symbol(ses, room, line, 1, x, y, TERRAIN_FLAG_DOUBLE));
					strcat(buf, draw_terrain_symbol(ses, room, line, 2, x, y, TERRAIN_FLAG_DOUBLE));
				}

				if (room->exit_grid[EXIT_GRID_N] || room->exit_grid[EXIT_GRID_U])
				{
					cat_sprintf(buf, "%s%s", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_N]), room->exit_grid[EXIT_GRID_N]  ? "|"  : draw_terrain_symbol(ses, room, line, 3, x, y, flags));
					cat_sprintf(buf, "%s%s", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_U]), room->exit_grid[EXIT_GRID_U]  ? "+"  : draw_terrain_symbol(ses, room, line, 4, x, y, flags));
				}
				else
				{
					strcat(buf, draw_terrain_symbol(ses, room, line, 3, x, y, TERRAIN_FLAG_DOUBLE));
					strcat(buf, draw_terrain_symbol(ses, room, line, 4, x, y, TERRAIN_FLAG_DOUBLE));
				}

				if (room->exit_grid[EXIT_GRID_NE])
				{
					cat_sprintf(buf, "%s%s%s", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_NE]), "/", draw_terrain_symbol(ses, room, line, 6, x, y, flags));
				}
				else
				{
					strcat(buf, draw_terrain_symbol(ses, room, line, 5, x, y, TERRAIN_FLAG_DOUBLE));
					strcat(buf, draw_terrain_symbol(ses, room, line, 6, x, y, TERRAIN_FLAG_DOUBLE));
				}
				break;

			case 2:
				if (HAS_BIT(room->flags, ROOM_FLAG_CURVED))
				{
					sprintf(room_left,  "%s%s", room_color, ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_RL_CURVED]);
					sprintf(room_right, "%s%s", room_color, ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_RR_CURVED]);
				}
				else
				{
					sprintf(room_left,  "%s%s", room_color, ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_RL]);
					sprintf(room_right, "%s%s", room_color, ses->map->legend[LEGEND_UNICODE_GRAPHICS + UNICODE_DIR_RR]);
				}

				if (!HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS) && symsize <= 3)
				{
					cat_sprintf(buf, "%s%s",
						get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_W]),
						room->exit_grid[EXIT_GRID_W]  ? "-"  : draw_terrain_symbol(ses, room, line, 1, x, y, flags));
				}

				if (room->vnum == ses->map->in_room)
				{
					if (!HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS))
					{
						cat_sprintf(buf, "%s%s%s%s", room_left, ses->map->color[MAP_COLOR_USER], ses->map->legend[index], room_right);
					}
					else
					{
						if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIILENGTH))
						{
							cat_sprintf(buf, "%s%5.1f", ses->map->color[MAP_COLOR_USER], room->length, ses->map->color[MAP_COLOR_EXIT]);
						}
						else
						{
							cat_sprintf(buf, "%s%05d", ses->map->color[MAP_COLOR_USER], room->vnum);
						}
					}
				}
				else
				{
					if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS))
					{
						if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIILENGTH))
						{
							cat_sprintf(buf, "%s%5.1f%s", ses->map->color[MAP_COLOR_USER], room->length, ses->map->color[MAP_COLOR_EXIT]);
						}
						else
						{
							cat_sprintf(buf, "%s%05d%s", room_color, room->vnum, ses->map->color[MAP_COLOR_EXIT]);
						}
					}
					else if (symsize > 3)
					{
						cat_sprintf(buf, "%s%-5s", ses->map->color[MAP_COLOR_SYMBOL], room_symbol);
					}
					else if (HAS_BIT(room->flags, ROOM_FLAG_VOID))
					{
						if (*room_symbol != ' ' && symsize == 1)
						{
							if (room->exit_dirs == (MAP_DIR_E|MAP_DIR_W))
							{
								cat_sprintf(buf, "%s-%s%s%s-", ses->map->color[MAP_COLOR_EXIT], ses->map->color[MAP_COLOR_SYMBOL], room_symbol, ses->map->color[MAP_COLOR_EXIT]);
							}
							else
							{
								cat_sprintf(buf, "%s %-2s", ses->map->color[MAP_COLOR_SYMBOL], room_symbol);
							}
						}
						else if (*room_symbol != ' ' && strip_color_strlen(ses, room_symbol) > 1)
						{
							cat_sprintf(buf, "%s %-2s", ses->map->color[MAP_COLOR_SYMBOL], room_symbol);
						}
						else
						{
							strcat(buf, ses->map->color[MAP_COLOR_EXIT]);

							switch (room->exit_dirs)
							{
								case MAP_DIR_N|MAP_DIR_S:
									cat_sprintf(buf, " | ");
									break;
								case MAP_DIR_E|MAP_DIR_W:
									cat_sprintf(buf, "---");
									break;
								case MAP_DIR_NE|MAP_DIR_SW:
									cat_sprintf(buf, " / ");
									break;
								case MAP_DIR_NW|MAP_DIR_SE:
									cat_sprintf(buf, " \\ ");
									break;
								default:
									cat_sprintf(buf, " * ");
									break;
							}
						}
					}
					else
					{
						if (symsize <= 1)
						{
							cat_sprintf(buf, "%s%s%-1s%s", room_left, ses->map->color[MAP_COLOR_SYMBOL], room_symbol, room_right);
						}
						else
						{
							cat_sprintf(buf, "%s%s%-3s", room_color, ses->map->color[MAP_COLOR_SYMBOL], room_symbol);
						}
					}
				}

				if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS) || symsize > 3)
				{
					cat_sprintf(buf, "%s%s", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_E]), room->exit_grid[EXIT_GRID_E] ? "-" : draw_terrain_symbol(ses, room, line, 6, x, y, flags));
				}
				else
				{
					if (room->exit_grid[EXIT_GRID_E])
					{
						cat_sprintf(buf, "%s--", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_E]));
					}
					else
					{
						strcat(buf, draw_terrain_symbol(ses, room, line, 5, x, y, TERRAIN_FLAG_DOUBLE));
						strcat(buf, draw_terrain_symbol(ses, room, line, 6, x, y, TERRAIN_FLAG_DOUBLE));
					}
				}
				break;

			case 3:
				if (room->exit_grid[EXIT_GRID_SW] || room->exit_grid[EXIT_GRID_D])
				{
					cat_sprintf(buf, "%s%s", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_SW]), room->exit_grid[EXIT_GRID_SW] ? "/"  : draw_terrain_symbol(ses, room, line, 1, x, y, flags));
					cat_sprintf(buf, "%s%s", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_D]), room->exit_grid[EXIT_GRID_D]  ? "-"  : draw_terrain_symbol(ses, room, line, 2, x, y, flags));
				}
				else
				{
					strcat(buf, draw_terrain_symbol(ses, room, line, 1, x, y, TERRAIN_FLAG_DOUBLE));
					strcat(buf, draw_terrain_symbol(ses, room, line, 2, x, y, TERRAIN_FLAG_DOUBLE));
				}

				if (room->exit_grid[EXIT_GRID_S])
				{
					cat_sprintf(buf, "%s|%s", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_S]), draw_terrain_symbol(ses, room, line, 4, x, y, flags));
				}
				else
				{
					strcat(buf, draw_terrain_symbol(ses, room, line, 3, x, y, TERRAIN_FLAG_DOUBLE));
					strcat(buf, draw_terrain_symbol(ses, room, line, 4, x, y, TERRAIN_FLAG_DOUBLE));
				}

				if (room->exit_grid[EXIT_GRID_SE])
				{
					cat_sprintf(buf, "%s\\%s", get_exit_color(ses, room->vnum, room->exit_grid[EXIT_GRID_SE]), draw_terrain_symbol(ses, room, line, 6, x, y, flags));
				}
				else
				{
					strcat(buf, draw_terrain_symbol(ses, room, line, 5, x, y, TERRAIN_FLAG_DOUBLE));
					strcat(buf, draw_terrain_symbol(ses, room, line, 6, x, y, TERRAIN_FLAG_DOUBLE));
				}
				break;
		}
		pop_call();
		return buf;
	}


	if (room->vnum == ses->map->in_room)
	{
		exits = ses->map->dir;

		DEL_BIT(exits, MAP_EXIT_U|MAP_EXIT_D);

		if (HAS_BIT(ses->map->flags, MAP_FLAG_DIRECTION))
		{
			switch (exits)
			{
				case MAP_EXIT_N:
					index = 24;
					break;
				case MAP_EXIT_N+MAP_EXIT_E:
					index = 25;
					break;
				case MAP_EXIT_E:
					index = 26;
					break;
				case MAP_EXIT_S+MAP_EXIT_E:
					index = 27;
					break;
				case MAP_EXIT_S:
					index = 28;
					break;
				case MAP_EXIT_S+MAP_EXIT_W:
					index = 29;
					break;
				case MAP_EXIT_W:
					index = 30;
					break;
				case MAP_EXIT_N+MAP_EXIT_W:
					index = 31;
					break;

				default:
					index = 17;
					break;
			}
		}
		else
		{
			index = 16;
		}

		if (HAS_BIT(ses->map->flags, MAP_FLAG_MUDFONT))
		{
			sprintf(buf, "%s%s%s", ses->map->color[MAP_COLOR_USER], ses->map->legend[offset + index], ses->map->legend[offset + index]);
		}
		else
		{
			sprintf(buf, "%s%s", ses->map->color[MAP_COLOR_USER], ses->map->legend[offset + index]);
		}
		pop_call();
		return buf;
	}

	exit1 = 0;
	exit2 = 0;
	exits = 0;

	if (HAS_BIT(room->exit_dirs, MAP_DIR_N))
	{
		SET_BIT(exit1, 1 << 0);
		SET_BIT(exit2, 1 << 0);
		SET_BIT(exits, MAP_EXIT_N);
	}

	if (HAS_BIT(room->exit_dirs, MAP_DIR_W))
	{
		SET_BIT(exit1, 1 << 2);
		SET_BIT(exits, MAP_EXIT_W);
	}

	if (HAS_BIT(room->exit_dirs, MAP_DIR_E))
	{
		SET_BIT(exit2, 1 << 2);
		SET_BIT(exits, MAP_EXIT_E);
	}

	if (HAS_BIT(room->exit_dirs, MAP_DIR_S))
	{
		SET_BIT(exit1, 1 << 4);
		SET_BIT(exit2, 1 << 4);
		SET_BIT(exits, MAP_EXIT_S);
	}

	if (HAS_BIT(ses->map->flags, MAP_FLAG_MUDFONT))
	{
		if (HAS_BIT(room->exit_dirs, MAP_DIR_NW))
		{
			SET_BIT(exit1, 1 << 1);
		}
		if (HAS_BIT(room->exit_dirs, MAP_DIR_NE))
		{
			SET_BIT(exit2, 1 << 1);
		}
		if (HAS_BIT(room->exit_dirs, MAP_DIR_SW))
		{
			SET_BIT(exit1, 1 << 3);
		}
		if (HAS_BIT(room->exit_dirs, MAP_DIR_SE))
		{
			SET_BIT(exit2, 1 << 3);
		}

		room1 = exit1 + LEGEND_MUDFONT_NWS;
		room2 = exit2 + LEGEND_MUDFONT_NES;

		if (HAS_BIT(room->flags, ROOM_FLAG_VOID))
		{
			room1 += 64;
			room2 += 64;
		}

		if (HAS_BIT(room->flags, ROOM_FLAG_CURVED))
		{
			switch (room->exit_dirs)
			{
				case MAP_DIR_N|MAP_DIR_E:
				case MAP_DIR_N|MAP_DIR_SE:
					room1 = LEGEND_MUDFONT_CURVED + 0;
					break;
				case MAP_DIR_S|MAP_DIR_E:
				case MAP_DIR_S|MAP_DIR_NE:
					room1 = LEGEND_MUDFONT_CURVED + 1;
					break;
				case MAP_DIR_S|MAP_DIR_W:
				case MAP_DIR_S|MAP_DIR_NW:
					room2 = LEGEND_MUDFONT_CURVED + 2;
					break;
				case MAP_DIR_N|MAP_DIR_W:
				case MAP_DIR_N|MAP_DIR_SW:
					room2 = LEGEND_MUDFONT_CURVED + 3;
					break;
			}
		}

		sprintf(buf, "%s%s%s", room_color, ses->map->legend[room1], ses->map->legend[room2]);
	}
	else
	{
		if (HAS_BIT(ses->map->flags, MAP_FLAG_SYMBOLGRAPHICS) && room_symbol[0] && room_symbol[0] != ' ')
		{
			sprintf(buf, "%s%-1s", room_color, room_symbol);
		}
		else
		{
			if (HAS_BIT(room->flags, ROOM_FLAG_VOID) && (exits == MAP_EXIT_N+MAP_EXIT_S || exits == MAP_EXIT_E+MAP_EXIT_W))
			{
				sprintf(buf, "%s%s", room_color, exits == MAP_EXIT_N+MAP_EXIT_S ? ses->map->legend[offset+16+2] : ses->map->legend[offset+16+3]);
			}
			else
			{
				if (HAS_BIT(room->flags, ROOM_FLAG_CURVED))
				{
					switch (room->exit_dirs)
					{
						case MAP_DIR_N|MAP_DIR_E:
							exits = 16 + 4;
							break;
						case MAP_DIR_S|MAP_DIR_E:
							exits = 16 + 5;
							break;
						case MAP_DIR_S|MAP_DIR_W:
							exits = 16 + 6;
							break;
						case MAP_DIR_N|MAP_DIR_W:
							exits = 16 + 7;
							break;
					}
				}
				sprintf(buf, "%s%s", room_color, ses->map->legend[offset + exits]);
			}
		}
	}
	pop_call();
	return buf;
}

void search_keywords(struct session *ses, char *arg, char *out, char *var)
{
	char buf[MAP_SEARCH_MAX][BUFFER_SIZE], arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], *str;
	int type, max;

	push_call("search_keywords(%p,%p,%p,%p)",ses,arg,out,var);

	for (type = 0 ; type < MAP_SEARCH_MAX ; type++)
	{
		buf[type][0] = 0;
	}

	var[0] = 0;

	type = 0;

	while (*arg && type < MAP_SEARCH_MAX)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

		if (*arg1 == '{')
		{
			strcpy(arg2, arg1);

			arg = get_arg_in_braces(ses, arg2, arg1, GET_ALL);
		}

		if (!strcasecmp(arg1, "roomid"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_ID], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(arg1, "roomname"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_NAME], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(arg1, "roomexits"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_EXITS], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(arg1, "roomdesc"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_DESC], GET_ALL, SUB_VAR|SUB_FUN);

			str = buf[MAP_SEARCH_DESC];

			while ((str = strchr(str, '\n')))
			{
				*str = ' ';
			}
		}
		else if (!strcasecmp(arg1, "roomarea"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_AREA], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(arg1, "roomnote"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_NOTE], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(arg1, "roomterrain"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_TERRAIN], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(arg1, "roomflag"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_FLAG], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(arg1, "variable"))
		{
			arg = sub_arg_in_braces(ses, arg, var, GET_ALL, SUB_VAR|SUB_FUN);
		}
		else
		{
			strcpy(buf[type++], arg1);
		}
	}

	for (max = MAP_SEARCH_MAX - 1 ; max >= 0 ; max--)
	{
		if (*buf[max])
		{
			break;
		}
	}

	out[0] = 0;

	for (type = 0 ; type <= max ; type++)
	{
		cat_sprintf(out, "{%s}", buf[type]);
	}
	pop_call();
	return;
}

void map_search_compile(struct session *ses, char *arg, char *var)
{
	char tmp[BUFFER_SIZE], buf[BUFFER_SIZE], *ptb;
	struct listnode *node;

	push_call("map_search_compile(%p,%p,%p)",ses,arg,var);

	search_keywords(ses, arg, tmp, var);

	arg = sub_arg_in_braces(ses, tmp, buf, GET_ALL, SUB_VAR|SUB_FUN); // name

	ses->map->search->min = ses->map->search->max = ses->map->search->vnum = 0;

	if (is_math(ses, buf))
	{
		if (strstr(buf, ".."))
		{
			get_ellipsis(ses, ses->map->size, buf, &ses->map->search->min, &ses->map->search->max);
			ses->map->search->min++;
			ses->map->search->max++;
		}
		else
		{
			ses->map->search->vnum = (int) get_number(ses, buf);
		}
	}

	if (ses->map->search->vnum || ses->map->search->min || ses->map->search->max)
	{
		pop_call();
		return;
	}

	if (ses->map->search->arg)
	{
		free(ses->map->search->arg);
	}

	if (*buf)
	{
		ses->map->search->arg = strdup(buf);

		node = search_node_list(ses->list[LIST_LANDMARK], ses->map->search->arg);

		if (node)
		{
			ses->map->search->vnum = node->val32[0];

			if (ses->map->search->vnum)
			{
				pop_call();
				return;
			}
		}
	}
	else
	{
		ses->map->search->arg = NULL;
	}

	if (ses->map->search->name)
	{
		free(ses->map->search->name);
	}

	if (*buf)
	{
		strcat(buf, "$");

		ses->map->search->name = tintin_regexp_compile(ses, NULL, buf, PCRE_ANCHORED);
	}
	else
	{
		ses->map->search->name = NULL;
	}

	arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN); // exits

	ses->map->search->exit_dirs = 0;
	ses->map->search->exit_size = 0;

	if (ses->map->search->exit_list)
	{
		free(ses->map->search->exit_list);
	}

	if (*buf)
	{
		char exit[BUFFER_SIZE];
		ptb = buf;

		tmp[0] = 0;

		if (is_math(ses, buf))
		{
			ses->map->search->exit_dirs = get_number(ses, buf);

			if (HAS_BIT(ses->map->search->exit_dirs, MAP_DIR_N))  ses->map->search->exit_size++;
			if (HAS_BIT(ses->map->search->exit_dirs, MAP_DIR_E))  ses->map->search->exit_size++;
			if (HAS_BIT(ses->map->search->exit_dirs, MAP_DIR_S))  ses->map->search->exit_size++;
			if (HAS_BIT(ses->map->search->exit_dirs, MAP_DIR_W))  ses->map->search->exit_size++;
			if (HAS_BIT(ses->map->search->exit_dirs, MAP_DIR_U))  ses->map->search->exit_size++;
			if (HAS_BIT(ses->map->search->exit_dirs, MAP_DIR_D))  ses->map->search->exit_size++;
			if (HAS_BIT(ses->map->search->exit_dirs, MAP_DIR_NE)) ses->map->search->exit_size++;
			if (HAS_BIT(ses->map->search->exit_dirs, MAP_DIR_NW)) ses->map->search->exit_size++;
			if (HAS_BIT(ses->map->search->exit_dirs, MAP_DIR_SE)) ses->map->search->exit_size++;
			if (HAS_BIT(ses->map->search->exit_dirs, MAP_DIR_SW)) ses->map->search->exit_size++;
		}
		else
		{
			while (*ptb)
			{
				ptb = get_arg_in_braces(ses, ptb, exit, GET_ONE);

				ses->map->search->exit_size++;

				node = search_node_list(ses->list[LIST_PATHDIR], exit);

				if (node)
				{
					SET_BIT(ses->map->search->exit_dirs, 1LL << pdir(node));
				}
				else
				{
					SET_BIT(ses->map->search->exit_dirs, 1); // flag indicates no exits

					cat_sprintf(tmp, "{%s}", exit);
				}

				if (*ptb == COMMAND_SEPARATOR)
				{
					ptb++;
				}
			}
		}
		ses->map->search->exit_list = strdup(tmp);
	}
	else
	{
		ses->map->search->exit_list = strdup("");
	}

	arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN); // desc

	if (ses->map->search->desc)
	{
		free(ses->map->search->desc);
	}

	if (*buf)
	{
		strcat(buf, "$");

		ses->map->search->desc = tintin_regexp_compile(ses, NULL, buf, PCRE_ANCHORED|PCRE_DOLLAR_ENDONLY|PCRE_DOTALL);
	}
	else
	{
		ses->map->search->desc = NULL;
	}

	arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN);

	// area

	if (ses->map->search->area)
	{
		free(ses->map->search->area);
	}

	if (*buf)
	{
		strcat(buf, "$");

		ses->map->search->area = tintin_regexp_compile(ses, NULL, buf, PCRE_ANCHORED);
	}
	else
	{
		ses->map->search->area = NULL;
	}

	// note

	arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN);

	if (ses->map->search->note)
	{
		free(ses->map->search->note);
	}

	if (*buf)
	{
		strcat(buf, "$");

		ses->map->search->note = tintin_regexp_compile(ses, NULL, buf, PCRE_ANCHORED);
	}
	else
	{
		ses->map->search->note = NULL;
	}

	// terrain

	arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN);

	if (ses->map->search->terrain)
	{
		free(ses->map->search->terrain);
	}

	if (*buf)
	{
		strcat(buf, "$");

		ses->map->search->terrain = tintin_regexp_compile(ses, NULL, buf, PCRE_ANCHORED);
	}
	else
	{
		ses->map->search->terrain = NULL;
	}

	arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN); // flag

	if (*buf)
	{
		char flags[BUFFER_SIZE];

		ses->map->search->flag = get_number(ses, buf);

		ptb = buf;

		while (*ptb)
		{
			ptb = sub_arg_in_braces(ses, ptb, flags, GET_ONE, SUB_NONE);

			if (is_abbrev(buf, "avoid"))
			{
				SET_BIT(ses->map->search->flag, ROOM_FLAG_AVOID);
			}
			else if (is_abbrev(buf, "curved"))
			{
				SET_BIT(ses->map->search->flag, ROOM_FLAG_CURVED);
			}
			else if (is_abbrev(buf, "hide"))
			{
				SET_BIT(ses->map->search->flag, ROOM_FLAG_HIDE);
			}
			else if (is_abbrev(buf, "invis"))
			{
				SET_BIT(ses->map->search->flag, ROOM_FLAG_INVIS);
			}
			else if (is_abbrev(buf, "leave"))
			{
				SET_BIT(ses->map->search->flag, ROOM_FLAG_LEAVE);
			}
			else if (is_abbrev(buf, "void"))
			{
				SET_BIT(ses->map->search->flag, ROOM_FLAG_VOID);
			}
			else if (is_abbrev(buf, "static"))
			{
				SET_BIT(ses->map->search->flag, ROOM_FLAG_STATIC);
			}

			if (*ptb == COMMAND_SEPARATOR)
			{
				ptb++;
			}
		}
	}
	else
	{
		ses->map->search->flag = 0;
	}

	arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN); // id

	if (ses->map->search->id)
	{
		free(ses->map->search->id);
	}

	if (*buf)
	{
		ses->map->search->id = strdup(buf);
	}
	else
	{
		ses->map->search->id = NULL;
	}

	pop_call();
	return;
}

int match_room(struct session *ses, int vnum, struct search_data *search)
{
	struct room_data *room = ses->map->room_list[vnum];

	if (room == NULL)
	{
		return 0;
	}

	if (search->vnum)
	{
		return room->vnum == search->vnum;
	}

	if (search->min || search->max)
	{
		return room->vnum >= search->min && room->vnum <= search->max;
	}

	if (search->id)
	{
		return !strcmp(room->id, search->id);
	}

	if (search->name)
	{
		if (!regexp_compare(ses, search->name, room->name, "", 0, 0))
		{
			return 0;
		}
	}

	if (search->exit_dirs)
	{
		char *arg, exit[BUFFER_SIZE];

		if (search->exit_dirs != room->exit_dirs)
		{
			return 0;
		}
		if (search->exit_size != room->exit_size)
		{
			return 0;
		}

		arg = search->exit_list;

		while (*arg)
		{
			arg = get_arg_in_braces(ses, arg, exit, GET_ONE);

			if (!find_exit(ses, vnum, exit))
			{
				return 0;
			}

			if (*arg == COMMAND_SEPARATOR)
			{
				arg++;
			}
		}
	}

	if (search->desc)
	{
		if (!regexp_compare(ses, search->desc, room->desc, "", 0, 0))
		{
			return 0;
		}
	}

	if (search->area)
	{
		if (!regexp_compare(ses, search->area, room->area, "", 0, 0))
		{
			return 0;
		}
	}

	if (search->note)
	{
		if (!regexp_compare(ses, search->note, room->note, "", 0, 0))
		{
			return 0;
		}
	}

	if (search->terrain)
	{
		if (!regexp_compare(ses, search->terrain, room->terrain, "", 0, 0))
		{
			return 0;
		}
	}

	if (search->flag)
	{
		if ((room->flags & search->flag) != search->flag)
		{
			return 0;
		}
	}
	return 1;
}

int find_path(struct session *ses, char *arg)
{
	struct exit_data *exit;
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int room;

	push_call("find_path(%p,%p)",ses,arg);

	arg = substitute_speedwalk(ses, arg, arg1);

	room = ses->map->in_room;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

		exit = find_exit(ses, room, arg2);

		if (exit == NULL)
		{
			pop_call();
			return 0;
		}

		room = tunnel_void(ses, room, exit->vnum, exit->dir);

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}

	pop_call();
	return room == ses->map->in_room ? 0 : room;
}

int find_location(struct session *ses, char *arg)
{
	struct listnode *dir;
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];
	int x, y, z;
	
	push_call("find_location(%p,%p)",ses,arg);

	if (find_exit(ses, ses->map->in_room, arg))
	{
		pop_call();
		return find_exit(ses, ses->map->in_room, arg)->vnum;
	}

	sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (is_math(ses, arg1))
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

		x = get_number(ses, arg1);
		y = get_number(ses, arg2);
		z = get_number(ses, arg3);
	}
	else
	{
		x = y = z = 0;

		arg = substitute_speedwalk(ses, arg, arg2);

		while (*arg)
		{
			arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

			dir = search_node_list(ses->list[LIST_PATHDIR], arg3);

			if (dir == NULL)
			{
//				show_error(ses, LIST_COMMAND, "#ERROR: #MAP FIND_LOCATION: {%s} IS AN INVALID PATHDIR.", arg3);

				pop_call();
				return 0;
			}

			x += (HAS_BIT(pdir(dir), MAP_EXIT_E) ? 1 : HAS_BIT(pdir(dir), MAP_EXIT_W) ? -1 : 0);
			y += (HAS_BIT(pdir(dir), MAP_EXIT_N) ? 1 : HAS_BIT(pdir(dir), MAP_EXIT_S) ? -1 : 0);
			z += (HAS_BIT(pdir(dir), MAP_EXIT_U) ? 1 : HAS_BIT(pdir(dir), MAP_EXIT_D) ? -1 : 0);

			if (*arg == COMMAND_SEPARATOR)
			{
				arg++;
			}
		}
	}

	pop_call();
	return spatialgrid_find(ses, ses->map->in_room, x, y, z);
}

int find_room(struct session *ses, char *arg)
{
	char var[BUFFER_SIZE];
	struct listnode *node;
	int room;

	push_call("find_room(%p,%s)",ses,arg);

	map_search_compile(ses, arg, var);

	if (ses->map->search->vnum > 0 && ses->map->search->vnum < ses->map->size)
	{
		if (ses->map->room_list[ses->map->search->vnum])
		{
			pop_call();
			return ses->map->search->vnum;
		}
		pop_call();
		return 0;
	}

	if (ses->map->search->arg)
	{
		node = search_node_list(ses->list[LIST_LANDMARK], ses->map->search->arg);

		if (node)
		{
			if (ses->map->room_list[node->val32[0]])
			{
				pop_call();
				return node->val32[0];
			}
			pop_call();
			return 0;
		}
	}

	if (ses->map->in_room)
	{
		room = searchgrid_find(ses, ses->map->in_room, ses->map->search);

		if (room)
		{
			pop_call();
			return room;
		}
	}

	for (room = 0 ; room < ses->map->size ; room++)
	{
		if (ses->map->room_list[room] == NULL)
		{
			continue;
		}

		if (!match_room(ses, room, ses->map->search))
		{
			continue;
		}
		pop_call();
		return room;
	}
	pop_call();
	return 0;
}

void goto_room(struct session *ses, int room)
{
	int last_room = ses->map->in_room;
	char *dir;

	push_call("goto_room(%p,%d)",ses,room);

	dir = dir_to_exit(ses, ses->map->dir);

	if (ses->map->in_room)
	{
		check_all_events(ses, EVENT_FLAG_MAP, 0, 3, "MAP EXIT ROOM", ntos(last_room), ntos(room), dir);
		check_all_events(ses, EVENT_FLAG_MAP, 1, 3, "MAP EXIT ROOM %d", last_room, ntos(last_room), ntos(room), dir);
	}

	ses->map->in_room = room;

	DEL_BIT(ses->map->room_list[room]->flags, ROOM_FLAG_PATH);
	DEL_BIT(ses->map->room_list[room]->flags, ROOM_FLAG_FOG);

	if (last_room == 0)
	{
		check_all_events(ses, EVENT_FLAG_MAP, 0, 1, "MAP ENTER MAP", ntos(room));
	}

	check_all_events(ses, EVENT_FLAG_MAP, 0, 3, "MAP ENTER ROOM", ntos(room), ntos(last_room), dir);
	check_all_events(ses, EVENT_FLAG_MAP, 1, 3, "MAP ENTER ROOM %d", room, ntos(room), ntos(last_room), dir);

	pop_call();
	return;
}

int find_new_room(struct session *ses)
{
	int room;

	for (room = 1 ; room < ses->map->size ; room++)
	{
		if (ses->map->room_list[room] == NULL)
		{
			break;
		}
	}

	if (room == ses->map->size)
	{
		show_error(ses, LIST_COMMAND, "#MAP CREATE ROOM: Maximum amount of rooms of %d reached. Use #map resize.", ses->map->size);

		return 0;
	}
	return room;
}

int dir_flags(struct session *ses, int room, int dir)
{
	struct exit_data *exit;

	for (exit = ses->map->room_list[room]->f_exit ; exit ; exit = exit->next)
	{
		if (exit->dir == dir)
		{
			return exit->flags; /* | HAS_BIT(ses->map->room_list[exit->vnum]->flags, EXIT_FLAG_ALL);*/
		}
	}
	return 0;
}

struct exit_data *find_exit(struct session *ses, int room, char *arg)
{
	struct exit_data *exit;

	for (exit = ses->map->room_list[room]->f_exit ; exit ; exit = exit->next)
	{
//		if (!strcmp(exit->name, arg) || exit->vnum == atoi(arg))
		if (!strcmp(exit->name, arg))
		{
			return exit;
		}
	}
	return NULL;
}

struct exit_data *find_exit_vnum(struct session *ses, int room, int vnum)
{
	struct exit_data *exit;

	for (exit = ses->map->room_list[room]->f_exit ; exit ; exit = exit->next)
	{
		if (exit->vnum == vnum)
		{
			return exit;
		}
	}
	return NULL;
}

int check_global(struct session *ses, int room)
{
	if (HAS_BIT(ses->map->room_list[room]->flags, ROOM_FLAG_NOGLOBAL))
	{
		return FALSE;
	}
	
	if (ses->map->room_list[ses->map->global_vnum] == NULL)
	{
		return FALSE;
	}

	if (room == ses->map->global_vnum)
	{
		return FALSE;
	}
	return TRUE;
}

int tunnel_void(struct session *ses, int from, int room, int dir)
{
	push_call("tunnel_void(%p,%d,%d,%d)",ses,from,room,dir);

	if (!HAS_BIT(ses->map->room_list[room]->flags, ROOM_FLAG_VOID))
	{
		pop_call();
		return room;
	}

	if (get_room_exits(ses, room) != 2)
	{
		struct exit_data *exit = ses->map->room_list[room]->exit_grid[dir_to_grid(dir)];

		if (exit)
		{
			pop_call();
			return tunnel_void(ses, room, exit->vnum, exit->dir);
		}
		pop_call();
		return room;
	}

	if (ses->map->room_list[room]->f_exit->vnum != from)
	{
		pop_call();
		return tunnel_void(ses, room, ses->map->room_list[room]->f_exit->vnum, ses->map->room_list[room]->f_exit->dir);
	}

	if (ses->map->room_list[room]->l_exit->vnum != from)
	{
		pop_call();
		return tunnel_void(ses, room, ses->map->room_list[room]->l_exit->vnum, ses->map->room_list[room]->l_exit->dir);
	}
	show_error(ses, LIST_COMMAND, "\e[1;31mtunnel_void(%p,%d,%d,%d) NO VALID EXITS FOUND.",ses,from,room,dir);

	pop_call();
	return room;
}

// shortest_path() utilities

int searchgrid_find(struct session *ses, int from, struct search_data *search)
{
	int vnum, head, tail, index;
	float length;
	struct grid_node *node, *temp, list[MAP_BF_SIZE];
	struct exit_data *exit;
	struct room_data *room;

	search->stamp++;

	head = 0;
	tail = 1;

	node = &list[head];

	node->vnum   = from;
	node->length = ses->map->room_list[from]->weight;

	// for map_list

	node->w      = 0;
	node->x      = 0;
	node->y      = 0;
	node->z      = 0;

	while (head != tail)
	{
		node = &list[head];

		room = ses->map->room_list[node->vnum];

		length = node->length;

		head = (head + 1) % MAP_BF_SIZE;

		if (search->stamp != room->search_stamp)
		{
			room->search_stamp = search->stamp;

			// first come first serve like with spatialgrid_find

			room->w = node->w;
			room->x = node->x;
			room->y = node->y;
			room->z = node->z;

			DEL_BIT(room->flags, ROOM_FLAG_PATH);
		}
		else if (length >= room->length)
		{
			if (room->vnum != ses->map->global_vnum && room->w && node->w == 0)
			{
				room->w = node->w;
				room->x = node->x;
				room->y = node->y;
				room->z = node->z;
			}
			continue;
		}

		room->length = length;

		if (search->vnum)
		{
			if (room->vnum == search->vnum)
			{
				return room->vnum;
			}
		}
		else
		{
			if (match_room(ses, room->vnum, search))
			{
				return room->vnum;
			}
		}

		if (check_global(ses, room->vnum))
		{
			exit = ses->map->global_exit;
		}
		else
		{
			exit = room->f_exit;
		}

		for ( ; exit ; exit = exit->next)
		{
			vnum = tunnel_void(ses, room->vnum, exit->vnum, exit->dir);

			if (HAS_BIT(exit->flags, EXIT_FLAG_AVOID|EXIT_FLAG_BLOCK) || HAS_BIT(ses->map->room_list[vnum]->flags, ROOM_FLAG_AVOID|ROOM_FLAG_BLOCK))
			{
				goto next_exit;
			}

			length = room->length + exit->weight + ses->map->room_list[vnum]->weight;

			if (search->stamp == ses->map->room_list[vnum]->search_stamp)
			{
				if (length >= ses->map->room_list[vnum]->length)
				{
					goto next_exit;
				}
			}

			temp = &list[tail];

			temp->vnum   = vnum;
			temp->length = length;
			temp->w      = room->vnum == ses->map->global_vnum ? 1 : room->w;
			temp->x      = room->x + (HAS_BIT(exit->dir, MAP_EXIT_E) ?  1 : HAS_BIT(exit->dir, MAP_EXIT_W) ? -1 : 0);
			temp->y      = room->y + (HAS_BIT(exit->dir, MAP_EXIT_N) ?  1 : HAS_BIT(exit->dir, MAP_EXIT_S) ? -1 : 0);
			temp->z      = room->z + (HAS_BIT(exit->dir, MAP_EXIT_U) ?  1 : HAS_BIT(exit->dir, MAP_EXIT_D) ? -1 : 0);

			/*
				list must remain ordered by length
			*/

			index = tail;

			while (index != head)
			{
				temp = &list[index];

				node = &list[index ? index - 1 : MAP_BF_SIZE - 1];

				if (temp->length >= node->length)
				{
					break;
				}

				vnum         = temp->vnum;
				length       = temp->length;

				temp->vnum   = node->vnum;
				temp->length = node->length;

				node->vnum   = vnum;
				node->length = length;

				index = index ? index - 1 : MAP_BF_SIZE - 1;
			}

			tail = (tail + 1) % MAP_BF_SIZE;

			if (tail == head)
			{
				show_error(ses, LIST_COMMAND, "#SHORTEST PATH: MAP TOO BIG FOR BF STACK OF %d", MAP_BF_SIZE);
				break;
			}

			next_exit:

			if (exit == ses->map->global_exit)
			{
				exit->next = room->f_exit;
			}
		}
	}
	return 0;
}

int searchgrid_walk(struct session *ses, int offset, int from, int dest)
{
	int vnum, trim, head, tail, index;
	float length;
	struct grid_node *node, *temp, list[MAP_BF_SIZE];
	struct exit_data *exit;
	struct room_data *room;

	head = 0;
	tail = 1;

	list[head].vnum   = from;
	list[head].length = ses->map->room_list[from]->weight;

	while (head != tail)
	{
		node = &list[head];

		room = ses->map->room_list[node->vnum];

		length = node->length;

		head = (head + 1) % MAP_BF_SIZE;


		if (length >= room->length)
		{
			continue;
		}

		room->length = length;

		if (room->vnum == dest)
		{
			return room->vnum;
		}

		trim = 1;

		if (check_global(ses, room->vnum))
		{
			exit = ses->map->global_exit;
		}
		else
		{
			exit = room->f_exit;
		}

		for ( ; exit ; exit = exit->next)
		{
			vnum = tunnel_void(ses, room->vnum, exit->vnum, exit->dir);

			if (HAS_BIT(exit->flags, EXIT_FLAG_AVOID|EXIT_FLAG_BLOCK) || HAS_BIT(ses->map->room_list[vnum]->flags, ROOM_FLAG_AVOID|ROOM_FLAG_BLOCK))
			{
				goto next_exit;
			}

			length = room->length + exit->weight + ses->map->room_list[vnum]->weight;

			if (ses->map->search->stamp != ses->map->room_list[vnum]->search_stamp)
			{
				goto next_exit;
			}

			if (length >= ses->map->room_list[vnum]->length || length >= ses->map->room_list[dest]->length)
			{
				goto next_exit;
			}

			temp = &list[tail];

			temp->vnum   = vnum;
			temp->length = length;

			/*
				list must remain ordered by length
			*/

			index = tail;

			while (index != head)
			{
				temp = &list[index];

				node = &list[index ? index - 1 : MAP_BF_SIZE - 1];

				if (temp->length >= node->length)
				{
					break;
				}

				vnum = temp->vnum;
				length = temp->length;

				temp->vnum = node->vnum;
				temp->length = node->length;

				node->vnum = vnum;
				node->length = length;

				index = index ? index - 1 : MAP_BF_SIZE - 1;
			}

			tail = (tail + 1) % MAP_BF_SIZE;

			if (tail == head)
			{
				show_error(ses, LIST_COMMAND, "#SHORTEST PATH: MAP TOO BIG FOR BF STACK OF %d", MAP_BF_SIZE);
				break;
			}
			trim = 0;

			next_exit:

			if (exit == ses->map->global_exit)
			{
				exit->next = room->f_exit;
			}
		}

		if (trim)
		{
			room->length = 0;
		}
	}
	return 0;
}

void shortest_path(struct session *ses, int run, char *delay, char *arg)
{
	char var[BUFFER_SIZE];
	struct exit_data *exit;
	struct room_data *room;
	int vnum, dest;

	if (HAS_BIT(ses->flags, SES_FLAG_PATHMAPPING))
	{
		show_error(ses, LIST_COMMAND, "#SHORTEST PATH: You have to use #PATH END first.");

		return;
	}

	kill_list(ses->list[LIST_PATH]);

	map_search_compile(ses, arg, var);

	dest = searchgrid_find(ses, ses->map->in_room, ses->map->search);

	if (dest == 0 || dest == ses->map->global_vnum)
	{
		show_error(ses, LIST_COMMAND, "#SHORTEST PATH: NO PATH FOUND TO %s.", arg);
		return;
	}

	if (dest == ses->map->in_room)
	{
		show_error(ses, LIST_COMMAND, "#SHORTEST PATH: Already there.");
		return;
	}

	vnum = ses->map->in_room;

	// Slower than a backtrace, but works with mazes.

	while (TRUE)
	{
		room = ses->map->room_list[vnum];

		if (check_global(ses, room->vnum))
		{
			exit = ses->map->global_exit;
		}
		else
		{
			exit = room->f_exit;
		}

		for ( ; exit ; exit = exit->next)
		{
			if (HAS_BIT(exit->flags, EXIT_FLAG_AVOID|EXIT_FLAG_BLOCK) || HAS_BIT(ses->map->room_list[exit->vnum]->flags, ROOM_FLAG_AVOID|ROOM_FLAG_BLOCK))
			{
				goto exit_next;
			}

			vnum = tunnel_void(ses, room->vnum, exit->vnum, exit->dir);

			if (searchgrid_walk(ses, room->length, vnum, dest))
			{
				break;
			}

			exit_next:

			if (exit == ses->map->global_exit)
			{
				exit->next = room->f_exit;
			}
		}

		if (exit == NULL)
		{
			show_error(ses, LIST_COMMAND, "#SHORTEST PATH: UNKNOWN ERROR.");
			return;
		}

		if (exit != ses->map->global_exit)
		{
			if (HAS_BIT(ses->map->flags, MAP_FLAG_NOFOLLOW))
			{
				check_append_path(ses, exit->cmd, "", exit->delay, 0);
			}
			else
			{
				check_append_path(ses, exit->name, "", exit->delay, 0);
			}
		}

		SET_BIT(ses->map->room_list[vnum]->flags, ROOM_FLAG_PATH);

		if (ses->map->room_list[vnum]->search_stamp != ses->map->search->stamp)
		{
			show_error(ses, LIST_COMMAND, "%d bad search stamp %d vs %d", vnum, ses->map->room_list[vnum]->search_stamp, ses->map->search->stamp);
		}

		if (vnum == dest)
		{
			break;
		}
	}

	if (run)
	{
		path_run(ses, delay);
	}
}

/*
	Virtual coordinate search for linkable rooms when creating a new room.
*/

int find_coord(struct session *ses, char *arg)
{
	int dir, x, y, z, room;

	dir = get_exit_dir(ses, arg);

	if (dir == 0)
	{
		return 0;
	}

	x = (HAS_BIT(dir, MAP_EXIT_E) ? 1 : HAS_BIT(dir, MAP_EXIT_W) ? -1 : 0);
	y = (HAS_BIT(dir, MAP_EXIT_N) ? 1 : HAS_BIT(dir, MAP_EXIT_S) ? -1 : 0);
	z = (HAS_BIT(dir, MAP_EXIT_U) ? 1 : HAS_BIT(dir, MAP_EXIT_D) ? -1 : 0);

	room = spatialgrid_find(ses, ses->map->in_room, x, y, z);

	if (ses->map->room_list[room])
	{
		if (HAS_BIT(ses->map->room_list[room]->flags, ROOM_FLAG_STATIC))
		{
			show_message(ses, LIST_PATH, "#MAP: Linkable room is marked static. Creating overlapping room instead.");

			return 0;
		}
	}
	return room;
}

// Used by #map jump and the auto linker

int spatialgrid_find(struct session *ses, int from, int x, int y, int z)
{
	int head, tail;
	struct grid_node *node, *temp, list[MAP_BF_SIZE];
	struct exit_data *exit;
	struct room_data *room;

	push_call("spatialgrid_find(%s,%d,%d,%d,%d)",ses->name,from,x,y,z);

	head = 0;
	tail = 1;

	node = &list[head];

	node->vnum   = from;
	node->x      = 0;
	node->y      = 0;
	node->z      = 0;
	node->length = 0;
	node->flags  = 0;

	ses->map->display_stamp++;

	while (head != tail)
	{
		node = &list[head];

		head = (head + 1) % MAP_BF_SIZE;

		room = ses->map->room_list[node->vnum];

		if (ses->map->display_stamp != room->display_stamp)
		{
			room->display_stamp = ses->map->display_stamp;
		}
		else if (room->length <= node->length)
		{
			continue;
		}

		room->length = node->length;
/*
		if (HAS_BIT(node->flags, GRID_FLAG_HIDE))
		{
			continue;
		}
*/
		if (node->x == x && node->y == y && node->z == z)
		{
			pop_call();
			return node->vnum;
		}

		for (exit = room->f_exit ; exit ; exit = exit->next)
		{
			if (ses->map->display_stamp == ses->map->room_list[exit->vnum]->display_stamp)
			{
				if (room->length >= ses->map->room_list[exit->vnum]->length)
				{
					continue;
				}
			}

			if (exit->dir == 0)
			{
				continue;
			}

			if (HAS_BIT(exit->flags, EXIT_FLAG_HIDE) || HAS_BIT(ses->map->room_list[exit->vnum]->flags, ROOM_FLAG_HIDE))
			{
				continue;
			}

			if (head == (tail + 1) % MAP_BF_SIZE)
			{
				break;
			}

			temp = &list[tail];

			temp->vnum   = exit->vnum;
			temp->w      = node->w;
			temp->x      = node->x + (HAS_BIT(exit->dir, MAP_EXIT_E) ?  1 : HAS_BIT(exit->dir, MAP_EXIT_W) ? -1 : 0);
			temp->y      = node->y + (HAS_BIT(exit->dir, MAP_EXIT_N) ?  1 : HAS_BIT(exit->dir, MAP_EXIT_S) ? -1 : 0);
			temp->z      = node->z + (HAS_BIT(exit->dir, MAP_EXIT_U) ?  1 : HAS_BIT(exit->dir, MAP_EXIT_D) ? -1 : 0);
			temp->length = node->length + 1;
			temp->flags  = 0;
/*
			if (HAS_BIT(exit->flags, EXIT_FLAG_HIDE) || HAS_BIT(ses->map->room_list[exit->vnum]->flags, ROOM_FLAG_HIDE))
			{
				SET_BIT(temp->flags, GRID_FLAG_HIDE);

				temp->length += 1000;
			}
*/
			tail = (tail + 1) % MAP_BF_SIZE;
		}
	}
	pop_call();
	return 0;
}

void explore_path(struct session *ses, int run, char *arg1, char *arg2)
{
	struct exit_data *exit;
	int room, vnum;

	for (vnum = 0 ; vnum < ses->map->size ; vnum++)
	{
		if (ses->map->room_list[vnum])
		{
			DEL_BIT(ses->map->room_list[vnum]->flags, ROOM_FLAG_PATH);
		}
	}

	if (HAS_BIT(ses->flags, SES_FLAG_PATHMAPPING))
	{
		show_error(ses, LIST_COMMAND, "#MAP EXPLORE: You have to use #PATH END first.");

		return;
	}

	kill_list(ses->list[LIST_PATH]);

	room = ses->map->in_room;

	exit = find_exit(ses, room, arg1);

	if (exit == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP: There's no exit named '%s'.", arg1);
		return;
	}

	vnum = exit->vnum;

	if (HAS_BIT(ses->map->flags, MAP_FLAG_NOFOLLOW))
	{
		check_append_path(ses, exit->cmd, "", exit->delay, 0);
	}
	else
	{
		check_append_path(ses, exit->name, "", exit->delay, 0);
	}

	SET_BIT(ses->map->room_list[room]->flags, ROOM_FLAG_PATH);
	SET_BIT(ses->map->room_list[vnum]->flags, ROOM_FLAG_PATH);

	while (get_room_exits(ses, vnum) == 2)
	{
		exit = ses->map->room_list[vnum]->f_exit;

		if (HAS_BIT(ses->map->room_list[exit->vnum]->flags, ROOM_FLAG_PATH))
		{
			exit = ses->map->room_list[vnum]->l_exit;

			if (HAS_BIT(ses->map->room_list[exit->vnum]->flags, ROOM_FLAG_PATH))
			{
				break;
			}
		}

		if (!HAS_BIT(ses->map->room_list[vnum]->flags, ROOM_FLAG_VOID))
		{
			if (HAS_BIT(ses->map->flags, MAP_FLAG_NOFOLLOW))
			{
				check_append_path(ses, exit->cmd, "", exit->delay, 0);
			}
			else
			{
				check_append_path(ses, exit->name, "", exit->delay, 0);
			}
		}

		vnum = exit->vnum;

		SET_BIT(ses->map->room_list[vnum]->flags, ROOM_FLAG_PATH);
	}

	DEL_BIT(ses->map->room_list[room]->flags, ROOM_FLAG_PATH);

	if (run)
	{
		path_run(ses, arg2);
	}
}

void map_mouse_handler(struct session *ses, char *arg1, char *arg2, int row, int col, int rev_row, int rev_col, int height, int width)
{
	char exit[10];
	int x, y, max_x, max_y;
	int top_row, top_col, bot_row, bot_col, rows, cols, char_height, vnum = 0;

	push_call("map_mouse_handler(%p,%p,%p,%d,%d)",ses,arg1,arg2,row,col);

	if (ses->map == NULL || !HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP) || ses->map->room_list[ses->map->in_room] == NULL)
	{
		pop_call();
		return;
	}
/*
	if (!HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		if (arg1 && arg2)
		{
//			do_screen(ses, "{RAISE} {SCREEN MOUSE LOCATION}");
		}
	}
*/

	x = col;
	y = row;

	exit[0] = 0;

	char_height = 1 + height % UMAX(1, gtd->screen->char_height);

	if (ses->map->rows > 1 && ses->map->cols > 1)
	{
		top_row = ses->map->top_row;
		top_col = ses->map->top_col;
		bot_row = ses->map->bot_row;
		bot_col = ses->map->bot_col;

		rows    = ses->map->rows;
		cols    = ses->map->cols;
	}
	else
	{
		top_row = 1;
		top_col = 1;
		bot_row = UMAX(1, ses->split->top_row - 2);
		bot_col = gtd->screen->cols;

		rows    = UMAX(1, ses->split->top_row - 2);
		cols    = gtd->screen->cols;
	}

	y = y - 1;
	x = x - 1;

	if (y > bot_row || y < top_row)
	{
		pop_call();
		return;
	}

	if (x > bot_col || x < top_col)
	{
		pop_call();
		return;
	}

	y = y + 1 - top_row;
	x = x + 1 - top_col;

	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		char *grid[] = { "nw",  "2",  "n",  "u", "ne",  "6",
				  "w", "RL", "RC", "RR",  "e", "e",
				 "sw",  "d",  "s", "16", "se", "18" };

		strcpy(exit, grid[URANGE(0, y % 3 * 6 + x % 6, 17)]);

		y /= 3;
		x /= 6;

		max_y = 2 + rows / 3;
		max_x = 2 + cols / 6;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS))
	{
		char *grid[] = { "se", "sw",  "d",  "s",  "5",   "ne", "nw",  "8",  "n",  "u",   "e", "w", "RL", "RC", "RR",   "e",  "w", "RL", "RC", "RR" };
		int x_mod, y_mod;

		y_mod = y  % 2 * 2 + (char_height * 2 / gtd->screen->char_height ? 1 : 0);
		x_mod = x  % 5;

		y = y  / 2;
		x = x  / 5;

		strcpy(exit, grid[URANGE(0, 5 * y_mod + x_mod, 20)]);

		switch (5 * y_mod + x_mod)
		{
			case 0:
				y--;
			case 5:
			case 10:
			case 15:
				x--;
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				y--;
				break;
		}

		max_y = 2 + (rows + 2) / 2;
		max_x = 2 + (cols + 4) / 5;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS))
	{
		y /= 2;
		x /= 5;

		max_y = 2 + rows / 2;
		max_x = 2 + cols / 5;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_MUDFONT))
	{
		x /= 2;

		max_y = 2 + rows;
		max_x = 2 + cols / 2;
	}
	else
	{
		max_y = 2 + rows;
		max_x = 2 + cols;
	}

	y = max_y - 1 - y;

	if (x < 0 || y < 0)
	{
		pop_call();
		return;
	}

	if (max_x != map_grid_x || max_y != map_grid_y)
	{
		pop_call();
		return;
	}

	vnum = ses->map->grid_rooms[x + 1 + max_x * (y - 1)] ? ses->map->grid_rooms[x + 1 + max_x * (y - 1)]->vnum : 0;

	if (arg1 && arg2)
	{
		check_all_events(ses, EVENT_FLAG_MOUSE, 2, 6, "MAP REGION %s %s", arg1, arg2, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), ntos(vnum), exit);
	}

	if (vnum)
	{
		if (arg1 && arg2)
		{
			check_all_events(ses, EVENT_FLAG_MOUSE, 2, 6,      "MAP %s %s", arg1, arg2, ntos(vnum), exit, ntos(rev_row), ntos(rev_col), ntos(vnum), exit);
			check_all_events(ses, EVENT_FLAG_MOUSE, 2, 6, "MAP ROOM %s %s", arg1, arg2, ntos(row), ntos(col), ntos(rev_row), ntos(rev_col), ntos(vnum), exit);
		}
		else
		{
			check_all_events(ses, EVENT_FLAG_MOUSE, 0, 6, "MAP MOUSE LOCATION", ntos(row), ntos(col), exit, ntos(rev_row), ntos(rev_col), ntos(vnum), exit);
		}
	}
		
	pop_call();
	return;
}

void update_terrain(struct session *ses)
{
	struct room_data *room;
	int vnum;

	DEL_BIT(ses->map->flags, MAP_FLAG_UPDATETERRAIN);

	for (vnum = 1 ; vnum < ses->map->size ; vnum++)
	{
		room = ses->map->room_list[vnum];

		if (room)
		{
			room->terrain_index = bsearch_alpha_list(ses->list[LIST_TERRAIN], room->terrain, 0);
		}
	}
}

/*
	Map options
*/


DO_MAP(map_at)
{
	int new_room;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_NONE);

	if (ses->map->at_room)
	{
		show_error(ses, LIST_COMMAND, "#MAP AT: Nested #map at call from room {%d}.", ses->map->in_room);

		return;
	}

	new_room = find_room(ses, arg1);

	ses->map->at_room = ses->map->in_room;

	if (new_room == 0)
	{
		if (ses->map->in_room)
		{
			new_room = find_location(ses, arg1);
		}

		if (new_room == 0)
		{
			show_message(ses, LIST_COMMAND, "#MAP AT: Couldn't find room or exit {%s}.", arg1);

			ses->map->at_room = 0;

			return;
		}
	}

	ses->map->in_room = new_room;

	script_driver(ses, LIST_COMMAND, arg2);

	if (ses->map)
	{
		ses->map->in_room = ses->map->at_room;
		ses->map->at_room = 0;
	}
}

DO_MAP(map_center)
{
	char arg3[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		ses->map->center_x = ses->map->center_y = ses->map->center_z = 0;
	}
	else
	{
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

		if (!is_math(ses, arg1) || !is_math(ses, arg2) || !is_math(ses, arg3))
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP CENTER {X} {Y} {Z}");

			return;
		}
		else
		{
			ses->map->center_x = get_number(ses, arg1);
			ses->map->center_y = get_number(ses, arg2);
			ses->map->center_z = get_number(ses, arg3);

			show_message(ses, LIST_COMMAND, "#MAP CENTER SET TO {%d} {%d} {%d}", ses->map->center_x, ses->map->center_y, ses->map->center_z);
		}
	}
}

DO_MAP(map_color)
{
	char buf[BUFFER_SIZE];
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1)
	{
		if (!strcasecmp(arg1, "RESET"))
		{
			for (index = 0 ; map_color_table[index].name ; index++)
			{
				strncpy(ses->map->color[index], map_color_table[index].code, COLOR_SIZE - 1);
				strncpy(ses->map->color_raw[index], map_color_table[index].code, COLOR_SIZE - 1);
			}

			return;
		}

		for (index = 0 ; map_color_table[index].name ; index++)
		{
			if (is_abbrev(arg1, map_color_table[index].name))
			{
				if (is_abbrev(arg2, "RESET"))
				{
					translate_color_names(ses, map_color_table[index].code, ses->map->color[index]);
					strncpy(ses->map->color_raw[index], map_color_table[index].code, COLOR_SIZE - 1);
				}
				else
				{
					translate_color_names(ses, arg2, ses->map->color[index]);
					strncpy(ses->map->color_raw[index], arg2, COLOR_SIZE - 1);
				}

				get_color_names(ses, ses->map->color[index], buf);

				show_message(ses, LIST_COMMAND, "#MAP COLOR %s%10s\e[0m SET TO {%s}", ses->map->color[index], buf, map_color_table[index].name, ses->map->color_raw[index]);

				break;
			}
		}

		if (map_color_table[index].name == NULL)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP COLOR {AVOID|BACKGROUND|EXIT|FOG|HIDE|INVIS|PATH|ROOM|USER} {COLOR CODE}");

			return;
		}
		show_message(ses, LIST_COMMAND, "#MAP: %s color set to: %s", arg1, arg2);
	}
	else
	{
		for (index = 0 ; map_color_table[index].name ; index++)
		{
			get_color_names(ses, ses->map->color[index], buf);

			show_message(ses, LIST_COMMAND, "#MAP COLOR %s%10s\e[0m SET TO {%s}", buf, map_color_table[index].name, ses->map->color_raw[index]);
		}
	}
}

DO_MAP(map_create)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	create_map(ses, arg1, 0);

	show_message(ses, LIST_COMMAND, "#MAP: %d room map created, use #map goto 1, to proceed", ses->map->size);
}

DO_MAP(map_debug)
{
	tintin_printf2(ses, "max spatial grid x: %d", ses->map->max_grid_x);
	tintin_printf2(ses, "max spatial grid y: %d", ses->map->max_grid_y);
	tintin_printf2(ses, "     max undo size: %d", ses->map->undo_size);
	tintin_printf2(ses, "           in room: %d", ses->map->in_room);
	tintin_printf2(ses, "           at room: %d", ses->map->at_room);
	tintin_printf2(ses, "         last room: %d", ses->map->last_room);
	tintin_printf2(ses, "             stamp: %d", ses->map->search->stamp);
	tintin_printf2(ses, "            length: %f", ses->map->room_list[ses->map->in_room]->length);
	tintin_printf2(ses, "          nofollow: %d", ses->map->nofollow);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1)
	{
		if (is_abbrev(arg1, "undo"))
		{
			struct link_data *link;

			for (link = ses->map->undo_head ; link ; link = link->next)
			{
				tintin_printf2(ses, "%05s %05s %s", link->str1, link->str2, link->str3);
			}
		}
	}
}

DO_MAP(map_delete)
{
	int room;
	struct exit_data *exit;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (is_number(arg1))
	{
		room = find_room(ses, arg1);

		if (room == 0)
		{
			show_error(ses, LIST_COMMAND, "#MAP DELETE {%s} - No room with that vnum found", arg1);

			return;
		}
	}
	else if (ses->map->in_room)
	{
		exit = find_exit(ses, ses->map->in_room, arg1);

		if (exit)
		{
			room = exit->vnum;
		}

		if (exit == NULL)
		{
			show_error(ses, LIST_COMMAND, "#MAP DELETE: No exit with that name found");
			
			return;
		}

		room = exit->vnum;
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#MAP DELETE: You must first enter the map");

		return;
	}

	if (room == ses->map->in_room || room == ses->map->at_room)
	{
		show_error(ses, LIST_COMMAND, "#MAP DELETE: You must first leave the room you're trying to delete");
		
		return;
	}

	delete_room(ses, room, TRUE);

	show_message(ses, LIST_COMMAND, "#MAP: Room {%d} deleted", room);
}

DO_MAP(map_destroy)
{
	struct exit_data *exit;
	int index, cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (is_abbrev(arg1, "AREA"))
	{
		if (*arg2 == 0)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP DESTROY AREA {<AREA NAME>}");

			return;
		}

		if (ses->map->room_list[ses->map->in_room] && !strcmp(arg2, ses->map->room_list[ses->map->in_room]->area))
		{
			show_error(ses, LIST_COMMAND, "#MAP DESTROY AREA: YOU MUST FIRST LEAVE THE AREA YOU ARE TRYING TO DESTROY.");

			return;
		}

		for (index = cnt = 0 ; index < ses->map->size ; index++)
		{
			if (ses->map->room_list[index])
			{
				if (!strcmp(arg2, ses->map->room_list[index]->area))
				{
					cnt++;

					delete_room(ses, index, FALSE);
				}
			}
		}

		for (index = 0 ; index < ses->map->size ; index++)
		{
			if (ses->map->room_list[index])
			{
				for (exit = ses->map->room_list[index]->f_exit ; exit ; exit = exit->next)
				{
					if (ses->map->room_list[exit->vnum] == NULL)
					{
						delete_exit(ses, index, exit);

						if (ses->map->room_list[index]->f_exit)
						{
							exit = ses->map->room_list[index]->f_exit;
						}
						else
						{
							break;
						}
					}
				}
			}
		}
		show_message(ses, LIST_COMMAND, "#MAP DESTROY AREA: DELETED %d ROOMS.", cnt);
	}
	else if (is_abbrev(arg1, "WORLD"))
	{
		cnt = delete_map(ses);

		tintin_printf2(ses, "#MAP DESTROY WORLD: DELETED %d ROOMS.", cnt);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP DESTROY {AREA|WORLD} {<ARGUMENT>}");
	}
}

DO_MAP(map_dig)
{
	char arg3[BUFFER_SIZE];
	int room;
	struct exit_data *exit;
	struct listnode *dir;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP DIG {<DIRECTION>|<VNUM>} {<LOCATION>|NEW}");
		
		return;
	}

	room = (int) tintoi(arg1);

	if (room > 0 && room < ses->map->size)
	{
		if (ses->map->room_list[room] == NULL)
		{
			add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_CREATE);

			create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0} {}", room);
		}
		else
		{
			show_message(ses, LIST_COMMAND, "#MAP DIG {%s}: Room %d already exists.", arg1, room);
		}
		return;
	}

	exit = find_exit(ses, ses->map->in_room, arg1);

	if (exit)
	{
		show_message(ses, LIST_COMMAND, "#MAP DIG: There is already a room in that direction.");
		return;
	}

	if (*arg2 && strcasecmp(arg2, "new"))
	{
		if (is_number(arg2))
		{
			room = get_number(ses, arg2);
		}
		else
		{
			room = find_room(ses, arg2);
		}

		if (room == 0 && !strcasecmp(arg3, "new"))
		{
			room = find_new_room(ses);
		}

		if (room <= 0 || room >= ses->map->size)
		{
			show_error(ses, LIST_COMMAND, "#MAP DIG {%s}: Couldn't find room {%s}.", arg1, arg2);

			return;
		}

		if (ses->map->room_list[room] == NULL)
		{
			add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_CREATE|MAP_UNDO_LINK);

			create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0} {%s}", room, ses->map->search->id ? ses->map->search->id : "");
			create_exit(ses, ses->map->in_room, "{%d} {%s} {%s}", room, arg1, arg1);
		}
		else
		{
			add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_LINK);

			create_exit(ses, ses->map->in_room, "{%d} {%s} {%s}", room, arg1, arg1);
		}
		return;
	}

	room = find_coord(ses, arg1);

	if (room && strcasecmp(arg2, "new"))
	{
		show_message(ses, LIST_COMMAND, "#MAP CREATE LINK %5d {%s}.", room, ses->map->room_list[room]->name);

		add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_LINK);

		create_exit(ses, ses->map->in_room, "{%d} {%s} {%s}", room, arg1, arg1);
	}
	else
	{
		for (room = 1 ; room < ses->map->size ; room++)
		{
			if (ses->map->room_list[room] == NULL)
			{
				break;
			}
		}

		if (room == ses->map->size)
		{
			show_error(ses, LIST_COMMAND, "#MAP DIG: Maximum amount of rooms of %d reached.", ses->map->size);
			
			return;
		}
		add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_CREATE|MAP_UNDO_LINK);

		create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0} {}", room);
		create_exit(ses, ses->map->in_room, "{%d} {%s} {%s}", room, arg1, arg1);
	}

	if ((dir = search_node_list(ses->list[LIST_PATHDIR], arg1)) != NULL)
	{
		if (find_exit(ses, room, dir->arg2) == NULL)
		{
			create_exit(ses, room, "{%d} {%s} {%s}", ses->map->in_room, dir->arg2, dir->arg2);
		}
	}
}

void exit_edit(struct session *ses, struct exit_data *exit, char *arg, char *arg1, char *arg2, char *arg3)
{
	int vnum, dir;

	if (*arg2 == 0)
	{
		tintin_printf2(ses, "      color: %s", str_convert_meta(exit->color, TRUE));
		tintin_printf2(ses, "    command: %s", exit->cmd);
		tintin_printf2(ses, "       data: %s", exit->data);
		tintin_printf2(ses, "destination: %d", tunnel_void(ses, ses->map->in_room, exit->vnum, exit->dir));
		tintin_printf2(ses, "  direction: %d", exit->dir);
		tintin_printf2(ses, "      flags: %d", exit->flags);
		tintin_printf2(ses, "       name: %s", exit->name);
		tintin_printf2(ses, "       vnum: %d", exit->vnum);
		tintin_printf2(ses, "     weight: %.3f", exit->weight);
		tintin_printf2(ses, "      delay: %.3f", exit->delay);
	}
	else if (is_abbrev(arg2, "COLOR"))
	{
		translate_color_names(ses, arg3, arg2);

		RESTRING(exit->color, arg2);

		show_message(ses, LIST_COMMAND, "#MAP %s {%s} : COLOR SET TO {%s}.", arg, arg1, exit->color);
	}
	else if (is_abbrev(arg2, "COMMAND"))
	{
		RESTRING(exit->cmd, arg3);

		show_message(ses, LIST_COMMAND, "#MAP %s {%s} : COMMAND SET TO {%s}.", arg, arg1, exit->cmd);
	}
	else if (is_abbrev(arg2, "DELAY"))
	{
		if (get_number(ses, arg3) < 0.0)
		{
			show_message(ses, LIST_COMMAND, "#MAP %s {%s} : DELAY MUST BE A POSITIVE NUMBER.", arg, arg1);
		}
		else
		{
			exit->delay = (float) get_number(ses, arg3);

			show_message(ses, LIST_COMMAND, "#MAP %s {%s} : DELAY SET TO {%.3f}", arg, arg1, exit->delay);
		}
	}
	else if (is_abbrev(arg2, "DIRECTION"))
	{
		if (is_math(ses, arg3))
		{
			dir = (int) get_number(ses, arg3);
		}
		else if ((dir = get_exit_dir(ses, arg3)) == 0)
		{
			show_error(ses, LIST_COMMAND, "#MAP %s {%s} : DIRECTION {%s} NOT FOUND.", arg, arg1, arg3);
			
			return;
		}

		exit->dir = dir;

		exit->grid = dir_to_grid(exit->dir);

		set_room_exits(ses, ses->map->in_room);

		show_message(ses, LIST_COMMAND, "#MAP %s {%s} : DIRECTION {%s} SET TO {%d}.", arg, arg1, arg3, dir);
	}
	else if (is_abbrev(arg2, "FLAGS"))
	{
		exit->flags = (int) get_number(ses, arg3);

		show_message(ses, LIST_COMMAND, "#MAP %s {%s} : FLAGS SET TO {%d}.", arg, arg1, exit->flags);
	}
	else if (is_abbrev(arg2, "GET"))
	{
		if (*arg3)
		{
			set_nest_node_ses(ses, arg3, "%s", exit->data);
		}
		else
		{
			tintin_printf2(ses, "#MAP %s GET: No destination variable.", arg);
		}
	}
	else if (is_abbrev(arg2, "NAME"))
	{
		RESTRING(exit->name, arg3);

		show_message(ses, LIST_COMMAND, "#MAP %s {%s} : NAME SET TO {%s}.", arg, arg1, exit->name);
	}
	else if (is_abbrev(arg2, "SAVE"))
	{
		if (*arg3)
		{
			set_nest_node_ses(ses, arg3, "{color}{%s}{command}{%s}{delay}{%.3f}{destination}{%d}{dir}{%d}{flags}{%d}{name}{%s}{vnum}{%d}{weight}{%.3f}", exit->color, exit->cmd, exit->delay, tunnel_void(ses, ses->map->in_room, exit->vnum, exit->dir), exit->dir, exit->flags, exit->name, exit->vnum, exit->weight);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#MAP %s SAVE: No destination variable.", arg);
		}
	}
	else if (is_abbrev(arg2, "SET"))
	{
		RESTRING(exit->data, arg3);

		show_message(ses, LIST_COMMAND, "#MAP %s {%s} : DATA SET TO {%s}.", arg, arg1, exit->data);
	}
	else if (is_abbrev(arg2, "VNUM"))
	{
		vnum = atoi(arg3);

		if (vnum <= 0 || vnum >= ses->map->size)
		{
			show_error(ses, LIST_COMMAND, "#MAP %s VNUM: Invalid room vnum: %d.", arg, vnum);
			return;
		}

		if (ses->map->room_list[vnum] == NULL)
		{
			show_error(ses, LIST_COMMAND, "#MAP %s VNUM: Non existant room vnum: %d.", arg, vnum);
			return;
		}
		exit->vnum = vnum;

		show_message(ses, LIST_COMMAND, "#MAP %s {%s} : VNUM SET TO {%s}.", arg, arg1, arg3);
	}
	else if (is_abbrev(arg2, "WEIGHT"))
	{
		if (get_number(ses, arg3) < 0.001)
		{
			show_message(ses, LIST_COMMAND, "#MAP %s {%s} : WEIGHT SHOULD BE AT LEAST 0.001", arg, arg1);
		}
		else
		{
			exit->weight = (float) get_number(ses, arg3);

			show_message(ses, LIST_COMMAND, "#MAP %s {%s} : WEIGHT SET TO {%.3f}", arg, arg1, exit->weight);
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "Syntax: #MAP %s {<NAME>} {COMMAND|DIRECTION|GET|NAME|FLAGS|SAVE|SET|VNUM|WEIGHT} {<argument>}", arg);
	}
}

DO_MAP(map_entrance)
{
	char arg3[BUFFER_SIZE];
	struct exit_data *exit, *rev_exit;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

	exit = find_exit(ses, ses->map->in_room, arg1);

	if (exit == NULL)
	{
		show_message(ses, LIST_COMMAND, "#MAP ENTRANCE: Exit {%s} not found.", arg1);
		
		return;
	}

	rev_exit = ses->map->room_list[exit->vnum]->exit_grid[revdir_to_grid(exit->dir)];

	if (rev_exit == NULL)
	{
		show_message(ses, LIST_COMMAND, "#MAP ENTRANCE {%s}: Exit {%s} has no matching entrance.");

		return;
	}

	exit_edit(ses, rev_exit, "ENTRANCE", arg1, arg2, arg3);
}

DO_MAP(map_exit)
{
	char arg3[BUFFER_SIZE];
	struct exit_data *exit;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

	exit = find_exit(ses, ses->map->in_room, arg1);

	if (exit == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP EXIT: Exit {%s} not found.", arg1);
		
		return;
	}

	exit_edit(ses, exit, "EXIT", arg1, arg2, arg3);
}

DO_MAP(map_exitflag)
{
	struct exit_data *exit;
	char arg3[BUFFER_SIZE], arg4[BUFFER_SIZE];
	int flag;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg4, GET_ONE, SUB_VAR|SUB_FUN);

	exit = find_exit(ses, ses->map->in_room, arg1);

	if (exit == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP EXITFLAG: EXIT {%s} NOT FOUND.", arg1);

		return;
	}

	if (*arg2 == 0)
	{
		tintin_printf2(ses, "#MAP: AVOID FLAG IS SET TO: %s.", HAS_BIT(exit->flags, EXIT_FLAG_AVOID) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: HIDE FLAG IS SET TO: %s.", HAS_BIT(exit->flags, EXIT_FLAG_HIDE) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: INVIS FLAG IS SET TO: %s.", HAS_BIT(exit->flags, EXIT_FLAG_INVIS) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: BLOCK FLAG IS SET TO: %s.", HAS_BIT(exit->flags, EXIT_FLAG_BLOCK) ? "ON" : "OFF");

		return;
	}

	if (is_abbrev(arg2, "AVOID"))
	{
		flag = EXIT_FLAG_AVOID;
	}
	else if (is_abbrev(arg2, "BLOCK"))
	{
		flag = EXIT_FLAG_BLOCK;
	}
	else if (is_abbrev(arg2, "HIDE"))
	{
		flag = EXIT_FLAG_HIDE;
	}
	else if (is_abbrev(arg2, "INVISIBLE"))
	{
		flag = EXIT_FLAG_INVIS;
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP EXITFLAG {%s} <AVOID|BLOCK|HIDE|INVIS> [ON|OFF]", arg1);

		return;
	}

	if (*arg3 == 0)
	{
		TOG_BIT(exit->flags, flag);
	}
	else if (is_abbrev(arg3, "ON"))
	{
		SET_BIT(exit->flags, flag);
	}
	else if (is_abbrev(arg3, "OFF"))
	{
		DEL_BIT(exit->flags, flag);
	}
	else if (is_abbrev(arg3, "GET"))
	{
		if (*arg4 == 0)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX #MAP EXITFLAG {%s} {%s} {GET} {<VARIABLE>}.", arg1, arg2);
		}
		else
		{
			set_nest_node_ses(ses, arg4, "%d", HAS_BIT(exit->flags, flag));
		}
		return;
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP EXITFLAG {%s} {%s} [GET|ON|OFF]", arg1, arg2);

		return;
	}

	if (is_abbrev(arg2, "AVOID"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: AVOID FLAG SET TO %s.", HAS_BIT(exit->flags, EXIT_FLAG_AVOID) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg2, "BLOCK"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: BLOCK FLAG SET TO %s.", HAS_BIT(exit->flags, EXIT_FLAG_BLOCK) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg2, "HIDE"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: HIDE FLAG SET TO %s.", HAS_BIT(exit->flags, EXIT_FLAG_HIDE) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg2, "INVISIBLE"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: INVIS FLAG SET TO %s.", HAS_BIT(exit->flags, EXIT_FLAG_INVIS) ? "ON" : "OFF");
	}
}

DO_MAP(map_explore)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	explore_path(ses, FALSE, arg1, "");
}

DO_MAP(map_find)
{
	shortest_path(ses, FALSE, arg1, arg);
}

DO_MAP(map_flag)
{
	int flag = 0, unflag = 0;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1)
	{
		if (is_abbrev(arg1, "static"))
		{
			flag   = MAP_FLAG_STATIC;
		}
		else if (is_abbrev(arg1, "vtmap"))
		{
			flag   = MAP_FLAG_VTMAP;
		}
		else if (is_abbrev(arg1, "asciigraphics"))
		{
			flag   = MAP_FLAG_ASCIIGRAPHICS;
			unflag = MAP_FLAG_MUDFONT|MAP_FLAG_UNICODEGRAPHICS|MAP_FLAG_BLOCKGRAPHICS;
		}
		else if (is_abbrev(arg1, "asciilength"))
		{
			flag   = MAP_FLAG_ASCIIVNUMS|MAP_FLAG_ASCIILENGTH;
		}
		else if (is_abbrev(arg1, "asciivnums"))
		{
			flag   = MAP_FLAG_ASCIIVNUMS;
			unflag = MAP_FLAG_ASCIILENGTH;
		}
		else if (is_abbrev(arg1, "blockgraphics"))
		{
			flag = MAP_FLAG_BLOCKGRAPHICS;
			unflag = MAP_FLAG_MUDFONT|MAP_FLAG_UNICODEGRAPHICS|MAP_FLAG_ASCIIGRAPHICS;
		}
		else if (is_abbrev(arg1, "direction"))
		{
			flag = MAP_FLAG_DIRECTION;
		}
		else if (is_abbrev(arg1, "mudfont"))
		{
			flag = MAP_FLAG_MUDFONT;
			unflag = MAP_FLAG_ASCIIGRAPHICS|MAP_FLAG_UNICODEGRAPHICS|MAP_FLAG_BLOCKGRAPHICS;
		}
		else if (is_abbrev(arg1, "nofollow"))
		{
			flag = MAP_FLAG_NOFOLLOW;
		}
		else if (is_abbrev(arg1, "quiet"))
		{
			flag = MAP_FLAG_QUIET;
		}
		else if (is_abbrev(arg1, "simplegraphics"))
		{
			unflag = MAP_FLAG_ASCIIVNUMS|MAP_FLAG_SYMBOLGRAPHICS|MAP_FLAG_MUDFONT|MAP_FLAG_ASCIIGRAPHICS|MAP_FLAG_UNICODEGRAPHICS|MAP_FLAG_BLOCKGRAPHICS;
		}
		else if (is_abbrev(arg1, "symbolgraphics"))
		{
			flag = MAP_FLAG_SYMBOLGRAPHICS;
			unflag = MAP_FLAG_ASCIIGRAPHICS|MAP_FLAG_UNICODEGRAPHICS|MAP_FLAG_BLOCKGRAPHICS;
		}
		else if (is_abbrev(arg1, "terrain"))
		{
			flag = MAP_FLAG_TERRAIN;
		}
		else if (is_abbrev(arg1, "unicodegraphics"))
		{
			flag = MAP_FLAG_UNICODEGRAPHICS;
			unflag = MAP_FLAG_ASCIIGRAPHICS|MAP_FLAG_MUDFONT|MAP_FLAG_BLOCKGRAPHICS;
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#MAP: Invalid flag {%s}.", arg1);

			return;
		}
	}
	else
	{
		tintin_printf2(ses, "#MAP: AsciiGraphics flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: AsciiVnums flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: BlockGraphics flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Direction flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_DIRECTION) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: MudFont flag is set to %s", HAS_BIT(ses->map->flags, MAP_FLAG_MUDFONT) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: NoFollow flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_NOFOLLOW) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Quiet flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_QUIET) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: SimpleGraphics flag is set to %s.", !HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS|MAP_FLAG_SYMBOLGRAPHICS|MAP_FLAG_MUDFONT|MAP_FLAG_ASCIIGRAPHICS|MAP_FLAG_UNICODEGRAPHICS|MAP_FLAG_BLOCKGRAPHICS) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Static flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_STATIC) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: SymbolGraphics flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_SYMBOLGRAPHICS) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Terrain flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_TERRAIN) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: UnicodeGraphics flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: VTmap flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP) ? "ON" : "OFF");

		return;
	}

	if (is_abbrev(arg2, "ON"))
	{
		SET_BIT(ses->map->flags, flag);
	}
	else if (is_abbrev(arg2, "OFF"))
	{
		DEL_BIT(ses->map->flags, flag);
	}
	else
	{
		TOG_BIT(ses->map->flags, flag);
	}

	if (unflag)
	{
		DEL_BIT(ses->map->flags, unflag);
	}

	if (is_abbrev(arg1, "asciigraphics"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: AsciiGraphics flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "asciivnums"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: AsciiVnums flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "direction"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Direction flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_DIRECTION) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "mudfont"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: MudFont flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_MUDFONT) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "nofollow"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: NoFollow flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_NOFOLLOW) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "quiet"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Quiet flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_QUIET) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "static"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Static flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_STATIC) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "simplegraphics"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: All graphic modes have been disabled.");
	}
	else if (is_abbrev(arg1, "symbolgraphics"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: SymbolGraphics flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_SYMBOLGRAPHICS) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "terrain"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Terrain flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_TERRAIN) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "unicodegraphics"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: UnicodeGraphics flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "vtmap"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: VTmap flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP) ? "ON" : "OFF");
	}


}

DO_MAP(map_get)
{
	struct room_data *room = ses->map->room_list[ses->map->in_room];
	struct exit_data *exit;
	char exits[BUFFER_SIZE], arg3[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg3)
	{
		if (atoi(arg3) > 0 && atoi(arg3) < ses->map->size)
		{
			room = ses->map->room_list[atoi(arg3)];
		}
		else
		{
			room = NULL;
		}
	}

	if (room == NULL)
	{
		set_nest_node_ses(ses, arg2, "0");
	}
	else if (*arg1 == 0 || *arg2 == 0)
	{
		tintin_printf2(ses, " worldflags: %d", ses->map->flags);
		tintin_printf2(ses, "  worldsize: %d", ses->map->size);
		tintin_printf2(ses, "  direction: %d", ses->map->dir);
		tintin_printf2(ses, "    pathdir: %s", dir_to_exit(ses, ses->map->dir));
		tintin_printf2(ses, "");
		tintin_printf2(ses, "   roomvnum: %d", room->vnum);
		tintin_printf2(ses, "   roomarea: %s", room->area);
		tintin_printf2(ses, "  roomcolor: %s", room->color);
		tintin_printf2(ses, "   roomdata: %s", room->data);
		tintin_printf2(ses, "   roomdesc: %s", room->desc);
		tintin_printf2(ses, "  roomexits: %d", get_room_exits(ses, room->vnum));
		tintin_printf2(ses, "  roomflags: %d", room->flags);
		tintin_printf2(ses, "     roomid: %s", room->id);
		tintin_printf2(ses, "   roomname: %s", room->name);
		tintin_printf2(ses, "   roomnote: %s", room->note);
		tintin_printf2(ses, " roomsymbol: %s", room->symbol);
		tintin_printf2(ses, "roomterrain: %s", room->terrain);
		tintin_printf2(ses, " roomweight: %.3f", room->weight);
	}
	else
	{
		if (is_abbrev(arg1, "all"))
		{
			exits[0] = 0;

			for (exit = room->f_exit ; exit ; exit = exit->next)
			{
				cat_sprintf(exits, "{%s}{%d}", exit->name, exit->vnum);
			}
			set_nest_node_ses(ses, arg2, "{area}{%s}{color}{%s}{data}{%s}{desc}{%s}{direction}{%d}{exits}{%s}{flags}{%d}{id}{%s}{name}{%s}{note}{%s}{pathdir}{%s}{symbol}{%s}{terrain}{%s}{vnum}{%d}{weight}{%.3f}", room->area, room->color, room->data, room->desc, ses->map->dir, exits, room->flags, room->id, room->name, room->note, dir_to_exit(ses, ses->map->dir), room->symbol, room->terrain, room->vnum, room->weight);
		}
		else if (is_abbrev(arg1, "roomarea"))
		{
			set_nest_node_ses(ses, arg2, "%s", room->area);
		}
		else if (is_abbrev(arg1, "roomcolor"))
		{
			set_nest_node_ses(ses, arg2, "%s", room->color);
		}
		else if (is_abbrev(arg1, "roomdata"))
		{
			set_nest_node_ses(ses, arg2, "%s", room->data);
		}
		else if (is_abbrev(arg1, "roomdesc"))
		{
			set_nest_node_ses(ses, arg2, "%s", room->desc);
		}
		else if (is_abbrev(arg1, "roomflags"))
		{
			set_nest_node_ses(ses, arg2, "%d", room->flags);
		}
		else if (is_abbrev(arg1, "roomid"))
		{
			set_nest_node_ses(ses, arg2, "%s", room->id);
		}
		else if (is_abbrev(arg1, "roomname"))
		{
			set_nest_node_ses(ses, arg2, "%s", room->name);
		}
		else if (is_abbrev(arg1, "roomnote"))
		{
			set_nest_node_ses(ses, arg2, "%s", room->note);
		}
		else if (is_abbrev(arg1, "roomsymbol"))
		{
			set_nest_node_ses(ses, arg2, "%s", room->symbol);
		}
		else if (is_abbrev(arg1, "roomterrain"))
		{
			set_nest_node_ses(ses, arg2, "%s", room->terrain);
		}
		else if (is_abbrev(arg1, "roomvnum"))
		{
			set_nest_node_ses(ses, arg2, "%d", room->vnum);
		}
		else if (is_abbrev(arg1, "roomweight"))
		{
			set_nest_node_ses(ses, arg2, "%.3f", room->weight);
		}
		else if (is_abbrev(arg1, "roomexits"))
		{
			exits[0] = 0;

			for (exit = room->f_exit ; exit ; exit = exit->next)
			{
				cat_sprintf(exits, "{%s}{%d}", exit->name, exit->vnum);
			}
			set_nest_node_ses(ses, arg2, "%s", exits);
		}
		else if (is_abbrev(arg1, "worldflags"))
		{
			set_nest_node_ses(ses, arg2, "%d", ses->map->flags);
		}
		else if (is_abbrev(arg1, "worldsize"))
		{
			set_nest_node_ses(ses, arg2, "%d", ses->map->size);
		}
		else if (is_abbrev(arg1, "direction"))
		{
			set_nest_node_ses(ses, arg2, "%d", ses->map->dir);
		}
		else if (is_abbrev(arg1, "pathdir"))
		{
			set_nest_node_ses(ses, arg2, "%s", dir_to_exit(ses, ses->map->dir));
		}
		else
		{
			show_message(ses, LIST_COMMAND, "#MAP GET: unknown option: %s.", arg1);
		}
	}
}

DO_MAP(map_global)
{
	int room;

	sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_message(ses, LIST_COMMAND, "#MAP GLOBAL: GLOBAL ROOM SET TO VNUM %d.", ses->map->global_vnum);
	}
	else if (!strcmp(arg1, "0"))
	{
		ses->map->global_vnum = ses->map->global_exit->vnum = 0;

		show_message(ses, LIST_COMMAND, "#MAP GLOBAL: GLOBAL ROOM SET TO VNUM %d.", 0);
	}
	else
	{
		room = find_room(ses, arg);

		if (room)
		{
			ses->map->global_vnum = ses->map->global_exit->vnum = room;

			show_message(ses, LIST_COMMAND, "#MAP GLOBAL: GLOBAL ROOM SET TO VNUM %d.", room);
		}
		else
		{
			show_message(ses, LIST_COMMAND, "#MAP GLOBAL: COULDN'T FIND ROOM %s.", arg1);
		}
	}
}

DO_MAP(map_goto)
{
	int room;

	room = find_room(ses, arg);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN); // look for dig argument

	if (room == 0 && ses->map->search->vnum > 0 && ses->map->search->vnum < ses->map->size && !strcasecmp(arg2, "dig"))
	{
		room = ses->map->search->vnum;

		create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0} {}", room);
	}

	if (room == 0)
	{
		room = find_room(ses, arg1);
	}

	if (room == 0 && ses->map->search->id && *ses->map->search->id && !strcasecmp(arg2, "dig"))
	{
		room = find_new_room(ses);

		if (room)
		{
			create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0} {%s}", room, ses->map->search->id);
		}
	}

	if (room == 0 && ses->map->in_room)
	{
		room = find_path(ses, arg1);

		if (room == 0)
		{
			show_error(ses, LIST_COMMAND, "#MAP GOTO: COULDN'T FIND ROOM OR EXIT {%s}.", arg1);

			return;
		}
	}

	if (room == 0)
	{
		show_error(ses, LIST_COMMAND, "#MAP GOTO: COULDN'T FIND ROOM %s.", arg1);

		return;
	}
	add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_MOVE);

	goto_room(ses, room);

	show_message(ses, LIST_COMMAND, "#MAP GOTO: MOVED TO ROOM %d {%s}.", room, *ses->map->room_list[room]->name ? ses->map->room_list[room]->name : ses->map->room_list[room]->id);
}

DO_MAP(map_info)
{
	int room, cnt, exits;
	struct exit_data *exit;
	struct room_data *in_room = ses->map->room_list[ses->map->in_room];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (ses->map->in_room)
	{
		exit = find_exit(ses, ses->map->in_room, arg1);

		if (exit)
		{
			tintin_printf2(ses, "    color: %s", str_convert_meta(exit->color, TRUE));
			tintin_printf2(ses, "  command: %s", exit->cmd);
			tintin_printf2(ses, "    delay: %.3f", exit->delay);
			tintin_printf2(ses, "direction: %d", exit->dir);
			tintin_printf2(ses, "    flags: %d", exit->flags);
			tintin_printf2(ses, "  get/set: %s", exit->data);
			tintin_printf2(ses, "   length: %d", get_exit_length(ses, exit));
			tintin_printf2(ses, "     name: %s", exit->name);
			tintin_printf2(ses, "     vnum: %d", exit->vnum);
			tintin_printf2(ses, "   weight: %.3f", exit->weight);

			return;
		}
	}

	for (room = cnt = exits = 0 ; room < ses->map->size ; room++)
	{
		if (ses->map->room_list[room])
		{
			cnt++;

			exits += get_room_exits(ses, room);
		}
	}

	tintin_printf2(ses, " %+16s %-7d %+16s %-7d %+16s %-7d", "Total rooms:", cnt, "Total exits:", exits, "World size:", ses->map->size);
	tintin_printf2(ses, " %+16s %-7d %+16s %-7d %+16s %-7d",  "Direction:", ses->map->dir, "Last room:", ses->map->last_room, "Undo size:", ses->map->undo_size);
	tintin_printf2(ses, "");

	strcpy(arg1, "");
	cat_sprintf(arg1, " %+16s %-7s", "AsciiGraphics:", HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS) ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s", "AsciiVnums:", HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS) ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s", "BlockGraphics:", HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS) ? "on" : "off");
	tintin_puts2(ses, arg1);

	strcpy(arg1, "");
	cat_sprintf(arg1, " %+16s %-7s", "Direction:", HAS_BIT(ses->map->flags, MAP_FLAG_DIRECTION) ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s", "MudFont:", HAS_BIT(ses->map->flags, MAP_FLAG_MUDFONT) ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s", "Nofollow:", HAS_BIT(ses->map->flags, MAP_FLAG_NOFOLLOW) ? "on" : "off");
	tintin_puts2(ses, arg1);

	strcpy(arg1, "");
	cat_sprintf(arg1, " %+16s %-7s", "Static:", HAS_BIT(ses->map->flags, MAP_FLAG_STATIC) ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s", "SymbolGraphics:", HAS_BIT(ses->map->flags, MAP_FLAG_SYMBOLGRAPHICS) ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s", "UnicodeGraphics:", HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS) ? "on" : "off");
	tintin_puts2(ses, arg1);

	strcpy(arg1, "");
	cat_sprintf(arg1, " %+16s %-7s", "Vtmap:", HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP) ? "on" : "off");
	tintin_puts2(ses, arg1);

/*
	strcpy(arg1, "");
	cat_sprintf(arg1, " %+16s %-7s",
	cat_sprintf(arg1, " %+16s %-7s",
	cat_sprintf(arg1, " %+16s %-7s",
	tintin_puts2(ses, arg1);
*/

	tintin_printf2(ses, "");
	tintin_printf2(ses, " %+16s %4d %4d %4d %4d", "Map Offset:", ses->map->sav_top_row, ses->map->sav_top_col, ses->map->sav_bot_row, ses->map->sav_bot_col);
	tintin_printf2(ses, " %+16s %4d %4d %4d %4d", "Current Offset:", ses->map->top_row, ses->map->top_col, ses->map->bot_row, ses->map->bot_col);

	if (ses->map->in_room == 0)
	{
		return;
	}

	tintin_printf2(ses, "");

	tintin_printf2(ses, "%+16s %s", "Room area:",    in_room->area);
	tintin_printf2(ses, "%+16s %s", "Room data:",    in_room->data);
	tintin_printf2(ses, "%+16s %s", "Room desc:",    in_room->desc);
	tintin_printf2(ses, "%+16s %s", "Room id:",      in_room->id);
	tintin_printf2(ses, "%+16s %s", "Room name:",    in_room->name);
	tintin_printf2(ses, "%+16s %s", "Room note:",    in_room->note);
	tintin_printf2(ses, "%+16s %s", "Room color:",   str_convert_meta(in_room->color, TRUE));
	tintin_printf2(ses, "%+16s %s (%d)", "Room terrain:", in_room->terrain, in_room->terrain_index);

	tintin_printf2(ses, "");
	tintin_printf2(ses, " %+16s %-7d %+16s %-7.3f %+16s %-7s", "Room vnum:", ses->map->in_room, "Room weight:", in_room->weight, "Room symbol:", in_room->symbol);

	tintin_printf2(ses, " %+16s %-7d %+16s %-7d %+16s %-7d",   "Center x:", ses->map->center_x, "Center y:", ses->map->center_y, "Center z:", ses->map->center_z);

	tintin_printf2(ses, "");

	strcpy(arg1, "");
	cat_sprintf(arg1, " %+16s %-7s",    "Avoid:", HAS_BIT(in_room->flags, ROOM_FLAG_AVOID)    ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s",   "Curved:", HAS_BIT(in_room->flags, ROOM_FLAG_CURVED)   ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s",     "Hide:", HAS_BIT(in_room->flags, ROOM_FLAG_HIDE)     ? "on" : "off");

	tintin_puts2(ses, arg1);

	strcpy(arg1, "");
	cat_sprintf(arg1, " %+16s %-7s",    "Invis:", HAS_BIT(in_room->flags, ROOM_FLAG_INVIS)    ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s",    "Leave:", HAS_BIT(in_room->flags, ROOM_FLAG_LEAVE)    ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s", "NoGlobal:", HAS_BIT(in_room->flags, ROOM_FLAG_NOGLOBAL) ? "on" : "off");
	tintin_puts2(ses, arg1);

	strcpy(arg1, "");
	cat_sprintf(arg1, " %+16s %-7s",   "Static:", HAS_BIT(in_room->flags, ROOM_FLAG_STATIC)   ? "on" : "off");
	cat_sprintf(arg1, " %+16s %-7s",     "Void:", HAS_BIT(in_room->flags, ROOM_FLAG_VOID)     ? "on" : "off");
	tintin_puts2(ses, arg1);

	tintin_printf2(ses, "");

	for (exit = in_room->f_exit ; exit ; exit = exit->next)
	{
		tintin_printf2(ses, "%+16s %-3s (%3s)   to room: %-5d (%5s)", "Exit:", exit->name, exit->cmd, exit->vnum, ses->map->room_list[exit->vnum]->name);
	}

	tintin_printf2(ses, "");

	for (room = 0 ; room < ses->map->size ; room++)
	{
		if (ses->map->room_list[room])
		{
			for (exit = ses->map->room_list[room]->f_exit ; exit ; exit = exit->next)
			{
				if (exit->vnum == ses->map->in_room)
				{
					tintin_printf2(ses, "%+16s %-3s (%3s) from room: %-5d (%5s)", "Entrance:", exit->name, exit->cmd, room, ses->map->room_list[room]->name);
				}
			}
		}
	}
/*
	for (exit = in_room->f_exit ; exit ; exit = exit->next)
	{
		tintin_printf2(ses, "%+14s %-4s %+14s %5d %+14s %5d %+14s %5s", "Exit name:", exit->name, "vnum:", exit->vnum, "flags:", exit->flags, "command:", exit->cmd);
		tintin_printf2(ses, "%+14s %s", "Exit data:", exit->data);
	}
*/
}

DO_MAP(map_insert)
{
	int room, in_room, to_room;
	struct exit_data *exit;
	struct listnode *dir;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	for (room = 1 ; room < ses->map->size ; room++)
	{
		if (ses->map->room_list[room] == NULL)
		{
			break;
		}
	}

	exit = find_exit(ses, ses->map->in_room, arg1);

	dir = search_node_list(ses->list[LIST_PATHDIR], arg1);

	if (exit == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP INSERT {%s}: There is no room in that direction.", arg1);

		return;
	}

	if (room == ses->map->size)
	{
		show_error(ses, LIST_COMMAND, "#MAP INSERT {%s}: Maximum amount of rooms of %d reached.", arg1, ses->map->size);
		return;
	}

	if (dir == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP INSERT {%s}: Given direction must be a pathdir.", arg1);
		return;
	}

	in_room = ses->map->in_room;
	to_room = exit->vnum;

	add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_INSERT);

	create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0} {}", room);

	create_exit(ses, room, "{%d} {%s} {%s}", to_room, dir->arg1, dir->arg1);

	create_exit(ses, room, "{%d} {%s} {%s}", in_room, dir->arg2, dir->arg2);

	exit->vnum = room;

	if ((exit = find_exit(ses, to_room, dir->arg2)) != NULL)
	{
		exit->vnum = room;
	}

	if (*arg)
	{
		ses->map->in_room = room;
		map_roomflag(ses, arg, arg1, arg2);
		ses->map->in_room = in_room;
	}
	show_message(ses, LIST_COMMAND, "#MAP: Inserted room {%d}.", room);
}

DO_MAP(map_jump)
{
	int room;

	sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	room = find_location(ses, arg1);

	if (room)
	{
		add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_MOVE);

		goto_room(ses, room);

		show_message(ses, LIST_COMMAND, "#MAP JUMP: JUMPED TO ROOM %d {%s}.", room, *ses->map->room_list[room]->name ? ses->map->room_list[room]->name : ses->map->room_list[room]->id);
	}
	else
	{
		show_message(ses, LIST_COMMAND, "#MAP JUMP: Couldn't find a room at {%s}.", arg1);
	}
}

DO_MAP(map_landmark)
{
	struct listroot *root = ses->list[LIST_LANDMARK];
	struct listnode *node;
	char arg3[BUFFER_SIZE], arg4[BUFFER_SIZE];
	int room, i, found;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg4, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 || *arg2 == 0)
	{
		i = bsearch_alpha_list(root, arg1, 0);

		if (i > 0)
		{
			tintin_printf2(ses, "name: %-16s  vnum:%7d  size: %7s desc %s", root->list[i]->arg1, root->list[i]->val32[0], root->list[i]->arg4, root->list[i]->arg3);
		}
		else
		{
			for (found = i = 0 ; i < root->used ; i++)
			{
				if (*arg1 == 0 || match(ses, root->list[i]->arg1, arg1, SUB_NONE))
				{
					tintin_printf2(ses, "name: %-16s  vnum:%7d  size: %7s desc: %s", root->list[i]->arg1, root->list[i]->val32[0], root->list[i]->arg4, root->list[i]->arg3);

					found = TRUE;
				}
			}

			if (found == FALSE)
			{
				show_message(ses, LIST_COMMAND, "#MAP LANDMARK: NO MATCHES FOUND FOR {%s}.", arg1);
			}
		}
	}
	else
	{
		room = (int) get_number(ses, arg2);

		if (room <= 0 || room >= ses->map->size)
		{
			show_error(ses, LIST_COMMAND, "#MAP LANDMARK: INVALID VNUM {%s}.", arg2);
		}
		else
		{
			node = update_node_list(root, arg1, arg2, arg3, arg4);

			node->val32[0] = room;

			show_message(ses, LIST_COMMAND, "#OK. LANDMARK {%s} HAS VNUM {%d} AND IS DESCRIBED AS {%s} WITH SIZE {%s}.", arg1, room, arg3, arg4);
		}
	} 
}

DO_MAP(map_unlandmark)
{
	delete_node_with_wild(ses, LIST_LANDMARK, arg);
}

DO_MAP(map_leave)
{
	if (ses->map->in_room == 0)
	{
		show_error(ses, LIST_COMMAND, "#MAP: You're not currently inside the map.");
	}
	else
	{
		ses->map->last_room = ses->map->in_room;
		ses->map->in_room = 0;

		show_message(ses, LIST_COMMAND, "#MAP: Leaving the map. Use goto or return to return.");

		check_all_events(ses, EVENT_FLAG_MAP, 0, 1, "MAP EXIT MAP", ntos(ses->map->in_room));
	}
}

void map_legend_index(struct session *ses, char *arg, int head, int tail)
{
	char esc[BUFFER_SIZE], raw[BUFFER_SIZE];
	int cnt;

	for (cnt = head ; cnt < tail ; cnt++)
	{
		arg = sub_arg_in_braces(ses, arg, raw, GET_ONE, SUB_NONE);

		substitute(ses, raw, esc, SUB_ESC);

		if (is_number(esc))
		{
			numbertocharacter(ses, esc);
		}
		sprintf(ses->map->legend[cnt],     "%.*s", LEGEND_SIZE - 1, esc);
		sprintf(ses->map->legend_raw[cnt], "%.*s", LEGEND_SIZE - 1, raw);

		if (*arg == 0)
		{
			break;
		}
	}
	return;
}

DO_MAP(map_legend)
{
	char buf[BUFFER_SIZE], arg3[BUFFER_SIZE];
	int group, legend;

	push_call("map_legend(%p,%p,%p,%p)",ses,arg,arg1,arg2);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	strcpy(buf, arg2);

	if (*arg1 == 0)
	{
		for (group = 0 ; map_legend_group_table[group].name ; group++)
		{
			tintin_printf2(ses, " [%-22s] [%-22s] [%3d] [%3d]",
				map_legend_group_table[group].group,
				map_legend_group_table[group].name,
				map_legend_group_table[group].start,
				map_legend_group_table[group].end);
		}
		pop_call();
		return;
	}

	if (is_abbrev(arg1, "RESET"))
	{
		map_legend(ses, "{ASCII} {RESET}", arg1, arg2);
		map_legend(ses, "{NESW} {RESET}", arg1, arg2);
		map_legend(ses, "{MUDFONT BRAILLE TUBE} {RESET}", arg1, arg2);
		map_legend(ses, "{UNICODE GRAPHICS} {RESET}", arg1, arg2);

		pop_call();
		return;
	}

	if (is_math(ses, arg1))
	{
		legend = (int) get_number(ses, arg1);

		if (legend < map_legend_group_table[0].start || legend >= map_legend_group_table[0].end)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP LEGEND {%d - %d} {[SYMBOL]}", map_legend_group_table[0].start,  map_legend_group_table[0].end);
		}
		else if (*arg2 == 0)
		{
			if (strip_vt102_strlen(ses, ses->map->legend[legend]) > 1)
			{
				tintin_printf2(ses, " [%-22s] [%-20s] [%3d] [ %12s ] [ %s]",
					map_legend_table[legend].group,
					map_legend_table[legend].name,
					legend,
					ses->map->legend_raw[legend],
					ses->map->legend[legend]);
			}
			else
			{
				tintin_printf2(ses, " [%-22s] [%-20s] [%3d] [ %12s ] [ %s ]",
					map_legend_table[legend].group,
					map_legend_table[legend].name,
					legend,
					ses->map->legend_raw[legend],
					ses->map->legend[legend]);
			}
		}
		else
		{
			map_legend_index(ses, arg2, legend, legend + 1);
		}
		pop_call();
		return;
	}

	for (group = 0 ; map_legend_group_table[group].name ; group++)
	{
		if (is_abbrev(arg1, map_legend_group_table[group].name))
		{
			break;
		}
	}


	if (!map_legend_group_table[group].name)
	{
		show_error(ses, LIST_COMMAND, "#MAP LEGEND: UNKNOWN LEGEND {%s} TRY:", arg1);

		map_legend(ses, "", arg1, arg2);

		pop_call();
		return;
	}

	if (*arg2 == 0)
	{
		for (legend = map_legend_group_table[group].start ; legend < map_legend_group_table[group].end ; legend++)
		{
			if (strip_vt102_strlen(ses, ses->map->legend[legend]) > 1)
			{
				tintin_printf2(ses, " [%-22s] [%-20s] [%3d] [ %12s ] [ %s]",
					map_legend_table[legend].group,
					map_legend_table[legend].name,
					legend,
					ses->map->legend_raw[legend],
					ses->map->legend[legend]);
			}
			else
			{
				tintin_printf2(ses, " [%-22s] [%-20s] [%3d] [ %12s ] [ %s ]",
					map_legend_table[legend].group,
					map_legend_table[legend].name,
					legend,
					ses->map->legend_raw[legend],
					ses->map->legend[legend]);
			}
		}
		pop_call();
		return;
	}

	if (is_abbrev(arg2, "RESET"))
	{
		map_legend_index(ses, map_legend_group_table[group].reset, map_legend_group_table[group].start, map_legend_group_table[group].end);

		pop_call();
		return;
	}

	for (legend = map_legend_group_table[group].start ; legend < map_legend_group_table[group].end ; legend++)
	{
		if (!strcasecmp(space_out(arg2), space_out(map_legend_table[legend].name)))
		{
			if (*arg3 == 0)
			{
				tintin_printf2(ses, "  [%-22s]  [%-20s]  [%3d]  [ %8s ]  [ %s ]",
					map_legend_group_table[group].name,
					map_legend_table[legend].name,
					legend,
					ses->map->legend_raw[legend],
					ses->map->legend[legend]);
			}
			else
			{
				map_legend_index(ses, arg3, legend, legend + 1);
			}
			break;
		}
	}

	if (legend == map_legend_group_table[group].end)
	{
		for (legend = map_legend_group_table[group].start ; legend < map_legend_group_table[group].end ; legend++)
		{
			if (is_abbrev(space_out(arg2), space_out(map_legend_table[legend].name)))
			{
				if (*arg3 == 0)
				{
					tintin_printf2(ses, "  [%-22s]  [%-20s]  [%3d]  [ %8s ]  [ %s ]",
						map_legend_group_table[group].name,
						map_legend_table[legend].name,
						legend,
						ses->map->legend_raw[legend],
						ses->map->legend[legend]);
				}
				else
				{
					map_legend_index(ses, arg3, legend, legend + 1);
				}
				break;
			}
		}
	}


	if (legend == map_legend_group_table[group].end)
	{
		if (strlen(arg2) > (map_legend_group_table[group].end - map_legend_group_table[group].start) * 2)
		{
			map_legend_index(ses, arg2, map_legend_group_table[group].start, map_legend_group_table[group].end);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP LEGEND {%s} {{arg %d} {arg %d} ... {arg %d} {arg %d}",
				map_legend_group_table[group].group,
				map_legend_group_table[group].start,
				map_legend_group_table[group].start - 1,
				map_legend_group_table[group].end - 1,
				map_legend_group_table[group].end);
		}
	}

	pop_call();
	return;
}

DO_MAP(map_link)
{
	char arg3[BUFFER_SIZE];
	struct listnode *dir;
	struct exit_data *exit;
	int room;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 || *arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP LINK {<DIRECTION>} {<LOCATION>} {BOTH}");
		return;
	}

	room = find_room(ses, arg2);

	if (room == 0)
	{
		show_error(ses, LIST_COMMAND, "#MAP LINK {%s}: Couldn't find room {%s}.", arg1, arg2);
		return;
	}

	exit = find_exit(ses, ses->map->in_room, arg1);

	if (exit)
	{
		delete_exit(ses, ses->map->in_room, exit);
	}

	create_exit(ses, ses->map->in_room, "{%d} {%s} {%s}", room, arg1, arg1);

	if (is_abbrev(arg3, "both"))
	{
		if ((dir = search_node_list(ses->list[LIST_PATHDIR], arg1)) != NULL)
		{
			if (find_exit(ses, room, dir->arg2) == NULL)
			{
				create_exit(ses, room, "{%d} {%s} {%s}", ses->map->in_room, dir->arg2, dir->arg2);
			}
		}
	}
	show_message(ses, LIST_COMMAND, "#MAP LINK: Connected room {%s} to {%s}.", ses->map->room_list[ses->map->in_room]->name, ses->map->room_list[room]->name);
}

DO_MAP(map_list)
{
	struct room_data *room;
	char var[BUFFER_SIZE];
	int vnum;

	map_search_compile(ses, "0", var);

	searchgrid_find(ses, ses->map->in_room, ses->map->search);

	map_search_compile(ses, arg, var);

	if (*var)
	{
		set_nest_node_ses(ses, var, "");
	}

	for (vnum = 0 ; vnum < ses->map->size ; vnum++)
	{
		if (match_room(ses, vnum, ses->map->search))
		{
			room = ses->map->room_list[vnum];

			if (*var)
			{
				add_nest_node_ses(ses, var, "{%d} {{distance}{%.3f}{x}{%d}{y}{%d}{z}{%d}}", room->vnum, ses->map->search->stamp == room->search_stamp ? room->length  : -1, room->x, room->y, room->z);
			}
			else
			{
				if (ses->map->search->stamp == room->search_stamp)
				{
					if (room->w == 0)
					{
						tintin_printf2(ses, "vnum: %5d  dist: %8.3f  x: %4d  y: %4d  z: %4d  name: %s", room->vnum, room->length, room->x, room->y, room->z, room->name);
					}
					else
					{
						tintin_printf2(ses, "vnum: %5d  dist: %8.3f  x: %4s  y: %4s  z: %4s  name: %s", room->vnum, room->length, "?", "?", "?", room->name);
					}
				}
				else
				{
						tintin_printf2(ses, "vnum: %5d  dist: %8.8s  x:    ?  y:    ?  z:    ?  name: %s", room->vnum, "-1", room->name);
				}
			}
		}
	}
}

DO_MAP(map_map)
{
	char arg3[BUFFER_SIZE], arg4[BUFFER_SIZE], tmp[BUFFER_SIZE];
	FILE *logfile = NULL;
	int x, y, line, row;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg4, GET_ALL, SUB_VAR|SUB_FUN);

	push_call("map_map(%p,%p)",ses,arg);

	if (is_math(ses, arg1))
	{
		map_grid_y = get_number(ses, arg1);

		if (map_grid_y <= 0)
		{
			map_grid_y = UMAX(0, get_scroll_rows(ses) + map_grid_y);
		}
	}
	else
	{
		map_grid_y = get_scroll_rows(ses);
	}

	if (is_math(ses, arg2))
	{
		map_grid_x = get_number(ses, arg2);

		if (map_grid_x <= 0)
		{
			map_grid_x = UMAX(0, gtd->screen->cols + map_grid_x);
		}
	}
	else
	{
		map_grid_x = get_scroll_cols(ses);
	}

	if (*arg3)
	{
		switch (*arg3)
		{
			case 'a':
			case 'A':

				strcpy(arg3, "APPEND");

				logfile = fopen(arg4, "a");

				if (logfile)
				{
					loginit(ses, logfile, LOG_FLAG_APPEND | HAS_BIT(ses->logmode, LOG_FLAG_HTML));
				}
				else
				{
					show_error(ses, LIST_COMMAND, "#ERROR: #MAP MAP {%s} {%s} {%s} FAILED TO OPEN FILE {%s}", arg1, arg2, arg3, arg4);
					pop_call();
					return;
				}
				break;

			case 'd':
			case 'D':
				strcpy(arg3, "DRAW");

				if (*arg4 == 0)
				{
					show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP MAP {%s} {%s} {%s} {square}", arg1, arg2, arg3);

					pop_call();
					return;
				}
				break;

			case 'o':
			case 'O':
				strcpy(arg3, "OVERWRITE");

				logfile = fopen(arg4, "w");

				if (logfile)
				{
					loginit(ses, logfile, LOG_FLAG_OVERWRITE | HAS_BIT(ses->logmode, LOG_FLAG_HTML));
				}
				else
				{
					show_error(ses, LIST_COMMAND, "#ERROR: #MAP MAP {%s} {%s} {%s} FAILED TO OPEN FILE {%s}", arg1, arg2, arg3, arg4);
					pop_call();
					return;
				}
				break;

			case 'l':
			case 'L':
				strcpy(arg3, "LIST");
				break;

			case 's':
			case 'S':
				strcpy(arg3, "SAVE");
				strcpy(arg4, "SAVE");
				break;

			case 'v':
			case 'V':
				strcpy(arg3, "VARIABLE");
				break;

			default:
				show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP MAP {rows} {cols} {append|overwrite|list|variable} {name}");
				pop_call();
				return;
		}

		if (*arg4 == 0)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP MAP {%s} {%s} {%s} {name}", arg1, arg2, arg3);
			pop_call();
			return;
		}
	}

	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		map_grid_y = 2 + map_grid_y / 3;
		map_grid_x = 2 + map_grid_x / 6;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS))
	{
		map_grid_y = 2 + (map_grid_y + 1) / 2;
		map_grid_x = 2 + (map_grid_x + 4) / 5;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS))
	{
		map_grid_y = 2 + map_grid_y / 2;
		map_grid_x = 2 + map_grid_x / 5;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_MUDFONT))
	{
		map_grid_y = 2 + map_grid_y / 1;
		map_grid_x = 2 + map_grid_x / 2;
	}
	else
	{
		map_grid_y = 2 + map_grid_y;
		map_grid_x = 2 + map_grid_x;
	}

	if (map_grid_x > ses->map->max_grid_x || map_grid_y > ses->map->max_grid_y)
	{
		if (map_grid_x > ses->map->max_grid_x)
		{
			ses->map->max_grid_x = map_grid_x + 1;
		}
		if (map_grid_y > ses->map->max_grid_y)
		{
			ses->map->max_grid_y = map_grid_y + 1;
		}

		ses->map->grid_rooms = (struct room_data **) realloc(ses->map->grid_rooms, ses->map->max_grid_x * ses->map->max_grid_y * sizeof(struct room_data *));
		ses->map->grid_vnums = (int *) realloc(ses->map->grid_vnums, ses->map->max_grid_x * ses->map->max_grid_y * sizeof(int));
	}

	displaygrid_build(ses, ses->map->in_room, map_grid_x, map_grid_y, 0);

	*arg1 = row = 0;

	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (line = 1 ; line <= 3 ; line++)
			{
				gtd->buf = str_cpy_printf(&gtd->buf, "\e[0m%s", ses->map->color[MAP_COLOR_BACK]);

				for (x = 1 ; x < map_grid_x - 1 ; x++)
				{
					str_cat(&gtd->buf, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));
				}

				str_clone(&gtd->out, gtd->buf);

				substitute(ses, gtd->buf, gtd->out, SUB_COL|SUB_LIT);

				switch (*arg3)
				{
					case 'A':
					case 'O':
						logit(ses, gtd->out, logfile, LOG_FLAG_LINEFEED);
						break;
					
					case 'L':
						cat_sprintf(arg1, "{%02d}{%s}", ++row, gtd->out);
						break;

					case 'S':
						cat_sprintf(arg1, "\n%s", gtd->out);
						break;

					case 'D':
					case 'V':
						cat_sprintf(arg1, "%s\n", gtd->out);
						break;

					default:
						tintin_puts2(ses, gtd->out);
						break;
				}
			}
		}
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS))
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (line = 1 ; line <= 2 ; line++)
			{
				str_cpy_printf(&gtd->buf, "\e[0m%s", ses->map->color[MAP_COLOR_BACK]);

				for (x = 1 ; x < map_grid_x - 1 ; x++)
				{
					if (x == map_grid_x - 2)
					{
						strcpy(tmp, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));

						str_cat_printf(&gtd->buf, "%.*s", string_str_raw_len(ses, tmp, 0, 2), tmp);
					}
					else
					{
						str_cat(&gtd->buf, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));
					}
				}

				str_clone(&gtd->out, gtd->buf);

				substitute(ses, gtd->buf, gtd->out, SUB_COL|SUB_LIT);

				switch (*arg3)
				{
					case 'A':
					case 'O':
						fprintf(logfile, "%s\n", gtd->out);
						break;

					case 'L':
						cat_sprintf(arg1, "{%02d}{%s\e[0m}", ++row, gtd->out);
						break;

					case 'S':
						cat_sprintf(arg1, "\n%s\e[0m", gtd->out);
						break;

					case 'D':
					case 'V':
						cat_sprintf(arg1, "%s\e[0m\n", gtd->out);
						break;

					default:
						tintin_puts2(ses, gtd->out);
						break;
				}
			}
		}
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS))
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (line = 1 ; line <= 2 ; line++)
			{
				str_cpy_printf(&gtd->buf, "\e[0m%s", ses->map->color[MAP_COLOR_BACK]);

				for (x = 1 ; x < map_grid_x - 1 ; x++)
				{
					str_cat(&gtd->buf, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));
				}

				str_clone(&gtd->out, gtd->buf);

				substitute(ses, gtd->buf, gtd->out, SUB_COL|SUB_LIT);

				switch (*arg3)
				{
					case 'A':
					case 'O':
						fprintf(logfile, "%s\n", gtd->out);
						break;

					case 'L':
						cat_sprintf(arg1, "{%02d}{%s\e[0m}", ++row, gtd->out);
						break;

					case 'S':
						cat_sprintf(arg1, "\n%s\e[0m", gtd->out);
						break;

					case 'D':
					case 'V':
						cat_sprintf(arg1, "%s\e[0m\n", gtd->out);
						break;

					default:
						tintin_puts2(ses, gtd->out);
						break;
				}
			}
		}
	}
	else
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			str_cpy_printf(&gtd->buf, "\e[0m%s", ses->map->color[MAP_COLOR_BACK]);

			for (x = 1 ; x < map_grid_x - 1 ; x++)
			{
				str_cat(&gtd->buf, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], 0, x, y));
			}

			str_clone(&gtd->out, gtd->buf);

			substitute(ses, gtd->buf, gtd->out, SUB_COL|SUB_LIT);

			switch (*arg3)
			{
				case 'A':
				case 'O':
					fprintf(logfile, "%s\n", gtd->out);
					break;
				
				case 'L':
					cat_sprintf(arg1, "{%02d}{%s}", ++row, gtd->out);
					break;

				case 'S':
					cat_sprintf(arg1, "\n%s", gtd->out);
					break;

				case 'D':
				case 'V':
					cat_sprintf(arg1, "%s\n", gtd->out);
					break;
				
				default:
					tintin_puts2(ses, gtd->out);
					break;
			}
		}
	}

	switch (*arg3)
	{
		case 'A':
		case 'O':
			fclose(logfile);
			break;

		case 'D':
			command(ses, do_draw, "tile %s {%s}", arg4, arg1);
			break;

		case 'S':
			str_cpy(&gtd->buf, arg1);
			break;

		case 'L':
			set_nest_node_ses(ses, arg4, "%s", arg1);
			break;

		case 'V':
			set_nest_node_ses(ses, arg4, "%s", arg1);
			break;
	}

	pop_call();
	return;
}

DO_MAP(map_move)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	arg = substitute_speedwalk(ses, arg1, arg2);

	ses->map->nofollow++;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

		follow_map(ses, arg1);

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}

	ses->map->nofollow--;
}

DO_MAP(map_name)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	RESTRING(ses->map->room_list[ses->map->in_room]->name, arg1);
}

DO_MAP(map_offset)
{
	char arg3[BUFFER_SIZE], arg4[BUFFER_SIZE];

	if (arg)
	{
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);
		arg = sub_arg_in_braces(ses, arg, arg4, GET_ONE, SUB_VAR|SUB_FUN);

		ses->map->sav_top_row = get_number(ses, arg1);
		ses->map->sav_top_col = get_number(ses, arg2);
		ses->map->sav_bot_row = get_number(ses, arg3);
		ses->map->sav_bot_col = get_number(ses, arg4);
	}

	ses->map->top_row = get_row_index(ses, ses->map->sav_top_row);
	ses->map->top_col = get_col_index(ses, ses->map->sav_top_col);

	ses->map->bot_row = get_row_index(ses, ses->map->sav_bot_row);
	ses->map->bot_col = get_col_index(ses, ses->map->sav_bot_col);

	ses->map->rows = ses->map->bot_row - ses->map->top_row;
	ses->map->cols = ses->map->bot_col - ses->map->top_col;

	if (arg)
	{
		show_message(ses, LIST_COMMAND, "#MAP OFFSET: SQUARE {%d, %d, %d, %d} ROWS {%d} COLS {%d}", ses->map->top_row, ses->map->top_col, ses->map->bot_row, ses->map->bot_col, ses->map->rows, ses->map->cols);
	}
}


DO_MAP(map_read)
{
	FILE *myfile;
	struct room_data *room;
	struct exit_data *exit;
	char buffer[BUFFER_SIZE], file[BUFFER_SIZE], *cptr;
	int line = 1, vnum = 0;

	arg = sub_arg_in_braces(ses, arg, file, GET_ALL, SUB_VAR|SUB_FUN);

	if ((myfile = fopen(file, "r")) == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP: Map file {%s} not found.", file);

		return;
	}

	if (!fgets(buffer, BUFFER_SIZE - 1, myfile))
	{
		show_error(ses, LIST_COMMAND, "#MAP: INVALID READ ON LINE %d. ABORTING READ..", line);

		fclose(myfile);
		
		return;
	}

	// Check for map files edited on Windows systems

	cptr = strchr(buffer, '\r');

	if (cptr)
	{
		*cptr = 0;
	}

	cptr = strchr(buffer, '\n');

	if (cptr)
	{
		*cptr = 0;
	}

	if (buffer[0] != 'C' || buffer[1] != ' ')
	{
		show_error(ses, LIST_COMMAND, "#MAP READ {%s}: INVALID START OF FILE. ABORTING READ..", file);

		fclose(myfile);

		return;
	}

	gtd->level->quiet++;

	if (ses->map == NULL || !HAS_BIT(ses->map->flags, MAP_FLAG_SYNC))
	{
		gtd->level->quiet++;

		create_map(ses, buffer + 2, MAP_FLAG_READ);

		gtd->level->quiet--;
	}
	else
	{
		SET_BIT(ses->map->flags, MAP_FLAG_READ);
	}

	while (fgets(buffer, BUFFER_SIZE - 1, myfile))
	{
		line++;

		cptr = strchr(buffer, '\r'); /* For map files editor on Windows systems. */

		if (cptr)
		{
			*cptr = 0;
		}

		cptr = strchr(buffer, '\n');

		if (cptr)
		{
			*cptr = 0;
		}

		switch (buffer[0])
		{
			case 'C':
				switch (buffer[1])
				{
					case ' ':
						gtd->level->quiet--;

						show_error(ses, LIST_COMMAND, "#MAP: INVALID COMMAND {%d} {%s} ON LINE %d. ABORTING READ..", buffer[0], buffer, line);

						fclose(myfile);

						delete_map(ses);

						return;

					case 'A':
					case 'B':
					case 'E':
					case 'F':
					case 'H':
					case 'I':
					case 'P':
					case 'R':
					case 'S':
					case 'U':
						if (!HAS_BIT(ses->map->flags, MAP_FLAG_SYNC))
						{
							map_color(ses, buffer + 1, arg1, arg2);
						}
						break;

					default:
						show_error(ses, LIST_COMMAND, "#MAP READ: INVALID COMMAND {%d} {%s} ON LINE %d. ABORTING READ..", buffer[0], buffer, line);
						break;
				}
				break;

			case 'E':
				create_exit(ses, vnum, "%s", buffer + 2);
				break;

			case 'F':
				if (!HAS_BIT(ses->map->flags, MAP_FLAG_SYNC))
				{
					ses->map->flags = atoi(buffer + 2) | MAP_FLAG_READ;
				}
				break;

			case 'G':
				if (ses->map->global_vnum == 0)
				{
					ses->map->global_vnum = ses->map->global_exit->vnum = atoi(buffer + 2);
				}
				break;

			case 'I':
				if (ses->map->last_room == 0)
				{
					ses->map->last_room = atoi(buffer + 2);
				}
				break;

			case 'L':
				switch (buffer[1])
				{
					case ' ':
						if (!HAS_BIT(ses->map->flags, MAP_FLAG_SYNC))
						{
							map_legend(ses, buffer + 2, arg1, arg2);
						}
						break;

					case 'M':
						map_landmark(ses, buffer + 3, arg1, arg2);
						break;
				}
				break;

			case 'R':
				room = create_room(ses, "%s", buffer + 2);
				vnum = room->vnum;
				break;

			case 'T':
				if (!HAS_BIT(ses->map->flags, MAP_FLAG_SYNC))
				{
					map_terrain(ses, buffer + 2, arg1, arg2);
				}
				break;

			case 'V':
				if (ses->map->version == 0)
				{
					ses->map->version = atoi(buffer + 2);
				}
				break;

			case '#':
				buffer[0] = gtd->tintin_char;
				ses = script_driver(ses, LIST_COMMAND, buffer);
				break;

			case  0:
			case 13:
				break;

			default:
				gtd->level->quiet--;

				show_error(ses, LIST_COMMAND, "#MAP: INVALID COMMAND {%d} {%s} ON LINE %d.", buffer[0], buffer, line);

				gtd->level->quiet++;
		}
	}

	gtd->level->quiet--;

	DEL_BIT(ses->map->flags, MAP_FLAG_READ);

	fclose(myfile);

	for (vnum = 0 ; vnum < ses->map->size ; vnum++)
	{
		if (ses->map->room_list[vnum] == NULL)
		{
			continue;
		}

		for (exit = ses->map->room_list[vnum]->f_exit ; exit ; exit = exit->next)
		{
			if (exit->vnum < 0 || exit->vnum >= ses->map->size || ses->map->room_list[exit->vnum] == NULL)
			{
				show_error(ses, LIST_COMMAND, "#MAP READ: Room %d - invalid exit '%s' to room %d.", vnum, exit->name, exit->vnum);

				delete_exit(ses, vnum, exit);

				if (ses->map->room_list[vnum]->f_exit)
				{
					exit = ses->map->room_list[vnum]->f_exit;
				}
				else
				{
					break;
				}
			}
		}
	}

	show_message(ses, LIST_COMMAND, "#MAP READ: Map file {%s} loaded.", file);
}

DO_MAP(map_resize)
{
	int size, vnum, room;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	size = atoi(arg1);

	if (size <= ses->map->size)
	{
		for (room = vnum = 1 ; vnum < ses->map->size ; vnum++)
		{
			if (ses->map->room_list[vnum])
			{
				room = vnum;
			}
		}

		if (room >= size)
		{
			show_error(ses, LIST_COMMAND, "#MAP RESIZE: YOU MUST DELETE ALL ROOMS WITH VNUMS ABOVE (%d) FIRST.", size);
			return;
		}
	}

	ses->map->room_list = (struct room_data **) realloc(ses->map->room_list, size * sizeof(struct room_data *));

	if (ses->map->size < size)
	{
		while (ses->map->size < size)
		{
			ses->map->room_list[ses->map->size++] = NULL;
		}
	}
	else
	{
		ses->map->size = size;
	}

	show_message(ses, LIST_COMMAND, "#MAP RESIZE: MAP RESIZED TO %d ROOMS.", ses->map->size);
}

DO_MAP(map_return)
{
	if (ses->map == NULL || ses->map->room_list[ses->map->last_room] == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP RETURN: NO KNOWN LAST ROOM.");

		return;
	}

	if (ses->map->in_room)
	{
		show_error(ses, LIST_COMMAND, "#MAP RETURN: ALREADY IN THE MAP.");
	}
	else
	{
		goto_room(ses, ses->map->last_room);

		show_message(ses, LIST_COMMAND, "#MAP RETURN: RETURNED TO ROOM %d {%s}.", ses->map->in_room, ses->map->room_list[ses->map->in_room]->name);
	}
}

DO_MAP(map_roomflag)
{
	char buf[BUFFER_SIZE], *str, arg3[BUFFER_SIZE];
	int flag = 0;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_printf2(ses, "#MAP: Avoid flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_AVOID) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Block flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_BLOCK) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Curved flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_CURVED) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Fog flag is set to %s.",   HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_FOG) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Hide flag is set to %s.",  HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_HIDE) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Invis flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_INVIS) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Leave flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_LEAVE) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: NoGlobal flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_NOGLOBAL) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Static flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_STATIC) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: Void flag is set to %s.",  HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_VOID) ? "ON" : "OFF");



		return;
	}

	str = arg1;

	while (*str)
	{
		str = get_arg_in_braces(ses, str, buf, GET_ONE);

		if (is_abbrev(buf, "avoid"))
		{
			SET_BIT(flag, ROOM_FLAG_AVOID);
		}
		else if (is_abbrev(buf, "block"))
		{
			SET_BIT(flag, ROOM_FLAG_BLOCK);
		}
		else if (is_abbrev(buf, "curved"))
		{
			SET_BIT(flag, ROOM_FLAG_CURVED);
		}
		else if (is_abbrev(buf, "fog"))
		{
			SET_BIT(flag, ROOM_FLAG_FOG);
		}
		else if (is_abbrev(buf, "hide"))
		{
			SET_BIT(flag, ROOM_FLAG_HIDE);
		}
		else if (is_abbrev(buf, "invisible"))
		{
			SET_BIT(flag, ROOM_FLAG_INVIS);
		}
		else if (is_abbrev(buf, "leave"))
		{
			SET_BIT(flag, ROOM_FLAG_LEAVE);
		}
		else if (is_abbrev(buf, "noglobal"))
		{
			SET_BIT(flag, ROOM_FLAG_NOGLOBAL);
		}
		else if (is_abbrev(buf, "static"))
		{
			SET_BIT(flag, ROOM_FLAG_STATIC);
		}
		else if (is_abbrev(buf, "void"))
		{
			SET_BIT(flag, ROOM_FLAG_VOID);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#MAP: Invalid room flag {%s}.", buf);

			return;
		}

		if (*str == COMMAND_SEPARATOR)
		{
			str++;
		}
	}

	if (*arg2 == 0)
	{
		TOG_BIT(ses->map->room_list[ses->map->in_room]->flags, flag);	
	}
	else if (is_abbrev(arg2, "ON"))
	{
		SET_BIT(ses->map->room_list[ses->map->in_room]->flags, flag);
	}
	else if (is_abbrev(arg2, "OFF"))
	{
		DEL_BIT(ses->map->room_list[ses->map->in_room]->flags, flag);
	}
	else if (is_abbrev(arg2, "GET"))
	{
		if (*arg3 == 0)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX #MAP ROOMFLAG {%s} {GET} {<VARIABLE>}.", buf);
		}
		else
		{
			set_nest_node_ses(ses, arg3, "%d", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, flag));
		}
		return;
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX #MAP ROOMFLAG {%s} {[GET|ON|OFF]}.", buf);
	}


	if (HAS_BIT(flag, ROOM_FLAG_AVOID))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Avoid flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_AVOID) ? "ON" : "OFF");
	}
	if (HAS_BIT(flag, ROOM_FLAG_BLOCK))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Block flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_AVOID) ? "ON" : "OFF");
	}
	if (HAS_BIT(flag, ROOM_FLAG_CURVED))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Curved flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_CURVED) ? "ON" : "OFF");
	}
	if (HAS_BIT(flag, ROOM_FLAG_FOG))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Fog flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_FOG) ? "ON" : "OFF");
	}
	if (HAS_BIT(flag, ROOM_FLAG_HIDE))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Hide flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_HIDE) ? "ON" : "OFF");
	}
	if (HAS_BIT(flag, ROOM_FLAG_INVIS))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Invis flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_INVIS) ? "ON" : "OFF");
	}
	if (HAS_BIT(flag, ROOM_FLAG_LEAVE))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Leave flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_LEAVE) ? "ON" : "OFF");
	}
	if (HAS_BIT(flag, ROOM_FLAG_NOGLOBAL))
	{
		show_message(ses, LIST_COMMAND, "#MAP: NoGlobal flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_NOGLOBAL) ? "ON" : "OFF");
	}
	if (HAS_BIT(flag, ROOM_FLAG_STATIC))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Static flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_STATIC) ? "ON" : "OFF");
	}
	if (HAS_BIT(flag, ROOM_FLAG_VOID))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Void flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_VOID) ? "ON" : "OFF");
	}
}

DO_MAP(map_set)
{
	struct room_data *room = ses->map->room_list[ses->map->in_room];
	char arg3[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg3)
	{
		if (atoi(arg3) > 0 && atoi(arg3) < ses->map->size)
		{
			room = ses->map->room_list[atoi(arg3)];
		}
		else
		{
			room = NULL;
		}
	}

	if (room == NULL)
	{
		show_message(ses, LIST_COMMAND, "#MAP SET: invalid room vnum: %s", arg3);
	}
	else if (*arg1 == 0)
	{
		tintin_printf2(ses, "   roomarea: %s", room->area);
		tintin_printf2(ses, "  roomcolor: %s", room->color);
		tintin_printf2(ses, "   roomdata: %s", room->data);
		tintin_printf2(ses, "   roomdesc: %s", room->desc);
		tintin_printf2(ses, "  roomflags: %d", room->flags);
		tintin_printf2(ses, "     roomid: %s", room->id);
		tintin_printf2(ses, "   roomname: %s", room->name);
		tintin_printf2(ses, "   roomnote: %s", room->note);
		tintin_printf2(ses, " roomsymbol: %s", room->symbol);
		tintin_printf2(ses, "roomterrain: %s", room->terrain);
		tintin_printf2(ses, " roomweight: %.3f", room->weight);
	}
	else
	{
		if (is_abbrev(arg1, "roomarea"))
		{
			RESTRING(room->area, arg2);
			show_message(ses, LIST_COMMAND, "#MAP SET: roomarea set to: %s", room->area);
		}
		else if (is_abbrev(arg1, "roomcolor"))
		{
			RESTRING(room->color, arg2);
			show_message(ses, LIST_COMMAND, "#MAP SET: roomcolor set to: %s", arg2);
		}
		else if (is_abbrev(arg1, "roomdata"))
		{
			RESTRING(room->data, arg2);
			show_message(ses, LIST_COMMAND, "#MAP SET: roomdata set to: %s", arg2);
		}
		else if (is_abbrev(arg1, "roomdesc"))
		{
			arg = arg2;

			while ((arg = strchr(arg, '\n')))
			{
				*arg = ' ';
			}
			RESTRING(room->desc, arg2);
			show_message(ses, LIST_COMMAND, "#MAP SET: roomdesc set to: %s", arg2);
		}
		else if (is_abbrev(arg1, "roomflags"))
		{
			room->flags = (int) get_number(ses, arg2);

			show_message(ses, LIST_COMMAND, "#MAP SET: roomflags set to: %d", room->flags);
		}
		else if (is_abbrev(arg1, "roomid"))
		{
			RESTRING(room->id, arg2);

			show_message(ses, LIST_COMMAND, "#MAP SET: roomid set to: %s", room->id);
		}
		else if (is_abbrev(arg1, "roomname"))
		{
			RESTRING(room->name, arg2);

			show_message(ses, LIST_COMMAND, "#MAP SET: roomname set to: %s", room->name);
		}
		else if (is_abbrev(arg1, "roomnote"))
		{
			RESTRING(room->note, arg2);
			show_message(ses, LIST_COMMAND, "#MAP SET: roomnote set to: %s", arg2);
		}
		else if (is_abbrev(arg1, "roomsymbol"))
		{
			RESTRING(room->symbol, arg2);

			show_message(ses, LIST_COMMAND, "#MAP SET: roomsymbol set to: %s", room->symbol);
		}
		else if (is_abbrev(arg1, "roomterrain"))
		{
			RESTRING(room->terrain, arg2);
			room->terrain_index = bsearch_alpha_list(ses->list[LIST_TERRAIN], room->terrain, 0);
			show_message(ses, LIST_COMMAND, "#MAP SET: roomterrain set to: %s (%d)", arg2, room->terrain_index);
		}
		else if (is_abbrev(arg1, "roomweight"))
		{
			if (get_number(ses, arg2) < 0.001)
			{
				show_message(ses, LIST_COMMAND, "#MAP SET: roomweight should be at least 0.001");
			}
			else
			{
				room->weight = (float) get_number(ses, arg2);

				show_message(ses, LIST_COMMAND, "#MAP SET: roomweight set to: %.3f", room->weight);
			}
		}
		else if (is_abbrev(arg1, "direction"))
		{
			ses->map->dir = URANGE(0, atoi(arg2), 63);
		}
		else if (is_abbrev(arg1, "pathdir"))
		{
			ses->map->dir = exit_to_dir(ses, arg2);
		}
		else
		{
			show_message(ses, LIST_COMMAND, "#MAP SET: unknown option: %s", arg1);
		}
	}
}

DO_MAP(map_sync)
{
	if (ses->map)
	{
		SET_BIT(ses->map->flags, MAP_FLAG_SYNC);
	}

	map_read(ses, arg, arg1, arg2);

	if (ses->map)
	{
		DEL_BIT(ses->map->flags, MAP_FLAG_SYNC);
	}
}

DO_MAP(map_terrain)
{
	struct listroot *root = ses->list[LIST_TERRAIN];
	struct listnode *node;
	struct room_data *room;
	char arg3[BUFFER_SIZE], buf1[BUFFER_SIZE];
	int i, found, flags, density, spread;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, buf1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg2 == 0 || (*arg1 == 0 && *arg2 == 0))
	{
		i = bsearch_alpha_list(root, arg1, 0);

		if (i > 0)
		{
			tintin_printf2(ses, "name: %-16s  index %4d symbol: %-12s  flags: %s %s", root->list[i]->arg1, i, root->list[i]->arg2, root->list[i]->arg3, root->list[i]->arg4);
		}
		else
		{
			for (found = i = 0 ; i < root->used ; i++)
			{
				if (*arg1 == 0 || match(ses, root->list[i]->arg1, arg1, SUB_NONE))
				{
					room = root->list[i]->room;

					if (!HAS_BIT(room->flags, ROOM_FLAG_TERRAIN))
					{
						tintin_printf2(ses, "error: no terrain flag set.");
					}
					tintin_printf2(ses, "name: %-16s  index %4d  symbol: %-16s  %s %s", room->name, room->terrain_index, room->symbol, root->list[i]->arg3, root->list[i]->arg4);

					found = TRUE;
				}
			}

			if (found == FALSE)
			{
				show_message(ses, LIST_COMMAND, "#MAP TERRAIN: NO MATCHES FOUND FOR {%s}.", arg1);
			}
		}
	}
	else
	{
		SET_BIT(ses->map->flags, MAP_FLAG_UPDATETERRAIN);

		density = TERRAIN_FLAG_AMPLE;
		spread = TERRAIN_FLAG_STANDARD;
		flags = 0;

		arg = arg3;

		while (*arg)
		{
			arg = get_arg_in_braces(ses, arg, buf1, GET_ONE);

			if (is_abbrev(buf1, "DENSE"))
			{
				density = TERRAIN_FLAG_DENSE;
			}
			else if (is_abbrev(buf1, "AMPLE"))
			{
				density = TERRAIN_FLAG_AMPLE;
			}
			else if (is_abbrev(buf1, "SPARSE"))
			{
				density = TERRAIN_FLAG_SPARSE;
			}
			else if (is_abbrev(buf1, "SCANT"))
			{
				density = TERRAIN_FLAG_SCANT;
			}
			else if (is_abbrev(buf1, "NARROW"))
			{
				spread = TERRAIN_FLAG_NARROW;
			}
			else if (is_abbrev(buf1, "WIDE"))
			{
				spread = TERRAIN_FLAG_WIDE;
			}
			else if (is_abbrev(buf1, "VAST"))
			{
				spread = TERRAIN_FLAG_WIDE|TERRAIN_FLAG_VAST;
			}
			else if (is_abbrev(buf1, "FADEIN"))
			{
				SET_BIT(flags, TERRAIN_FLAG_FADEIN);
			}
			else if (is_abbrev(buf1, "FADEOUT"))
			{
				SET_BIT(flags, TERRAIN_FLAG_FADEOUT);
			}
			else if (is_abbrev(buf1, "DOUBLE"))
			{
				SET_BIT(flags, TERRAIN_FLAG_DOUBLE);
			}
			else
			{
				show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP TERRAIN {%s} [DENSE|DOUBLE|SPARSE|SCANT|NARROW|WIDE|VAST|FADEIN|FADEOUT]", arg1);
			}

			if (*arg == COMMAND_SEPARATOR)
			{
				arg++;
			}
		}

		strcpy(buf1, "");

		SET_BIT(flags, density);
		SET_BIT(flags, spread);

		if (flags)
		{
			if (HAS_BIT(flags, TERRAIN_FLAG_DENSE))
			{
				strcat(buf1, "DENSE ");
			}
			else if (HAS_BIT(flags, TERRAIN_FLAG_SPARSE))
			{
				strcat(buf1, "SPARSE ");
			}
			else if (HAS_BIT(flags, TERRAIN_FLAG_SCANT))
			{
				strcat(buf1, "SCANT ");
			}

			if (HAS_BIT(flags, TERRAIN_FLAG_NARROW))
			{
				strcat(buf1, "NARROW ");
			}
			else if (HAS_BIT(flags, TERRAIN_FLAG_VAST))
			{
				strcat(buf1, "VAST ");
			}
			else if (HAS_BIT(flags, TERRAIN_FLAG_WIDE))
			{
				strcat(buf1, "WIDE ");
			}

			if (HAS_BIT(flags, TERRAIN_FLAG_FADEIN))
			{
				strcat(buf1, "FADEIN ");
			}
			else if (HAS_BIT(flags, TERRAIN_FLAG_FADEOUT))
			{
				strcat(buf1, "FADEOUT ");
			}

			if (HAS_BIT(flags, TERRAIN_FLAG_DOUBLE))
			{
				strcat(buf1, "DOUBLE ");
			}

			if (*buf1)
			{
				buf1[strlen(buf1) - 1] = 0;
			}
		}

		node = update_node_list(root, arg1, arg2, buf1, "");

		if (node->room)
		{
			if (node->room->symbol)
			{
				RESTRING(node->room->terrain, arg1);
				RESTRING(node->room->symbol, arg2);
			}
		}
		else
		{
			node->room = create_room(ses, "{0} {%d} {} {%s} {%s} {} {} {} {} {} {1.0} {}", ROOM_FLAG_TERRAIN, node->arg1, node->arg2);
		}

		node->room->terrain_flags = flags;

		for (i = 0 ; i < root->used ; i++)
		{
			root->list[i]->room->terrain_index = i;
		}

		show_message(ses, LIST_COMMAND, "#OK. TERRAIN {%s} HAS BEEN SET TO {%s} {%s}.", arg1, arg2, buf1);
	}
}

DO_MAP(map_unterrain)
{
	if (delete_node_with_wild(ses, LIST_TERRAIN, arg))
	{
		SET_BIT(ses->map->flags, MAP_FLAG_UPDATETERRAIN);
	}
}

DO_MAP(map_travel)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	explore_path(ses, TRUE, arg1, arg2);
}


DO_MAP(map_uninsert)
{
	int room1, room2, room3;
	struct exit_data *exit1, *exit2, *exit3;
	struct listnode *dir;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	room1 = ses->map->in_room;
	exit1 = find_exit(ses, room1, arg1);

	dir = search_node_list(ses->list[LIST_PATHDIR], arg1);

	if (exit1 == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNINSERT: There is no room in that direction.");

		return;
	}

	if (dir == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNINSERT: Given direction must be a pathdir.");
		return;
	}

	room2 = exit1->vnum;
	exit2 = find_exit(ses, room2, dir->arg1);

	if (exit2 == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNINSERT: Unable to find backlink room.");
		return;
	}

	room3 = exit2->vnum;
	exit3 = find_exit(ses, room3, dir->arg2);

	if (exit3 == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNINSERT: Unable to find backlink exit.");

		return;
	}

	exit1->vnum = room3;
	exit3->vnum = room1;

	delete_room(ses, room2, TRUE);

	show_message(ses, LIST_COMMAND, "#MAP UNINSERT: Uninserted room {%d}.", room2);
}

// 1) timestamp 2) type 3) data

DO_MAP(map_undo)
{
	struct link_data *link;
	struct room_data *room;
	int undo_flag;
	struct exit_data *exit1, *exit2, *exit3;

	link = ses->map->undo_tail;

	if (link == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNDO: No known last move.");

		return;
	}

	room = ses->map->room_list[atoi(link->str1)];

	if (room == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNDO: Room %s does not exist.", link->str2);
		return;
	}

	if (ses->map->room_list[atoi(link->str2)] == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNDO: Invalid last move.");
		return;
	}

	undo_flag = atoi(link->str3);

	if (HAS_BIT(undo_flag, MAP_UNDO_MOVE))
	{
 		if (ses->map->in_room != room->vnum)
		{
			show_error(ses, LIST_COMMAND, "#MAP UNDO: Invalid last move.");
			return;
		}
		show_message(ses, LIST_COMMAND, "#MAP UNDO: Moving to room %s.", link->str2);

		goto_room(ses, atoi(link->str2));
	}

	if (HAS_BIT(undo_flag, MAP_UNDO_CREATE))
	{
		show_message(ses, LIST_COMMAND, "#MAP UNDO: Deleting room %d.", room->vnum);
		delete_room(ses, room->vnum, TRUE);
	}
	else if (HAS_BIT(undo_flag, MAP_UNDO_LINK))
	{
		exit1 = find_exit_vnum(ses, room->vnum, atoi(link->str2));

		if (exit1)
		{
			show_message(ses, LIST_COMMAND, "#MAP UNDO: Deleting exit leading %s.", exit1->name);
			delete_exit(ses, room->vnum, exit1);
		}
		else
		{
			show_message(ses, LIST_COMMAND, "#MAP UNDO: Could not find exit between %s and %d.", link->str2, room->vnum);
		}

		exit2 = find_exit_vnum(ses, atoi(link->str2), atoi(link->str1));

		if (exit2)
		{
			show_message(ses, LIST_COMMAND, "#MAP UNDO: Deleting exit leading %s.", exit2->name);
			delete_exit(ses, atoi(link->str2), exit2);
		}
		else
		{
			show_message(ses, LIST_COMMAND, "#MAP UNDO: Could not find exit leading %s and %s.", link->str2, link->str1);
		}
	}
	else if (HAS_BIT(undo_flag, MAP_UNDO_INSERT))
	{
		exit1 = find_exit_vnum(ses, atoi(link->str2), atoi(link->str1));

		if (exit1 == NULL)
		{
			show_error(ses, LIST_COMMAND, "#MAP UNDO: Can't find exit between %s and %s.", link->str2, link->str1);
			return;
		}

		exit2 = find_exit(ses, room->vnum, exit1->name);

		if (exit2 == NULL)
		{
			show_error(ses, LIST_COMMAND, "#MAP UNDO: No valid exit found in room %d.", room->vnum);
			return;
		}

		exit3 = find_exit_vnum(ses, exit2->vnum, room->vnum);

		if (exit3 == NULL)
		{
			show_error(ses, LIST_COMMAND, "#MAP UNDO: Can't find exit between %d and %d.", room->vnum, exit2->vnum);
			return;
		}

		exit1->vnum = exit2->vnum;
		exit3->vnum = atoi(link->str2);

		delete_room(ses, room->vnum, TRUE);

		show_message(ses, LIST_COMMAND, "#MAP UNDO: Uninserting room %s.", link->str1);
	}
	del_undo(ses, link);
}

DO_MAP(map_unlink)
{
	struct exit_data *exit1;
	struct exit_data *exit2;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	exit1 = find_exit(ses, ses->map->in_room, arg1);

	if (exit1 == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNLINK: No exit with that name found");

		return;
	}

	if (*arg2 == 'b' || *arg == 'B')
	{
		struct listnode *dir;

		dir = search_node_list(ses->list[LIST_PATHDIR], arg1);

		if (dir)
		{
			exit2 = find_exit(ses, exit1->vnum, dir->arg2);

			if (exit2)
			{
				delete_exit(ses, exit1->vnum, exit2);
			}
		}
	}

	delete_exit(ses, ses->map->in_room, exit1);

	show_message(ses, LIST_COMMAND, "#MAP UNLINK: Exit deleted.");
}

DO_MAP(map_update)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (ses->map == NULL)
	{
		show_message(ses, LIST_COMMAND, "#MAP UPDATE: NO MAP DATA.");
	}
	else if (ses->map->room_list[ses->map->in_room] == NULL)
	{
		show_message(ses, LIST_COMMAND, "#MAP UPDATE: NOT INSIDE MAP.");
	}
	else if (!HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP))
	{
		show_message(ses, LIST_COMMAND, "#MAP UPDATE: VTMAP FLAG NOT SET.");
	}
	else if (ses != gtd->ses)
	{
		show_message(ses, LIST_COMMAND, "#MAP UPDATE: NOT THE ACTIVE SESSION.");
	}
	else
	{
		if (is_abbrev(arg1, "NOW"))
		{
			show_message(ses, LIST_COMMAND, "#MAP UPDATE: MAP UPDATING NOW.");

			DEL_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);

			show_vtmap(ses);
		}
		else
		{
			show_message(ses, LIST_COMMAND, "#MAP UPDATE: MAP SCHEDULED FOR UPDATE.");

			SET_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);
		}
	}
}

DO_MAP(map_run)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	shortest_path(ses, TRUE, arg2, arg1);
}

DO_MAP(map_vnum)
{
	struct listroot *root = ses->list[LIST_LANDMARK];
	int vnum, vnum1, vnum2, old_room, new_room;
	struct exit_data *exit;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	vnum1 = atoi(arg1);

	if (*arg2)
	{
		vnum2 = atoi(arg2);
	}
	else
	{
		vnum2 = vnum1;
	}
	
	if (vnum1 <= 0 || vnum1 >= ses->map->size || vnum2 <= 0 || vnum2 >= ses->map->size)
	{
		show_error(ses, LIST_COMMAND, "#MAP VNUM {%s} {%s} - VNUMS MUST BE BETWEEN {1} and {%d}", arg1, arg2, ses->map->size - 1);
		return;
	}

	for (vnum = vnum1 ; vnum <= vnum2 ; vnum++)
	{
		if (ses->map->room_list[vnum] == NULL)
		{
			break;
		}
	}

	if (vnum > vnum2)
	{
		show_error(ses, LIST_COMMAND, "#MAP VNUM {%s} {%s} - NO FREE VNUM FOUND.", arg1, arg2);
		return;
	}

	old_room = ses->map->in_room;
	new_room = vnum;

	ses->map->room_list[new_room] = ses->map->room_list[old_room];
	ses->map->room_list[new_room]->vnum = new_room;
	ses->map->room_list[old_room] = NULL;
	ses->map->in_room = new_room;

	if (ses->map->at_room == old_room)
	{
		ses->map->at_room = new_room;
	}

	for (vnum = 1 ; vnum < ses->map->size ; vnum++)
	{
		if (ses->map->room_list[vnum] == NULL)
		{
			continue;
		}

		for (exit = ses->map->room_list[vnum]->f_exit ; exit ; exit = exit->next)
		{
			if (exit->vnum == old_room)
			{
				exit->vnum = new_room;
			}
		}
	}

	for (vnum = 0 ; vnum < root->used ; vnum++)
	{
		if (root->list[vnum]->val32[0] == old_room)
		{
			root->list[vnum]->val32[0] = new_room;

			tintin_printf(ses, "#MAP VNUM: MOVED LANDMARK {%s} FROM ROOM %d TO %d.", root->list[vnum]->arg1, old_room, new_room);
		}
	}

	tintin_printf(ses, "#MAP VNUM: MOVED ROOM %d TO %d.", old_room, new_room);
}

DO_MAP(map_write)
{
	struct listroot *root;
	FILE *file;
	struct exit_data *exit;
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP WRITE {<filename>} {FORCE}");

		return;
	}

	if (is_suffix(arg1, ".tin") && !is_abbrev(arg2, "FORCE"))
	{
		show_error(ses, LIST_COMMAND, "#MAP WRITE {%s}: USE {FORCE} TO OVERWRITE .tin FILES.");

		return;
	}

	if ((file = fopen(arg1, "w")) == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP WRITE {%s} - COULDN'T OPEN FILE TO WRITE.", arg1);

		return;
	}

	fprintf(file, "C %d\n\n", ses->map->size);

	fprintf(file, "V 2020\n\n");

	for (index = 0 ; map_color_table[index].name ; index++)
	{
		fprintf(file, "C%s %s\n", map_color_table[index].name, ses->map->color[index]);
	}
	fprintf(file, "\n");

	fprintf(file, "F %d\n\n", ses->map->flags);

	fprintf(file, "G %d\n\n", ses->map->global_vnum);

	fprintf(file, "I %d\n\n", ses->map->in_room ? ses->map->in_room : ses->map->last_room);

	for (index = 0 ; map_legend_table[index].name ; index++)
	{
		fprintf(file, "L {%s} {%s} {%s}\n", map_legend_table[index].group, map_legend_table[index].name, ses->map->legend_raw[index]);
	}
	fprintf(file, "\n\n");

	root = ses->list[LIST_LANDMARK];

	for (index = 0 ; index < root->used ; index++)
	{
		fprintf(file, "LM {%s} {%d} {%s} {%s}\n", root->list[index]->arg1, root->list[index]->val32[0], root->list[index]->arg3, root->list[index]->arg4);
	}
	fprintf(file, "\n\n");	

	root = ses->list[LIST_TERRAIN];

	for (index = 0 ; index < root->used ; index++)
	{
		fprintf(file, "T {%s} {%s} {%s}\n", root->list[index]->arg1, root->list[index]->arg2, root->list[index]->arg3);
	}
	fprintf(file, "\n\n");	

	for (index = 0 ; index < ses->map->size ; index++)
	{
		if (ses->map->room_list[index])
		{
			DEL_BIT(ses->map->room_list[index]->flags, ROOM_FLAG_PATH);

			fprintf(file, "\nR {%5d} {%d} {%s} {%s} {%s} {%s} {%s} {%s} {%s} {%s} {%.3f} {%s}\n",
				ses->map->room_list[index]->vnum,
				ses->map->room_list[index]->flags,
				ses->map->room_list[index]->color,
				ses->map->room_list[index]->name,
				ses->map->room_list[index]->symbol,
				ses->map->room_list[index]->desc,
				ses->map->room_list[index]->area,
				ses->map->room_list[index]->note,
				ses->map->room_list[index]->terrain,
				ses->map->room_list[index]->data,
				ses->map->room_list[index]->weight,
				ses->map->room_list[index]->id);

			for (exit = ses->map->room_list[index]->f_exit ; exit ; exit = exit->next)
			{
				fprintf(file, "E {%5d} {%s} {%s} {%d} {%d} {%s} {%.3f} {%s} {%.2f}\n",
					exit->vnum,
					exit->name,
					exit->cmd,
					exit->dir,
					exit->flags,
					exit->data,
					exit->weight,
					exit->color,
					exit->delay);
			}
		}
	}

	fclose(file);

	show_message(ses, LIST_COMMAND, "#MAP: Map file written to {%s}.", arg1);
}
