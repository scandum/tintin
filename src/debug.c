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
*               (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                  *
*                                                                             *
*                     coded by Igor van den Hoven 2004                        *
******************************************************************************/


#include "tintin.h"

#define MAX_STACK_SIZE     51
#define MAX_DEBUG_SIZE     400

char debug_stack[MAX_STACK_SIZE][MAX_DEBUG_SIZE];

short debug_index;

int push_call(char *f, ...)
{
	va_list ap;

	if (debug_index < MAX_STACK_SIZE)
	{
		va_start(ap, f);

		vsnprintf(debug_stack[debug_index], MAX_DEBUG_SIZE - 1, f, ap);

		va_end(ap);
	}

	if (++debug_index == 10000)
	{
		dump_stack();

		return 1;
	}

	return 0;
}

void pop_call(void)
{
	if (debug_index > 0)
	{
		debug_index--;
	}
	else
	{
		tintin_printf2(gtd->ses, "pop_call: index is zero.");
		dump_full_stack();
	}
}

void dump_stack(void)
{
	unsigned char i;

	if (gtd && gtd->ses)
	{
		for (i = 0 ; i < debug_index && i < MAX_STACK_SIZE ; i++)
		{
			tintin_printf2(gtd->ses, "\e[1;32mDEBUG_STACK[\e[1;31m%03d\e[1;32m] = \e[1;31m%s\e[0m", i, debug_stack[i]);
		}
	}
	else
	{
		for (i = 0 ; i < debug_index && i < MAX_STACK_SIZE ; i++)
		{
			tintin_printf2(gtd->ses, "\e[1;32mDEBUG_STACK[\e[1;31m%03d\e[1;32m] = \e[1;31m%s\e[0m\n", i, debug_stack[i]);
		}
	}
}

void dump_stack_fatal(void)
{
	unsigned char i;

	for (i = 0 ; i < debug_index && i < MAX_STACK_SIZE ; i++)
	{
		printf("\e[1;32mDEBUG_STACK[\e[1;31m%03d\e[1;32m] = \e[1;31m%s\e[0m\n", i, debug_stack[i]);
	}
}

void dump_full_stack(void)
{
	unsigned char i;

	tintin_header(gtd->ses, " FULL DEBUG STACK ");

	for (i = 0 ; i < MAX_STACK_SIZE ; i++)
	{
		if (*debug_stack[i])
		{
			tintin_printf2(gtd->ses, "\e[1;31mDEBUG_STACK[%03d] = %s", i, debug_stack[i]);
		}
	}
	tintin_header(gtd->ses, "");
}
