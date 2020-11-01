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
#include <sys/socket.h>
#include <signal.h>

#define TIMER_UPDATE_INPUT               0
#define TIMER_UPDATE_SESSIONS            1
#define TIMER_UPDATE_DELAYS              2
#define TIMER_UPDATE_DAEMON              3
#define TIMER_UPDATE_CHAT                4
#define TIMER_UPDATE_PORT                5
#define TIMER_UPDATE_TICKS               6
#define TIMER_UPDATE_PATHS               7
#define TIMER_UPDATE_PACKETS             8
#define TIMER_UPDATE_TERMINAL            9
#define TIMER_UPDATE_TIME               10
#define TIMER_UPDATE_MEMORY             11
#define TIMER_STALL_PROGRAM             12
#define TIMER_CPU                       13


#define PULSE_PER_SECOND               100

#define PULSE_UPDATE_INPUT               1
#define PULSE_UPDATE_SESSIONS            1
#define PULSE_UPDATE_DELAYS              1
#define PULSE_UPDATE_DAEMON              1
#define PULSE_UPDATE_CHAT               10
#define PULSE_UPDATE_PORT               10
#define PULSE_UPDATE_TICKS              10
#define PULSE_UPDATE_PATHS              10
#define PULSE_UPDATE_PACKETS            10
#define PULSE_UPDATE_TERMINAL           10
#define PULSE_UPDATE_MEMORY             10
#define PULSE_UPDATE_TIME               10

#define TIMER_UPDATE_INPUT               0
#define TIMER_UPDATE_SESSIONS            1
#define TIMER_UPDATE_DELAYS              2
#define TIMER_UPDATE_DAEMON              3
#define TIMER_UPDATE_CHAT                4
#define TIMER_UPDATE_PORT                5
#define TIMER_UPDATE_TICKS               6
#define TIMER_UPDATE_PATHS               7
#define TIMER_UPDATE_PACKETS             8
#define TIMER_UPDATE_TERMINAL            9
#define TIMER_UPDATE_TIME               10
#define TIMER_UPDATE_MEMORY             11
#define TIMER_STALL_PROGRAM             12
#define TIMER_CPU                       13

long long cpu_timer[TIMER_CPU][5];

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
	static struct timeval wait_time;
	static struct pulse_type pulse;
	static int wait_time_val, span_time_val;
	static unsigned long long start_utime, end_utime;

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
		start_utime = utime();

		gtd->total_io_exec  += span_time_val;
		gtd->total_io_delay += wait_time_val;

		if (gtd->memory->stack_len > 0)
		{
			tintin_printf2(NULL, "\e[1;31merror: memory_stack leak detected.\n");

			gtd->memory->debug_len = gtd->memory->debug_max;
			dump_stack();

			gtd->memory->debug_len = 1;
			gtd->memory->stack_len = 0;
		}

		if (gtd->memory->debug_len > 1)
		{
			tintin_printf2(NULL, "\e[1;31merror: debug_stack leak detected.\n");

			gtd->memory->debug_len = gtd->memory->debug_max;
			dump_stack();

			gtd->memory->debug_len = 1;
		}

		if (--pulse.update_delays == 0)
		{
			open_timer(TIMER_UPDATE_DELAYS);

			pulse.update_delays = PULSE_UPDATE_DELAYS;

			delay_update();

			close_timer(TIMER_UPDATE_DELAYS);
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
			open_timer(TIMER_UPDATE_SESSIONS);

			pulse.update_sessions = PULSE_UPDATE_SESSIONS;

			update_sessions();

			close_timer(TIMER_UPDATE_SESSIONS);
		}

		if (--pulse.update_daemon == 0)
		{
			open_timer(TIMER_UPDATE_DAEMON);

			pulse.update_daemon = PULSE_UPDATE_DAEMON;

			update_daemon();
			
			close_timer(TIMER_UPDATE_DAEMON);
		}

		if (--pulse.update_chat == 0)
		{
			open_timer(TIMER_UPDATE_CHAT);

			pulse.update_chat = PULSE_UPDATE_CHAT;

			update_chat();

			close_timer(TIMER_UPDATE_CHAT);
		}

		if (--pulse.update_port == 0)
		{
			open_timer(TIMER_UPDATE_PORT);

			pulse.update_port = PULSE_UPDATE_PORT;

			update_port();

			close_timer(TIMER_UPDATE_PORT);
		}	

		if (--pulse.update_ticks == 0)
		{
			open_timer(TIMER_UPDATE_TICKS);

			pulse.update_ticks = PULSE_UPDATE_TICKS;

			tick_update();

			close_timer(TIMER_UPDATE_TICKS);
		}

		if (--pulse.update_paths == 0)
		{
			open_timer(TIMER_UPDATE_PATHS);

			pulse.update_paths = PULSE_UPDATE_PATHS;

			path_update();

			close_timer(TIMER_UPDATE_PATHS);
		}


		if (--pulse.update_packets == 0)
		{
			open_timer(TIMER_UPDATE_PACKETS);

			pulse.update_packets = PULSE_UPDATE_PACKETS;

			packet_update();

			close_timer(TIMER_UPDATE_PACKETS);
		}

		if (--pulse.update_terminal == 0)
		{
			open_timer(TIMER_UPDATE_TERMINAL);

			pulse.update_terminal = PULSE_UPDATE_TERMINAL;

			terminal_update();

			close_timer(TIMER_UPDATE_TERMINAL);
		}

		if (--pulse.update_memory == 0)
		{
			open_timer(TIMER_UPDATE_MEMORY);

			pulse.update_memory = PULSE_UPDATE_MEMORY;

			memory_update();

			close_timer(TIMER_UPDATE_MEMORY);
		}

		if (--pulse.update_time == 0)
		{

			open_timer(TIMER_UPDATE_TIME);

			pulse.update_time = PULSE_UPDATE_TIME;

			time_update();

			close_timer(TIMER_UPDATE_TIME);
		}

		end_utime = utime();

		span_time_val = end_utime - start_utime;

		wait_time_val = 1000000 / PULSE_PER_SECOND - span_time_val;

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
	static unsigned char sleep;

	if (gtd->time_input + 60 < gtd->time)
	{
		if (sleep < 10)
		{
			sleep++;
			return;
		}
		sleep = 0;
	}

	if (gtd->detach_port)
	{
		return;
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

		gtd->time_input = gtd->time;

		process_input();

		fflush(stdout);

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
	static unsigned char sleep;
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

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_SESSIONUPDATE))
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_SESSIONUPDATE);

		for (ses = gts ; ses ; ses = gtd->update)
		{
			gtd->update = ses->next;

			if (HAS_BIT(ses->flags, SES_FLAG_PRINTLINE) && ses->check_output == 0)
			{
				DEL_BIT(ses->flags, SES_FLAG_PRINTLINE);

				SET_BIT(ses->flags, SES_FLAG_PRINTBUFFER);

				if (ses == gtd->ses)
				{
					if (HAS_BIT(ses->scroll->flags, SCROLL_FLAG_RESIZE))
					{
						buffer_refresh(ses, "", "", "");
					}
					else
					{
						print_scroll_region(ses);
					}
				}
				else
				{
					buffer_end(ses, "", "", "");
				}

				DEL_BIT(ses->flags, SES_FLAG_PRINTBUFFER);
			}

			if (HAS_BIT(ses->flags, SES_FLAG_BUFFERUPDATE))
			{
				check_all_events(ses, EVENT_FLAG_UPDATE, 0, 0, "BUFFER UPDATE");

				DEL_BIT(ses->flags, SES_FLAG_BUFFERUPDATE);
			}
		}
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_DISPLAYUPDATE))
	{
		check_all_events(gtd->ses, EVENT_FLAG_UPDATE, 0, 0, "DISPLAY UPDATE");

		DEL_BIT(gtd->flags, TINTIN_FLAG_DISPLAYUPDATE);

		fflush(stdout);
	}
}

void update_daemon(void)
{
	fd_set read_fd, error_fd;
	static struct timeval timeout;
	static unsigned char sleep;
	socklen_t len;
	int rv;

	if (gtd->time_daemon + 10 < gtd->time)
	{
		if (sleep < 10)
		{
			sleep++;

			return;
		}
		sleep = 0;
	}

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
					gtd->time_daemon = gtd->time;

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

					check_all_events(gtd->ses, EVENT_FLAG_SYSTEM, 0, 2, "DAEMON ATTACHED", gtd->detach_file, ntos(gtd->detach_info.pid));
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
					gtd->time_daemon = gtd->time;

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

				gtd->time_daemon = gtd->time;

				rv = read(gtd->attach_sock, buffer, BUFFER_SIZE -1);

				if (rv <= 0)
				{
					gtd->attach_sock = close(gtd->attach_sock);

					winch_handler(0);

					dirty_screen(gtd->ses);

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
				fflush(stdout); // used to be down
			}
			else
			{
				return;
			}
		}
//		fflush(stdout);
	}

}

void update_chat(void)
{
	fd_set read_fd, write_fd, error_fd;
	static struct timeval timeout;
	struct chat_data *buddy, *buddy_next;
	int rv;

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
				return;
			}
			syserr_fatal(-1, "update_chat: select");
		}
		process_chat_connections(&read_fd, &write_fd, &error_fd);

	}
}

void update_port(void)
{
	struct session *ses;
	fd_set read_fd, write_fd, error_fd;
	static struct timeval timeout;
	struct port_data *buddy;
	int rv;

	for (ses = gts->next ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		if (ses->port && ses->port->port)
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
}

void tick_update(void)
{
	struct session *ses;
	struct listnode *node;
	struct listroot *root;

	if (gtd->utime < gtd->utime_next_tick)
	{
		return;
	}

	gtd->utime_next_tick = gtd->utime + 1000000000;

	for (ses = gts ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		root = ses->list[LIST_TICKER];

		for (root->update = 0 ; root->update < root->used ; root->update++)
		{
			node = root->list[root->update];

			if (node->val64 == 0)
			{
				tintin_printf2(gtd->ses, "error: tick_update: node->val64 == 0");
			}

			if (node->val64 <= gtd->utime)
			{
				node->val64 = gtd->utime + (long long) (get_number(ses, node->arg3) * 1000000LL);

				show_info(ses, LIST_TICKER, "#INFO TICK {%s} INITIALIZED WITH TIMESTAMP {%lld}", node->arg1, node->val64);

				if (node->val64 < gtd->utime_next_tick)
				{
					gtd->utime_next_tick = node->val64;
				}

				if (!HAS_BIT(root->flags, LIST_FLAG_IGNORE))
				{
					show_debug(ses, LIST_TICKER, "#DEBUG TICKER {%s}", node->arg2);

					if (node->shots && --node->shots == 0)
					{
						delete_node_list(ses, LIST_TICKER, node);
					}
					script_driver(ses, LIST_TICKER, node->arg2);
				}
			}
			else
			{
				if (node->val64 < gtd->utime_next_tick)
				{
					gtd->utime_next_tick = node->val64;
				}
			}
		}
	}
}

void delay_update(void)
{
	struct session *ses;
	struct listnode *node;
	struct listroot *root;

	if (gtd->utime < gtd->utime_next_delay)
	{
		return;
	}

	gtd->utime_next_delay = gtd->utime + 1000000000;

	for (ses = gts ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		root = ses->list[LIST_DELAY];

		for (root->update = 0 ; root->update < root->used ; root->update++)
		{
			node = root->list[root->update];

			if (node->val64 <= gtd->utime)
			{
				show_debug(ses, LIST_DELAY, "#DEBUG DELAY {%s}", node->arg2);

				delete_node_list(ses, LIST_DELAY, node);

				script_driver(ses, LIST_DELAY, node->arg2);
			}
			else
			{
				if (node->val64 < gtd->utime_next_delay)
				{
					gtd->utime_next_delay = node->val64;
				}
				break;
			}
		}
	}
}

void path_update(void)
{
	struct session *ses;
	struct listnode *node;
	struct listroot *root;

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

				if (root->update == root->used)
				{
					check_all_events(ses, EVENT_FLAG_MAP, 0, 0, "END OF RUN");
				}
			}
			break;
		}
	}
}

void packet_update(void)
{
	struct session *ses;

	for (ses = gts->next ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		if (ses->check_output && gtd->utime > ses->check_output)
		{
			char result[STRING_SIZE];

			if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
			{
				save_pos(ses);

				goto_pos(ses, ses->split->bot_row, 1);
			}

			SET_BIT(ses->flags, SES_FLAG_READMUD);

			strcpy(result, ses->more_output);

			if (HAS_BIT(ses->charset, CHARSET_FLAG_ALL_TOUTF8))
			{
				all_to_utf8(ses, ses->more_output, result);
			}
			else
			{
				strcpy(result, ses->more_output);
			}
			str_cpy(&ses->more_output, "");

			process_mud_output(ses, result, TRUE);

			DEL_BIT(ses->flags, SES_FLAG_READMUD);

			if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
			{
				restore_pos(ses);
			}
		}

		if (HAS_BIT(ses->telopts, TELOPT_FLAG_UPDATENAWS))
		{
			client_send_sb_naws(ses, 0, NULL);
					
			DEL_BIT(ses->telopts, TELOPT_FLAG_UPDATENAWS);
		}
	}

	if (HAS_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLUPDATE))
	{
		int line = gtd->ses->scroll->line >= 0 ? gtd->ses->scroll->line : gtd->ses->scroll->used + 1;

		line = URANGE(1, line - get_scroll_rows(gtd->ses), gtd->ses->scroll->used);

		check_all_events(ses, EVENT_FLAG_UPDATE, 0, 2, "SCROLLBAR UPDATE", ntos(line), ntos(gtd->ses->scroll->used));

		DEL_BIT(gtd->screen->flags, SCREEN_FLAG_SCROLLUPDATE);

		print_stdout(0, 0, "\e[%d;%d#t", line, gtd->ses->scroll->used);
	}
}

void terminal_update(void)
{
	struct session *ses;

	for (ses = gts ; ses ; ses = ses->next)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_UPDATEVTMAP))
		{
			DEL_BIT(ses->flags, SES_FLAG_UPDATEVTMAP);

			show_vtmap(ses);

			check_all_events(ses, EVENT_FLAG_MAP, 0, 0, "MAP UPDATED VTMAP");
		}
	}
}

void memory_update(void)
{
	while (gtd->dispose_next)
	{
		dispose_session(gtd->dispose_next);
	}

	while (gtd->dispose_list->used)
	{
		dispose_node(gtd->dispose_list->list[--gtd->dispose_list->used]);
	}
}

void time_update(void)
{
	static char str_sec[9], str_min[9], str_hour[9], str_wday[9], str_mday[9], str_mon[9], str_year[9];
	static struct tm calendar, old_calendar;
	static short event_date, event_minute, event_second, event_time;

	if (gtd->time == time(NULL))
	{
		return;
	}
	gtd->time = time(NULL);

	if (!HAS_BIT(gtd->event_flags, EVENT_FLAG_TIME))
	{
		return;
	}

	calendar = *localtime(&gtd->time);

	// Initialize on the first call.

	if (old_calendar.tm_year == 0)
	{
		old_calendar.tm_sec  = calendar.tm_sec;
		old_calendar.tm_min  = calendar.tm_min;
		old_calendar.tm_hour = calendar.tm_hour;
		old_calendar.tm_wday = calendar.tm_wday;
		old_calendar.tm_mday = calendar.tm_mday;
		old_calendar.tm_mon  = calendar.tm_mon;
		old_calendar.tm_year = calendar.tm_year;

		strftime(str_sec,  9, "%S", &calendar);
		strftime(str_min,  9, "%M", &calendar);
		strftime(str_hour, 9, "%H", &calendar);
		strftime(str_wday, 9, "%w", &calendar);
		strftime(str_mday, 9, "%d", &calendar);
		strftime(str_mon,  9, "%m", &calendar);
		strftime(str_year, 9, "%Y", &calendar);

		while (strcmp(event_table[++event_date].name, "DATE"))
		{
		}
		event_minute = event_date;

		while (strcmp(event_table[++event_minute].name, "MINUTE"))
		{
		}
		event_second = event_minute;

		while (strcmp(event_table[++event_second].name, "SECOND"))
		{
		}
		event_time = event_second;

		while (!strcmp(event_table[event_time].name, "TIME"))
		{
		}
	}

	strftime(str_sec, 9, "%S", &calendar);
	old_calendar.tm_sec = calendar.tm_sec;

	if (calendar.tm_min == old_calendar.tm_min)
	{
		goto time_event_sec;
	}

	strftime(str_min, 9, "%M", &calendar);
	old_calendar.tm_min = calendar.tm_min;

	if (calendar.tm_hour == old_calendar.tm_hour)
	{
		goto time_event_min;
	}

	strftime(str_hour, 9, "%H", &calendar);
	old_calendar.tm_hour = calendar.tm_hour;

	if (calendar.tm_mday == old_calendar.tm_mday)
	{
		goto time_event_hour;
	}

	strftime(str_wday, 9, "%w", &calendar);
	old_calendar.tm_wday = calendar.tm_wday;

	strftime(str_mday, 9, "%d", &calendar);
	old_calendar.tm_mday = calendar.tm_mday;

	if (calendar.tm_mon == old_calendar.tm_mon)
	{
		goto time_event_mday;
	}

	strftime(str_mon, 9, "%m", &calendar);
	old_calendar.tm_mon = calendar.tm_mon;

	if (calendar.tm_year == old_calendar.tm_year)
	{
		goto time_event_mon;
	}

	strftime(str_year, 9, "%Y", &calendar);
	old_calendar.tm_year = calendar.tm_year;

	check_all_events(NULL, EVENT_FLAG_TIME, 0, 7, "YEAR", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, EVENT_FLAG_TIME, 1, 7, "YEAR %s", str_year, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);


	time_event_mon:

	check_all_events(NULL, EVENT_FLAG_TIME, 0, 7, "MONTH", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, EVENT_FLAG_TIME, 1, 7, "MONTH %s", str_mon, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);


	time_event_mday:

	check_all_events(NULL, EVENT_FLAG_TIME, 0, 7, "WEEK", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, EVENT_FLAG_TIME, 1, 7, "WEEK %s", str_wday, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);

	check_all_events(NULL, EVENT_FLAG_TIME, 2, 7, "DATE %s-%s", str_mon, str_mday, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);

	check_all_events(NULL, EVENT_FLAG_TIME, 0, 7, "DAY", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, EVENT_FLAG_TIME, 1, 7, "DAY %s", str_mday, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);


	time_event_hour:

	check_all_events(NULL, EVENT_FLAG_TIME, 0, 7, "HOUR", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	check_all_events(NULL, EVENT_FLAG_TIME, 1, 7, "HOUR %s", str_hour, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);


	time_event_min:

	if (event_table[event_date].level)
	{
		check_all_events(NULL, EVENT_FLAG_TIME, 4, 7, "DATE %s-%s %s:%s", str_mon, str_mday, str_hour, str_min, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	}

	if (event_table[event_time].level)
	{
		check_all_events(NULL, EVENT_FLAG_TIME, 2, 7, "TIME %s:%s", str_hour, str_min, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	}

	if (event_table[event_minute].level)
	{
		check_all_events(NULL, EVENT_FLAG_TIME, 0, 7, "MINUTE", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
		check_all_events(NULL, EVENT_FLAG_TIME, 1, 7, "MINUTE %s", str_min, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	}

	time_event_sec:

	old_calendar.tm_sec = calendar.tm_sec;

	if (event_table[event_time].level)
	{
		check_all_events(NULL, EVENT_FLAG_TIME, 3, 7, "TIME %s:%s:%s", str_hour, str_min, str_sec, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	}

	if (event_table[event_second].level)
	{
		check_all_events(NULL, EVENT_FLAG_TIME, 0, 7, "SECOND", str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
		check_all_events(NULL, EVENT_FLAG_TIME, 1, 7, "SECOND %s", str_sec, str_year, str_mon, str_wday, str_mday, str_hour, str_min, str_sec);
	}
	return;
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

	if (cpu_timer[timer][1] == 0 || cpu_timer[timer][4] == 0)
	{
		return 0;
	}

//	indicated_usage = cpu_timer[timer][0] / cpu_timer[timer][1] * cpu_timer[timer][4];

	indicated_usage = cpu_timer[timer][0];

	tintin_printf2(ses, "%-29s %8.1f       %8lld      %8.2f     %8.3f",
		timer_table[timer].name,
		(double) cpu_timer[timer][0] / (double) cpu_timer[timer][1],
		cpu_timer[timer][3] / cpu_timer[timer][4] / 1000,
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

	if (cpu_timer[timer][2] == 0)
	{
		cpu_timer[timer][2] = current_time;
	}
	else
	{
		cpu_timer[timer][3] += current_time - cpu_timer[timer][2];
		cpu_timer[timer][2]  = current_time;
		cpu_timer[timer][4] ++;
	}
}


void close_timer(int timer)
{
	struct timeval last_time;
	long long current_time;

	gettimeofday(&last_time, NULL);

	current_time = (long long) last_time.tv_usec + 1000000LL * (long long) last_time.tv_sec;

	cpu_timer[timer][0] += (current_time - cpu_timer[timer][2]);
	cpu_timer[timer][1] ++;
}
