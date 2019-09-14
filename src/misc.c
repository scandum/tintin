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


DO_COMMAND(do_bell)
{
	printf("\007");

	return ses;
}


DO_COMMAND(do_commands)
{
	char buf[BUFFER_SIZE] = { 0 }, add[BUFFER_SIZE];
	int cmd;

	tintin_header(ses, " %s ", "COMMANDS");

	for (cmd = 0 ; *command_table[cmd].name != 0 ; cmd++)
	{
		if (*arg && !is_abbrev(arg, command_table[cmd].name))
		{
			continue;
		}
		if ((int) strlen(buf) + 20 > gtd->screen->cols)
		{
			tintin_puts2(ses, buf);
			buf[0] = 0;
		}
		sprintf(add, "%20s", command_table[cmd].name);
		strcat(buf, add);
	}
	if (buf[0])
	{
		tintin_puts2(ses, buf);
	}
	tintin_header(ses, "");

	return ses;
}


DO_COMMAND(do_cr)
{
	write_mud(ses, "", SUB_EOL);

	return ses;
}


DO_COMMAND(do_echo)
{
	char format[BUFFER_SIZE], result[BUFFER_SIZE], temp[BUFFER_SIZE], *output, left[BUFFER_SIZE];
	int lnf;

	arg = sub_arg_in_braces(ses, arg, format, GET_ONE, SUB_VAR|SUB_FUN);

	format_string(ses, format, arg, result);

	arg = result;

	if (*arg == DEFAULT_OPEN)
	{
		arg = get_arg_in_braces(ses, arg, left, GET_ALL);
		      get_arg_in_braces(ses, arg, temp, GET_ALL);

		if (*temp)
		{
			int row = (int) get_number(ses, temp);

			substitute(ses, left, temp, SUB_COL|SUB_ESC);

			do_one_prompt(ses, temp, row, 0);

			return ses;
		}
	}

	lnf = !str_suffix(arg, "\\");

	substitute(ses, arg, temp, SUB_COL|SUB_ESC);

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		output = str_dup_printf("\n\e[0m%s\e[0m", temp);
	}
	else
	{
		output = str_dup_printf("\e[0m%s\e[0m", temp);
	}

	add_line_buffer(ses, output, lnf);

	if (ses == gtd->ses)
	{
		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			save_pos(ses);
			goto_rowcol(ses, ses->bot_row, 1);
		}

		print_line(ses, &output, lnf);

		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			restore_pos(ses);
		}
	}
	str_free(output);

	return ses;
}


DO_COMMAND(do_end)
{
	char arg1[BUFFER_SIZE];

	if (*arg)
	{
		sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN|SUB_COL|SUB_ESC|SUB_LNF);

		quitmsg(arg1);
	}
	else
	{
		quitmsg(NULL);
	}
	return NULL;
}


DO_COMMAND(do_forall)
{
	tintin_printf2(ses, "\e[1;31m#NOTICE: The #forall command no longer exists, please switch to the #foreach command.\n");

	return ses;
}


DO_COMMAND(do_nop)
{
	return ses;
}


DO_COMMAND(do_send)
{
	char arg1[BUFFER_SIZE];

	push_call("do_send(%p,%p)",ses,arg);

	get_arg_in_braces(ses, arg, arg1, GET_ALL);

	write_mud(ses, arg1, SUB_VAR|SUB_FUN|SUB_ESC|SUB_EOL);

	pop_call();
	return ses;
}




DO_COMMAND(do_test)
{
/*
	char pts;
	unsigned char ptu;
	int bla = 0;

	if (*arg == 'c')
	{
		strcpy((char *) bla, "crash baby crash");
	}

	pts = 140;
	ptu = 140;

	printf("pts: %d %d\n", pts, (unsigned char) pts);
	printf("ptu: %d %d\n", (signed char) ptu, ptu);
	printf("pts: %d %d\n", pts, (unsigned int) pts);
	printf("ptu: %d %d\n", (signed int) ptu, ptu);
*/
	return ses;
}
