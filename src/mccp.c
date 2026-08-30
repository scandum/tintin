/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright 2004-2026 Igor van den Hoven                                    *
*                                                                             *
*   SPDX-License-Identifier: LGPL-2.1-or-later                                *
******************************************************************************/

/******************************************************************************
*                               T I N T I N + +                               *
*                                                                             *
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"
#include "telnet.h"

/*
	client MCCP 2 + 3 + 4
*/

extern  int  client_recv_will_mccp2(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_send_dont_mccp2(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_init_mccp2(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_will_mccp3(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_dont_mccp3(struct session *ses, int cplen, unsigned char *cpsrc);
extern  int  client_recv_wont_mccp3(struct session *ses, int cplen, unsigned char *cpsrc);

extern  int  client_init_mccp3(struct session *ses);

void *zlib_alloc( void *opaque, unsigned int items, unsigned int size )
{
	return calloc(items, size);
}


void zlib_free( void *opaque, void *address ) 
{
	free(address);
}

/*
	MCCP2
*/

int client_recv_will_mccp2(struct session *ses, int cplen, unsigned char *cpsrc)
{
	check_all_events(ses, EVENT_FLAG_TELNET, 0, 0, "IAC WILL MCCP2");

	if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH IAC WILL MCCP2"))
	{
		return 3;
	}

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_MCCP))
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
	if (ses->mccp2)
	{
		client_telopt_debug(ses, "INFO MCCP2 ALREADY INITIALIZED");
		return 5;
	}

	ses->mccp2 = (z_stream *) calloc(1, sizeof(z_stream));

	ses->mccp2->data_type = Z_ASCII;
	ses->mccp2->zalloc    = zlib_alloc;
	ses->mccp2->zfree     = zlib_free;
	ses->mccp2->opaque    = NULL;

	if (inflateInit(ses->mccp2) != Z_OK)
	{
		tintin_puts2(ses, "MCCP2: FAILED TO INITIALIZE");
		client_send_dont_mccp2(ses, 0, NULL);
		free(ses->mccp2);
		ses->mccp2 = NULL;
	}
	else
	{
		client_telopt_debug(ses, "INFO MCCP2 INITIALIZED");
	}
	return 5;
}

void client_end_mccp2(struct session *ses)
{
	if (ses->mccp2 == NULL)
	{
		return;
	}
/*
	ses->mccp2->next_in     = NULL;
	ses->mccp2->avail_in    = 0;

	ses->mccp2->next_out    = gtd->mccp_buf;
	ses->mccp2->avail_out   = gtd->mccp_len;

	if (deflate(ses->mccp2, Z_FINISH) != Z_STREAM_END)
	{
		tintin_printf2(ses, "MCCP2: FAILED TO DEFLATE");
	}
*/
	if (inflateEnd(ses->mccp2) == Z_STREAM_ERROR)
	{
		client_telopt_debug(ses, "MCCP2: inflateEnd failed:");
	}

	free(ses->mccp2);

	ses->mccp2 = NULL;

	client_telopt_debug(ses, "MCCP2: COMPRESSION END, DISABLING MCCP2");

	return;
}


// MCCP3

int client_recv_will_mccp3(struct session *ses, int cplen, unsigned char *cpsrc)
{
	check_all_events(ses, EVENT_FLAG_TELNET, 0, 0, "IAC WILL MCCP3");

	if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH IAC WILL MCCP3"))
	{
		return 3;
	}

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_MCCP))
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
	check_all_events(ses, EVENT_FLAG_TELNET, 0, 0, "IAC DONT MCCP3");

	if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH IAC DONT MCCP3"))
	{
	 	return 3;
	}

	if (ses->mccp3)
	{
		client_end_mccp3(ses);
	}
	return 3;
}

int client_recv_wont_mccp3(struct session *ses, int cplen, unsigned char *cpsrc)
{
	check_all_events(ses, EVENT_FLAG_TELNET, 0, 0, "IAC WONT MCCP3");

	if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH IAC WONT MCCP3"))
	{
	 	return 3;
	}

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
		client_telopt_debug(ses, "INFO MCCP3 ALREADY INITIALIZED");

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

//	if (deflateInit2(stream, Z_BEST_COMPRESSION, Z_DEFLATED, 11, 5, Z_DEFAULT_STRATEGY) != Z_OK)
	if (deflateInit(stream, Z_BEST_COMPRESSION) != Z_OK)
	{
		client_telopt_debug(ses, "INFO MCCP3 FAILED TO INITIALIZE");

		free(stream);

		return FALSE;
	}

	telnet_printf(ses, 5, "%c%c%c%c%c", IAC, SB, TELOPT_MCCP3, IAC, SE);

	client_telopt_debug(ses, "SENT IAC SB MCCP3 IAC SE");

	client_telopt_debug(ses, "INFO MCCP3 INITIALIZED");

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

// MCCP4

int client_recv_will_mccp4(struct session *ses, int cplen, unsigned char *cpsrc)
{

	check_all_events(ses, EVENT_FLAG_TELNET, 0, 0, "IAC WILL TELOPT_MCCP4");

	if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH IAC WILL MCCP4"))
	{
		return 3;
	}
#ifdef HAVE_ZSTD_H
	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_MCCP))
	{
		telnet_printf(ses, 3, "%c%c%c", IAC, DO, TELOPT_MCCP4);

		client_telopt_debug(ses, "SENT IAC DO MCCP4");

		telnet_printf(ses, 10, "%c%c%c%c%s%c%c", IAC, SB, TELOPT_MCCP4, MCCP4_ACCEPT_ENCODING, "zstd", IAC, SE);
	}
	else
	{
		telnet_printf(ses, 3, "%c%c%c", IAC, DONT, TELOPT_MCCP4);

		client_telopt_debug(ses, "SENT IAC DONT MCCP4 (#CONFIG MCCP HAS BEEN DISABLED)");
	}
#endif
	return 3;
}

int client_send_dont_mccp4(struct session *ses, int cplen, unsigned char *cpsrc)
{
	telnet_printf(ses, 3, "%c%c%c", IAC, DONT, TELOPT_MCCP4);

	client_telopt_debug(ses, "SENT DONT MCCP4");

	return 3;
}

int client_init_mccp4(struct session *ses, int cplen, unsigned char *cpsrc)
{
#ifdef HAVE_ZSTD_H
	if (ses->mccp4)
	{
		client_telopt_debug(ses, "INFO MCCP4 ALREADY INITIALIZED");
		return 10;
	}

	ses->mccp4 = ZSTD_createDStream();

	if (ses->mccp4 == NULL)
	{
		tintin_puts2(ses, "MCCP4: FAILED TO ALLOCATE STREAM");
		return 10;
	}

	if (ZSTD_isError(ZSTD_initDStream(ses->mccp4)))
	{
		tintin_puts2(ses, "MCCP4: FAILED TO INITIALIZE");
		client_send_dont_mccp4(ses, 0, NULL);
		ZSTD_freeDStream(ses->mccp4);
		ses->mccp4 = NULL;
	}
	else
	{
		client_telopt_debug(ses, "INFO MCCP4 INITIALIZED");
	}
#endif
	return 10;
}

void client_end_mccp4(struct session *ses)
{
#ifdef HAVE_ZSTD_H
	if (ses->mccp4 == NULL)
	{
		return;
	}

	if (ZSTD_isError(ZSTD_freeDStream(ses->mccp4)))
	{
		client_telopt_debug(ses, "MCCP4: ZSTD_freeDStream failed");
	}

	ses->mccp4 = NULL;

	client_telopt_debug(ses, "MCCP4: COMPRESSION END, DISABLING MCCP4");
#endif
	return;
}

// MCCP 3+5

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

	if (result == -1)
	{
		syserr_printf(ses, "client_write_compressed: write");

		return -1;
	}

	return result;
}
