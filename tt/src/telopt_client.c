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
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"
#include "telnet.h"

extern  int  client_send_do_eor(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_mark_prompt(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_do_naws(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_sb_naws(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_sb_tspeed(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_dont_ttype(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_sb_ttype(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_wont_status(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_dont_status(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_do_sga(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_will_sga(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_wont_oldenviron(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_wont_echo(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_will_echo(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_do_echo(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_ip(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_wont_telopt(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_dont_telopt(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_will_telopt(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_do_telopt(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_sb_mssp(struct session *ses, int cplen, unsigned char *src);
extern  int  client_recv_sb_msdp(struct session *ses, int cplen, unsigned char *src);
extern  int  client_recv_sb_gmcp(struct session *ses, int cplen, unsigned char *src);
extern  int  client_recv_sb_charset(struct session *ses, int cplen, unsigned char *src);
extern  int  client_recv_sb_new_environ(struct session *ses, int cplen, unsigned char *src);
extern  int  client_recv_sb_zmp(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_will_mssp(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_will_mccp2(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_dont_mccp2(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_init_mccp2(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_will_mccp3(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_dont_mccp3(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_init_mccp3(struct session *ses);
extern void  client_end_mccp3(struct session *ses);
extern  int  client_skip_sb(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_sb(struct session *ses, int cplen, unsigned char *cpsrc);

struct iac_type
{
	int      size;
	unsigned char * code;
	int   (* func) (struct session *ses, int cplen, unsigned char *cpsrc);
};

struct iac_type iac_client_table [] =
{
	{   3,  (unsigned char []) {IAC, DO,   TELOPT_SGA},                       &client_recv_do_sga             },
	{   3,  (unsigned char []) {IAC, WILL, TELOPT_SGA},                       &client_recv_will_sga           },
	{   3,  (unsigned char []) {IAC, DO,   TELOPT_NAWS},                      &client_recv_do_naws            },
	{   3,  (unsigned char []) {IAC, DO,   TELOPT_ECHO},                      &client_recv_do_echo            },
	{   3,  (unsigned char []) {IAC, WILL, TELOPT_ECHO},                      &client_recv_will_echo          },
	{   3,  (unsigned char []) {IAC, WONT, TELOPT_ECHO},                      &client_recv_wont_echo          },
	{   3,  (unsigned char []) {IAC, WILL, TELOPT_MCCP2},                     &client_recv_will_mccp2         },
	{   3,  (unsigned char []) {IAC, WILL, TELOPT_MCCP3},                     &client_recv_will_mccp3         },
	{   3,  (unsigned char []) {IAC, DONT, TELOPT_MCCP3},                     &client_recv_dont_mccp3         },
	{   3,  (unsigned char []) {IAC, WILL, TELOPT_MSSP},                      &client_recv_will_mssp          },
	{   3,  (unsigned char []) {IAC, SB,   TELOPT_MSSP},                      &client_recv_sb_mssp            },
	{   3,  (unsigned char []) {IAC, SB,   TELOPT_MSDP},                      &client_recv_sb_msdp            },
	{   3,  (unsigned char []) {IAC, SB,   TELOPT_GMCP},                      &client_recv_sb_gmcp            },
	{   3,  (unsigned char []) {IAC, SB,   TELOPT_CHARSET},                   &client_recv_sb_charset         },
	{   3,  (unsigned char []) {IAC, SB,   TELOPT_NEW_ENVIRON},               &client_recv_sb_new_environ     },
	{   6,  (unsigned char []) {IAC, SB,   TELOPT_TSPEED, ENV_SEND, IAC, SE}, &client_recv_sb_tspeed          },
	{   3,  (unsigned char []) {IAC, DONT, TELOPT_TTYPE},                     &client_recv_dont_ttype         },
	{   6,  (unsigned char []) {IAC, SB,   TELOPT_TTYPE,  ENV_SEND, IAC, SE}, &client_recv_sb_ttype           },
	{   3,  (unsigned char []) {IAC, SB,   TELOPT_ZMP},                       &client_recv_sb_zmp             },
	{   5,  (unsigned char []) {IAC, SB,   TELOPT_MCCP1, IAC, SE},            &client_init_mccp2              },
	{   5,  (unsigned char []) {IAC, SB,   TELOPT_MCCP2, IAC, SE},            &client_init_mccp2              },
	{   2,  (unsigned char []) {IAC, EOR},                                    &client_mark_prompt             },
	{   2,  (unsigned char []) {IAC, GA},                                     &client_mark_prompt             },
	{   0,  NULL,                                                             NULL                            }
};


void client_telopt_debug(struct session *ses, char *format, ...)
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


int client_translate_telopts(struct session *ses, unsigned char *src, int cplen)
{
	int skip, cnt, retval;
	unsigned char *cpdst, *cpsrc;

	push_call("client_translate_telopts(%p,%p,%d)",ses,src,cplen);

	if (cplen == 0)
	{
		gtd->mud_output_buf[gtd->mud_output_len] = 0;	

		pop_call();
		return 0;
	}

	if (ses->mccp)
	{
		ses->mccp->next_in   = src;
		ses->mccp->avail_in  = cplen;

		ses->mccp->next_out  = gtd->mccp_buf;
		ses->mccp->avail_out = gtd->mccp_len;

		inflate:

		retval = inflate(ses->mccp, Z_SYNC_FLUSH);

		switch (retval)
		{
			case Z_BUF_ERROR:
				if (ses->mccp->avail_out == 0)
				{
					gtd->mccp_len *= 2;
					gtd->mccp_buf  = (unsigned char *) realloc(gtd->mccp_buf, gtd->mccp_len);

					ses->mccp->avail_out = gtd->mccp_len / 2;
					ses->mccp->next_out  = gtd->mccp_buf + gtd->mccp_len / 2;

					goto inflate;
				}
				else
				{
					tintin_puts2(ses, "");
					tintin_puts2(ses, "#COMPRESSION ERROR, Z_BUF_ERROR, DISABLING MCCP2.");
					client_send_dont_mccp2(ses, 0, NULL);
					inflateEnd(ses->mccp);
					free(ses->mccp);
					ses->mccp = NULL;
					cpsrc = src;
					cplen = 0;
				}
				break;

			case Z_OK:
				if (ses->mccp->avail_out == 0)
				{
					gtd->mccp_len *= 2;
					gtd->mccp_buf  = (unsigned char *) realloc(gtd->mccp_buf, gtd->mccp_len);

					ses->mccp->avail_out = gtd->mccp_len / 2;
					ses->mccp->next_out  = gtd->mccp_buf + gtd->mccp_len / 2;

					goto inflate;
				}
				cplen = ses->mccp->next_out - gtd->mccp_buf;
				cpsrc = gtd->mccp_buf;
				break;

			case Z_STREAM_END:
				client_telopt_debug(ses, "#COMPRESSION END, DISABLING MCCP2.");

				cnt = ses->mccp->next_out - gtd->mccp_buf;

				cpsrc = src + (cplen - ses->mccp->avail_in);
				cplen = ses->mccp->avail_in;

				inflateEnd(ses->mccp);
				free(ses->mccp);
				ses->mccp = NULL;

				client_translate_telopts(ses, gtd->mccp_buf, cnt);
				break;

			default:
				tintin_puts2(ses, "");
				tintin_printf2(ses, "#COMPRESSION ERROR, DISABLING MCCP2, RETVAL %d.", retval);
				client_send_dont_mccp2(ses, 0, NULL);
				inflateEnd(ses->mccp);
				free(ses->mccp);
				ses->mccp = NULL;
				cpsrc = src;
				cplen = 0;
				break;
		}
	}
	else
	{
		cpsrc = src;
	}

	if (HAS_BIT(ses->flags, SES_FLAG_LOGLEVEL) && ses->logfile)
	{
		fwrite(cpsrc, 1, cplen, ses->logfile);

		fflush(ses->logfile);
	}

 	if (ses->read_len + cplen >= ses->read_max)
	{
		ses->read_max = ses->read_len + cplen + 1000;
		ses->read_buf = (unsigned char *) realloc(ses->read_buf, ses->read_max);
	}

	memcpy(ses->read_buf + ses->read_len, cpsrc, cplen);

	cpsrc = ses->read_buf;
	cplen = ses->read_len + cplen;

	if (gtd->mud_output_len + cplen >= gtd->mud_output_max)
	{
		gtd->mud_output_max = gtd->mud_output_len + cplen + 1000;
		gtd->mud_output_buf = (char *) realloc(gtd->mud_output_buf, gtd->mud_output_max);
	}

	cpdst = (unsigned char *) gtd->mud_output_buf + gtd->mud_output_len;

	while (cplen > 0)
	{
		if (!HAS_BIT(ses->flags, SES_FLAG_RUN) && *cpsrc == IAC)
		{
			skip = 2;

			if (HAS_BIT(ses->telopts, TELOPT_FLAG_DEBUG))
			{
				switch(cpsrc[1])
				{
					case NOP:   
					case DM:
					case BREAK: 
					case IP:    
					case AO:    
					case AYT:   
					case EC:    
					case EL:
					case IAC:
					case GA:
					case EOR:
						tintin_printf2(ses, "RCVD IAC %s", TELCMD(cpsrc[1]));
						break;

					case DO:
					case DONT:
					case WILL:
					case WONT:
						if (cplen > 2)
						{
							tintin_printf2(ses, "RCVD IAC %s %s", TELCMD(cpsrc[1]), telopt_table[cpsrc[2]].name);
						}
						else
						{
							tintin_printf2(ses, "RCVD IAC %s %d (BAD TELOPT)", TELCMD(cpsrc[1]), cpsrc[2]);
						}
						break;

					case SB:
						if (cplen > 2)
						{
							tintin_printf2(ses, "RCVD IAC SB %s", telopt_table[cpsrc[2]].name);
						}
						break;

					default:
						if (TELCMD_OK(cpsrc[1]))
						{
							tintin_printf2(ses, "RCVD IAC %s %d", TELCMD(cpsrc[1]), cpsrc[2]);
						}
						else
						{
							tintin_printf2(ses, "RCVD IAC %d %d", cpsrc[1], cpsrc[2]);
						}
						break;
				}
			}

			for (cnt = 0 ; iac_client_table[cnt].code ; cnt++)
			{
				if (cplen < iac_client_table[cnt].size && !memcmp(cpsrc, iac_client_table[cnt].code, cplen))
				{
					skip = iac_client_table[cnt].size; // broken packet handling

					break;
				}

				if (cplen >= iac_client_table[cnt].size && !memcmp(cpsrc, iac_client_table[cnt].code, iac_client_table[cnt].size))
				{
					skip = iac_client_table[cnt].func(ses, cplen, cpsrc);

					if (iac_client_table[cnt].func == client_init_mccp2)
					{
						pop_call();
						return client_translate_telopts(ses, cpsrc + skip, cplen - skip);
					}
					break;
				}
			}

			if (iac_client_table[cnt].code == NULL && cplen > 1)
			{
				switch (cpsrc[1])
				{
					case SE:
					case NOP:
					case DM:
					case BREAK:
					case IP:
					case AO:
					case AYT:
					case EC:
					case EL:
					case GA:
					case EOR:
					     skip = 2;
					     break;

					case WILL:
						if (cplen > 2)
						{
							if (!check_all_events(ses, SUB_ARG|SUB_SEC, 1, 0, "IAC WILL %s", telopt_table[cpsrc[2]].name) && !check_all_events(ses, SUB_ARG|SUB_SEC, 1, 0, "CATCH IAC WILL %s", telopt_table[cpsrc[2]].name))
							{
								if (!HAS_BIT(ses->telopt_flag[cpsrc[2] / 32], 1 << cpsrc[2] % 32))
								{
									if (telopt_table[cpsrc[2]].want)
									{
										skip = client_send_do_telopt(ses, cplen, cpsrc);
									}
									else
									{
										skip = client_send_dont_telopt(ses, cplen, cpsrc);
									}
									SET_BIT(ses->telopt_flag[cpsrc[2] / 32], 1 << cpsrc[2] % 32);
								}
							}
						}
						skip = 3;
						break;

					case DO:
						if (cplen > 2)
						{
							if (!check_all_events(ses, SUB_ARG|SUB_SEC, 1, 0, "IAC DO %s", telopt_table[cpsrc[2]].name) && !check_all_events(ses, SUB_ARG|SUB_SEC, 1, 0, "IAC DO %s", telopt_table[cpsrc[2]].name))
							{
								if (!HAS_BIT(ses->telopt_flag[cpsrc[2] / 32], 1 << cpsrc[2] % 32))
								{
									if (telopt_table[cpsrc[2]].want)
									{
										skip = client_send_will_telopt(ses, cplen, cpsrc);
									}
									else
									{
										skip = client_send_wont_telopt(ses, cplen, cpsrc);
									}
									SET_BIT(ses->telopt_flag[cpsrc[2] / 32], 1 << cpsrc[2] % 32);
								}
							}
						}
						skip = 3;
						break;

					case WONT:
					case DONT:
						if (cplen > 2)
						{
							check_all_events(ses, SUB_ARG|SUB_SEC, 2, 0, "IAC %s %s", TELCMD(cpsrc[1]), telopt_table[cpsrc[2]].name);

							DEL_BIT(ses->telopt_flag[cpsrc[2] / 32], 1 << cpsrc[2] % 32);
						}
						skip = 3;
						break;

					case SB:
						skip = client_recv_sb(ses, cplen, cpsrc);
						break;

					case IAC:
						gtd->mud_output_len++;
						*cpdst++ = 0xFF;
						skip = 2;
						break;

					default:
						tintin_printf(NULL, "RCVD IAC %d (BAD TELOPT)", cpsrc[1]);
						skip = 1;
						break;
				}
			}

			if (skip <= cplen)
			{
				cplen -= skip;
				cpsrc += skip;
			}
			else
			{
				memmove(ses->read_buf, cpsrc, cplen);

				gtd->mud_output_buf[gtd->mud_output_len] = 0;

				pop_call();
				return cplen;
			}
		}
		else
		{
			/*
				skip '\0' and '\r' in text input
			*/

			switch (*cpsrc)
			{
				case '\0':
					cpsrc++;
					cplen--;
					continue;

				case 5:
					check_all_events(ses, SUB_ARG, 0, 1, "VT100 ENQ", gtd->term);
					cpsrc++;
					cplen--;
					continue;

				case '\r':
					if (cplen > 1 && cpsrc[1] == '\n')
					{
						cpsrc++;
						cplen--;
						continue;
					}
					break;

				case '\n':
					if (HAS_BIT(ses->telopts, TELOPT_FLAG_PROMPT))
					{
						DEL_BIT(ses->telopts, TELOPT_FLAG_PROMPT);
					}

					*cpdst++ = *cpsrc++;
					gtd->mud_output_len++;
					cplen--;

					while (*cpsrc == '\r')
					{
						cpsrc++;
						cplen--;
					}

					continue;

				default:
					if (cpsrc[0] == ESCAPE)
					{
						if (cplen >= 2 && cpsrc[1] == 'Z')
						{
							check_all_events(ses, SUB_ARG, 0, 0, "VT100 DECID");
							cpsrc += 2;
							cplen -= 2;
							continue;
						}

						if (cplen >= 3 && cpsrc[1] == '[')
						{
							if (cpsrc[2] == 'c')
							{
								check_all_events(ses, SUB_ARG, 0, 0, "VT100 DA");
								cpsrc += 3;
								cplen -= 3;
								continue;
							}
							if (cplen >= 4)
							{
								if (cpsrc[2] >= '5' && cpsrc[2] <= '6' && cpsrc[3] == 'n')
								{
									if (cpsrc[2] == '5')
									{
										check_all_events(ses, SUB_ARG, 0, 0, "VT100 DSR");
									}
									if (cpsrc[2] == '6')
									{
										check_all_events(ses, SUB_ARG, 0, 2, "VT100 CPR", ntos(gtd->screen->cols), ntos(gtd->screen->rows));
									}
									cpsrc += 4;
									cplen -= 4;
									continue;
								}
								if (cpsrc[2] == '0' && cpsrc[3] == 'c')
								{
									check_all_events(ses, SUB_ARG, 0, 0, "VT100 DA");
									cpsrc += 4;
									cplen -= 4;
									continue;
								}
							}
						}

						if (cplen >= 3 && cpsrc[1] == ']')
						{
							char osc[BUFFER_SIZE];

							for (skip = 2 ; cplen >= skip ; skip++)
							{
								if (cpsrc[skip] == '\a')
								{
									break;
								}
							}
							sprintf(osc, "%.*s", skip - 2, cpsrc + 2);

							check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "VT100 OSC", osc);

							if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "CATCH VT100 OSC", osc))
							{
								cpsrc += skip;
								cplen -= skip;

								continue;
							}
						}
					}

					if (HAS_BIT(ses->telopts, TELOPT_FLAG_PROMPT))
					{
						DEL_BIT(ses->telopts, TELOPT_FLAG_PROMPT);

						/*
							Fix up non vt muds
						*/

						if (HAS_BIT(ses->flags, SES_FLAG_SPLIT) || !IS_SPLIT(ses))
						{
							*cpdst++ = '\n';
							gtd->mud_output_len++;
						}
					}
					break;
			}
			*cpdst++ = *cpsrc++;
			gtd->mud_output_len++;
			cplen--;
		}
	}

	gtd->mud_output_buf[gtd->mud_output_len] = 0;

	pop_call();
	return 0;
}

/*
	SGA
*/

int client_recv_will_sga(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC WILL SGA"))
	{
		return 3;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC WILL SGA");

	SET_BIT(ses->telopts, TELOPT_FLAG_SGA);

	if (!HAS_BIT(ses->telopt_flag[TELOPT_SGA / 32], 1 << TELOPT_SGA % 32))
	{
		SET_BIT(ses->telopt_flag[TELOPT_SGA / 32], 1 << TELOPT_SGA % 32);

		telnet_printf(ses, 3, "%c%c%c", IAC, DO, TELOPT_SGA);

		client_telopt_debug(ses, "SENT IAC DO %s", telopt_table[TELOPT_SGA].name);
	}
	return 3;
}

int client_recv_do_sga(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC DO SGA"))
	{
		return 3;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC DO SGA");

	SET_BIT(ses->telopts, TELOPT_FLAG_SGA);

	if (!HAS_BIT(ses->telopt_flag[TELOPT_SGA / 32], 1 << TELOPT_SGA % 32))
	{
		SET_BIT(ses->telopt_flag[TELOPT_SGA / 32], 1 << TELOPT_SGA % 32);

		telnet_printf(ses, 3, "%c%c%c", IAC, WILL, TELOPT_SGA);

		client_telopt_debug(ses, "SENT IAC WILL %s", telopt_table[TELOPT_SGA].name);
	}
	return 3;
}

int client_mark_prompt(struct session *ses, int cplen, unsigned char *cpsrc)
{
	SET_BIT(ses->telopts, TELOPT_FLAG_PROMPT);

	if (cpsrc[1] == GA)
	{
		check_all_events(ses, SUB_ARG, 0, 0, "IAC GA");
	}
	else if (cpsrc[1] == EOR)
	{
		check_all_events(ses, SUB_ARG, 0, 0, "IAC EOR");
	}
	return 2;
}

/*
	TTYPE
*/

int client_recv_dont_ttype(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC DONT TTYPE"))
	{
		return 3;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC DONT TTYPE");

	DEL_BIT(ses->telopts, TELOPT_FLAG_TTYPE);

	DEL_BIT(ses->telopt_flag[cpsrc[2] / 32], 1 << cpsrc[2] % 32);

	return 3;
}

int client_recv_sb_ttype(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "CATCH IAC SB TTYPE", ntos(cpsrc[3])))
	{
		return 6;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "IAC SB TTYPE", ntos(cpsrc[3]));

	if (HAS_BIT(ses->telopts, TELOPT_FLAG_MTTS))
	{
		char mtts[BUFFER_SIZE];

		sprintf(mtts, "MTTS %d",
			(HAS_BIT(ses->flags, SES_FLAG_ANSICOLOR) ? 1 : 0) +
			(HAS_BIT(ses->flags, SES_FLAG_SPLIT) ? 0 : 2) +
			(HAS_BIT(ses->flags, SES_FLAG_UTF8) && !HAS_BIT(ses->flags, SES_FLAG_BIG5TOUTF8) ? 4 : 0) +
			(HAS_BIT(ses->flags, SES_FLAG_256COLOR) ? 8 : 0) +
			(HAS_BIT(ses->flags, SES_FLAG_SCREENREADER) ? 64 : 0) +
			(HAS_BIT(ses->flags, SES_FLAG_TRUECOLOR) ? 256 : 0));

		telnet_printf(ses, 6 + strlen(mtts), "%c%c%c%c%s%c%c", IAC, SB, TELOPT_TTYPE, 0, mtts, IAC, SE);

		client_telopt_debug(ses, "SENT IAC SB TTYPE %s", mtts);
	}
	else if (HAS_BIT(ses->telopts, TELOPT_FLAG_TTYPE))
	{
		telnet_printf(ses, 6 + strlen(gtd->term), "%c%c%c%c%s%c%c", IAC, SB, TELOPT_TTYPE, 0, gtd->term, IAC, SE);

		client_telopt_debug(ses, "SENT IAC SB TTYPE %s", gtd->term);

		SET_BIT(ses->telopts, TELOPT_FLAG_MTTS);
	}
	else
	{
		telnet_printf(ses, 14, "%c%c%c%c%s%c%c", IAC, SB, TELOPT_TTYPE, 0, "TINTIN++", IAC, SE);

		client_telopt_debug(ses, "SENT IAC SB TTYPE %s", "TINTIN++");

		SET_BIT(ses->telopts, TELOPT_FLAG_TTYPE);
	}
	return 6;
}


/*
	TSPEED
*/

int client_recv_sb_tspeed(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "CATCH IAC SB TSPEED", ntos(cpsrc[3])))
	{
		return 6;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "IAC SB TSPEED", ntos(cpsrc[3]));

	SET_BIT(ses->telopts, TELOPT_FLAG_TSPEED);

	telnet_printf(ses, 17, "%c%c%c%c%s%c%c", IAC, SB, TELOPT_TSPEED, 0, "38400,38400", IAC, SE);

	client_telopt_debug(ses, "SENT IAC SB 0 %s 38400,38400 IAC SB", telopt_table[TELOPT_TSPEED].name);

	return 6;
}


/*
	NAWS
*/

int client_recv_do_naws(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC DO NAWS"))
	{
		return 3;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC DO NAWS");

	SET_BIT(ses->telopts, TELOPT_FLAG_NAWS);

	if (!HAS_BIT(ses->telopt_flag[TELOPT_NAWS / 32], 1 << TELOPT_NAWS % 32))
	{
		SET_BIT(ses->telopt_flag[TELOPT_NAWS / 32], 1 << TELOPT_NAWS % 32);

		telnet_printf(ses, 3, "%c%c%c", IAC, WILL, TELOPT_NAWS);

		client_telopt_debug(ses, "SENT IAC WILL NAWS");
	}

	return client_send_sb_naws(ses, cplen, cpsrc);
}

int client_send_sb_naws(struct session *ses, int cplen, unsigned char *cpsrc)
{
	int rows;

	rows = HAS_BIT(ses->flags, SES_FLAG_SPLIT) ? ses->bot_row - ses->top_row + 1 : gtd->screen->rows;

	// Properly handle row and colum size of 255

	if (gtd->screen->cols % 256 == IAC && gtd->screen->rows % 256 == IAC)
	{
		telnet_printf(ses, 11, "%c%c%c%c%c%c%c%c%c%c%c", IAC, SB, TELOPT_NAWS, gtd->screen->cols / 256, IAC, gtd->screen->cols % 256, rows / 256, IAC, rows % 256, IAC, SE);
	}
	else if (gtd->screen->cols % 256 == IAC)
	{
		telnet_printf(ses, 10, "%c%c%c%c%c%c%c%c%c%c", IAC, SB, TELOPT_NAWS, gtd->screen->cols / 256, IAC, gtd->screen->cols % 256, rows / 256, rows % 256, IAC, SE);
	}
	else if (gtd->screen->rows % 256 == IAC)
	{
		telnet_printf(ses, 10, "%c%c%c%c%c%c%c%c%c%c", IAC, SB, TELOPT_NAWS, gtd->screen->cols / 256, gtd->screen->cols % 256, rows / 256, IAC, rows % 256, IAC, SE);
	}
	else
	{
		telnet_printf(ses, 9, "%c%c%c%c%c%c%c%c%c", IAC, SB, TELOPT_NAWS, gtd->screen->cols / 256, gtd->screen->cols % 256, rows / 256, rows % 256, IAC, SE);
	}

	client_telopt_debug(ses, "SENT IAC SB NAWS %d %d %d %d", gtd->screen->cols / 256, gtd->screen->cols % 256, gtd->screen->rows / 256, gtd->screen->rows % 256);

	return 3;
}

// Server requests client to enable local echo

int client_recv_wont_echo(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC WONT ECHO"))
	{
		return 3;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC WONT ECHO");

	SET_BIT(ses->telopts, TELOPT_FLAG_ECHO);

	if (HAS_BIT(ses->telopt_flag[TELOPT_ECHO / 32], 1 << TELOPT_ECHO % 32))
	{
		DEL_BIT(ses->telopt_flag[TELOPT_ECHO / 32], 1 << TELOPT_ECHO % 32);

//		telnet_printf(ses, 3, "%c%c%c", IAC, DONT, TELOPT_ECHO);

		client_telopt_debug(ses, "SENT IAC DONT ECHO (SKIPPED)");
	}
	else
	{
		client_telopt_debug(ses, "DID NOT SEND IAC DONT ECHO, INFINITE LOOP PROTECTION.");
	}

	return 3;
}

// Server requests client to disable local echo

int client_recv_will_echo(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC WILL ECHO"))
	{
		return 3;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC WILL ECHO");

	DEL_BIT(ses->telopts, TELOPT_FLAG_ECHO);

	if (!HAS_BIT(ses->telopt_flag[TELOPT_ECHO / 32], 1 << TELOPT_ECHO % 32))
	{
		SET_BIT(ses->telopt_flag[TELOPT_ECHO / 32], 1 << TELOPT_ECHO % 32);

		telnet_printf(ses, 3, "%c%c%c", IAC, DO, TELOPT_ECHO);

		client_telopt_debug(ses, "SENT IAC DO ECHO (SKIPPED)");
	}
	else
	{
		client_telopt_debug(ses, "DID NOT SEND IAC DO ECHO, INFINITE LOOP PROTECTION.");
	}
	return 3;
}

// Shouldn't be received, but we'll handle it as a disable local echo request

int client_recv_do_echo(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC DO ECHO"))
	{
		return 3;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC DO ECHO");

	DEL_BIT(ses->telopts, TELOPT_FLAG_ECHO);

	if (!HAS_BIT(ses->telopt_flag[TELOPT_ECHO / 32], 1 << TELOPT_ECHO % 32))
	{
		SET_BIT(ses->telopt_flag[TELOPT_ECHO / 32], 1 << TELOPT_ECHO % 32);

		telnet_printf(ses, 3, "%c%c%c", IAC, WILL, TELOPT_ECHO);

		client_telopt_debug(ses, "SENT IAC WILL ECHO");

	}
	else
	{
		client_telopt_debug(ses, "DID NOT SEND IAC WILL ECHO, INFINITE LOOP PROTECTION.");
	}

	return 3;
}

/*
	IP
*/

int client_send_ip(struct session *ses, int cplen, unsigned char *cpsrc)
{
	telnet_printf(ses, 5, "%c%c%c%c%c", IAC, IP, IAC, DO, TELOPT_TIMINGMARK);

	client_telopt_debug(ses, "SENT IAC IP");
	client_telopt_debug(ses, "SENT IAC DO TIMING MARK");

	return 3;
}

/*
	Automatic telopt handling
*/

int client_send_wont_telopt(struct session *ses, int cplen, unsigned char *cpsrc)
{
	telnet_printf(ses, 3, "%c%c%c", IAC, WONT, cpsrc[2]);

	client_telopt_debug(ses, "SENT IAC WONT %s", telopt_table[cpsrc[2]].name);

	return 3;
}

int client_send_dont_telopt(struct session *ses, int cplen, unsigned char *cpsrc)
{
	telnet_printf(ses, 3, "%c%c%c", IAC, DONT, cpsrc[2]);

	client_telopt_debug(ses, "SENT IAC DONT %s", telopt_table[cpsrc[2]].name);

	return 3;
}

int client_send_will_telopt(struct session *ses, int cplen, unsigned char *cpsrc)
{
	telnet_printf(ses, 3, "%c%c%c", IAC, WILL, cpsrc[2]);

	client_telopt_debug(ses, "SENT IAC WILL %s", telopt_table[cpsrc[2]].name);

	return 3;
}

int client_send_do_telopt(struct session *ses, int cplen, unsigned char *cpsrc)
{
	telnet_printf(ses, 3, "%c%c%c", IAC, DO, cpsrc[2]);

	client_telopt_debug(ses, "SENT IAC DO %s", telopt_table[cpsrc[2]].name);

	return 3;
}

/*
	MSSP (Mud Server Status Protocol)
*/

int client_recv_will_mssp(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (!check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC WILL MSSP") && !check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC WILL MSSP"))
	{
		if (HAS_BIT(ses->telopts, TELOPT_FLAG_DEBUG))
		{
			telnet_printf(ses, 3, "%c%c%c", IAC, DO, TELOPT_MSSP);

			client_telopt_debug(ses, "SENT IAC DO MSSP");
		}
	}
	return 3;
}

int client_recv_sb_mssp(struct session *ses, int cplen, unsigned char *src)
{
	char var[BUFFER_SIZE], val[BUFFER_SIZE];
	char *pto;
	int i;

	var[0] = val[0] = i = 0;

	if (client_skip_sb(ses, cplen, src) > cplen)
	{
		return cplen + 1;
	}

	while (i < cplen && src[i] != SE)
	{
		switch (src[i])
		{
			case MSSP_VAR:
				i++;
				pto = var;

				while (i < cplen && src[i] >= 3 && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;
				break;

			case MSSP_VAL:
				i++;
				pto = val;

				while (i < cplen && src[i] >= 3 && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;

				client_telopt_debug(ses, "RCVD IAC SB MSSP VAR %-20s VAL %s", var, val);

				check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "IAC SB MSSP", var, val);
				check_all_events(ses, SUB_ARG|SUB_SEC, 1, 2, "IAC SB MSSP %s", var, var, val);
				break;

			default:
				i++;
				break;
		}
	}

	client_telopt_debug(ses, "RCVD IAC SB MSSP IAC SE");

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC SB MSSP IAC SE");

	return UMIN(i + 1, cplen);
}

/*
	MSDP (Mud Server Data Protocol)
*/

int client_recv_sb_msdp(struct session *ses, int cplen, unsigned char *src)
{
	char var[BUFFER_SIZE], val[BUFFER_SIZE], *pto;
	int i, nest, state[100], last;

	var[0] = val[0] = state[0] = nest = last = 0;

	if (client_skip_sb(ses, cplen, src) > cplen)
	{
		return cplen + 1;
	}

	i = 3;
	pto = var;

	while (i < cplen && nest < 99)
	{
		if (src[i] == IAC && src[i+1] == SE)
		{
			break;
		}

		switch (src[i])
		{
			case MSDP_TABLE_OPEN:
				nest++;
				state[nest] = 0;
				last = MSDP_TABLE_OPEN;
				break;

			case MSDP_TABLE_CLOSE:
				if (nest)
				{
					if (last == MSDP_VAL || last == MSDP_VAR)
					{
						*pto++ = '}';
					}
					nest--;
				}
				if (nest)
				{
					*pto++ = '}';
				}
				last = MSDP_TABLE_CLOSE;
				break;

			case MSDP_ARRAY_OPEN:
				nest++;
				state[nest] = 1;
				last = MSDP_ARRAY_OPEN;
				break;

			case MSDP_ARRAY_CLOSE:
				if (nest)
				{
					if (last == MSDP_VAL)
					{
						*pto++ = '}';
					}
					nest--;
				}
				if (nest)
				{
					*pto++ = '}';
				}
				last = MSDP_ARRAY_CLOSE;
				break;

			case MSDP_VAR:
				if (nest)
				{
					if (last == MSDP_VAL)
					{
						*pto++ = '}';
					}
					*pto++ = '{';
				}
				else
				{
					*pto = 0;

					if (last)
					{
						client_telopt_debug(ses, "RCVD IAC SB MSDP VAR %-20s VAL %s", var, val);
						check_all_events(ses, SUB_ARG, 1, 2, "IAC SB MSDP %s", var, var, val);
						check_all_events(ses, SUB_ARG, 0, 2, "IAC SB MSDP", var, val);
					}
					pto = var;
				}
				last = MSDP_VAR;
				break;

			case MSDP_VAL:
				if (nest)
				{
					if (last == MSDP_VAR || last == MSDP_VAL)
					{
						*pto++ = '}';
					}
					if (state[nest])
					{
						pto += sprintf(pto, "{%d}", state[nest]++);
					}
					*pto++ = '{';
				}
				else
				{
					*pto = 0;

					if (last != MSDP_VAR)
					{
						client_telopt_debug(ses, "RCVD IAC SB MSDP VAR %-20s VAL %s", var, val);
						check_all_events(ses, SUB_ARG, 1, 2, "IAC SB MSDP %s", var, var, val);
						check_all_events(ses, SUB_ARG, 0, 2, "IAC SB MSDP", var, val);
					}
					pto = val;
				}
				last = MSDP_VAL;
				break;

			case '\\':
				*pto++ = '\\';
				*pto++ = '\\';
				break;

			case '{':
				*pto++ = '\\';
				*pto++ = 'x';
				*pto++ = '7';
				*pto++ = 'B';
				break;

			case '}':
				*pto++ = '\\';
				*pto++ = 'x';
				*pto++ = '7';
				*pto++ = 'D';
				break;

			case COMMAND_SEPARATOR:
				*pto++ = '\\';
				*pto++ = COMMAND_SEPARATOR;
				break;

			default:
				*pto++ = src[i];
				break;
		}
		i++;
	}

	if (src[i] == IAC && nest < 99)
	{
		*pto = 0;

		if (last)
		{
			client_telopt_debug(ses, "RCVD IAC SB MSDP VAR %-20s VAL %s", var, val);
			check_all_events(ses, SUB_ARG, 1, 2, "IAC SB MSDP %s", var, var, val);
			check_all_events(ses, SUB_ARG, 0, 2, "IAC SB MSDP", var, val);
		}
		i++;
	}

	var[0] = val[0] = last = 0;

	i = 3;
	pto = var;

	while (i < cplen && nest < 99)
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
				state[nest] = 0;
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
				state[nest] = 1;
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
				if (nest)
				{
					if (last == MSDP_VAL || last == MSDP_VAR)
					{
						*pto++ = '"';
					}
					if (last == MSDP_VAL || last == MSDP_VAR || last == MSDP_TABLE_CLOSE || last == MSDP_ARRAY_CLOSE)
					{
						*pto++ = ',';
					}
					*pto++ = '"';
				}
				else
				{
					*pto = 0;

					if (last)
					{
						check_all_events(ses, SUB_ARG, 1, 2, "IAC SB MSDP2JSON %s", var, var, val);
						check_all_events(ses, SUB_ARG, 0, 2, "IAC SB MSDP2JSON", var, val);
					}
					pto = var;
				}
				last = MSDP_VAR;
				break;

			case MSDP_VAL:
				if (nest)
				{
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
				}
				else
				{
					*pto = 0;

					if (last != MSDP_VAR)
					{
						check_all_events(ses, SUB_ARG, 1, 2, "IAC SB MSDP2JSON %s", var, var, val);
						check_all_events(ses, SUB_ARG, 0, 2, "IAC SB MSDP2JSON", var, val);
					}
					pto = val;
				}
				last = MSDP_VAL;
				break;

			case '\\':
				*pto++ = '\\';
				*pto++ = '\\';
				break;

			case '{':
				*pto++ = '\\';
				*pto++ = 'x';
				*pto++ = '7';
				*pto++ = 'B';
				break;

			case '}':
				*pto++ = '\\';
				*pto++ = 'x';
				*pto++ = '7';
				*pto++ = 'D';
				break;

			case COMMAND_SEPARATOR:
				*pto++ = '\\';
				*pto++ = COMMAND_SEPARATOR;
				break;

			default:
				*pto++ = src[i];
				break;
		}
		i++;
	}

	if (src[i] == IAC && nest < 99)
	{
		*pto = 0;

		if (last)
		{
			check_all_events(ses, SUB_ARG, 1, 2, "IAC SB MSDP2JSON %s", var, var, val);
			check_all_events(ses, SUB_ARG, 0, 2, "IAC SB MSDP2JSON", var, val);
		}
		i++;
	}

	client_telopt_debug(ses, "RCVD IAC SB MSDP IAC SE");

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC SB MSDP IAC SE");

	return UMIN(i + 1, cplen);
}


/*
	CHARSET
*/

int client_recv_sb_charset(struct session *ses, int cplen, unsigned char *src)
{
	char buf[BUFFER_SIZE], var[BUFFER_SIZE];
	char *pto;
	int i;

	if (client_skip_sb(ses, cplen, src) > cplen)
	{
		return cplen + 1;
	}

	client_telopt_debug(ses, "IAC SB CHARSET %d %d", src[3], src[4]);

	i = 5;

	while (i < cplen && src[i] != SE)
	{
		pto = buf;

		while (i < cplen && src[i] != src[4] && src[i] != IAC)
		{
			*pto++ = src[i++];
		}
		*pto = 0;

		substitute(ses, buf, var, SUB_SEC);

		switch (src[3])
		{
			case CHARSET_REQUEST:
				strcpy(buf, "REQUEST");
				break;
			case CHARSET_ACCEPTED:
				strcpy(buf, "ACCEPTED");
				break;
			case CHARSET_REJECTED:
				strcpy(buf, "REJECTED");
				break;
			default:
				sprintf(buf, "%d", src[4]);
				break;
		}

		client_telopt_debug(ses, "IAC SB CHARSET %s %s", buf, var);

		check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "IAC SB CHARSET", buf, var);
		check_all_events(ses, SUB_ARG|SUB_SEC, 2, 2, "IAC SB CHARSET %s %s", buf, var, buf, var);

		if (!check_all_events(ses, SUB_ARG|SUB_SEC, 2, 2, "CATCH IAC SB CHARSET %s %s", buf, var, buf, var))
		{
			if (!strcmp(buf, "REQUEST"))
			{
				if (!strcasecmp(var, "UTF-8"))
				{
					if (HAS_BIT(ses->flags, SES_FLAG_UTF8) && !HAS_BIT(ses->flags, SES_FLAG_BIG5TOUTF8) && !HAS_BIT(ses->flags, SES_FLAG_FANSITOUTF8))
					{
						telnet_printf(ses, 12, "%c%c%c%c UTF-8%c%c", IAC, SB, TELOPT_CHARSET, CHARSET_ACCEPTED, IAC, SE);

						client_telopt_debug(ses, "SENT IAC SB CHARSET ACCEPTED UTF-8");
					}
					else
					{
						telnet_printf(ses, 12, "%c%c%c%c UTF-8%c%c", IAC, SB, TELOPT_CHARSET, CHARSET_REJECTED, IAC, SE);

						client_telopt_debug(ses, "SENT IAC SB CHARSET REJECTED UTF-8");
					}
				}
				else if (!strcasecmp(var, "BIG-5"))
				{
					if (HAS_BIT(ses->flags, SES_FLAG_BIG5) || HAS_BIT(ses->flags, SES_FLAG_BIG5TOUTF8))
					{
						telnet_printf(ses, 11, "%c%c%c%c BIG-5%c%c", IAC, SB, TELOPT_CHARSET, CHARSET_ACCEPTED, IAC, SE);

						client_telopt_debug(ses, "SENT IAC SB CHARSET ACCEPTED BIG-5");
					}
					else
					{
						telnet_printf(ses, 11, "%c%c%c%c BIG-5%c%c", IAC, SB, TELOPT_CHARSET, CHARSET_REJECTED, IAC, SE);

						client_telopt_debug(ses, "SENT IAC SB CHARSET REJECTED BIG-5");
					}
				}
				else if (!strcasecmp(var, "FANSI"))
				{
					if (!check_all_events(ses, SUB_ARG|SUB_SEC, 2, 2, "CATCH IAC SB CHARSET %s %s", buf, var, buf, var))
					{
						if (HAS_BIT(ses->flags, SES_FLAG_FANSITOUTF8))
						{
							telnet_printf(ses, 11, "%c%c%c%c FANSI%c%c", IAC, SB, TELOPT_CHARSET, CHARSET_ACCEPTED, IAC, SE);

							client_telopt_debug(ses, "SENT IAC SB CHARSET ACCEPTED FANSI");
						}
						else
						{
							telnet_printf(ses, 11, "%c%c%c%c FANSI%c%c", IAC, SB, TELOPT_CHARSET, CHARSET_REJECTED, IAC, SE);

							client_telopt_debug(ses, "SENT IAC SB CHARSET REJECTED FANSI");
						}
					}
				}
			}
		}
		i++;
	}

	client_telopt_debug(ses, "IAC SB CHARSET IAC SE");

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC SB CHARSET IAC SE");

	return i + 1;
}

/*
	NEW-ENVIRON
*/

int client_recv_sb_new_environ(struct session *ses, int cplen, unsigned char *src)
{
	char buf[BUFFER_SIZE], var[BUFFER_SIZE], val[BUFFER_SIZE], sub1[NUMBER_SIZE], sub2[NUMBER_SIZE];
	char *pto;
	int i;

	var[0] = val[0] = 0;

	if (client_skip_sb(ses, cplen, src) > cplen)
	{
		return cplen + 1;
	}

	client_telopt_debug(ses, "IAC SB NEW-ENVIRON %d %d", src[3], src[4]);

	switch (src[3])
	{
		case ENV_IS:
			strcpy(sub1, "IS");
			break;
		case ENV_SEND:
			strcpy(sub1, "SEND");
			break;
		case ENV_INFO:
			strcpy(sub1, "INFO");
			break;
		default:
			strcpy(sub1, "UNKNOWN");
			break;
	}

	i = 4;

	while (i < cplen && src[i] != SE)
	{
		switch (src[i])
		{
			case ENV_VAR:
				strcpy(sub2, "VAR");
				break;
			case ENV_VAL:
				strcpy(sub2, "VAL");
				break;
			case ENV_USR:
				strcpy(sub2, "USERVAR");
				break;
			default:
				strcpy(sub2, "UNKNOWN");
				break;
		}

		switch (src[i])
		{
			case ENV_VAR:
			case ENV_USR:
				i++;
				pto = buf;

				while (i < cplen && src[i] >= 4 && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;

				substitute(ses, buf, var, SUB_SEC);

				if (src[3] == ENV_SEND)
				{
					client_telopt_debug(ses, "IAC SB NEW-ENVIRON SEND %s", sub2);

					check_all_events(ses, SUB_ARG|SUB_SEC, 0, 4, "IAC SB NEW-ENVIRON", sub1, sub2, var, "");
					check_all_events(ses, SUB_ARG|SUB_SEC, 1, 4, "IAC SB NEW-ENVIRON SEND %s", var, sub1, sub2, var, "");
				}
				break;

			case ENV_VAL:
				i++;
				pto = buf;

				while (i < cplen && src[i] >= 4 && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;

				substitute(ses, buf, val, SUB_SEC);

				client_telopt_debug(ses, "IAC SB NEW-ENVIRON %s %s", sub1, sub2);

				check_all_events(ses, SUB_ARG|SUB_SEC, 0, 4, "IAC SB NEW-ENVIRON", sub1, sub2, var, val);
				check_all_events(ses, SUB_ARG|SUB_SEC, 2, 4, "IAC SB NEW-ENVIRON %s %s", sub1, sub2, sub1, sub2, var, val);
				break;

			case IAC:
				client_telopt_debug(ses, "IAC SB NEW-ENVIRON (ERROR) %s %s (%s) (%s)", sub1, sub2, var, val);
				i++;
				break;

			default:
				client_telopt_debug(ses, "IAC SB NEW-ENVIRON (ERROR) %03d %c", src[i], src[i]);
				i++;
				break;
		}
	}

	client_telopt_debug(ses, "IAC SB NEW-ENVIRON IAC SE");

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC SB NEW-ENVIRON IAC SE");

	return i + 1;
}

int client_recv_sb_zmp(struct session *ses, int cplen, unsigned char *src)
{
	char buf[BUFFER_SIZE], var[BUFFER_SIZE], val[BUFFER_SIZE];
	char *pto;
	int i, x;

	var[0] = val[0] = x = 0;

	if (client_skip_sb(ses, cplen, src) > cplen)
	{
		return cplen + 1;
	}

	i = 3;

	while (i < cplen && src[i] != SE)
	{
		switch (src[i])
		{
			case IAC:
				i++;
				break;

			default:
				pto = buf;

				while (i < cplen && src[i])
				{
					*pto++ = src[i++];
				}
				*pto = src[i++];

				substitute(ses, buf, x ? val : var, SUB_SEC);

				if (x++)
				{
					client_telopt_debug(ses, "IAC SB ZMP %s", var);

					check_all_events(ses, SUB_ARG|SUB_SEC, 1, 1, "IAC SB ZMP %s", var, val);
				}
				break;
		}
	}

	client_telopt_debug(ses, "IAC SB ZMP %s IAC SE", var);

	check_all_events(ses, SUB_ARG|SUB_SEC, 1, 0, "IAC SB ZMP %s IAC SE", var);

	return UMIN(i + 1, cplen);
}

int client_recv_sb_gmcp(struct session *ses, int cplen, unsigned char *src)
{
	char mod[BUFFER_SIZE], val[BUFFER_SIZE], json[BUFFER_SIZE], *pto;
	int i, state[100], nest, type;

	push_call("client_recv_sb_gmcp(%p,%d,%p)",ses,cplen,src);

	if (client_skip_sb(ses, cplen, src) > cplen)
	{
		pop_call();
		return cplen + 1;
	}

	mod[0] = val[0] = state[0] = nest = type = 0;

	i = 3;

	pto = mod;

	// space out

	while (i < cplen && src[i] == ' ')
	{
		i++;
	}

	// grab module

	while (i < cplen && src[i] != IAC)
	{
		if (src[i] == ' ' || src[i] == '{' || src[i] == '[')
		{
			break;
		}
		*pto++ = src[i++];
	}

	*pto = 0;

	// parse JSON content

	pto = val;

	while (i < cplen && src[i] != IAC && nest < 99)
	{
		switch (src[i])
		{
			case ' ':
				i++;
				break;

			case '{':
				if (nest != 0)
				{
					*pto++ = '{';
				}
				i++;
				state[++nest] = 0;
				break;

			case '}':
				nest--;
				i++;
				if (nest != 0)
				{
					*pto++ = '}';
				}
				break;

			case '[':
				if (nest != 0)
				{
					*pto++ = '{';
				}
				i++;
				state[++nest] = 1;
				pto += sprintf(pto, "{%d}", state[nest]);
				break;

			case ']':
				nest--;
				i++;
				if (nest != 0)
				{
					*pto++ = '}';
				}
				break;

			case ':':
				i++;
				break;

			case ',':
				i++;
				if (state[nest])
				{
					pto += sprintf(pto, "{%d}", ++state[nest]);
				}
				break;

			case '"':
				i++;
				if (nest)
				{
					*pto++ = '{';
				}
				type = 1;

				while (i < cplen && src[i] != IAC && type == 1)
				{
					switch (src[i])
					{
						case '\\':
							i++;

							if (i < cplen && src[i] == '"')
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

						case '{':
							i++;
							*pto++ = '\\';
							*pto++ = 'x';
							*pto++ = '7';
							*pto++ = 'B';
							break;

						case '}':
							i++;
							*pto++ = '\\';
							*pto++ = 'x';
							*pto++ = '7';
							*pto++ = 'D';
							break;

						case COMMAND_SEPARATOR:
							i++;
							*pto++ = '\\';
							*pto++ = COMMAND_SEPARATOR;
							break;

						default:
							*pto++ = src[i++];
							break;
					}
				}

				if (nest)
				{
					*pto++ = '}';
				}
				break;

			default:
				if (nest)
				{
					*pto++ = '{';
				}

				type = 1;

				while (i < cplen && src[i] != IAC && type == 1)
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

				if (nest)
				{
					*pto++ = '}';
				}
				break;
		}
	}
	*pto = 0;

	// Raw json data for debugging purposes.

	pto = json;
	i = 3;

	while (i < cplen && src[i] != IAC)
	{
		switch (src[i])
		{
			case '\\':
				i++;
				*pto++ = '\\';
				*pto++ = '\\';
				break;

			case '{':
				i++;
				*pto++ = '\\';
				*pto++ = 'x';
				*pto++ = '7';
				*pto++ = 'B';
				break;

			case '}':
				i++;
				*pto++ = '\\';
				*pto++ = 'x';
				*pto++ = '7';
				*pto++ = 'D';
				break;

			case COMMAND_SEPARATOR:
				i++;
				*pto++ = '\\';
				*pto++ = COMMAND_SEPARATOR;
				break;

			default:
				*pto++ = src[i++];
				break;
		}
	}
	*pto = 0;

	while (i < cplen && src[i] != SE)
	{
		i++;
	}

	client_telopt_debug(ses, "IAC SB GMCP %s IAC SE", mod);

	check_all_events(ses, SUB_ARG, 1, 2, "IAC SB GMCP %s IAC SE", mod, val, json);

	pop_call();
	return UMIN(i + 1, cplen);
}

/*
	MCCP2
*/

int client_recv_will_mccp2(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC WILL MCCP2"))
	{
		return 3;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC WILL MCCP2");

	if (HAS_BIT(ses->flags, SES_FLAG_MCCP))
	{
		telnet_printf(ses, 3, "%c%c%c", IAC, DO, TELOPT_MCCP2);

		client_telopt_debug(ses, "SENT IAC DO MCCP2");
	}
	else
	{
		telnet_printf(ses, 3, "%c%c%c", IAC, DONT, TELOPT_MCCP2);

		client_telopt_debug(ses, "SENT IAC DONT MCCP2 (#CONFIG MCCP HAS BEEN DISABLED)");
	}
	return 3;
}

int client_send_dont_mccp2(struct session *ses, int cplen, unsigned char *cpsrc)
{
	telnet_printf(ses, 3, "%c%c%c", IAC, DONT, TELOPT_MCCP2);

	client_telopt_debug(ses, "SENT DONT MCCP2");

	return 3;
}


int client_init_mccp2(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (ses->mccp)
	{
		return 5;
	}

	ses->mccp = (z_stream *) calloc(1, sizeof(z_stream));

	ses->mccp->data_type = Z_ASCII;
	ses->mccp->zalloc    = zlib_alloc;
	ses->mccp->zfree     = zlib_free;
	ses->mccp->opaque    = NULL;

	if (inflateInit(ses->mccp) != Z_OK)
	{
		tintin_puts2(ses, "MCCP2: FAILED TO INITIALIZE");
		client_send_dont_mccp2(ses, 0, NULL);
		free(ses->mccp);
		ses->mccp = NULL;
	}
	else
	{
		client_telopt_debug(ses, "MCCP2: INITIALIZED");
	}
	return 5;
}



// MCCP3

int client_recv_will_mccp3(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC WILL MCCP3"))
	{
		return 3;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC WILL MCCP3");

	if (HAS_BIT(ses->flags, SES_FLAG_MCCP))
	{
		telnet_printf(ses, 3, "%c%c%c", IAC, DO, TELOPT_MCCP3);

		client_telopt_debug(ses, "SENT IAC DO MCCP3");

		client_init_mccp3(ses);

	}
	else
	{
		telnet_printf(ses, 3, "%c%c%c", IAC, DONT, TELOPT_MCCP3);

		client_telopt_debug(ses, "SENT IAC DONT MCCP3 (#CONFIG MCCP HAS BEEN DISABLED)");
	}
	return 3;
}

int client_recv_dont_mccp3(struct session *ses, int cplen, unsigned char *cpsrc)
{
	if (check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH IAC DONT MCCP3"))
	{
	 	return 3;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "IAC DONT MCCP3");

	if (ses->mccp3)
	{
		client_end_mccp3(ses);
	}
	return 3;
}

int client_init_mccp3(struct session *ses)
{
	z_stream *stream;

	if (ses->mccp3)
	{
		client_telopt_debug(ses, "MCCP3: ALREADY INITIALIZED");

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

	if (deflateInit(stream, Z_BEST_COMPRESSION) != Z_OK)
	{
		client_telopt_debug(ses, "MCCP3: FAILED TO INITIALIZE");
		free(stream);

		return FALSE;
	}

	telnet_printf(ses, 5, "%c%c%c%c%c", IAC, SB, TELOPT_MCCP3, IAC, SE);

	client_telopt_debug(ses, "SENT IAC SB MCCP3 IAC SE");

	client_telopt_debug(ses, "MCCP3: INITIALIZED");

	ses->mccp3 = stream;

	return TRUE;
}

void client_end_mccp3(struct session *ses)
{
	if (ses->mccp3 == NULL)
	{
		return;
	}

	ses->mccp3->next_in	= NULL;
	ses->mccp3->avail_in	= 0;

	ses->mccp3->next_out	= gtd->mccp_buf;
	ses->mccp3->avail_out	= gtd->mccp_len;

	if (deflate(ses->mccp3, Z_FINISH) != Z_STREAM_END)
	{
		tintin_printf2(ses, "MCCP3: FAILED TO DEFLATE");
	}

//	process_compressed(d);

	if (deflateEnd(ses->mccp3) != Z_OK)
	{
		tintin_printf2(ses, "MCCP3: FAILED TO DEFLATE_END");
	}

	free(ses->mccp3);

	ses->mccp3 = NULL;

	client_telopt_debug(ses, "MCCP3: COMPRESSION END, DISABLING MCCP3");

	return;
}

int client_write_compressed(struct session *ses, char *txt, int length)
{
	int result;

	ses->mccp3->next_in    = (unsigned char *) txt;
	ses->mccp3->avail_in   = length;

	ses->mccp3->next_out   = gtd->mccp_buf;
	ses->mccp3->avail_out  = gtd->mccp_len;

	if (deflate(ses->mccp3, Z_SYNC_FLUSH) != Z_OK)
	{
		syserr_printf(ses, "client_write_compressed: deflate");

		return 0;
	}

#ifdef HAVE_GNUTLS_H

	if (ses->ssl)
	{
		result = gnutls_record_send(ses->ssl, gtd->mccp_buf, gtd->mccp_len - ses->mccp3->avail_out);

		while (result == GNUTLS_E_INTERRUPTED || result == GNUTLS_E_AGAIN)
		{
			result = gnutls_record_send(ses->ssl, 0, 0);
		}
		return result;
	}
	else
#endif

	result = write(ses->socket, gtd->mccp_buf, gtd->mccp_len - ses->mccp3->avail_out);

	if (result < 1)
	{
		syserr_printf(ses, "client_write_compressed: write");

		return -1;
	}

	return result;
}

/*
	Returns the length of a telnet subnegotiation
*/

int client_skip_sb(struct session *ses, int cplen, unsigned char *cpsrc)
{
	int i;

	for (i = 1 ; i < cplen ; i++)
	{
		if (cpsrc[i] == SE && cpsrc[i-1] == IAC)
		{
			return i + 1;
		}
	}

	client_telopt_debug(ses, "SKIP SB (%d)", cplen);

	return cplen + 1;
}

int client_recv_sb(struct session *ses, int cplen, unsigned char *cpsrc)
{
	char *pt1, *pt2, var1[BUFFER_SIZE], var2[BUFFER_SIZE];
	int i;

	if (client_skip_sb(ses, cplen, cpsrc) > cplen)
	{
		return cplen + 1;
	}

	pt1 = var1;
	pt2 = var2;

	for (i = 3 ; i < cplen ; i++)
	{
		if (cpsrc[i] == IAC && i + 1 < cplen && cpsrc[i+1] == SE)
		{
			break;
		}
		else
		{
			*pt1++ = cpsrc[i];

			sprintf(pt2, "%03d ", cpsrc[i]);

			pt2 += 4;
		}
	}

	*pt1 = 0;
	*pt2 = 0;

	check_all_events(ses, SUB_ARG|SUB_SEC, 1, 2, "IAC SB %s", telopt_table[cpsrc[2]].name, var1, var2);

	return i + 2;
}
