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
#include <sys/types.h>

/*
	IPv6 compatible connect code.
*/

#ifdef HAVE_GETADDRINFO

int connect_mud(struct session *ses, char *host, char *port)
{
	int sock, error;
	struct addrinfo *address;
	static struct addrinfo hints;
	char ip[100];

	if (!is_number(port))
	{
		tintin_puts(ses, "#THE PORT SHOULD BE A NUMBER.");
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

//	fcntl(sock, F_SETFL, O_NDELAY);

        ses->connect_error = connect(sock, address->ai_addr, address->ai_addrlen);

        if (ses->connect_error)
        {
                syserr_printf(ses, "connect_mud: connect");

                close(sock);

                freeaddrinfo(address);

                return -1;
        }

	if (fcntl(sock, F_SETFL, O_NDELAY|O_NONBLOCK) == -1)
	{
		syserr_printf(ses, "connect_mud: fcntl O_NDELAY|O_NONBLOCK");

		close(sock);

		freeaddrinfo(address);

		return -1;
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

	print_stdout("debug: NO ADDRESS INFO?\n");

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
		tintin_puts(ses, "#THE PORT SHOULD BE A NUMBER.");
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

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "SEND OUTPUT", line, ntos(size));

	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "CATCH SEND OUTPUT", line, ntos(size)))
	{
		pop_call();
		return;
	}

	if (ses == gts)
	{
		if (HAS_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK))
		{
			tintin_printf2(ses, "#THIS SESSION IS CHILD LOCKED, PRESS CTRL-D TO EXIT.");
		}
		else
		{
			tintin_printf2(ses, "#NO SESSION ACTIVE. USE: %csession {name} {host} {port} TO START ONE.", gtd->tintin_char);
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

		size = utf8_to_all(ses, line, buf);

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

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "SENT OUTPUT", line, ntos(size));

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
	char *line, *next_line;
	char linebuf[BUFFER_SIZE];
	int len;
	struct session *cts;

	push_call("readmud(%p)", ses);

	gtd->mud_output_len = 0;

	if (gtd->mud_output_len < BUFFER_SIZE)
	{
		check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "RECEIVED OUTPUT", gtd->mud_output_buf);

		if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "CATCH RECEIVED OUTPUT", gtd->mud_output_buf))
		{
			pop_call();
			return;
		}
	}

	/* separate into lines and print away */

	// cts = current tintin session, may have to make this global to avoid glitches

	cts = gtd->ses;

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_SPLIT))
	{
		save_pos(gtd->ses);

		goto_pos(gtd->ses, gtd->ses->split->bot_row, 1);
	}

	SET_BIT(cts->flags, SES_FLAG_READMUD);

	for (line = gtd->mud_output_buf ; line && *line ; line = next_line)
	{
		next_line = strchr(line, '\n');

		if (next_line)
		{
			if (next_line - line >= BUFFER_SIZE / 3)
			{
				// This is not ideal, but being a rare case it'll suffice for now

				next_line = &line[BUFFER_SIZE / 3];
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

			if (strlen(ses->more_output) < BUFFER_SIZE / 3)
			{
				if (!HAS_BIT(ses->telopts, TELOPT_FLAG_PROMPT))
				{
					if (ses->packet_patch)
					{
						strcat(ses->more_output, line);
						ses->check_output = utime() + ses->packet_patch;

						break;
					}
					else if (HAS_BIT(ses->flags, SES_FLAG_AUTOPATCH))
					{
						if (ses->list[LIST_PROMPT]->list[0])
						{
							if (!detect_prompt(ses, line))
							{
								strcat(ses->more_output, line);
								ses->check_output = utime() + 500000ULL;
								break;
							}
						}
						else if (HAS_BIT(ses->flags, SES_FLAG_AUTOPROMPT))
						{
							strcat(ses->more_output, line);
							ses->check_output = utime() + 500000ULL;
							break;
						}
					}
				}
			}
		}

		if (ses->more_output[0])
		{
			if (ses->check_output)
			{
				strcat(ses->more_output, line);
				strcpy(linebuf, ses->more_output);

				ses->more_output[0] = 0;
			}
			else
			{
				strcpy(linebuf, line);
			}
		}
		else
		{
			strcpy(linebuf, line);
		}

		if (HAS_BIT(ses->charset, CHARSET_FLAG_ALL_TOUTF8))
		{
			char tempbuf[BUFFER_SIZE];

			all_to_utf8(ses, linebuf, tempbuf);

			process_mud_output(ses, tempbuf, next_line == NULL);
		}
		else
		{
			process_mud_output(ses, linebuf, next_line == NULL);
		}
	}
	DEL_BIT(cts->flags, SES_FLAG_READMUD);

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_SPLIT))
	{
		restore_pos(gtd->ses);
	}

	pop_call();
	return;
}


void process_mud_output(struct session *ses, char *linebuf, int prompt)
{
	char line[STRING_SIZE];
	int str_len, raw_len;

	push_call("process_mud_output(%p,%p,%d)",ses,linebuf,prompt);

	ses->check_output = 0;

	raw_len = strlen(linebuf);
	str_len = strip_vt102_codes(linebuf, line);

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "RECEIVED LINE", linebuf, line);

	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "CATCH RECEIVED LINE", linebuf, line))
	{
		pop_call();
		return;
	}

	if (str_len && prompt)
	{
		check_all_events(ses, SUB_ARG|SUB_SEC, 0, 4, "RECEIVED PROMPT", linebuf, line, ntos(raw_len), ntos(str_len));

		if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 4, "CATCH RECEIVED PROMPT", linebuf, line, ntos(raw_len), ntos(str_len)))
		{
			pop_call();
			return;
		}
	}

	if (HAS_BIT(ses->flags, SES_FLAG_COLORPATCH))
	{
		sprintf(line, "%s%s%s", ses->color_patch, linebuf, "\e[0m");

		get_color_codes(ses->color_patch, linebuf, ses->color_patch, GET_ALL);

		linebuf = line;
	}

	do_one_line(linebuf, ses);   /* changes linebuf */

	/*
		Take care of gags, vt102 support still goes
	*/

	if (ses->gagline > 0)
	{
		ses->gagline--;

		strip_non_vt102_codes(linebuf, line);

		print_stdout("%s", line);

		strip_vt102_codes(linebuf, line);

		show_debug(ses, LIST_GAG, "#DEBUG GAG {%d} {%s}", ses->gagline + 1, line);

		pop_call();
		return;
	}

	add_line_buffer(ses, linebuf, prompt);

	if (ses == gtd->ses)
	{
		char *output = str_dup(linebuf);

		print_line(ses, &output, prompt);

		str_free(output);

		if (prompt)
		{
			if (!IS_SPLIT(ses))
			{
				ses->input->off = 1 + strip_vt102_strlen(ses, linebuf);
			}
		}
	}
	pop_call();
	return;
}
