/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright 2001-2019 Igor van den Hoven                                    *
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
*                      coded by Igor van den Hoven 2009                       *
******************************************************************************/

#include "tintin.h"
#include "telnet.h"

void unannounce_support(struct session *ses, struct port_data *buddy);
void telopt_debug(struct session *ses, char *format, ...);
void debug_telopts(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen);
void send_echo_off(struct session *ses, struct port_data *buddy);
void send_echo_on(struct session *ses, struct port_data *buddy);
void send_eor(struct session *ses, struct port_data *buddy);
int process_do_eor(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_will_ttype(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_sb_ttype_is(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_sb_naws(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_will_new_environ(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_sb_new_environ(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_do_charset(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_sb_charset(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_do_msdp(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_sb_msdp(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_do_gmcp(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_sb_gmcp(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_do_mssp(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int start_mccp2(struct session *ses, struct port_data *buddy);
void process_mccp2(struct session *ses, struct port_data *buddy);
int process_do_mccp2(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_dont_mccp2(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_do_mccp3(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
int process_sb_mccp3(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );

int skip_sb(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );

#define TELOPT_DEBUG 1

struct iac_type
{
	int      size;
	unsigned char * code;
	int   (* func) (struct session *ses, struct port_data *buddy, unsigned char *src, int srclen );
};

struct iac_type iac_server_table [] =
{
	{ 3, (unsigned char []) { IAC, DO,   TELOPT_EOR, 0 },                       &process_do_eor},

	{ 3, (unsigned char []) { IAC, WILL, TELOPT_TTYPE, 0 },                     &process_will_ttype},
	{ 4, (unsigned char []) { IAC, SB,   TELOPT_TTYPE, ENV_IS, 0 },             &process_sb_ttype_is},

	{ 3, (unsigned char []) { IAC, SB,   TELOPT_NAWS, 0 },                      &process_sb_naws},

	{ 3, (unsigned char []) { IAC, WILL, TELOPT_NEW_ENVIRON, 0 },               &process_will_new_environ},
	{ 3, (unsigned char []) { IAC, SB,   TELOPT_NEW_ENVIRON, 0 },               &process_sb_new_environ},

	{ 3, (unsigned char []) { IAC, DO,   TELOPT_CHARSET, 0 },                   &process_do_charset},
	{ 3, (unsigned char []) { IAC, SB,   TELOPT_CHARSET, 0 },                   &process_sb_charset},

	{ 3, (unsigned char []) { IAC, DO,   TELOPT_MSSP, 0 },                      &process_do_mssp},

	{ 3, (unsigned char []) { IAC, DO,   TELOPT_MSDP, 0 },                      &process_do_msdp},
	{ 3, (unsigned char []) { IAC, SB,   TELOPT_MSDP, 0 },                      &process_sb_msdp},

	{ 3, (unsigned char []) { IAC, DO,   TELOPT_GMCP, 0 },                      &process_do_gmcp},
	{ 3, (unsigned char []) { IAC, SB,   TELOPT_GMCP, 0 },                      &process_sb_gmcp},

	{ 3, (unsigned char []) { IAC, DO,   TELOPT_MCCP2, 0 },                     &process_do_mccp2},
	{ 3, (unsigned char []) { IAC, DONT, TELOPT_MCCP2, 0 },                     &process_dont_mccp2},

	{ 3, (unsigned char []) { IAC, DO,   TELOPT_MCCP3, 0 },                     &process_do_mccp3},
	{ 5, (unsigned char []) { IAC, SB,   TELOPT_MCCP3, IAC, SE, 0 },            &process_sb_mccp3},

	{ 0, NULL,                                                                  NULL}
};

/*
	Call this to announce support for telopts marked as such in tables.c
*/

void announce_support(struct session *ses, struct port_data *buddy)
{
	int i;

	push_call("announce_support(%p,%p)",ses,buddy);
	
	for (i = 0 ; i < 255 ; i++)
	{
		if (telopt_table[i].flags)
		{
			if (HAS_BIT(telopt_table[i].flags, ANNOUNCE_WILL))
			{
				port_telnet_printf(ses, buddy, 3, "%c%c%c", IAC, WILL, i);
			}
			if (HAS_BIT(telopt_table[i].flags, ANNOUNCE_DO))
			{
				port_telnet_printf(ses, buddy, 3, "%c%c%c", IAC, DO, i);
			}
		}
	}
	pop_call();
	return;
}

/*
	Call this right before a copyover to reset the telnet state
*/

void unannounce_support(struct session *ses, struct port_data *buddy)
{
	int i;

	for (i = 0 ; i < 255 ; i++)
	{
		if (telopt_table[i].flags)
		{
			if (HAS_BIT(telopt_table[i].flags, ANNOUNCE_WILL))
			{
				port_telnet_printf(ses, buddy, 3, "%c%c%c", IAC, WONT, i);
			}
			if (HAS_BIT(telopt_table[i].flags, ANNOUNCE_DO))
			{
				port_telnet_printf(ses, buddy, 3, "%c%c%c", IAC, DONT, i);
			}
		}
	}
}

/*
	This is the main routine that strips out and handles telopt negotiations.
	It also deals with \r and \0 so commands are separated by a single \n.
*/

int server_translate_telopts(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen, unsigned char *out, int outlen)
{
	int cnt, skip;
	unsigned char *pti, *pto;

	push_call("server_translate_telopts(%p,%p,%p,%d,%p,%d)",ses,buddy,src,srclen,out,outlen);

	pti = src;
	pto = out + outlen;

	if (srclen > 0 && buddy->mccp3)
	{
		buddy->mccp3->next_in   = pti;
		buddy->mccp3->avail_in  = srclen;

		buddy->mccp3->next_out   = gtd->mccp_buf;
		buddy->mccp3->avail_out  = gtd->mccp_len;

		inflate:

		switch (inflate(buddy->mccp3, Z_SYNC_FLUSH))
		{
			case Z_BUF_ERROR:
				if (buddy->mccp3->avail_out == 0)
				{
					gtd->mccp_len *= 2;
					gtd->mccp_buf  = (unsigned char *) realloc(gtd->mccp_buf, gtd->mccp_len);

					buddy->mccp3->avail_out = gtd->mccp_len / 2;
					buddy->mccp3->next_out  = gtd->mccp_buf + gtd->mccp_len / 2;

					goto inflate;
				}
				else
				{
					port_socket_printf(ses, buddy, "%c%c%c", IAC, DONT, TELOPT_MCCP3);
					inflateEnd(buddy->mccp3);
					free(buddy->mccp3);
					buddy->mccp3 = NULL;
					srclen = 0;
				}
				break;

			case Z_OK:
				if (buddy->mccp3->avail_out == 0)
				{
					gtd->mccp_len *= 2;
					gtd->mccp_buf  = (unsigned char *) realloc(gtd->mccp_buf, gtd->mccp_len);

					buddy->mccp3->avail_out = gtd->mccp_len / 2;
					buddy->mccp3->next_out  = gtd->mccp_buf + gtd->mccp_len / 2;

					goto inflate;
				}
				srclen = buddy->mccp3->next_out - gtd->mccp_buf;
				pti = gtd->mccp_buf;

				if (srclen + outlen > BUFFER_SIZE)
				{
					srclen = BUFFER_SIZE - outlen - 1;
				}
				break;

			case Z_STREAM_END:
				port_log_printf(ses, buddy, "MCCP3: Compression end, disabling MCCP3.");

				skip = buddy->mccp3->next_out - gtd->mccp_buf;

				pti += (srclen - buddy->mccp3->avail_in);
				srclen = buddy->mccp3->avail_in;

				inflateEnd(buddy->mccp3);
				free(buddy->mccp3);
				buddy->mccp3 = NULL;

				while (skip + srclen + 1 > gtd->mccp_len)
				{
					gtd->mccp_len *= 2;
					gtd->mccp_buf  = (unsigned char *) realloc(gtd->mccp_buf, gtd->mccp_len);
				}
				memcpy(gtd->mccp_buf + skip, pti, srclen);
				pti = gtd->mccp_buf;
				srclen += skip;
				break;

			default:
				port_log_printf(ses, buddy, "MCCP3: Compression error, disabling MCCP3.");

				syserr_printf(ses, "server_translate_telopts: inflate:");

				port_socket_printf(ses, buddy, "%c%c%c", IAC, DONT, TELOPT_MCCP3);
				inflateEnd(buddy->mccp3);
				free(buddy->mccp3);
				buddy->mccp3 = NULL;
				srclen = 0;
				break;
		}
	}

	// packet patching

	if (buddy->teltop)
	{
		if (buddy->teltop + srclen + 1 < BUFFER_SIZE)
		{
			memcpy(buddy->telbuf + buddy->teltop, pti, srclen);

			srclen += buddy->teltop;

			pti = (unsigned char *) buddy->telbuf;
		}
		else
		{
			port_log_printf(ses, buddy, "server_translate_telopts: buffer overflow.");
		}
		buddy->teltop = 0;
	}

	while (srclen > 0)
	{
		switch (*pti)
		{
			case IAC:
				skip = 2;

				debug_telopts(ses, buddy, pti, srclen);

				for (cnt = 0 ; iac_server_table[cnt].code ; cnt++)
				{
					if (srclen < iac_server_table[cnt].size)
					{
						if (!memcmp(pti, iac_server_table[cnt].code, srclen))
						{
							skip = iac_server_table[cnt].size;

							break;
						}
					}
					else
					{
						if (!memcmp(pti, iac_server_table[cnt].code, iac_server_table[cnt].size))
						{
							skip = iac_server_table[cnt].func(ses, buddy, pti, srclen);

							if (iac_server_table[cnt].func == process_sb_mccp3)
							{
								pop_call();
								return server_translate_telopts(ses, buddy, pti + skip, srclen - skip, out, pto - out);
							}
							break;
						}
					}
				}

				if (iac_server_table[cnt].code == NULL && srclen > 1)
				{
					switch (pti[1])
					{
						case WILL:
						case DO:
						case WONT:
						case DONT:
							skip = 3;
							break;

						case SB:
							skip = skip_sb(ses, buddy, pti, srclen);
							break;

						case IAC:
							*pto++ = *pti++;
							srclen--;
							skip = 1;
							break;

						default:
							if (TELCMD_OK(pti[1]))
							{
								skip = 2;
							}
							else
							{
								skip = 1;
							}
							break;
					}
				}

				if (skip <= srclen)
				{
					pti += skip;
					srclen -= skip;
				}
				else
				{
					memcpy(buddy->telbuf, pti, srclen);
					buddy->teltop = srclen;

					*pto = 0;
					pop_call();
					return strlen((char *) out);
				}
				break;

			case '\r':
				if (srclen > 1 && pti[1] == '\0')
				{
					*pto++ = '\n';
				}
				pti++;
				srclen--;
				break;

			case '\0':
				pti++;
				srclen--;
				break;

			default:
				*pto++ = *pti++;
				srclen--;
				break;
		}
	}
	*pto = 0;

	pop_call();
	return strlen((char *) out);
}

void telopt_debug(struct session *ses, char *format, ...)
{
	char buf[BUFFER_SIZE];
	va_list args;

	if (HAS_BIT(ses->telopts, TELOPT_FLAG_DEBUG))
	{
		va_start(args, format);
		vsprintf(buf, format, args);
		va_end(args);

		tintin_puts(ses, buf);
	}
}

void debug_telopts(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen)
{
	if (srclen > 1 && TELOPT_DEBUG)
	{
		switch(src[1])
		{
			case IAC:
				tintin_printf2(ses, "RCVD IAC IAC");
				break;

			case DO:
			case DONT:
			case WILL:
			case WONT:
			case SB:
				if (srclen > 2)
				{
					if (src[1] == SB)
					{
						if (skip_sb(ses, buddy, src, srclen) == srclen + 1)
						{
							tintin_printf2(ses, "RCVD IAC SB %s ?", TELOPT(src[2]));
						}
						else
						{
							tintin_printf2(ses, "RCVD IAC SB %s IAC SE", TELOPT(src[2]));
						}
					}
					else
					{
						tintin_printf2(ses, "RCVD IAC %s %s", TELCMD(src[1]), TELOPT(src[2]));
					}
				}
				else
				{
					tintin_printf2(ses, "RCVD IAC %s ?", TELCMD(src[1]));
				}
				break;

			default:
				if (TELCMD_OK(src[1]))
				{
					tintin_printf2(ses, "RCVD IAC %s", TELCMD(src[1]));
				}
				else
				{
					tintin_printf2(ses, "RCVD IAC %d", src[1]);
				}
				break;
		}
	}
	else
	{
		tintin_printf2(ses, "RCVD IAC ?");
	}
}

/*
	Send to client to have it disable local echo
*/

void send_echo_off(struct session *ses, struct port_data *buddy)
{
	SET_BIT(buddy->comm_flags, COMM_FLAG_PASSWORD);

	port_socket_printf(ses, buddy, "%c%c%c", IAC, WILL, TELOPT_ECHO);
}

/*
	Send to client to have it enable local echo
*/

void send_echo_on(struct session *ses, struct port_data *buddy)
{
	DEL_BIT(buddy->comm_flags, COMM_FLAG_PASSWORD);

	port_socket_printf(ses, buddy, "%c%c%c", IAC, WONT, TELOPT_ECHO);
}

/*
	Send right after the prompt to mark it as such.
*/

void send_eor(struct session *ses, struct port_data *buddy)
{
	if (HAS_BIT(buddy->comm_flags, COMM_FLAG_EOR))
	{
		port_socket_printf(ses, buddy, "%c%c", IAC, EOR);
	}
}

/*
	End Of Record negotiation - not enabled by default in tables.c
*/

int process_do_eor(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	SET_BIT(buddy->comm_flags, COMM_FLAG_EOR);

	return 3;
}

/*
	Terminal Type negotiation - make sure buddy->ttype is initialized.
*/

int process_will_ttype(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	if (*buddy->ttype == 0)
	{
		// Request the first three terminal types to see if MTTS is supported, next reset to default.

		port_socket_printf(ses, buddy, "%c%c%c%c%c%c", IAC, SB, TELOPT_TTYPE, ENV_SEND, IAC, SE);
		port_socket_printf(ses, buddy, "%c%c%c%c%c%c", IAC, SB, TELOPT_TTYPE, ENV_SEND, IAC, SE);
		port_socket_printf(ses, buddy, "%c%c%c%c%c%c", IAC, SB, TELOPT_TTYPE, ENV_SEND, IAC, SE);
		port_socket_printf(ses, buddy, "%c%c%c", IAC, DONT, TELOPT_TTYPE);
	}
	return 3;
}

int process_sb_ttype_is(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	char val[BUFFER_SIZE];
	char *pto;
	int i;

	if (skip_sb(ses, buddy, src, srclen) > srclen)
	{
		return srclen + 1;
	}

	pto = val;

	for (i = 4 ; i < srclen && src[i] != SE ; i++)
	{
		switch (src[i])
		{
			default:			
				*pto++ = src[i];
				break;

			case IAC:
				*pto = 0;

				if (TELOPT_DEBUG)
				{
					tintin_printf2(ses, "INFO IAC SB TTYPE RCVD VAL %s.", val);
				}

				if (*buddy->ttype == 0)
				{
					RESTRING(buddy->ttype, val);
				}
				else
				{
					if (sscanf(val, "MTTS %d", &buddy->mtts_flags) == 1)
					{
						if (HAS_BIT(buddy->mtts_flags, MTTS_FLAG_256COLORS))
						{
							SET_BIT(buddy->comm_flags, COMM_FLAG_256COLORS);
						}

						if (HAS_BIT(buddy->mtts_flags, MTTS_FLAG_UTF8))
						{
							SET_BIT(buddy->comm_flags, COMM_FLAG_UTF8);
						}
					}

					if (strstr(val, "-256color") || strstr(val, "-256COLOR") || strcasecmp(val, "xterm"))
					{
						SET_BIT(buddy->comm_flags, COMM_FLAG_256COLORS);
					}
				}
				break;
		}
	}
	return i + 1;
}

/*
	NAWS: Negotiate About Window Size
*/

int process_sb_naws(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	int i, j;

	buddy->cols = buddy->rows = 0;

	if (skip_sb(ses, buddy, src, srclen) > srclen)
	{
		return srclen + 1;
	}

	for (i = 3, j = 0 ; i < srclen && j < 4 ; i++, j++)
	{
		switch (j)
		{
			case 0:
				buddy->cols += (src[i] == IAC) ? src[i++] * 256 : src[i] * 256;
				break;
			case 1:
				buddy->cols += (src[i] == IAC) ? src[i++] : src[i];
				break;
			case 2:
				buddy->rows += (src[i] == IAC) ? src[i++] * 256 : src[i] * 256;
				break;
			case 3:
				buddy->rows += (src[i] == IAC) ? src[i++] : src[i];
				break;
		}
	}

	if (TELOPT_DEBUG)
	{
		tintin_printf2(ses, "INFO IAC SB NAWS RCVD ROWS %d COLS %d", buddy->rows, buddy->cols);
	}

	return skip_sb(ses, buddy, src, srclen);
}

/*
	NEW ENVIRON, used here to discover Windows telnet.
*/

int process_will_new_environ(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	port_socket_printf(ses, buddy, "%c%c%c%c%c%s%c%c", IAC, SB, TELOPT_NEW_ENVIRON, ENV_SEND, ENV_VAR, "SYSTEMTYPE", IAC, SE);

	return 3;
}

int process_sb_new_environ(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	char var[BUFFER_SIZE], val[BUFFER_SIZE];
	char *pto;
	int i;

	if (skip_sb(ses, buddy, src, srclen) > srclen)
	{
		return srclen + 1;
	}

	var[0] = val[0] = 0;

	i = 4;

	while (i < srclen && src[i] != SE)
	{
		switch (src[i])
		{
			case ENV_VAR:
			case ENV_USR:
				i++;
				pto = var;

				while (i < srclen && src[i] >= 32 && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;

				if (src[i] != ENV_VAL)
				{
					tintin_printf2(ses, "INFO IAC SB NEW-ENVIRON RCVD %d VAR %s", src[3], var);
				}
				break;

			case ENV_VAL:
				i++;
				pto = val;

				while (i < srclen && src[i] >= 32 && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;

				if (TELOPT_DEBUG)
				{
					tintin_printf2(ses, "INFO IAC SB NEW-ENVIRON RCVD %d VAR %s VAL %s", src[3], var, val);
				}

				if (src[3] == ENV_IS)
				{
					// Detect Windows telnet and enable remote echo.

					if (!strcasecmp(var, "SYSTEMTYPE") && !strcasecmp(val, "WIN32"))
					{
						if (!strcasecmp(buddy->ttype, "ANSI"))
						{
							SET_BIT(buddy->comm_flags, COMM_FLAG_REMOTEECHO);

							RESTRING(buddy->ttype, "WINDOWS TELNET");
						}
					}

					// Get the real IP address when connecting to mudportal and other MTTS compliant proxies.

					if (!strcasecmp(var, "IPADDRESS"))
					{
						RESTRING(buddy->proxy, val);
					}
				}
				break;

			default:
				i++;
				break;
		}
	}
	return i + 1;
}

/*
	CHARSET, used to detect UTF-8 support
*/

int process_do_charset(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	port_socket_printf(ses, buddy, "%c%c%c%c%c%s%c%c", IAC, SB, TELOPT_CHARSET, CHARSET_REQUEST, ' ', "UTF-8", IAC, SE);

	return 3;
}

int process_sb_charset(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	char val[BUFFER_SIZE];
	char *pto;
	int i;

	if (skip_sb(ses, buddy, src, srclen) > srclen)
	{
		return srclen + 1;
	}

	val[0] = 0;

	i = 5;

	while (i < srclen && src[i] != SE && src[i] != src[4])
	{
		pto = val;

		while (i < srclen && src[i] != src[4] && src[i] != IAC)
		{
			*pto++ = src[i++];
		}
		*pto = 0;

		if (TELOPT_DEBUG)
		{
			tintin_printf2(ses, "INFO IAC SB CHARSET RCVD %d VAL %s", src[3], val);
		}

		if (src[3] == CHARSET_ACCEPTED)
		{
			if (!strcasecmp(val, "UTF-8"))
			{
				SET_BIT(buddy->comm_flags, COMM_FLAG_UTF8);
			}
		}
		else if (src[3] == CHARSET_REJECTED)
		{
			if (!strcasecmp(val, "UTF-8"))
			{
				DEL_BIT(buddy->comm_flags, COMM_FLAG_UTF8);
			}
		}
		i++;
	}
	return i + 1;
}

/*
	MSDP: Mud Server Data Protocol

	http://tintin.sourceforge.net/msdp
*/

int process_do_msdp(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	int index;

	if (buddy->msdp_data)
	{
		return 3;
	}

	buddy->msdp_data = (struct msdp_data **) calloc(gtd->msdp_table_size, sizeof(struct msdp_data *));

	for (index = 0 ; index < gtd->msdp_table_size ; index++)
	{
		buddy->msdp_data[index] = (struct msdp_data *) calloc(1, sizeof(struct msdp_data));

		buddy->msdp_data[index]->flags = msdp_table[index].flags;
		buddy->msdp_data[index]->value = strdup("");
	}

	tintin_printf2(ses, "INFO MSDP INITIALIZED");

	// Easiest to handle variable initialization here.

	msdp_update_var(ses, buddy, "SPECIFICATION", "http://tintin.sourceforge.net/msdp");

	msdp_update_varf(ses, buddy, "SCREEN_ROWS",     "%d", gtd->screen->rows);
	msdp_update_varf(ses, buddy, "SCREEN_COLS",     "%d", gtd->screen->cols);
	msdp_update_varf(ses, buddy, "SCREEN_HEIGHT",   "%d", gtd->screen->height);
	msdp_update_varf(ses, buddy, "SCREEN_WIDTH",    "%d", gtd->screen->width);

	msdp_update_varf(ses, buddy, "SCREEN_LOCATION_HEIGHT", "%d", gtd->screen->pos_height);
	msdp_update_varf(ses, buddy, "SCREEN_LOCATION_WIDTH",  "%d", gtd->screen->pos_width);

	msdp_update_varf(ses, buddy, "SCREEN_FOCUS",     "%d", gtd->screen->focus);
	msdp_update_varf(ses, buddy, "SCREEN_MINIMIZED", "%d", gtd->screen->minimized);

	return 3;
}

int process_sb_msdp(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	char var[BUFFER_SIZE], val[BUFFER_SIZE];
	char *pto;
	int i, nest;

	if (skip_sb(ses, buddy, src, srclen) > srclen)
	{
		return srclen + 1;
	}

	var[0] = val[0] = 0;

	i = 3;
	nest = 0;

	while (i < srclen && src[i] != SE)
	{
		switch (src[i])
		{
			case MSDP_VAR:
				i++;
				pto = var;

				while (i < srclen && src[i] != MSDP_VAL && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;

				break;

			case MSDP_VAL:
				i++;
				pto = val;

				while (i < srclen && src[i] != IAC)
				{
					if (src[i] == MSDP_TABLE_OPEN || src[i] == MSDP_ARRAY_OPEN)
					{
						nest++;
					}
					else if (src[i] == MSDP_TABLE_CLOSE || src[i] == MSDP_ARRAY_CLOSE)
					{
						nest--;
					}
					else if (nest == 0 && (src[i] == MSDP_VAR || src[i] == MSDP_VAL))
					{
						break;
					}
					*pto++ = src[i++];
				}
				*pto = 0;

				if (nest == 0)
				{
					if (buddy->msdp_data)
					{
						process_msdp_varval(ses, buddy, var, val);
					}
				}
				break;

			default:
				i++;
				break;
		}
	}
	return i + 1;
}

// MSDP over GMCP

int process_do_gmcp(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	if (buddy->msdp_data)
	{
		return 3;
	}
	tintin_printf2(ses, "INFO MSDP OVER GMCP INITIALIZED");

	SET_BIT(buddy->comm_flags, COMM_FLAG_GMCP);

	return process_do_msdp(ses, buddy, src, srclen);
}

int process_sb_gmcp(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	char out[BUFFER_SIZE];
	int outlen, skiplen;

	skiplen = skip_sb(ses, buddy, src, srclen);

	if (skiplen > srclen)
	{
		return srclen + 1;
	}

	outlen = json2msdp(src, srclen, out);

	process_sb_msdp(ses, buddy, (unsigned char *) out, outlen);

	return skiplen;
}

/*
	MSSP: Mud Server Status Protocol

	http://tintin.sourceforge.net/mssp

	Uncomment and update as needed
*/

int process_do_mssp(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	char buffer[BUFFER_SIZE] = { 0 };

	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "NAME",              MSSP_VAL, "TINTIN COMMANDER");
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "PLAYERS",           MSSP_VAL, ses->port->total);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "UPTIME",            MSSP_VAL, ses->created);

//	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "HOSTNAME",          MSSP_VAL, "example.com");
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "PORT",              MSSP_VAL, ses->port->port);

	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "CODEBASE",          MSSP_VAL, CLIENT_NAME);
//	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "CONTACT",           MSSP_VAL, "mud@example.com");
//	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "LANGUAGE",          MSSP_VAL, "English");
//	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "MINIMUM AGE",       MSSP_VAL, "13");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "WEBSITE",           MSSP_VAL, "https://tintin.sourceforge.io");

	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "FAMILY",            MSSP_VAL, "TINTIN");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "INTERMUD",          MSSP_VAL, "Arachnos");

	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "ANSI",              MSSP_VAL, 1);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "MCCP",              MSSP_VAL, 1);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "MSDP",              MSSP_VAL, 1);
//	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "MSP",               MSSP_VAL, 0);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "UTF-8",             MSSP_VAL, 1);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "VT100",             MSSP_VAL, 1);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "XTERM 256 COLORS",  MSSP_VAL, 1);

	port_socket_printf(ses, buddy, "%c%c%c%s%c%c", IAC, SB, TELOPT_MSSP, buffer, IAC, SE);

	return 3;
}

/*
	MCCP: Mud Client Compression Protocol
*/

int start_mccp2(struct session *ses, struct port_data *buddy)
{
	z_stream *stream;

	if (buddy->mccp2)
	{
		return TRUE;
	}

	stream = calloc(1, sizeof(z_stream));

	stream->next_in	    = NULL;
	stream->avail_in    = 0;

	stream->next_out    = gtd->mccp_buf;
	stream->avail_out   = gtd->mccp_len;

	stream->data_type   = Z_ASCII;
	stream->zalloc      = zlib_alloc;
	stream->zfree       = zlib_free;
	stream->opaque      = Z_NULL;

	/*
		12, 5 = 32K of memory, more than enough
	*/

	if (deflateInit2(stream, Z_BEST_COMPRESSION, Z_DEFLATED, 12, 5, Z_DEFAULT_STRATEGY) != Z_OK)
	{
		tintin_printf2(ses, "start_mccp2: failed deflateInit2");
		free(stream);

		return FALSE;
	}

	port_socket_printf(ses, buddy, "%c%c%c%c%c", IAC, SB, TELOPT_MCCP2, IAC, SE);

	/*
		The above call must send all pending output to the descriptor, since from now on we'll be compressing.
	*/

	buddy->mccp2 = stream;

	return TRUE;
}


void end_mccp2(struct session *ses, struct port_data *buddy)
{
	if (buddy->mccp2 == NULL)
	{
		return;
	}

	buddy->mccp2->next_in	= NULL;
	buddy->mccp2->avail_in	= 0;

	buddy->mccp2->next_out	= gtd->mccp_buf;
	buddy->mccp2->avail_out	= gtd->mccp_len;

	if (deflate(buddy->mccp2, Z_FINISH) != Z_STREAM_END)
	{
		tintin_printf2(ses, "end_mccp2: failed to deflate");
	}

	if (!HAS_BIT(buddy->comm_flags, COMM_FLAG_DISCONNECT))
	{
		process_mccp2(ses, buddy);
	}

	if (deflateEnd(buddy->mccp2) != Z_OK)
	{
		tintin_printf2(ses, "end_mccp2: failed to deflateEnd");
	}

	free(buddy->mccp2);

	buddy->mccp2 = NULL;

	tintin_printf2(ses, "MCCP2: COMPRESSION END");

	return;
}


void write_mccp2(struct session *ses, struct port_data *buddy, char *txt, int length)
{
	buddy->mccp2->next_in    = (unsigned char *) txt;
	buddy->mccp2->avail_in   = length;

	buddy->mccp2->next_out   = (unsigned char *) gtd->mccp_buf;
	buddy->mccp2->avail_out  = gtd->mccp_len;

	if (deflate(buddy->mccp2, Z_SYNC_FLUSH) != Z_OK)
	{
		return;
	}

	process_mccp2(ses, buddy);

	return;
}

void process_mccp2(struct session *ses, struct port_data *buddy)
{
	if (HAS_BIT(buddy->flags, PORT_FLAG_LINKLOST))
	{
		return;
	}

	if (write(buddy->fd, gtd->mccp_buf, gtd->mccp_len - buddy->mccp2->avail_out) < 1)
	{
		syserr_printf(ses, "process_mccp2: write");

		SET_BIT(buddy->comm_flags, COMM_FLAG_DISCONNECT);
	}
}

int process_do_mccp2(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	start_mccp2(ses, buddy);

	return 3;
}

int process_dont_mccp2(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	end_mccp2(ses, buddy);

	return 3;
}

// MCCP3

int process_do_mccp3(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	return 3;
}

int process_sb_mccp3(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	if (buddy->mccp3)
	{
		tintin_printf2(ses, "\e[1;31mERROR: MCCP3 ALREADY INITIALIZED");
		return 5;
	}

	buddy->mccp3 = (z_stream *) calloc(1, sizeof(z_stream));

	buddy->mccp3->data_type = Z_ASCII;
	buddy->mccp3->zalloc    = zlib_alloc;
	buddy->mccp3->zfree     = zlib_free;
	buddy->mccp3->opaque    = NULL;

	if (inflateInit(buddy->mccp3) != Z_OK)
	{
		tintin_printf2(ses, "INFO IAC SB MCCP3 FAILED TO INITIALIZE");

		port_socket_printf(ses, buddy, "%c%c%c", IAC, WONT, TELOPT_MCCP3);

		free(buddy->mccp3);
		buddy->mccp3 = NULL;
	}
	else
	{
		tintin_printf2(ses, "INFO IAC SB MCCP3 INITIALIZED");
	}
	return 5;
}

void end_mccp3(struct session *ses, struct port_data *buddy)
{
	if (buddy->mccp3)
	{
		tintin_printf2(ses, "MCCP3: COMPRESSION END");
		inflateEnd(buddy->mccp3);
		free(buddy->mccp3);
		buddy->mccp3 = NULL;
	}
}

int skip_sb(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen )
{
	int i;

	for (i = 1 ; i < srclen ; i++)
	{
		if (src[i] == SE && src[i-1] == IAC)
		{
			return i + 1;
		}
	}

	return srclen + 1;
}
