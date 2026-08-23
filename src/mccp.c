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
	client MCCP 2 + 3
*/


void *zlib_alloc( void *opaque, unsigned int items, unsigned int size )
{
	return calloc(items, size);
}


void zlib_free( void *opaque, void *address ) 
{
	free(address);
}
