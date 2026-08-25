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

