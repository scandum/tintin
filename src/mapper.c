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

int dir_flags(struct session *ses, int room, int dir);

DO_COMMAND(do_map)
{
	int cnt;
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (*arg1 == 0)
	{
		tintin_printf2(ses, "Available map options");
		tintin_printf2(ses, "");
		tintin_printf2(ses, "#map at       <location>  <command>    (execute command at given location)");
		tintin_printf2(ses, "#map color    <field>     <color>      (set the color for given field)");
		tintin_printf2(ses, "#map create   [size]                   (creates the initial map)");
		tintin_printf2(ses, "#map destroy                           (destroys the map)");
		tintin_printf2(ses, "#map delete   <direction>              (delete the room at given dir)");
		tintin_printf2(ses, "#map dig      <direction> [new] [vnum] (creates a new room)");
		tintin_printf2(ses, "#map exit     <direction>  <command>   (sets the exit command)");
		tintin_printf2(ses, "#map exitflag <direction> <exit flag>  (set the exit direction)");
		tintin_printf2(ses, "#map explore  <direction>              (saves path to #path)");
		tintin_printf2(ses, "#map info                              (info on map and current room)");
		tintin_printf2(ses, "#map insert   <direction>  [room flag] (insert a new room)");
		tintin_printf2(ses, "#map jump     <x> <y> <z>              (go to given coordinate)");
		tintin_printf2(ses, "#map find     <location> [exits]       (saves path to #path)");
		tintin_printf2(ses, "#map flag     <map flag>               (set map wide flags)");
		tintin_printf2(ses, "#map get      <option>     <variable>  (get various values)");
		tintin_printf2(ses, "#map global   <room vnum>              (sets the global exit room)");
		tintin_printf2(ses, "#map goto     <location> [exits]       (moves you to given room)");
		tintin_printf2(ses, "#map leave                             (leave the map, return with goto)");
		tintin_printf2(ses, "#map legend   <symbols>                (sets the map legend)");
		tintin_printf2(ses, "#map link     <direction>  <room name> (links 2 rooms)");
		tintin_printf2(ses, "#map list     <location>               (shows list of matching rooms)");
		tintin_printf2(ses, "#map map      <radius> <filename>      (shows an ascii map)");
		tintin_printf2(ses, "#map move     <direction>              (move to given direction)");
		tintin_printf2(ses, "#map read     <filename>               (load a map from file)");
		tintin_printf2(ses, "#map resize   <size>                   (resize the maximum size)");
		tintin_printf2(ses, "#map roomflag <room flag>              (set room based flags)");
		tintin_printf2(ses, "#map set      <option>     <value>     (set various values)");
		tintin_printf2(ses, "#map return                            (return to last room.)");
		tintin_printf2(ses, "#map run      <location>   [delay]     (run to given room)");
		tintin_printf2(ses, "#map travel   <direction>  [delay]     (run in given direction)");
		tintin_printf2(ses, "#map undo                              (undo last move)");
		tintin_printf2(ses, "#map uninsert <direction>              (opposite of insert)");
		tintin_printf2(ses, "#map unlink   <direction> [both]       (deletes an exit)");
		tintin_printf2(ses, "#map vnum     <low vnum> [high vnum]   (change room vnum)");
		tintin_printf2(ses, "#map write    <filename>               (save the map)");

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
					show_error(ses, LIST_COMMAND, "#MAP: This session has no map data. Use #map create or #map read to create one.");
					
					return ses;
				}
				if (map_table[cnt].check > 1 && ses->map->room_list[ses->map->in_room] == NULL)
				{
					show_error(ses, LIST_COMMAND, "#MAP: You are not inside the map. Use #map goto to enter it.");

					return ses;
				}
				*arg1 = *arg2 = 0;

				if (gtd->ignore_level == 0)
				{
					if (HAS_BIT(map_table[cnt].flags, MAP_FLAG_VTMAP))
					{
						SET_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);
					}
				}

				push_call("do_map(%s,%p)",map_table[cnt].name, arg);

				map_table[cnt].map (ses, arg, arg1, arg2);

				pop_call();
				return ses;
			}
		}

		do_map(ses, "");
	}
	return ses;
}

DO_MAP(map_at)
{
	int new_room;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_NONE);

	new_room = find_room(ses, arg1);

	if (new_room == 0)
	{
		show_message(ses, LIST_COMMAND, "#MAP AT: Couldn't find room {%s}.", arg1);

		return;
	}

	ses->map->at_room = ses->map->in_room;
	ses->map->in_room = new_room;

	script_driver(ses, LIST_COMMAND, arg2);

	if (ses->map)
	{
		ses->map->in_room = ses->map->at_room;
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
			}

			return;
		}

		for (index = 0 ; map_color_table[index].name ; index++)
		{
			if (is_abbrev(arg1, map_color_table[index].name))
			{
				if (is_abbrev(arg2, "RESET"))
				{
					strncpy(ses->map->color[index], map_color_table[index].code, COLOR_SIZE - 1);
				}
				else
				{
					strncpy(ses->map->color[index], arg2, COLOR_SIZE - 1);
				}
				get_highlight_codes(ses, ses->map->color[index], buf);

				show_message(ses, LIST_COMMAND, "#MAP COLOR %s%10s\e[0m SET TO {%s}", buf, map_color_table[index].name, ses->map->color[index]);

				break;
			}
		}

		if (map_color_table[index].name == NULL)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP COLOR {AVOID|BACKGROUND|EXIT|HIDE|INVIS|PATH|ROOM|USER} {COLOR CODE}");

			return;
		}
		show_message(ses, LIST_COMMAND, "#MAP: %s color set to: %s", arg1, arg2);
	}
	else
	{
		for (index = 0 ; map_color_table[index].name ; index++)
		{
			get_highlight_codes(ses, ses->map->color[index], buf);

			show_message(ses, LIST_COMMAND, "#MAP COLOR %s%10s\e[0m SET TO {%s}", buf, map_color_table[index].name, ses->map->color[index]);
		}
	}
}

DO_MAP(map_create)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	create_map(ses, arg1);

	tintin_printf2(ses, "#MAP: %d room map created, use #map goto 1, to proceed", ses->map->size);
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
			show_error(ses, LIST_COMMAND, "#MAP DELETE {%d} - No room with that vnum found", arg1);

			return;
		}
	}
	else
	{
		exit = find_exit(ses, ses->map->in_room, arg1);

		if (exit)
		{
			room = exit->vnum;
		}

		if (exit == NULL)
		{
			show_error(ses, LIST_COMMAND, "#MAP: No exit with that name found");
			
			return;
		}

		room = exit->vnum;
	}

	if (room == ses->map->in_room)
	{
		show_error(ses, LIST_COMMAND, "#MAP: You must first leave the room you're trying to delete");
		
		return;
	}

	delete_room(ses, room, TRUE);

	show_message(ses, LIST_COMMAND, "#MAP: Room {%d} deleted", room);
}

DO_MAP(map_destroy)
{
	delete_map(ses);

	tintin_printf2(ses, "#MAP: Map destroyed.");
}

DO_MAP(map_dig)
{
	int room;
	struct exit_data *exit;
	struct listnode *node;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP DIG {<DIRECTION>|<VNUM>} {NEW|<VNUM>}");
		
		return;
	}

	room = atoi(arg1);

	if (room > 0 && room < ses->map->size)
	{
		if (ses->map->room_list[room] == NULL)
		{
			add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_CREATE);

			create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0}", room);
		}
		return;
	}

	exit = find_exit(ses, ses->map->in_room, arg1);

	if (exit)
	{
		show_message(ses, LIST_COMMAND, "#MAP DIG: There is already a room in that direction.");
		return;
	}

	if (is_number(arg2))
	{
		room = atoi(arg2);

		if (room <= 0 || room >= ses->map->size)
		{
			show_error(ses, LIST_COMMAND, "#MAP DIG: Invalid room vnum: %d.", room);

			return;
		}

		if (ses->map->room_list[room] == NULL)
		{
			add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_CREATE|MAP_UNDO_LINK);

			create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0}", room);
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

		create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0}", room);
		create_exit(ses, ses->map->in_room, "{%d} {%s} {%s}", room, arg1, arg1);
	}

	if ((node = search_node_list(ses->list[LIST_PATHDIR], arg1)) != NULL)
	{
		if (find_exit(ses, room, node->arg2) == NULL)
		{
			create_exit(ses, room, "{%d} {%s} {%s}", ses->map->in_room, node->arg2, node->arg2);
		}
	}
}

DO_MAP(map_exit)
{
	char arg3[BUFFER_SIZE];
	struct exit_data *exit;
	int room, dir;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

	exit = find_exit(ses, ses->map->in_room, arg1);

	if (exit == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP: Exit {%s} not found.", arg1);
		
		return;
	}

	if (*arg2 == 0)
	{
		tintin_printf2(ses, "  command: %s", exit->cmd);
		tintin_printf2(ses, "direction: %d", exit->dir);
		tintin_printf2(ses, "    flags: %d", exit->flags);
		tintin_printf2(ses, "  get/set: %s", exit->data);
		tintin_printf2(ses, "     name: %s", exit->name);
		tintin_printf2(ses, "     vnum: %d", exit->vnum);
                tintin_printf2(ses, "   weight: %.3f", exit->weight);
	}
	else if (is_abbrev(arg2, "COMMAND"))
	{
		RESTRING(exit->cmd, arg3);

		show_message(ses, LIST_COMMAND, "#MAP EXIT {%s} : COMMAND SET TO {%s}.", arg1, exit->cmd);
	}
	else if (is_abbrev(arg2, "DIRECTION"))
	{
		if (is_math(ses, arg3))
		{
			dir = (int) get_number(ses, arg3);
		}
		else if ((dir = get_exit_dir(ses, arg3)) == 0)
		{
			show_error(ses, LIST_COMMAND, "#MAP EXIT {%s} : DIRECTION {%s} NOT FOUND.", arg1, arg3);
			
			return;
		}

		exit->dir = dir;

		set_room_exits(ses, ses->map->in_room);

		show_message(ses, LIST_COMMAND, "#MAP EXIT {%s} : DIRECTION {%s} SET TO {%d}.", arg1, arg3, dir);
	}
	else if (is_abbrev(arg2, "FLAGS"))
	{
		exit->flags = (int) get_number(ses, arg3);

		show_message(ses, LIST_COMMAND, "#MAP EXIT {%s} : FLAGS SET TO {%d}.", arg1, exit->flags);
	}
	else if (is_abbrev(arg2, "GET"))
	{
		if (*arg3)
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg3, "%s", exit->data);
		}
		else
		{
			tintin_printf2(ses, "#MAP EXIT GET: No destination variable.");
		}
	}
	else if (is_abbrev(arg2, "NAME"))
	{
		RESTRING(exit->name, arg3);

		show_message(ses, LIST_COMMAND, "#MAP EXIT {%s} : NAME SET TO {%s}.", arg1, exit->name);
	}
	else if (is_abbrev(arg2, "SAVE"))
	{
		if (*arg3)
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg3, "{command}{%s}{destination}{%d}{dir}{%d}{flags}{%d}{name}{%s}{vnum}{%d}{weight}{%.3f}", exit->cmd, tunnel_void(ses, ses->map->in_room, exit->vnum, exit->dir), exit->dir, exit->flags, exit->name, exit->vnum, exit->weight);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#MAP EXIT SAVE: No destination variable.");
		}
	}
	else if (is_abbrev(arg2, "SET"))
	{
		RESTRING(exit->data, arg3);

		show_message(ses, LIST_COMMAND, "#MAP EXIT {%s} : DATA SET TO {%s}.", arg1, exit->data);
	}
	else if (is_abbrev(arg2, "VNUM"))
	{
		room = atoi(arg3);

		if (room <= 0 || room >= ses->map->size)
		{
			show_error(ses, LIST_COMMAND, "#MAP EXIT VNUM: Invalid room vnum: %d.", room);
			return;
		}

		if (ses->map->room_list[room] == NULL)
		{
			show_error(ses, LIST_COMMAND, "#MAP EXIT VNUM: Non existant room vnum: %d.", room);
			return;
		}
		exit->vnum = room;

		show_message(ses, LIST_COMMAND, "#MAP EXIT {%s} : VNUM SET TO {%s}.", arg1, arg3);
	}
	else if (is_abbrev(arg2, "WEIGHT"))
	{
		if (get_number(ses, arg3) < 0.001)
		{
			show_message(ses, LIST_COMMAND, "#MAP EXIT {%s} : WEIGHT SHOULD BE AT LEAST 0.001", arg1);
		}
		else
		{
			exit->weight = (float) get_number(ses, arg3);

			show_message(ses, LIST_COMMAND, "#MAP EXIT {%s} : WEIGHT SET TO {%.3f}", arg1, exit->weight);
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "Syntax: #MAP EXIT {<NAME>} {COMMAND|DIRECTION|GET|NAME|FLAGS|SAVE|SET|VNUM|WEIGHT} {<argument>}");
	}
}

DO_MAP(map_exitflag)
{
	struct exit_data *exit;
	char arg3[BUFFER_SIZE];
	int flag;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

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
		tintin_printf2(ses, "#MAP: INVIS FLAG IS SET TO: %s.", HAS_BIT(exit->flags, EXIT_FLAG_INVISIBLE) ? "ON" : "OFF");

		return;
	}

	if (is_abbrev(arg2, "AVOID"))
	{
		flag = EXIT_FLAG_AVOID;
	}
	else if (is_abbrev(arg2, "HIDE"))
	{
		flag = EXIT_FLAG_HIDE;
	}
	else if (is_abbrev(arg2, "INVISIBLE"))
	{
		flag = EXIT_FLAG_INVISIBLE;
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP EXITFLAG {%s} <AVOID|HIDE|INVIS> [ON|OFF]", arg1);

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
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP EXITFLAG {%s} {%s} [ON|OFF]", arg3);
	}

	if (is_abbrev(arg2, "AVOID"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: AVOID FLAG SET TO %s.", HAS_BIT(exit->flags, EXIT_FLAG_AVOID) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg2, "HIDE"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: HIDE FLAG SET TO %s.", HAS_BIT(exit->flags, EXIT_FLAG_HIDE) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg2, "INVISIBLE"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: INVIS FLAG SET TO %s.", HAS_BIT(exit->flags, EXIT_FLAG_HIDE) ? "ON" : "OFF");
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
			flag = MAP_FLAG_STATIC;
		}
		else if (is_abbrev(arg1, "vtmap"))
		{
			flag = MAP_FLAG_VTMAP;
		}
		else if (is_abbrev(arg1, "asciigraphics"))
		{
			flag = MAP_FLAG_ASCIIGRAPHICS;
			unflag = MAP_FLAG_MUDFONT|MAP_FLAG_UNICODEGRAPHICS|MAP_FLAG_BLOCKGRAPHICS;
		}
		else if (is_abbrev(arg1, "asciivnums"))
		{
			flag = MAP_FLAG_ASCIIVNUMS;
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
		else if (is_abbrev(arg1, "symbolgraphics"))
		{
			flag = MAP_FLAG_SYMBOLGRAPHICS;
			unflag = MAP_FLAG_ASCIIGRAPHICS|MAP_FLAG_UNICODEGRAPHICS|MAP_FLAG_BLOCKGRAPHICS;
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
		tintin_printf2(ses, "#MAP: Static flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_STATIC) ? "ON" : "OFF");
		tintin_printf2(ses, "#MAP: SymbolGraphics flag is set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_SYMBOLGRAPHICS) ? "ON" : "OFF");
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
	else if (is_abbrev(arg1, "static"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Static flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_STATIC) ? "ON" : "OFF");
	}
	else if (is_abbrev(arg1, "symbolgraphics"))
	{
		show_message(ses, LIST_COMMAND, "#MAP: SymbolGraphics flag set to %s.", HAS_BIT(ses->map->flags, MAP_FLAG_SYMBOLGRAPHICS) ? "ON" : "OFF");
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
		set_nest_node(ses->list[LIST_VARIABLE], arg2, "0");
	}
	else if (*arg1 == 0 || *arg2 == 0)
	{
		tintin_printf2(ses, " worldflags: %d", ses->map->flags);
		tintin_printf2(ses, "  worldsize: %d", ses->map->size);
		tintin_printf2(ses, "");
		tintin_printf2(ses, "   roomvnum: %d", room->vnum);
		tintin_printf2(ses, "   roomarea: %s", room->area);
		tintin_printf2(ses, "  roomcolor: %s", room->color);
		tintin_printf2(ses, "   roomdata: %s", room->data);
		tintin_printf2(ses, "   roomdesc: %s", room->desc);
		tintin_printf2(ses, "  roomexits: %d", get_room_exits(ses, room->vnum));
		tintin_printf2(ses, "  roomflags: %d", room->flags);
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
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "{area}{%s}{color}{%s}{data}{%s}{desc}{%s}{exits}{%s}{flags}{%d}{name}{%s}{note}{%s}{symbol}{%s}{terrain}{%s}{vnum}{%d}{weight}{%.3f}", room->area, room->color, room->data, room->desc, exits, room->flags, room->name, room->note, room->symbol, room->terrain, room->vnum, room->weight);
		}
		else if (is_abbrev(arg1, "roomarea"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", room->area);
		}
		else if (is_abbrev(arg1, "roomcolor"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", room->color);
		}
		else if (is_abbrev(arg1, "roomdata"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", room->data);
		}
		else if (is_abbrev(arg1, "roomdesc"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", room->desc);
		}
		else if (is_abbrev(arg1, "roomflags"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", room->flags);
		}
		else if (is_abbrev(arg1, "roomname"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", room->name);
		}
		else if (is_abbrev(arg1, "roomnote"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", room->note);
		}
		else if (is_abbrev(arg1, "roomsymbol"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", room->symbol);
		}
		else if (is_abbrev(arg1, "roomterrain"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", room->terrain);
		}
		else if (is_abbrev(arg1, "roomvnum"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", room->vnum);
		}
		else if (is_abbrev(arg1, "roomweight"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%.3f", room->weight);
		}
		else if (is_abbrev(arg1, "roomexits"))
		{
			exits[0] = 0;

			for (exit = room->f_exit ; exit ; exit = exit->next)
			{
				cat_sprintf(exits, "{%s}{%d}", exit->name, exit->vnum);
			}
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%s", exits);
		}
		else if (is_abbrev(arg1, "worldflags"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", ses->map->flags);
		}
		else if (is_abbrev(arg1, "worldsize"))
		{
			set_nest_node(ses->list[LIST_VARIABLE], arg2, "%d", ses->map->size);
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

		create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0}", room);
	}

	if (room)
	{
		add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_MOVE);

		goto_room(ses, room);

		show_message(ses, LIST_COMMAND, "#MAP GOTO: MOVED TO ROOM %d {%s}.", room, ses->map->room_list[room]->name);
	}
	else
	{

		show_message(ses, LIST_COMMAND, "#MAP GOTO: COULDN'T FIND ROOM %s.", arg1);
	}
}

DO_MAP(map_info)
{
	int room, cnt, exits;
	struct exit_data *exit;

	for (room = cnt = exits = 0 ; room < ses->map->size ; room++)
	{
		if (ses->map->room_list[room])
		{
			cnt++;

			exits += get_room_exits(ses, room);
		}
	}

	tintin_printf2(ses, "%+16s %-7d %+16s %-6d %+16s %-6d", "Total rooms:", cnt, "Total exits:", exits, "World size:", ses->map->size);
	tintin_printf2(ses, "%+16s %-7d %+16s %-6d %+16s %-6d",  "Direction:", ses->map->dir, "Last room:", ses->map->last_room, "Undo size:", ses->map->undo_size);

	tintin_printf2(ses, "");

	tintin_printf2(ses, "%+16s %-7s %+16s %-7s %+16s %-7s", "Vtmap:", HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP) ? "ON" : "off", "Static:", HAS_BIT(ses->map->flags, MAP_FLAG_STATIC) ? "ON" : "off", "SymbolGraphics:", HAS_BIT(ses->map->flags, MAP_FLAG_SYMBOLGRAPHICS) ? "ON" : "off");
	tintin_printf2(ses, "%+16s %-7s %+16s %-7s %+16s %-7s", "AsciiGraphics:", HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS) ? "ON" : "off", "Asciivnums:", HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS) ? "ON" : "off", "Nofollow:", HAS_BIT(ses->map->flags, MAP_FLAG_NOFOLLOW) ? "ON" : "off");
	tintin_printf2(ses, "%+16s %-7s %+16s %-7s %+16s %-7s", "UnicodeGraphics:", HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS) ? "ON" : "off", "BlockGraphics", HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS) ? "ON" : "off", "", "");

	if (ses->map->in_room == 0)
	{
		return;
	}

	tintin_printf2(ses, "");

	tintin_printf2(ses, "%+16s %s", "Room area:",    ses->map->room_list[ses->map->in_room]->area);
	tintin_printf2(ses, "%+16s %s", "Room data:",    ses->map->room_list[ses->map->in_room]->data);
	tintin_printf2(ses, "%+16s %s", "Room desc:",    ses->map->room_list[ses->map->in_room]->desc);
	tintin_printf2(ses, "%+16s %s", "Room name:",    ses->map->room_list[ses->map->in_room]->name);
	tintin_printf2(ses, "%+16s %s", "Room note:",    ses->map->room_list[ses->map->in_room]->note);
	tintin_printf2(ses, "%+16s %s", "Room terrain:", ses->map->room_list[ses->map->in_room]->terrain);

	tintin_printf2(ses, "");
	tintin_printf2(ses, "%+16s %-7d %+16s %-7.3f %+16s %-7s", "Room vnum:", ses->map->in_room, "Room weight:", ses->map->room_list[ses->map->in_room]->weight, "Room symbol:", ses->map->room_list[ses->map->in_room]->symbol);
//	tintin_printf2(ses, "%+16s %-7d %+16s %-7d %+16s %-7d",    "Room x:", ses->map->room_list[ses->map->in_room]->x, "Room y:", ses->map->room_list[ses->map->in_room]->y, "Room z:", ses->map->room_list[ses->map->in_room]->z);

	tintin_printf2(ses, "");
	tintin_printf2(ses, "%+16s %-7s %+16s %-7s %+16s %-7s", "Avoid:", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_AVOID) ? "ON" : "off", "Hide:", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_HIDE) ? "ON" : "off", "Leave:",  HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_LEAVE) ? "ON" : "off");
	tintin_printf2(ses, "%+16s %-7s %+16s %-7s",             "Void:", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_VOID) ? "ON" : "off", "Static:", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_STATIC) ? "ON" : "off");

	tintin_printf2(ses, "");

	for (exit = ses->map->room_list[ses->map->in_room]->f_exit ; exit ; exit = exit->next)
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
	for (exit = ses->map->room_list[ses->map->in_room]->f_exit ; exit ; exit = exit->next)
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
	struct listnode *node;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	for (room = 1 ; room < ses->map->size ; room++)
	{
		if (ses->map->room_list[room] == NULL)
		{
			break;
		}
	}

	exit = find_exit(ses, ses->map->in_room, arg1);

	node = search_node_list(ses->list[LIST_PATHDIR], arg1);

	if (exit == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP: There is no room in that direction.");

		return;
	}

	if (room == ses->map->size)
	{
		show_error(ses, LIST_COMMAND, "#MAP: Maximum amount of rooms of %d reached.", ses->map->size);
		return;
	}

	if (node == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP: Given direction must be a pathdir.");
		return;
	}

	in_room = ses->map->in_room;
	to_room = exit->vnum;

	add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_INSERT);

	create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0}", room);

	create_exit(ses, room, "{%d} {%s} {%s}", to_room, node->arg1, node->arg1);

	create_exit(ses, room, "{%d} {%s} {%s}", in_room, node->arg2, node->arg2);

	exit->vnum = room;

	if ((exit = find_exit(ses, to_room, node->arg2)) != NULL)
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
	char arg3[BUFFER_SIZE];
	int room, x, y, z;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	x = get_number(ses, arg1);
	y = get_number(ses, arg2);
	z = get_number(ses, arg3);

	room = spatialgrid_find(ses, ses->map->in_room, x, y, z);

	if (room)
	{
		add_undo(ses, "%d %d %d", room, ses->map->in_room, MAP_UNDO_MOVE);

		goto_room(ses, room);

		show_message(ses, LIST_COMMAND, "#MAP JUMP: JUMPED TO ROOM %d {%s}.", room, ses->map->room_list[room]->name);
	}
	else
	{
		show_message(ses, LIST_COMMAND, "#MAP JUMP: Couldn't find a room at {%d} {%d} {%d}.", x, y, z);
	}
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

		check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "MAP EXIT MAP", ntos(ses->map->in_room));
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
		snprintf(ses->map->legend[cnt],     LEGEND_SIZE - 1, "%s", esc);
		snprintf(ses->map->legend_raw[cnt], LEGEND_SIZE - 1, "%s", raw);

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
		for (group = 0 ; map_group_table[group].name ; group++)
		{
			tintin_printf2(ses, "  [%-22s]  [%-22s]  [%3d]  [%3d]",
				map_group_table[group].group,
				map_group_table[group].name,
				map_group_table[group].start,
				map_group_table[group].end);
		}
		pop_call();
		return;
	}

	if (is_abbrev(arg1, "RESET"))
	{
		map_legend(ses, "{ASCII} {RESET}", arg1, arg2);
		map_legend(ses, "{NESW} {RESET}", arg1, arg2);
		map_legend(ses, "{MUDFONT BRAILLE TUBE} {RESET}", arg1, arg2);

		pop_call();
		return;
	}

	if (is_math(ses, arg1))
	{
		legend = (int) get_number(ses, arg1);

		if (legend < map_group_table[0].start || legend >= map_group_table[0].end)
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP LEGEND {%d - %d} {[SYMBOL]}", map_group_table[0].start,  map_group_table[0].end);
		}
		else if (*arg2 == 0)
		{
			tintin_printf2(ses, "  [%-22s]  [%-20s]  [%3d]  [ %6s ]  [ %s ]",
				map_legend_table[legend].group,
				map_legend_table[legend].name,
				legend,
				ses->map->legend_raw[legend],
				ses->map->legend[legend]);
		}
		else
		{
			map_legend_index(ses, arg2, legend, legend + 1);
		}
		pop_call();
		return;
	}

	for (group = 0 ; map_group_table[group].name ; group++)
	{
		if (is_abbrev(arg1, map_group_table[group].name))
		{
			break;
		}
	}


	if (!map_group_table[group].name)
	{
		show_error(ses, LIST_COMMAND, "#MAP LEGEND: UNKNOWN LEGEND {%s} TRY:", arg1);

		map_legend(ses, "", arg1, arg2);

		pop_call();
		return;
	}

	if (*arg2 == 0)
	{
		for (legend = map_group_table[group].start ; legend < map_group_table[group].end ; legend++)
		{
			tintin_printf2(ses, "  [%-22s]  [%-20s]  [%3d]  [ %6s ]  [ %s ]",
				map_group_table[group].name,
				map_legend_table[legend].name,
				legend,
				ses->map->legend_raw[legend],
				ses->map->legend[legend]);
		}
		pop_call();
		return;
	}

	if (is_abbrev(arg2, "RESET"))
	{
		map_legend_index(ses, map_group_table[group].reset, map_group_table[group].start, map_group_table[group].end);

		pop_call();
		return;
	}

	for (legend = map_group_table[group].start ; legend < map_group_table[group].end ; legend++)
	{
		if (!strcasecmp(space_out(arg2), space_out(map_legend_table[legend].name)))
		{
			if (*arg3 == 0)
			{
				tintin_printf2(ses, "  [%-22s]  [%-20s]  [%3d]  [ %6s ]  [ %s ]",
					map_group_table[group].name,
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

	if (legend == map_group_table[group].end)
	{
		for (legend = map_group_table[group].start ; legend < map_group_table[group].end ; legend++)
		{
			if (is_abbrev(space_out(arg2), space_out(map_legend_table[legend].name)))
			{
				if (*arg3 == 0)
				{
					tintin_printf2(ses, "  [%-22s]  [%-20s]  [%3d]  [ %6s ]  [ %s ]",
						map_group_table[group].name,
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


	if (legend == map_group_table[group].end)
	{
		if (strlen(arg2) > (map_group_table[group].end - map_group_table[group].start) * 2)
		{
			map_legend_index(ses, arg2, map_group_table[group].start, map_group_table[group].end);
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP LEGEND {%s} {{arg %d} {arg %d} ... {arg %d} {arg %d}",
				map_group_table[group].group,
				map_group_table[group].start,
				map_group_table[group].start - 1,
				map_group_table[group].end - 1,
				map_group_table[group].end);
		}
	}

	pop_call();
	return;
}

DO_MAP(map_link)
{
	char arg3[BUFFER_SIZE];
	struct listnode *node;
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
		show_error(ses, LIST_COMMAND, "#MAP: Couldn't find room {%s}.", arg1);
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
		if ((node = search_node_list(ses->list[LIST_PATHDIR], arg1)) != NULL)
		{
			if (find_exit(ses, room, node->arg2) == NULL)
			{
				create_exit(ses, room, "{%d} {%s} {%s}", ses->map->in_room, node->arg2, node->arg2);
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

	set_nest_node(ses->list[LIST_VARIABLE], var, "");

	for (vnum = 0 ; vnum < ses->map->size ; vnum++)
	{
		if (match_room(ses, vnum, ses->map->search))
		{
			room = ses->map->room_list[vnum];

			if (*var)
			{
				add_nest_node(ses->list[LIST_VARIABLE], var, "{%d} {{distance}{%.3f}{x}{%d}{y}{%d}{z}{%d}}", room->vnum, ses->map->search->stamp == room->search_stamp ? room->length  : -1, room->x, room->y, room->z);
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
	char arg3[BUFFER_SIZE], arg4[BUFFER_SIZE];
	FILE *logfile = NULL;
	int x, y, line, row;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg4, GET_ONE, SUB_VAR|SUB_FUN);

	push_call("map_map(%p,%p)",ses,arg);

	if (is_math(ses, arg1))
	{
		map_grid_y = get_number(ses, arg1);

		if (map_grid_y <= 0)
		{
			map_grid_y = UMAX(0, get_scroll_size(ses) + map_grid_y);
		}
	}
	else
	{
		map_grid_y = get_scroll_size(ses);
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
		map_grid_x = gtd->screen->cols;
	}

	if (*arg3)
	{
		switch (*arg3)
		{
			case 'a':
			case 'A':
				strcpy(arg3, "APPEND");

				logfile = fopen(arg4, "a");

				if (HAS_BIT(ses->flags, SES_FLAG_LOGHTML))
				{
					fseek(logfile, 0, SEEK_END);

					if (ftell(logfile) == 0)
					{
						write_html_header(ses, logfile);
					}
				}
				break;

			case 'o':
			case 'O':
				strcpy(arg3, "OVERWRITE");

				logfile = fopen(arg4, "w");

				if (HAS_BIT(ses->flags, SES_FLAG_LOGHTML))
				{
					write_html_header(ses, logfile);
				}
				break;

			case 'l':
			case 'L':
				strcpy(arg3, "LIST");
				break;

			case 'v':
			case 'V':
				strcpy(arg3, "VARIABLE");
				break;

			default:
				show_error(ses, LIST_COMMAND, "#SYNTAX: #MAP MAP {rows} {cols} {append|write|list|variable} {name}");
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
		map_grid_y = 2 + map_grid_y / 2;
		map_grid_x = 2 + map_grid_x / 5;
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
		ses->map->grid_flags =               (int *) realloc(ses->map->grid_flags, ses->map->max_grid_x * ses->map->max_grid_y * sizeof(int));
	}

	displaygrid_build(ses, ses->map->in_room, map_grid_x, map_grid_y, 0);

	*arg1 = row = 0;

	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (line = 1 ; line <= 3 ; line++)
			{
				str_cpy(&gtd->buf, ses->map->color[MAP_COLOR_BACK]);

				for (x = 1 ; x < map_grid_x - 1 ; x++)
				{
					str_cat(&gtd->buf, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));
				}

/*				if (*ses->map->color[MAP_COLOR_BACK] == 0)
				{
					for (x = strlen(buf) - 1 ; x > 0 ; x--)
					{
						if (buf[x] != ' ')
						{
							break;
						}
					}
					buf[x+1] = 0;
					strcat(buf, "<088>");
				}
*/
				str_clone(&gtd->out, gtd->buf);

				substitute(ses, gtd->buf, gtd->out, SUB_COL|SUB_CMP|SUB_LIT);

				if (logfile)
				{
					logit(ses, gtd->out, logfile, TRUE);
				}
				else if (*arg3 == 'L')
				{
					cat_sprintf(arg1, "{%02d}{%s}", ++row, gtd->out);
				}
				else if (*arg3 == 'V')
				{
					cat_sprintf(arg1, "%s\n", gtd->out);
				}
				else
				{
					tintin_puts2(ses, gtd->out);
				}
			}
		}
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS) || HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS))
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (line = 1 ; line <= 2 ; line++)
			{
				str_cpy(&gtd->buf, ses->map->color[MAP_COLOR_BACK]);

				for (x = 1 ; x < map_grid_x - 1 ; x++)
				{
					str_cat(&gtd->buf, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));
				}

/*				if (*ses->map->color[MAP_COLOR_BACK] == 0)
				{
					for (x = strlen(buf) - 1 ; x > 0 ; x--)
					{
						if (buf[x] != ' ')
						{
							break;
						}
					}
					buf[x+1] = 0;
					str_cat(&buf, "<088>");
				}
*/
				str_clone(&gtd->out, gtd->buf);

				substitute(ses, gtd->buf, gtd->out, SUB_COL|SUB_CMP|SUB_LIT);

				if (logfile)
				{
					fprintf(logfile, "%s\n", gtd->out);
				}
				else if (*arg3 == 'L')
				{
					cat_sprintf(arg1, "{%02d}{%s\e[0m}", ++row, gtd->out);
				}
				else if (*arg3 == 'V')
				{
					cat_sprintf(arg1, "%s\e[0m\n", gtd->out);
				}
				else
				{
					tintin_puts2(ses, gtd->out);
				}
			}
		}
	}
	else
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			str_cpy(&gtd->buf, ses->map->color[MAP_COLOR_BACK]);

			for (x = 1 ; x < map_grid_x - 1 ; x++)
			{
				str_cat(&gtd->buf, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], 0, x, y));
			}
/*
			if (*ses->map->color[MAP_COLOR_BACK] == 0)
			{
				for (x = strlen(buf) - 1 ; x > 0 ; x--)
				{
					if (buf[x] != ' ')
					{
						break;
					}
				}
				buf[x+1] = 0;

				strcat(buf, "<088>");
			}
*/
			str_clone(&gtd->out, gtd->buf);

			substitute(ses, gtd->buf, gtd->out, SUB_COL|SUB_CMP|SUB_LIT);

			if (logfile)
			{
				fprintf(logfile, "%s\n", gtd->out);
			}
			else if (*arg3 == 'L')
			{
				cat_sprintf(arg1, "{%02d}{%s}", ++row, gtd->out);
			}
			else if (*arg3 == 'V')
			{
				cat_sprintf(arg1, "%s\n", gtd->out);
			}
			else
			{
				tintin_puts2(ses, gtd->out);
			}
		}
	}

	if (logfile)
	{
		fclose(logfile);
	}
	else if (*arg3 == 'L')
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg4, "%s", arg1);
	}
	else if (*arg3 == 'V')
	{
		set_nest_node(ses->list[LIST_VARIABLE], arg4, "%s", arg1);
	}

	pop_call();
	return;
}

DO_MAP(map_move)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	ses->map->nofollow++;

	follow_map(ses, arg1);

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

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg4, GET_ONE, SUB_VAR|SUB_FUN);

	ses->map->top_row = get_number(ses, arg1);
	ses->map->top_col = get_number(ses, arg2);
	ses->map->bot_row = get_number(ses, arg3);
	ses->map->bot_col = get_number(ses, arg4);

	if (ses->map->top_row == 0)
	{
		ses->map->top_row = 1;
	}
	else if (ses->map->top_row < 0)
	{
		ses->map->top_row = 1 + gtd->screen->rows + ses->map->top_row;
	}

	if (ses->map->top_col == 0)
	{
		ses->map->top_col = 1;
	}
	else if (ses->map->top_col < 0)
	{
		ses->map->top_col = 1 + gtd->screen->cols + ses->map->top_row;
	}

	if (ses->map->bot_row == 0)
	{
		ses->map->bot_row = ses->top_row;
	}
	else if (ses->map->bot_row < 0)
	{
		ses->map->bot_row = 1 + gtd->screen->rows + ses->map->bot_row;
	}

	if (ses->map->bot_col == 0)
	{
		ses->map->bot_col = gtd->screen->cols;
	}
	else if (ses->map->bot_col < 0)
	{
		ses->map->bot_col = 1 + gtd->screen->cols + ses->map->bot_col;
	}

	ses->map->rows = ses->map->bot_row - ses->map->top_row;
	ses->map->cols = ses->map->bot_col - ses->map->top_col;

	show_message(ses, LIST_COMMAND, "#MAP OFFSET: SQUARE {%d, %d, %d, %d} ROWS {%d} COLS {%d}", ses->map->top_row, ses->map->top_col, ses->map->bot_row, ses->map->bot_col, ses->map->rows, ses->map->cols);
}

DO_MAP(map_read)
{
	FILE *myfile;
	struct exit_data *exit;
	char buffer[BUFFER_SIZE], file[BUFFER_SIZE], *cptr;
	int line = 1, room = 0;

	arg = sub_arg_in_braces(ses, arg, file, GET_ALL, SUB_VAR|SUB_FUN);

	if ((myfile = fopen(file, "r")) == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP: Map file {%s} not found.", file);

		return;
	}

	gtd->quiet++;

	if (fgets(buffer, BUFFER_SIZE - 1, myfile))
	{
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

		if (buffer[0] == 'C' && buffer[1] == ' ')
		{
			create_map(ses, buffer + 2);
		}
		else
		{
			gtd->quiet--;

			show_error(ses, LIST_COMMAND, "#MAP READ {%s}: INVALID START OF FILE. ABORTING READ..", file);

			fclose(myfile);

			return;
		}
	}
	else
	{
		gtd->quiet--;

		show_error(ses, LIST_COMMAND, "#MAP: INVALID READ ON LINE %d. ABORTING READ..", line);

		fclose(myfile);
		
		return;
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
						gtd->quiet--;

						show_error(ses, LIST_COMMAND, "#MAP: INVALID COMMAND {%d} {%s} ON LINE %d. ABORTING READ..", buffer[0], buffer, line);

						fclose(myfile);

						delete_map(ses);

						return;

					case 'A':
					case 'B':
					case 'E':
					case 'H':
					case 'I':
					case 'P':
					case 'R':
					case 'S':
					case 'U':
						map_color(ses, buffer + 1, arg1, arg2);
						break;

        	                          default:
						show_error(ses, LIST_COMMAND, "#MAP READ: INVALID COMMAND {%d} {%s} ON LINE %d. ABORTING READ..", buffer[0], buffer, line);
						break;
				}
				break;

			case 'E':
				create_exit(ses, room, "%s", buffer + 2);
				break;

			case 'F':
				ses->map->flags = atoi(buffer + 2);
				break;

			case 'G':
				ses->map->global_vnum = ses->map->global_exit->vnum = atoi(buffer + 2);
				break;

			case 'I':
				ses->map->last_room = atoi(buffer + 2);
				break;

			case 'L':
				map_legend(ses, buffer + 2, arg1, arg2);
				break;

			case 'R':
				room = create_room(ses, "%s", buffer + 2);
				break;

			case 'V':
				ses->map->version = atoi(buffer + 2);
				break;

			case '#':
				buffer[0] = gtd->tintin_char;
				ses = script_driver(ses, LIST_COMMAND, buffer);
				break;

			case  0:
			case 13:
				break;

			default:
				gtd->quiet--;

				show_error(ses, LIST_COMMAND, "#MAP: INVALID COMMAND {%d} {%s} ON LINE %d. ABORTING READ..", buffer[0], buffer, line);

				fclose(myfile);

				delete_map(ses);

				return;
		}
	}

	gtd->quiet--;

	fclose(myfile);

	for (room = 0 ; room < ses->map->size ; room++)
	{
		if (ses->map->room_list[room] == NULL)
		{
			continue;
		}

		for (exit = ses->map->room_list[room]->f_exit ; exit ; exit = exit->next)
		{
			if (exit->vnum < 0 || exit->vnum >= ses->map->size || ses->map->room_list[exit->vnum] == NULL)
			{
				show_error(ses, LIST_COMMAND, "#MAP READ: Room %d - invalid exit '%s' to room %d.", room, exit->name, exit->vnum);

				delete_exit(ses, room, exit);

				if (ses->map->room_list[room]->f_exit)
				{
					exit = ses->map->room_list[room]->f_exit;
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
	char buf[BUFFER_SIZE];
	int flag = 0;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_printf2(ses, "#MAP: Avoid flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_AVOID) ? "ON" : "off");
		tintin_printf2(ses, "#MAP: Hide flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_HIDE) ? "ON" : "off");
		tintin_printf2(ses, "#MAP: Invis flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_INVISIBLE) ? "ON" : "off");
		tintin_printf2(ses, "#MAP: Leave flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_LEAVE) ? "ON" : "off");
		tintin_printf2(ses, "#MAP: Void flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_VOID) ? "ON" : "off");
		tintin_printf2(ses, "#MAP: Static flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_STATIC) ? "ON" : "off");
		tintin_printf2(ses, "#MAP: Curved flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_CURVED) ? "ON" : "off");
		tintin_printf2(ses, "#MAP: NoGlobal flag is set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_NOGLOBAL) ? "ON" : "off");
		return;
	}

	while (*arg1)
	{
		arg1 = get_arg_in_braces(ses, arg1, buf, GET_ONE);

		if (is_abbrev(buf, "avoid"))
		{
			SET_BIT(flag, ROOM_FLAG_AVOID);
		}
		else if (is_abbrev(buf, "curved"))
		{
			SET_BIT(flag, ROOM_FLAG_CURVED);
		}
		else if (is_abbrev(buf, "hide"))
		{
			SET_BIT(flag, ROOM_FLAG_HIDE);
		}
		else if (is_abbrev(buf, "invisible"))
		{
			SET_BIT(flag, ROOM_FLAG_INVISIBLE);
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
		
		if (*arg1 == COMMAND_SEPARATOR) arg1++;
	}

	if (*arg2 == 0)
	{
		TOG_BIT(ses->map->room_list[ses->map->in_room]->flags, flag);	
	}
	if (is_abbrev(arg2, "ON"))
	{
		SET_BIT(ses->map->room_list[ses->map->in_room]->flags, flag);
	}
	else if (is_abbrev(arg2, "OFF"))
	{
		DEL_BIT(ses->map->room_list[ses->map->in_room]->flags, flag);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX #MAP ROOMFLAG {%s} {[ON|OFF]}.", buf);
	}


	if (HAS_BIT(flag, ROOM_FLAG_AVOID))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Avoid flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_AVOID) ? "ON" : "off");
	}
	if (HAS_BIT(flag, ROOM_FLAG_CURVED))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Curved flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_CURVED) ? "ON" : "off");
	}
	if (HAS_BIT(flag, ROOM_FLAG_HIDE))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Hide flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_HIDE) ? "ON" : "off");
	}
	if (HAS_BIT(flag, ROOM_FLAG_INVISIBLE))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Invis flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_INVISIBLE) ? "ON" : "off");
	}
	if (HAS_BIT(flag, ROOM_FLAG_LEAVE))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Leave flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_LEAVE) ? "ON" : "off");
	}
	if (HAS_BIT(flag, ROOM_FLAG_NOGLOBAL))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Global flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_NOGLOBAL) ? "ON" : "off");
	}
	if (HAS_BIT(flag, ROOM_FLAG_VOID))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Void flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_VOID) ? "ON" : "off");
	}
	if (HAS_BIT(flag, ROOM_FLAG_STATIC))
	{
		show_message(ses, LIST_COMMAND, "#MAP: Static flag set to %s.", HAS_BIT(ses->map->room_list[ses->map->in_room]->flags, ROOM_FLAG_STATIC) ? "ON" : "off");
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
			RESTRING(room->desc, arg2);
			show_message(ses, LIST_COMMAND, "#MAP SET: roomdesc set to: %s", arg2);
		}
		else if (is_abbrev(arg1, "roomflags"))
		{
			room->flags = (int) get_number(ses, arg2);

			show_message(ses, LIST_COMMAND, "#MAP SET: roomflags set to: %d", room->flags);
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
			show_message(ses, LIST_COMMAND, "#MAP SET: roomterrain set to: %s", arg2);
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
		else
		{
			show_message(ses, LIST_COMMAND, "#MAP SET: unknown option: %s", arg1);
		}
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
	struct listnode *node;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	room1 = ses->map->in_room;
	exit1 = find_exit(ses, room1, arg1);

	node = search_node_list(ses->list[LIST_PATHDIR], arg1);

	if (exit1 == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNINSERT: There is no room in that direction.");

		return;
	}

	if (node == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNINSERT: Given direction must be a pathdir.");
		return;
	}

	room2 = exit1->vnum;
	exit2 = find_exit(ses, room2, node->arg1);

	if (exit2 == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNINSERT: Unable to find backlink room.");
		return;
	}

	room3 = exit2->vnum;
	exit3 = find_exit(ses, room3, node->arg2);

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
		exit1 = find_exit(ses, room->vnum, link->str2);

		if (exit1)
		{
			show_message(ses, LIST_COMMAND, "#MAP UNDO: Deleting exit leading %s.", exit1->name);
			delete_exit(ses, room->vnum, exit1);
		}

		exit2 = find_exit(ses, atoi(link->str2), link->str1);

		if (exit2)
		{
			show_message(ses, LIST_COMMAND, "#MAP UNDO: Deleting exit leading %s.", exit2->name);
			delete_exit(ses, atoi(link->str2), exit2);
		}
	}
	else if (HAS_BIT(undo_flag, MAP_UNDO_INSERT))
	{
		exit1 = find_exit(ses, atoi(link->str2), link->str1);

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

		exit3 = find_exit(ses, exit2->vnum, ntos(room->vnum));

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
	struct listnode *node;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	node = search_node_list(ses->list[LIST_PATHDIR], arg1);

	exit1 = find_exit(ses, ses->map->in_room, arg1);

	if (exit1 == NULL)
	{
		show_error(ses, LIST_COMMAND, "#MAP UNLINK: No exit with that name found");

		return;
	}

	if (*arg2 == 'b' || *arg == 'B')
	{
		if (node)
		{
			exit2 = find_exit(ses, exit1->vnum, node->arg2);

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
	show_message(ses, LIST_COMMAND, "#MAP UPDATE: OK.");
}

DO_MAP(map_run)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	shortest_path(ses, TRUE, arg2, arg1);
}

DO_MAP(map_vnum)
{
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

	tintin_printf(ses, "#MAP VNUM: MOVED ROOM %d TO %d.", old_room, new_room);
}

DO_MAP(map_write)
{
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

	if (!str_suffix(arg1, ".tin") && !is_abbrev(arg2, "FORCE"))
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

	for (index = 0 ; index < ses->map->size ; index++)
	{
		if (ses->map->room_list[index])
		{
			fprintf(file, "\nR {%5d} {%d} {%s} {%s} {%s} {%s} {%s} {%s} {%s} {%s} {%.3f}\n",
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
				ses->map->room_list[index]->weight);

			for (exit = ses->map->room_list[index]->f_exit ; exit ; exit = exit->next)
			{
				fprintf(file, "E {%5d} {%s} {%s} {%d} {%d} {%s} {%.3f}\n",
					exit->vnum,
					exit->name,
					exit->cmd,
					exit->dir,
					exit->flags,
					exit->data,
					exit->weight);
			}
		}
	}

	fclose(file);

	show_message(ses, LIST_COMMAND, "#MAP: Map file written to {%s}.", arg1);
}

void create_map(struct session *ses, char *arg)
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
	ses->map->grid_flags =             (int *) calloc(ses->map->max_grid_x * ses->map->max_grid_y, sizeof(struct room_data *));

	ses->map->search = (struct search_data *) calloc(1, sizeof(struct search_data));

	ses->map->flags = MAP_FLAG_ASCIIGRAPHICS|MAP_FLAG_DIRECTION;

	ses->map->global_exit         = (struct exit_data *) calloc(1, sizeof(struct exit_data));
		ses->map->global_exit->vnum   = ses->map->global_vnum;
		ses->map->global_exit->name   = restringf(ses->map->global_exit->name, "%cnop global", gtd->tintin_char);
		ses->map->global_exit->cmd    = restringf(ses->map->global_exit->cmd, "%cnop global", gtd->tintin_char);
		ses->map->global_exit->data   = strdup("");
		ses->map->global_exit->weight = 1;

	do_map(ses, "{COLOR} {RESET}");

	ses->map->display_stamp = 1;
	ses->map->search->stamp = 1;

	create_room(ses, "%s", "{1} {0} {} {} { } {} {} {} {} {} {1.0}");

	strcpy(arg, "");

	for (group = 0 ; map_group_table[group].name ; group++)
	{
		for (legend = 0 ; map_legend_table[legend].group ; legend++)
		{
			if (*map_group_table[group].group == 0 || is_abbrev(map_group_table[group].group, map_legend_table[legend].group))
			{
				break;
			}
		}

		if (map_legend_table[legend].group)
		{
			map_group_table[group].start = legend;
		}
		else
		{
			show_error(ses, LIST_COMMAND, "create_map: unknown legend group: %s, %s", map_group_table[group].name, map_group_table[group].group);

			continue;
		}

		while (map_legend_table[++legend].group)
		{
			if (*map_group_table[group].group && !is_abbrev(map_group_table[group].group, map_legend_table[legend].group))
			{
				break;
			}
		}
		map_group_table[group].end = legend;
	}
				
	gtd->quiet++;
	do_map(ses, "LEGEND RESET");
	gtd->quiet--;

	pop_call();
	return;
}

void delete_map(struct session *ses)
{
	int index;

	for (index = 1 ; index < ses->map->size ; index++)
	{
		if (ses->map->room_list[index])
		{
			delete_room(ses, index, FALSE);
		}
	}
	free(ses->map->room_list);

	while (ses->map->undo_head)
	{
		del_undo(ses, ses->map->undo_head);
	}

	free(ses->map->global_exit->name);
	free(ses->map->global_exit->cmd);
	free(ses->map->global_exit->data);
        free(ses->map->global_exit);

	free(ses->map);

	ses->map = NULL;
}

int create_room(struct session *ses, char *format, ...)
{
	char *arg, buf[BUFFER_SIZE];
	struct room_data *newroom;
	va_list args;

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	newroom = (struct room_data *) calloc(1, sizeof(struct room_data));

	arg = buf;

	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->vnum    = atoi(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->flags   = atoi(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->color   = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->name    = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->symbol  = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->desc    = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->area    = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->note    = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->terrain = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->data    = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE); newroom->weight  = atof(buf);

	if (newroom->weight <= 0)
	{
		newroom->weight = 1;
	}

	ses->map->room_list[newroom->vnum] = newroom;

	show_message(ses, LIST_COMMAND, "#MAP CREATE ROOM %5d {%s}.", newroom->vnum, newroom->name);

	return newroom->vnum;
}

void delete_room(struct session *ses, int room, int exits)
{
	struct exit_data *exit, *exit_next;
	int cnt;

	while (ses->map->room_list[room]->f_exit)
	{
		delete_exit(ses, room, ses->map->room_list[room]->f_exit);
	}


	free(ses->map->room_list[room]->color);
	free(ses->map->room_list[room]->name);
	free(ses->map->room_list[room]->symbol);
	free(ses->map->room_list[room]->desc);
	free(ses->map->room_list[room]->area);
	free(ses->map->room_list[room]->note);
	free(ses->map->room_list[room]->terrain);
	free(ses->map->room_list[room]->data); 

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

struct exit_data *create_exit(struct session *ses, int room, char *format, ...)
{
	struct exit_data *newexit;
	va_list args;
	char *arg, buf[BUFFER_SIZE];

	push_call("create_exit(%p,%d,%p)",ses,room,format);

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	newexit = (struct exit_data *) calloc(1, sizeof(struct exit_data));

	arg = buf;

	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);	newexit->vnum   = atoi(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);	newexit->name   = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ALL);	newexit->cmd    = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);	newexit->dir    = atoi(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);	newexit->flags  = atoi(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ALL);	newexit->data   = strdup(buf);
	arg = get_arg_in_braces(ses, arg, buf, GET_ONE);	newexit->weight = atof(buf);

	if (newexit->dir == 0)
	{
		newexit->dir = get_exit_dir(ses, newexit->name);
	}

	if (newexit->weight <= 0)
	{
		newexit->weight = 1;
	}

	LINK(newexit, ses->map->room_list[room]->f_exit, ses->map->room_list[room]->l_exit);

	ses->map->room_list[room]->exit_size++;
	SET_BIT(ses->map->room_list[room]->exit_dirs, (1LL << newexit->dir));

	show_message(ses, LIST_COMMAND, "#MAP CREATE EXIT %5d {%s} {%s}.", newexit->vnum, newexit->name, newexit->cmd);

	pop_call();
	return newexit;
}

void delete_exit(struct session *ses, int room, struct exit_data *exit)
{
	free(exit->name);
	free(exit->cmd);
	free(exit->data);

	UNLINK(exit, ses->map->room_list[room]->f_exit, ses->map->room_list[room]->l_exit)

	set_room_exits(ses, room);

	free(exit);
}

int get_exit_dir(struct session *ses, char *arg)
{
	struct listnode *node;

	node = search_node_list(ses->list[LIST_PATHDIR], arg);

	if (node)
	{
		return atoi(node->arg3);
	}
	else
	{
		return 0;
	}
}

int get_room_exits(struct session *ses, int room)
{
	return ses->map->room_list[room]->exit_size;
}

void set_room_exits(struct session *ses, int room)
{
	struct exit_data *exit;

	ses->map->room_list[room]->exit_dirs = 0;
	ses->map->room_list[room]->exit_size = 0;

	for (exit = ses->map->room_list[room]->f_exit ; exit ; exit = exit->next)
	{
		SET_BIT(ses->map->room_list[room]->exit_dirs, 1LL << exit->dir);

		ses->map->room_list[room]->exit_size++;
	}
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

			check_all_events(ses, SUB_ARG, 0, 5, "MAP FOLLOW MAP", ntos(in_room), ntos(vnum), exit->name);
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

		if (ses->map->nofollow == 0)
		{
			ses->map->nofollow++;

			script_driver(ses, LIST_COMMAND, exit->cmd);

			ses->map->nofollow--;
		}

		vnum = tunnel_void(ses, in_room, exit->vnum, exit->dir);

		check_all_events(ses, SUB_ARG, 0, 5, "MAP FOLLOW MAP", ntos(in_room), ntos(vnum), exit->name);

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
		struct listnode *node;

		if ((node = search_node_list(ses->list[LIST_PATHDIR], argument)) == NULL)
		{
			pop_call();
			return 0;
		}

		in_room = find_coord(ses, argument);

		if (in_room)
		{
			show_message(ses, LIST_COMMAND, "#MAP CREATE LINK %5d {%s}.", in_room, ses->map->room_list[in_room]->name);

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

			create_room(ses, "{%d} {0} {} {} { } {} {} {} {} {} {1.0}", in_room);
		}

		exit = create_exit(ses, ses->map->in_room, "{%d} {%s} {%s}", in_room, node->arg1, node->arg1);

		ses->map->dir = exit->dir;

		if (find_exit(ses, in_room, node->arg2) == NULL)
		{
			create_exit(ses, in_room, "{%d} {%s} {%s}", ses->map->in_room, node->arg2, node->arg2);
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
	char buf[BUFFER_SIZE], *arg, dir[BUFFER_SIZE], rev[BUFFER_SIZE], flag[BUFFER_SIZE];
	va_list args;

	push_call("add_undo(%s,%s)",ses->name, format);

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	arg = get_arg_in_braces(ses, buf, dir, GET_ONE);
	arg = get_arg_in_braces(ses, arg, rev, GET_ONE);
	arg = get_arg_in_braces(ses, arg, flag, GET_ONE);

	link = (struct link_data *) calloc(1, sizeof(struct link_data));

	link->str1 = strdup(dir);
	link->str2 = strdup(rev);
	link->str3 = strdup(flag);

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
	int flags;
	float length;
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
	struct room_data *room;

	push_call("displaygrid_build(%p,%d,%d,%d,%d)",ses,vnum,x,y,z);

	map_grid_x = x;
	map_grid_y = y;

	head = 0;
	tail = 1;

	node = &list[head];

	node->vnum   = vnum;
	node->x      = x / 2;
	node->y      = y / 2;
	node->z      = z / 2;
	node->length = 0;
	node->flags  = 0;

	ses->map->display_stamp++;

	for (vnum = 0 ; vnum < x * y ; vnum++)
	{
		ses->map->grid_rooms[vnum] = NULL;
		ses->map->grid_flags[vnum] = 0;
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
		else if (room->length <= node->length)
		{
			continue;
		}

		room->length = node->length;

		if (node->x >= 0 && node->x < map_grid_x && node->y >= 0 && node->y < map_grid_y && node->z == 0)
		{
			if (ses->map->grid_rooms[node->x + map_grid_x * node->y] == NULL/* || HAS_BIT(ses->map->grid_flags[node->x + map_grid_x * node->y], GRID_FLAG_HIDE)*/)
			{
				ses->map->grid_rooms[node->x + map_grid_x * node->y] = room;
				ses->map->grid_flags[node->x + map_grid_x * node->y] = node->flags;
			}
			else
			{
				continue;
			}
		}
/*
		if (HAS_BIT(node->flags, GRID_FLAG_HIDE))
		{
			continue;
		}
*/
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
			if (HAS_BIT(exit->flags, EXIT_FLAG_INVISIBLE) || HAS_BIT(ses->map->room_list[exit->vnum]->flags, ROOM_FLAG_INVISIBLE))
			{
				SET_BIT(temp->flags, GRID_FLAG_INVISIBLE);
			}

			tail = (tail + 1) % MAP_BF_SIZE;
		}
	}
	pop_call();
	return;
}




void show_vtmap(struct session *ses)
{
	char buf[BUFFER_SIZE], out[BUFFER_SIZE];
	int x, y, line;
	int top_row, top_col, bot_row, bot_col, rows, cols;

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
		bot_row = ses->top_row - 2;
		bot_col = gtd->screen->cols;
		rows    = ses->top_row - 2;
		cols    = gtd->screen->cols;
	}

	printf("\e[%d;%d;%d;%d${", top_row, top_col, bot_row, bot_col);

	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		map_grid_y = 2 + rows / 3;
		map_grid_x = 2 + cols / 6;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS) || HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS))
	{
		map_grid_y = 2 + rows / 2;
		map_grid_x = 2 + cols / 5;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_MUDFONT))
	{
		map_grid_y = 2 + rows;
		map_grid_x = 2 + cols / 2;
	}
	else
	{
		map_grid_y = 2 + rows;
		map_grid_x = 2 + cols;
	}

	displaygrid_build(ses, ses->map->in_room, map_grid_x, map_grid_y, 0);

	save_pos(ses);

	goto_rowcol(ses, top_row, 1);

	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (line = 1 ; line <= 3 ; line++)
			{
				strcpy(buf, ses->map->color[MAP_COLOR_BACK]);

				for (x = 1 ; x < map_grid_x - 1 ; x++)
				{
					strcat(buf, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));
				}
/*
				if (*ses->map->color[MAP_COLOR_BACK] == 0)
				{
					for (x = strlen(buf) - 1 ; x > 0 ; x--)
					{
						if (buf[x] != ' ')
						{
							break;
						}
					}
					buf[x+1] = 0;
				}
*/
				substitute(ses, buf, out, SUB_COL|SUB_CMP|SUB_LIT);

				printf("\e[%dG%s\n", top_col, out);
			}
		}
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS) || HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS))
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			for (line = 1 ; line <= 2 ; line++)
			{
				strcpy(buf, ses->map->color[MAP_COLOR_BACK]);

				for (x = 1 ; x < map_grid_x - 1 ; x++)
				{
					strcat(buf, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], line, x, y));
				}
/*
				if (*ses->map->color[MAP_COLOR_BACK] == 0)
				{
					for (x = strlen(buf) - 1 ; x > 0 ; x--)
					{
						if (buf[x] != ' ')
						{
							break;
						}
					}
					buf[x+1] = 0;
				}
*/
				substitute(ses, buf, out, SUB_COL|SUB_CMP|SUB_LIT);

				printf("\e[%dG%s\n", top_col, out);
			}
		}
	}
	else
	{
		for (y = map_grid_y - 2 ; y >= 1 ; y--)
		{
			strcpy(buf, ses->map->color[MAP_COLOR_BACK]);

			for (x = 1 ; x < map_grid_x - 1 ; x++)
			{
				strcat(buf, draw_room(ses, ses->map->grid_rooms[x + map_grid_x * y], 0, x, y));
			}
/*
			if (*ses->map->color[MAP_COLOR_BACK] == 0)
			{
				for (x = strlen(buf) - 1 ; x > 0 ; x--)
				{
					if (buf[x] != ' ')
					{
						break;
					}
				}
				buf[x+1] = 0;
			}
*/
			substitute(ses, buf, out, SUB_COL|SUB_CMP|SUB_LIT);

			printf("\e[%dG%s\n", top_col, out);
		}
	}

	restore_pos(ses);

	pop_call();
	return;
}

// http://shapecatcher.com/unicode/block/Mathematical_Operators diagonal #⊥⊤#

char *draw_room(struct session *ses, struct room_data *room, int line, int x, int y)
{
	static char buf[201], *room_color, room_left[101], room_right[101];
	int index, flags, exits, exit1, exit2, room1, room2, offset;

	push_call("draw_room(%p,%p,%p,%d,%d)",ses,room,line,x,y);

	offset = HAS_BIT(ses->flags, SES_FLAG_UTF8) ? LEGEND_UNICODE : LEGEND_ASCII;

	if (room)
	{
		if (*room->color)
		{
			room_color = room->color;
		}
		else
		{
			if (HAS_BIT(ses->map->grid_flags[x + map_grid_x * y], GRID_FLAG_INVISIBLE))
			{
				room_color = ses->map->color[MAP_COLOR_INVIS];
			}
			else if (HAS_BIT(room->flags, ROOM_FLAG_INVISIBLE))
			{
				room_color = ses->map->color[MAP_COLOR_INVIS];
			}
/*			else if (HAS_BIT(ses->map->grid_flags[x + map_grid_x * y], GRID_FLAG_HIDE))
			{
				room_color = ses->map->color[MAP_COLOR_HIDE];
			}*/
			else if (HAS_BIT(room->flags, ROOM_FLAG_HIDE))
			{
				room_color = ses->map->color[MAP_COLOR_HIDE];
			}
			else if (HAS_BIT(room->flags, ROOM_FLAG_AVOID))
			{
				room_color = ses->map->color[MAP_COLOR_AVOID];
			}
			else if (HAS_BIT(room->flags, ROOM_FLAG_PATH) && room->search_stamp == ses->map->search->stamp)
			{
				room_color = ses->map->color[MAP_COLOR_PATH];
			}
			else if (HAS_BIT(ses->map->flags, MAP_FLAG_SYMBOLGRAPHICS))
			{
				room_color = ses->map->color[MAP_COLOR_SYMBOL];
			}
			else
			{
				room_color = ses->map->color[MAP_COLOR_ROOM];
			}
		}

		if (HAS_BIT(room->flags, ROOM_FLAG_CURVED))
		{
			sprintf(room_left, "%s(", room_color);
			sprintf(room_right, "%s)", room_color);
		}
		else
		{
			sprintf(room_left, "%s[", room_color);
			sprintf(room_right, "%s]", room_color);
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
						case MAP_EXIT_W+MAP_EXIT_S:
							index = 29;
							break;
						case MAP_EXIT_W:
							index = 30;
							break;
						case MAP_EXIT_W+MAP_EXIT_N:
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
		}
	}

	if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS))
	{
		struct room_data *room_n, *room_nw, *room_w;
		long long exit_n, exit_nw, exit_w;

		room_n  = ses->map->grid_rooms[x + 0 + map_grid_x * (y + 1)];
		room_nw = ses->map->grid_rooms[x - 1 + map_grid_x * (y + 1)];
		room_w  = ses->map->grid_rooms[x - 1 + map_grid_x * (y + 0)];

		exit_n = exit_nw = exit_w = 0;

		if (room && HAS_BIT(room->exit_dirs, MAP_DIR_N))
		{
			SET_BIT(exit_n, MAP_DIR_N);
		}
		if (room_n && HAS_BIT(room_n->exit_dirs, MAP_DIR_S))
		{
			SET_BIT(exit_n, MAP_DIR_S);
		}
		if (room_n && HAS_BIT(room_n->exit_dirs, MAP_DIR_D))
		{
			SET_BIT(exit_n, MAP_DIR_D);
		}

		if (room && HAS_BIT(room->exit_dirs, MAP_DIR_NW))
		{
			SET_BIT(exit_nw, MAP_DIR_SE);
		}
		if (room_n && HAS_BIT(room_n->exit_dirs, MAP_DIR_SW))
		{
			SET_BIT(exit_nw, MAP_DIR_NE);
		}
		if (room_nw && HAS_BIT(room_nw->exit_dirs, MAP_DIR_SE))
		{
			SET_BIT(exit_nw, MAP_DIR_NW);
		}
		if (room_w && HAS_BIT(room_w->exit_dirs, MAP_DIR_NE))
		{
			SET_BIT(exit_nw, MAP_DIR_SW);
		}

		if (room && HAS_BIT(room->exit_dirs, MAP_DIR_W))
		{
			SET_BIT(exit_w, MAP_DIR_W);
		}
		if (room_w && HAS_BIT(room_w->exit_dirs, MAP_DIR_E))
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
						strcat(buf, "  ");
						break;
					case MAP_DIR_NE:
						strcat(buf, " ⸍");
						break;
					case MAP_DIR_SE:
						strcat(buf, " ⸜");
						break;
					case MAP_DIR_NE|MAP_DIR_SE:
						strcat(buf, " <");
						break;
					case MAP_DIR_SW:
						strcat(buf, "⸝ ");
						break;
					case MAP_DIR_NE|MAP_DIR_SW:
						strcat(buf, "／");
						break;
					case MAP_DIR_SE|MAP_DIR_SW:
						strcat(buf, "⸝⸜");
						break;
					case MAP_DIR_NE|MAP_DIR_SE|MAP_DIR_SW:
						strcat(buf, "⸝<");
						break;
					case MAP_DIR_NW:
						strcat(buf, "⸌ ");
						break;
					case MAP_DIR_NE|MAP_DIR_NW:
						strcat(buf, "⸌⸍");
						break;
					case MAP_DIR_SE|MAP_DIR_NW:
						strcat(buf, "＼");
						break;
					case MAP_DIR_SW|MAP_DIR_NW:
						strcat(buf, "> ");
						break;
					case MAP_DIR_NE|MAP_DIR_SE|MAP_DIR_NW:
						strcat(buf, "⸌<");
						break;
					case MAP_DIR_NE|MAP_DIR_SW|MAP_DIR_NW:
						strcat(buf, ">⸍");
						break;
					case MAP_DIR_SE|MAP_DIR_SW|MAP_DIR_NW:
						strcat(buf, ">⸜");
						break;
					case MAP_DIR_NW|MAP_DIR_SE|MAP_DIR_NE|MAP_DIR_SW:
//						strcat(buf, "ᐳᐸ");
						strcat(buf, "><");
						break;
					default:
						strcat(buf, "??");
						break;
				}

				if (HAS_BIT(exit_n, MAP_DIR_D))
				{
					flags = dir_flags(ses, room_n->vnum, MAP_EXIT_D); // merging

					if (HAS_BIT(flags, EXIT_FLAG_AVOID))
					{
						cat_sprintf(buf, "%s-%s", ses->map->color[MAP_COLOR_AVOID], ses->map->color[MAP_COLOR_EXIT]);
					}
					else if (HAS_BIT(flags, EXIT_FLAG_HIDE))
					{
						cat_sprintf(buf, "%s-%s", ses->map->color[MAP_COLOR_HIDE], ses->map->color[MAP_COLOR_EXIT]);
					}
					else if (HAS_BIT(flags, EXIT_FLAG_INVISIBLE))
					{
						cat_sprintf(buf, "%s-%s", ses->map->color[MAP_COLOR_INVIS], ses->map->color[MAP_COLOR_EXIT]);
					}
					else
					{
						strcat(buf, "-");
//						strcat(buf, "⏷");
					}
				}
				else
				{
					strcat(buf, " ");
				}

				switch (HAS_BIT(exit_n, MAP_DIR_N|MAP_DIR_S))
				{
					case MAP_DIR_N:
						strcat(buf, "↑");
						break;
					case MAP_DIR_S:
						strcat(buf, "↓");
						break;
					case MAP_DIR_N|MAP_DIR_S:
						strcat(buf, "│");
						break;
					default:
						strcat(buf, " ");
						break;
				}

				if (room && HAS_BIT(room->exit_dirs, MAP_DIR_U))
				{
					flags = dir_flags(ses, room->vnum, MAP_EXIT_U);

					if (HAS_BIT(flags, EXIT_FLAG_AVOID))
					{
						cat_sprintf(buf, "%s+%s", ses->map->color[MAP_COLOR_AVOID], ses->map->color[MAP_COLOR_EXIT]);
					}
					else if (HAS_BIT(flags, EXIT_FLAG_HIDE))
					{
						cat_sprintf(buf, "%s+%s", ses->map->color[MAP_COLOR_HIDE], ses->map->color[MAP_COLOR_EXIT]);
					}
					else if (HAS_BIT(flags, EXIT_FLAG_INVISIBLE))
					{
						cat_sprintf(buf, "%s+%s", ses->map->color[MAP_COLOR_INVIS], ses->map->color[MAP_COLOR_EXIT]);
					}
					else
					{
						strcat(buf, "+");
//						strcat(buf, "⏶");
					}
				}
				else
				{
					strcat(buf, " ");
				}
				break;

			case 2:
				if (room == NULL)
				{
					strcpy(buf, "     ");
					pop_call();
					return buf;
				}

				switch (exit_w)
				{
					case 0:
						strcpy(buf, "  ");
						break;
					case MAP_DIR_W:
						sprintf(buf, "%s ‒", ses->map->color[MAP_COLOR_EXIT]);
						break;
					case MAP_DIR_E:
						sprintf(buf, "%s‒ ", ses->map->color[MAP_COLOR_EXIT]);
						break;
					case MAP_DIR_W|MAP_DIR_E:
						sprintf(buf, "%s‒‒", ses->map->color[MAP_COLOR_EXIT]);
						break;
					default:
						strcat(buf, "??");
						break;
				}

				if (room->vnum == ses->map->in_room)
				{
					cat_sprintf(buf, "%s%s%s%s%s", room_left, ses->map->color[MAP_COLOR_USER], ses->map->legend[offset + index], room_right, ses->map->color[MAP_COLOR_EXIT]);
					pop_call();
					return buf;
				}
				else
				{
					// use a yet to be made 1x1 8 exit legend for void room drawing, call semi-recursively.

					if (strip_color_strlen(ses, room->symbol) > 1)
					{
						cat_sprintf(buf, "%s%-3s%s", ses->map->color[MAP_COLOR_SYMBOL], room->symbol, ses->map->color[MAP_COLOR_EXIT]);
					}
					else if (HAS_BIT(room->flags, ROOM_FLAG_VOID))
					{
						if (HAS_BIT(room->exit_dirs, MAP_DIR_W) && !HAS_BIT(room->exit_dirs, MAP_DIR_NW|MAP_DIR_SW))
						{
							cat_sprintf(buf, "‒");
						}
						else
						{
							cat_sprintf(buf, " ");
						}

						if (*room->symbol != ' ' && strip_color_strlen(ses, room->symbol) == 1)
						{
							cat_sprintf(buf, "%s%-1s%s", ses->map->color[MAP_COLOR_SYMBOL], room->symbol, ses->map->color[MAP_COLOR_EXIT]);
						}
						else
						{

							if (room->vnum == ses->map->in_room)
							{
								cat_sprintf(buf, "%s", ses->map->color[MAP_COLOR_USER]);
							}

							switch (HAS_BIT(room->exit_dirs, MAP_DIR_N|MAP_DIR_E|MAP_DIR_S|MAP_DIR_W|MAP_DIR_NW|MAP_DIR_NE|MAP_DIR_SE|MAP_DIR_SW))
							{
								case MAP_DIR_S:
								case MAP_DIR_N:
								case MAP_DIR_N|MAP_DIR_S:
									cat_sprintf(buf, "│");
									break;

								case MAP_DIR_E:
								case MAP_DIR_W:
								case MAP_DIR_E|MAP_DIR_W:
									cat_sprintf(buf, "‒");
									break;

								default:
									cat_sprintf(buf, "*");
									break;
							}

							if (room->vnum == ses->map->in_room)
							{
								cat_sprintf(buf, "%s", ses->map->color[MAP_COLOR_EXIT]);
							}
						}

						if (HAS_BIT(room->exit_dirs, MAP_DIR_E) && !HAS_BIT(room->exit_dirs, MAP_DIR_NE|MAP_DIR_SE))
						{
							cat_sprintf(buf, "‒");
						}
						else
						{
							cat_sprintf(buf, " ");
						}
					}
					else
					{
						cat_sprintf(buf, "%s%s%-1s%s%s", room_left, ses->map->color[MAP_COLOR_SYMBOL], room->symbol, room_right, ses->map->color[MAP_COLOR_EXIT]);
					}
				}
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

	if (room == NULL)
	{
		if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
		{
			sprintf(buf, "      ");
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
		sprintf(buf, "%s", ses->map->color[MAP_COLOR_EXIT]);

		switch (line)
		{
			case 1:
				strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_NW) ? "\\ " : "  ");
				strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_N)  ? "|"   : " ");
				strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_U)  ? "+"   : " ");
				strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_NE) ? "/ "  : "  ");
				break;

			case 2:
				if (!HAS_BIT(room->flags, ROOM_FLAG_VOID) && !HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS))
				{
					strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_W) ? "-" : " ");
				}

				if (room->vnum == ses->map->in_room)
				{
					if (!HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS))
					{
						cat_sprintf(buf, "%s%s%s%s%s", room_left, ses->map->color[MAP_COLOR_USER], ses->map->legend[index], room_right, ses->map->color[MAP_COLOR_EXIT]);
					}
					else
					{
						cat_sprintf(buf, "%s%05d%s", ses->map->color[MAP_COLOR_USER], room->vnum, ses->map->color[MAP_COLOR_EXIT]);
					}
				}
				else
				{
					if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS))
					{
						cat_sprintf(buf, "%s%05d%s", room_color, room->vnum, ses->map->color[MAP_COLOR_EXIT]);
					}
					else if (HAS_BIT(room->flags, ROOM_FLAG_VOID))
					{
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
					else
					{
						if (strip_color_strlen(ses, room->symbol) <= 1)
						{
							cat_sprintf(buf, "%s%s%-1s%s%s", room_left, ses->map->color[MAP_COLOR_SYMBOL], room->symbol, room_right, ses->map->color[MAP_COLOR_EXIT]);
						}
						else
						{
							cat_sprintf(buf, "%s%s%-3s%s", room_color, ses->map->color[MAP_COLOR_SYMBOL], room->symbol, ses->map->color[MAP_COLOR_EXIT]);
						}
					}
				}

				if (HAS_BIT(room->flags, ROOM_FLAG_VOID) || !HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIVNUMS))
				{
					strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_E) ? "--" : "  ");
				}
				else
				{
					strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_E) ? "-" : " ");
				}
				break;

			case 3:
				strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_SW) ? "/"   : " ");
				strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_D)  ? "-"   : " ");
				strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_S)  ? "| "  : "  ");
				strcat(buf, HAS_BIT(room->exit_dirs, MAP_DIR_SE) ? "\\ " : "  ");
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
			SET_BIT(exits, MAP_EXIT_NW);
		}
		if (HAS_BIT(room->exit_dirs, MAP_DIR_NE))
		{
			SET_BIT(exit2, 1 << 1);
			SET_BIT(exits, MAP_EXIT_NE);
		}
		if (HAS_BIT(room->exit_dirs, MAP_DIR_SW))
		{
			SET_BIT(exit1, 1 << 3);
			SET_BIT(exits, MAP_EXIT_SW);
		}
		if (HAS_BIT(room->exit_dirs, MAP_DIR_SE))
		{
			SET_BIT(exit2, 1 << 3);
			SET_BIT(exits, MAP_EXIT_SE);
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
			switch (exits)
			{
				case MAP_EXIT_N|MAP_EXIT_E:
				case MAP_EXIT_N|MAP_EXIT_SE:
					room1 = LEGEND_MUDFONT_CURVED + 0;
					break;
				case MAP_EXIT_S|MAP_EXIT_E:
				case MAP_EXIT_S|MAP_EXIT_NE:
					room1 = LEGEND_MUDFONT_CURVED + 1;
					break;
				case MAP_EXIT_S|MAP_EXIT_W:
				case MAP_EXIT_S|MAP_EXIT_NW:
					room2 = LEGEND_MUDFONT_CURVED + 2;
					break;
				case MAP_EXIT_N|MAP_EXIT_W:
				case MAP_EXIT_N|MAP_EXIT_SW:
					room2 = LEGEND_MUDFONT_CURVED + 3;
					break;
			}
		}

		sprintf(buf, "%s%s%s", room_color, ses->map->legend[room1], ses->map->legend[room2]);
	}
	else
	{
		if (HAS_BIT(ses->map->flags, MAP_FLAG_SYMBOLGRAPHICS) && room->symbol[0] && room->symbol[0] != ' ')
		{
			sprintf(buf, "%s%-1s", room_color, room->symbol);
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
					switch (exits)
					{
						case MAP_EXIT_N|MAP_EXIT_E:
							exits = 16 + 4;
							break;
						case MAP_EXIT_S|MAP_EXIT_E:
							exits = 16 + 5;
							break;
						case MAP_EXIT_S|MAP_EXIT_W:
							exits = 16 + 6;
							break;
						case MAP_EXIT_N|MAP_EXIT_W:
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
	char buf[MAP_SEARCH_MAX][BUFFER_SIZE], tmp[BUFFER_SIZE];
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
		arg = sub_arg_in_braces(ses, arg, tmp, GET_ALL, SUB_VAR|SUB_FUN);

		if (!strcasecmp(tmp, "roomname"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_NAME], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(tmp, "roomexits"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_EXITS], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(tmp, "roomdesc"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_DESC], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(tmp, "roomarea"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_AREA], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(tmp, "roomnote"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_NOTE], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(tmp, "roomterrain"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_TERRAIN], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(tmp, "roomflag"))
		{
			arg = sub_arg_in_braces(ses, arg, buf[MAP_SEARCH_FLAG], GET_ALL, SUB_VAR|SUB_FUN);
		}
		else if (!strcasecmp(tmp, "variable"))
		{
			arg = sub_arg_in_braces(ses, arg, var, GET_ALL, SUB_VAR|SUB_FUN);
		}
		else
		{
			strcpy(buf[type++], tmp);
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

	push_call("map_search_compile(%p,%p,%p)",ses,arg,var);

	search_keywords(ses, arg, tmp, var);

	arg = sub_arg_in_braces(ses, tmp, buf, GET_ALL, SUB_VAR|SUB_FUN); // name

	ses->map->search->vnum = (int) get_number(ses, buf);

	if (ses->map->search->vnum)
	{
		pop_call();
		return;
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
		struct listnode *node;
		char exit[BUFFER_SIZE];
		ptb = buf;

		tmp[0] = 0;

		ses->map->search->exit_dirs = get_number(ses, buf);

		while (*ptb)
		{
			ptb = get_arg_in_braces(ses, ptb, exit, GET_ONE);

			node = search_node_list(ses->list[LIST_PATHDIR], exit);

			ses->map->search->exit_size++;

			if (node)
			{
				SET_BIT(ses->map->search->exit_dirs, 1LL << atoi(node->arg3));
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

		ses->map->search->desc = tintin_regexp_compile(ses, NULL, buf, PCRE_ANCHORED);
	}
	else
	{
		ses->map->search->desc = NULL;
	}

	arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN); // area

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

	arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN); // note

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

	arg = sub_arg_in_braces(ses, arg, buf, GET_ALL, SUB_VAR|SUB_FUN); // terrain

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

		while (*buf)
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
				SET_BIT(ses->map->search->flag, ROOM_FLAG_INVISIBLE);
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

	if (search->name)
	{
		if (!regexp_compare(search->name, room->name, "", 0, 0))
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
		if (!regexp_compare(search->desc, room->desc, "", 0, 0))
		{
			return 0;
		}
	}

	if (search->area)
	{
		if (!regexp_compare(search->area, room->area, "", 0, 0))
		{
			return 0;
		}
	}

	if (search->note)
	{
		if (!regexp_compare(search->note, room->note, "", 0, 0))
		{
			return 0;
		}
	}

	if (search->terrain)
	{
		if (!regexp_compare(search->terrain, room->terrain, "", 0, 0))
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

int find_room(struct session *ses, char *arg)
{
	char var[BUFFER_SIZE];
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

	push_call("goto_room(%p,%d)",ses,room);

	if (ses->map->in_room)
	{
		check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "MAP EXIT ROOM", ntos(last_room), ntos(room));
		check_all_events(ses, SUB_ARG|SUB_SEC, 1, 2, "MAP EXIT ROOM %d", last_room, ntos(last_room), ntos(room));
	}

	ses->map->in_room = room;

	DEL_BIT(ses->map->room_list[room]->flags, ROOM_FLAG_PATH);

	if (last_room == 0)
	{
		check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "MAP ENTER MAP", ntos(room));
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "MAP ENTER ROOM", ntos(room), ntos(last_room));
	check_all_events(ses, SUB_ARG|SUB_SEC, 1, 2, "MAP ENTER ROOM %d", room, ntos(room), ntos(last_room));

	pop_call();
	return;
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
		if (!strcmp(exit->name, arg) || exit->vnum == atoi(arg))
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
	if (!HAS_BIT(ses->map->room_list[room]->flags, ROOM_FLAG_VOID))
	{
		return room;
	}

	if (get_room_exits(ses, room) != 2)
	{
		struct exit_data *exit;

		for (exit = ses->map->room_list[room]->f_exit ; exit ; exit = exit->next)
		{
			if (exit->dir == dir)
			{
				return tunnel_void(ses, room, exit->vnum, exit->dir);
			}
		}
		return room;
	}

	if (ses->map->room_list[room]->f_exit->vnum != from)
	{
		return tunnel_void(ses, room, ses->map->room_list[room]->f_exit->vnum, ses->map->room_list[room]->f_exit->dir);
	}
	else
	{
		return tunnel_void(ses, room, ses->map->room_list[room]->l_exit->vnum, ses->map->room_list[room]->l_exit->dir);
	}
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

			if (HAS_BIT(exit->flags, EXIT_FLAG_AVOID) || HAS_BIT(ses->map->room_list[vnum]->flags, ROOM_FLAG_AVOID))
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

			if (HAS_BIT(exit->flags, EXIT_FLAG_AVOID) || HAS_BIT(ses->map->room_list[vnum]->flags, ROOM_FLAG_AVOID))
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

	ses->list[LIST_PATH]->update = 0;

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
			if (HAS_BIT(exit->flags, EXIT_FLAG_AVOID) || HAS_BIT(ses->map->room_list[exit->vnum]->flags, ROOM_FLAG_AVOID))
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
				check_append_path(ses, exit->cmd, "", 0);
			}
			else
			{
				check_append_path(ses, exit->name, "", 0);
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
			show_message(ses, LIST_COMMAND, "#MAP: Linkable room is marked static. Creating overlapping room instead.");

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

	ses->list[LIST_PATH]->update = 0;

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
		check_append_path(ses, exit->cmd, "", 0);
	}
	else
	{
		check_append_path(ses, exit->name, "", 0);
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
				check_append_path(ses, exit->cmd, "", 0);
			}
			else
			{
				check_append_path(ses, exit->name, "", 0);
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

void map_mouse_handler(struct session *ses, char *arg1, char *arg2, int x, int y)
{
	int max_x, max_y;
	int top_row, top_col, bot_row, bot_col, rows, cols;

	if (ses->map == NULL || !HAS_BIT(ses->map->flags, MAP_FLAG_VTMAP) || ses->map->room_list[ses->map->in_room] == NULL)
	{
		return;
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
		bot_row = ses->top_row - 2;
		bot_col = gtd->screen->cols;
		rows    = ses->top_row - 2;
		cols    = gtd->screen->cols;
	}

	y = y - 1;
	x = x - 1;

	if (y > bot_row || y < top_row)
	{
		return;
	}

	if (x > bot_col || x < top_col)
	{
		return;
	}

	y = y + 1 - top_row;
	x = x + 1 - top_col;

	if (HAS_BIT(ses->map->flags, MAP_FLAG_ASCIIGRAPHICS))
	{
		y /= 3;
		x /= 6;

		max_y = 2 + rows / 3;
		max_x = 2 + cols / 6;
	}
	else if (HAS_BIT(ses->map->flags, MAP_FLAG_UNICODEGRAPHICS) || HAS_BIT(ses->map->flags, MAP_FLAG_BLOCKGRAPHICS))
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

	if (max_x != map_grid_x || max_y != map_grid_y)
	{
		return;
	}

	if (ses->map->grid_rooms[x + 1 + max_x * (y - 1)])
	{
                check_all_events(ses, SUB_ARG, 2, 1, "MAP %s %s", arg1, arg2, ntos(ses->map->grid_rooms[x + 1 + max_x * (y - 1)]->vnum));
	}
}
