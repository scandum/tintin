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
*                      coded by Igor van den Hoven 2011                       *
******************************************************************************/

#include "tintin.h"
#include "telnet.h"

// Set table size and check for errors. Call once at startup.

void init_msdp_table(void)
{
	int index;

	for (index = 0 ; *msdp_table[index].name ; index++)
	{
		if (strcmp(msdp_table[index].name, msdp_table[index+1].name) > 0)
		{
			if (*msdp_table[index+1].name)
			{
				print_stdout("\e[31minit_msdp_table: Improperly sorted variable: %s.\e0m", msdp_table[index+1].name);
			}
		}
	}
	gtd->msdp_table_size = index;
}

// Binary search on the msdp_table.

int msdp_find(char *var)
{
	int val, bot, top, srt;

	bot = 0;
	top = gtd->msdp_table_size - 1;
	val = top / 2;

	while (bot <= top)
	{
		srt = strcmp(var, msdp_table[val].name);

		if (srt < 0)
		{
			top = val - 1;
		}
		else if (srt > 0)
		{
			bot = val + 1;
		}
		else
		{
			return val;
		}
		val = bot + (top - bot) / 2;
	}
        tintin_printf2(NULL, "msdp_find: %s (-1)", var, val);
	return -1;
}

void arachnos_devel(struct session *ses, char *fmt, ...)
{
	char buf[STRING_SIZE];

	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	port_printf(ses, "ARACHNOS: %s", buf);
}

void arachnos_mudlist(struct session *ses, char *fmt, ...)
{
	char buf[STRING_SIZE];

	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	port_printf(ses, "ARACHNOS: %s", buf);
}

void msdp_update_all(char *var, char *fmt, ...)
{
	struct session *ses;
	struct port_data *buddy;
	char buf[STRING_SIZE];
	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	for (ses = gts->next ; ses ; ses = ses->next)
	{
		if (ses->port)
		{
			for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
			{
				if (buddy->msdp_data)
				{
					msdp_update_var(ses, buddy, var, buf);
				}
			}
		}
	}
}

// Update a variable and queue it if it's being reported.

void msdp_update_index(struct session *ses, struct port_data *buddy, int index, char *str)
{
	if (strcmp(buddy->msdp_data[index]->value, str))
	{
		if (HAS_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_REPORTED))
		{
			SET_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_UPDATED);
			SET_BIT(buddy->comm_flags, COMM_FLAG_MSDPUPDATE);
		}
		RESTRING(buddy->msdp_data[index]->value, str);
	}
}

void msdp_update_var(struct session *ses, struct port_data *buddy, char *var, char *str)
{
	int index;

	index = msdp_find(var);

	if (index == -1)
	{
		port_printf(ses, "msdp_update_var: Unknown variable: %s.", var);

		return;
	}

	msdp_update_index(ses, buddy, index, str);
}

void msdp_update_varf(struct session *ses, struct port_data *buddy, char *var, char *fmt, ...)
{
	char buf[STRING_SIZE];
	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	msdp_update_var(ses, buddy, var, buf);
}

// Update a variable and send it instantly.

void msdp_update_var_instant(struct session *ses, struct port_data *buddy, char *var, char *fmt, ...)
{
	char buf[BUFFER_SIZE], out[STRING_SIZE];
	int index, length;
	va_list args;

	index = msdp_find(var);

	if (index == -1)
	{
		port_printf(ses, "msdp_update_var_instant: Unknown variable: %s.", var);

		return;
	}

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	if (strcmp(buddy->msdp_data[index]->value, buf))
	{
		RESTRING(buddy->msdp_data[index]->value, buf);
	}

	if (HAS_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_REPORTED))
	{
		length = sprintf(out, "%c%c%c%c%s%c%s%c%c", IAC, SB, TELOPT_MSDP, MSDP_VAR, msdp_table[index].name, MSDP_VAL, buf, IAC, SE);

		write_msdp_to_descriptor(ses, buddy, out, length);
	}
}

// Send all reported variables that have been updated.

void msdp_send_update(struct session *ses, struct port_data *buddy)
{
	char *ptr, buf[STRING_SIZE];
	int index;

	if (buddy->msdp_data == NULL)
	{
		return;
	}

	ptr = buf;

	ptr += sprintf(ptr, "%c%c%c", IAC, SB, TELOPT_MSDP);

	for (index = 0 ; index < gtd->msdp_table_size ; index++)
	{
		if (HAS_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_UPDATED))
		{
			ptr += sprintf(ptr, "%c%s%c%s", MSDP_VAR, msdp_table[index].name, MSDP_VAL, buddy->msdp_data[index]->value);

			DEL_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_UPDATED);
		}

		if (ptr - buf > STRING_SIZE - BUFFER_SIZE)
		{
			port_printf(ses, "msdp_send_update: MSDP BUFFER OVERFLOW");
			break;
		}
	}

	ptr += sprintf(ptr, "%c%c", IAC, SE);

	write_msdp_to_descriptor(ses, buddy, buf, ptr - buf);

	DEL_BIT(buddy->comm_flags, COMM_FLAG_MSDPUPDATE);
}


char *msdp_get_var(struct session *ses, struct port_data *buddy, char *var)
{
	int index;

	index = msdp_find(var);

	if (index == -1)
	{
		port_printf(ses, "msdp_get_var: Unknown variable: %s.", var);

		return NULL;
	}

	return buddy->msdp_data[index]->value;
}

// 1d array support for commands

void process_msdp_index_val(struct session *ses, struct port_data *buddy, int var_index, char *val )
{
	int val_index;

	val_index = msdp_find(val);

	if (val_index >= 0)
	{
		if (msdp_table[var_index].fun)
		{
			msdp_table[var_index].fun(ses, buddy, val_index);
		}
	}
} 

// 1d array support for commands

void process_msdp_array(struct session *ses, struct port_data *buddy, int var_index, char *val)
{
	char buf[BUFFER_SIZE], *pto, *pti;

	pti = val;
	pto = buf;

	while (*pti)
	{
		switch (*pti)
		{
			case MSDP_ARRAY_OPEN:
				break;

			case MSDP_VAL:
				*pto = 0;

				if (*buf)
				{
					process_msdp_index_val(ses, buddy, var_index, buf);
				}
				pto = buf;
				break;

			case MSDP_ARRAY_CLOSE:
				*pto = 0;

				if (*buf)
				{
					process_msdp_index_val(ses, buddy, var_index, buf);
				}
				return;

			default:
				*pto++ = *pti;
				break;
		}
		pti++;
	}
	*pto = 0;

	if (*buf)
	{
		process_msdp_index_val(ses, buddy, var_index, buf);
	}
	return;
}

void process_msdp_varval(struct session *ses, struct port_data *buddy, char *var, char *val )
{
	int var_index, val_index;

	var_index = msdp_find(var);

	if (var_index == -1)
	{
		return;
	}

	if (HAS_BIT(msdp_table[var_index].flags, MSDP_FLAG_CONFIGURABLE))
	{
		RESTRING(buddy->msdp_data[var_index]->value, val);

		if (msdp_table[var_index].fun)
		{
			msdp_table[var_index].fun(ses, buddy, var_index);
		}
		return;
	}

	// Commands only take variables as arguments.

	if (HAS_BIT(msdp_table[var_index].flags, MSDP_FLAG_COMMAND))
	{
		if (*val == MSDP_ARRAY_OPEN)
		{
			port_printf(ses, "process_msdp_varval: array");

			process_msdp_array(ses, buddy, var_index, val);
		}
		else
		{
			val_index = msdp_find(val);

			if (val_index == -1)
			{
				return;
			}

			if (msdp_table[var_index].fun)
			{
				msdp_table[var_index].fun(ses, buddy, val_index);
			}
		}
		return;
	}
} 

void msdp_command_list(struct session *ses, struct port_data *buddy, int index)
{
	char *ptr, buf[STRING_SIZE];
	int flag;

	if (!HAS_BIT(msdp_table[index].flags, MSDP_FLAG_LIST))
	{
		return;
	}

	ptr = buf;

	flag = msdp_table[index].flags;

	ptr += sprintf(ptr, "%c%c%c%c%s%c%c", IAC, SB, TELOPT_MSDP, MSDP_VAR, msdp_table[index].name, MSDP_VAL, MSDP_ARRAY_OPEN);

	for (index = 0 ; index < gtd->msdp_table_size ; index++)
	{
		if (flag != MSDP_FLAG_LIST)
		{
			if (HAS_BIT(buddy->msdp_data[index]->flags, flag) && !HAS_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_LIST))
			{
				ptr += sprintf(ptr, "%c%s", MSDP_VAL, msdp_table[index].name);
			}
		}
		else
		{
			if (HAS_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_LIST))
			{
				ptr += sprintf(ptr, "%c%s", MSDP_VAL, msdp_table[index].name);
			}
		}
	}

	ptr += sprintf(ptr, "%c%c%c", MSDP_ARRAY_CLOSE, IAC, SE);

	write_msdp_to_descriptor(ses, buddy, buf, ptr - buf);
}

void msdp_command_report(struct session *ses, struct port_data *buddy, int index)
{
	port_printf(ses, "msdp_command_report(%s,%s,%d)", ses->name, buddy->name, index);

	if (!HAS_BIT(msdp_table[index].flags, MSDP_FLAG_REPORTABLE))
	{
		return;
	}

	SET_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_REPORTED);

	if (!HAS_BIT(msdp_table[index].flags, MSDP_FLAG_SENDABLE))
	{
		return;
	}

	SET_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_UPDATED);
	SET_BIT(buddy->comm_flags, COMM_FLAG_MSDPUPDATE);
}

void msdp_command_reset(struct session *ses, struct port_data *buddy, int index)
{
	int flag;

	if (!HAS_BIT(msdp_table[index].flags, MSDP_FLAG_LIST))
	{
		return;
	}

	flag = msdp_table[index].flags &= ~MSDP_FLAG_LIST;

	for (index = 0 ; index < gtd->msdp_table_size ; index++)
	{
		if (HAS_BIT(buddy->msdp_data[index]->flags, flag))
		{
			buddy->msdp_data[index]->flags = msdp_table[index].flags;
		}
	}
}

void msdp_command_send(struct session *ses, struct port_data *buddy, int index)
{
	if (HAS_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_SENDABLE))
	{
		SET_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_UPDATED);	
		SET_BIT(buddy->comm_flags, COMM_FLAG_MSDPUPDATE);
	}
}

void msdp_command_unreport(struct session *ses, struct port_data *buddy, int index)
{
	if (!HAS_BIT(msdp_table[index].flags, MSDP_FLAG_REPORTABLE))
	{
		return;
	}

	DEL_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_REPORTED);
}

// Arachnos Map support

void msdp_configure_map(struct session *ses, struct port_data *buddy, int index)
{
	char val[BUFFER_SIZE];
	struct room_data *room;
	struct exit_data *exit;
	int vnum;

	if (ses->map == NULL || *buddy->msdp_data[index]->value == 0)
	{
		return;
	}

	if (!strcmp(msdp_table[index].name, "MAP_ROOM_INFO"))
	{
		vnum = atoi(buddy->msdp_data[index]->value);

		port_printf(ses, "MAP_ROOM_INFO: %d", vnum);

		if (vnum <= 0 || vnum >= ses->map->size)
		{
			return;
		}

		room = ses->map->room_list[vnum];

		if (room == NULL)
		{
			return;
		}

		sprintf(val, "ROOM\002%c", MSDP_TABLE_OPEN);

		cat_sprintf(val, "\001AREA\002%s",    room->area);
		cat_sprintf(val, "\001COLOR\002%s",   room->color);
		cat_sprintf(val, "\001DESC\002%s",    room->desc);

		cat_sprintf(val, "\001EXITS\002%c", MSDP_TABLE_OPEN);

		for (exit = room->f_exit ; exit ; exit = exit->next)
		{
			cat_sprintf(val, "\001%s\002", exit->name);
			cat_sprintf(val, "%c", MSDP_TABLE_OPEN);
				cat_sprintf(val, "\001COMMAND\002%s",  exit->cmd);
				cat_sprintf(val, "\001FLAGS\002%d",    exit->flags);
				cat_sprintf(val, "\001NAME\002%s",     exit->name);
				cat_sprintf(val, "\001VNUM\002%d",     exit->vnum);
				cat_sprintf(val, "\001WEIGHT\002%.3f", exit->weight);
			cat_sprintf(val, "%c", MSDP_TABLE_CLOSE);
		}
		cat_sprintf(val, "%c", MSDP_TABLE_CLOSE);

		cat_sprintf(val, "\001FLAGS\002%d",     room->flags);
		cat_sprintf(val, "\001NAME\002%s",      room->name);
		cat_sprintf(val, "\001NOTE\002%s",      room->note);
		cat_sprintf(val, "\001SYMBOL\002%s",    room->symbol);
		cat_sprintf(val, "\001TERRAIN\002%s",   room->terrain);
		cat_sprintf(val, "\001VNUM\002%d",      room->vnum);
		cat_sprintf(val, "\001WEIGHT\002%.3f",  room->weight);

		cat_sprintf(val, "%c", MSDP_TABLE_CLOSE);

		msdp_update_index(ses, buddy, index, val);

		SET_BIT(buddy->msdp_data[index]->flags, MSDP_FLAG_UPDATED);
		SET_BIT(buddy->comm_flags, COMM_FLAG_MSDPUPDATE);
	}
	else if (!strcmp(msdp_table[index].name, "MAP_ROOM_GOTO"))
	{
		// to be imped
	}
}

// Arachnos Intermud support

void msdp_configure_arachnos(struct session *ses, struct port_data *buddy, int index)
{
	char var[BUFFER_SIZE], val[BUFFER_SIZE];
	char mud_name[BUFFER_SIZE], mud_host[BUFFER_SIZE], mud_port[BUFFER_SIZE];
	char msg_user[BUFFER_SIZE], msg_time[BUFFER_SIZE], msg_body[BUFFER_SIZE];
	char mud_players[BUFFER_SIZE], mud_uptime[BUFFER_SIZE], mud_update[BUFFER_SIZE];
	char *pti, *pto;

	struct tm timeval_tm;
	time_t timeval_t;

	var[0] = val[0] = mud_name[0] = mud_host[0] = mud_port[0] = msg_user[0] = msg_time[0] = msg_body[0] = mud_players[0] = mud_uptime[0] = mud_update[0] = 0;

	pti = buddy->msdp_data[index]->value;

	while (*pti)
	{
		switch (*pti)
		{
			case MSDP_VAR:
				pti++;
				pto = var;

				while (*pti > MSDP_ARRAY_CLOSE)
				{
					*pto++ = *pti++;
				}
				*pto = 0;
				break;

			case MSDP_VAL:
				pti++;
				pto = val;

				while (*pti > MSDP_ARRAY_CLOSE)
				{
					*pto++ = *pti++;
				}
				*pto = 0;

				if (!strcmp(var, "MUD_NAME"))
				{
					strcpy(mud_name, val);
				}
				else if (!strcmp(var, "MUD_HOST"))
				{
					strcpy(mud_host, val);
				}
				else if (!strcmp(var, "MUD_PORT"))
				{
					strcpy(mud_port, val);
				}
				else if (!strcmp(var, "MSG_USER"))
				{
					strcpy(msg_user, val);
				}
				else if (!strcmp(var, "MSG_TIME"))
				{
					timeval_t = (time_t) atoll(val);
					timeval_tm = *localtime(&timeval_t);
					
					strftime(msg_time, 20, "%T %D", &timeval_tm);
				}
				else if (!strcmp(var, "MSG_BODY"))
				{
					strcpy(msg_body, val);
				}
				else if (!strcmp(var, "MUD_UPTIME"))
				{
					timeval_t = (time_t) atoll(val);
					timeval_tm = *localtime(&timeval_t);
					
					strftime(mud_uptime, 20, "%T %D", &timeval_tm);
				}
				else if (!strcmp(var, "MUD_UPDATE"))
				{
					timeval_t = (time_t) atoll(val);
					timeval_tm = *localtime(&timeval_t);
					
					strftime(mud_update, 20, "%T %D", &timeval_tm);
				}
				else if (!strcmp(var, "MUD_PLAYERS"))
				{
					strcpy(mud_players, val);
				}
				break;

			default:
				pti++;
				break;
		}
	}

	if (*mud_name && *mud_host && *mud_port)
	{
		if (!strcmp(msdp_table[index].name, "ARACHNOS_DEVEL"))
		{
			if (*msg_user && *msg_time && *msg_body)
			{
				arachnos_devel(ses, "%s %s@%s:%s devtalks: %s", msg_time, msg_user, mud_host, mud_port, msg_body);
			}
		}
		else if (!strcmp(msdp_table[index].name, "ARACHNOS_MUDLIST"))
		{
			if (*mud_uptime && *mud_update && *mud_players)
			{
				arachnos_mudlist(ses, "%18.18s %14.14s %5.5s %17.17s %17.17s %4.4s", mud_name, mud_host, mud_port, mud_update, mud_uptime, mud_players);
			}
		}
	}
}

struct msdp_type msdp_table[] =
{
	{    "ARACHNOS_DEVEL",            MSDP_FLAG_CONFIGURABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SCOUT, msdp_configure_arachnos },
	{    "ARACHNOS_MUDLIST",                               MSDP_FLAG_CONFIGURABLE,  PORT_RANK_SCOUT, msdp_configure_arachnos },
	{    "COMMANDS",                             MSDP_FLAG_COMMAND|MSDP_FLAG_LIST,  PORT_RANK_SPY,   NULL },
	{    "CONFIGURABLE_VARIABLES",          MSDP_FLAG_CONFIGURABLE|MSDP_FLAG_LIST,  PORT_RANK_SPY,   NULL },
	{    "LIST",                                                MSDP_FLAG_COMMAND,  PORT_RANK_SPY,   msdp_command_list },
	{    "LISTS",                                                  MSDP_FLAG_LIST,  PORT_RANK_SPY,   NULL },
	{    "MAP_ROOM_INFO",             MSDP_FLAG_CONFIGURABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SCOUT, msdp_configure_map },
        {    "REPORT",                                              MSDP_FLAG_COMMAND,  PORT_RANK_SPY,   msdp_command_report },
	{    "REPORTABLE_VARIABLES",              MSDP_FLAG_REPORTABLE|MSDP_FLAG_LIST,  PORT_RANK_SPY,   NULL },
	{    "REPORTED_VARIABLES",                  MSDP_FLAG_REPORTED|MSDP_FLAG_LIST,  PORT_RANK_SPY,   NULL },
	{    "RESET",                                               MSDP_FLAG_COMMAND,  PORT_RANK_SPY,   msdp_command_reset },
	{    "SCREEN_CHARACTER_HEIGHT",       MSDP_FLAG_SENDABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SPY,   NULL },
	{    "SCREEN_CHARACTER_WIDTH",        MSDP_FLAG_SENDABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SPY,   NULL },
	{    "SCREEN_COLS",                   MSDP_FLAG_SENDABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SPY,   NULL },
	{    "SCREEN_FOCUS",                  MSDP_FLAG_SENDABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SPY,   NULL },
	{    "SCREEN_HEIGHT",                 MSDP_FLAG_SENDABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SPY,   NULL },
	{    "SCREEN_LOCATION_HEIGHT",        MSDP_FLAG_SENDABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SPY,   NULL },
	{    "SCREEN_LOCATION_WIDTH",         MSDP_FLAG_SENDABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SPY,   NULL },
	{    "SCREEN_MINIMIZED",              MSDP_FLAG_SENDABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SPY,   NULL },
	{    "SCREEN_ROWS",                   MSDP_FLAG_SENDABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SPY,   NULL },
	{    "SCREEN_WIDTH",                  MSDP_FLAG_SENDABLE|MSDP_FLAG_REPORTABLE,  PORT_RANK_SPY,   NULL },
	{    "SEND",                                                MSDP_FLAG_COMMAND,  PORT_RANK_SPY,   msdp_command_send },
	{    "SENDABLE_VARIABLES",                  MSDP_FLAG_SENDABLE|MSDP_FLAG_LIST,  PORT_RANK_SPY,   NULL },
	{    "SPECIFICATION",                                      MSDP_FLAG_SENDABLE,  PORT_RANK_SPY,   NULL },
	{    "UNREPORT",                                            MSDP_FLAG_COMMAND,  PORT_RANK_SPY,   msdp_command_unreport },
	{    "",                                                                    0,              0,   NULL }
};

void write_msdp_to_descriptor(struct session *ses, struct port_data *buddy, char *src, int length)
{
	char out[STRING_SIZE];

	if (!HAS_BIT(buddy->comm_flags, COMM_FLAG_GMCP))
	{
		port_telnet_printf(ses, buddy, length, "%s", src);
	}
	else
	{
		length = msdp2json((unsigned char *) src, length, out);

		port_telnet_printf(ses, buddy, length, "%s", out);
	}
}

int msdp2json(unsigned char *src, int srclen, char *out)
{
	char *pto;
	int i, nest, last;

	nest = last = 0;

	pto = out;

	if (src[2] == TELOPT_MSDP)
	{
		pto += sprintf(pto, "%c%c%cMSDP {", IAC, SB, TELOPT_GMCP);
	}

	i = 3;

	while (i < srclen)
	{
		if (src[i] == IAC && src[i+1] == SE)
		{
			break;
		}

		switch (src[i])
		{
			case MSDP_TABLE_OPEN:
				*pto++ = '{';
				nest++;
				last = MSDP_TABLE_OPEN;
				break;

			case MSDP_TABLE_CLOSE:
				if (last == MSDP_VAL || last == MSDP_VAR)
				{
					*pto++ = '"';
				}
				if (nest)
				{
					nest--;
				}
				*pto++ = '}';
				last = MSDP_TABLE_CLOSE;
				break;

			case MSDP_ARRAY_OPEN:
				*pto++ = '[';
				nest++;
				last = MSDP_ARRAY_OPEN;
				break;

			case MSDP_ARRAY_CLOSE:
				if (last == MSDP_VAL || last == MSDP_VAR)
				{
					*pto++ = '"';
				}
				if (nest)
				{
					nest--;
				}
				*pto++ = ']';
				last = MSDP_ARRAY_CLOSE;
				break;

			case MSDP_VAR:
				if (last == MSDP_VAL || last == MSDP_VAR)
				{
					*pto++ = '"';
				}
				if (last == MSDP_VAL || last == MSDP_VAR || last == MSDP_TABLE_CLOSE || last == MSDP_ARRAY_CLOSE)
				{
					*pto++ = ',';
				}
				*pto++ = '"';
				last = MSDP_VAR;
				break;

			case MSDP_VAL:
				if (last == MSDP_VAR)
				{
					*pto++ = '"';
					*pto++ = ':';
				}
				if (last == MSDP_VAL)
				{
					*pto++ = '"';
					*pto++ = ',';
				}

				if (src[i+1] != MSDP_TABLE_OPEN && src[i+1] != MSDP_ARRAY_OPEN)
				{
					*pto++ = '"';
				}
				last = MSDP_VAL;
				break;

			case '\\':
				*pto++ = '\\';
				*pto++ = '\\';
				break;

			case '"':
				*pto++ = '\\';
				*pto++ = '"';
				break;

			default:
				*pto++ = src[i];
				break;
		}
		i++;
	}

	pto += sprintf(pto, "}%c%c", IAC, SE);

	return pto - out;
}

int json2msdp(unsigned char *src, int srclen, char *out)
{
	char *pto;
	int i, nest, last, type, state[100];

	nest = last = 0;

	pto = out;

	if (src[2] == TELOPT_GMCP)
	{
		pto += sprintf(pto, "%c%c%c", IAC, SB, TELOPT_MSDP);
	}

	i = 3;

	if (!strncmp((char *) &src[3], "MSDP {", 6))
	{
		i += 6;
	}

	state[0] = nest = type = 0;

	while (i < srclen && src[i] != IAC && nest < 99)
	{
		switch (src[i])
		{
			case ' ':
				i++;
				break;

			case '{':
				*pto++ = MSDP_TABLE_OPEN;
				i++;
				state[++nest] = 0;
				break;

			case '}':
				nest--;
				i++;
				if (nest < 0)
				{
					pto += sprintf(pto, "%c%c", IAC, SE);
					return pto - out;
				}
				*pto++ = MSDP_TABLE_CLOSE;
				break;

			case '[':
				i++;
				state[++nest] = 1;
				*pto++ = MSDP_ARRAY_OPEN;
				break;

			case ']':
				nest--;
				i++;
				*pto++ = MSDP_ARRAY_CLOSE;
				break;

			case ':':
				*pto++ = MSDP_VAL;
				i++;
				break;

			case ',':
				i++;
				if (state[nest])
				{
					*pto++ = MSDP_VAL;
				}
				else
				{
					*pto++ = MSDP_VAR;
				}
				break;

			case '"':
				i++;
				if (last == 0)
				{
					last = MSDP_VAR;
					*pto++ = MSDP_VAR;
				}
				type = 1;

				while (i < srclen && src[i] != IAC && type)
				{
					switch (src[i])
					{
						case '\\':
							i++;

							if (i < srclen && src[i] == '"')
							{
								*pto++ = src[i++];
							}
							else
							{
								*pto++ = '\\';
							}
							break;

						case '"':
							i++;
							type = 0;
							break;

						default:
							*pto++ = src[i++];
							break;
					}
				}
				break;

			default:
				type = 1;

				while (i < srclen && src[i] != IAC && type)
				{
					switch (src[i])
					{
						case '}':
						case ']':
						case ',':
						case ':':
							type = 0;
							break;

						case ' ':
							i++;
							break;

						default:
							*pto++ = src[i++];
							break;
					}
				}
				break;
		}
	}
	pto += sprintf(pto, "%c%c", IAC, SE);

	return pto - out;
}

