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
*                      coded by Igor van den Hoven 2006                       *
******************************************************************************/

#include "tintin.h"

#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/un.h>

extern void update_input(void);
extern void update_sessions(void);
extern void update_daemon(void);
extern void update_chat(void);
extern void update_port(void);
extern void tick_update(void);
extern void delay_update(void);
extern void path_update(void);
extern void packet_update(void);
extern void terminal_update(void);
extern void memory_update(void);
extern void time_update(void);

extern long long display_timer(struct session *ses, int timer);
extern void open_timer(int timer);
extern void close_timer(int timer);

void mainloop(void)
{
	static struct timeval start_time, end_time, wait_time;
	static struct pulse_type pulse;
	static int wait_time_val, span_time_val;

	pulse.update_input    =  0 + PULSE_UPDATE_INPUT;
	pulse.update_sessions =  0 + PULSE_UPDATE_SESSIONS;
	pulse.update_delays   =  0 + PULSE_UPDATE_DELAYS;
	pulse.update_daemon   =  0 + PULSE_UPDATE_DAEMON;
	pulse.update_chat     =  2 + PULSE_UPDATE_CHAT;
	pulse.update_port     =  2 + PULSE_UPDATE_PORT;
	pulse.update_ticks    =  3 + PULSE_UPDATE_TICKS;
	pulse.update_paths    =  3 + PULSE_UPDATE_PATHS;
	pulse.update_packets  =  4 + PULSE_UPDATE_PACKETS;
	pulse.update_terminal =  6 + PULSE_UPDATE_TERMINAL;
	pulse.update_memory   =  7 + PULSE_UPDATE_MEMORY;
	pulse.update_time     =  8 + PULSE_UPDATE_TIME;

	push_call("mainloop()");

	while (TRUE)
	{
		gettimeofday(&start_time, NULL);

		gtd->total_io_exec  += span_time_val;
		gtd->total_io_delay += wait_time_val;

		if (gtd->memory->stack_len > 1000)
		{
			tintin_printf2(NULL, "debug: memory_stack leak detected.\n");
			gtd->memory->stack_len = 0;
		}

		if (--pulse.update_delays == 0)
		{
			pulse.update_delays = PULSE_UPDATE_DELAYS;

			delay_update();
		}

		if (--pulse.update_input == 0)
		{
			open_timer(TIMER_UPDATE_INPUT);

			pulse.update_input = PULSE_UPDATE_INPUT;

			update_input();

			close_timer(TIMER_UPDATE_INPUT);
		}

		if (--pulse.update_sessions == 0)
		{
			pulse.update_sessions = PULSE_UPDATE_SESSIONS;

			update_sessions();
		}

		if (--pulse.update_daemon == 0)
		{
			pulse.update_daemon = PULSE_UPDATE_DAEMON;

			update_daemon();
		}

		if (--pulse.update_chat == 0)
		{
			pulse.update_chat = PULSE_UPDATE_CHAT;

			update_chat();
		}	

		if (--pulse.update_port == 0)
		{
			pulse.update_port = PULSE_UPDATE_PORT;

			update_port();
		}	

		if (--pulse.update_ticks == 0)
		{
			pulse.update_ticks = PULSE_UPDATE_TICKS;

			tick_update();
		}

		if (--pulse.update_paths == 0)
		{
			pulse.update_paths = PULSE_UPDATE_PATHS;

			path_update();
		}


		if (--pulse.update_packets == 0)
		{
			pulse.update_packets = PULSE_UPDATE_PACKETS;

			packet_update();
		}

		if (--pulse.update_terminal == 0)
		{
			pulse.update_terminal = PULSE_UPDATE_TERMINAL;

			terminal_update();
		}

		if (--pulse.update_memory == 0)
		{
			pulse.update_memory = PULSE_UPDATE_MEMORY;

			memory_update();
		}

		if (--pulse.update_time == 0)
		{
			pulse.update_time = PULSE_UPDATE_TIME;

			time_update();
		}

		gettimeofday(&end_time, NULL);

		if (start_time.tv_sec == end_time.tv_sec)
		{
			span_time_val = end_time.tv_usec - start_time.tv_usec;
		}
		else
		{
			span_time_val = (end_time.tv_sec * 1000000LL + end_time.tv_usec) - (start_time.tv_sec * 1000000LL + start_time.tv_usec);
		}

		wait_time_val = 1000000 / PULSE_PER_SECOND - span_time_val;

		wait_time.tv_usec = 1000000 / PULSE_PER_SECOND - span_time_val;

		if (wait_time_val > 0)
		{
			wait_time.tv_usec = wait_time_val;

			select(0, NULL, NULL, NULL, &wait_time);
		}
		else
		{
			wait_time_val = 0;
		}
	}
	pop_call();
	return;
}

void update_input(void)
{
	fd_set read_fd;
	static struct timeval timeout;
	static int sleep;

	if (gtd->detach_port)
	{
		return;

		if (gtd->detach_sock)
		{
			while (TRUE)
			{
				FD_ZERO(&read_fd);

				FD_SET(gtd->detach_sock, &read_fd);
			
				if (select(FD_SETSIZE, &read_fd, NULL, NULL, &timeout) <= 0)
				{
					break;
				}

				if (!FD_ISSET(gtd->detach_sock, &read_fd))
				{
					break;
				}
//				process_input();
			}
		}
		return;
	}

	if (gtd->time_input + 10 < gtd->time)
	{
		if (sleep < 10)
		{
			sleep++;
			return;
		}
		sleep = 0;
	}

	while (TRUE)
	{
		FD_ZERO(&read_fd);

		FD_SET(STDIN_FILENO, &read_fd);

		if (select(FD_SETSIZE, &read_fd, NULL, NULL, &timeout) <= 0)
		{
			break;
		}

		if (!FD_ISSET(STDIN_FILENO, &read_fd))
		{
			break;
		}

		process_input();

		SET_BIT(gtd->flags, TINTIN_FLAG_DISPLAYUPDATE);

		if (gtd->detach_port)
		{
			return;
		}
	}
	return;
}

void update_sessions(void)
{
	fd_set read_fd, error_fd;
	static struct timeval timeout;
	static int sleep;
	struct session *ses;
	int rv;

	if (gtd->time_session + 10 < gtd->time)
	{
		if (sleep < 10)
		{
			sleep++;
			return;
		}
		sleep = 0;
	}

	push_call("update_sessions(void)");

	open_timer(TIMER_UPDATE_SESSIONS);

	if (gts->next)
	{
		FD_ZERO(&read_fd);
		FD_ZERO(&error_fd);

		for (ses = gts->next ; ses ; ses = gtd->update)
		{
			gtd->update = ses->next;

			if (HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
			{
				while (TRUE)
				{
					FD_SET(ses->socket, &read_fd);
					FD_SET(ses->socket, &error_fd);

					rv = select(FD_SETSIZE, &read_fd, NULL, &error_fd, &timeout);

					if (rv < 0)
					{
						break; // bug report after removal.

						syserr_printf(ses, "update_sessions: select:");

						cleanup_session(ses);

						gtd->mud_output_len = 0;

						break;
					}

					if (rv == 0)
					{
						break;
					}

					if (FD_ISSET(ses->socket, &read_fd))
					{
						if (read_buffer_mud(ses) == FALSE)
						{
							readmud(ses);

							cleanup_session(ses);

							gtd->mud_output_len = 0;

							break;
						}
					}

					if (FD_ISSET(ses->socket, &error_fd))
					{
						FD_CLR(ses->socket, &read_fd);

						cleanup_session(ses);

						gtd->mud_output_len = 0;

						break;
					}
				}

				gtd->time_session = gtd->time;

				if (gtd->mud_output_len)
				{
					readmud(ses);
				}
			}
		}
	}

	for (ses = gts ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		if (ses->check_output == 0 && HAS_BIT(ses->flags, SES_FLAG_PRINTLINE))
		{
			DEL_BIT(ses->flags, SES_FLAG_PRINTLINE);
			SET_BIT(ses->flags, SES_FLAG_PRINTBUFFER);

			buffer_end(ses, "");

			DEL_BIT(ses->flags, SES_FLAG_PRINTBUFFER);
		}
		if (HAS_BIT(ses->flags, SES_FLAG_BUFFERUPDATE))
		{
			check_all_events(ses, SUB_ARG|SUB_SIL, 0, 0, "BUFFER UPDATE");

			DEL_BIT(ses->flags, SES_FLAG_BUFFERUPDATE);
		}
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_DISPLAYUPDATE))
	{
		check_all_events(gtd->ses, SUB_ARG|SUB_SIL, 0, 0, "DISPLAY UPDATE");

		DEL_BIT(gtd->flags, TINTIN_FLAG_DISPLAYUPDATE);

//		if (gtd->detach_port == 0)
		{
			fflush(stdout);
		}
	}

	close_timer(TIMER_UPDATE_SESSIONS);

	pop_call();
	return;
}

void update_daemon(void)
{
	fd_set read_fd, error_fd;
	static struct timeval timeout;
	socklen_t len;
	int rv;

	if (gtd->detach_port)
	{
		if (TRUE)
		{
			FD_ZERO(&read_fd);

			FD_SET(gtd->detach_port, &read_fd);

			rv = select(FD_SETSIZE, &read_fd, NULL, NULL, &timeout);

			if (rv > 0)
			{
				if (FD_ISSET(gtd->detach_port, &read_fd))
				{
					if (gtd->detach_sock)
					{
						tintin_printf2(gtd->ses, "#DAEMON UPDATE: ANOTHER CONNECTION IS TAKING OVER {%s}.", gtd->detach_file);
						kill((pid_t) gtd->detach_sock, SIGTSTP);
						close(gtd->detach_sock);
					}

					gtd->detach_sock = accept(gtd->detach_port, 0, 0);

					if (gtd->detach_sock < 0)
					{
						syserr_printf(gtd->ses, "update_daemon: detach_port: accept");
						
						gtd->detach_sock = close(gtd->detach_sock);
						
						goto attach;
					}

					if (fcntl(gtd->detach_sock, F_SETFL, O_NDELAY|O_NONBLOCK) == -1)
					{
						syserr_printf(gtd->ses, "update_daemon: detach_port: fcntl O_NDELAY|O_NONBLOCK");

						gtd->detach_sock = close(gtd->detach_sock);
						
						goto attach;
					}

					len = sizeof(struct process_data);

					if (getsockopt(gtd->detach_sock, SOL_SOCKET, SO_PEERCRED, &gtd->detach_info, &len) == -1)
					{
						syserr_printf(gtd->ses, "update_daemon: getsockopt:");

						gtd->detach_sock = close(gtd->detach_sock);

						goto attach;
					}

					if (geteuid() != gtd->detach_info.uid)
					{
						tintin_printf2(gtd->ses, "#DAEMON UPDATE: YOUR UID IS %d WHILE {%s} HAS UID {%d}.", geteuid(), gtd->detach_file, gtd->detach_info.uid);

						gtd->detach_sock = close(gtd->detach_sock);

						goto attach;
					}

//					tintin_printf2(gtd->ses, "sock=%d pid=%d, euid=%d, egid=%d", gtd->detach_port, getpid(), geteuid(), getegid());
//					tintin_printf2(gtd->ses, "sock=%d pid=%d, euid=%d, egid=%d", gtd->detach_sock, gtd->detach_info.pid, gtd->detach_info.uid, gtd->detach_info.gid);

					winch_handler(0);

					dirty_screen(gtd->ses);

					tintin_printf2(gtd->ses, "#DAEMON UPDATE: ATTACHED {%s} TO PID {%d}.", gtd->detach_file, gtd->detach_info.pid);
				}
			}
			else if (rv < 0)
			{
				if (errno != EINTR)
				{
					syserr_printf(gtd->ses, "update_daemon: select:");
				}
			}
		}

		if (gtd->detach_sock > 0)
		{
			while (gtd->detach_sock)
			{
				FD_ZERO(&read_fd);
//				FD_ZERO(&write_fd);
				FD_ZERO(&error_fd);

				FD_SET(gtd->detach_sock, &read_fd);
//				FD_SET(gtd->detach_sock, &write_fd);
				FD_SET(gtd->detach_sock, &error_fd);

				rv = select(FD_SETSIZE, &read_fd, NULL, &error_fd, &timeout);

//				tintin_printf2(gtd->ses, "debug: rv: %d (%d,%d,%d)\n", rv, FD_ISSET(gtd->detach_sock, &read_fd), FD_ISSET(gtd->detach_sock, &write_fd), FD_ISSET(gtd->detach_sock, &error_fd));

				if (rv == 0)
				{
					break;
				}
				else if (rv < 0)
				{
					FD_CLR(gtd->detach_sock, &read_fd);

					gtd->detach_sock = close(gtd->detach_sock);

	                                syserr_printf(gtd->ses, "update_daemon: detach_sock: select:");

	                                break;
				}
				else if (rv > 0)
				{
					if (FD_ISSET(gtd->detach_sock, &error_fd))
					{
						FD_CLR(gtd->detach_sock, &read_fd);

						gtd->detach_sock = close(gtd->detach_sock);

						show_error(gtd->ses, LIST_COMMAND, "update_daemon: detach_sock: error_fd");

						goto attach;
					}

/*					if (!FD_ISSET(gtd->detach_sock, &write_fd))
					{
						FD_CLR(gtd->detach_sock, &read_fd);

						gtd->detach_sock = close(gtd->detach_sock);

						show_error(gtd->ses, LIST_COMMAND, "update_daemon: detach_sock: write_fd");

						goto attach;
					}
*/
					if (!FD_ISSET(gtd->detach_sock, &read_fd))
					{
//						gtd->detach_sock = close(gtd->detach_sock); // experimental
						break;
					}
					process_input();
				}
			}
		}
	}

	attach:

	if (gtd->attach_sock)
	{
		FD_ZERO(&read_fd);
		FD_ZERO(&error_fd);

		FD_SET(gtd->attach_sock, &read_fd);
		FD_SET(gtd->attach_sock, &error_fd);

		rv = select(FD_SETSIZE, &read_fd, NULL, &error_fd, &timeout);

		if (rv < 0)
		{
			gtd->attach_sock = close(gtd->attach_sock);
	
			show_message(gtd->ses, LIST_COMMAND, "#DAEMON UPDATE: UNATTACHING {%s} DUE TO SELECT ERROR.", gtd->attach_file);
		}
		else if (rv > 0)
		{
			if (FD_ISSET(gtd->attach_sock, &read_fd))
			{
				char buffer[BUFFER_SIZE];

				rv = read(gtd->attach_sock, buffer, BUFFER_SIZE -1);

				if (rv <= 0)
				{
					gtd->attach_sock = close(gtd->attach_sock);

					winch_handler(0);

					show_message(gtd->ses, LIST_COMMAND, "#DAEMON UPDATE: UNATTACHING {%s}.", gtd->attach_file);
				}
				else
				{
					buffer[rv] = 0;
/*
					if (buffer[rv - 1] == (char) 255)
					{
						gtd->attach_sock = close(gtd->attach_sock);

						show_message(gtd->ses, LIST_COMMAND, "\n#DAEMON {%s} SIGTSTP: UNATTACHING.", gtd->attach_file);

						dirty_screen(gtd->ses);

						return;
					}
*/
					if (gtd->level->quiet == 0)
					{
						printf("%s", buffer);
					}

					if (FD_ISSET(gtd->attach_sock, &error_fd))
					{
						FD_CLR(gtd->attach_sock, &read_fd);

						gtd->attach_sock = close(gtd->attach_sock);

						show_message(gtd->ses, LIST_COMMAND, "#DAEMON UPDATE: UNATTACHING {%s} DUE TO EXCEPTION ERROR.", gtd->attach_file);
					}
				}
			}
			else
			{
				return;
			}
		}
		fflush(stdout);
	}

}

void update_chat(void)
{
	fd_set read_fd, write_fd, error_fd;
	static struct timeval timeout;
	struct chat_data *buddy, *buddy_next;
	int rv;

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

		FD_ZERO(&read_fd);
		FD_ZERO(&write_fd);
		FD_ZERO(&error_fd);

		FD_SET(gtd->chat->fd, &read_fd);

		for (buddy = gtd->chat->next ; buddy ; buddy = buddy->next)
		{
			FD_SET(buddy->fd, &read_fd);
			FD_SET(buddy->fd, &write_fd);
			FD_SET(buddy->fd, &error_fd);
		}

		rv = select(FD_SETSIZE, &read_fd, &write_fd, &error_fd, &timeout);

		if (rv <= 0)
		{
			if (rv == 0 || errno == EINTR)
			{
				goto update_chat_end;
			}
			syserr_fatal(-1, "update_chat: select");
		}
		process_chat_connections(&read_fd, &write_fd, &error_fd);

	}
	update_chat_end:

	close_timer(TIMER_UPDATE_CHAT);
}

void update_port(void)
{
	struct session *ses;
	fd_set read_fd, write_fd, error_fd;
	static struct timeval timeout;
	struct port_data *buddy;
	int rv;

	open_timer(TIMER_UPDATE_PORT);

	for (ses = gts->next ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		if (ses->port)
		{
			FD_ZERO(&read_fd);
			FD_ZERO(&write_fd);
			FD_ZERO(&error_fd);

			FD_SET(ses->port->fd, &read_fd);

			for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
			{
				FD_SET(buddy->fd, &read_fd);
				FD_SET(buddy->fd, &write_fd);
				FD_SET(buddy->fd, &error_fd);
			}

			rv = select(FD_SETSIZE, &read_fd, &write_fd, &error_fd, &timeout);

			if (rv <= 0)
			{
				if (rv == 0 || errno == EINTR)
				{
					continue;
				}
				syserr_fatal(-1, "update_port: select");
			}

			process_port_connections(ses, &read_fd, &write_fd, &error_fd);
		}
	}

	close_timer(TIMER_UPDATE_PORT);
}

void tick_update(void)
{
	struct session *ses;
	struct listnode *node;
	struct listroot *root;
	char buf[BUFFER_SIZE];

	open_timer(TIMER_UPDATE_TICKS);

	utime();

//	for (ses = gts->next ; ses ; ses = gtd->update)
	for (ses = gts ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		root = ses->list[LIST_TICKER];

		for (root->update = 0 ; root->update < root->used ; root->update++)
		{
			node = root->list[root->update];

			if (node->val64 == 0)
			{
				node->val64 = gtd->utime + (long long) (get_number(ses, node->arg3) * 1000000LL);

				show_info(ses, LIST_TICKER, "#INFO TICK {%s} INITIALIZED WITH TIMESTAMP {%lld}", node->arg1, node->val64);
			}

			if (node->val64 <= gtd->utime)
			{
				node->val64 += (long long) (get_number(ses, node->arg3) * 1000000LL);

				show_info(ses, LIST_TICKER, "#INFO TICK {%s} INITIALIZED WITH TIMESTAMP {%lld}", node->arg1, node->val64);

				if (!HAS_BIT(root->flags, LIST_FLAG_IGNORE))
				{
					show_debug(ses, LIST_TICKER, "#DEBUG TICKER {%s}", node->arg2);

					if (node->shots && --node->shots == 0)
					{
						strcpy(buf, node->arg2);

						delete_node_list(ses, LIST_TICKER, node);

						script_driver(ses, LIST_TICKER, buf);
					}
					else
					{
						script_driver(ses, LIST_TICKER, node->arg2);
					}
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

			if (node->val64 == 0)
			{
				node->val64 = gtd->utime + (long long) (get_number(ses, node->arg3) * 1000000LL);

				show_info(ses, LIST_DELAY, "#INFO DELAY {%s} INITIALIZED WITH TIMESTAMP {%lld}", node->arg1, node->val64);
			}

			if (node->val64 <= gtd->utime)
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

void path_update(void)
{
	struct session *ses;
	struct listnode *node;
	struct listroot *root;

	open_timer(TIMER_UPDATE_PATHS);

	for (ses = gts ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		root = ses->list[LIST_PATH];

		while (root->update < root->used)
		{
			node = root->list[root->update];

			if (node->val64 > 0 && node->val64 <= gtd->utime)
			{
				root->update++;

				node->val64 = 0;

				show_debug(ses, LIST_COMMAND, "#DEBUG PATH {%s}", node->arg1);

				script_driver(ses, LIST_COMMAND, node->arg1);
			}
			break;
		}
	}
	close_timer(TIMER_UPDATE_PATHS);
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

				goto_pos(ses, ses->split->bot_row, 1);
			}

			SET_BIT(ses->flags, SES_FLAG_READMUD);

			strcpy(result, ses->more_output);

			ses->more_output[0] = 0;

			if (HAS_BIT(ses->charset, CHARSET_FLAG_ALL_TOUTF8))
			{
				char buf[BUFFER_SIZE];

				all_to_utf8(ses, result, buf);

				process_mud_output(ses, buf, TRUE);
			}
			else
			{
				process_mud_output(ses, result, TRUE);
			}
			DEL_BIT(ses->flags, SES_FLAG_READMUD);

			if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
			{
				restore_pos(ses);
			}
		}
	}
	close_timer(TIMER_UPDATE_PACKETS);
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

	strftime(str_sec, 9, "%S", &gtd->calendar);
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

	check_all_events(NULL, SUB_ARG|SUB_SIL, 3, 7, "TIME %s:%s:%s", str_hour, str_min, str_sec, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);

	check_all_events(NULL, SUB_ARG|SUB_SIL, 0, 7, "SECOND", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, SUB_ARG|SUB_SIL, 1, 7, "SECOND %s", str_sec, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);

	time_event_end:

	close_timer(TIMER_UPDATE_TIME);
}


void show_cpu(struct session *ses)
{
	long long total_cpu = 0;
	int timer;

	tintin_printf2(ses, "Section                           Time (usec)    Freq (msec)  %%Prog         %%CPU");

	tintin_printf2(ses, "");

	for (timer = 0 ; timer < TIMER_CPU ; timer++)
	{
		total_cpu += display_timer(ses, timer);
	}

	tintin_printf2(ses, "");

	tintin_printf2(ses, "Unknown CPU Usage:             %7.3f percent", (gtd->total_io_exec - total_cpu) * 100.0 / (gtd->total_io_delay + gtd->total_io_exec));
	tintin_printf2(ses, "Average CPU Usage:             %7.3f percent", (gtd->total_io_exec)             * 100.0 / (gtd->total_io_delay + gtd->total_io_exec));
//	tintin_printf2(ses, "Total   CPU Usecs:             %10ld", gtd->total_io_exec);
//	tintin_printf2(ses, "Total   CPU Delay:             %10ld", gtd->total_io_delay);

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

//	indicated_usage = gtd->timer[timer][0] / gtd->timer[timer][1] * gtd->timer[timer][4];

	indicated_usage = gtd->timer[timer][0];

	tintin_printf2(ses, "%-30s%8lld       %8lld      %8.2f     %8.3f",
		timer_table[timer].name,
		gtd->timer[timer][0] / gtd->timer[timer][1],
		gtd->timer[timer][3] / gtd->timer[timer][4] / 1000,
		(double) (100000 * indicated_usage / gtd->total_io_exec) / 1000.0,
		(double) (100000 * indicated_usage / total_usage) / 1000.0
		);

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
		gtd->timer[timer][2] = current_time;
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
