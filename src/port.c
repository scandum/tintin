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
*                      coded by Igor van den Hoven 2017                       *
******************************************************************************/

#include "tintin.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define CALL_TIMEOUT 5

DO_COMMAND(do_port)
{
	char cmd[BUFFER_SIZE];
	int cnt;

	arg = get_arg_in_braces(ses, arg, cmd, GET_ONE);

	if (*cmd == 0)
	{
		info:

		tintin_header(ses, 80, " PORT OPTIONS ");

		for (cnt = 0 ; *port_table[cnt].name != 0 ; cnt++)
		{
			tintin_printf2(ses, "  [%-13s] %s", port_table[cnt].name, port_table[cnt].desc);
		}
		tintin_header(ses, 80, "");

		return ses;
	}

	for (cnt = 0 ; *port_table[cnt].name != 0 ; cnt++)
	{
		if (!is_abbrev(cmd, port_table[cnt].name))
		{
			continue;
		}

		if (port_table[cnt].fun != port_initialize && ses->port == NULL)
		{
			tintin_printf(ses, "#PORT: You must initialize a port first.");

			return ses;
		}

		arg = sub_arg_in_braces(ses, arg, arg1,  port_table[cnt].lval, SUB_VAR|SUB_FUN);

		arg = sub_arg_in_braces(ses, arg, arg2, port_table[cnt].rval, SUB_VAR|SUB_FUN);

		ses = port_table[cnt].fun(ses, arg, arg1, arg2);

		return ses;
	}

	goto info;

	return ses;
}


DO_PORT(port_initialize)
{
	char temp[BUFFER_SIZE], file[BUFFER_SIZE];
	struct sockaddr_in sin;
	struct linger ld;
	int sock = 0, port, reuse = 1;

	arg = sub_arg_in_braces(ses, arg, file,  GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 || *arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #PORT INITIALIZE {NAME} {PORT} {FILE}");
		
		return ses;
	}

	if (find_session(arg1))
	{
		tintin_printf(ses, "#PORT INITIALIZE: THERE'S A SESSION NAMED {%s} ALREADY.", arg1);

		return ses;
	}

	if (!is_number(arg2))
	{
		tintin_printf(ses, "#PORT INITIALIZE: {%s} IS NOT A VALID PORT NUMBER.", arg2);

		return ses;
	}

	tintin_printf(ses, "#TRYING TO LAUNCH '%s' ON PORT '%s'.", arg1, arg2);

	sprintf(temp, "{localhost} {%d} {%.*s}", atoi(arg2), PATH_SIZE, file);

	port = atoi(arg2);

	if (port)
	{
		sin.sin_family      = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port        = htons(port);

		sock = socket(AF_INET, SOCK_STREAM, 0);

		if (sock < 0)
		{
			syserr_printf(ses, "port_initialize: socket");

			return ses;
		}

		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
		{
			syserr_printf(ses, "port_initialize: setsockopt");

			return ses;
		}

		ld.l_onoff  = 0; 
		ld.l_linger = 100;

		setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

		if (fcntl(sock, F_SETFL, O_NDELAY|O_NONBLOCK) == -1)
		{
			syserr_printf(ses, "port_initialize: fcntl O_NDELAY|O_NONBLOCK");

			return ses;
		}

		if (bind(sock, (struct sockaddr *) &sin, sizeof(sin)) < 0)
		{
			tintin_printf(NULL, "#PORT INITIALIZE: PORT %d IS ALREADY IN USE.", port);

			close(sock);

			return ses;
		}

		if (listen(sock, 32) == -1)
		{
			syserr_printf(ses, "port_initialize: listen");

			close(sock);

			return ses;
		}
/*
		socklen_t len = sizeof(sin);

		if (getsockname(sock, (struct sockaddr *) &sin, &len) == -1)
		{
			syserr_printf(ses, "port_initialize: getsockname");
		}
		else
		{
			printf("debug: %d\n", sin.sin_port);
		}
*/

	}

	ses = new_session(ses, arg1, temp, -1, 0);

	ses->port = (struct port_data *) calloc(1, sizeof(struct port_data));

	ses->port->fd       = sock;
	ses->port->port     = port;

	ses->port->name     = strdup(arg1);
	ses->port->group    = strdup("");
	ses->port->color    = strdup("\e[0;1;36m");
	ses->port->ip       = strdup("<Unknown>");
	ses->port->prefix   = strdup("<PORT> ");

	check_all_events(ses, EVENT_FLAG_PORT, 0, 3, "PORT INITIALIZED", ses->name, ntos(ses->port->port), ntos(ses->port->fd));

	if (!check_all_events(ses, EVENT_FLAG_PORT, 0, 2, "GAG PORT INITIALIZED", ses->name, ntos(ses->port->port)))
	{
		tintin_printf(ses, "#PORT INITIALIZE: SESSION {%s} IS LISTENING ON PORT %d.", ses->name, ses->port->port);
	}

	return ses;
}


DO_PORT(port_uninitialize)
{
	int port = ses->port->port;

	while (ses->port->next)
	{
		close_port(ses, ses->port->next, TRUE);
	}

	close_port(ses, ses->port, FALSE);

	ses->port = NULL;

	check_all_events(ses, EVENT_FLAG_PORT, 0, 2, "PORT UNINITIALIZED", ses->name, ntos(port));

	if (!check_all_events(ses, EVENT_FLAG_PORT, 0, 2, "GAG PORT UNINITIALIZED", ses->name, ntos(port)))
	{
		tintin_printf(ses, "#PORT UNINITIALIZE: CLOSED PORT {%d}.", port);
	}

	if (!HAS_BIT(ses->flags, SES_FLAG_CLOSED))
	{
		cleanup_session(ses);
	}

	return gtd->ses;
}


int port_new(struct session *ses, int sock)
{
	struct port_data *new_buddy;
	struct sockaddr_in sock_addr;
	socklen_t len;
	int fd;

	push_call("port_new(%p,%d)",ses,sock);

	len = sizeof(sock);

	getsockname(sock, (struct sockaddr *) &sock_addr, &len);

	if ((fd = accept(sock, (struct sockaddr *) &sock_addr, &len)) < 0)
	{
		syserr_printf(ses, "port_new: accept");

		pop_call();
		return -1;
	}

	if (fcntl(fd, F_SETFL, O_NDELAY|O_NONBLOCK) == -1)
	{
		syserr_printf(ses, "port_new: fcntl O_NDELAY|O_NONBLOCK");
	}

	if (HAS_BIT(ses->port->flags, PORT_FLAG_DND))
	{
		close(fd);

		pop_call();
		return -1;
	}

	if (HAS_BIT(ses->port->flags, PORT_FLAG_PRIVATE))
	{
		if (strcmp(inet_ntoa(sock_addr.sin_addr), "127.0.0.1"))
		{
			port_printf(ses, "%s D%d Refusing remote connection, private flag set.", inet_ntoa(sock_addr.sin_addr), fd);

			close(fd);

			pop_call();
			return -1;
		}
	}

	new_buddy = (struct port_data *) calloc(1, sizeof(struct port_data));

	new_buddy->fd       = fd;

	new_buddy->name     = strdup(ntos(fd));
	new_buddy->group    = strdup("");
	new_buddy->ip       = strdup(inet_ntoa(sock_addr.sin_addr));
	new_buddy->prefix   = strdup("");
	new_buddy->color    = strdup("");
	new_buddy->proxy    = strdup("");
	new_buddy->ttype    = strdup("");

	new_buddy->port     = 0;

	ses->port->total++;

	LINK(new_buddy, ses->port->next, ses->port->prev);

	port_printf(ses, "New connection: %s D%d.", new_buddy->ip, new_buddy->fd);

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_TELNET))
	{
		announce_support(ses, new_buddy);
	}

	check_all_events(ses, EVENT_FLAG_PORT, 0, 3, "PORT CONNECTION", new_buddy->name, new_buddy->ip, ntos(new_buddy->port));

	pop_call();
	return 0;
}



void close_port(struct session *ses, struct port_data *buddy, int unlink)
{
	buddy->flags = 0;

	push_call("close_port(%p,%p,%d)",ses,buddy,unlink);

	if (unlink)
	{
		ses->port->total--;

		if (buddy == ses->port->update)
		{
			ses->port->update = buddy->next;
		}
		UNLINK(buddy, ses->port->next, ses->port->prev);
	}

	SET_BIT(buddy->flags, PORT_FLAG_LINKLOST);

	if (buddy != ses->port)
	{
		if (*buddy->name == 0)
		{
			port_printf(ses, "Closing connection to %s D%d", buddy->ip, buddy->fd);
		}
		else
		{
			port_printf(ses, "Closing connection to %s@%s D%d.", buddy->name, buddy->ip, buddy->fd);
		}
		check_all_events(ses, EVENT_FLAG_PORT, 0, 3, "PORT DISCONNECTION", buddy->name, buddy->ip, ntos(buddy->port));
	}

	if (buddy->fd && close(buddy->fd) == -1)
	{
		syserr_printf(ses, "close_port: close");
	}

	end_mccp2(ses, buddy);
	end_mccp3(ses, buddy);

	free(buddy->group);
	free(buddy->ip);
	free(buddy->name);
	free(buddy->prefix);
	free(buddy->color);

	free(buddy);

	pop_call();
	return;
}


void process_port_connections(struct session *ses, fd_set *read_set, fd_set *write_set, fd_set *exc_set)
{
	struct port_data *buddy;

	push_call("process_port_connections(%p,%p,%p)",read_set,write_set,exc_set);

	if (FD_ISSET(ses->port->fd, read_set))
	{
		port_new(ses, ses->port->fd);
	}

	for (buddy = ses->port->next ; buddy ; buddy = ses->port->update)
	{
		ses->port->update = buddy->next;

		if (HAS_BIT(buddy->flags, PORT_FLAG_LINKLOST) || FD_ISSET(buddy->fd, exc_set))
		{
			FD_CLR(buddy->fd, write_set);
			FD_CLR(buddy->fd, read_set);

			close_port(ses, buddy, TRUE);
		}
		else if (FD_ISSET(buddy->fd, read_set))
		{
			if (process_port_input(ses, buddy) < 0)
			{
				FD_CLR(buddy->fd, write_set);
				FD_CLR(buddy->fd, read_set);

				close_port(ses, buddy, TRUE);
			}
		}

		if (HAS_BIT(buddy->comm_flags, COMM_FLAG_MSDPUPDATE))
		{
			msdp_send_update(ses, buddy);
		}
	}
	pop_call();
	return;
}

void port_socket_write(struct session *ses, struct port_data *buddy, char *str, int len)
{
	if (!HAS_BIT(buddy->flags, PORT_FLAG_LINKLOST))
	{
		if (buddy->mccp2)
		{
			write_mccp2(ses, buddy, str, len);
		}
		else
		{
			if (write(buddy->fd, str, len) < 0)
			{
				port_printf(ses, "Lost link to socket '%s'.", buddy->name);

				SET_BIT(buddy->flags, PORT_FLAG_LINKLOST);
			}
		}
	}
}

void port_socket_printf(struct session *ses, struct port_data *buddy, char *format, ...)
{
	char buf[BUFFER_SIZE];
	va_list args;
	int len;

	va_start(args, format);
	len = vsnprintf(buf, BUFFER_SIZE / 3, format, args);
	va_end(args);

	port_socket_write(ses, buddy, buf, len);
}

void port_telnet_printf(struct session *ses, struct port_data *buddy, size_t length, char *format, ...)
{
	size_t size;

	char buf[BUFFER_SIZE];
	va_list args;

	va_start(args, format);
	size = vsprintf(buf, format, args);
	va_end(args);

	if (size != length && HAS_BIT(ses->telopts, TELOPT_FLAG_DEBUG))
	{
		tintin_printf(ses, "DEBUG TELNET: port_telnet_printf size difference: %d vs %d", size, length);
	}

	port_socket_write(ses, buddy, buf, length);
}

void port_printf(struct session *ses, char *format, ...)
{
	char buf[BUFFER_SIZE / 2], tmp[BUFFER_SIZE];
	int len;
	va_list args;

	len = BUFFER_SIZE / 2 - strlen(ses->port->prefix) - 5;

	va_start(args, format);
	vsnprintf(buf, len, format, args);
	va_end(args);

	sprintf(tmp, "%s%s", ses->port->prefix, buf);

	strip_vt102_codes_non_graph(tmp, buf);

	sprintf(tmp, "%s%s\e[0m", ses->port->color, buf);

	check_all_events(ses, SUB_SEC|EVENT_FLAG_PORT, 0, 2, "PORT MESSAGE", tmp, buf);

	if (!check_all_events(ses, SUB_SEC|EVENT_FLAG_CATCH, 0, 2, "CATCH PORT MESSAGE", tmp, buf))
	{
		tintin_printf(ses, "%s", tmp);
	}
}

void port_log_printf(struct session *ses, struct port_data *buddy, char *format, ...)
{
	char buf[BUFFER_SIZE / 2], tmp[BUFFER_SIZE];
	int len;
	va_list args;

	len = BUFFER_SIZE / 2 - strlen(ses->port->prefix) - strlen(buddy->name) - strlen(buddy->ip) - 7;

	va_start(args, format);
	vsnprintf(buf, len, format, args);
	va_end(args);

	sprintf(tmp, "%s%s@%s %s", ses->port->prefix, buddy->name, buddy->ip, buf);

	strip_vt102_codes_non_graph(tmp, buf);

	sprintf(tmp, "%s%s\e[0m", ses->port->color, buf);

	check_all_events(ses, EVENT_FLAG_PORT, 0, 5, "PORT LOG MESSAGE", buddy->name, buddy->ip, ntos(buddy->fd), tmp, buf);

	if (!check_all_events(ses, EVENT_FLAG_CATCH, 0, 5, "CATCH PORT LOG MESSAGE", buddy->name, buddy->ip, ntos(buddy->fd), tmp, buf))
	{
		tintin_printf(ses, "%s", tmp);
	}
}

int process_port_input(struct session *ses, struct port_data *buddy)
{
	char input[BUFFER_SIZE], *pt1, *pt2;
	int size, echo;

	push_call("process_port_input(%p)",buddy);

//	while (TRUE)
	{
		size = read(buddy->fd, input, BUFFER_SIZE / 4);

		if (size < 0)
		{
			if (errno == EWOULDBLOCK)
			{
				pop_call();
				return 0;
			}

			syserr_printf(ses, "process_port_input: read:");

			pop_call();
			return -1;
		}

		if (size == 0)
		{
			pop_call();
			return -1;
		}

		input[size] = 0;

		echo = buddy->intop;

		buddy->intop += server_translate_telopts(ses, buddy, (unsigned char *) input, size, (unsigned char *) buddy->inbuf, buddy->intop);

		// Handle local echo for Windows telnet

		if (HAS_BIT(buddy->comm_flags, COMM_FLAG_REMOTEECHO))
		{
			while (echo < buddy->intop)
			{
				switch (input[echo++])
				{
					case   8:
					case 127:
						input[echo] = '\b';
						port_socket_printf(ses, buddy, "\b \b");
						break;

					case '\n':
						port_socket_printf(ses, buddy, "\r\n");
						break;

					default:
						if (HAS_BIT(buddy->comm_flags, COMM_FLAG_PASSWORD))
						{
							port_socket_printf(ses, buddy, "*");
						}
						else
						{
							port_socket_printf(ses, buddy, "%c", input[echo]);
						}
						break;
				}
			}
		}

		if (buddy->intop > BUFFER_SIZE / 4)
		{
			port_socket_printf(ses, buddy, "\e[1;31mYou overflowed your input buffer, you must reconnect.\n");
			input[BUFFER_SIZE / 2] = 0;
			port_socket_printf(ses, buddy, "%s\n", input);
			port_log_printf(ses, buddy, "Buffer overflow, closing connection.");

			pop_call();
			return -1;
		}

		if (buddy->intop)
		{
			pt2 = buddy->inbuf;

			while (pt2)
			{
				pt1 = pt2;
				pt2 = strchr(pt1, '\n');

				if (pt2)
				{
					*pt2++ = 0;

					get_port_commands(ses, buddy, pt1, pt2 - pt1);
				}
			}
			buddy->intop = strlen(pt1);

			memmove(buddy->inbuf, pt1, buddy->intop);
		}
	}
	pop_call();
	return 0;
}

void get_port_commands(struct session *ses, struct port_data *buddy, char *buf, int len)
{
	char txt[STRING_SIZE];

	push_call("get_port_commands(%s,%d,%s)",buddy->name,len,buf);

	strip_vt102_codes(buf, txt);

	check_all_events(ses, SUB_SEC|EVENT_FLAG_PORT, 0, 5, "PORT RECEIVED MESSAGE", buddy->name, buddy->ip, ntos(buddy->port), buf, txt);

	if (!check_all_events(ses, SUB_SEC|EVENT_FLAG_CATCH, 0, 5, "CATCH PORT RECEIVED MESSAGE", buddy->name, buddy->ip, ntos(buddy->port), buf, txt))
	{
		port_receive_message(ses, buddy, buf);
	}

	pop_call();
	return;
}


void port_receive_message(struct session *ses, struct port_data *buddy, char *txt)
{
	if (HAS_BIT(buddy->flags, PORT_FLAG_IGNORE))
	{
		return;
	}

	port_printf(ses, "%s", txt);
}

DO_PORT(port_call)
{
	int sock, error;
	char host[BUFFER_SIZE], port[BUFFER_SIZE];
	struct addrinfo *address;
	static struct addrinfo hints;
	struct port_data *new_buddy;
	struct timeval to;
	fd_set wds, rds;

	to.tv_sec = CALL_TIMEOUT;
	to.tv_usec = 0;

	strcpy(host, arg1);
	strcpy(port, arg2);

	port_printf(ses, "Attempting to call {%s} {%s} ...", host, port);

	hints.ai_family   = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;

	error = getaddrinfo(host, port, &hints, &address);

	if (error)
	{
		hints.ai_family = AF_INET6;

		error = getaddrinfo(host, port, &hints, &address);

		if (error)
		{
			port_printf(ses, "Failed to call %s, unknown host.", host);
			
			return ses;
		}
	}

	sock = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

	if (sock < 0)
	{
		syserr_printf(ses, "port_call: socket");

		freeaddrinfo(address);

		return ses;
	}

	switch (address->ai_family)
	{
		case AF_INET:
			inet_ntop(address->ai_family, &((struct sockaddr_in *)address->ai_addr)->sin_addr, host, address->ai_addrlen);
			break;

		case AF_INET6:
			inet_ntop(address->ai_family, &((struct sockaddr_in6 *)address->ai_addr)->sin6_addr, host, address->ai_addrlen);
			break;
	}

	error = connect(sock, address->ai_addr, address->ai_addrlen);

	if (error)
	{
		syserr_printf(ses, "port_call: connect");

		port_printf(ses, "Failed to connect to %s:%s", host, port);

		close(sock);

		freeaddrinfo(address);

		return ses;
	}

	freeaddrinfo(address);

	FD_ZERO(&wds);

	FD_SET(sock, &wds);

	error = select(FD_SETSIZE, NULL, &wds, NULL, &to);

	if (error < 0)
	{
		syserr_printf(ses, "port_call: select wds:");

		port_printf(ses, "Failed to connect to %s %s", host, port);

		close(sock);

		return ses;
	}

	if (!FD_ISSET(sock, &wds))
	{
		port_printf(ses, "Connection timed out.");

		close(sock);

		return ses;
	}

	new_buddy = calloc(1, sizeof(struct port_data));

	new_buddy->fd       = sock;
	new_buddy->port     = atoi(port);

	new_buddy->group    = strdup("");
	new_buddy->ip       = strdup(host);
	new_buddy->name     = strdup(ntos(sock));
	new_buddy->color    = strdup("");
	new_buddy->prefix   = strdup("");

	check_all_events(ses, EVENT_FLAG_PORT, 0, 3, "PORT CONNECTION", new_buddy->name, new_buddy->ip, ntos(new_buddy->port));

	FD_ZERO(&rds);
	FD_SET(sock, &rds);

	to.tv_sec  = CALL_TIMEOUT;
	to.tv_usec = 0;

	error = select(FD_SETSIZE, &rds, NULL, NULL, &to);

	if (error < 0)
	{
		syserr_printf(ses, "port_call: select rds:");

		close_port(ses, new_buddy, FALSE);

		return ses;
	}

	if (process_port_input(ses, new_buddy) == -1)
	{
		FD_CLR(new_buddy->fd, &rds);

		close_port(ses, new_buddy, FALSE);

		return ses;
	}

	// NULL check because of threading.

	if (ses->port == NULL || *new_buddy->name == 0)
	{
		close_port(ses, new_buddy, FALSE);

		return ses;
	}

	if (fcntl(sock, F_SETFL, O_NDELAY|O_NONBLOCK) == -1)
	{
		syserr_printf(ses, "port_new: fcntl O_NDELAY|O_NONBLOCK");
	}

	ses->port->total++;

	LINK(new_buddy, ses->port->next, ses->port->prev);

	port_printf(ses, "Connection made to %s.", new_buddy->name);

	return ses;
}

DO_PORT(port_info)
{
	tintin_printf2(ses, "Port                 : %d", ses->port->port);
	tintin_printf2(ses, "Prefix               : %s", ses->port->prefix);
	tintin_printf2(ses, "Color                : %s", str_convert_meta(ses->port->color, TRUE));
	tintin_printf2(ses, "DND                  : %s", HAS_BIT(ses->port->flags, PORT_FLAG_DND) ? "On" : "Off");

	return ses;
}


DO_PORT(port_name)
{
	struct port_data *buddy;

	buddy = find_port_buddy(ses, arg1);

	substitute(ses, arg2, arg2, SUB_COL|SUB_ESC);

	if (buddy == NULL)
	{
		port_printf(ses, "There is no socket named '%s'.", arg1);

		return ses;
	}

	RESTRING(buddy->name, arg2);

	port_printf(ses, "Name of socket '%s' changed to '%s'.", arg1, buddy->name);

	return ses;
}


DO_PORT(port_prefix)
{
	RESTRING(ses->port->prefix, arg1);

	port_printf(ses, "Prefix set to '%s'", ses->port->prefix);

	return ses;
}

DO_PORT(port_proxy)
{
	struct session *bridge;
	struct port_data *buddy;

	if ((buddy = find_port_buddy(ses, arg1)) == NULL)
	{
		port_printf(ses, "You are not connected to anyone named '%s'.", arg1);

		return ses;
	}

	if (buddy->ses && *arg2 == 0)
	{
		port_printf(ses, "Socket '%s' is no longer a proxy for '%s'.", arg1, buddy->ses->name);

		buddy->ses->proxy = NULL;
		buddy->ses = NULL;

		return ses;
	}

	if ((bridge = find_session(arg2)) == NULL)
	{
		port_printf(ses, "The session '%s' could not be found.", arg2);

		return ses;
	}

	buddy->ses = ses;
	bridge->proxy = buddy;

	port_printf(ses, "Socket '%s' is now a proxy for '%s'.", arg1, buddy->ses->name);

	return ses;
}

DO_PORT(port_send)
{
	struct port_data *buddy;

	substitute(gtd->ses, arg2, arg2, SUB_COL|SUB_ESC|SUB_LNF);

	if (!strcasecmp(arg1, "ALL"))
	{
		for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
		{
			port_socket_printf(ses, buddy, "%s", arg2);
		}
	}
	else
	{
		if ((buddy = find_port_buddy(ses, arg1)) != NULL)
		{
			port_socket_printf(ses, buddy, "%s", arg2);
		}
		else if (find_port_group(ses, arg1) != NULL)
		{
			for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
			{
				if (!strcmp(buddy->group, arg1))
				{
					port_socket_printf(ses, buddy, "%s", arg2);
				}
			}
		}
		else
		{
			port_printf(ses, "There is no socket named '%s'.", arg1);
		}
	}

	return ses;
}

DO_PORT(port_who)
{
	struct port_data *buddy;
	int cnt = 1;

	tintin_printf(ses, "     %-15s  %-5s  %-20s  %-5s  ", "Name", "Flags", "Address", "Port");
	tintin_printf(ses, "     ===============  =====  ====================  =====  ");

	for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
	{
		tintin_printf(ses, " %03d %+15s  %s%s%s%s%s  %+20s  %+5u",
			cnt++,
			buddy->name,
			HAS_BIT(buddy->flags, PORT_FLAG_PRIVATE)   ? "P" : " ",
			HAS_BIT(buddy->flags, PORT_FLAG_IGNORE)    ? "I" : " ",
			HAS_BIT(buddy->flags, PORT_FLAG_SERVE)     ? "S" : " ",
			HAS_BIT(buddy->flags, PORT_FLAG_FORWARD)   ? "F" :
			HAS_BIT(buddy->flags, PORT_FLAG_FORWARDBY) ? "f" : " ",
			" ",
			buddy->ip,
			buddy->port);
	}
	tintin_printf(ses, "     ===============  =====  ====================  ===== ");

	return ses;
}


DO_PORT(port_zap)
{
	struct port_data *buddy;

	if (!strcasecmp(arg1, "ALL"))
	{
		while (ses->port->next)
		{
			close_port(ses, ses->port->next, TRUE);
		}
	}
	else
	{
		if ((buddy = find_port_buddy(ses, arg1)))
		{
			close_port(ses, buddy, TRUE);
		}
		else
		{
			port_printf(ses, "There is no socket named '%s'.", arg1);
		}
	}

	return ses;
}


DO_PORT(port_color)
{
	if (*arg1 == 0 || !is_color_name(arg1))
	{
		port_printf(ses, "Valid colors are:\n\nreset, bold, dim, light, dark, underscore, blink, reverse, black, red, green, yellow, blue, magenta, cyan, white, b black, b red, b green, b yellow, b blue, b magenta, b cyan, b white");

		return ses;
	}
	get_color_names(gtd->ses, arg1, arg2);

	RESTRING(ses->port->color, arg2);

	port_printf(ses, "Color has been set to %s", arg1);

	return ses;
}

DO_PORT(port_dnd)
{
	TOG_BIT(ses->port->flags, PORT_FLAG_DND);

	if (HAS_BIT(ses->port->flags, PORT_FLAG_DND))
	{
		port_printf(ses, "New connections are no longer accepted.");
	}
	else
	{
		port_printf(ses, "New connections are accepted.");
	}

	return ses;
}

DO_PORT(port_flag)
{
	if (*arg1 == 0)
	{
		tintin_printf2(ses, "#SYNTAX: #PORT FLAG <DND|PRIVATE> [ON|OFF]");
	}
	else if (is_abbrev(arg1, "DND"))
	{
		if (!strcasecmp(arg2, "ON"))
		{
			SET_BIT(ses->port->flags, PORT_FLAG_DND);
		}
		else if (!strcasecmp(arg2, "OFF"))
		{
			DEL_BIT(ses->port->flags, PORT_FLAG_DND);
		}
		else if (*arg2 == 0)
		{
			TOG_BIT(ses->port->flags, PORT_FLAG_DND);
		}
		else
		{
			tintin_printf2(ses, "#SYNTAX: #PORT FLAG DND [ON|OFF]");
			
			return ses;
		}

		if (HAS_BIT(ses->port->flags, PORT_FLAG_DND))
		{
			port_printf(ses, "New connections are no longer accepted.");
		}
		else
		{
			port_printf(ses, "New connections are accepted.");
		}
	}
	else if (is_abbrev(arg1, "PRIVATE"))
	{
		if (!strcasecmp(arg2, "ON"))
		{
			SET_BIT(ses->port->flags, PORT_FLAG_PRIVATE);
		}
		else if (!strcasecmp(arg2, "OFF"))
		{
			DEL_BIT(ses->port->flags, PORT_FLAG_PRIVATE);
		}
		else if (*arg2 == 0)
		{
			TOG_BIT(ses->port->flags, PORT_FLAG_PRIVATE);
		}
		else
		{
			tintin_printf2(ses, "#SYNTAX: #PORT FLAG PRIVATE [ON|OFF]");
			
			return ses;
		}

		if (HAS_BIT(ses->port->flags, PORT_FLAG_PRIVATE))
		{
			port_printf(ses, "Remote connections are no longer accepted.");
		}
		else
		{
			port_printf(ses, "Remote connections are accepted.");
		}
	}
	return ses;
}

DO_PORT(port_group)
{
	struct port_data *buddy;
	int cnt = 0;

	if (*arg1 == 0)
	{
		tintin_printf(NULL, "     %-15s  %-20s  %-5s  %-15s", "Name", "Address", "Port", "Group");
		tintin_printf(NULL, "     ===============  ====================  =====  ==================== ");

		for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
		{
			tintin_printf(NULL, " %03d %-15s  %-20s  %-5u  %-20s",
				cnt++,
				buddy->name,
				buddy->ip,
				buddy->port,
				buddy->group);
		}
		tintin_printf(NULL, "     ===============  ====================  =====  ==================== ");
	}
	else if (!strcasecmp(arg1, "ALL"))
	{
		port_printf(ses, "You set everyone's group to '%s'", arg2);

		for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
		{
			RESTRING(buddy->group, arg2);
		}
	}
	else
	{
		if ((buddy = find_port_buddy(ses, arg1)) != NULL)
		{
			RESTRING(buddy->group, arg2);

			port_printf(ses, "You set %s's group to '%s'", buddy->name, arg2);
		}
		else
		{
			port_printf(ses, "You are not connected to anyone named '%s'.", arg1);
		}
	}

	return ses;
}


DO_PORT(port_ignore)
{
	struct port_data *buddy;

	if ((buddy = find_port_buddy(ses, arg1)) == NULL)
	{
		port_printf(ses, "You are not connected to anyone named '%s'.", arg1);

		return ses;
	}

	TOG_BIT(buddy->flags, PORT_FLAG_IGNORE);

	if (HAS_BIT(buddy->flags, PORT_FLAG_IGNORE))
	{
		port_printf(ses, "You are now ignoring %s.", buddy->name);
	}
	else
	{
		port_printf(ses, "You are no longer ignoring %s.", buddy->name);
	}
	
	return ses;
}




struct port_data *find_port_buddy(struct session *ses, char *arg)
{
	struct port_data *buddy;
	int fd;

	if (*arg == 0)
	{
		return NULL;
	}

	fd = is_number(arg);

	if (fd)
	{
		for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
		{
			if (fd == buddy->fd)
			{
				return buddy;
			}
		}
	}

	for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
	{
		if (!strcmp(arg, buddy->ip))
		{
			return buddy;
		}
	}

	for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
	{
		if (is_abbrev(arg, buddy->name))
		{
			return buddy;
		}
	}

	return NULL;
}


struct port_data *find_port_group(struct session *ses, char *arg)
{
	struct port_data *buddy;

	if (*arg == 0)
	{
		return NULL;
	}

	for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
	{
		if (!strcmp(arg, buddy->group))
		{
			return buddy;
		}
	}
	return NULL;
}

DO_PORT(port_rank)
{
	struct port_data *buddy;
	int cnt, rank;

	if (*arg1 == 0)
	{
		tintin_printf(NULL, "     %-15s  %-20s  %-5s  %-15s", "Name", "Address", "Port", "Rank");
		tintin_printf(NULL, "     ===============  ====================  =====  ==================== ");

		cnt = 0;

		for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
		{
			tintin_printf(NULL, " %03d %-15s  %-20s  %-5u  %-20s",
				cnt++,
				buddy->name,
				buddy->ip,
				buddy->port,
				rank_table[buddy->rank].name);
		}
		tintin_printf(NULL, "     ===============  ====================  =====  ==================== ");

		return ses;
	}

	if (*arg2 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #PORT RANK <NAME> <SPY|SCOUT>");

		return ses;
	}
	else if (is_abbrev(arg2, "SPY"))
	{
		rank = PORT_RANK_SPY;
	}
	else if (is_abbrev(arg2, "SCOUT"))
	{
		rank = PORT_RANK_SCOUT;
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #PORT RANK <NAME> <SPY|SCOUT>");
		
		return ses;
	}

	if (!strcasecmp(arg1, "ALL"))
	{
		port_printf(ses, "You set everyone's rank to '%s'", rank_table[rank].name);

		for (buddy = ses->port->next ; buddy ; buddy = buddy->next)
		{
			buddy->rank = rank;
		}
	}
	else
	{
		if ((buddy = find_port_buddy(ses, arg1)) != NULL)
		{
			buddy->rank = rank;

			port_printf(ses, "YOU SET %s'S RANK TO '%s'", buddy->name, rank_table[buddy->rank]);
		}
		else
		{
			port_printf(ses, "YOU ARE NOT CONNECTED TO ANYONE NAMED '%s'.", arg1);
		}
	}
	return ses;
}
