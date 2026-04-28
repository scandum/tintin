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

#define DO_LOG(log) void log (struct session *ses, char *arg, char *arg1, char *arg2)

DO_LOG(log_append);
DO_LOG(log_info);
DO_LOG(log_make);
DO_LOG(log_move);
DO_LOG(log_overwrite);
DO_LOG(log_off);
DO_LOG(log_remove);
DO_LOG(log_timestamp);

static void write_single_log(struct session *ses, struct log_data *log_ptr, char *txt, FILE *file, int flags);

typedef void LOG (struct session *ses, char *arg, char *arg1, char *arg2);

struct log_type
{
	char                  * name;
	LOG                   * fun;
	char                  * desc;
};

struct log_type log_table[] =
{
	{    "APPEND",            log_append,          "Start logging, appending to given file."        },
	{    "INFO",              log_info,            "Some logging related info."                     },
	{    "MAKE",              log_make,            "Make the given directory."                      },
	{    "MOVE",              log_move,            "Move the given file."                           },
	{    "OFF",               log_off,             "Stop logging."                                  },
	{    "OVERWRITE",         log_overwrite,       "Start logging, overwriting the given file."     },
	{    "REMOVE",            log_remove,          "Remove the given file or directory."            },
	{    "TIMESTAMP",         log_timestamp,       "Timestamp prepended to each log line."          },
	{    "",                  NULL,                ""                                               }
};

DO_COMMAND(do_log)
{
	int cnt;

	push_call("do_log(%p,%p)",ses,arg);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN|SUB_ESC);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " LOG OPTIONS ");

		for (cnt = 0 ; *log_table[cnt].fun != NULL ; cnt++)
		{
			if (*log_table[cnt].desc)
			{
				tintin_printf2(ses, "  [%-13s] %s", log_table[cnt].name, log_table[cnt].desc);
			}
		}
		pop_call();
		return ses;
	}

	for (cnt = 0 ; *log_table[cnt].name ; cnt++)
	{
		if (is_abbrev(arg1, log_table[cnt].name))
		{
			log_table[cnt].fun(ses, arg, arg1, arg2);

			pop_call();
			return ses;
		}
	}
	show_error(ses, LIST_COMMAND, "#ERROR: #LOG {%s}: INVALID LOG OPTION.", arg1);

	pop_call();
	return ses;
}

DO_LOG(log_append)
{
	char arg3[BUFFER_SIZE], arg4[BUFFER_SIZE];
	char name[BUFFER_SIZE], color[BUFFER_SIZE], filename[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN|SUB_ESC|SUB_COL);
	arg = sub_arg_in_braces(ses, arg, arg4, GET_ONE, SUB_VAR|SUB_FUN|SUB_ESC|SUB_COL);

	if (*arg3 == 0) {
		strcpy(name, "default");
		strcpy(color, "");
		strcpy(filename, arg2);
	} else if (*arg4 == 0) {
		strcpy(name, arg2);
		strcpy(color, "");
		strcpy(filename, arg3);
	} else {
		strcpy(name, arg2);
		strcpy(color, arg3);
		strcpy(filename, arg4);
	}

	struct log_data *log_ptr, *log_tail = NULL;
	for (log_ptr = ses->log; log_ptr; log_ptr = log_ptr->next) {
		if (!strcmp(log_ptr->name, name)) break;
		log_tail = log_ptr;
	}

	if (!log_ptr) {
		log_ptr = calloc(1, sizeof(struct log_data));
		RESTRING(log_ptr->name, name);
		RESTRING(log_ptr->stamp_strf, "");
		LINK(log_ptr, ses->log, log_tail);
	}

	if (log_ptr->file && log_ptr->file != (FILE *)1)
	{
		fclose(log_ptr->file);
	}

	if ((log_ptr->file = fopen(filename, "a")))
	{
		SET_BIT(log_ptr->mode, LOG_FLAG_APPEND);

		RESTRING(log_ptr->line_name, filename);
		RESTRING(log_ptr->color_target, color);

		logheader(ses, log_ptr->file, log_ptr->mode | (ses->log ? ses->log->mode : 0));

		if (*color) {
			show_message(ses, LIST_COMMAND, "#LOG: LOGGING OUTPUT TO '%s' (COLOR FILTER ACTIVE) FILESIZE: %ld", filename, ftell(log_ptr->file));
		} else {
			show_message(ses, LIST_COMMAND, "#LOG: LOGGING OUTPUT TO '%s' FILESIZE: %ld", filename, ftell(log_ptr->file));
		}

		if (!ses->log->file) ses->log->file = (FILE *)1;
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #LOG {%s} {%s}: COULDN'T OPEN FILE.", arg1, filename);
	}
}

DO_LOG(log_info)
{
	struct log_data *log_ptr;
	for (log_ptr = ses->log; log_ptr; log_ptr = log_ptr->next) {
		if (log_ptr->file && log_ptr->file != (FILE *)1) {
			tintin_printf2(ses, "#LOG INFO: ID    = %s", log_ptr->name);
			tintin_printf2(ses, "#LOG INFO: FILE  = %s", log_ptr->line_name);
			tintin_printf2(ses, "#LOG INFO: LEVEL = %s", HAS_BIT(log_ptr->mode, LOG_FLAG_LOW) ? "LOW" : "HIGH");
			tintin_printf2(ses, "#LOG INFO: MODE  = %s", HAS_BIT(log_ptr->mode, LOG_FLAG_HTML) ? "HTML" : HAS_BIT(log_ptr->mode, LOG_FLAG_PLAIN) ? "PLAIN" : HAS_BIT(log_ptr->mode, LOG_FLAG_RAW) ? "RAW" : "UNSET");
			if (log_ptr->color_target && *log_ptr->color_target) {
				tintin_printf2(ses, "#LOG INFO: COLOR = FILTER ACTIVE");
			}
			tintin_printf2(ses, "#LOG INFO: STAMP = %s\n", log_ptr->stamp_strf);
		}
	}
}

DO_LOG(log_make)
{
	if (mkdir(arg2, 0755))
	{
		if (errno != EEXIST)
		{
			show_error(ses, LIST_COMMAND, "#ERROR: #LOG MAKE: FAILED TO CREATE DIRECTORY {%s} (%s).", arg2, strerror(errno));
		}
		else
		{
			show_message(ses, LIST_COMMAND, "#LOG MAKE: DIRECTORY {%s} ALREADY EXISTS.", arg2);
		}
	}
	else
	{
		show_message(ses, LIST_COMMAND, "#LOG MAKE: CREATED DIRECTORY {%s}.", arg2);
	}
}

DO_LOG(log_move)
{
	char *arg3;
	int result;

	arg3 = str_alloc_stack(0);

	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	result = rename(arg2, arg3);

	if (result == 0)
	{
		show_message(ses, LIST_COMMAND, "#LOG MOVE: FILE {%s} MOVED TO {%s}.", arg2, arg3);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#LOG MOVE: COULDN'T MOVE FILE {%s} TO {%s}.", arg2, arg3);
	}
}

DO_LOG(log_overwrite)
{
	char arg3[BUFFER_SIZE], arg4[BUFFER_SIZE];
	char name[BUFFER_SIZE], color[BUFFER_SIZE], filename[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN|SUB_ESC|SUB_COL);
	arg = sub_arg_in_braces(ses, arg, arg4, GET_ONE, SUB_VAR|SUB_FUN|SUB_ESC|SUB_COL);

	if (*arg3 == 0) {
		strcpy(name, "default");
		strcpy(color, "");
		strcpy(filename, arg2);
	} else if (*arg4 == 0) {
		strcpy(name, arg2);
		strcpy(color, "");
		strcpy(filename, arg3);
	} else {
		strcpy(name, arg2);
		strcpy(color, arg3);
		strcpy(filename, arg4);
	}

	struct log_data *log_ptr, *log_tail = NULL;
	for (log_ptr = ses->log; log_ptr; log_ptr = log_ptr->next) {
		if (!strcmp(log_ptr->name, name)) break;
		log_tail = log_ptr;
	}

	if (!log_ptr) {
		log_ptr = calloc(1, sizeof(struct log_data));
		RESTRING(log_ptr->name, name);
		RESTRING(log_ptr->stamp_strf, "");
		LINK(log_ptr, ses->log, log_tail);
	}

	if (log_ptr->file && log_ptr->file != (FILE *)1)
	{
		fclose(log_ptr->file);
	}

	if ((log_ptr->file = fopen(filename, "w")))
	{
		SET_BIT(log_ptr->mode, LOG_FLAG_OVERWRITE);

		RESTRING(log_ptr->line_name, filename);
		RESTRING(log_ptr->color_target, color);

		logheader(ses, log_ptr->file, log_ptr->mode | (ses->log ? ses->log->mode : 0));

		if (*color) {
			show_message(ses, LIST_COMMAND, "#LOG: LOGGING OUTPUT TO '%s' (COLOR FILTER ACTIVE)", filename);
		} else {
			show_message(ses, LIST_COMMAND, "#LOG: LOGGING OUTPUT TO '%s'", filename);
		}

		if (!ses->log->file) ses->log->file = (FILE *)1;
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #LOG {%s} {%s}: COULDN'T OPEN FILE.", arg1, filename);
	}
}

DO_LOG(log_off)
{
	char name[BUFFER_SIZE];
	if (*arg2 == 0) strcpy(name, "default");
	else if (!strcasecmp(arg2, "ALL")) strcpy(name, "ALL");
	else strcpy(name, arg2);

	struct log_data *log_ptr, *next_log;
	int closed_any = 0;

	for (log_ptr = ses->log; log_ptr; log_ptr = next_log) {
		next_log = log_ptr->next;
		if (!strcasecmp(name, "ALL") || !strcmp(log_ptr->name, name)) {
			if (log_ptr->file && log_ptr->file != (FILE *)1) {
				DEL_BIT(log_ptr->mode, LOG_FLAG_APPEND|LOG_FLAG_OVERWRITE);
				fclose(log_ptr->file);
				log_ptr->file = NULL;
				closed_any = 1;
				show_message(ses, LIST_COMMAND, "#LOG {OFF}: LOGGING TURNED OFF FOR '%s'.", log_ptr->name);
			}
		}
	}

	if (!closed_any) {
		show_message(ses, LIST_COMMAND, "#LOG: LOGGING ALREADY TURNED OFF FOR '%s'.", name);
	}

	if (ses->log->file == NULL) {
		for (log_ptr = ses->log->next; log_ptr; log_ptr = log_ptr->next) {
			if (log_ptr->file && log_ptr->file != (FILE *)1) {
				ses->log->file = (FILE *)1;
				break;
			}
		}
	}
}

DO_LOG(log_remove)
{
	int result = remove(arg2);

	if (result == 0)
	{
		show_message(ses, LIST_COMMAND, "#LOG REMOVE: FILE {%s} REMOVED.", arg2);
	}
	else
	{
		show_error(ses, LIST_COMMAND, "#LOG REMOVE: COULDN'T REMOVE FILE {%s}.", arg2);
	}
}

DO_LOG(log_timestamp)
{
	char arg3[BUFFER_SIZE];
	char name[BUFFER_SIZE], format[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg3, GET_ONE, SUB_VAR|SUB_FUN|SUB_ESC|SUB_COL);

	if (*arg3 == 0) {
		strcpy(name, "ALL");
		strcpy(format, arg2);
	} else {
		strcpy(name, arg2);
		strcpy(format, arg3);
	}

	struct log_data *log_ptr;
	int found = 0;

	for (log_ptr = ses->log; log_ptr; log_ptr = log_ptr->next) {
		if (!strcasecmp(name, "ALL") || !strcmp(log_ptr->name, name)) {
			RESTRING(log_ptr->stamp_strf, format);
			log_ptr->stamp_time = 0;
			found = 1;
		}
	}

	if (found) {
		if (!strcasecmp(name, "ALL")) {
			show_message(ses, LIST_COMMAND, "#LOG TIMESTAMP: FORMAT SET TO {%s} FOR ALL LOGS.", format);
		} else {
			show_message(ses, LIST_COMMAND, "#LOG TIMESTAMP: FORMAT SET TO {%s} FOR LOG '%s'.", format, name);
		}
	} else {
		show_error(ses, LIST_COMMAND, "#ERROR: #LOG TIMESTAMP: NO ACTIVE LOG NAMED '%s' FOUND.", name);
	}
}

void init_log(struct session *ses)
{
	if (!ses->log) ses->log = calloc(1, sizeof(struct log_data));
	ses->log->name        = strdup("default");
	ses->log->color_target= strdup("");
	ses->log->next_name   = strdup("");
	ses->log->line_name   = strdup("");
	ses->log->stamp_strf  = strdup("");
}

void free_log(struct session *ses)
{
	struct log_data *log_ptr, *next_log;
	for (log_ptr = ses->log; log_ptr; log_ptr = next_log) {
		next_log = log_ptr->next;
		if (log_ptr->name) free(log_ptr->name);
		if (log_ptr->color_target) free(log_ptr->color_target);
		if (log_ptr->next_name) free(log_ptr->next_name);
		if (log_ptr->line_name) free(log_ptr->line_name);
		if (log_ptr->stamp_strf) free(log_ptr->stamp_strf);
		if (log_ptr->file && log_ptr->file != (FILE *)1) {
			fclose(log_ptr->file);
			log_ptr->file = NULL;
		}
		free(log_ptr);
	}
	ses->log = NULL;
}

static void write_single_log(struct session *ses, struct log_data *log_ptr, char *txt, FILE *file, int flags)
{
	char out[BUFFER_SIZE];

	if (log_ptr && log_ptr->stamp_strf && *log_ptr->stamp_strf && (HAS_BIT(log_ptr->mode, LOG_FLAG_STAMP) || file == log_ptr->file))
	{
		if (log_ptr->stamp_time != gtd->time)
		{
			struct tm timeval_tm = *localtime(&gtd->time);

			log_ptr->stamp_time = gtd->time;

			substitute(ses, log_ptr->stamp_strf, out, SUB_COL|SUB_ESC|SUB_VAR|SUB_FUN);

			strftime(log_ptr->stamp_text, 99, out, &timeval_tm);
		}
		fputs(log_ptr->stamp_text, file);
	}

	int mode = log_ptr ? (log_ptr->mode | (ses->log ? ses->log->mode : 0)) : flags;

	if (HAS_BIT(mode, LOG_FLAG_PLAIN) || HAS_BIT(flags, LOG_FLAG_PLAIN))
	{
		strip_vt102_codes(txt, out);
	}
	else if (HAS_BIT(mode, LOG_FLAG_HTML))
	{
		vt102_to_html(ses, txt, out);
	}
	else
	{
		strcpy(out, txt);
	}

	if (HAS_BIT(flags, LOG_FLAG_LINEFEED))
	{
		strcat(out, "\n");
	}
	fputs(out, file);

	fflush(file);
}

void logit(struct session *ses, char *txt, FILE *file, int flags)
{
	struct log_data *log_ptr;
	char next_color[COLOR_SIZE];

	push_call("logit(%p,%p,%p,%d)",ses,txt,file,flags);

	int is_session_log = 0;
	for (log_ptr = ses->log; log_ptr; log_ptr = log_ptr->next) {
		if (file == log_ptr->file && file != NULL) {
			is_session_log = 1;
			break;
		}
	}
	
	if (file == (FILE *)1) is_session_log = 1;

	if (!is_session_log && file != NULL) {
		write_single_log(ses, ses->log, txt, file, flags);
	} else {
		// Calculate the final color state at the end of the line
		get_color_codes(NULL, ses->active_log_color, txt, next_color, GET_ALL);

		for (log_ptr = ses->log; log_ptr; log_ptr = log_ptr->next) {
			if (!log_ptr->file || log_ptr->file == (FILE *)1) continue;

			if (log_ptr->color_target && *log_ptr->color_target) {
				char norm_target[COLOR_SIZE];
				get_color_codes(NULL, "", log_ptr->color_target, norm_target, GET_ALL);

				int is_match = 0;
				char current_state[COLOR_SIZE];
				strcpy(current_state, ses->active_log_color);

				char *pti = txt;
				while (*pti) {
					int skip = skip_vt102_codes(pti);
					if (skip) {
						// State change detected. Evaluate the color state up to this point.
						pti += skip;
						char temp_char = *pti;
						*pti = '\0';
						get_color_codes(NULL, ses->active_log_color, txt, current_state, GET_ALL);
						*pti = temp_char;
					} else {
						// Printable character detected. Is the terminal currently in our target state?
						if (strcmp(current_state, norm_target) == 0) {
							is_match = 1;
							break;
						}
						pti++;
					}
				}
				
				// Fallback: If the line ended in the target state but had no printable text (just formatting)
				if (!is_match && strcmp(current_state, norm_target) == 0) {
					is_match = 1;
				}

				if (!is_match) continue;
			}

			write_single_log(ses, log_ptr, txt, log_ptr->file, flags);
		}
		
		// Commit the evaluated state to memory for the next incoming line
		strcpy(ses->active_log_color, next_color);
	}

	pop_call();
	return;
}

void logheader(struct session *ses, FILE *file, int flags)
{
	push_call("logheader(%p,%p,%d)",ses,file,flags);

	if (HAS_BIT(flags, LOG_FLAG_APPEND))
	{
		if (HAS_BIT(flags, LOG_FLAG_HTML))
		{
			fseek(file, 0, SEEK_END);

			if (ftell(file) == 0)
			{
				write_html_header(ses, file);
			}
		}
	}
	else if (HAS_BIT(flags, LOG_FLAG_OVERWRITE) && HAS_BIT(flags, LOG_FLAG_HTML))
	{
		if (HAS_BIT(flags, LOG_FLAG_HTML))
		{
			write_html_header(ses, file);
		}
	}
	pop_call();
	return;
}

char *get_charset_html(struct session *ses)
{
	int index;

	for (index = 0 ; *charset_table[index].name ; index++)
	{
		if (ses->charset == charset_table[index].flags)
		{
			return charset_table[index].html;
		}
	}
	return "";
}

void write_html_header(struct session *ses, FILE *fp)
{
	char header[BUFFER_SIZE];

		sprintf(header,
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<meta http-equiv='content-type' content='text/html; charset=%s'>\n"
		"<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
		"<style type='text/css'>\n"
		"body {font-family:Consolas;font-size:12pt}\n"
		"a {text-decoration:none}\n"
		"a:link {color:#06b}\n"
		"a:visited {color:#6b0}\n"
		"a:hover {text-decoration:underline}\n"
		"a:active {color:#b06}\n"
		"</style>\n"
		"<body bgcolor='#000000'>\n"
		"</head>\n"
		"<pre>\n"
		"<span style='background-color:#000'><span style='color:#FFF'>\n",
		get_charset_html(ses));

	fputs(header, fp);
}


void vt102_to_html(struct session *ses, char *txt, char *out)
{
	char tmp[BUFFER_SIZE], *pti, *pto;
	char xtc[6]  = { '0', '6', '8', 'B', 'D', 'F' };
	char *ans[16] = { "000", "A00", "0A0", "AA0", "00A", "A0A", "0AA", "AAA", "555", "F55", "5F5", "FF5", "55F", "F5F", "5FF", "FFF" };

	int vtc, fgc, bgc, cnt;
	int rgb[6] = { 0, 0, 0, 0, 0, 0 };

	vtc = ses->vtc;
	fgc = ses->fgc;
	bgc = ses->bgc;

	pti = txt;
	pto = out;

	while (*pti)
	{
		while (skip_vt102_codes_non_graph(pti))
		{
			pti += skip_vt102_codes_non_graph(pti);
		}

		switch (*pti)
		{
			case 27:
				pti += 2;

				for (cnt = 0 ; pti[cnt] ; cnt++)
				{
					tmp[cnt] = pti[cnt];

					if (pti[cnt] == ';' || pti[cnt] == 'm')
					{
						tmp[cnt] = 0;

						cnt = -1;
						pti += 1 + strlen(tmp);

						if (HAS_BIT(vtc, COL_XTF_R))
						{
							fgc = URANGE(0, atoi(tmp), 255);
							DEL_BIT(vtc, COL_XTF_R);
							SET_BIT(vtc, COL_XTF);
						}
						else if (HAS_BIT(vtc, COL_XTB_R))
						{
							bgc = URANGE(0, atoi(tmp), 255);
							DEL_BIT(vtc, COL_XTB_R);
							SET_BIT(vtc, COL_XTB);
						}
						else if (HAS_BIT(vtc, COL_TCF_R))
						{
							if (rgb[0] == 256)
							{
								rgb[0] = URANGE(0, atoi(tmp), 255);
							}
							else if (rgb[1] == 256)
							{
								rgb[1] = URANGE(0, atoi(tmp), 255);
							}
							else if (rgb[2] == 256)
							{
								rgb[2] = URANGE(0, atoi(tmp), 255);

								fgc = rgb[0] * 256 * 256 + rgb[1] * 256 + rgb[2];

								DEL_BIT(vtc, COL_TCF_R);
								SET_BIT(vtc, COL_TCF);
							}
						}
						else if (HAS_BIT(vtc, COL_TCB_R))
						{
							if (rgb[3] == 256)
							{
								rgb[3] = URANGE(0, atoi(tmp), 255);
							}
							else if (rgb[4] == 256)
							{
								rgb[4] = URANGE(0, atoi(tmp), 255);
							}
							else if (rgb[5] == 256)
							{
								rgb[5] = URANGE(0, atoi(tmp), 255);

								bgc = rgb[3] * 256 * 256 + rgb[4] * 256 + rgb[5];

								DEL_BIT(vtc, COL_TCB_R);
								SET_BIT(vtc, COL_TCB);
							}
						}
						else
						{
							switch (atoi(tmp))
							{
								case 0:
									vtc = 0;
									fgc = 7;
									bgc = 0;
									break;
								case 1:
									SET_BIT(vtc, COL_BLD);
									break;
								case 2:
									if (HAS_BIT(vtc, COL_TCF_2))
									{
										DEL_BIT(vtc, COL_XTF_5|COL_TCF_2);
										SET_BIT(vtc, COL_TCF_R);
										rgb[0] = 256; rgb[1] = 256; rgb[2] = 256;
									}
									else if (HAS_BIT(vtc, COL_TCB_2))
									{
										DEL_BIT(vtc, COL_XTB_5|COL_TCF_2);
										SET_BIT(vtc, COL_TCB_R);
										rgb[3] = 256; rgb[4] = 256; rgb[5] = 256;
									}
									else
									{
										DEL_BIT(vtc, COL_BLD);
									}
									break;
								case 5:
									if (HAS_BIT(vtc, COL_XTF_5))
									{
										DEL_BIT(vtc, COL_XTF_5|COL_TCF_2);
										SET_BIT(vtc, COL_XTF_R);
									}
									else if (HAS_BIT(vtc, COL_XTB_5))
									{
										DEL_BIT(vtc, COL_XTB_5|COL_TCF_2);
										SET_BIT(vtc, COL_XTB_R);
									}
									break;
								case 7:
									SET_BIT(vtc, COL_REV);
									break;
								case 21:
								case 22:
									DEL_BIT(vtc, COL_BLD);
									break;
								case 27:
									DEL_BIT(vtc, COL_REV);
									break;
								case 38:
								case 39:
									SET_BIT(vtc, COL_XTF_5|COL_TCF_2);
									fgc = 7;
									break;
								case 48:
								case 49:
									SET_BIT(vtc, COL_XTB_5|COL_TCB_2);
									bgc = 0;
									break;

								default:
									switch (atoi(tmp) / 10)
									{
										case 3:
										case 9:
											DEL_BIT(vtc, COL_XTF|COL_TCF);
											break;
										case 4:
										case 10:
											DEL_BIT(vtc, COL_XTB|COL_TCB);
											break;
									}
									if (atoi(tmp) / 10 == 4)
									{
										bgc = atoi(tmp) % 10;
									}
									else if (atoi(tmp) / 10 == 10)
									{
										bgc = atoi(tmp) % 10;
									}
									else if (atoi(tmp) / 10 == 3)
									{
										fgc = atoi(tmp) % 10;
									}
									else if (atoi(tmp) / 10 == 9)
									{
										SET_BIT(vtc, COL_BLD);

										fgc = atoi(tmp) % 10;
									}
									break;
							}
						}
					}

					if (pti[-1] == 'm')
					{
						break;
					}
				}

				if (!HAS_BIT(vtc, COL_REV) && HAS_BIT(ses->vtc, COL_REV))
				{
					cnt = fgc;
					fgc = ses->fgc = bgc;
					bgc = ses->bgc = cnt;
				}

				if (bgc != ses->bgc || fgc != ses->fgc || vtc != ses->vtc)
				{
					sprintf(pto, "</span>");
					pto += strlen(pto);

					if (bgc != ses->bgc)
					{
						if (HAS_BIT(vtc, COL_XTB))
						{
							if (bgc < 16)
							{
								sprintf(pto, "</span><span style='background-color: #%s'>", ans[bgc]);
							}
							else if (bgc < 232)
							{
								sprintf(pto, "</span><span style='background-color: #%c%c%c'>", xtc[(bgc-16) / 36], xtc[(bgc-16) % 36 / 6], xtc[(bgc-16) % 6]);
							}
							else
							{
								sprintf(pto, "</span><span style='background-color: rgb(%d,%d,%d)'>", (bgc-232) * 10 + 8, (bgc-232) * 10 + 8, (bgc-232) * 10 + 8);
							}
						}
						else if (HAS_BIT(vtc, COL_TCB))
						{
							sprintf(pto, "</span><span style='background-color:#%02x%02x%02x'>", rgb[3], rgb[4], rgb[5]);
						}
						else
						{
							sprintf(pto, "</span><span style='background-color:#%s'>", ans[bgc]);
						}
						pto += strlen(pto);
					}

					if (HAS_BIT(vtc, COL_XTF))
					{
						if (fgc < 16)
						{
							sprintf(pto, "</span><span style='color:#%s'>", ans[fgc]);
						}
						else if (fgc < 232)
						{
							sprintf(pto, "<span style='color:#%c%c%c'>", xtc[(fgc-16) / 36], xtc[(fgc-16) % 36 / 6], xtc[(fgc-16) % 6]);
						}
						else
						{
							sprintf(pto, "<span style='color:rgb(%d,%d,%d)'>", (fgc-232) * 10 + 8, (fgc-232) * 10 + 8,(fgc-232) * 10 + 8);
						}
					}
					else if (HAS_BIT(vtc, COL_TCF))
					{
						sprintf(pto, "<span style='color:#%02x%02x%02x'>", rgb[0], rgb[1], rgb[2]);
					}
					else
					{
						if (HAS_BIT(vtc, COL_BLD))
						{
							sprintf(pto, "<span style='color:#%s'>", ans[fgc+8]);
						}
						else
						{
							sprintf(pto, "<span style='color:#%s'>", ans[fgc]);
						}
					}
					pto += strlen(pto);
				}

				if (HAS_BIT(vtc, COL_REV) && !HAS_BIT(ses->vtc, COL_REV))
				{
					cnt = fgc;
					fgc = ses->fgc = bgc;
					bgc = ses->bgc = cnt;
				}

				ses->vtc = vtc;
				ses->fgc = fgc;
				ses->bgc = bgc;
				break;

			case  6:
				*pto++ = '&';
				pti++;
				break;

			case 28:
				*pto++ = '<';
				pti++;
				break;

			case 30:
				*pto++ = '>';
				pti++;
				break;

			case '>':
				sprintf(pto, "&gt;");
				pto += strlen(pto);
				pti++;
				break;

			case '<':
				sprintf(pto, "&lt;");
				pto += strlen(pto);
				pti++;
				break;

			case '"':
				sprintf(pto, "&quot;");
				pto += strlen(pto);
				pti++;
				break;

			case '&':
				sprintf(pto, "&amp;");
				pto += strlen(pto);
				pti++;
				break;

			case '$':
				sprintf(pto, "&dollar;");
				pto += strlen(pto);
				pti++;
				break;

			case '\\':
				sprintf(pto, "&bsol;");
				pto += strlen(pto);
				pti++;
				break;
			case 0:
				break;

			default:
				*pto++ = *pti++;
				break;
		}
	}
	*pto = 0;
}
