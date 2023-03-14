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
	int prompt;

	out = str_alloc_stack(0);
	tmp = str_alloc_stack(0);

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	prompt = is_suffix(arg1, "\\") && !is_suffix(arg1, "\\\\");

	substitute(ses, arg1, tmp, SUB_VAR|SUB_FUN);
	substitute(ses, tmp, arg1, SUB_COL|SUB_ESC);

	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

	if (strchr(arg1, '\n'))
	{
		strip_vt102_codes(arg1, tmp);

		check_one_line_multi(ses, arg1, tmp);
	}
	else
	{
		check_one_line(ses, arg1);
	}

	if (ses->gagline > 0)
	{
		ses->gagline--;

		show_debug(ses, LIST_GAG, COLOR_DEBUG "#DEBUG GAG " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} " COLOR_COMMAND "[" COLOR_STRING "%d" COLOR_COMMAND "]", arg1, ses->gagline + 1);

		return ses;
	}

	if (*arg2)
	{
		split_show(ses, arg1, arg2, arg3);

		return ses;
	}

	str_cpy_printf(&out, "%s%s%s", COLOR_TEXT, arg1, COLOR_TEXT);

	tintin_puts3(ses, out, prompt);

	return ses;
}

DO_COMMAND(do_echo)
{
	char *result, *out;
	int prompt;

	result = arg2;

	out = str_alloc_stack(0);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	format_string(ses, arg1, arg, result);

	if (*result == DEFAULT_OPEN)
	{
		arg = get_arg_in_braces(ses, result, arg1, GET_ALL|GET_SPC);
	}
	else
	{
		strcpy(arg1, result);

		arg += strlen(arg);
	}

	substitute(ses, arg1, arg1, SUB_COL|SUB_ESC);

	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg2)
	{
		split_show(ses, arg1, arg2, arg3);

		return ses;
	}

	prompt = is_suffix(arg1, "\\") && !is_suffix(arg1, "\\\\");

	str_cpy_printf(&out, "%s%s%s", COLOR_TEXT, arg1, COLOR_TEXT);

	tintin_puts3(ses, out, prompt);

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
		if (ses->log->file)
		{
			va_start(args, format);

			if (vasprintf(&buffer, format, args) == -1)
			{
				syserr_printf(ses, "show_message: vasprintf2:");

				pop_call();
				return;
			}
			va_end(args);

			logit(ses, buffer, ses->log->file, LOG_FLAG_LINEFEED);

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
		if (ses->log->file)
		{
			logit(ses, buffer, ses->log->file, LOG_FLAG_LINEFEED);
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
	char *buffer;
	va_list args;

	push_call("show_debug(%p,%p,%p)",ses,index,format);

	root = ses->list[index];

	if (gtd->level->debug == 0 && !HAS_BIT(root->flags, LIST_FLAG_DEBUG) && !HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		pop_call();
		return;
	}

	va_start(args, format);
	vasprintf(&buffer, format, args);
	va_end(args);

	if (gtd->level->debug || HAS_BIT(root->flags, LIST_FLAG_DEBUG))
	{
		gtd->level->verbose++;

		tintin_puts2(ses, buffer);

		gtd->level->verbose--;

		goto end;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		if (ses->log->file)
		{
			logit(ses, buffer, ses->log->file, LOG_FLAG_LINEFEED);
		}
	}
	end:
	
	free(buffer);

	pop_call();
	return;
}

void show_info(struct session *ses, int index, char *format, ...)
{
	struct listroot *root;
	char *buffer;
	va_list args;

	push_call("show_info(%p,%p,%p)",ses,index,format);

	root = ses->list[index];

	if (gtd->level->info == 0 && !HAS_BIT(root->flags, LIST_FLAG_INFO))
	{
		pop_call();
		return;
	}
	buffer = str_alloc_stack(0);

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	gtd->level->verbose++;

	tintin_puts(ses, buffer);

	gtd->level->verbose--;

	pop_call();
	return;
}

void print_lines(struct session *ses, int flags, char *color, char *format, ...)
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

		show_lines(ses, color, str_buf);
	}
	else
	{
		show_lines(ses, color, buffer);
	}

	free(buffer);

	pop_call();
	return;
}

void show_lines(struct session *ses, char *color, char *str)
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

		if (*color)
		{
			tintin_printf3(ses, "%s%s", color, str);
		}
		else
		{
			tintin_puts3(ses, str, FALSE);
		}
		str = ptf;
	}
	pop_call();
	return;
}

void tintin_header(struct session *ses, int width, char *format, ...)
{
	char *title, *buffer;
	va_list args;
	int cols;

	push_call("tintin_header(%p,%p)",ses,format);

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

	va_start(args, format);
	vasprintf(&title, format, args);
	va_end(args);

	if ((int) strlen(title) > cols - 2)
	{
		title[cols - 2] = 0;
	}

	buffer = calloc(1, cols + 1);

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_SCREENREADER))
	{
		memset(buffer, ' ', cols);
	}
	else
	{
		memset(buffer, '#', cols);
	}

	memcpy(&buffer[(cols - strlen(title)) / 2], title, strlen(title));

	tintin_puts2(ses, buffer);

	free(title);
	free(buffer);

	pop_call();
	return;
}


void tintin_printf(struct session *ses, char *format, ...)
{
	char *buffer;
	va_list args;

	push_call("tintin_printf(%p,%p,...)",ses,format);

	buffer = str_alloc_stack(0);

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	tintin_puts(ses, buffer);

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

void tintin_printf3(struct session *ses, char *format, ...)
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

	tintin_puts3(ses, buffer, FALSE);

	free(buffer);

	pop_call();
	return;
}

// Show string and fire triggers

void tintin_puts(struct session *ses, char *string)
{
	if (ses == NULL)
	{
		ses = gtd->ses;
	}

	check_one_line(ses, string);

	if (ses->gagline > 0)
	{
		ses->gagline--;

		gtd->level->ignore++;

		show_debug(ses, LIST_GAG, COLOR_DEBUG "#DEBUG GAG " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} " COLOR_COMMAND "[" COLOR_STRING "%d" COLOR_COMMAND "]", string, ses->gagline + 1);

		gtd->level->ignore--;
	}
	else
	{
		tintin_puts2(ses, string);
	}
}

// show string and don't fire triggers

void tintin_puts2(struct session *ses, char *string)
{
	char *output;

	push_call("tintin_puts2(%p,%p)",ses,string);

	output = str_alloc_stack(0);

	str_cpy_printf(&output, "%s%s%s", COLOR_TEXT, string, COLOR_TEXT);

	tintin_puts3(ses, output, FALSE);

	pop_call();
	return;
}

// show string, no triggers, no color reset

void tintin_puts3(struct session *ses, char *string, int prompt)
{
	char *output;

	push_call("tintin_puts3(%p,%p,%d)",ses,string,prompt);

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

	if (ses->check_output)
	{
		process_more_output(ses, "", FALSE);
	}

	output = str_alloc_stack(0);

	// no new line when prompt is overwritten with input in split mode

	if (gtd->level->scroll == 0 && *ses->scroll->input)
	{
		str_cpy(&output, "\n");
	}

	str_cat(&output, string);

	add_line_buffer(ses, output, prompt);

	if (ses == gtd->ses)
	{
		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			save_pos(ses);

			goto_pos(ses, ses->split->bot_row, ses->split->top_col);
		}

		print_line(ses, &output, prompt);

		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			restore_pos(ses);
		}
	}
	pop_call();
	return;
}
