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
*                         coded by Peter Unold 1992                           *
******************************************************************************/


#include "tintin.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>

/*
	IPv6 compatible connect code.
*/

int wait_on_connect(struct session *ses, int sock, int connect_error)
{
	static struct timeval timeout;
	static fd_set rfd;
	static fd_set wfd;
	socklen_t len, val;

	FD_ZERO(&rfd);
	FD_ZERO(&wfd);

	FD_SET(sock, &rfd);
	FD_SET(sock, &wfd);

	timeout.tv_sec = 4;

	if (connect_error)
	{
		switch (select(FD_SETSIZE, &rfd, &wfd, NULL, &timeout))
		{
			case 0:
				syserr_printf(ses, "wait_on_connect:");
				return -1;

			case -1:
				syserr_printf(ses, "wait_on_connect: select");
				return -1;
		}
	}
	else
	{
		switch (select(sock+1, NULL, &wfd, NULL, &timeout))
		{
			case 0:
				syserr_printf(ses, "wait_on_connect2:");
				return -1;

			case -1:
				syserr_printf(ses, "wait_on_connect2: select");
				return -1;
		}
	}

	len = sizeof(val);

	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &val, &len) == -1)
	{
		syserr_printf(ses, "wait_on_connect: getsockopt:");

		return -1;
	}

	if (val)
	{
		errno = val;

		syserr_printf(ses, "wait_on_connect: getsockopt:");

		return -1;
	}
	return 0;
}

#ifdef HAVE_GETADDRINFO

int connect_mud(struct session *ses, char *host, char *port)
{
	int sock, error, optval;
	struct addrinfo *address;
	static struct addrinfo hints;
	char ip[100];

	if (!is_number(port))
	{
		show_error(ses, LIST_COMMAND, "#CONNECT: THE PORT {%s} SHOULD BE A NUMBER.", port);
		return -1;
	}

//	hints.ai_family   = AF_UNSPEC;
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
			tintin_printf2(ses, "#SESSION '%s' COULD NOT CONNECT - UNKNOWN HOST.", ses->name);

			return -1;
		}
	}

	sock = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

	if (sock < 0)
	{
		syserr_printf(ses, "connect_mud: socket");

		freeaddrinfo(address);

		return -1;
	}

	optval = 1;

	if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0)
	{
		syserr_printf(ses, "connect_mud: setsockopt:");
	}

	if (gts->connect_retry == 0)
	{
		ses->connect_error = connect(sock, address->ai_addr, address->ai_addrlen);
	}

	if (fcntl(sock, F_SETFL, O_NDELAY|O_NONBLOCK) == -1)
	{
		syserr_printf(ses, "connect_mud: fcntl O_NDELAY|O_NONBLOCK");

		close(sock);

		freeaddrinfo(address);

		return -1;
	}

	if (gts->connect_retry)
	{
		ses->connect_error = connect(sock, address->ai_addr, address->ai_addrlen);

		if (ses->connect_error)
		{
			ses->connect_error = wait_on_connect(ses, sock, ses->connect_error);
		}
	}

	if (ses->connect_error)
	{
		if (gts->connect_retry == 0)
		{
			syserr_printf(ses, "connect_mud: connect");
		}
		close(sock);

		freeaddrinfo(address);

		return 0;
	}

	error = getnameinfo(address->ai_addr, address->ai_addrlen, ip, 100, NULL, 0, NI_NUMERICHOST);

	if (error)
	{
		syserr_printf(ses, "connect_mud: getnameinfo:");
	}
	else
	{
		RESTRING(ses->session_ip, ip);
	}

	freeaddrinfo(address);

	return sock;
}

#else

int connect_mud(struct session *ses, char *host, char *port)
{
	int sock, d;
	struct sockaddr_in sockaddr;

	if (sscanf(host, "%d.%d.%d.%d", &d, &d, &d, &d) == 4)
	{
		sockaddr.sin_addr.s_addr = inet_addr(host);
	}
	else
	{
		struct hostent *hp;

		if (!(hp = gethostbyname(host)))
		{
			tintin_puts2(ses, "#ERROR - UNKNOWN HOST.");

			return -1;
		}
		memcpy((char *)&sockaddr.sin_addr, hp->h_addr, sizeof(sockaddr.sin_addr));
	}

	if (is_number(port))
	{
		sockaddr.sin_port = htons(atoi(port));
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#CONNECT: THE PORT {%s} SHOULD BE A NUMBER.", port);
		return -1;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		syserr_printf(ses, "old_connect_mud: socket()");

		return -1;
	}

	sockaddr.sin_family = AF_INET;

	ses->connect_error = connect(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr));

	if (ses->connect_error)
	{
		syserr_printf(ses, "connect_mud: connect");

		close(sock);

		return 0;
	}

	if (fcntl(sock, F_SETFL, O_NDELAY|O_NONBLOCK) == -1)
	{
		syserr_printf(ses, "connect_mud: fcntl O_NDELAY|O_NONBLOCK");
	}

	RESTRING(ses->session_ip, inet_ntoa(sockaddr.sin_addr));

	return sock;
}

#endif

void write_line_mud(struct session *ses, char *line, int size)
{
	int result;

	push_call("write_line_mud(%p,%p)",line,ses);

	check_all_events(ses, SUB_SEC|EVENT_FLAG_INPUT, 0, 2, "SEND OUTPUT", line, ntos(size));

	if (check_all_events(ses, SUB_SEC|EVENT_FLAG_CATCH, 0, 2, "CATCH SEND OUTPUT", line, ntos(size)))
	{
		pop_call();
		return;
	}

	if (ses == gts)
	{
		if (HAS_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK))
		{
			tintin_printf2(ses, "#NO SESSION ACTIVE. TINTIN IS CHILD LOCKED, PRESS CTRL-D TO EXIT.");
		}
		else
		{
			check_all_events(ses, SUB_SEC|EVENT_FLAG_INPUT, 0, 2, "NO SESSION ACTIVE", line, ntos(size));

			if (!check_all_events(ses, SUB_SEC|EVENT_FLAG_GAG, 0, 2, "GAG NO SESSION ACTIVE", line, ntos(size)))
			{
				tintin_printf2(ses, "#NO SESSION ACTIVE. USE: %csession {name} {host} {port} TO START ONE.", gtd->tintin_char);
			}
		}
		pop_call();
		return;
	}

	if (!HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
	{
		tintin_printf2(ses, "#THIS SESSION IS NOT CONNECTED, CANNOT SEND: %s", line);

		pop_call();
		return;
	}


	if (!HAS_BIT(ses->telopts, TELOPT_FLAG_TELNET) && HAS_BIT(ses->charset, CHARSET_FLAG_ALL_TOUTF8))
	{
		char buf[BUFFER_SIZE];

		size = utf8_to_all(ses, line, buf, size);

		memcpy(line, buf, size);

		line[size] = 0;
	}

	{
		if (ses->mccp3)
		{
			result = client_write_compressed(ses, line, size);
		}
#ifdef HAVE_GNUTLS_H
		else if (ses->ssl)
		{
			result = gnutls_record_send(ses->ssl, line, size);

			while (result == GNUTLS_E_INTERRUPTED || result == GNUTLS_E_AGAIN)
			{
				result = gnutls_record_send(ses->ssl, 0, 0);
			}
		}
#endif
		else
		{
			result = write(ses->socket, line, size);

			if (result == -1)
			{
				syserr_printf(ses, "write_line_mud: write");
			}
		}
		
		if (result == -1)
		{
			cleanup_session(ses);

			pop_call();
			return;
		}
	}

	check_all_events(ses, SUB_SEC|EVENT_FLAG_INPUT, 0, 2, "SENT OUTPUT", line, ntos(size));

	pop_call();
	return;
}


int read_buffer_mud(struct session *ses)
{
	unsigned char buffer[BUFFER_SIZE];
	int size;

	push_call("read_buffer_mud(%p)",ses);

#ifdef HAVE_GNUTLS_H

	if (ses->ssl)
	{
		do
		{
			size = gnutls_record_recv(ses->ssl, buffer, BUFFER_SIZE - 1);
		}
		while (size == GNUTLS_E_INTERRUPTED || size == GNUTLS_E_AGAIN);

		if (size < 0)
		{
			tintin_printf2(ses, "#SSL ERROR: %s", gnutls_strerror(size));
		}
	}
	else
#endif
	size = read(ses->socket, buffer, BUFFER_SIZE - 1);

	if (size <= 0)
	{
		pop_call();
		return FALSE;
	}
	buffer[size] = 0;

	ses->read_len = client_translate_telopts(ses, buffer, size);

	pop_call();
	return TRUE;
}

int detect_prompt(struct session *ses, char *original)
{
	char strip[BUFFER_SIZE];
	struct listroot *root = ses->list[LIST_PROMPT];
	struct listnode *node;

	if (HAS_BIT(ses->charset, CHARSET_FLAG_ALL_TOUTF8))
	{
		all_to_utf8(ses, original, strip);

		strcpy(original, strip);
	}

	strip_vt102_codes(original, strip);

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		node = root->list[root->update];

		if (check_one_regexp(ses, node, strip, original, 0))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void readmud(struct session *ses)
{
	char *line, *next_line/*, *strip*/;
	char linebuf[STRING_SIZE];
	int len;
	struct session *cts;

	push_call("readmud(%p)", ses);

	line = gtd->mud_output_buf;

	if (ses->check_output && *line == '\n')
	{
		line++;

		process_more_output(ses, "", FALSE);
	}
	else if (HAS_BIT(ses->config_flags, CONFIG_FLAG_COMPACT))
	{
		if (*ses->scroll->input == 0 && *line == '\n')
		{
			line++;
		}
	}

//	strip = str_alloc(gtd->mud_output_len);

	gtd->mud_output_strip_len = strip_vt102_codes(line, gtd->mud_output_strip_buf);

	check_all_events(ses, SUB_SEC|EVENT_FLAG_OUTPUT, 0, 2, "RECEIVED OUTPUT", gtd->mud_output_buf, gtd->mud_output_strip_buf);

	if (check_all_events(ses, SUB_SEC|EVENT_FLAG_CATCH, 0, 2, "CATCH RECEIVED OUTPUT", gtd->mud_output_buf, gtd->mud_output_strip_buf))
	{
		gtd->mud_output_len = 0;

		// str_free(strip);
		pop_call();
		return;
	}

	check_one_line_multi(ses, line, gtd->mud_output_strip_buf);

//	str_free(strip);

	// cts = current tintin session, may have to make this global to avoid glitches

	cts = gtd->ses;

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_SPLIT))
	{
		save_pos(gtd->ses);

		goto_pos(gtd->ses, gtd->ses->split->bot_row, 1);
	}


	// separate into lines and print away

	SET_BIT(cts->flags, SES_FLAG_READMUD);

	for ( ; line && *line ; line = next_line)
	{
		next_line = strchr(line, '\n');

		if (next_line)
		{
			if (next_line - line >= BUFFER_SIZE / 3)
			{
				next_line = &line[BUFFER_SIZE / 3]; // Not ideal. Shorten too long lines.
			}
			*next_line++ = 0;
		}
		else
		{
			if (*line == 0)
			{
				break;
			}

			len = strlen(line);

			if (strlen(line) > BUFFER_SIZE / 3)
			{
				len = BUFFER_SIZE / 3;

				next_line = &line[len];

				*next_line++ = 0;
			}

			if (str_len(ses->more_output) < BUFFER_SIZE / 3)
			{
				if (!HAS_BIT(ses->telopts, TELOPT_FLAG_PROMPT))
				{
					if (ses->packet_patch || HAS_BIT(ses->config_flags, CONFIG_FLAG_AUTOPATCH))
					{
						if (ses->list[LIST_PROMPT]->list[0])
						{
							if (!detect_prompt(ses, line))
							{
								str_cat(&ses->more_output, line);
								ses->check_output = gtd->utime + (ses->packet_patch ? ses->packet_patch : 500000ULL);

								check_all_events(ses, EVENT_FLAG_OUTPUT, 0, 0, "PACKET PATCH");
								break;
							}
						}
						else if (ses->packet_patch || HAS_BIT(ses->config_flags, CONFIG_FLAG_AUTOPROMPT))
						{
							str_cat(&ses->more_output, line);
							ses->check_output = gtd->utime + (ses->packet_patch ? ses->packet_patch : 500000ULL);

							check_all_events(ses, EVENT_FLAG_OUTPUT, 0, 0, "PACKET PATCH");
							break;
						}
					}
				}
			}
		}

		if (ses->check_output)
		{
			process_more_output(ses, line, next_line == NULL);

			continue;
		}

		if (HAS_BIT(ses->charset, CHARSET_FLAG_ALL_TOUTF8))
		{
			all_to_utf8(ses, line, linebuf);
		}
		else
		{
			strcpy(linebuf, line);
		}
		gtd->mud_output_line = linebuf;

		process_one_line(ses, linebuf, next_line == NULL);

		gtd->mud_output_line = gtd->mud_output_buf + gtd->mud_output_len;
	}
	DEL_BIT(cts->flags, SES_FLAG_READMUD);

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_SPLIT))
	{
		restore_pos(gtd->ses);
	}
	gtd->mud_output_len = 0;

	pop_call();
	return;
}

void process_more_output(struct session *ses, char *append, int prompt)
{
	char line[STRING_SIZE];

	int readmud = HAS_BIT(ses->flags, SES_FLAG_READMUD);

	if (readmud == 0)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
		{
			save_pos(ses);

			goto_pos(ses, ses->split->bot_row, 1);
		}
		SET_BIT(ses->flags, SES_FLAG_READMUD);
	}

	if (*append)
	{
		str_cat(&ses->more_output, append);
	}

	if (HAS_BIT(ses->charset, CHARSET_FLAG_ALL_TOUTF8))
	{
		all_to_utf8(ses, ses->more_output, line);
	}
	else
	{
		strcpy(line, ses->more_output);
	}
	str_cpy(&ses->more_output, "");
	ses->check_output = 0;

	process_one_line(ses, line, prompt);

	if (readmud == 0)
	{
		DEL_BIT(ses->flags, SES_FLAG_READMUD);

		if (HAS_BIT(ses->flags, SES_FLAG_SPLIT))
		{
			restore_pos(ses);
		}
	}
}

void process_one_line(struct session *ses, char *linebuf, int prompt)
{
	char temp[STRING_SIZE];
	int str_len, raw_len;

	push_call("process_one_line(%p,%p,%d)",ses,linebuf,prompt);

	raw_len = strlen(linebuf);
	str_len = strip_vt102_codes(linebuf, temp);

	check_all_events(ses, SUB_SEC|EVENT_FLAG_OUTPUT, 0, 2, "RECEIVED LINE", linebuf, temp);

	if (check_all_events(ses, SUB_SEC|EVENT_FLAG_CATCH, 0, 2, "CATCH RECEIVED LINE", linebuf, temp))
	{
		pop_call();
		return;
	}

	if (str_len && prompt)
	{
		check_all_events(ses, SUB_SEC|EVENT_FLAG_OUTPUT, 0, 4, "RECEIVED PROMPT", linebuf, temp, ntos(raw_len), ntos(str_len));

		if (check_all_events(ses, SUB_SEC|EVENT_FLAG_CATCH, 0, 4, "CATCH RECEIVED PROMPT", linebuf, temp, ntos(raw_len), ntos(str_len)))
		{
			pop_call();
			return;
		}
	}

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_COLORPATCH))
	{
		sprintf(temp, "%s%s%s", ses->color_patch, linebuf, "\e[0m");

		get_color_codes(ses->color_patch, linebuf, ses->color_patch, GET_ALL);

		strcpy(linebuf, temp);
	}

	check_one_line(ses, linebuf);   /* changes linebuf */

	/*
		Take care of gags, vt102 support still goes
	*/

	if (ses->gagline > 0)
	{
		ses->gagline--;

		strip_non_vt102_codes(linebuf, temp);

		print_stdout(0, 0, "%s", temp);

		strip_vt102_codes(linebuf, temp);

		show_debug(ses, LIST_GAG, NULL, COLOR_DEBUG "#DEBUG GAG " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} " COLOR_COMMAND "[" COLOR_STRING "%d" COLOR_COMMAND "]", temp, ses->gagline + 1);

		pop_call();
		return;
	}

	if (HAS_BIT(ses->event_flags, EVENT_FLAG_OUTPUT|EVENT_FLAG_CATCH))
	{
		strip_vt102_codes(linebuf, temp);

		check_all_events(ses, SUB_SEC|EVENT_FLAG_OUTPUT, 0, 3, "PROCESSED LINE", linebuf, temp, ntos(prompt));

		if (check_all_events(ses, SUB_SEC|EVENT_FLAG_CATCH, 0, 3, "CATCH PROCESSED LINE", linebuf, temp, ntos(prompt)))
		{
			pop_call();
			return;
		}
	}

	add_line_buffer(ses, linebuf, prompt);

	if (ses == gtd->ses)
	{
		char *output = str_alloc_stack(0);
		
		str_cpy(&output, linebuf);

		print_line(ses, &output, prompt);

		if (!IS_SPLIT(ses))
		{
			if (prompt)
			{
				int height, width;

				word_wrap_split(ses, linebuf, temp, ses->wrap, 0, 0, FLAG_NONE, &height, &width);

				if (height > 1)
				{
					word_wrap_split(ses, linebuf, temp, ses->wrap, height, height, WRAP_FLAG_SPLIT, &height, &width);
				}
				ses->input->str_off = 1 + width;
			}
			else
			{
				ses->input->str_off = 1;
			}
		}
		else
		{
			if (!HAS_BIT(ses->flags, SES_FLAG_SPLIT))
			{
				ses->input->str_off = 1;
			}
		}
	}
	pop_call();
	return;
}
