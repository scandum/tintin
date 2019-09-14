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
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE], temp[STRING_SIZE], *output;
	int lnf;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	lnf = !str_suffix(arg1, "\\");

	substitute(ses, arg1, temp, SUB_VAR|SUB_FUN);
	substitute(ses, temp, arg1, SUB_COL|SUB_ESC);

	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN);

	do_one_line(arg1, ses);

	if (HAS_BIT(ses->flags, SES_FLAG_GAG))
	{
		DEL_BIT(ses->flags, SES_FLAG_GAG);

		gtd->ignore_level++;

		show_info(ses, LIST_GAG, "#INFO GAG {%s}", arg1);

		gtd->ignore_level--;

		return ses;
	}

	if (*arg2)
	{
		do_one_prompt(ses, arg1, (int) get_number(ses, arg2), (int) get_number(ses, arg3));

		return ses;
	}

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		output = str_dup_printf("\n%s%s%s", COLOR_TEXT, arg1, COLOR_TEXT);
	}
	else
	{
		output = str_dup_printf("%s%s%s", COLOR_TEXT, arg1, COLOR_TEXT);
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

void show_message(struct session *ses, int index, char *format, ...)
{
	struct listroot *root;
	char *buffer;
	va_list args;

	push_call("show_message(%p,%p,%p)",ses,index,format);

	root = ses->list[index];

	if (gtd->verbose_level)
	{
		goto display;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_DEBUG))
	{
		goto display;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_MESSAGE))
	{
		if (gtd->input_level == 0)
		{
			goto display;
		}
		pop_call();
		return;
	}

	display:

	va_start(args, format);
	vasprintf(&buffer, format, args);
	va_end(args);

	tintin_puts2(ses, buffer);

	if (HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		if (ses->logfile)
		{
			logit(ses, buffer, ses->logfile, TRUE);
		}
	}

	free(buffer);

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
	vasprintf(&buffer, format, args);
	va_end(args);

	if (gtd->verbose_level)
	{
		tintin_puts2(ses, buffer);

		goto end;
	}

	root = ses->list[index];

	if (HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		if (ses->logfile)
		{
			logit(ses, buffer, ses->logfile, TRUE);
		}
	}

	if (HAS_BIT(root->flags, LIST_FLAG_DEBUG))
	{
		tintin_puts2(ses, buffer);

		goto end;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_MESSAGE))
	{
		tintin_puts2(ses, buffer);
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

	if (!HAS_BIT(root->flags, LIST_FLAG_DEBUG) && !HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		pop_call();
		return;
	}

	va_start(args, format);

	vsprintf(buf, format, args);

	va_end(args);

	if (HAS_BIT(root->flags, LIST_FLAG_DEBUG))
	{
		gtd->verbose_level++;

		tintin_puts2(ses, buf);

		gtd->verbose_level--;

		pop_call();
		return;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		if (ses->logfile)
		{
			logit(ses, buf, ses->logfile, TRUE);
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

	if (!HAS_BIT(root->flags, LIST_FLAG_INFO))
	{
		pop_call();
		return;
	}

	va_start(args, format);

	vsprintf(buf, format, args);

	va_end(args);

	gtd->verbose_level++;

	tintin_puts(ses, buf);

	gtd->verbose_level--;

	pop_call();
	return;
}

void show_lines(struct session *ses, int flags, char *format, ...)
{
	char *buffer, *str_buf, *pto, *ptf;
	va_list args;

	push_call("show_lines(%p,%d,%p,...)",ses,flags,format);

	va_start(args, format);
	vasprintf(&buffer, format, args);
	va_end(args);

	str_buf = str_alloc(BUFFER_SIZE + strlen(buffer) * 2);

	substitute(ses, buffer, str_buf, flags);


	pto = str_buf;

	while (*pto)
	{
		ptf = strchr(pto, '\n');

		if (ptf == NULL)
		{
			break;
		}
		*ptf++ = 0;

		tintin_puts3(ses, pto);

		pto = ptf;
	}

	free(buffer);
	str_free(str_buf);

	pop_call();
	return;
}


void tintin_header(struct session *ses, char *format, ...)
{
	char arg[BUFFER_SIZE], buf[BUFFER_SIZE];
	va_list args;

	va_start(args, format);
	vsprintf(arg, format, args);
	va_end(args);

	if ((int) strlen(arg) > gtd->screen->cols - 2)
	{
		arg[gtd->screen->cols - 2] = 0;
	}

	if (HAS_BIT(ses->flags, SES_FLAG_SCREENREADER))
	{
		memset(buf, ' ', gtd->screen->cols);
	}
	else
	{
		memset(buf, '#', gtd->screen->cols);
	}

	memcpy(&buf[(gtd->screen->cols - strlen(arg)) / 2], arg, strlen(arg));

	buf[gtd->screen->cols] = 0;

	tintin_puts2(ses, buf);
}

void tintin_printf2(struct session *ses, char *format, ...)
{
	char *buffer;
	va_list args;

	push_call("tintin_printf2(%p,%p,...)",ses,format);

	va_start(args, format);
	vasprintf(&buffer, format, args);
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

	if (HAS_BIT(ses->flags, SES_FLAG_GAG))
	{
		DEL_BIT(ses->flags, SES_FLAG_GAG);

		gtd->ignore_level++;

		show_info(ses, LIST_GAG, "#INFO GAG {%s}", string);

		gtd->ignore_level--;
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

	output = str_dup_printf("%s%s%s", COLOR_TEXT, string, COLOR_TEXT);

	tintin_puts3(ses, output);

/*

	if (ses == NULL)
	{
		ses = gtd->ses;
	}

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_VERBOSE) && gtd->quiet && gtd->verbose_level == 0)
	{
		pop_call();
		return;
	}

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		output = str_dup_printf("\n\e[0m%s\e[0m", string);
	}
	else
	{
		output = str_dup_printf("\e[0m%s\e[0m", string);
	}

	add_line_buffer(ses, output, FALSE);

	if (ses == gtd->ses)
	{
		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			save_pos(ses);
			goto_rowcol(ses, ses->bot_row, 1);
		}

		print_line(ses, &output, FALSE);

		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			restore_pos(ses);
		}
	}
*/
	str_free(output);

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

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_VERBOSE) && gtd->quiet && gtd->verbose_level == 0)
	{
		pop_call();
		return;
	}

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		output = str_dup_printf("\n%s", string);
	}
	else
	{
		output = str_dup_printf("%s", string);
	}

	add_line_buffer(ses, output, FALSE);

	if (ses == gtd->ses)
	{
		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			save_pos(ses);
			goto_rowcol(ses, ses->bot_row, 1);
		}

		print_line(ses, &output, FALSE);

		if (!HAS_BIT(ses->flags, SES_FLAG_READMUD) && IS_SPLIT(ses))
		{
			restore_pos(ses);
		}
	}

	str_free(output);

	pop_call();
	return;
}
