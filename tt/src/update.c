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
*               (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                  *
*                                                                             *
*                     coded by Igor van den Hoven 2006                        *
******************************************************************************/

#include "tintin.h"

#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>


void mainloop(void)
{
	static struct timeval curr_time, wait_time, last_time;
	int usec_loop, usec_wait;

	short int pulse_poll_input      = 0 + PULSE_POLL_INPUT;
	short int pulse_poll_sessions   = 0 + PULSE_POLL_SESSIONS;
	short int pulse_poll_chat       = 0 + PULSE_POLL_CHAT;
	short int pulse_poll_port       = 0 + PULSE_POLL_PORT;
	short int pulse_update_ticks    = 0 + PULSE_UPDATE_TICKS;
	short int pulse_update_delays   = 0 + PULSE_UPDATE_DELAYS;
	short int pulse_update_packets  = 0 + PULSE_UPDATE_PACKETS;
	short int pulse_update_chat     = 0 + PULSE_UPDATE_CHAT;
	short int pulse_update_terminal = 0 + PULSE_UPDATE_TERMINAL;
	short int pulse_update_memory   = 0 + PULSE_UPDATE_MEMORY;
	short int pulse_update_time     = 1 + PULSE_UPDATE_TIME;

	wait_time.tv_sec = 0;

	push_call("mainloop()");

	while (TRUE)
	{
		gettimeofday(&last_time, NULL);

		if (--pulse_poll_input == 0)
		{
			open_timer(TIMER_POLL_INPUT);

			pulse_poll_input = PULSE_POLL_INPUT;

			poll_input();

			close_timer(TIMER_POLL_INPUT);
		}

		if (--pulse_poll_sessions == 0)
		{
			pulse_poll_sessions = PULSE_POLL_SESSIONS;

			poll_sessions();
		}

		if (--pulse_poll_chat == 0)
		{
			pulse_poll_chat = PULSE_POLL_CHAT;

			poll_chat();
		}	

		if (--pulse_poll_port == 0)
		{
			pulse_poll_port = PULSE_POLL_PORT;

			poll_port();
		}	

		if (--pulse_update_ticks == 0)
		{
			pulse_update_ticks = PULSE_UPDATE_TICKS;

			tick_update();
		}

		if (--pulse_update_delays == 0)
		{
			pulse_update_delays = PULSE_UPDATE_DELAYS;

			delay_update();
		}

		if (--pulse_update_packets == 0)
		{
			pulse_update_packets = PULSE_UPDATE_PACKETS;

			packet_update();
		}

		if (--pulse_update_chat == 0)
		{
			pulse_update_chat = PULSE_UPDATE_CHAT;

			chat_update();
		}

		if (--pulse_update_terminal == 0)
		{
			pulse_update_terminal = PULSE_UPDATE_TERMINAL;

			terminal_update();
		}

		if (--pulse_update_memory == 0)
		{
			pulse_update_memory = PULSE_UPDATE_MEMORY;

			memory_update();
		}

		if (--pulse_update_time == 0)
		{
			pulse_update_time = PULSE_UPDATE_TIME;

			time_update();
		}

		gettimeofday(&curr_time, NULL);

		if (curr_time.tv_sec == last_time.tv_sec)
		{
			usec_loop = curr_time.tv_usec - last_time.tv_usec;
		}
		else
		{
			usec_loop = 1000000 - last_time.tv_usec + curr_time.tv_usec;
		}

		usec_wait = 1000000 / PULSE_PER_SECOND - usec_loop;

		wait_time.tv_usec = usec_wait;

		gtd->total_io_exec  += usec_loop;
		gtd->total_io_delay += usec_wait;

		if (usec_wait > 0)
		{
			select(0, NULL, NULL, NULL, &wait_time);
		}
	}
	pop_call();
	return;
}

void poll_input(void)
{
	fd_set readfds;
	static struct timeval to;

	while (TRUE)
	{
		FD_ZERO(&readfds);

		FD_SET(0, &readfds);

		if (select(FD_SETSIZE, &readfds, NULL, NULL, &to) <= 0)
		{
			return;
		}

		if (FD_ISSET(0, &readfds))
		{
			process_input();
		}
		else
		{
			return;
		}
	}
}

void poll_sessions(void)
{
	fd_set readfds, excfds;
	static struct timeval to;
	struct session *ses;
	int rv;

	push_call("poll_sessions(void)");

	open_timer(TIMER_POLL_SESSIONS);

	if (gts->next)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&excfds);

		for (ses = gts->next ; ses ; ses = gtd->update)
		{
			gtd->update = ses->next;

			if (HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
			{
				while (TRUE)
				{
					FD_SET(ses->socket, &readfds);
					FD_SET(ses->socket, &excfds);

					rv = select(FD_SETSIZE, &readfds, NULL, &excfds, &to);

					if (rv < 0)
					{
						break;

//						ses->
						syserr_printf(ses, "poll_sessions: select = %d:", rv);

						cleanup_session(ses);

						gtd->mud_output_len = 0;

						break;;
					}

					if (rv == 0)
					{
						break;
					}

					if (FD_ISSET(ses->socket, &readfds))
					{
						if (read_buffer_mud(ses) == FALSE)
						{
							readmud(ses);

							cleanup_session(ses);

							gtd->mud_output_len = 0;

							break;
						}
					}

					if (FD_ISSET(ses->socket, &excfds))
					{
						FD_CLR(ses->socket, &readfds);

						cleanup_session(ses);

						gtd->mud_output_len = 0;

						break;
					}
				}

				if (gtd->mud_output_len)
				{
					readmud(ses);
				}
			}
		}
	}
	close_timer(TIMER_POLL_SESSIONS);

	pop_call();
	return;
}

void poll_chat(void)
{
	fd_set readfds, writefds, excfds;
	static struct timeval to;
	struct chat_data *buddy;
	int rv;

	open_timer(TIMER_POLL_CHAT);

	if (gtd->chat)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&excfds);

		FD_SET(gtd->chat->fd, &readfds);

		for (buddy = gtd->chat->next ; buddy ; buddy = buddy->next)
		{
			FD_SET(buddy->fd, &readfds);
			FD_SET(buddy->fd, &writefds);
			FD_SET(buddy->fd, &excfds);
		}

		rv = select(FD_SETSIZE, &readfds, &writefds, &excfds, &to);

		if (rv <= 0)
		{
			if (rv == 0 || errno == EINTR)
			{
				goto poll_chat_end;
			}
			syserr_fatal(-1, "poll_chat: select");
		}
		process_chat_connections(&readfds, &writefds, &excfds);
	}

	poll_chat_end:

	close_timer(TIMER_POLL_CHAT);
}

void poll_port(void)
{
	struct session *ses;
	fd_set readfds, writefds, excfds;
	static struct timeval to;
	struct port_data *buddy;
	int rv;

	open_timer(TIMER_POLL_PORT);

	for (ses = gts->next ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		if (ses->port)
		{
			FD_ZERO(&readfds);
			FD_ZERO(&writefds);
			FD_ZERO(&excfds);

			FD_SET(ses->port->fd, &readfds);

			for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
			{
				FD_SET(buddy->fd, &readfds);
				FD_SET(buddy->fd, &writefds);
				FD_SET(buddy->fd, &excfds);
			}

			rv = select(FD_SETSIZE, &readfds, &writefds, &excfds, &to);

			if (rv <= 0)
			{
				if (rv == 0 || errno == EINTR)
				{
					continue;
				}
				syserr_fatal(-1, "poll_port: select");
			}

			process_port_connections(ses, &readfds, &writefds, &excfds);
		}
	}

	close_timer(TIMER_POLL_PORT);
}

void tick_update(void)
{
	struct session *ses;
	struct listnode *node;
	struct listroot *root;

	open_timer(TIMER_UPDATE_TICKS);

	utime();

	for (ses = gts->next ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		root = ses->list[LIST_TICKER];

		for (root->update = 0 ; root->update < root->used ; root->update++)
		{
			node = root->list[root->update];

			if (node->data == 0)
			{
				node->data = gtd->utime + (long long) (get_number(ses, node->arg3) * 1000000LL);

				show_info(ses, LIST_DELAY, "#INFO TICK {%s} INITIALIZED WITH TIMESTAMP {%lld}", node->arg1, node->data);
			}

			if (node->data <= gtd->utime)
			{
				node->data += (long long) (get_number(ses, node->arg3) * 1000000LL);

				show_info(ses, LIST_DELAY, "#INFO TICK {%s} INITIALIZED WITH TIMESTAMP {%lld}", node->arg1, node->data);

				if (!HAS_BIT(root->flags, LIST_FLAG_IGNORE))
				{
					show_debug(ses, LIST_TICKER, "#DEBUG TICKER {%s}", node->arg2);

					script_driver(ses, LIST_TICKER, node->arg2);
				}
			}
		}
	}
	close_timer(TIMER_UPDATE_TICKS);
}

void delay_update(void)
{
	struct session *ses;
	struct listnode *node;
	struct listroot *root;
	char buf[BUFFER_SIZE];

	open_timer(TIMER_UPDATE_DELAYS);

	for (ses = gts ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		root = ses->list[LIST_DELAY];	

		for (root->update = 0 ; root->update < root->used ; root->update++)
		{
			node = root->list[root->update];

			if (node->data == 0)
			{
				node->data = gtd->utime + (long long) (get_number(ses, node->arg3) * 1000000LL);

				show_info(ses, LIST_DELAY, "#INFO DELAY {%s} INITIALIZED WITH TIMESTAMP {%lld}", node->arg1, node->data);
			}

			if (node->data <= gtd->utime)
			{
				strcpy(buf, node->arg2);

				show_debug(ses, LIST_DELAY, "#DEBUG DELAY {%s}", buf);

				delete_node_list(ses, LIST_DELAY, node);

				script_driver(ses, LIST_DELAY, buf);
			}
		}
	}
	close_timer(TIMER_UPDATE_DELAYS);
}

void packet_update(void)
{
	char result[STRING_SIZE];
	struct session *ses;

	open_timer(TIMER_UPDATE_PACKETS);

	for (ses = gts->next ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		if (ses->check_output && gtd->utime > ses->check_output)
		{
			if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
			{
				save_pos(ses);
				goto_rowcol(ses, ses->bot_row, 1);
			}

			SET_BIT(ses->flags, SES_FLAG_READMUD);

			strcpy(result, ses->more_output);

			ses->more_output[0] = 0;

			process_mud_output(ses, result, TRUE);

			DEL_BIT(ses->flags, SES_FLAG_READMUD);

			if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
			{
				restore_pos(ses);
			}
		}
	}
	close_timer(TIMER_UPDATE_PACKETS);
}

void chat_update(void)
{
	struct chat_data *buddy, *buddy_next;

	open_timer(TIMER_UPDATE_CHAT);

	if (gtd->chat)
	{
		for (buddy = gtd->chat->next ; buddy ; buddy = buddy_next)
		{
			buddy_next = buddy->next;

			if (buddy->timeout && buddy->timeout < gtd->time)
			{
				chat_socket_printf(buddy, "<CHAT> Connection timed out.");

				close_chat(buddy, TRUE);
			}
		}

		if (gtd->chat->paste_time && gtd->chat->paste_time < gtd->utime)
		{
			chat_paste(NULL, NULL);
		}
	}
	close_timer(TIMER_UPDATE_CHAT);
}


void terminal_update(void)
{
	struct session *ses;

	open_timer(TIMER_UPDATE_TERMINAL);

	for (ses = gts ; ses ; ses = ses->next)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_UPDATEVTMAP))
		{
			DEL_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);

			show_vtmap(ses);

			check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "MAP UPDATED VTMAP");
		}
	}
	fflush(stdout);

	close_timer(TIMER_UPDATE_TERMINAL);
}

void memory_update(void)
{
	open_timer(TIMER_UPDATE_MEMORY);

	while (gtd->dispose_next)
	{
		dispose_session(gtd->dispose_next);
	}

	close_timer(TIMER_UPDATE_MEMORY);
}

void time_update(void)
{
	static char str_sec[9], str_min[9], str_hour[9], str_wday[9], str_mday[9], str_mon[9], str_year[9];

	static struct tm old_calendar;

	gtd->time = time(NULL);

	gtd->calendar = *localtime(&gtd->time);

	open_timer(TIMER_UPDATE_TIME);

	// Initialize on the first call.

	if (old_calendar.tm_year == 0)
	{
		old_calendar.tm_sec  = gtd->calendar.tm_sec;
		old_calendar.tm_min  = gtd->calendar.tm_min;
		old_calendar.tm_hour = gtd->calendar.tm_hour;
		old_calendar.tm_wday = gtd->calendar.tm_wday;
		old_calendar.tm_mday = gtd->calendar.tm_mday;
		old_calendar.tm_mon  = gtd->calendar.tm_mon;
		old_calendar.tm_year = gtd->calendar.tm_year;

		strftime(str_sec,  9, "%S", &gtd->calendar);
		strftime(str_min,  9, "%M", &gtd->calendar);
		strftime(str_hour, 9, "%H", &gtd->calendar);
		strftime(str_wday, 9, "%w", &gtd->calendar);
		strftime(str_mday, 9, "%d", &gtd->calendar);
		strftime(str_mon,  9, "%m", &gtd->calendar);
		strftime(str_year, 9, "%Y", &gtd->calendar);

		return;
	}

	if (gtd->calendar.tm_sec == old_calendar.tm_sec)
	{
		goto time_event_end;
	}

	strftime(str_min, 9, "%S", &gtd->calendar);
	old_calendar.tm_sec = gtd->calendar.tm_sec;

	if (gtd->calendar.tm_min == old_calendar.tm_min)
	{
		goto time_event_sec;
	}

	strftime(str_min, 9, "%M", &gtd->calendar);
	old_calendar.tm_min = gtd->calendar.tm_min;

	if (gtd->calendar.tm_hour == old_calendar.tm_hour)
	{
		goto time_event_min;
	}

	strftime(str_hour, 9, "%H", &gtd->calendar);
	old_calendar.tm_hour = gtd->calendar.tm_hour;

	if (gtd->calendar.tm_mday == old_calendar.tm_mday)
	{
		goto time_event_hour;
	}

	strftime(str_wday, 9, "%w", &gtd->calendar);
	old_calendar.tm_wday = gtd->calendar.tm_wday;

	strftime(str_mday, 9, "%d", &gtd->calendar);
	old_calendar.tm_mday = gtd->calendar.tm_mday;

	if (gtd->calendar.tm_mon == old_calendar.tm_mon)
	{
		goto time_event_mday;
	}

	strftime(str_mon, 9, "%m", &gtd->calendar);
	old_calendar.tm_mon = gtd->calendar.tm_mon;

	if (gtd->calendar.tm_year == old_calendar.tm_year)
	{
		goto time_event_mon;
	}

	strftime(str_year, 9, "%Y", &gtd->calendar);
	old_calendar.tm_year = gtd->calendar.tm_year;

	check_all_events(NULL, SUB_ARG, 0, 7, "YEAR", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, SUB_ARG, 1, 7, "YEAR %s", str_year, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);


	time_event_mon:

	check_all_events(NULL, SUB_ARG, 0, 7, "MONTH", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, SUB_ARG, 1, 7, "MONTH %s", str_mon, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);


	time_event_mday:

	check_all_events(NULL, SUB_ARG, 0, 7, "WEEK", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, SUB_ARG, 1, 7, "WEEK %s", str_wday, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);

	check_all_events(NULL, SUB_ARG, 2, 7, "DATE %s-%s", str_mon, str_mday, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);

	check_all_events(NULL, SUB_ARG, 0, 7, "DAY", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, SUB_ARG, 1, 7, "DAY %s", str_mday, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);


	time_event_hour:

	check_all_events(NULL, SUB_ARG, 0, 7, "HOUR", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, SUB_ARG, 1, 7, "HOUR %s", str_hour, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);


	time_event_min:

	check_all_events(NULL, SUB_ARG, 4, 7, "DATE %s-%s %s:%s", str_mon, str_mday, str_hour, str_min, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);

	check_all_events(NULL, SUB_ARG, 2, 7, "TIME %s:%s", str_hour, str_min, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);

	check_all_events(NULL, SUB_ARG, 0, 7, "MINUTE", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, SUB_ARG, 1, 7, "MINUTE %s", str_min, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);


	time_event_sec:

	old_calendar.tm_sec = gtd->calendar.tm_sec;

	check_all_events(NULL, SUB_ARG, 3, 7, "TIME %s:%s:%s", str_hour, str_min, str_sec, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);

	check_all_events(NULL, SUB_ARG, 0, 7, "SECOND", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, SUB_ARG, 1, 7, "SECOND %s", str_sec, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);

	time_event_end:

	close_timer(TIMER_UPDATE_TIME);
}


void show_cpu(struct session *ses)
{
	long long total_cpu;
	int timer;

	tintin_printf2(ses, "Section                           Time (usec)    Freq (msec)  %%Prog         %%CPU");

	tintin_printf2(ses, "");

	for (total_cpu = timer = 0 ; timer < TIMER_CPU ; timer++)
	{
		total_cpu += display_timer(ses, timer);
	}

	tintin_printf2(ses, "");

	tintin_printf2(ses, "Unknown CPU Usage:             %7.3f percent", (gtd->total_io_exec - total_cpu) * 100.0 / (gtd->total_io_delay + gtd->total_io_exec));
	tintin_printf2(ses, "Average CPU Usage:             %7.3f percent", (gtd->total_io_exec)             * 100.0 / (gtd->total_io_delay + gtd->total_io_exec));
}


long long display_timer(struct session *ses, int timer)
{
	long long total_usage, indicated_usage;

	total_usage = gtd->total_io_exec + gtd->total_io_delay;

	if (total_usage == 0)
	{
		return 0;
	}

	if (gtd->timer[timer][1] == 0 || gtd->timer[timer][4] == 0)
	{
		return 0;
	}

	indicated_usage = gtd->timer[timer][0] / gtd->timer[timer][1] * gtd->timer[timer][4];

	tintin_printf2(ses, "%-30s%8lld       %8lld      %8.2f     %8.3f",
		timer_table[timer].name,
		gtd->timer[timer][0] / gtd->timer[timer][1],
		gtd->timer[timer][3] / gtd->timer[timer][4] / 1000,
		100.0 * (double) indicated_usage / (double) gtd->total_io_exec,
		100.0 * (double) indicated_usage / (double) total_usage);

	return indicated_usage;
}


void open_timer(int timer)
{
	struct timeval last_time;
	long long current_time;

	gettimeofday(&last_time, NULL);

	current_time = (long long) last_time.tv_usec + 1000000LL * (long long) last_time.tv_sec;

	if (gtd->timer[timer][2] == 0)
	{
		gtd->timer[timer][2] = current_time ;
	}
	else
	{
		gtd->timer[timer][3] += current_time - gtd->timer[timer][2];
		gtd->timer[timer][2]  = current_time;
		gtd->timer[timer][4] ++;
	}
}


void close_timer(int timer)
{
	struct timeval last_time;
	long long current_time;

	gettimeofday(&last_time, NULL);

	current_time = (long long) last_time.tv_usec + 1000000LL * (long long) last_time.tv_sec;

	gtd->timer[timer][0] += (current_time - gtd->timer[timer][2]);
	gtd->timer[timer][1] ++;
}
