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
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/


#include "tintin.h"

int push_call_printf(char *format, ...)
{
	int len;
	va_list ap;

	len = gtd->memory->debug_len;

	if (len == gtd->memory->debug_max)
	{
		gtd->memory->debug_max++;

		gtd->memory->debug = (struct stack_data **) realloc(gtd->memory->debug, sizeof(struct str_data *) * gtd->memory->debug_max);

		gtd->memory->debug[len] = calloc(1, sizeof(struct stack_data));

		gtd->memory->debug[len]->name = calloc(1, NAME_SIZE);
	}

	va_start(ap, format);

	vsnprintf(gtd->memory->debug[len]->name, NAME_SIZE - 1, format, ap);

	va_end(ap);

	gtd->memory->debug[len]->index = gtd->memory->stack_len;

	if (gtd->memory->debug_len++ == 100)
	{
		tintin_printf2(gtd->ses, "\e[1;31merror: push_call_printf: stack size is going past 100.");

		tintin_printf2(gtd->ses, "\e[1;32mDEBUG_STACK[\e[1;31m%03d\e[1;32m] [%03d] = \e[1;31m%s\e[0m", len, gtd->memory->debug[len]->index, gtd->memory->debug[len]->name);

		return FALSE;
	}
	if (gtd->memory->debug_len > 500)
	{
		return FALSE;
	}
	return TRUE;
}

void push_call(char *format, ...)
{
	int len;

	len = gtd->memory->debug_len;

	if (len == gtd->memory->debug_max)
	{
		gtd->memory->debug_max++;

		gtd->memory->debug = (struct stack_data **) realloc(gtd->memory->debug, sizeof(struct str_data *) * gtd->memory->debug_max);

		gtd->memory->debug[len] = calloc(1, sizeof(struct stack_data));

		gtd->memory->debug[len]->name = calloc(1, NAME_SIZE);
	}

	strcpy(gtd->memory->debug[len]->name, format);

	gtd->memory->debug[len]->index = gtd->memory->stack_len;

	gtd->memory->debug_len++;
}

void pop_call(void)
{
	if (gtd->memory->debug_len > 0)
	{
		gtd->memory->debug_len--;
		gtd->memory->stack_len = gtd->memory->debug[gtd->memory->debug_len]->index;
	}
	else
	{
		tintin_printf2(gtd->ses, "\e[1;31merror: pop_call: index is zero.");

		gtd->memory->debug_len = gtd->memory->debug_max;

		dump_stack();

		gtd->memory->debug_len = 0;
	}
}

void dump_stack(void)
{
	unsigned int i;

	if (gtd && gtd->ses)
	{
		for (i = 0 ; i < gtd->memory->debug_len ; i++)
		{
			tintin_printf2(gtd->ses, "\e[1;32mDEBUG_STACK[\e[1;31m%03d\e[1;32m] [%03d] = \e[1;31m%s\e[0m", i, gtd->memory->debug[i]->index, gtd->memory->debug[i]->name);
		}
	}
}
