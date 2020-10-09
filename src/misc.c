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
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		print_stdout(0, 0, "\007");
	}
	else if (is_abbrev(arg1, "FLASH"))
	{
		if (is_abbrev(arg2, "ON"))
		{
			print_stdout(0, 0, "\e[?1042h");
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			print_stdout(0, 0, "\e[?1042l");
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #BELL FLASH {ON|OFF}");
		}
	}
	else if (is_abbrev(arg1, "FOCUS"))
	{
		if (is_abbrev(arg2, "ON"))
		{
			print_stdout(0, 0, "\e[?1043h");
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			print_stdout(0, 0, "\e[?1043l");
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #BELL FOCUS {ON|OFF}");
		}
	}
	else if (is_abbrev(arg1, "MARGIN"))
	{
		if (is_abbrev(arg2, "ON"))
		{
			print_stdout(0, 0, "\e[?44h");
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			print_stdout(0, 0, "\e[?44l");
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #BELL MARGIN {ON|OFF}");
		}
	}
	else if (is_abbrev(arg1, "RING"))
	{
		print_stdout(0, 0, "\007");
	}
	else if (is_abbrev(arg1, "VOLUME"))
	{
		if (is_math(ses, arg2))
		{
			print_stdout(0, 0, "\e[ %dt", (int) get_number(ses, arg2));
		}
		else
		{
			show_error(ses, LIST_COMMAND, "#SYNTAX: #BELL VOLUME {1-8}");
		}
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #BELL {FLASH|FOCUS|MARGIN|RING|VOLUME} {ARGUMENT}");
	}

	return ses;
}


DO_COMMAND(do_cr)
{
	write_mud(ses, "", SUB_EOL);

	return ses;
}


DO_COMMAND(do_echo)
{
	char *format, *result, *temp, *output, *left;
	int lnf;

	format = arg1;
	result = arg2;

	temp = str_alloc_stack(0);
	left = str_alloc_stack(0);

	arg = sub_arg_in_braces(ses, arg, format, GET_ONE, SUB_VAR|SUB_FUN);

	format_string(ses, format, arg, result);

	arg = result;

	if (*arg == DEFAULT_OPEN)
	{
		arg = get_arg_in_braces(ses, arg, left, GET_ALL);
		      get_arg_in_braces(ses, arg, temp, GET_ALL);

		if (*temp)
		{
//			int row = (int) get_number(ses, temp);

			substitute(ses, left, format, SUB_COL|SUB_ESC);

			split_show(ses, format, temp, "");

			return ses;
		}
	}

	lnf = is_suffix(arg, "\\") && !is_suffix(arg, "\\\\");

	substitute(ses, arg, temp, SUB_COL|SUB_ESC);

	output = str_alloc_stack(0);

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		str_cpy_printf(&output, "\n\e[0m%s\e[0m", temp);
	}
	else
	{
		str_cpy_printf(&output, "\e[0m%s\e[0m", temp);
	}

	add_line_buffer(ses, output, lnf);

	if (ses == gtd->ses)
	{
		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			save_pos(ses);

			goto_pos(ses, ses->split->bot_row, 1);

			print_line(ses, &output, lnf);
			
			restore_pos(ses);
		}
		else
		{
			print_line(ses, &output, lnf);
		}
	}

	return ses;
}


DO_COMMAND(do_end)
{
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


DO_COMMAND(do_nop)
{
	return ses;
}


DO_COMMAND(do_send)
{
	push_call("do_send(%p,%p)",ses,arg);

	get_arg_in_braces(ses, arg, arg1, GET_ALL);

	write_mud(ses, arg1, SUB_VAR|SUB_FUN|SUB_ESC|SUB_EOL);

	pop_call();
	return ses;
}



DO_COMMAND(do_test)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (!strcmp(arg1, "rain"))
	{
		strcpy(arg2, "9");
		strcpy(arg3, "<f3c3>");
		strcpy(arg1, "1 9");

		if (is_digit(arg[0]))
		{
			sprintf(arg2, "%d", (arg[0] - '0') * (arg[0] - '0'));

			if ((is_hex(arg[1]) && is_hex(arg[2]) && is_hex(arg[3])) || (arg[1] == '?' && arg[2] == '?' && arg[3] == '?'))
			{
				sprintf(arg3, "<f%c%c%c>", arg[1], arg[2], arg[3]);

				if (is_digit(arg[4]) && is_digit(arg[5]))
				{
					sprintf(arg1, "%f %d %s", (arg[4] - '0') * (arg[4] - '0') / 10.0, (arg[5] - '0') * (arg[5] - '0'), &arg[6]);
				}
			}
		}
		command(gtd->ses, do_line, "quiet {#event {RECEIVED KEYPRESS} {#end \\};#screen cursor hide;#screen clear all;#event {SECOND} #loop 0 %s cnt #delay {$cnt / (1.0+%s)} #draw %s rain 1 1 -1 -1 rain %s}", arg2, arg2, arg3, arg1);

		return ses;
	}
	return ses;
}

