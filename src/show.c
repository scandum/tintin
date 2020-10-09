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
*                       coded by Peter Unold 1992                             *
*                  recoded by Igor van den Hoven 2005                         *
******************************************************************************/

#include "tintin.h"


DO_COMMAND(do_showme)
{
	char *out, *tmp;
	int lnf;

	out = str_alloc_stack(0);
	tmp = str_alloc_stack(0);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	lnf = is_suffix(arg1, "\\") && !is_suffix(arg1, "\\\\");

	substitute(ses, arg1, tmp, SUB_VAR|SUB_FUN);
	substitute(ses, tmp, arg1, SUB_COL|SUB_ESC);

	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

	do_one_line(arg1, ses);

	if (ses->gagline > 0)
	{
		ses->gagline--;

		show_debug(ses, LIST_GAG, "#DEBUG GAG {%d} {%s}", ses->gagline + 1, arg1);

		return ses;
	}

	if (*arg2)
	{
		split_show(ses, arg1, arg2, arg3);

		return ses;
	}

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		str_cpy_printf(&out, "\n%s%s%s", COLOR_TEXT, arg1, COLOR_TEXT);
	}
	else
	{
		str_cpy_printf(&out, "%s%s%s", COLOR_TEXT, arg1, COLOR_TEXT);
	}

	add_line_buffer(ses, out, lnf);

	if (ses == gtd->ses)
	{
		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			save_pos(ses);

			goto_pos(ses, ses->split->bot_row, ses->split->top_col);
		}

		print_line(ses, &out, lnf);

		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			restore_pos(ses);
		}
	}

	return ses;
}

void show_message(struct session *ses, int index, char *format, ...)
{
	struct listroot *root;
	char *buffer;
	va_list args;

	push_call("show_message(%p,%p,%p)",ses,index,format);

	root = ses->list[index];

	if (gtd->level->verbose || gtd->level->debug)
	{
		goto display;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_DEBUG))
	{
		goto display;
	}

	if (!HAS_BIT(root->flags, LIST_FLAG_MESSAGE))
	{
		goto end;
	}

	if (gtd->level->input)
	{
		goto end;
	}

	display:

	va_start(args, format);

	if (vasprintf(&buffer, format, args) == -1)
	{
		syserr_printf(ses, "show_message: vasprintf1:");

		pop_call();
		return;
	}

	va_end(args);

	tintin_puts2(ses, buffer);

	free(buffer);

	pop_call();
	return;

	end:

	if (HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		if (ses->logfile)
		{
			va_start(args, format);

			if (vasprintf(&buffer, format, args) == -1)
			{
				syserr_printf(ses, "print_lines: vasprintf2:");
				
				pop_call();
				return;
			}
			va_end(args);

			logit(ses, buffer, ses->logfile, LOG_FLAG_LINEFEED);

			free(buffer);
		}
	}

	pop_call();
	return;
}

void show_error(struct session *ses, int index, char *format, ...)
{
	struct listroot *root;
	char *buffer;
	va_list args;

	push_call("show_error(%p,%p,%p)",ses,index,format);

	va_start(args, format);
	if (vasprintf(&buffer, format, args) == -1)
	{
		syserr_printf(ses, "show_error: vasprintf:");

		pop_call();
		return;
	}
	va_end(args);

	check_all_events(ses, SUB_SEC|EVENT_FLAG_SYSTEM, 0, 1, "RECEIVED ERROR", buffer);

	if (gtd->level->verbose || gtd->level->debug)
	{
		tintin_puts2(ses, buffer);

		goto end;
	}

	root = ses->list[index];

	if (HAS_BIT(root->flags, LIST_FLAG_DEBUG))
	{
		tintin_puts2(ses, buffer);

		goto end;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_MESSAGE))
	{
		tintin_puts2(ses, buffer);

		goto end;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		if (ses->logfile)
		{
			logit(ses, buffer, ses->logfile, LOG_FLAG_LINEFEED);
		}
	}

	end:

	free(buffer);

	pop_call();
	return;
}

void show_debug(struct session *ses, int index, char *format, ...)
{
	struct listroot *root;
	char buf[STRING_SIZE];
	va_list args;

	push_call("show_debug(%p,%p,%p)",ses,index,format);

	root = ses->list[index];

	if (gtd->level->debug == 0 && !HAS_BIT(root->flags, LIST_FLAG_DEBUG) && !HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		pop_call();
		return;
	}

	va_start(args, format);

	vsprintf(buf, format, args);

	va_end(args);

	if (gtd->level->debug || HAS_BIT(root->flags, LIST_FLAG_DEBUG))
	{
		gtd->level->verbose++;

		tintin_puts2(ses, buf);

		gtd->level->verbose--;

		pop_call();
		return;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		if (ses->logfile)
		{
			logit(ses, buf, ses->logfile, LOG_FLAG_LINEFEED);
		}
	}
	pop_call();
	return;
}

void show_info(struct session *ses, int index, char *format, ...)
{
	struct listroot *root;
	char buf[STRING_SIZE];
	va_list args;

	push_call("show_info(%p,%p,%p)",ses,index,format);

	root = ses->list[index];

	if (gtd->level->info == 0 && !HAS_BIT(root->flags, LIST_FLAG_INFO))
	{
		pop_call();
		return;
	}

	va_start(args, format);

	vsprintf(buf, format, args);

	va_end(args);

	gtd->level->verbose++;

	tintin_puts(ses, buf);

	gtd->level->verbose--;

	pop_call();
	return;
}

void print_lines(struct session *ses, int flags, char *format, ...)
{
	char *buffer, *str_buf;
	va_list args;

	push_call("print_lines(%p,%d,%p,...)",ses,flags,format);

	va_start(args, format);

	if (vasprintf(&buffer, format, args) == -1)
	{
		syserr_printf(ses, "print_lines: vasprintf:");

		pop_call();
		return;
	}

	va_end(args);

	if (flags)
	{
		str_buf = str_alloc_stack(strlen(buffer) * 2);

		substitute(ses, buffer, str_buf, flags);

		show_lines(ses, str_buf);
	}
	else
	{
		show_lines(ses, buffer);
	}

	free(buffer);

	pop_call();
	return;
}

void show_lines(struct session *ses, char *str)
{
	char *ptf;

	push_call("show_lines(%p,%p,...)",ses,str);

	while (*str)
	{
		ptf = strchr(str, '\n');

		if (ptf == NULL)
		{
			break;
		}
		*ptf++ = 0;

		tintin_puts3(ses, str);

		str = ptf;
	}
	pop_call();
	return;
}


void tintin_header(struct session *ses, int width, char *format, ...)
{
	char arg[BUFFER_SIZE], buf[BUFFER_SIZE];
	va_list args;
	int cols;

	push_call("tintin_header(%p,%p)",ses,format);

	va_start(args, format);
	vsprintf(arg, format, args);
	va_end(args);

	if (width)
	{
		cols = UMIN(width, get_scroll_cols(ses));
	}
	else
	{
		cols = get_scroll_cols(ses);
	}

	if (cols < 2)
	{
		pop_call();
		return;
	}

	if ((int) strlen(arg) > cols - 2)
	{
		arg[cols - 2] = 0;
	}

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_SCREENREADER))
	{
		memset(buf, ' ', cols);
	}
	else
	{
		memset(buf, '#', cols);
	}

	memcpy(&buf[(cols - strlen(arg)) / 2], arg, strlen(arg));

	buf[cols] = 0;

	tintin_puts2(ses, buf);

	pop_call();
	return;
}

void tintin_printf2(struct session *ses, char *format, ...)
{
	char *buffer;
	va_list args;

	push_call("tintin_printf2(%p,%p,...)",ses,format);

	va_start(args, format);
	if (vasprintf(&buffer, format, args) == -1)
	{
		syserr_printf(ses, "tintin_printf2: vasprintf:");

		pop_call();
		return;
	}
	va_end(args);

	tintin_puts2(ses, buffer);

	free(buffer);

	pop_call();
	return;
}

void tintin_printf(struct session *ses, char *format, ...)
{
	char buffer[BUFFER_SIZE];
	va_list args;

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	tintin_puts(ses, buffer);
}

/*
	Show string and fire triggers
*/

void tintin_puts(struct session *ses, char *string)
{
	if (ses == NULL)
	{
		ses = gtd->ses;
	}

	do_one_line(string, ses);

	if (ses->gagline)
	{
		ses->gagline--;

		gtd->level->ignore++;

		show_debug(ses, LIST_GAG, "#DEBUG GAG {%d} {%s}", ses->gagline + 1, string);

		gtd->level->ignore--;
	}
	else
	{
		tintin_puts2(ses, string);
	}
}

/*
	show string and don't fire triggers
*/

void tintin_puts2(struct session *ses, char *string)
{
	char *output;

	push_call("tintin_puts2(%p,%p)",ses,string);

	output = str_alloc_stack(0);

	str_cpy_printf(&output, "%s%s%s", COLOR_TEXT, string, COLOR_TEXT);

	tintin_puts3(ses, output);

	pop_call();
	return;
}



/*
	show string, no triggers, no color reset
*/

void tintin_puts3(struct session *ses, char *string)
{
	char *output;

	push_call("tintin_puts3(%p,%p)",ses,string);

	if (ses == NULL)
	{
		ses = gtd->ses;
	}

	if (ses->line_capturefile)
	{
		if (ses->line_captureindex == 1)
		{
			set_nest_node_ses(ses, ses->line_capturefile, "{%d}{%s}", ses->line_captureindex++, string);
		}
		else
		{
			add_nest_node_ses(ses, ses->line_capturefile, "{%d}{%s}", ses->line_captureindex++, string);
		}
	}

	if (gtd->level->quiet && gtd->level->verbose == 0)
	{
		pop_call();
		return;
	}

	output = str_alloc_stack(0);

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		str_cpy_printf(&output, "\n%s", string);
	}
	else
	{
		str_cpy_printf(&output, "%s", string);
	}

	add_line_buffer(ses, output, FALSE);

	if (ses == gtd->ses)
	{
		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			save_pos(ses);

			goto_pos(ses, ses->split->bot_row, ses->split->top_col);
		}

		print_line(ses, &output, FALSE);

		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			restore_pos(ses);
		}
	}

	pop_call();
	return;
}

