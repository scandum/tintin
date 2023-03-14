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

struct help_type
{
	char                  * name;
	int                     type;
	char                  * text;
	char                  * also;
};

struct help_type help_table[];

size_t help_size();

int find_help(char *keyword)
{
	unsigned int bot, mid, top;

	bot = 0;
	top = help_size();

	while (top > 1)
	{
		mid = top / 2;

		if (is_abbrev_cmp(keyword, help_table[bot + mid].name) >= 0)
		{
			bot += mid;
		}
		top -= mid;
	}
	if (!is_abbrev_cmp(keyword, help_table[bot].name))
	{
		return bot;
	}
	show_error(gtd->ses, LIST_COMMAND, "find_help: Could not find '%s' in help_table.", keyword);

	return help_size();
}

char *help_related(struct session *ses, int index, int html)
{
	char *arg, *tmp, *link;
	static char buf[INPUT_SIZE];
	int hlp;

	push_call("help_related(%p,%d,%d)",ses,index,html);

	tmp  = str_alloc_stack(0);
	link = str_alloc_stack(0);

	arg  = help_table[index].also;

	buf[0] = 0;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, tmp, GET_ONE);

		if (html == 1)
		{
			sprintf(link, "\\c<a href='%s.php'\\c>%s\\c</a\\c>", tmp, tmp);
		}
		else if (html == 2)
		{
			sprintf(link, "\\c<a href='#%s'\\c>%s\\c</a\\c>", capitalize(tmp), tmp);
		}
		else if (html == 3)
		{
			hlp = find_help(tmp);

			if (hlp == help_size())
			{
				printf("error: unknown help entry: %s see also: %s\n", help_table[index].name, tmp);
			}

			if (help_table[hlp].type != TOKEN_TYPE_STRING)
			{
				sprintf(link, "\\c<a href='help.html#%s'\\c>%s\\c</a\\c>", capitalize(tmp), tmp);
			}
			else
			{
				sprintf(link, "\\c<a href='#%s'\\c>%s\\c</a\\c>", capitalize(tmp), tmp);
			}
		}
		else if (HAS_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING))
		{
			sprintf(link, "\e]68;6;;%s\a\e[4m%s\e[24m", tmp, tmp);
		}
		else
		{
			strcpy(link, tmp);
		}

		if (*buf == 0)
		{
			sprintf(buf, "<178>Related<278>: %s", link);
		}
		else
		{
			if (*arg)
			{
				cat_sprintf(buf, ", %s", link);
			}
			else
			{
				cat_sprintf(buf, " and %s.", link);
			}
		}
	}
	pop_call();
	return buf;
}


DO_COMMAND(do_help)
{
	char buf[BUFFER_SIZE], color[COLOR_SIZE], tmp[INPUT_SIZE];
	int cnt, tut, found, rows, cols, size, col, row;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		tintin_header(ses, 0, " %s ", "HELP");

		*buf = 0;

		size = help_size();
		rows = UMAX(1, ses->wrap / 16);
		cols = size / rows + (size % rows > 0);

		for (cnt = col = 0 ; col < cols ; col++)
		{
			if (!HAS_BIT(ses->config_flags, CONFIG_FLAG_SCREENREADER))
			{
				cnt = col;
			}

			for (row = 0 ; row < rows ; row++)
			{
				switch (help_table[cnt].type)
				{
					case TOKEN_TYPE_STATEMENT:
//						strcpy(color, COLOR_STATEMENT);
//						break;
					case TOKEN_TYPE_CONFIG:
//						strcpy(color, COLOR_CONFIG);
//						break;
					case TOKEN_TYPE_COMMAND:
						strcpy(color, COLOR_COMMAND);
						break;
					case TOKEN_TYPE_STRING:
						strcpy(color, COLOR_STRING);
						break;
					default:
						strcpy(color, "");
						break;
				}

				if (HAS_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING))
				{
					cat_sprintf(buf, "\e]68;6;;%s\a\e[4m%s%s\e[24m%.*s", help_table[cnt].name, color, help_table[cnt].name, 16 - (int) strlen(help_table[cnt].name), "                ");
				}
				else
				{
					cat_sprintf(buf, "%s%-16s", color, help_table[cnt].name);
				}
				cnt += HAS_BIT(ses->config_flags, CONFIG_FLAG_SCREENREADER) ? 1 : cols;

				if (row + 1 == rows || cnt >= size)
				{
					print_lines(ses, SUB_COL, "", "<088>%s<088>\n", buf);

					*buf = 0;

					break;
				}
			}
		}

		if (*buf)
		{
			print_lines(ses, SUB_COL, "", "<088>%s<088>\n", buf);
		}
		tintin_header(ses, 0, "");
	}
	else if (!strcasecmp(arg1, "dump"))
	{
		// help

		FILE *logfile = fopen("../docs/help.html", "w");

		script_driver(ses, LIST_COMMAND, "#config {log} {html}");

		if (HAS_BIT(ses->log->mode, LOG_FLAG_HTML))
		{
			write_html_header(ses, logfile);
		}

		*buf = 0;

		command(ses, do_function, "clink {#format result {%%+%%1h} {%%2};#replace result {#} { };#replace result {%%2} {\\c<a href='%%3'\\c>%%2\\c</a\\c>}}");

		command(ses, do_line, "log {../docs/help.html} {<138>        ╭──────────────────────────────────────────────────────────────────────╮}");

		command(ses, do_line, "log {../docs/help.html} {<138>        │@clink{70;Home;index.html}│}");

		command(ses, do_line, "log {../docs/help.html} {<138>        ╰──────────────────────────────────────────────────────────────────────╯\n}");

		fseek(logfile, 0, SEEK_END);

		size = help_size();
		rows = 4;
		cols = size / rows + (size % rows > 0);

		for (cnt = col = 0 ; col < cols ; col++)
		{
			cnt = col;

			for (row = 0 ; row < rows ; row++)
			{
				filename_string(help_table[cnt].name, tmp);

				cat_sprintf(buf, " \\c<a href='#%s'\\c>%-16s\\c</a\\c> ", tmp, help_table[cnt].name);

				cnt += cols;

				if (row + 1 == rows || cnt >= size)
				{
					substitute(ses, buf, buf, SUB_ESC|SUB_COL);

					logit(ses, "        ", logfile, 0);
					logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

					*buf = 0;

					break;
				}
			}
		}
		cat_sprintf(buf, "\n\n");

		substitute(ses, buf, buf, SUB_ESC|SUB_COL);

		logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			filename_string(help_table[cnt].name, tmp);

			sprintf(buf, "\\c<a name='%.100s'\\c>\\c</a\\c>\n", tmp);

			substitute(ses, buf, buf, SUB_ESC|SUB_COL);

			logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

			sprintf(buf, "<138>         %s\n", help_table[cnt].name);

			substitute(ses, buf, buf, SUB_ESC|SUB_COL);

			logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

			substitute(ses, help_table[cnt].text, buf, SUB_COL);

			logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

			if (*help_table[cnt].also)
			{
				substitute(ses, help_related(ses, cnt, 2), buf, SUB_ESC|SUB_COL);

				logit(ses, buf, logfile, LOG_FLAG_LINEFEED);
			}
		}
		fclose(logfile);

		// tutorial

		int tutorial[size];

		logfile = fopen("../docs/tutorial.html", "w");

		if (HAS_BIT(ses->log->mode, LOG_FLAG_HTML))
		{
			write_html_header(ses, logfile);
		}

		command(ses, do_line, "log {../docs/tutorial.html} {<138>        ╭──────────────────────────────────────────────────────────────────────╮}");

		command(ses, do_line, "log {../docs/tutorial.html} {<138>        │@clink{70;Home;index.html}│}");

		command(ses, do_line, "log {../docs/tutorial.html} {<138>        ╰──────────────────────────────────────────────────────────────────────╯\n}");

		fseek(logfile, 0, SEEK_END);

		tut = 2;

		for (cnt = 0 ; cnt < size ; cnt++)
		{
			if (help_table[cnt].type != TOKEN_TYPE_STRING)
			{
				continue;
			}

			if (is_abbrev("INDEX", help_table[cnt].name))
			{
				tutorial[0] = cnt;
			}
			else if (is_abbrev("INTRODUCTION", help_table[cnt].name))
			{
				tutorial[1] = cnt;
			}
			else
			{
				tutorial[tut++] = cnt;
			}
		}
		size = tut;
		rows = 4;
		cols = size / rows + (size % rows > 0);

		*buf = 0;

		for (cnt = col = 0 ; col < cols ; col++)
		{
			cnt = col;

			for (row = 0 ; row < rows ; row++)
			{
				tut = tutorial[cnt];

				filename_string(help_table[tut].name, tmp);

				cat_sprintf(buf, " \\c<a href='#%s'\\c>%-16s\\c</a\\c> ", tmp, help_table[tut].name);

				cnt += cols;

				if (row + 1 == rows || cnt >= size)
				{
					substitute(ses, buf, buf, SUB_ESC|SUB_COL);

					logit(ses, "        ", logfile, 0);
					logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

					*buf = 0;

					break;
				}
			}
		}
		cat_sprintf(buf, "\n\n");

		substitute(ses, buf, buf, SUB_ESC|SUB_COL);

		logit(ses, "        ", logfile, 0);
		logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

		for (cnt = 0 ; cnt < size ; cnt++)
		{
			tut = tutorial[cnt];

			filename_string(help_table[tut].name, tmp);

			sprintf(buf, "\\c<a name='%.100s'\\c>\\c</a\\c>\n", tmp);

			substitute(ses, buf, buf, SUB_ESC|SUB_COL);

			logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

			sprintf(buf, "<138>         %s\n", help_table[tut].name);

			substitute(ses, buf, buf, SUB_ESC|SUB_COL);

			logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

			substitute(ses, help_table[tut].text, buf, SUB_COL);

			logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

			if (*help_table[tut].also)
			{
				substitute(ses, help_related(ses, tut, 3), buf, SUB_ESC|SUB_COL);

				logit(ses, buf, logfile, LOG_FLAG_LINEFEED);
			}
		}
		fclose(logfile);
	}
	else if (!strcasecmp(arg1, "dump.php"))
	{
		FILE *logfile;

		script_driver(ses, LIST_COMMAND, "#config {log} {html}");

		*buf = 0;

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			filename_string(help_table[cnt].name, arg1);

			lowerstring(arg1);

			sprintf(buf, "../../manual/%s.php", arg1);

			logfile = fopen(buf, "w");

			fprintf(logfile, "<?php\n\tinclude 'manual.php';\n\n\tshow_head(\"%s.php\");\n\n\tshow_example(\"\n", arg1);

			if (*help_table[cnt].also)
			{
				sprintf(buf, "<128>         %s\n", help_table[cnt].name);

				substitute(ses, buf, buf, SUB_ESC|SUB_COL);

				logit(ses, buf, logfile, LOG_FLAG_LINEFEED);
			}
			substitute(ses, help_table[cnt].text, buf, SUB_COL);

			logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

			if (*help_table[cnt].also)
			{
				substitute(ses, help_related(ses, cnt, 1), buf, SUB_ESC|SUB_COL);

				logit(ses, buf, logfile, LOG_FLAG_LINEFEED);
			}
			fprintf(logfile, "\n\t\");\n\n");

			fprintf(logfile, "\tshow_tail();\n?>\n");

			fclose(logfile);
		}
	}
	else
	{
		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			if (is_abbrev(arg1, help_table[cnt].name))
			{
				print_lines(ses, SUB_COL, "", /*COLOR_HELP_DIM,*/ "%s<088>\n", help_table[cnt].text);

				if (*help_table[cnt].also)
				{
					print_lines(ses, SUB_COL, "", "%s<088>\n\n", help_related(ses, cnt, 0));
				}
				return ses;
			}
		}
		found = FALSE;

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			if (match(ses, help_table[cnt].name, arg1, SUB_VAR|SUB_FUN))
			{
				print_lines(ses, SUB_COL, "", /*COLOR_HELP_DIM,*/ "%s<088>\n", help_table[cnt].text);

				if (*help_table[cnt].also)
				{
					print_lines(ses, SUB_COL, "", "%s<088>\n\n", help_related(ses, cnt, 0));
				}
				found = TRUE;
			}
		}

		if (found == FALSE)
		{
			tintin_printf2(ses, "No help found for '%s'", arg1);
		}
	}
	return ses;
}

struct help_type help_table[] =
{
	{
		"ACTION",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #action <178>{<278>message<178>} {<278>commands<178>} {<278>priority<178>}\n"
		"\n"
		"<278>         The #action command can be used to respond with one or several\n"
		"<278>         commands to a specific message send by the server. The %1-%99\n"
		"<278>         variables are substituted from the message and can be used in the\n"
		"<278>         command part of the action.\n"
		"\n"
		"<278>         If the message starts with a ~ color codes must be matched. You can\n"
		"<278>         enable #config {convert meta} on to display meta characters.\n"
		"\n"
		"<278>         For more information on pattern matching see the section on PCRE.\n"
		"\n"
		"<178>Example<278>: #action {%1 tells you '%2'} {tell %1 I'm afk.}\n"
		"\n"
		"<278>         Actions can be triggered by the #show command. If you don't want a\n"
		"<278>         #show to get triggered use: #line ignore #show {text}\n"
		"\n"
		"<278>         Actions are ordered alphabetically and only one action can trigger at\n"
		"<278>         a time. To change the order you can assign a priority, which defaults\n"
		"<278>         to 5, with a lower number indicating a higher priority. The priority\n"
		"<278>         can be a floating point number and should be between 1 and 9.\n"
		"\n"
		"<278>         To remove an action with %* as the message, use #unaction {%%*} or\n"
		"<278>         #unaction {\\%*}. Alternatively you could wrap the action inside a\n"
		"<278>         class, and kill that class when you no longer need the action.\n"
		"\n"
		"<178>Comment<278>: You can remove an action with the #unaction command.\n"
		,
		"pcre gag highlight prompt substitute"
	},
	{
		"ALIAS",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #alias <178>{<278>name<178>} {<278>commands<178>} {<278>priority<178>}\n"
		"\n"
		"<278>         The #alias command can be used to shorten up long or oftenly used\n"
		"<278>         commands. The %1-99 variables are substituted from the arguments when\n"
		"<278>         using an alias and represent the 1st till 99th word which can be used\n"
		"<278>         in the commands part of the alias. If %0 is used it will contain all\n"
		"<278>         arguments. The priority part is optional and determines the priority\n"
		"<278>         of the alias, it defaults to 5.\n"
		"\n"
		"<278>         If no % variable is used in the commands section any argument will be\n"
		"<278>         appended to the end as if %0 was used. This feature might be removed\n"
		"<278>         in the future, and shouldn't be used.\n"
		"\n"
		"<178>Example<278>: #alias {k} {kill %1;kick}\n"
		"\n"
		"<278>         Typing 'k orc' would result in attacking the orc followed by a kick.\n"
		"\n"
		"<278>         You can create multi-word aliases by using variables in the name\n"
		"<278>         section.\n"
		"\n"
		"<178>Example<278>: #alias {k %1 with %2} {draw %2;attack %1;slash %1 with %2;\n"
		"<278>           kick at %2;strike %1 with %2}\n"
		"\n"
		"<278>         Using the above alias you could type k blue smurf with battle axe\n"
		"\n"
		"<278>         To have an alias that matches all user input, use %* as the name.\n"
		"\n"
		"<178>Example<278>: #alias {%*} {#show You wrote: %0}\n"
		"\n"
		"<278>         Aliases are ordered alphabetically and only one alias can trigger at\n"
		"<278>         a time. To change the order you can assign a priority, which defaults\n"
		"<278>         to 5, with a lower number indicating a higher priority. The priority\n"
		"<278>         can be a floating point number.\n"
		"\n"
		"<278>         To remove an alias with %* as the name, use #unalias {%%*} or #unalias\n"
		"<278>         {\\%*}. Alternatively you can wrap the alias inside a class, and kill\n"
		"<278>         that class when you no longer need the alias.\n"
		"\n"
		"<278>         For more information on pattern matching see the section on PCRE.\n"
		"\n"
		"<178>Comment<278>: You can remove an alias with the #unalias command.\n"
		,
		"cursor history keypad macro speedwalk tab"
	},
	{
		"ALL",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #all <178>{<278>string<178>}\n"
		"\n"
		"<278>         If you have multiple sessions in one terminal you can use #all to\n"
		"<278>         execute the command with all sessions, excluding the startup session.\n"
		"\n"
		"<178>Example<278>: #all quit\n"
		"\n"
		"<278>         Sends 'quit' to all sessions.\n"
		,
		"port run session sessionname snoop ssl zap"
	},
	{
		"BELL",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #bell <178>{<278>flash<178>|<278>focus<178>|<278>margin<178>|<278>ring<178>|<278>volume<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         The #bell command without an argument will ring the terminal bell.\n"
		"\n"
		"<178>Example<278>: #action {Bubba tells you} {#bell}\n"
		"\n"
		"<278>         If you aren't watching the screen this could be useful if you don't\n"
		"<278>         want to miss out on a conversation with Bubba. Alternatively you can\n"
		"<278>         use #system to play a sound file.\n"
		"\n"
		"<278>         Some terminals will allow you to use VT100 Operating System Commands\n"
		"<278>         to change the terminal's bell behavior which can be used to flash the\n"
		"<278>         taskbar icon and or focus the window on receival of a bell.\n"
		"\n"
		"<178>Example<278>: #action {Bubba tells you} {#screen save title;#screen set title Tell!;\n"
		"<278>           #bell ring;#delay 10 #screen load title}\n"
		"\n"
		"<278>         The above example will save your window title, change the title to\n"
		"<278>         'Tell!', ring the bell, next reset the window title after 10 seconds.\n"
		"\n"
		"<278>         It's possible to set the terminal to pop to the foreground upon\n"
		"<278>         ringing of the alarm bell.\n"
		"\n"
		"<178>Example<278>: #bell focus on;#bell ring;#bell focus off\n"
		"\n"
		"<278>         It's possible to adjust the alarm bell volume on some terminals.\n"
		"\n"
		"<178>Example<278>: #loop {1} {8} {cnt} {#line substitute variables\n"
		"<278>           #delay {$cnt} {#show Volume $cnt: #bell volume $cnt;#bell}\n"
		,
		"screen"
	},
	{
		"BREAK",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #break\n"
		"\n"
		"<278>         The break command can be used inside the #else, #elseif, #if, #foreach,\n"
		"<278>         #loop, #parse, #switch, and #while statements. When #break is found,\n"
		"<278>         tintin will stop executing the statement it is currently in and move on\n"
		"<278>         to the next.\n"
		"\n"
		"<178>Example<278>: #while {1} {#math cnt $cnt + 1;#if {$cnt == 20} {#break}}\n",
		
		"statements"
	},
	{
		"BUFFER",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #buffer <178>{<278>option<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         The buffer command has various options to manipulate your scrollback\n"
		"<278>         buffer.\n"
		"\n"
		"<278>         The size of the scrollback buffer can be configured using #config\n"
		"<278>         buffer_size <size>. The size must be either 100, 1000, 10000, 100000\n"
		"<278>         or 1000000 lines.\n"
		"\n"
		"<278>         While scrolling through the scrollback buffer incoming text is not\n"
		"<278>         displayed, this can be disabled using #config scroll_lock off. The\n"
		"<278>         scroll lock is automatically disabled when manual input is received,\n"
		"<278>         subsequently #buffer up and down only work properly when used in a\n"
		"<278>         macro or mouse event.\n"
		"\n"
		"<278>         <178>#buffer {clear} {[lower bound]} {[upper bound]}\n"
		"\n"
		"<278>         Without an argument this will clear the entire scrollback buffer.\n"
		"<278>         Otherwise it will clear the given range.\n"
		"\n"
		"<278>         Positive numbers are measured from the start of the scrollback buffer,\n"
		"<278>         negative numbers from the end.\n"
		"\n"
		"<278>         <178>#buffer {down} [lines]\n"
		"\n"
		"<278>         Moves your scrollback buffer down one page and displays the page. If\n"
		"<278>         a line number is provided it will scroll down the given number of\n"
		"<278>         lines.\n"
		"\n"
		"<278>         <178>#buffer {end}\n"
		"\n"
		"<278>         Moves you to the end of your scrollback buffer and displays the page.\n"
		"<278>         Disables scroll lock mode. Most useful when used in a #macro.\n"
		"\n"
		"<278>         <178>#buffer {find} {[number]} {<string>} {[variable]}\n"
		"\n"
		"<278>         Moves the buffer to the given string which can contain a regular\n"
		"<278>         expression. Optionally you can provide the number of matches to skip,\n"
		"<278>         allowing you to jump further back in the buffer.\n"
		"\n"
		"<278>         A positive number searches from the start of the buffer, a negative\n"
		"<278>         number from the end. If you provide a variable the location will be\n"
		"<278>         stored and no jump takes place.\n"
		"\n"
		"<278>         <178>#buffer {get} {<variable>} {<lower bound>} {[upper bound]}\n"
		"\n"
		"<278>         Allows you to store one or several lines from your scrollback buffer\n"
		"<278>         (including color codes) into a variable. The lower and upper bound\n"
		"<278>         must be between 1 and the size of the buffer. If the upper bound is\n"
		"<278>         omitted the given line is stored as a standard variable. If an upper\n"
		"<278>         bound is given the lines between the two bounds are stored as a list.\n"
		"\n"
		"<278>         Positive numbers are measured from the start of the scrollback buffer,\n"
		"<278>         negative numbers from the end.\n"
		"\n"
		"<278>         <178>#buffer {home}\n"
		"\n"
		"<278>         Moves you to the top of your scrollback buffer and displays the page.\n"
		"<278>         Enables scroll lock mode. Most useful when used in a #macro.\n"
		"\n"
		"<278>         <178>#buffer {info} {[save]} {[variable]}\n"
		"\n"
		"<278>         Display buffer info, optionally save the data to a variable.\n"
		"\n"
		"<278>         <178>#buffer {jump} {<location>}\n"
		"\n"
		"<278>         Moves the buffer to the given location. A positive number jumps from\n"
		"<278>         the start of the buffer, a negative number from the end.\n"
		"\n"
		"<278>         <178>#buffer {lock} {on|off}\n"
		"\n"
		"<278>         Toggles the lock on the scrollback buffer. When locked, newly incoming\n"
		"<278>         text won't be displayed, any command will disable the lock, though\n"
		"<278>         several buffer commands will re-enable the lock. When unlocking it'll\n"
		"<278>         move you to the end of your scrollback buffer and display the page.\n"
		"\n"
		"<278>         <178>#buffer {refresh}\n"
		"\n"
		"<278>         Marks the buffer as needing to be refreshed, only useful while in\n"
		"<278>         vertical split mode.\n"
		"\n"
		"<278>         <178>#buffer {up} [lines]\n"
		"\n"
		"<278>         Moves your scrollback buffer up one page and displays the page.\n"
		"<278>         Enables scroll lock mode. Most useful when used in a #macro. You\n"
		"<278>         can use #buffer {up} {1} to move the scrollback buffer up 1 line.\n"
		"\n"
		"<278>         <178>#buffer {write} {<filename>}\n"
		"\n"
		"<278>         Writes the scrollback buffer to the given file.\n"
		"\n"
		"<178>Example<278>: #macro {\\e[F} {#buffer end}\n"
		,
		"echo grep macro showme screen"
	},
	{
		"BUTTON",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #button <178>{<278>square<178>} {<278>commands<178>} {<278>priority<178>}\n"
		"\n"
		"<278>         The #button command can be used to respond with one or several\n"
		"<278>         commands to a mouse click received within the specified square.\n"
		"<278>         The click coordinates are stored in %0-%3 and can be used in the\n"
		"<278>         command part of the button.\n"
		"\n"
		"<278>         The square part should exists of two coordinates defining the\n"
		"<278>         upper left and bottom right corner using row, col, row, col syntax.\n"
		"<278>         The square arguments should be separated by spaces, semi-colons or\n"
		"<278>         braces.\n"
		"\n"
		"<278>         By default the button is set to respond to a mouse button press, to\n"
		"<278>         respond to other button presses you must add a 5th argument to the\n"
		"<278>         square that defines the button press type. You can enable #info\n"
		"<278>         button on to see button events and their type as they happen.\n"
		"\n"
		"<278>         The priority part is optional and determines the priority of the\n"
		"<278>         button, it defaults to 5.\n"
		"\n"
		"<278>         You must enable #config {mouse tracking} on for buttons to work.\n"
		"\n"
		"<278>         This command draws no visible button, you'll have to do so separately\n"
		"<278>         if needed.\n"
		"\n"
		"<178>Example<278>: #button {1;1;2;2} {#show You clicked the upper left corner.}\n"
		"\n"
		"<278>         Buttons are ordered alphabetically and only one button can trigger at\n"
		"<278>         a time. To change the order you can assign a priority, which defaults\n"
		"<278>         to 5, with a lower number indicating a higher priority. The priority\n"
		"<278>         can be a floating point number.\n"
		"\n"
		"<178>Comment<278>: To see button clicks trigger use #info button on.\n"
		"\n"
		"<178>Comment<278>: You can remove a button with the #unbutton command.\n"
		,
		"delay event ticker"
	},
	{
		"CASE",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #case <178>{<278>conditional<178>} {<278>arguments<178>}\n"
		"\n"
		"<278>         The case command must be used within the #switch command. When the\n"
		"<278>         conditional argument of the case command matches the conditional\n"
		"<278>         argument of the switch command the body of the case is executed.\n"
		"\n"
		"<278>         When comparing strings both the switch and case arguments must be\n"
		"<278>         surrounded in quotes.\n"
		"\n"
		"<178>Example<278>:\n"
		"\n"
		"<278>         #function {reverse_direction}\n"
		"<278>         {\n"
		"<278>             #switch {\"%1\"}\n"
		"<278>             {\n"
		"<278>                 #case {\"north\"} {#return south};\n"
		"<278>                 #case {\"east\"}  {#return west};\n"
		"<278>                 #case {\"south\"} {#return north};\n"
		"<278>                 #case {\"west\"}  {#return east};\n"
		"<278>                 #case {\"up\"}    {#return down};\n"
		"<278>                 #case {\"down\"}  {#return up}\n"
		"<278>             }\n"
		"<278>         }\n"
		"\n"
		"<278>         This function returns the reverse direction. @reverse_direction{north}\n"
		"<278>         would return south.\n"
		,
		"default statements switch"
	},
	{
		"CAT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #cat <178>{<278>variable<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         The cat command will concatenate the argument to the given variable.\n"
		,
		"format function local math replace script variable"
	},

	{
		"CHARACTERS",
		TOKEN_TYPE_STRING,
		"\n"
		"<278>         The following special characters are defined:\n"
		"\n"

		"<178>#        <278>The hashtag is the default character for starting a command and is\n"
		"<278>         subsequently known as the command character or tintin character.\n"
		"<278>         When loading a command file the command character is set to the\n"
		"<278>         first character in the file. The character can also be redefined\n"
		"<278>         using #config.\n"
		"\n"
		"<178>;        <278>The semi-colon is used as the command separator and can be used to\n"
		"<278>         separate two commands. Multiple commands can be strung together as\n"
		"<278>         well. Trailing semi-colons are ignored when reading a script file\n"
		"<278>         as this is a common error.\n"
		"\n"
		"<178>{ }      <278>Curly brackets aka braces are used for separating multi word command\n"
		"<278>         arguments, nesting commands, and nesting variables. Braces cannot\n"
		"<278>         easily be escaped and must always be used in pairs.\n"
		"\n"
		"<178>\" \"      <278>Quote characters are used for strings in the #math, #if, #switch,\n"
		"<278>         and #case commands. It is however suggested to use a set of braces\n"
		"<278>         { } to define strings instead, particularly when checking strings\n"
		"<278>         that may contain quotes.\n"
		"\n"
		"<178>!        <278>The exclamation sign is used to repeat commands, see #help history.\n"
		"<278>         The character can be redefined using #config.\n"
		"\n"
		"<178>\\        <278>An input line starting with a backslash is sent verbatim if you are\n"
		"<278>         connected to a server. This character can be configured with\n"
		"<278>         #config, and is itself sent verbatim when the verbatim config mode\n"
		"<278>         is enabled.\n"
		,
		"colors escape_codes function mathematics pcre variable"
	},
	{
		"CHAT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #chat <178>{<278>option<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         The #chat command is used to create peer to peer connections to other\n"
		"<278>         clients, typically for the purpose of chatting and sending files.\n"
		"<278>         This is a decentralized chat system, meaning you have to exchange ip\n"
		"<278>         addresses and port numbers with other users in order to connect to\n"
		"<278>         them.\n"
		"\n"
		"<278>         <178>#chat {init} {port}\n"
		"<278>           #chat initialize launches your chat server. The port number is\n"
		"<278>           optional, and by default 4050 is used as your port. After using\n"
		"<278>           this command other people can connect to your chat server using\n"
		"<278>           your ip address and port number, and in turn you can connect to\n"
		"<278>           other people.\n"
		"<278>         <178>#chat {name} {name}\n"
		"<278>           By default your name is set to TinTin, but most servers will\n"
		"<278>           reject you if there is already someone with the name TinTin\n"
		"<278>           connected, so one of the first things you'd want to do is\n"
		"<278>           change your chat name. Your name can include color codes. Some\n"
		"<278>           names aren't accepted by tt++ chat servers, like the name 'all'\n"
		"<278>           and names longer than 20 characters.\n"
		"<278>         <178>#chat {message} {buddy|all} {text}\n"
		"<278>           This is the main command used for communication. If you use\n"
		"<278>           #chat message all, the message is marked as public and send to\n"
		"<278>           everyone you are connected to.\n"
		"<278>         <178>#chat {accept} {buddy} {boost}\n"
		"<278>           Accept a file transfer from a buddy. The boost is optional and\n"
		"<278>           must be a value between 1 and 1000.\n"
		"<278>         <178>#chat {call}       {address} {port}\n"
		"<278>           #chat call is used to connect to another chat server. If you\n"
		"<278>           omit the port argument the default port (4050) is used.\n"
		"<278>         #chat {cancel}     {buddy}            Cancel a file transfer\n"
		"<278>         #chat {color}      {color names}      Set the default color\n"
		"<278>         #chat {decline}    {buddy}            Decline a file transfer\n"
		"<278>         #chat {dnd}                           Decline new connections\n"
		"<278>         #chat {download}   {directory}        Set your download directory\n"
		"<278>         #chat {emote}      {buddy|all} {text} Send an emote message\n"
		"<278>         #chat {forward}    {buddy}            Forward all chat messages\n"
		"<278>         #chat {forwardall} {buddy}            Forward all session output\n"
		"<278>         #chat {filestat}   {buddy}            Show file transfer data\n"
		"<278>         #chat {group}      {buddy} {name}     Assign a chat group\n"
		"<278>         #chat {ignore}     {buddy}            Ignores someone\n"
		"<278>         #chat {info}                          Displays your info\n"
		"<278>         #chat {ip}         {address}          Changes your IP address\n"
		"<278>         #chat {paste}      {buddy|all} {text} Pastes a block of text\n"
		"<278>         #chat {peek}       {buddy}            Show one's public connections\n"
		"<278>         #chat {ping}       {buddy}            Display response time\n"
		"<278>         #chat {private}    {buddy|all}        Make a connection private\n"
		"<278>         #chat {public}     {buddy|all}        Make a connection public\n"
		"<278>         #chat {reply}      {text}             Reply to last private message\n"
		"<278>         #chat {request}    {buddy}            Request one's public connections\n"
		"<278>         #chat {send}       {buddy|all} {text} Sends a raw data string\n"
		"<278>         #chat {sendfile}   {buddy} {filename} Start a file transfer\n"
		"<278>         #chat {serve}      {buddy}            Forward all public chat messages\n"
		"<278>         #chat {uninitialize}                  Uninitialize the chat port.\n"
		"<278>         <178>#chat {who}\n"
		"<278>           #chat who shows all people you are connected to. The first\n"
		"<278>           column shows a reference number for the connection, which can be\n"
		"<278>           used instead of the connection's name when sending someone a message\n"
		"<278>           The second column shows the connection's name. The third column\n"
		"<278>           shows flags set for the connection, (P)rivate, (I)gnore, (S)erve,\n"
		"<278>           (F)orward to user, and (f)orward from user. The next columns show\n"
		"<278>           ip, port, and client name.\n"
		"<278>         <178>#chat {zap}        {buddy}            Close a connection\n",
		
		"port"
	},
	{
		"CLASS",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #class <178>{<278>name<178>} {<278>option<178>} {<278>arg<178>}\n"
		"\n"
		"<278>         The class command is primarily used to assign groups of triggers and\n"
		"<278>         variables a label so they can be easily removed.\n"
		"\n"
		"<278>         <178>#class {<name>} {assign} {<argument>}\n"
		"<278>           Will open the class, execute argument, and close afterwards.\n"
		"<278>         <178>#class {<name>} {clear}\n"
		"<278>           Will delete all triggers associated with the given class.\n"
		"<278>         <178>#class {<name>} {close}\n"
		"<278>           Close the given class, opening the last open class, if any.\n"
		"<278>         <178>#class {<name>} {kill}\n"
		"<278>           Will clear, close, and remove the class.\n"
		"<278>         <178>#class {<name>} {list}\n"
		"<278>           List all triggers associated with the given class.\n"
		"<278>         <178>#class {<name>} {load}\n"
		"<278>           Will load the saved copy of the class from memory.\n"
		"<278>         <178>#class {<name>} {open}\n"
		"<278>           Open a class, closing a previously opened class. All triggers\n"
		"<278>           added afterwards are assigned to this class.\n"
		"<278>         <178>#class {<name>} {read} {<filename>\n"
		"<278>           Will open the class, read the file, and close afterwards.\n"
		"<278>         <178>#class {<name>} {save}\n"
		"<278>           Will save all triggers of the given class to memory.\n"
		"<278>         <178>#class {<name>} {size} {<variable>}\n"
		"<278>           Will store the size of the class in a variable.\n"
		"<278>         <178>#class {<name>} {write} {<filename>}\n"
		"<278>           Will write all triggers of the given class to file.\n"
		"\n"
		"<278>         Keep in mind that you need to use #class save before using\n"
		"<278>         #class clear and #class load\n"
		"\n"
		"<178>Example<278>: #class rich kill;#class rich read poor.tin\n"
		"<278>         Deletes all triggers of 'rich' class if any. Read 'poor.tin' file,\n"
		"<278>         all triggers loaded will be assigned to the 'rich' class.\n",
		
		"config debug ignore info kill line message"
	},
	{
		"COLORS",
		TOKEN_TYPE_STRING,
		"<178>Syntax<278>:  <<888>xyz>  with x, y, z being parameters\n"
		"\n"
		"<278>         Parameter 'x': VT100 code\n"
		"\n"
		"<278>         0 - Reset all colors and codes to default\n"
		"<278>         1 - Bold\n"
		"<278>         2 - Dim\n"
		"<278>         3 - Italic\n"
		"<278>         4 - Underscore\n"
		"<278>         5 - Blink\n"
		"<278>         7 - Reverse\n"
		"<278>         8 - Skip (use previous code)\n"
		"\n"
		"<278>         Parameter 'y':  Foreground color\n"
		"<278>         Parameter 'z':  Background color\n"
		"\n"
		"<278>         0 - Black                5 - Magenta\n"
		"<278>         1 - Red                  6 - Cyan\n"
		"<278>         2 - Green                7 - White\n"
		"<278>         3 - Yellow               8 - Skip\n"
		"<278>         4 - Blue                 9 - Default\n"
		"\n"
		"<278>         For xterm 256 colors support use <<888>aaa> to <<888>fff> for RGB foreground\n"
		"<278>         colors and <<888>AAA> to <<888>FFF> for RGB background colors. For the grayscale\n"
		"<278>         foreground colors use <<888>g00> to <<888>g23>, for grayscale background colors\n"
		"<278>         use <<888>G00> to <<888>G23>.\n"
		"\n"
		"<278>         The tertiary colors are as follows:\n"
		"\n"
		"<278>         <<888>acf> - Azure            <<888>afc> - Jade\n"
		"<278>         <<888>caf> - Violet           <<888>cfa> - Lime\n"
		"<278>         <<888>fac> - Pink             <<888>fca> - Orange\n"
		"\n"
		"<178>Example<278>: #show <<888>acf>Azure    <<888>afc>Jade     <<888>caf>Violet\n"
		"<178>Example<278>: #show <<888>cfa>Lime     <<888>fac>Pink     <<888>fca>Orange\n"
		"\n"
		"<278>         For 12 bit truecolor use <<888>F000> to <<888>FFFF> for foreground colors and\n"
		"<278>         <<888>B000> to <<888>BFFF> for background colors.\n"
		"\n"
		"<278>         For 24 bit truecolor use <<888>F000000> to <<888>FFFFFFF> for foreground\n"
		"<278>         colors and <<888>B000000> to <<888>BFFFFFF> for background colors.\n"
		"\n"
		"<278>         If the color code exceeds your configured color mode it will be\n"
		"<278>         downgraded to the closest match.\n"
		,
		"characters coordinates escape_codes mathematics pcre"
	},
	{
		"COMMANDS",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #commands <178>{<278>abbreviation<178>}\n"
		"\n"
		"<278>         Shows all commands, or all commands starting with the given\n"
		"<278>         abbreviation.\n"
		,
		"help info statements"
	},
	{
		"CONFIG",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #config <178>{<278>option<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         This allows you to configure various settings, the settings can be\n"
		"<278>         written to file with the #write command.\n"
		"\n"
		"<278>         If you configure the global session (the one you see as you start up\n"
		"<278>         tintin) all sessions started will inherite these settings.\n"
		"\n"
		"<278>         It's advised to make a configuration file to read on startup if you\n"
		"<278>         do not like the default settings.\n"
		"\n"
		"<278>         Use #config without an argument to see your current configuration as\n"
		"<278>         well as a brief explanation of each config option.\n"
		"\n"
		"<278>         The following config options are not listed by default:\n"
		"\n"
		"<278>         #CONFIG {AUTO TAB}      {NUMBER} Buffer lines used for tab completion\n"
		"<278>         #CONFIG {CHILD LOCK}    {ON|OFF} Enable or disable command input.\n"
		"<278>         #CONFIG {CONNECT RETRY} {NUMBER} Seconds to try to connect on failure.\n"
		"<278>         #CONFIG {CONVERT META}  {ON|OFF} Shows color codes and key bindings.\n"
		"<278>         #CONFIG {DEBUG TELNET}  {ON|OFF} Shows telnet negotiations y/n.\n"
		"<278>         #CONFIG {HIBERNATE}     {ON|OFF} Enable or disable low CPU usage mode.\n"
		"<278>         #CONFIG {LOG LEVEL}   {LOW|HIGH} LOW logs server output before triggers.\n"
		"<278>         #CONFIG {INHERITANCE}   {ON|OFF} Session trigger inheritance y/n.\n"
		"<278>         #CONFIG {MCCP}          {ON|OFF} Enable or disable MCCP support.\n"
		"<278>         #CONFIG {RANDOM SEED}   {NUMBER} Seed value used for random numbers.\n"
		"<278>         #CONFIG {TAB WIDTH}     {NUMBER} Number of spaces used for a tab\n"
		"<278>         #CONFIG {TINTIN CHAR}   {SYMBOL} Character used for TinTin++ commands.\n"
		,
		"class line"
	},
	{
		"CONTINUE",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #continue\n"
		"\n"
		"<278>         The continue command can be used inside the #FOREACH, #LOOP, #PARSE,\n"
		"<278>         #WHILE and #SWITCH commands. When #CONTINUE is found, tintin will go\n"
		"<278>         to the end of the command and proceed as normal, which may be to\n"
		"<278>         reiterate the command.\n"
		"\n"
		"<178>Example<278>: #loop 1 10 cnt {#if {$cnt % 2 == 0} {#continue} {say $cnt}}\n",
		
		"break foreach list loop parse repeat return while"
	},
	{
		"COORDINATES",
		TOKEN_TYPE_STRING,
		"\n"
		"<278>         When the 0,0 coordinate is in the upper left corner TinTin++ uses\n"
		"<278>         a y,x / row,col notation, starting at 1,1. Subsequently -1,-1\n"
		"<278>         will indicate the bottom right corner. This type of argument is\n"
		"<278>         used by the #showme command.\n"
		"\n"
		"<278>         When the 0,0 coordinate is in the bottom left corner tintin uses\n"
		"<278>         a standard x,y notation. This type of argument is used by the\n"
		"<278>         #map jump command.\n"
		"\n"
		"<278>         The vast majority of tintin commands use y,x / row,col notation,\n"
		"<278>         primarily because that is the notation used by the VT100 standard\n"
		"<278>         used for terminal emulation.\n"
		"\n"
		"<278>         <128>Squares\n"
		"\n"
		"<278>         A square argument takes 2 coordinates. The first coordinate defines\n"
		"<278>         the upper left corner, the last coordinate defines the bottom\n"
		"<278>         right corner. The upper left corner of the terminal is defined as\n"
		"<278>         1,1 and the bottom right corner as -1,-1. This type of argument is\n"
		"<278>         used by #draw, #button and #map offset.\n"
		"\n"
		"<278>         <128>Panes\n"
		"\n"
		"<278>         A pane argument takes 4 size values, which are: top pane, bottom\n"
		"<278>         pane, left pane, right pane. When a negative value is provided the\n"
		"<278>         size is the maximum size, minus the value. This type of argument\n"
		"<278>         is used by the #split command.\n"
		"\n"
		"<278>         <128>Ranges\n"
		"\n"
		"<278>         A range argument takes 2 values known as the upper bound and lower\n"
		"<278>         bound. The upper bound (first value) defines the start of the\n"
		"<278>         range, the lower bound (second value) the end. The first index of\n"
		"<278>         a range is defined as 1. When a negative value is provides the last\n"
		"<278>         index is defined as -1. This type of argument is used by #buffer\n"
		"<278>         and #variable.\n"
		,
		"characters colors escape_codes mathematics pcre"
	},
	{
		"CR",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #cr\n"
		"\n"
		"<278>         Sends a carriage return to the session.  Useful for aliases that need\n"
		"<278>         extra carriage returns.\n"
		"\n"
		"<278>         This command is obsolete as you can accomplish the same using #send\n"
		"<278>         without an argument or #send {}.\n"
		,
		"forall"
	},
	{
		"CURSOR",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #cursor <178>{<278>option<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         Typing #cursor without an option will show all available cursor\n"
		"<278>         options, their default binding, and an explanation of their function.\n"
		"\n"
		"<278>         The cursor command's primarly goal is adding customizable input editing\n"
		"<278>         with macros. Subsequently many cursor commands only work properly when\n"
		"<278>         used within a macro or event.\n"
		"\n"
		"<278>         <178>#cursor flag\n"
		"\n"
		"<278>         EOL         end of line character(s)\n"
		"<278>         ECHO        local echo\n"
		"<278>         OVERTYPE    overtype mode\n"
		"\n"
		"<278>         <178>#cursor macro\n"
		"\n"
		"<278>         PRESERVE    do not erase the macro from the macro input buffer\n"
		"<278>         RESET       erase the macro input buffer\n"
		"\n"
		"<278>         <178>#cursor tab\n"
		"\n"
		"<278>         CASELESS    makes tab completion caseless\n"
		"<278>         COMPLETE    makes tab completion work while editing\n"
		"\n"
		"<278>         DICTIONARY  performs tab completion on the dictionary\n"
		"<278>         LIST        performs tab completion on the tab completion list\n"
		"<278>         SCROLLBACK  performs tab completion on the scrollback buffer\n"
		"\n"
		"<278>         BACKWARD    specifies tab completion to go backward\n"
		"<278>         FORWARD     specifies tab completion to go forward\n"
		"\n"
		"<278>         Multiple options can/must be specified at once.\n"
		,
		"alias history keypad macro speedwalk tab"
	},
	{
		"DAEMON",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #daemon <178>{<278>attach<178>|<278>detach<178>|<278>kill<178>|<278>list<178>} <178>[<278>name<178>]\n"
		"\n"
		"<278>         #daemon provides functionality similar to that of the screen and tmux\n"
		"<278>         utilities.\n"
		"\n"
		"<278>         <178>#daemon attach [name]\n"
		"<278>           The attach option will try to find a daemonized tintin instance and\n"
		"<278>           take over control. The name argument is optional.\n"
		"\n"
		"<278>         <178>#daemon detach [name]\n"
		"<278>           The detach option will daemonize tintin, turning it into a background\n"
		"<278>           process. The name argument is optional and is useful if you have\n"
		"<278>           several daemonized tt++ instances running so you can keep them apart.\n"
		"\n"
		"<278>         <178>#daemon kill [name]\n"
		"<278>           Kills all daemons or daemons with matching name.\n"
		"\n"
		"<278>         <178>#daemon list [name]\n"
		"<278>           List all daemons or daemons with matching name.\n"
		"\n"
		"<278>         You can launch tintin and attach the first daemonized instance using\n"
		"<278>         tt++ -R. To attach a named instance use tt++ -R<name>.\n",
		
		"script system run"
	},
	{
		"DEBUG",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #debug <178>{<278>listname<178>} {<278>on<178>|<278>off<178>|<278>log<178>}\n"
		"\n"
		"<278>         Toggles a list on or off. With no argument it shows your current\n"
		"<278>         settings, as well as the list names that you can debug.\n"
		"\n"
		"<278>         If you for example set ACTIONS to ON you will get debug information\n"
		"<278>         whenever an action is triggered.\n"
		"\n"
		"<278>         #debug {listname} {log} will silently write debugging information to\n"
		"<278>         the log file, you must be logging in order for this to work.\n"
		"\n"
		"<278>         Not every list has debug support yet.\n",
		
		"class ignore info kill message"
	},
	{
		"DEFAULT",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #default <178>{<278>commands<178>}\n"
		"\n"
		"<278>         The default command can only be used within the switch command. When\n"
		"<278>         the conditional argument of non of the case commands matches the switch\n"
		"<278>         command's conditional statement the default command is executed.\n",
		
		"case default else elseif if switch regexp"
	},
	{
		"DELAY",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #delay <178>{<278>seconds<178>} {<278>command<178>}\n"
		"<178>Command<278>: #delay <178>{<278>name<178>} {<278>command<178>} {<278>seconds<178>}\n"
		"\n"
		"<278>         Delay allows you to have tintin wait the given amount of seconds\n"
		"<278>         before executing the given command.\n"
		"\n"
		"<278>         Nanosecond floating point precision is allowed. Delays will fire in\n"
		"<278>         0.01 second intervals.\n"
		"\n"
		"<278>         Named delays are treated as one-shot tickers, see #help tick.\n"
		"\n"
		"<178>Example<278>: #delay {1} {#show last};#show first\n"
		"<278>         This will print 'first', and 'last' around one second later.\n"
		"\n"
		"<178>Comment<278>: If you want to remove a delay with the #undelay command you can add\n"
		"<278>         a name as the first argument, be aware this changes the syntax. If\n"
		"<278>         the name is a number keep in mind that delays with the same numeric\n"
		"<278>         name will not be overwritten\n",
		
		"event ticker"
	},

	{
		"DRAW",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #draw <178>[<278>line color<178>] <178>[<278>options<178>] <178><<278>type<178>> <<278>square<178>> {<278>text<178>}\n"
		"\n"
		"<278>         The draw commands allows you to draw various types of lines and shapes\n"
		"<278>         on the screen. The types with a brief description are provided when you\n"
		"<278>         type #draw without an argument.\n"
		"\n"
		"<278>         The <square> arguments should exists of two coordinates defining the\n"
		"<278>         upper left and bottom right corner using row, col, row, col syntax.\n"
		"\n"
		"<278>         The square arguments can be negative, in which case the coordinates\n"
		"<278>         are calculated from the opposite side of the screen. In the case the\n"
		"<278>         screen is 80 columns wide using #draw box 1 60 10 70 will be the\n"
		"<278>         equivalent of #draw box 1 -21 10 -11, but with different screen\n"
		"<278>         widths the boxes would be drawn in different places.\n"
		"\n"
		"<278>         You can prefix the option with a color code or color name to color the\n"
		"<278>         lines and shapes.\n"
		"\n"
		"<278>         You can further prefix the option as following:\n"
		"\n"
		"<278>         ASCII       draw in ASCII mode.\n"
		"<278>         BALIGN      bottom align text.\n"
		"<278>         BLANKED     blank the lines and corners.\n"
		"<278>         BOTTOM      draw on the bottom side if possible.\n"
		"<278>         BOXED       draw a box along the square.\n"
		"<278>         BUMPED      precede the draw with an enter.\n"
		"<278>         CALIGN      center text.\n"
		"<278>         CIRCLED     circle the corners.\n"
		"<278>         CONVERT     draw text with meta conversion.\n"
		"<278>         CROSSED     cross the corners.\n"
		"<278>         CURSIVE     draw text with cursive letters.\n"
		"<278>         FAT         draw text with fat letters.\n"
		"<278>         FILLED      fill circles and jewels.\n"
		"<278>         FOREGROUND  draw even if session is not active.\n"
		"<278>         GRID        draw TABLE as a grid.\n"
		"<278>         HORIZONTAL  draw horizontal if possible.\n"
		"<278>         HUGE        draw text in huge letters.\n"
		"<278>         JEWELED     diamond the corners.\n"
		"<278>         JOINTED     draw corners.\n"
		"<278>         LALIGN      left align text.\n"
		"<278>         LEFT        draw on the left side if possible.\n"
		"<278>         NUMBERED    draw numbers instead of lines.\n"
		"<278>         PRUNED      prune the corners.\n"
		"<278>         RALIGN      right align text.\n"
		"<278>         RIGHT       draw on the right side if possible.\n"
		"<278>         ROUNDED     round the corners.\n"
		"<278>         SANSSERIF   draw text with sansserif letters.\n"
		"<278>         SCALED      fit the square to the text size.\n"
		"<278>         SCROLL      draw in the scrolling region.\n"
		"<278>         SHADOWED    shadow HUGE text.\n"
		"<278>         TALIGN      top align text too large to fit.\n"
		"<278>         TEED        tee the corners.\n"
		"<278>         TOP         draw on the top side if possible.\n"
		"<278>         TRACED      trace HUGE text.\n"
		"<278>         TUBED       draw tubes instead of lines.\n"
		"<278>         UALIGN      unwrap and rewrap text.\n"
		"<278>         UNICODE     draw in unicode mode.\n"
		"<278>         VERTICAL    draw vertical if possible.\n"
		"\n"
		"<278>         The following types are available.\n"
		"\n"
		"<278>         [HORIZONTAL] <178>BAR<278> {<MIN>;<MAX>;[COLOR]}\n"
		"<278>          will draw a bar, use two 256 color codes for a color gradient.\n"
		"<278>         [ASCII|UNICODE|HUGE] <178>BOX<278> {[TEXT1]} {[TEXT2]}\n"
		"<278>           will draw a box.\n"
		"<278>         [BOXED|FOREGROUND] <178>BUFFER\n"
		"<278>           will draw the scrollback buffer.\n"
		"<278>         [BLANKED|CIRCLED|CROSSED|JEWELED|ROUNDED|TEED|PRUNED] <178>CORNER\n"
		"<278>           will draw a corner.\n"
		"<278>         [BLANKED|HORIZONTAL|NUMBERED|TUBED|VERTICAL] <178>LINE<278> {[TEXT]}\n"
		"<278>           will draw a line.\n"
		"<278>         [BOXED] <178>MAP\n"
		"<278>           will draw the map\n"
		"<278>         <178>RAIN<278> {<VARIABLE>} {[SPAWN]} {[FADE]} {[LEGEND]}\n"
		"<278>           will draw digital rain.\n"
		"<278>         [JOINTED|TOP|LEFT|BOTTOM|RIGHT] <178>SIDE\n"
		"<278>           will draw one or more sides of a box.\n"
		"<278>         [GRID] <178>TABLE<278> {[LIST1]} {[LIST2]}\n"
		"<278>          will draw a table.\n"
		"<278>         [CURSIVE|FAT|HUGE|SANSSERIF] <178>TILE<278> {[TEXT1]} {[TEXT2]}\n"
		"<278>           will draw a tile\n"
		"\n"
		"<278>         All draw types take an optional text argument as long as a valid\n"
		"<278>         square with enough space has been defined. Text is automatically\n"
		"<278>         word wrapped and text formatting can be customized with the\n"
		"<278>         CALIGN, LALIGN, RALIGN, and UALIGN options.\n"
		"\n"
		"<178>Example<278>: #draw Blue box 1 1 3 20 {Hello world!}\n"
		,
		"buffer echo grep showme"
	},

	{
		"ECHO",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #echo <178>{<278>format<178>} {<278>argument1<178>} {<278>argument2<178>} {<278>etc<178>}\n"
		"\n"
		"<278>         Echo command displays text on the screen with formatting options. See\n"
		"<278>         the help file for the format command for more information.\n"
		"\n"
		"<278>         The echo command does not trigger actions.\n"
		"\n"
		"<278>         As with the #show command you can split the {format} argument up into\n"
		"<278>         two braced arguments, in which case the 2nd argument is the row number.\n"
		"\n"
		"<178>Example<278>: #echo {The current date is %t.} {%Y-%m-%d %H:%M:%S}\n"
		"<278>         #echo {[%38s][%-38s]} {Hello World} {Hello World}\n"
		"<278>         #echo {{this is %s on the top row} {1}} {printed}\n",
		
		"buffer format grep showme"
	},
	{
		"EDIT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #edit <178>{<278>option<178>} <178>[<278>argument<178>]\n"
		"\n"
		"<278>         The edit command can be used to turn the default line editor into a\n"
		"<278>         text editor.\n"
		"\n"
		"<278>         <178>#edit create <arguments>\n"
		"<278>           Create an editor, initialize using the provided arguments.\n"
		"\n"
		"<278>         <178>#edit load <variable>\n"
		"<278>           Create an editor, initialize using the provided list variable.\n"
		"\n"
		"<278>         <178>#edit read <filename>\n"
		"<278>           Create an editor, initialize using the provided file.\n"
		"\n"
		"<278>         <178>#edit resume\n"
		"<278>           Resume editing after a suspension.\n"
		"\n"
		"<278>         <178>#edit save <variable>\n"
		"<278>           Save the editor to the provided variable.\n"
		"\n"
		"<278>         <178>#edit suspend\n"
		"<278>           Suspend editing, similar to pressing enter except that no\n"
		"<278>           events are triggered.\n"
		"\n"
		"<278>         <178>#edit write <filename\n"
		"<278>           Write the editor content to file.\n"
		"\n"
		"<178>Example<278>: #edit create {bli}{bla}{blo}\n",
		
		"cursor macro"
	},
	{
		"EDITING",
		TOKEN_TYPE_STRING,
		"\n"
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"
		"<268>│<178>alt b                    <268>││<178>cursor backward word                        <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>alt f                    <268>││<178>cursor forward word                         <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"

		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"
		"<268>│<178>ctrl a                   <268>││<178>cursor home                                 <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl b                   <268>││<178>cursor backward                             <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl c                   <268>││<178>clear line                                  <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl d                   <268>││<178>delete or exit                              <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl e                   <268>││<178>cursor end                                  <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl f                   <268>││<178>cursor forward                              <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl g                   <268>││<178>                                            <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl h                   <268>││<178>backspace                                   <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl i                   <268>││<178>tab                                         <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl j                   <268>││<178>enter                                       <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl k                   <268>││<178>clear line right                            <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl l                   <268>││<178>redraw input                                <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl m                   <268>││<178>enter                                       <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl n                   <268>││<178>input history next                          <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl o                   <268>││<178>                                            <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl p                   <268>││<178>input history prev                          <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl q                   <268>││<178>                                            <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl r                   <268>││<178>input history search                        <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl s                   <268>││<178>                                            <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl t                   <268>││<178>scroll buffer lock                          <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl u                   <268>││<178>clear line left                             <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl v                   <268>││<178>convert meta characters                     <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl w                   <268>││<178>delete word left                            <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl x                   <268>││<178>                                            <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl y                   <268>││<178>paste                                       <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl z                   <268>││<178>suspend                                     <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"

		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"
		"<268>│<178>arrow left               <268>││<178>cursor left                                 <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>arrow right              <268>││<178>cursor right                                <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>arrow up                 <268>││<178>previous input line                         <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>arrow down               <268>││<178>next input line                             <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"
		"<268>│<178>ctrl arrow left          <268>││<178>cursor left word                            <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl arrow right         <268>││<178>cursor right word                           <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"
		"<268>│<178>backspace                <268>││<178>backspace                                   <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>alt backspace            <268>││<178>clear line left                             <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl backspace           <268>││<178>clear line                                  <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"
		"<268>│<178>delete                   <268>││<178>delete                                      <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl delete              <268>││<178>delete word right                           <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"	
		"<268>│<178>end                      <268>││<178>cursor end                                  <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl end                 <268>││<178>scroll buffer end                           <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"

		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"	
		"<268>│<178>enter                    <268>││<178>enter                                       <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>shift-enter              <268>││<178>soft enter                                  <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"
		"<268>│<178>home                     <268>││<178>cursor home                                 <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>ctrl home                <268>││<178>scroll buffer home                          <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"	
		"<268>│<178>page up                  <268>││<178>scroll buffer up                            <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>page down                <268>││<178>scroll buffer down                          <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"

		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐\n"	
		"<268>│<178>tab                      <268>││<178>complete word forward                       <268>│\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤\n"
		"<268>│<178>shift-tab                <268>││<178>complete word backward                      <268>│\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘\n"
		,
		"cursor edit macro"
	},

	{
		"ELSE",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #else <178>{<278>commands<178>}\n"
		"\n"
		"<278>         The else statement should follow an #IF or #ELSEIF statement and is\n"
		"<278>         only called if the proceeding #IF or #ELSEIF is false.\n"
		"\n"
		"<178>Example<278>: #if {1d2 == 1} {smile};#else {grin}\n",
		
		"case default elseif if switch regexp"
	},
	{
		"ELSEIF",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #elseif <178>{<278>conditional<178>} {<278>commands<178>}\n"
		"\n"
		"<278>         The elseif statement should follow an #IF or #ELSEIF statement and is\n"
		"<278>         only called when the statement is true and the proceeding #IF and\n"
		"<278>         #ELSEIF statements are false.\n"
		"\n"
		"<178>Example<278>: #if {1d3 == 1} {smirk};#elseif {1d2 == 1} {snicker}\n",
		
		"case default else if switch regexp"
	},
	{
		"END",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #end {<message>}\n"
		"\n"
		"<278>         Terminates tintin and return to unix.  On most systems, ctrl-c has\n"
		"<278>         the same result.\n"
		"\n"
		"<278>         The message is optional and is printed before tintin exits. When\n"
		"<278>         using #end {\\} tintin will terminate silently.\n",
		
		"zap"
	},
	{
		"ESCAPE CODES",
		TOKEN_TYPE_STRING,
		"<278>         You may use the escape character \\ for various special characters.\n"
		"\n"
		"<278>         \\a    beep the terminal.\n"
		"<278>         \\c    send a control character, \\ca for ctrl-a.\n"
		"<278>         \\e    start an escape sequence.\n"
		"<278>         \\f    send a form feed.\n"
		"<278>         \\n    send a line feed.\n"
		"<278>         \\r    send a carriage return.\n"
		"<278>         \\t    send a horizontal tab.\n"
		"<278>         \\x    print an 8 bit character using hexadecimal, \\xFF for example.\n"
		"<278>         \\x7B  send the '{' character.\n"
		"<278>         \\x7D  send the '}' character.\n"
		"<278>         \\u    print a 16 bit unicode character, \\uFFFD for example.\n"
		"<278>         \\u{}  print a 8-21 bit unicode character, \\u{2AF21} for example.\n"
		"<278>         \\U    print a 21 bit unicode character, \\U02AF21 for example.\n"
		"<278>         \\v    send a vertical tab\n"
		"\n"
		"<278>         Ending a line with \\ will stop tintin from appending a line feed.\n"
		"<278>         To escape arguments in an alias or action use %%0 %%1 %%2 etc.\n",
		
		"characters colors coordinates mathematics pcre"
	},
	{
		"EVENT",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #event <178>{<278>event type<178>}<278> <178>{<278>commands<178>}\n"
		"\n"
		"<278>         Events allow you to create triggers for predetermined client events.\n"
		"\n"
		"<278>         Use #event without an argument to see a list of possible events with\n"
		"<278>         a brief description. Use #event %* to see the current list of defined\n"
		"<278>         events. Use #info {events} {on} to see events get thrown.\n"
		"\n"
		"<278>         Events, like triggers in general, are case sensitive and event names\n"
		"<278>         must be defined using all upper case letters. Only one event can be\n"
		"<278>         defined for each event type.\n"
		"\n"
		"<278>         To enable mouse events use #config mouse_tracking on, to see mouse\n"
		"<278>         events use #config mouse_tracking info.\n"
		"\n"
		"<278>         <128>CATCH EVENTS\n"
		"\n"
		"<278>         <178>CATCH <EVENT>\n"
		"<278>           Some events can be prefixed with CATCH to interrupt default\n"
		"<278>           behavior.\n"
		"\n"
		"<278>         <128>CLASS EVENTS\n"
		"\n"
		"<278>         <178>CLASS ACTIVATED [CLASS],  CLASS_CLEAR [CLASS],  CLASS CREATED [CLASS],\n"
		"<278>         <178>CLASS DEACTIVATED [CLASS],  CLASS DESTROYED [CLASS],\n"
		"<278>         <178>CLASS_LOAD [CLASS]\n"
		"<278>           %0 class name\n"
		"\n"

		"<278>         <128>GAG EVENTS\n"
		"\n"
		"<278>         <178>GAG <EVENT>\n"
		"<278>           Some events can be prefixed with GAG to gag default system\n"
		"<278>           messages.\n"
		"\n"
		"<278>         <128>INPUT EVENTS\n"
		"\n"
		"<278>         <178>EDIT STARTED, EDIT FINISHED\n"
		"<278>           %0 name  %1 lines %2 size %3 data\n"
		"\n"
		"<278>         <178>HISTORY UPDATE\n"
		"<278>           %0 command\n"
		"\n"
		"<278>         <178>RECEIVED KEYPRESS, PROCESSED KEYPRESS\n"
		"<278>           %0 character  %1 unicode index  %2 edit row  %3 edit column\n"
		"\n"
		"<278>         <178>RECEIVED INPUT [NAME]\n"
		"<278>           %0 raw text\n"
		"\n"
		"<278>         <178>RECEIVED INPUT CHARACTER\n"
		"<278>           %0 character  %1 unicode index  %2 size  %3 width\n"
		"\n"
		"<278>         NO SESSION ACTIVE      %0 raw text %1 size\n"
		"<278>         SEND OUTPUT            %0 raw text %1 size\n"
		"<278>         SENT OUTPUT            %0 raw text %1 size\n"
		"\n"
		"<278>         <128>MAP EVENTS\n"
		"\n"
		"<278>         <178>END OF PATH,  END OF RUN, MAP UPDATED VTMAP\n"
		"<278>           These events have no additional arguments.\n"
		"\n"
		"<278>         <178>MAP CREATE EXIT, MAP DELETE EXIT\n"
		"<278>           %0 vnum  %1 exit name  %2 exit cmd  %3 exit vnum\n"
		"\n"
		"<278>         <178>MAP CREATE ROOM, MAP DELETE ROOM\n"
		"<278>           %0 vnum  %1 name\n"
		"\n"
		"<278>         <178>MAP ENTER MAP, MAP EXIT MAP\n"
		"<278>           %0 vnum\n"
		"\n"
		"<278>         <178>MAP ENTER ROOM [VNUM]\n"
		"<278>           %0 new vnum  %1 old vnum %2 direction\n"
		"\n"
		"<278>         <178>MAP EXIT ROOM [VNUM]\n"
		"<278>           %0 old vnum  %1 new vnum %2 direction\n"
		"\n"
		"<278>         <178>MAP FOLLOW MAP\n"
		"<278>           %0 old vnum  %1 new vnum  %2 exit name\n"
		"\n"
		"<278>         <178>MAP REGION <MOUSE>, MAP ROOM <MOUSE>\n"
		"<278>           %0 row  %1 col  %2 -row  %3 -col  %5 vnum  %6 info\n"
		"\n"
		"<278>         <128>MOUSE EVENTS\n"
		"\n"
		"<278>         <178>DOUBLE-CLICKED <MOUSE> <278>%0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"<278>         <178>LONG-CLICKED <MOUSE>   <278>%0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"<278>         <178>MOVED <MOUSE>          <278>%0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"<278>         <178>PRESSED <MOUSE>        <278>%0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"<278>         <178>SHORT-CLICKED <MOUSE>  <278>%0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"<278>         <178>RELEASED <MOUSE>       <278>%0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"<278>         <178>SCROLLED <MOUSE>       <278>%0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"<278>         <178>TRIPLE-CLICKED <MOUSE> <278>%0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"\n"
		"<278>         <178>MAP <MOUSE EVENT>\n"
		"<278>           Mouse events can be prefixed with MAP to only trigger when the mouse\n"
		"<278>           event occurs inside the VT100 map region.\n"
		"\n"
		"<278>         <178>SWIPED [DIR]\n"
		"<278>           %0 dir  %1 button  %2 row  %3 col  %4 -row  %5 -col\n"
		"<278>                              %6 row  %7 col  %8 -row  %9 -col %10 rows %11 cols\n"
		"\n"
		"<278>         <128>OUTPUT EVENTS\n"
		"\n"
		"<278>         <178>BUFFER UPDATE<278>, <178>DISPLAY UPDATE\n"
		"<278>           These events have no additional arguments.\n"
		"\n"
		"<278>         <178>RECEIVED LINE          <278>%0 raw text %1 plain text\n"
		"<278>         <178>RECEIVED OUTPUT        <278>%0 raw text %1 plain text\n"
		"<278>         <178>RECEIVED PROMPT        <278>%0 raw text %1 plain text\n"
		"\n"
		"<278>         <128>PORT EVENTS\n"
		"\n"
		"<278>         <178>CHAT MESSAGE<278>, <178>PORT MESSAGE\n"
		"<278>           %0 raw text  %1 plain text\n"
		"\n"
		"<278>         <178>PORT CONNECTION        <278>%0 name %1 ip %2 port\n"
		"<278>         <178>PORT DISCONNECTION     <278>%0 name %1 ip %2 port\n"
		"<278>         <178>PORT LOG MESSAGE       <278>%0 name %1 ip %2 port %3 data %4 plain data\n"
		"<278>         <178>PORT RECEIVED MESSAGE  <278>%0 name %1 ip %2 port %3 data %4 plain data\n"
		"\n"
		"<278>         <128>SCAN EVENTS\n"
		"\n"
		"<278>         <178>SCAN CSV HEADER        <278>%0 all args %1 arg1 %2 arg2 .. %99 arg99\n"
		"<278>         <178>SCAN CSV LINE          <278>%0 all args %1 arg1 %2 arg3 .. %99 arg99\n"
		"<278>         <178>SCAN TSV HEADER        <278>%0 all args %1 arg1 %2 arg3 .. %99 arg99\n"
		"<278>         <178>SCAN TSV LINE          <278>%0 all args %1 arg1 %2 arg3 .. %99 arg99\n"
		"\n"
		"<278>         <128>SCREEN EVENTS\n"
		"\n"
		"<278>         <178>SCREEN DIMENSIONS      <278>%0 height %1 width\n"
		"<278>         <178>SCREEN FOCUS           <278>%0 focus (0 or 1)\n"
		"<278>         <178>SCREEN LOCATION        <278>%0 rows %1 cols  %2 height %3 width\n"
		"\n"
		"<278>         <178>SCREEN MOUSE LOCATION\n"
		"<278>           %0 row  %1 col  %2 -row  %3 -col  %4 pix row  %5 pix col\n"
		"<278>           %6 -pix row  %7 -pix col  %8 location\n"
		"\n"
		"<278>         <178>SCREEN RESIZE          <278>%0 rows %1 cols %2 height %3 width\n"
		"<278>         <178>SCREEN SIZE            <278>%0 rows %1 cols\n"
		"<278>         <178>SCREEN SPLIT           <278>%0 top row %1 top col %2 bot row %3 bot col\n"
		"<278>         <178>SCREEN UNSPLIT         <278>%0 top row %1 top col %2 bot row %3 bot col\n"
		"\n"
		"<278>         <128>SESSION EVENTS\n"
		"\n"
		"<278>         <178>SESSION ACTIVATED      <278>%0 name\n"
		"<278>         <178>SESSION CONNECTED      <278>%0 name %1 host %2 ip %3 port %4 file\n"
		"<278>         <178>SESSION CREATED        <278>%0 name %1 host %2 ip %3 port %4 file\n"
		"<278>         <178>SESSION DEACTIVATED    <278>%0 name\n"
		"<278>         <178>SESSION DISCONNECTED   <278>%0 name %1 host %2 ip %3 port\n"
		"<278>         <178>SESSION TIMED OUT      <278>%0 name %1 host %2 ip %3 port\n"
		"\n"
		"<278>         <128>SYSTEM EVENTS\n"
		"\n"
		"<278>         <178>CONFIG                 <278>%0 name %1 value\n"
		"\n"
		"<278>         <178>DAEMON ATTACH TIMEOUT  <278>%0 file %1 pid\n"
		"<278>         <178>DAEMON ATTACHED        <278>%0 file %1 pid\n"
		"<278>         <178>DAEMON DETACHED        <278>%0 file %1 pid\n"
		"<278>         <178>PROGRAM START          <278>%0 startup arguments\n"
		"<278>         <178>PROGRAM TERMINATION    <278>%0 goodbye message\n"
		"\n"
		"<278>         <178>READ ERROR             <278>%0 filename %1 error message\n"
		"<278>         <178>READ FILE              <278>%0 filename\n"
		"<278>         <178>WRITE ERROR            <278>%0 filename %1 error message\n"
		"<278>         <178>WRITE FILE             <278>%0 filename\n"
		"\n"
		"<278>         <178>SYSTEM CRASH           <278>%0 message\n"
		"<278>         <178>SYSTEM ERROR           <278>%0 name %1 system msg %2 error %3 error msg\n"
		"<278>         <178>UNKNOWN COMMAND        <278>%0 raw text\n"
		"<278>         <178>SIGUSR                 <278>%0 signal\n"
		"\n"
		"<278>         <128>TELNET EVENTS\n"
		"\n"
		"<278>         <178>IAC <EVENT>\n"
		"<278>           IAC TELNET events are made visible using #config telnet info.\n"
		"\n"
		"<278>         <178>IAC SB GMCP            <278>%0 module    %1 data  %2 plain data\n"
		"<278>         <178>IAC SB GMCP <MODULE>   <278>             %1 data  %2 plain data\n"
		"<278>         <178>IAC SB MSSP            <278>%0 variable  %1 data\n"
		"<278>         <178>IAC SB MSDP            <278>%0 variable  %1 data  %2 plain data\n"
		"<278>         <178>IAC SB MSDP [VAR]      <278>%0 variable  %1 data  %2 plain data\n"
		"<278>         <178>IAC SB NEW-ENVIRON     <278>%0 variable  %1 data  %2 plain data\n"
		"<278>         <178>IAC SB ZMP <VAR>       <278>%0 variable  %1 data\n"
		"<278>         <178>IAC SB <VAR>           <278>%0 variable  %1 raw data  %2 plain data\n"
		"\n"
		"<278>         <128>TIME EVENTS\n"
		"\n"
		"<278>         <178>DATE <MONTH-DAY OF MONTH> [HOUR:MINUTE], DAY [DAY OF MONTH],\n"
		"<278>         <178>HOUR [HOUR], MONTH [DAY OF MONTH], TIME <HOUR:MINUTE>[:SECOND],\n"
		"<278>         <178>WEEK [DAY OF WEEK], YEAR [YEAR]\n"
		"<278>           %0 year  %1 month  %2 day of week  %3 day of month  %4 hour\n"
		"<278>           %5 minute  %6 second\n"
		"\n"

		"<278>         <128>VARIABLE EVENTS\n"
		"\n"
		"<278>         <178>VARIABLE UPDATE <VAR>  <278>%0 name %1 new value %2 path\n"
		"<278>         <178>VARIABLE UPDATED <VAR> <278>%0 name %1 new value %2 path\n"
		"\n"
		"<278>         <128>VT100 EVENTS\n"
		"\n"
		"<278>         <178>VT100 SCROLL REGION    <278>%0 top row %1 bot row %2 rows %3 cols %4 wrap\n"
		"\n"
		"<278>         To see all events trigger use #info event on. Since this can get\n"
		"<278>         rather spammy it's possible to gag event info messages.\n"
		"\n"
		"<178>Example<278>: #event {SESSION CONNECTED} {#read mychar.tin}\n"
		"\n"
		"<178>Comment<278>: You can remove an event with the #unevent command.\n",
		
		"button delay ticker"
	},
	{
		"FOREACH",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #foreach <178>{<278>list<178>} {<278>variable<178>} {<278>commands<178>}\n"
		"\n"
		"<278>         For each item in the provided list the foreach statement will update\n"
		"<278>         the given variable and execute the command part of the statement. List\n"
		"<278>         elements must be separated by braces or semicolons.\n"
		"\n"
		"<178>Example<278>: #foreach {bob;tim;kim} {name} {tell $name Hello}\n"
		"<178>Example<278>: #foreach {{bob}{tim}{kim}} {name} {tell $name Hello}\n",
		
		"break continue list loop parse repeat return while"
	},
	{
		"FORMAT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #format <178>{<278>variable<178>} {<278>format<178>} {<278>argument1<178>} {<278>argument2<178>} {<278>etc<178>}\n"
		"\n"
		"<278>         Allows you to store a string into a variable in the exact same way\n"
		"<278>         C's sprintf works with a few enhancements and limitations like a\n"
		"<278>         maximum of 30 arguments.\n"
		"\n"
		"<278>         If you use #format inside an alias or action you must escape %1s as\n"
		"<278>         %+1s or %%1s or %\\1s so the %1 isn't substituted by the trigger.\n"
		"\n"
		"<278>         #format {test} {%+9s} {string}  pad string with up to 9 spaces\n"
		"<278>         #format {test} {%-9s} {string}  post pad string with up to 9 spaces\n"
		"<278>         #format {test} {%.8s} {string}  copy at most 8 characters\n"
		"<278>         #format {test} {%a}   {number}  print corresponding charset character\n"
		"<278>         #format {test} {%c}   {string}  use a highlight color name\n"
		"<278>         #format {test} {%d}   {number}  print a number with integer formatting\n"
		"<278>         #format {test} {%f}   {string}  perform floating point math\n"
		"<278>         #format {test} {%g}   {number}  perform thousand grouping on {number}\n"
		"<278>         #format {test} {%h}   {string}  turn text into a header line\n"
		"<278>         #format {test} {%l}   {string}  lowercase text\n"
		"<278>         #format {test} {%m}   {string}  perform mathematical calculation\n"
		"<278>         #format {test} {%n}     {name}  capitalize the first letter\n"
		"<278>         #format {test} {%p}   {string}  strip leading and trailing spaces\n"
		"<278>         #format {test} {%r}   {string}  reverse text, hiya = ayih\n"
		"<278>         #format {test} {%s}   {string}  print given string\n"
		"<278>         #format {test} {%t}   {format}  display time with strftime format\n"
		"<278>                                         optional {{format}{time}} syntax\n"
		"<278>         #format {test} {%u}   {string}  uppercase text\n"
		"<278>         #format {list} {%w}   {string}  store word wrapped text in {list}\n"
		"<278>                                         optional {{string}{width}} syntax\n"
		"<278>         #format {test} {%x}      {hex}  print corresponding charset character\n"
		"<278>         #format {test} {%A}     {char}  store corresponding character value\n"
		"<278>         #format {test} {%D}      {hex}  convert hex to decimal in {test}\n"
		"<278>         #format {hash} {%H}   {string}  store a 64 bit string hash in {hash}\n"
		"<278>         #format {test} {%L}   {string}  store the string length in {test}\n"
		"<278>         #format {test} {%M}   {number}  convert number to metric in {test}\n"
		"<278>         #format {test} {%S}   {string}  store the number of spelling errors\n"
		"<278>         #format {time} {%T}         {}  store the epoch time in {time}\n"
		"<278>         #format {time} {%U}         {}  store the micro epoch time in {time}\n"
		"<278>         #format {test} {%X}      {dec}  convert dec to hexadecimal in {test}\n\n"
		"<278>         #format {test} {%%}             a literal % character\n"
		"\n"
		"<178>Comment<278>: See #help TIME for help on the %t argument.\n",
		
		"cat echo function local math replace script time variable"
	},

	{
		"FUNCTION",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #function <178>{<278>name<178>} {<278>operation<178>}\n"
		"\n"
		"<278>         Functions allow you to execute a script within a line of text, and\n"
		"<278>         replace the function call with the line of text generated by the\n"
		"<278>         function.\n"
		"\n"
		"<278>         Be aware that each function should use the #return command at the\n"
		"<278>         end of the function with the result, or set the {result} variable.\n"
		"\n"
		"<278>         To use a function use the @ character before the function name.\n"
		"<278>         The function arguments should be placed between braces behind the\n"
		"<278>         function name with argument separated by semicolons.\n"
		"\n"
		"<278>         Functions can be escaped by adding additional @ signs.\n"
		"\n"
		"<178>Example<278>: #function test #return 42;#showme @@test{}\n"
		"\n"
		"<278>         The function itself can use the provided arguments which are stored\n"
		"<278>         in %1 to %99, with %0 holding all arguments.\n"
		"\n"
		"<178>Example<278>: #function {rnd} {#math {result} {1 d (%2 - %1 + 1) + %1 - 1}}\n"
		"<278>         #show A random number between 100 and 200: @rnd{100;200}\n"
		"\n"
		"<178>Example<278>: #function gettime {#format result %t %H:%M}\n"
		"<278>         #show The current time is @gettime{}\n"
		"\n"
		"<178>Comment<278>: You can remove a function with the #unfunction command.\n",
		
		"format local math replace script variable"
	},
	{
		"GAG",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #gag <178>{<278>string<178>}\n"
		"\n"
		"<278>         Removes any line that contains the string.\n"
		"\n"
		"<178>Comment<278>: See '#help action', for more information about triggers.\n"
		"\n"
		"<278>         There are a system messages that can be gagged using gag events.\n"
		"\n"
		"<178>Comment<278>: You can remove a gag with the #ungag command.\n",
		
		"action highlight prompt substitute"
	},
	{
		"GREETING",
		TOKEN_TYPE_STRING,
		"<268>      #<268>##################################################################<268>#\n"
		"<268>      #<278>                                                                  <268>#\n"
		"<268>      #<278>                    T I N T I N + +   "CLIENT_VERSION"<278>                    <268>#\n"
		"<268>      #<278>                                                                  <268>#\n"
//		"<268>      #<278>                 <268>T<278>he K<268>i<278>cki<268>n<278> <268>T<278>ickin D<268>i<278>kuMUD Clie<268>n<278>t <268>                #\n"
//		"<268>      #<278>                                                                  <268>#\n"
		"<268>      #<278>      Code by Peter Unold, Bill Reis, and Igor van den Hoven      <268>#\n"
		"<268>      #<278>                                                                  <268>#\n"
		"<268>      #<268>##################################################################<268>#<288>\n",
		
		""
	},
	{
		"GREP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #grep <178>[<278>page<178>] {<278>search string<178>}\n"
		"\n"
		"<278>         This command allows you to search for matching lines in your scroll\n"
		"<278>         back buffer. The amount of matches shown equals your screen size. If\n"
		"<278>         you want to search back further use the optional page number. You can\n"
		"<278>         use wildcards for better search results. Be aware the search string\n"
		"<278>         is case sensitive, which can be disabled by using %i.\n"
		"\n"
		"<278>         By default grep searches from the end of the scrollback buffer to the\n"
		"<278>         beginning, this can be reversed by using a negative page number.\n"
		"\n"
		"<178>Example<278>: #grep Bubba tells you\n"
		"<278>         This will show all occasions where bubba tells you something.\n",
		
		"buffer echo showme"
	},
	{
		"HELP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #help <178>{<278>subject<178>}\n"
		"\n"
		"<278>         Without an argument #help will list all available help subjects.\n"
		"\n"
		"<278>         Using #help %* will display all help entries.\n",
		
		"commands debug ignore info message statements"
	},
	{
		"HIGHLIGHT",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #highlight <178>{<278>string<178>} {<278>color names<178>} {<278>priority<178>}\n"
		"\n"
		"<278>         The highlight command is used to change the color of incoming text.\n"
		"\n"
		"<278>         Available color options are:\n"
		"\n"
		"<278>         reset      - resets the color state to default\n"
		"<278>         light      - turns the color light.\n"
		"<278>         dark       - turns the color dark.\n"
		"<278>         underscore - underscores the text.\n"
		"<278>         blink      - makes the text blink.\n"
		"<278>         reverse    - reverse foreground and background color.\n"
		"<278>         b          - makes next color the background color.\n"
		"\n"
		"<278>         Available color names are:\n"
		"\n"
		"<278>         <<888>abd> - azure                 <<888>acf> - Azure\n"
		"<278>         <<888>aad> - blue                  <<888>aaf> - Blue\n"
		"<278>         <<888>add> - cyan                  <<888>aff> - Cyan\n"
		"<278>         <<888>aaa> - ebony                 <<888>bbb> - Ebony\n"
		"<278>         <<888>ada> - green                 <<888>afa> - Green\n"
		"<278>         <<888>adb> - jade                  <<888>afc> - Jade\n"
		"<278>         <<888>bda> - lime                  <<888>cfa> - Lime\n"
		"<278>         <<888>dad> - magenta               <<888>faf> - Magenta\n"
		"<278>         <<888>dba> - orange                <<888>fca> - Orange\n"
		"<278>         <<888>dab> - pink                  <<888>fac> - Pink\n"
		"<278>         <<888>daa> - red                   <<888>faa> - Red\n"
		"<278>         <<888>ccc> - silver                <<888>eee> - Silver\n"
		"<278>         <<888>cba> - tan                   <<888>eda> - Tan\n"
		"<278>         <<888>bad> - violet                <<888>caf> - Violet\n"
		"<278>         <<888>ddd> - white                 <<888>fff> - White\n"
		"<278>         <<888>dda> - yellow                <<888>ffa> - Yellow\n"
		"\n"
		"<278>         Colors can be provided as either a color code or one of the valid color\n"
		"<278>         names. If the color name is in all lower case a dark color is printed.\n"
		"<278>         If the first letter of the color name is capitalized a light color is\n"
		"<278>         printed.\n"
		"\n"
		"<278>         The %1-99 variables can be used as 'wildcards' that will match with any\n"
		"<278>         text. They are useful for highlighting a complete line. The %0 variable\n"
		"<278>         should never be used in highlights.\n"
		"\n"
		"<278>         You may start the string to highlight with a ^ to only highlight text\n"
		"<278>         if it begins the line.\n"
		"\n"
		"<278>         Besides color names also <<888>abc> color codes can be used.\n"
		"\n"
		"<178>Example<278>: #high {Valgar} {reverse underscore Jade}\n"
		"<278>         Prints every occurrence of 'Valgar' in underscored reverse video Jade.\n"
		"\n"
		"<178>Example<278>: #high {^You{|r} %1} {light cyan}\n"
		"<278>         Prints every line that starts with 'You' in light cyan.\n"
		"\n"
		"<178>Example<278>: #high {Bubba} {red underscore b Green}\n"
		"<278>         Highlights the name Bubba as red underscored text on green background.\n"
		"\n"
		"<178>Comment<278>: See '#help action', for more information about triggers.\n"
		"\n"
		"<178>Comment<278>: See '#help substitute', for more advanced color substitution.\n"
		"\n"
		"<178>Comment<278>: This command only works with ANSI/VT100 terminals or emulators.\n"
		"\n"
		"<178>Comment<278>: You can remove a highlight with the #unhighlight command.\n",
		
		"action gag prompt substitute"
	},
	{
		"HISTORY",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #history <178>{<278>delete<178>}<278>                 Delete the last command.\n"
		"<278>         #history <178>{<278>insert<178>}    {<278>command<178>}<278>    Insert a command.\n"
		"<278>         #history <178>{<278>list<178>}<278>                   Display the entire command history.\n"
		"<278>         #history <178>{<278>read<178>}      {<278>filename<178>}<278>   Read a command history from file.\n"
		"<278>         #history <178>{<278>write<178>}     {<278>filename<178>}<278>   Write a command history to file.\n"
		"\n"
		"<278>         Without an argument all available options are shown.\n"
		"\n"
		"<278>         By default all commands are saved to the history list and the history\n"
		"<278>         list is saved between sessions in the ~/.tintin/history.txt file.\n"
		"\n"
		"<278>         You can set the character to repeat a command in the history with the\n"
		"<278>         #config {REPEAT CHAR} {<character>} configuration option, by default\n"
		"<278>         this is set to the exclamation mark.\n"
		"\n"
		"<278>         You can use ! by itself to repeat the last command, or !<text> to\n"
		"<278>         repeat the last command starting with the given text.\n"
		"\n"
		"<278>         You can use #config {REPEAT ENTER} {ON} to repeat the last command\n"
		"<278>         when you press enter on an empty line.\n"
		"\n"
		"<278>         You can press ctrl-r to enter an interactive regex enabled history\n"
		"<278>         search mode, or by issuing #cursor {history search}.\n"
		"\n"
		"<278>         TinTin++ tries to bind the arrow up and down keys to scroll through\n"
		"<278>         the history list by default. You can bind these with a macro yourself\n"
		"<278>         using #cursor {history next} and #cursor {history prev}. Many #cursor\n"
		"<278>         commands only work properly when bound with a macro.\n",
		
		"alias cursor keypad macro speedwalk tab"
	},
	{
		"IF",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #if <178>{<278>conditional<178>} {<278>commands if true<178>}\n"
		"\n"
		"<278>         The #if command works similar to an if statement in other languages,\n"
		"<278>         and is based on the way C handles its conditional statements.\n"
		"<278>         When an #if command is encountered, the conditional statement is\n"
		"<278>         evaluated, and if TRUE (any non-zero result) the commands are executed.\n"
		"\n"
		"<278>         The conditional is evaluated exactly the same as in the #math command,\n"
		"<278>         if the conditional evaluates as anything except 0 the commands are\n"
		"<278>         executed. See the 'math' helpfile for more information.\n"
		"\n"
		"<278>         To handle the case where an if statement is false it can be followed\n"
		"<278>         by the #else command.\n"
		"\n"
		"<178>Example<278>: #action {%0 gives you %1 gold coins.} {#if {%1 > 5000} {thank %0}}\n"
		"<278>         If someone gives you more than 5000 coins, thank them.\n"
		"\n"
		"<178>Example<278>: #alias {k} {#if {\"%0\" == \"\"} {kill $target};#else {kill %0}}\n",
		
		"case default else elseif math switch regexp"
	},
	{
		"IGNORE",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #ignore <178>{<278>listname<178>} {<278>on<178>|<278>off<178>}\n"
		"\n"
		"<278>         Toggles a list on or off. With no arguments it shows your current\n"
		"<278>         settings, as well as the list names that you can ignore.\n"
		"\n"
		"<278>         If you for example use #IGNORE ACTIONS ON actions will no longer\n"
		"<278>         triger. Not every list can be ignored.\n"
		,
		"class debug info kill message"
	},
	{
		"INDEX",
		TOKEN_TYPE_STRING,
		"<acf>"
		"<acf>                   ████████┐██████┐███┐   ██┐████████┐██████┐███┐   ██┐\n"
		"<acf>                   └──██┌──┘└─██┌─┘████┐  ██│└──██┌──┘└─██┌─┘████┐  ██│\n"
		"<acf>                      ██│     ██│  ██┌██┐ ██│   ██│     ██│  ██┌██┐ ██│\n"
		"<acf>                      ██│     ██│  ██│└██┐██│   ██│     ██│  ██│└██┐██│\n"
		"<acf>                      ██│   ██████┐██│ └████│   ██│   ██████┐██│ └████│\n"
		"<acf>                      └─┘   └─────┘└─┘  └───┘   └─┘   └─────┘└─┘  └───┘\n"
		"<acf>                                       ██┐      ██┐\n"
		"<acf>                                       ██│      ██│\n"
		"<acf>                                    ████████┐████████┐\n"
		"<acf>                                    └──██┌──┘└──██┌──┘\n"
		"<acf>                                       ██│      ██│\n"
		"<acf>                                       └─┘      └─┘\n"
		"\n"
		"<278>                       <acf>(<abd>T<acf>)<abd>he K<acf>(<abd>I<acf>)<abd>cki<acf>(<abd>N<acf>)<abd> <acf>(<abd>T)ickin D<acf>(<abd>I<acf>)<abd>kumud Clie<acf>(<abd>N<acf>)<abd>t\n"
		"\n"
		"\n"
		"<278>         <128>What is TinTin++?\n"
		"\n"
		"<278>         TinTin++ is a client program specialized to help playing muds. This is\n"
		"<278>         a souped up version of TINTIN III with many new features.\n"
		"\n"
		"<278>         <128>Giving Credit Where Credit is Due\n"
		"\n"
		"<278>         None of this work would be possible, without the work done by Peter\n"
		"<278>         Unold. He was the author of TINTIN III, the base of TinTin++. Hats off\n"
		"<278>         to ya Peter. You started the ball rolling.\n"
		"\n"
		"<278>         <128>Introduction\n"
		"\n"
		"<278>         If you're new to TinTin++ a good place to start is the introduction,\n"
		"<278>         which should be linked below.\n"
		,
		"introduction"
	},
	{
		"INFO",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #info <178>{<278>listname<178>} {<278>LIST<178>|<278>ON<178>|<278>OFF<178>|<278>SAVE<178>}\n"
		"\n"
		"<278>         Without an argument info displays the settings of every tintin list.\n"
		"\n"
		"<278>         By providing the name of a list and the LIST option it shows all\n"
		"<278>         triggers/variables associated with that list. With the SAVE option\n"
		"<278>         this data is written to the info variable.\n"
		"\n"
		"<278>         #info arguments will show matched trigger arguments.\n"
		"<278>         #info big5toutf will show the big5 to utf8 translation table.\n"
		"<278>         #info cpu will show information about tintin's cpu usage.\n"
		"<278>         #info environ will show the environment variables.\n"
		"<278>         #info input will show information about the input line.\n"
		"<278>         #info matches will show matched command arguments.\n"
		"<278>         #info mccp will show information about data compression.\n"
		"<278>         #info memory will show information about the memory stack.\n"
		"<278>         #info output will show information about the mud output buffers.\n"
		"<278>         #info stack will show the low level debugging stack.\n"
		"<278>         #info session will show information on the session.\n"
		"<278>         #info sessions will show information on all sessions.\n"
		"<278>         #info system will show some system information.\n"
		"<278>         #info tokenizer will show information about the script stack.\n"
		"<278>         #info unicode will show information on the provided character.\n"
		,
		"class debug ignore kill message"
	},
	{
		"INTRODUCTION",
		TOKEN_TYPE_STRING,
		"<278>         On this page you'll find an introduction to using TinTin++. Additional\n"
		"<278>         information can be found in the individual help sections.\n"
		"\n"
		"<278>         <128>Starting and Ending\n"
		"\n"
		"<278>         The syntax for starting TinTin++ is: ./tt++ [command file]\n"
		"\n"
		"<278>         Read more about the command file in the files section below. Remember\n"
		"<278>         one thing though. All actions, aliases, substitutions, etc, defined\n"
		"<278>         when starting up TinTin++ are inherited by all sessions.\n"
		"\n"
		"<278>         If you want to exit TinTin++ type '#end' or press ctrl-d on an empty\n"
		"<278>         line.\n"
		"\n"
		"<278>         For the WinTin++ users, if you want to paste text use shift-insert,\n"
		"<278>         text is automatically copied upon selection. This is typical Linux\n"
		"<278>         behavior, but it can take some getting used to.\n"
		"\n"
		"\n"
		"<278>         <128>Basic features\n"
		"\n"
		"<278>         I'll start by explaining some of the very basic and important features:\n"
		"\n"
		"<278>         All TinTin++ commands starts with a '#'.\n"
		"\n"
		"<178>Example<278>: #help -- #help is a client command, and isn't send to the server.\n"
		"\n"
		"<278>         All TinTin++ commands can be abbreviated when typed.\n"
		"\n"
		"<278>         #he -- Typing #he is the same as typing #help though it's suggested to\n"
		"<278>         use at least 3 letter abbreviations just in case another command is\n"
		"<278>         added that starts with 'he'.\n"
		"\n"
		"<278>         All commands can be separated with a ';'.\n"
		"\n"
		"<278>         n;l dragon;s;say Dan Dare is back! -- do these 4 commands\n"
		"<278>         There are 3 ways ';'s can be overruled.\n"
		"\n"
		"<278>         \\say Hello ;) -- Lines starting with a '\\' aren't parsed by TinTin++.\n"
		"<278>         say Hello \\;) -- The escape character can escape 1 letter.\n"
		"<278>         #config verbatim on -- Everything is sent as is except '#' commands.\n"
		"\n"
		"<278>         <128>Connecting to a server\n"
		"\n"
		"<178>Command<278>: #session <178>{<278>session name<178>} {<278>server address<178>} {<278>port<178>}\n"
		"\n"
		"<178>Example<278>: #session someone tintin.sourceforge.net 4321\n"
		"\n"
		"<278>         You can have more than one session, in which case you can switch\n"
		"<278>         between sessions typing #<session name>.\n"
		"\n"
		"<278>         You can get a list of all sessions by typing: #session. The current\n"
		"<278>         active session is marked with (active). Snooped sessions with\n"
		"<278>         (snooped). MCCP sessions (compression) with (mccp 2) and (mccp 3).\n"
		"\n"
		"\n"
		"<278>         <128>Split\n"
		"\n"
		"<178>Command<278>: #split\n"
		"\n"
		"<278>         The split command will create a separated input and output area.\n"
		"\n"
		"<278>         Using the #prompt command you can capture the prompt and place it on\n"
		"<278>         the split line. To get rid of the split interface you can use #unsplit\n"
		"<278>         which will restore the terminal settings to default.\n"
		"\n"
		"\n"
		"<278>         <128>Alias\n"
		"\n"
		"<178>Command<278>: #alias <178>{<278>name<178>} {<278>commands<178>}\n"
		"\n"
		"<278>         The syntax of the #alias command is almost like alias in csh.\n"
		"<278>         Use this command to define aliases. The variables %0, %1.. %9 contain\n"
		"<278>         the arguments to the aliased command as follows:\n"
		"<278>         the %0 variable contains all the arguments.\n"
		"<278>         the %1 variable contains the 1st argument\n"
		"<278>         ....\n"
		"<278>         the %9 variable contains the 9th argument\n"
		"\n"
		"<178>Example<278>: #alias greet say Greetings, most honorable %1\n"
		"\n"
		"<278>         If you want an alias to execute more commands, you must use braces.\n"
		"\n"
		"<178>Example<278>: #alias ws <178>{<278>wake;stand<178>}\n"
		"\n"
		"<278>         To delete an alias use the #unalias command.\n"
		"\n"
		"<278>         WARNING! TinTin++ doesn't baby sit, and hence does not check for\n"
		"<278>         recursive aliases! You can avoid recursion by escaping the entire\n"
		"<278>         line.\n"
		"\n"
		"<178>Example<278>: #alias put \\put %1 in %2\n"
		"\n"
		"<278>         Or by using the send command.\n"
		"\n"
		"<178>Example<278>: #alias put #send put %1 in %2\n"
		"\n"
		"\n"
		"<128>         Action\n"
		"\n"
		"<178>Command<278>: #action <178>{<278>action-text<178>} {<278>commands<178>}\n"
		"\n"
		"<278>         Use this command to define an action to take place when a particular\n"
		"<278>         text appears on your screen. There are 99 variables you can use as\n"
		"<278>         wildcards in the action-text.\n"
		"\n"
		"<278>         These variables are %1, %2, %3 .... %9, %10, %11 ... %97, %98, %99.\n"
		"\n"
		"<178>Example<278>: #action <178>{<278>You are hungry<178>} {<278>get bread bag;eat bread<178>}\n"
		"\n"
		"<178>Example<278>: #action <178>{<278>%1 has arrived.<178>}<278> shake %1 -- shake hands with people arriving.\n"
		"\n"
		"<178>Example<278>: #action <178>{<278>%1 tells you '%2'<178>}\n"
		"<278>                   <178>{<278>tell bob %1 told me '%2'<178>}<278> -- forward tells.\n"
		"\n"
		"<178>Example<278>: #action <178>{<278>tells you<178>}<278> #bell -- beep on tell.\n"
		"\n"
		"<278>         You can have TinTin++ ignore actions if you type '#ignore actions on'.\n"
		"\n"
		"<278>         You can see what commands TinTin++ executes when an action triggers\n"
		"<278>         by typing '#debug actions on'.\n"
		"\n"
		"<278>         You can remove actions with the #unaction command.\n"
		"\n"
		"\n"
		"<278>         <128>Command files\n"
		"\n"
		"<278>         When you order TinTin++ to read a command file, it parses all the text\n"
		"<278>         in the file. You can use command files to keep aliases/actions in,\n"
		"<278>         login to a server (name, password etc..) and basically all kinds of\n"
		"<278>         commands.\n"
		"\n"
		"<278>         You can make the command files with either a text editor (suggested),\n"
		"<278>         or use the #write command to write out a file.\n"
		"\n"
		"<278>         Commands for files:\n"
		"\n"
		"<278>         #read filename -- read and execute the file.\n"
		"\n"
		"<278>         #write filename -- write all actions/aliases/substitutes/etc known for\n"
		"<278>         the current session to a file.\n"
		"\n"
		"<178>Example<278>:\n"
		"<278>         #session x mymud.com 1234\n"
		"<278>         myname\n"
		"<278>         mypassword\n"
		"<278>         #split\n"
		"<278>         #action {^You are hungry.} {eat bread}\n"
		"\n"
		"<278>         If you save the above five lines to a file named 'mymud.tin' you can\n"
		"<278>         use 'tt++ mymud.tin' to start tintin and execute the file, connecting\n"
		"<278>         you to your mud, logging in, enabling split mode, and setting an action\n"
		"<278>         to eat a bread whenever you go hungry.\n"
		"\n"
		"<278>         <128>Highlight\n"
		"\n"
		"<178>Command<278>: #highlight <178>{<278>text<178>} {<278>color<178>}\n"
		"\n"
		"<278>         This command works a bit like #action. The purpose of this command is\n"
		"<278>         to substitute text from the server with color you provide. This command\n"
		"<278>         is a simplified version of the #substitute command.\n"
		"\n"
		"<178>Example<278>: #high <178>{<278>Snowy<178>} {<278>light yellow<178>}\n"
		"\n"
		"<178>Example<278>: #high <178>{<278>%*Snowy%*<178>} {<278>light yellow<178>}\n"
		"\n"
		"<278>         Use #unhigh to delete highlights.\n"
		"\n"
		"\n"
		"<128>         Speedwalk\n"
		"\n"
		"<278>         If you type a command consisting ONLY of letters and numbers n, e, s,\n"
		"<278>         w, u, d - then this command can be interpreted as a serie of movement\n"
		"<278>         commands.\n"
		"\n"
		"<178>Example<278>: ssw2n -- go south, south, west, north, north\n"
		"\n"
		"<278>         If you have problems with typing some commands that actually ONLY\n"
		"<278>         consists of these letters, then type them in CAPS. For example when\n"
		"<278>         checking the NEWS or when asked to enter NEW as your name.\n"
		"\n"
		"<278>         You must enable speedwalking with: #config speedwalk on.\n"
		"\n"
		"\n"
		"<278>         <128>Ticker\n"
		"\n"
		"<178>Command<278>: #ticker <178>{<278>name<178>} {<278>commands<178>} {<278>seconds<178>}\n"
		"\n"
		"<278>         The name can be whatever you want it to be, and is only required for\n"
		"<278>         the unticker command. The commands will be executed every x amount of\n"
		"<278>         seconds, which is specified in the interval part.\n"
		"\n"
		"<178>Example<278>: #tick <178>{<278>tick<178>} {<278>#delay 50 #show 10 SECONDS TO TICK!;#show TICK!!!<178>} {<278>60<178>}\n"
		"\n"
		"<278>         This creates a ticker with the name <178>{<278>tick<178>}<278> which will print TICK!!!,\n"
		"<278>         as well as print a warning when the next tick will occure.\n"
		"\n"
		"<278>         You can remove tickers with #untick\n"
		"\n"
		"\n"
		"<278>         <128>Repeating Commands\n"
		"\n"
		"<278>         You can repeat a command, the syntax is: #number command\n"
		"\n"
		"<178>Example<278>: #5 cackle -- if you just killed bob the wizard.\n"
		"<178>Example<278>: #10 <178>{<278>buy bread;put bread bag<178>}<278> -- repeat these 2 commands 10 times.\n"
		"<178>Example<278>: #100 ooc w00t w00t!!!!! -- nochannel yourself.\n"
		"\n"
		"\n"
		"<278>         <128>History\n"
		"\n"
		"<278>         TinTin++ has a limited subset of the csh history features.\n"
		"\n"
		"<278>         ! -- repeat the last command\n"
		"<278>         !cast -- repeat the last command starting with cast\n"
		"<278>         ctrl-r -- enter the reverse history search mode.\n"
		"\n"
		"\n"
		"<278>         <128>Map commands\n"
		"\n"
		"<278>         TinTin++ has a powerful highly configurable automapper. Whenever\n"
		"<278>         you type n/ne/e/se/s/sw/w/nw/n/u/d tt++ tries to keep track of your\n"
		"<278>         movement.\n"
		"\n"
		"<278>         Commands for map:\n"
		"\n"
		"<278>         #map create -- create a map.\n"
		"<278>         #map goto 1 -- go to the first room in the map, created by default.\n"
		"<278>         #map map -- display the map.\n"
		"<278>         #map undo -- undo your last map alteration.\n"
		"<278>         #map write <filename> -- save the map to file.\n"
		"<278>         #map read <filename> -- load a map from file.\n"
		"\n"
		"<278>         There are many other map options and it's beyond the scope of this\n"
		"<278>         help section to explain everything there is to know, but I'll give\n"
		"<278>         a set of commands that will get most people started.\n"
		"\n"
		"<278>         #map create\n"
		"<278>         #split 12 1\n"
		"<278>         #map flag unicode on\n"
		"<278>         #map flag vt on\n"
		"<278>         #map goto 1\n"
		"\n"
		"<278>         These commands will create a 12 row vt100 split section at the top of\n"
		"<278>         your screen where a map drawn using unicode characters is displayed.\n"
		"\n"
		"<178>Example<278>: #action <178>{<278>There is no exit in that direction.<178>} {<278>#map undo<178>}\n"
		"\n"
		"<278>         The map will be automatically created as you move around.\n"
		"\n"
		"\n"
		"<278>         <128>Help\n"
		"\n"
		"<178>Command<278>: #help <178>{<278>subject<178>}\n"
		"\n"
		"<278>         The help command is your friend and contains the same helpfiles\n"
		"<278>         inside TinTin++ as are available on the website. If you type #help\n"
		"<278>         without an argument you will see the various available help subjects\n"
		"<278>         which try to explain the TinTin++ commands and features in greater\n"
		"<278>         detail. Entries in cyan describe commands, while entries in white\n"
		"<278>         describe various features, often in greater detail.\n"
		"\n"
		"\n"
		"<278>         <128>That's all for the introduction, enjoy\n"
		,
		"characters colors coordinates editing escape_codes greeting keypad lists mapping mathematics screen_reader sessionname speedwalk statements suspend time"
	},
	{
		"KEYPAD",
		TOKEN_TYPE_STRING,
		"<278>         When TinTin++ starts up it sends \\e= to the terminal to enable the\n"
		"<278>         terminal's application keypad mode, which can be disabled using #show {\\e>}\n"
		"\n"
		"<178>      Configuration A           Configuration B           Configuration C\n"
		"<268> ╭─────┬─────┬─────┬─────╮ ╭─────┬─────┬─────┬─────╮ ╭─────┬─────┬─────┬─────╮\n"
		"<268> │<178>num<268>  │<178>/<268>    │<178>*<268>    │<178>-<268>    │ │<178>num<268>  │<178>/<268>    │<178>*<268>    │<178>-<268>    │ │<178>Num<268>  │<178>nkp/<268> │<178>nkp*<268> │<178>nkp-<268> │\n"
		"<268> ├─────┼─────┼─────┼─────┤ ├─────┼─────┼─────┼─────┤ ├─────┼─────┼─────┼─────┤\n"
		"<268> │<178>7<268>    │<178>8<268>    │<178>9<268>    │<178>+<268>    │ │<178>Home<268> │<178>Up<268>   │<178>PgUp<268> │<178>+<268>    │ │<178>nkp7<268> │<178>nkp8<268> │<178>nkp9<268> │<178>nkp+<268> │\n"
		"<268> ├─────┼─────┼─────┤     │ ├─────┼─────┼─────┤     │ ├─────┼─────┼─────┤     │\n"
		"<268> │<178>4<268>    │<178>5<268>    │<178>6<268>    │     │ │<178>Left<268> │<178>Cntr<268> │<178>Right<268>│     │ │<178>nkp4<268> │<178>nkp5<268> │<178>nkp6<268> │     │\n"
		"<268> ├─────┼─────┼─────┼─────┤ ├─────┼─────┼─────┼─────┤ ├─────┼─────┼─────┼─────┤\n"
		"<268> │<178>1<268>    │<178>2<268>    │<178>3<268>    │<178>Enter<268>│ │<178>End<268>  │<178>Down<268> │<178>PgDn<268> │<178>Enter<268>│ │<178>nkp1<268> │<178>nkp2<268> │<178>nkp3<268> │<178>nkpEn<268>│\n"
		"<268> ├─────┴─────┼─────┤     │ ├─────┴─────┼─────┤     │ ├─────┴─────┼─────┤     │\n"
		"<268> │<178>0<268>          │<178>.<268>    │     │ │<178>Ins<268>        │<178>Del<268><268>  │     │ │<178>nkp0<268>       │<178>nkp.<268> │     │\n"
		"<268> ╰───────────┴─────┴─────╯ ╰───────────┴─────┴─────╯ ╰───────────┴─────┴─────╯\n"
		"\n"
		"<278>         With keypad mode disabled numlock on will give you configuration A,\n"
		"<278>         and numlock off will give you configuration B. With keypad mode\n"
		"<278>         enabled you'll get configuration C.\n"
		"\n"
		"<178>         Terminals that support keypad mode\n"
		"\n"
		"<278>         Linux Console, PuTTY, MinTTY, Eterm, aterm.\n"
		"\n"
		"<178>         Terminals that do not support keypad mode\n"
		"\n"
		"<278>         RXVT on Cygwin, Windows Console, Gnome Terminal, Konsole.\n"
		"\n"
		"<178>         Peculiar Terminals\n"
		"\n"
		"<278>         RXVT requires turning off numlock to enable configuration C.\n"
		"\n"
		"<278>         Xterm may require disabling Alt/NumLock Modifiers (num-lock) in the\n"
		"<278>         ctrl left-click menu. Or edit ~/.Xresources and add\n"
		"<278>         XTerm*VT100.numLock:false\n"
		"\n"
		"<278>         Mac OS X Terminal requires enabling 'strict vt100 keypad behavior' in\n"
		"<278>         Terminal -> Window Settings -> Emulation.\n"
		,
		"colors coordinates escape_codes mathematics pcre"
	},
	{
		"KILL",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #kill <178>{<278>list<178><178>} {<278>pattern<178>}\n"
		"\n"
		"<278>         Without an argument, the kill command clears all lists.  Useful if\n"
		"<278>         you don't want to exit tintin to reload your command files.\n"
		"\n"
		"<278>         With one argument a specific list can be cleared.\n"
		"\n"
		"<278>         With two arguments the triggers in the chosen list that match the\n"
		"<278>         given pattern will be removed.\n"
		"\n"
		"<178>Example<278>: #kill alias %*test*\n"
		,
		"class debug ignore info message"
	},
	{
		"LINE",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #line <178>{<278>option<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         <128>Line options that alter the argument.\n"
		"\n"
		"<278>         <178>#line json <variable> <argument>\n"
		"<278>           The variable is translated to json and the argument is executed\n"
		"<278>           with &0 holding the json data.\n"
		"\n"
		"<278>         <178>#line strip <argument>\n"
		"<278>           Argument is executed with all color codes stripped.\n"
		"\n"
		"<278>         <178>#line substitute <options> <argument>\n"
		"<278>           Argument is executed using the provided substitutions, available\n"
		"<278>           options are: arguments, braces, colors, escapes, functions, secure,\n"
		"<278>           and variables.\n"
		"\n"
		"<278>         <128>Line options that alter how the line is executed.\n"
		"\n"
		"<278>         <178>#line background <argument>\n"
		"<278>           Prevent new session activation.\n"
		"\n"
		"<278>         <178>#line capture <variable> <argument.\n"
		"<278>           Argument is executed and output stored in <variable>.\n"
		"\n"
		"<278>         <178>#line convert <argument>\n"
		"<278>           Argument is executed with escaped meta characters.\n"
		"\n"
		"<278>         <178>#line debug <argument>\n"
		"<278>           Argument is executed in debug mode.\n"
		"\n"
		"<278>         <178>#line gag [amount]\n"
		"<278>           Gag the next line, or given lines. Use + or - to increase\n"
		"<278>           or decrease the current amount.\n"
		"\n"
		"<278>         <178>#line ignore {argument}\n"
		"<278>           Argument is executed without any triggers being checked.\n"
		"\n"
		"<278>         <178>#line local {argument}\n"
		"<278>           Argument is executed with all newly and indirectly\n"
		"<278>           created variables being local.\n"
		"\n"
		"<278>         <178>#line log <filename> [text]\n"
		"<278>           Log the next line to file unless the [text] argument is\n"
		"<278>           provided.\n"
		"\n"
		"<278>         <178>#line logmode <option> <argument>\n"
		"<278>           Argument is executed using the provided logmode, available\n"
		"<278>           modes are: html, plain, raw, and stamp.\n"
		"\n"
		"<278>         <178>#line msdp <argument>\n"
		"<278>           Turn the argument into an msdp telnet sequence, starting at the\n"
		"<278>           first opening brace. Will turn tintin tables into msdp tables,\n"
		"<278>           with semicolons being used to create msdp arrays.\n"
		"\n"
		"<278>         <178>#line multishot <number> <argument>\n"
		"<278>           Argument is executed in multishot mode, all triggers created\n"
		"<278>           will only fire the given number of times.\n"
		"\n"
		"<278>         <178>#line oneshot <argument>\n"
		"<278>           Argument is executed in oneshot mode, all triggers created will\n"
		"<278>           only fire once.\n"
		"\n"
		"<278>         <178>#line quiet <argument>\n"
		"<278>           Argument is executed with suppression of most system messages.\n"
		"\n"
		"<278>         <178>#line verbatim <argument>\n"
		"<278>           Argument is executed verbatim, prohibiting variable and function\n"
		"<278>           substitutions.\n"
		"\n"
		"<278>         <178>#line verbose <argument>\n"
		"<278>           Argument is executed with most system messages enabled.\n"
		"\n"
		"<278>         When using #line log and logging in html format use \\c< \\c> \\c& \\c\" to\n"
		"<278>         log a literal < > & and \".\n"
		,
		"class config"
	},
	{
		"LIST",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #list <178>{<278>variable<178>} {<278>option<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         #list {var} {add} <items>              Add <items> to the list\n"
		"<278>         #list {var} {clear}                    Empty the given list\n"
		"<278>         #list {var} {collapse} <separator>     Turn list into a variable\n"
		"<278>         #list {var} {create} <items>           Create a list using <items>\n"
		"<278>         #list {var} {delete} <index> [amount]  Delete the item at <index>,\n"
		"<278>                                                the [amount] is optional.\n"
		"<278>         #list {var} {explode} <separator>      Turn variable into a list\n"
		"<278>         #list {var} {indexate} [key]           Index a list table for sorting\n"
		"<278>         #list {var} {insert} <index> <item>    Insert <item> at given index\n"
		"<278>         #list {var} {filter} <keep> [remove]   Filter with keep / remove regex\n"
		"<278>         #list {var} {find} <regex> <variable>  Return the found index\n"
		"<278>         #list {var} {get} <index> <variable>   Copy an item to {variable}\n"
		"<278>         #list {var} {numerate}                 Renumber a table or list\n"
		"<278>         #list {var} {order} [items]            Sort list alphanumerically\n"
		"<278>         #list {var} {refine} <keep> [remove]   Filter with keep / remove math\n"
		"<278>                                                with &0 holding the value\n"
		"<278>         #list {var} {reverse}                  Reverse the list\n"
		"<278>         #list {var} {shuffle}                  Shuffle the list\n"
		"<278>         #list {var} {set} <index> <item>       Change the item at {index}\n"
		"<278>         #list {var} {simplify} [items]         Turn list into a simple list\n"
		"<278>         #list {var} {size} <variable>          Copy list size to {variable}\n"
		"<278>         #list {var} {sort} [items]             Sort list alphabetically, if\n"
		"<278>                                                an item is given it's added.\n"
		"<278>         #list {var} {tokenize} <string>        Create a character list\n"
		"\n"
		"<278>         The index should be between +1 and the list's size. You can also give\n"
		"<278>         a negative value, in which case -1 equals the last item in the list, -2\n"
		"<278>         the second last, etc.\n"
		"\n"
		"<278>         When inserting an item a positive index will prepend the item at the\n"
		"<278>         given index, while a negative index will append the item.\n"
		"\n"
		"<278>         The add and create options allow using multiple items, as well\n"
		"<278>         as semicolon separated items.\n"
		"\n"
		"<278>         The get option will return the item or the indexation. Use\n"
		"<278>         $var[<index>] to retrieve the nested data of a list table.\n"
		"\n"
		"<278>         The order, sort and simplify options will perform the operation on\n"
		"<278>         the given list. Optional items can be provided which are added to\n"
		"<278>         the new or existing list before the operation is executed. Sorting\n"
		"<278>         and ordering are stable.\n"
		"\n"
		"<278>         The indexate option prepares a table or list table for order, sort,\n"
		"<278>         filter, refine, and find operations for the given key. It is similar\n"
		"<278>         to the SELECT option in SQL.\n"
		"\n"
		"<278>         A size of 0 is returned for an empty or non-existent list. You can\n"
		"<278>         directly access the size of a list using &var[].\n"
		"\n"
		"<278>         You can directly access elements in a list variable using $var[+1],\n"
		"<278>         $var[+2], $var[-1], etc.\n"
		,
		"break continue foreach loop parse repeat return while"
	},

	{
		"LISTS",
		TOKEN_TYPE_STRING,
		"<278>         There are several different types of lists in tintin which behave in a\n"
		"<278>         fairly universal manner. To properly explain lists it's easiest to\n"
		"<278>         explain the most basic variable type first before discussing more\n"
		"<278>         complex types.\n"
		"\n"
		"       - Basic variable: The standard key = value variable.\n"
		"\n"
		"       - Simple list: A string that contains semicolon delimited fields.\n"
		"<278>         {a;b;c}. Can be saved as a variable.\n"
		"\n"
		"       - Brace list: A string in which fields are delimited with braces.\n"
		"<278>         {a}{b}{c}. Brace lists cannot be stored as a variable because tables\n"
		"<278>         use braces as well, they must be stored as a simple list instead.\n"
		"\n"
		"       - Table: Think of this as variables nested within another variable. Or\n"
		"<278>          as variables contained within another variable.\n"
		"\n"
		"       - List: A table that uses integers for its indexes. Also known as an\n"
		"<278>         array. The #list command is a utility command for using tables as\n"
		"<278>         arrays.\n"
		"\n"
		"<128>         Simple Variables\n"
		"\n"
		"<178>Example<278>:\n"
		"<278>         #variable {simple} {Hello World!}\n"
		"<278>         #show $simple\n"
		"\n"
		"<278>         To see if the 'simple' variable exists you can use &{simple} which\n"
		"<278>         will display 0 if the variable does not exist, or the variable's index\n"
		"<278>         if it exists.\n"
		"\n"
		"<278>         If you have multiple variables they are sorted alphabetically and\n"
		"<278>         numerically. While it's not all that relevant for simple variables,\n"
		"<278>         the first variable has index 1, the second variable index 2, and so\n"
		"<278>         on.\n"
		"\n"
		"<278>         Variable names need to start with a letter and only exist of letters,\n"
		"<278>         numbers, and underscores. If you need to use a non standard variable\n"
		"<278>         name this is possible using braces.\n"
		"\n"
		"<178>Example<278>: #variable {:)} {Happy Happy!};#show ${:)}\n"
		"\n"
		"<278>         Variables can be accessed using their index. While primarily useful\n"
		"<278>         for tables it is possible to do this for simple variables. Use +1 for\n"
		"<278>         the first variable, +2 for the second variable, etc. Use -1 for the\n"
		"<278>         last variable, -2 for the second last variable, etc.\n"
		"\n"
		"<178>Example<278>: #show The first variable is: *{+1} with value: ${+1}\n"
		"\n"
		"<128>         Removing Variables\n"
		"\n"
		"<278>         To remove a variable, use #unvariable or #unvar (every command can be\n"
		"<278>         abbreviated). It's possible to remove multiple variables at once\n"
		"<278>         using #unvar {var 1} {var 2} {etc}\n"
		"\n"
		"<278>         Variables are unique to each session, so if you have multiple\n"
		"<278>         sessions, removing a variable from one session won't remove it from\n"
		"<278>         other sessions.\n"
		"\n"
		"<278>         If you remove a table variable, all variables contained within that\n"
		"<278>         table variable are removed as well.\n"
		"\n"
		"<128>         Simple Lists\n"
		"\n"
		"<278>         A simple list is a string that contains semicolon delimited fields.\n"
		"<278>         Commands can be entered as simple lists, for example:\n"
		"<278>         #show {a};#show {b} will execute a single line as two commands.\n"
		"\n"
		"<278>         Several commands take a simple list as their input, these are:\n"
		"<278>         #foreach, #line substitute, #path load, #list create, and #highlight.\n"
		"\n"
		"<128>         Brace Lists\n"
		"\n"
		"<278>         A brace list is a string in which fields are delimited with braces.\n"
		"<278>         Most commands take a brace list for their arguments, for example:\n"
		"<278>         #session {x} {mud.com} {1234} {mud.tin}. The session command takes\n"
		"<278>         4 arguments, the 4th argument (command file) is optional.\n"
		"\n"
		"<278>         Commands that take a simple list as their input will also accept a\n"
		"<278>         brace list, keep in mind you'll have to embed the brace list in an\n"
		"<278>         extra set of braces, for example: #path load {{n}{s}{w}{w}}, which is\n"
		"<278>         identical to: #path load {n;s;w;w}.\n"
		"\n"
		"<278>         Brace lists cannot be stored as variables because TinTin++ will\n"
		"<278>         confuse them with tables. You can convert a brace list to a table\n"
		"<278>         variable using: #list {bracelist} {create} {{a}{b}{c}} this will look\n"
		"<278>         internally as: {{1}{a}{2}{b}{3}{c}}. You can then convert this table\n"
		"<278>         back to a simple list using: #list {bracelist} {simplify} which will\n"
		"<278>         change it to {a;b;c}.\n"
		"\n"
		"<278>         Braces cannot easily be escaped in TinTin++. Using \\{ or \\} will not\n"
		"<278>         work. The reason for this is due to several factors, but primarily\n"
		"<278>         backward compatibility. To escape braces you must define them using\n"
		"<278>         hexadecimal notation using \\x7B and \\x7D. See #help escape for a list\n"
		"<278>         of escape options, and the help file will also remind you of how to\n"
		"<278>         escape braces.\n"
		"\n"
		"<128>         Tables\n"
		"\n"
		"<278>         Tables are key/value pairs stored within a variable. Tables are also\n"
		"<278>         known as associative arrays, dictionaries, maps, nested variables,\n"
		"<278>         structures, and probably a couple of other names. There are several\n"
		"<278>         ways to create and access tables.\n"
		"\n"
		"<178>Example<278>: #variable {friendlist} {{bob}{bob@mail.com} {bubba}{sunset@gmail.com}}\n"
		"\n"
		"<278>         This will create a friendlist with two entries, the key is the name of\n"
		"<278>         the friend, the value is the email address of the friend. You can see\n"
		"<278>         the email address of bob using: #show {$friendlist[bob]}. You can\n"
		"<278>         also define this table as following:\n"
		"\n"
		"<178>Example<278>:\n"
		"<278>         #variable {friendlist[bob]} {bob@mail.com}\n"
		"<278>         #variable {friendlist[bubba]} {sunset@gmail.com}\n"
		"\n"
		"<278>         This would create the exact same table as the single line declaration\n"
		"<278>         used previously. To see the first key in the table use:\n"
		"<278>         *friendlist[+1], to see the first value in the table use:\n"
		"<278>         $friendlist[+1]. To see the size of the table use &friendlist[]. To\n"
		"<278>         print a bracelist of all friends use *friendlist[], to print a\n"
		"<278>         bracelist of all friends whose name starts with the letter 'a' you\n"
		"<278>         would use: *friendlist[a%*]. Similarly to see the number of friends\n"
		"<278>         you have whose name ends with the letter 'b' you would use:\n"
		"<278>         &friendlist[%*b].\n"
		"\n"
		"<278>         See #help regexp for a brief overview of regular expression options.\n"
		"<278>         While TinTin++ supports PCRE (perl-compatible regular expressions), it\n"
		"<278>         embeds them within its own regular expression syntax that is simpler\n"
		"<278>         and less invasive, while still allowing the full power of PCRE for\n"
		"<278>         those who need it.\n"
		"\n"
		"<178>Example<278>: #unvariable {friendlist[bubba]}\n"
		"\n"
		"<278>         This would remove {bubba} from the friendlist. To remove the entire\n"
		"<278>         friendlist you would use: #unvariable {friendlist}.\n"
		"\n"
		"<178>Example<278>: #variable {friendlist} {{bob} {{email}{bob@ma.il} {phone}{123456789}}}\n"
		"\n"
		"<278>         There is no limit to the number of nests, simply add more braces. To\n"
		"<278>         see Bob's email in this example you would use:\n"
		"<278>         #show {$friendlist[bob][email]}.\n"
		"\n"
		"<278>         To merge two tables the #cat command can be used.\n"
		"<178>Example<278>:\n"
		"<278>         #variable {bli} {{a}{1}{b}{2}}\n"
		"<278>         #variable {blo} {{c}{3}{d}{4}}\n"
		"<278>         #cat {blo} {$bli}\n"
		"\n"
		"<128>         Lists\n"
		"\n"
		"<278>         Tables are sorted alphabetically with the exception of numbers which\n"
		"<278>         are sorted numerically. If you want to determine the sorting order\n"
		"<278>         yourself you can use use the #list command which helps you to use\n"
		"<278>         tables as arrays.\n"
		"\n"
		"<178>Example<278>: #action {%1 chats %2} {#list chats add {%0}}\n"
		"\n"
		"<278>         Each time a chat is received it's added to the end of the 'chats' list\n"
		"<278>         variable. If you type #variable chats this might look like:\n"
		"\n"
		"<278>         <138>#<168>VARIABLE <258>{<178>chats<258>}\n"
		"<278>         <258>{\n"
		"<278>                 <258>{<178>1<258>} {<178>Bubba chats Hi<258>}\n"
		"<278>                 <258>{<178>2<258>} {<178>Bob chats Hi bub<258>}\n"
		"<278>                 <258>{<178>3<258>} {<178>Bubba chats Bye<258>}\n"
		"<278>                 <258>{<178>4<258>} {<178>Bob chats bub bye<258>}\n"
		"<278>         <258>}\n"
		"\n"
		"<128>         Parsing\n"
		"\n"
		"<278>         There are various ways to parse lists and tables, using either #loop,\n"
		"<278>         #foreach, #while, or #<number>.\n"
		"\n"
		"<278>         #loop takes two numeric arguments, incrementing or decrementing the\n"
		"<278>         first number until it matches the second number. The value of the loop\n"
		"<278>         counter is stored in the provided variable.\n"
		"\n"
		"<278>         #foreach takes either a simple list or a brace list as its first\n"
		"<278>         argument. Foreach will go through each item in the list and store the\n"
		"<278>         value in the provided variable.\n"
		"\n"
		"<278>         #while will perform an if check on the first argument, if the result\n"
		"<278>         is true it will execute the commands in the second argument. Then it\n"
		"<278>         performs an if check on the first argument again. It will continue to\n"
		"<278>         repeat until the if check returns 0 or the loop is interrupted with a\n"
		"<278>         control flow command. It takes special care to avoid infinite loops.\n"
		"\n"
		"<278>         #<number> will execute the provided argument 'number' times. For\n"
		"<278>         example: #4 {#show beep! \\a}\n"
		"\n"
		"<278>         Here are some examples.\n"
		"\n"
		"<178>Example<278>: #list friends create {bob;bubba;zorro}\n"
		"\n"
		"<278>         Internally this looks like {{1}{bob}{2}{bubba}{3}{zorro}} and the\n"
		"<278>         list can be parsed in various ways.\n"
		"\n"
		"<178>Example<278>: #foreach {$friends[%*]} {name} {#show $name}\n"
		"\n"
		"<178>Example<278>: #foreach {*friends[%*]} {i} {#show $friends[$i]}\n"
		"\n"
		"<178>Example<278>: #loop {1} {&friends[]} {i} {#show $friends[+$i]}\n"
		"\n"
		"<178>Example<278>: #math i 1;#while {&friends[+$i]} {#show $friends[+$i];\n"
		"<278>         #math i $i + 1}\n"
		"\n"
		"<178>Example<278>: #math i 1;#&friends[] {#show $friends[+$i];#math i $i + 1}\n"
		"\n"
		"<278>         Each of the five examples above performs the same task; printing the\n"
		"<278>         three names in the friends list.\n"
		"\n"
		"<278>         If you want to get a better look at what goes on behind the scenes\n"
		"<278>         while executing scripts you can use '#debug all on'. To stop seeing\n"
		"<278>         debug information use '#debug all off'.\n"
		"\n"
		"<128>         List Tables\n"
		"\n"
		"<278>         List tables are also known as databases and the #list command has\n"
		"<278>         several options to manipulate them.\n"
		"\n"
		"<278>         For these options to work properly all tables need to have identical\n"
		"<278>         keys. Here is an example list table.\n"
		"\n"
		"<278>         #var {friendlist}\n"
		"<278>         {\n"
		"<278>             {1}{{name}{bob} {age}{54}}\n"
		"<278>             {2}{{name}{bubba} {age}{21}}\n"
		"<278>             {3}{{name}{pamela} {age}{36}}\n"
		"<278>         }\n"
		"\n"
		"<278>         To sort the list table by age you would use:\n"
		"\n"
		"<278>         #list friendlist indexate age\n"
		"<278>         #list friendlist order\n"
		"\n"
		"<278>         To remove everyone whose name starts with a 'b' you would use:\n"
		"\n"
		"<278>         #list friendlist indexate name\n"
		"<278>         #list friendlist filter {} {b%*}\n"
		"\n"
		"<278>         The filter option only supports regular expressions. To filter\n"
		"<278>         using mathematics you would loop through the list backwards:\n"
		"\n"
		"<278>         #loop &friendlist[] 1 index\n"
		"<278>         {\n"
		"<278>             #if {$friendlist[+$index][age] < 30}\n"
		"<278>             {\n"
		"<278>                 #list friendlist delete $index\n"
		"<278>             }\n"
		"<278>         }\n"
		"\n"
		"<278>         Alternatively you can use the refine option.\n"
		"\n"
		"<278>         #list friendlist indexate age\n"
		"<278>         #list friendlist refine {&0 >= 30}\n"
		"\n"
		"<278>         To add an item to a list table there are two options:\n"
		"\n"
		"<278>         #list friendlist add {{{name}{hobo} {age}{42}}}\n"
		"<278>         #list friendlist insert -1 {{name}{hobo} {age}{42}}\n"
		"\n"
		"<128>         Optimization\n"
		"\n"
		"<278>         TinTin++ tables are exceptionally fast while they remain under 100\n"
		"<278>         items. Once a table grows beyond 10000 items there can be performance\n"
		"<278>         issues when inserting and removing items in the beginning or middle of\n"
		"<278>         the table.\n"
		"\n"
		"<278>         The plan is to eventually implement an indexable and flexible data\n"
		"<278>         structure for large tables.\n"
		"\n"
		"<278>         If you load a large table from file it's important to make sure it's\n"
		"<278>         sorted, when using #write to save a table it's automatically sorted.\n"
		"\n"
		"<278>         If you notice performance issues on large tables it's relatively easy\n"
		"<278>         to create a hash table.\n"
		"\n"
		"<178>Example<278>:\n"
		"\n"
		"<278>         #alias {sethash}\n"
		"<278>         {\n"
		"<278>             #format hash %H %1;\n"
		"<278>             #math hash1 $hash % 100;\n"
		"<278>             #math hash2 $hash / 100 % 100;\n"
		"<278>             #var hashtable[$hash1][$hash2][%1] %2\n"
		"<278>         }\n"
		"\n"
		"<278>         #function {gethash}\n"
		"<278>         {\n"
		"<278>             #format hash %H %1;\n"
		"<278>             #math hash1 $hash % 100;\n"
		"<278>             #math hash2 $hash / 100 % 100;\n"
		"<278>             #return $hashtable[$hash1][$hash2][%1]\n"
		"<278>         }\n"
		"\n"
		"<278>         #alias {test}\n"
		"<278>         {\n"
		"<278>             sethash bli hey;\n"
		"<278>             sethash bla hi;\n"
		"<278>             sethash blo hello;\n"
		"<278>             #show The value of bla is: @gethash{bla}\n"
		"<278>         }\n"
		"\n"
		"<278>         The above script will rapidly store and retrieve over 1 million items.\n"
		"<278>         Looping through a hash table is relatively easy as well.\n"
		"\n"
		"<178>Example<278>:\n"
		"\n"
		"<278>         #alias {showhash}\n"
		"<278>         {\n"
		"<278>             #foreach {*hashtable[%*]} {hash1}\n"
		"<278>             {\n"
		"<278>                 #foreach {*hashtable[$hash1][%*]} {hash2}\n"
		"<278>                 {\n"
		"<278>                     #echo {%-20s = %s}\n"
		"<278>                                        {hashtable[$hash1][$hash2]}\n"
		"<278>                                        {$hashtable[$hash1][$hash2]}\n"
		"<278>                 }\n"
		"<278>             }\n"
		"        }\n",
		
		"break continue foreach loop parse repeat return while"
	},
	
	{
		"LOCAL",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #local <178>{<278>variable name<178>} {<278>text to fill variable<178>}\n"
		"\n"
		"<278>         The local command sets a local variable. Unlike a regular variable\n"
		"<278>         a local variable will only stay in memory for the duration of the\n"
		"<278>         event that created it. They are accessed in the same way as a\n"
		"<278>         regular variable.\n"
		"\n"
		"<278>         Commands that store information to a variable will use a local variable\n"
		"<278>         if it exists.\n"
		"\n"
		"<278>         Avoid setting the result variable as local in a function. Similarly,\n"
		"<278>         it is best to avoid setting a local variable that is identical to an\n"
		"<278>         existing regular variable.\n"
		"\n"
		"<178>Example<278>: #alias {swap} {#local x %0;#replace x {e} {u};#show $x}\n"
		"\n"
		"<178>Comment<278>: You can remove a local variable with the #unlocal command.\n"
		,
		"format function math replace script variable"
	},

	{
		"LOG",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #log <178>{<278>option<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         The log command allows logging session output to file. You can set the\n"
		"<278>         data type to either plain, raw, or html with the config command.\n"
		"\n"
		"<278>         <178>#log append <filename>\n"
		"<278>           Start logging to the given file, if the file already exists it won't\n"
		"<278>           be overwritten and data will be appended to the end.\n"
		"\n"
		"<278>         <178>#log move <filename_1> <filename_2>\n"
		"<278>           Move filename_1 to filename_2. This can be any file and doesn't need\n"
		"<278>           to be a log file.\n"
		"\n"
		"<278>         <178>#log overwrite <filename>\n"
		"<278>           Start logging to the given file, if the file already exists it will\n"
		"<278>           be overwritten.\n"
		"\n"
		"<278>         <178>#log off\n"
		"<278>           Stop logging.\n"
		"\n"
		"<278>         <178>#log remove <filename>\n"
		"<278>           Remove the file. This can be any file and doesn't need to be a log\n"
		"<278>           file.\n"
		"\n"
		"<278>         <178>#log timestamp <format>\n"
		"<278>           When set the timestamp will be prepended to each line logged to file.\n"
		"<278>           The format will be formatted as a date using the strftime format\n"
		"<278>           specifiers as described in #help time.\n"
		,
		"read scan textin time write"
	},

	{
		"LOOP",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #loop <178>{<278><start><178>} {<278><finish><178>} {<278><variable><178>} {<278>commands<178>}\n"
		"\n"
		"<278>         Like a for statement, loop will loop from start to finish incrementing\n"
		"<278>         or decrementing by 1 each time through.  The value of the loop counter\n"
		"<278>         is stored in the provided variable, which you can use in the commands.\n"
		"\n"
		"<178>Example<278>: #loop 1 3 loop {get all $loop.corpse}\n"
		"<278>         This equals 'get all 1.corpse;get all 2.corpse;get all 3.corpse'.\n"
		"\n"
		"<178>Example<278>: #loop 3 1 cnt {drop $cnt\\.key}\n"
		"<278>         This equals 'drop 3.key;drop 2.key;drop 1.key'.\n"
		,
		"break continue foreach list parse repeat return while"
	},
	{
		"MACRO",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #macro <178>{<278>key sequence<178>} {<278>commands<178>}\n"
		"\n"
		"<278>         Macros allow you to make tintin respond to function keys.\n"
		"\n"
		"<278>         The key sequence send to the terminal when pressing a function key\n"
		"<278>         differs for every OS and terminal. To find out what sequence is sent\n"
		"<278>         you can enable the CONVERT META config option.\n"
		"\n"
		"<278>         Another option is pressing ctrl-v, which will enable CONVERT META for\n"
		"<278>         the next key pressed.\n"
		"\n"
		"<278>         If you only want a key sequence to trigger at the start of an input\n"
		"<278>         line prefix the key sequence with ^.\n"
		"\n"
		"<178>Example<278>: #macro {(press ctrl-v)(press F1)} {#show \\e[2J;#buffer lock}\n"
		"<278>         Clear the screen and lock the window when you press F1, useful when the\n"
		"<278>         boss is near.\n"
		"\n"
		"<178>Example<278>: #macro {\\eOM} {#cursor enter}\n"
		"<278>         Makes the keypad's enter key work as an enter in keypad mode.\n"
		"\n"
		"<178>Example<278>: #macro {^nn} {n}\n"
		"<278>         Makes pressing n twice on an empty line execute north.\n"
		"\n"
		"<178>Comment<278>: Not all terminals properly initialize the keypad key sequences.\n"
		"<278>         If this is the case you can still use the keypad, but instead of the\n"
		"<278>         arrow keys use ctrl b, f, p, and n.\n"
		"\n"
		"<178>Comment<278>: You can remove a macro with the #unmacro command.\n"
		,
		"alias cursor history keypad speedwalk tab"
	},
	{
		"MAP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #map\n"
		"\n"
		"<278>         The map command is the backbone of the auto mapping feature.\n"
		"\n"
		"<278>         <178>#map at <exit|vnum> <command>\n"
		"<278>           Execute the command at the given exit or vnum.\n"
		"\n"
		"<278>         <178>#map center <x> <y> <z>\n"
		"<278>           Sets displaying center of the map viewer, default is 0 0 0.\n"
		"\n"
		"<278>         <178>#map color <field> [value]\n"
		"<278>           Sets the map color for the given color field. Use #map color reset\n"
		"<278>           to restore colors to default.\n"
		"\n"
		"<278>         <178>#map create <size>\n"
		"<278>           Creates a new map and room 1. The default size is 50000 rooms.\n"
		"\n"
		"<278>         <178>#map destroy {area|world} <name>\n"
		"<278>           Deletes the map or given area.\n"
		"\n"
		"<278>         <178>#map delete <exit|vnum>\n"
		"<278>           Deletes the room for the given exit or vnum.\n"
		"\n"
		"<278>         <178>#map dig <exit|vnum> [new|<vnum>]\n"
		"<278>           Creates an exit for the given exit name. If no valid exit name\n"
		"<278>           is given or no existing room is found a new room is created.\n"
		"<278>           Useful for portal links and other alternative forms of\n"
		"<278>           transportation. If the 'new' argument is provided all existing\n"
		"<278>           rooms are ignored and a new room is created. If a room vnum is\n"
		"<278>           given as the second argument an exit will be created leading\n"
		"<278>           to the given room vnum. If the room vnum doesn't exist a new\n"
		"<278>           room is created.\n"
		"\n"
		"<278>         <178>#map entrance <exit> <option> <arg> [both]\n"
		"<278>           Set the entrance data for the given exit. You must specify a\n"
		"<278>           valid two-way exit for this to work.\n"
		"\n"
		"<278>         <178>#map exit <exit> <option> <arg> [both]\n"
		"<278>           Set the exit data. Useful with a closed door where you can\n"
		"<278>           set the exit command: '#map exit e command {open east;e}'.\n"
		"<278>           Use #map exit <exit> for a list of available options.\n"
		"<278>           Use #map exit <exit> save to save all exit data.\n"
		"\n"
		"<278>         <178>#map exitflag <exit> <AVOID|BLOCK|HIDE|INVIS> [on|off]\n"
		"<278>           Set exit flags. See #map roomflag for more info.\n"
		"\n"
		"<278>         <178>#map explore <exit>\n"
		"<278>           Explores the given exit until a dead end or an\n"
		"<278>           intersection is found. The route is stored in #path and can\n"
		"<278>           subsequently be used with #walk. Useful for long roads.\n"
		"\n"
		"<278>         <178>#map find <name> <exits> <desc> <area> <note> <terrain> <flag>\n"
		"<278>           searches for the given room name. If found the shortest path\n"
		"<278>           from your current location to the destination is calculated.\n"
		"<278>           The route is stored in #path and can subsequently be used with\n"
		"<278>           the various #path commands. If #map flag nofollow is set it\n"
		"<278>           will store the exit commands instead of the exit names.\n"
		"\n"
		"<278>           If <exits> is provided all exits must be matched, if\n"
		"<278>           <roomdesc>, <roomarea> or <roomnote> or <roomterrain> or\n"
		"<278>           <roomflag> is provided these are matched as well against the\n"
		"<278>           room to be found.\n"
		"\n"
		"<278>           These search options are also available for the at, delete,\n"
		"<278>           goto, link, list and run commands.\n"
		"\n"
		"<278>         <178>#map flag asciigraphics\n"
		"<278>           Takes up more space but draws a more detailed\n"
		"<278>           map that displays the ne se sw nw exits and room symbols.\n"
		"\n"
		"<278>         <178>#map flag asciivnums\n"
		"<278>           Display room vnums if asciigraphics is enabled.\n"
		"\n"
		"<278>         <178>#map flag direction\n"
		"<278>           Display an arrow on the map showing the direction of your\n"
		"<278>           last movement command.\n"
		"\n"
		"<278>         <178>#map flag fast\n"
		"<278>           Limit coordinate searches to a 50 room radius. Useful to\n"
		"<278>           speed up map drawing and room creation on large maps.\n"
		"\n"
		"<278>         <178>#map flag nofollow\n"
		"<278>           When you enter movement commands the map will no longer\n"
		"<278>           automatically follow along. Useful for MSDP and GMCP\n"
		"<278>           automapping scripts. When you use #map find in nofollow\n"
		"<278>           mode it will store the exit command instead of the exit\n"
		"<278>           name into the path.\n"
		"\n"
		"<278>         <178>#map flag pancake\n"
		"<278>           Makes the map display rooms above or below you. You can use\n"
		"<278>           #map color room <aaa><fff> for a color gradient.\n"
		"\n"
		"<278>         <178>#map flag quiet\n"
		"<278>           Silence map messages when creating new rooms through movement.\n"
		"\n"
		"<278>         <178>#map flag static\n"
		"<278>           Will make the map static so new rooms are no longer\n"
		"<278>           created when walking into an unmapped direction. Useful when\n"
		"<278>           you're done mapping and regularly bump into walls accidentally\n"
		"<278>           creating a new room. #map dig etc will still work.\n"
		"\n"
		"<278>         <178>#map flag symbolgraphics\n"
		"<278>           Draw a 1x1 map using the defined room symbols.\n"
		"\n"
		"<278>         <178>#map flag terrain\n"
		"<278>           Fill up empty space surrounding rooms with terrain symbols\n"
		"\n"
		"<278>         <178>#map flag vtgraphics\n"
		"<278>           Enables vt line drawing on some terminals\n"
		"\n"
		"<278>         <178>#map flag vtmap\n"
		"<278>           Will enable the vtmap which is shown in the top split\n"
		"<278>           screen if you have one. You can create a 16 rows high top\n"
		"<278>           screen by using '#split 16 1'.\n"
		"\n"
		"<278>         <178>#map get <option> <variable> [vnum]\n"
		"<278>           Store a map value into a variable, if no vnum is given the\n"
		"<278>           current room is used. Use 'all' as the option to store all\n"
		"<278>           values as a table.\n"
		"\n"
		"<278>         <178>#map get roomexits <variable>\n"
		"<278>           Store all room exits into variable.\n"
		"\n"
		"<278>         <178>#map global <room vnum>\n"
		"<278>           Set the vnum of a room that contains global\n"
		"<278>           exits, for example an exit named 'recall' that leads to the\n"
		"<278>           recall location. The room can contain multiple exits, in case\n"
		"<278>           there are multiple commands that are similar to recall.\n"
		"\n"
		"<278>         <178>#map goto <room vnum> [dig]\n"
		"<278>           Takes you to the given room vnum, with the\n"
		"<278>           dig argument a new room will be created if none exists.\n"
		"\n"
		"<278>         <178>#map goto <name> <exits> <desc> <area> <note> <terrain>\n"
		"<278>           Takes you to the given room name, if you provide exits those\n"
		"<278>           must match.\n"
		"\n"
		"<278>         <178>#map info [save]\n"
		"<278>           Gives information about the map and room you are in. If the save\n"
		"<278>           argument is given the map data is saved to the info[map] variable.\n"
		"\n"
		"<278>         <178>#map insert <direction> [roomflag]\n"
		"<278>           Insert a room in the given direction. Most useful for inserting\n"
		"<278>           void rooms.\n"
		"\n"
		"<278>         <178>#map jump <x> <y> <z>\n"
		"<278>           Jump to the given coordinate, which is relative\n"
		"<278>           to your current room.\n"
		"\n"
		"<278>         <178>#map landmark <name> <vnum> [description] [size]\n"
		"<278>           Creates an alias to target the provided room vnum. The\n"
		"<278>           description is optional and should be brief. The size\n"
		"<278>           determines from how many rooms away the landmark can be\n"
		"<278>           seen.\n"
		"\n"
		"<278>         <178>#map leave\n"
		"<278>           Makes you leave the map. Useful when entering a maze. You\n"
		"<278>           can return to your last known room using #map return.\n"
		"\n"
		"<278>         <178>#map legend <legend> [symbols|reset]\n"
		"<278>         <178>#map legend <legend> <index> [symbol]\n"
		"<278>           There are several legends and sub-legends available for\n"
		"<278>           drawing maps to suit personal preference and character sets.\n"
		"<278>           Use #map legend all to see the legend as currently defined.\n"
		"<278>           Use #map legend <legend> <reset> to set the default legend.\n"
		"<278>           Use #map legend <legend> <character list> to create a custom\n"
		"<278>           legend. Custom legends are stored in the map file and can be\n"
		"<278>           saved and loaded using #map write and #map read.\n"
		"\n"
		"<278>         <178>#map link <direction> <room name> [both]\n"
		"<278>           Links two rooms. If the both\n"
		"<278>           argument and a valid direction is given the link is two ways.\n"
		"\n"
		"<278>         <178>#map list <name> <exits> <desc> <area> <note> <terrain>\n"
		"<278>           Lists all matching rooms and their distance. The following\n"
		"<278>           search keywords are supported.\n"
		"\n"
		"<278>           {distance}    <arg> will list rooms within given distance.\n"
		"<278>           {roomarea}    <arg> will list rooms with matching area name.\n"
		"<278>           {roomdesc}    <arg> will list rooms with matching room desc.\n"
		"<278>           {roomexits}   <arg> will list rooms with identical room exits.\n"
		"<278>                               Use * as an exit to ignore non pathdir exits.\n"
		"<278>           {roomflag}    <arg> will list rooms with matching room flags.\n"
		"<278>           {roomid}      <arg> will list rooms with identical id name.\n"
		"<278>           {roomname}    <arg> will list rooms with matching room name.\n"
		"<278>           {roomnote}    <arg> will list rooms with matching room note.\n"
		"<278>           {roomterrain} <arg> will list rooms with matching room terrain.\n"
		"<278>           {variable}    <arg> will save the output to given variable.\n"
		"\n"
		"<278>         <178>#map map <rows> <cols> <append|overwrite|list|variable> <name>\n"
		"<278>           Display a drawing of the map of the given height and width.\n"
		"<278>           All arguments are optional. If {rows} or {cols} are set to {}\n"
		"<278>           or {0} they will use the scrolling window size as the default.\n"
		"<278>           If {rows} or {cols} are a negative number this number is\n"
		"<278>           subtracted from the scrolling window size.\n"
		"\n"
		"<278>         <178>#map map <rows> <cols> draw <square>\n"
		"<278>           Display a drawing of the map of the given height and width.\n"
		"<278>           The square argument exists of 4 numbers formulating the top\n"
		"<278>           left corner and bottom right corner of a square.\n"
		"\n"
		"<278>           If you use {append|overwrite} the map is written to the specified\n"
		"<278>           file name which must be given as the 4th argument.\n"
		"<278>           If you use {list|variable} the map is saved to the specified\n"
		"<278>           variable name.\n"
		"\n"
		"<278>         <178>#map move <direction>\n"
		"<278>           This does the same as an actual movement command, updating your\n"
		"<278>           location on the map and creating new rooms. Useful when you are\n"
		"<278>           following someone and want the map to follow along. You will need\n"
		"<278>           to create actions using '#map move', for this to work.\n"
		"\n"
		"<278>         <178>#map offset <row> <col> <row> <col>\n"
		"<278>           Define the offset of the vtmap as a square. Without an argument\n"
		"<278>           it defaults to the entire top split region.\n"
		"\n"
		"<278>         <178>#map read <filename>\n"
		"<278>           Will load the given map file.\n"
		"\n"
		"<278>         <178>#map resize <size>\n"
		"<278>           Resize the map, setting the maximum number of rooms.\n"
		"\n"
		"<278>         <178>#map return\n"
		"<278>           Returns you to your last known room after leaving the map\n"
		"<278>           or loading a map.\n"
		"\n"
		"<278>         <178>#map roomflag <flags> <get|on|off>\n"
		"<278>         \n"
		"<278>         <178>#map roomflag avoid\n"
		"<278>           When set, '#map find' will avoid a route leading\n"
		"<278>           through that room. Useful for locked doors, etc.\n"
		"<278>         <178>#map roomflag block\n"
		"<278>           When set the automapper will prevent movement into or through\n"
		"<278>           the room. Useful for death traps.\n"
		"<278>         <178>#map roomflag hide\n"
		"<278>           When set, '#map' will not display the map beyond\n"
		"<278>           this room. When mapping overlapping areas or areas that aren't\n"
		"<278>           build consistently you need this flag as well to stop\n"
		"<278>           auto-linking, unless you use void rooms.\n"
		"<278>         <178>#map roomflag invis\n"
		"<278>           When set the room will be colored with the INVIS color.\n"
		"<278>         <178>#map roomflag leave\n"
		"<278>           When entering a room with this flag, you will\n"
		"<278>           automatically leave the map. Useful when set at the entrance\n"
		"<278>           of an unmappable maze.\n"
		"<278>         <178>#map roomflag noglobal\n"
		"<278>           This marks a room as not allowing global\n"
		"<278>           transportation, like norecall rooms that block recall.\n"
		"<278>         <178>#map roomflag void\n"
		"<278>           When set the room becomes a spacing room that can\n"
		"<278>           be used to connect otherwise overlapping areas. A void room\n"
		"<278>           should only have two exits. When entering a void room you are\n"
		"<278>           moved to the connecting room until you enter a non void room.\n"
		"<278>         <178>#map roomflag static\n"
		"<278>           When set the room will no longer be autolinked\n"
		"<278>           when walking around. Useful for mapping mazes.\n"
		"\n"
		"<278>         <178>#map run <room name> [delay]\n"
		"<278>           Calculates the shortest path to the destination and walks you\n"
		"<278>           there. The delay is optional and requires using braces. Besides\n"
		"<278>           the room name a list of exits can be provided for more precise\n"
		"<278>           matching.\n"
		"\n"
		"<278>         <178>#map set <option> <value> [vnum]\n"
		"<278>           Set a map value for your current room, or given room if a room\n"
		"<278>           vnum is provided.\n"
		"\n"
		"<278>         <178>#map sync <filename>\n"
		"<278>           Similar to #map read except the current map won't be unloaded\n"
		"<278>           or overwritten.\n"
		"\n"
		"<278>         <178>#map terrain <name> <symbol> [flag]\n"
		"<278>           Set the terrain symbol and flag.\n"
		"\n"
		"<278>         <178>#map terrain <name> <symbol> [DENSE|SPARSE|SCANT]\n"
		"<278>           Determine symbol density, omit for the default.\n"
		"\n"
		"<278>         <178>#map terrain <name> <symbol> [NARROW|WIDE|VAST]\n"
		"<278>           Determine symbol spread range, omit for the default.\n"
		"\n"
		"<278>         <178>#map terrain <name> <symbol> [FADEIN|FADEOUT]\n"
		"<278>           Determine symbol spread density, omit for the default.\n"
		"\n"
		"<278>         <178>#map terrain <name> <symbol> [DOUBLE]\n"
		"<278>           You're using two characters for the symbol.\n"
		"\n"
		"<278>         <178>#map travel <direction> <delay>\n"
		"<278>           Follows the direction until a dead end or an intersection is\n"
		"<278>           found. Use braces around the direction if you use the delay,\n"
		"<278>           which will add the given delay between movements.\n"
		"<278>           Use #path stop to stop a delayed run.\n"
		"\n"
		"<278>         <178>#map undo\n"
		"<278>           Will undo your last move. If this created a room or a link\n"
		"<278>           they will be deleted, otherwise you'll simply move back a\n"
		"<278>           room. Useful if you walked into a non-existent direction.\n"
		"\n"
		"<278>         <178>#map uninsert <direction>\n"
		"<278>           Exact opposite of the insert command.\n"
		"\n"
		"<278>         <178>#map unlandmark <name>\n"
		"<278>           Removes a landmark.\n"
		"\n"
		"<278>         <178>#map unlink <direction> [both]\n"
		"<278>           Will remove the exit, this isn't two way so you can have the\n"
		"<278>           properly display no exit rooms and mazes.\n"
		"<278>           If you use the both argument the exit is removed two-ways.\n"
		"\n"
		"<278>         <178>#map unterrain <name>\n"
		"<278>           Removes a terrain.\n"
		"\n"
		"<278>         <178>#map update [now]\n"
		"<278>           Sets the vtmap to update within the next 0.1 seconds, or\n"
		"<278>           instantly with the now argument.\n"
		"\n"
		"<278>         <178>#map vnum <low> [high]\n"
		"<278>           Change the room vnum to the given number, if a range is\n"
		"<278>           provided the first available room in that range is selected.\n"
		"\n"
		"<278>         <178>#map write <filename> [force]\n"
		"<278>           Will save the map, if you want to save a map to a .tin file\n"
		"<278>           you must provide the {force} argument.\n"
		,
		"path pathdir speedwalk"
	},

	{
		"MAPPING",
		TOKEN_TYPE_STRING,
		"\n"
		"<278>         TinTin++ has a powerful automapper that uses a room system similar to\n"
		"<278>         Diku MUDs which means that odd map layouts and weird exit\n"
		"<278>         configurations aren't a problem. The mapper provides tools to improve\n"
		"<278>         the visual map display. For basic path tracking see #help PATH.\n"
		"\n"
		"<178>         #map create [size]\n"
		"\n"
		"<278>         This command creates the initial map. The size is 50,000 by default\n"
		"<278>         and can be changed at any time with the #map resize command. If you\n"
		"<278>         play a MUD that uses MSDP or GMCP to provide room numbers you'll have\n"
		"<278>         to increase it to the highest reported room number. Increasing the\n"
		"<278>         size of the map doesn't decrease performance.\n"
		"\n"
		"<178>         #map goto <location>\n"
		"\n"
		"<278>         When you create the map you are not automatically inside the map. By\n"
		"<278>         default room number (vnum) 1 is created, so you can go to it using\n"
		"<278>         #map goto 1. Once you are inside the map new rooms are automatically\n"
		"<278>         created as you move around. Movement commands are defined with the\n"
		"<278>         pathdir command. By default n, ne, e, se, s, sw, w, nw, u, d are\n"
		"<278>         defined.\n"
		"\n"
		"<278>         <178>#map map <rows> <cols> <append|overwrite|list|variable> <name>\n"
		"\n"
		"<278>         To see the map you can use #map map. It's annoying to have to\n"
		"<278>         constantly type #map map however. Instead it's possible to use #split\n"
		"<278>         to display a vt100 map. To do so execute:\n"
		"\n"
		"<278>         <178>#split 16 1\n"
		"<278>         #map flag vtmap on\n"
		"\n"
		"<278>         The first command sets the top split lines to 16 and the bottom split\n"
		"<278>         line to 1. If you want a smaller or larger map display you can use a\n"
		"<278>         different value than 16.\n"
		"\n"
		"<278>         If you don't need to display diagonal exits and prefer a more compact\n"
		"<278>         look you can use #map flag AsciiGraphics off. This will enable the\n"
		"<278>         standard display which uses UTF-8 box drawing characters, results may\n"
		"<278>         vary depending on the font used.\n"
		"\n"
		"<278>         If your terminal supports UTF-8 you can also give #map flag unicode on\n"
		"<278>         a try.\n"
		"\n"
		"<278>         If you want to display the map in a different location of the screen\n"
		"<278>         use something like:\n"
		"\n"
		"<278>         <178>#split 0 1 0 -80\n"
		"<278>         #map offset 1 81 -4 -1\n"
		"\n"
		"<278>         This will display the map on the right side of the screen, if the\n"
		"<278>         width of the screen is wide enough.\n"
		"\n"
		"<278>         <178>#map undo\n"
		"\n"
		"<278>         If you accidentally walk into the wall on your MUD the mapper will\n"
		"<278>         still create a new room. You can easily fix this mistake by using\n"
		"<278>         #map undo. If you want to move around on the map without moving around\n"
		"<278>         on the MUD you can use: #map move {direction}. To delete a room\n"
		"<278>         manually you can use: #map delete {direction}. To create a room\n"
		"<278>         manually you can use: #map dig {direction}.\n"
		"\n"
		"<278>         <178>#map write <filename>\n"
		"\n"
		"<278>         You can save your map using #map write, to load a map you can use\n"
		"<278>         #map read <filename>.\n"
		"\n"
		"<278>         <178>#map set <option> <value>\n"
		"\n"
		"<278>         You can set the room name using #map set roomname <name>. You either\n"
		"<278>         have to do this manually or create triggers to set the room name\n"
		"<278>         automatically. Once the room name is set you can use #map goto with\n"
		"<278>         the room name to visit it. If there are two rooms with the same name\n"
		"<278>         #map goto will go to the most nearby room. If you want to always go\n"
		"<278>         to the same room you should memorize the room number or create a\n"
		"<278>         landmark.\n"
		"\n"
		"<278>         <178>#map landmark firstroom 1\n"
		"\n"
		"<278>         You can further narrow down the matches by providing additional\n"
		"<278>         arguments, for example:\n"
		"\n"
		"<278>         <178>#map goto {dark alley} {roomexits} {n;e} {roomarea} {Haddock Ville}\n"
		"\n"
		"<278>         You can set the room weight using #map set roomweight {value}. The\n"
		"<278>         weight by default is set to 1.0 and it represents the difficulty of\n"
		"<278>         traversing the room. If you have a lake as an alternative route, and\n"
		"<278>         traversing water rooms is 4 times slower than regular rooms, then you\n"
		"<278>         could set the weight of the lake rooms to 4.0. If the lake is 3 rooms\n"
		"<278>         wide the total weight is 12. If walking around the lake has a weight\n"
		"<278>         less than 12 the mapper will go around the lake, if the weight is\n"
		"<278>         greater than 12 the mapper will take a route through the lake.\n"
		"\n"
		"<278>         You can set the room symbol using #map set roomsymbol {value}. The\n"
		"<278>         symbol should be one, two, or three characters, which can be\n"
		"<278>         colorized. You can for example mark shops with an 'S' and colorize the\n"
		"<278>         'S' depending on what type of shop it is.\n"
		"\n"
		"<278>         <178>#map run <location> <delay>\n"
		"\n"
		"<278>         The run command will have tintin find the shortest path to the given\n"
		"<278>         location and execute the movement commands to get there. You can\n"
		"<278>         provide a delay in seconds with floating point precision, for example:\n"
		"\n"
		"<278>         <178>#map run {dark alley} {0.5}\n"
		"\n"
		"<278>         This will make you walk towards the nearest dark alley with 0.5 second\n"
		"<278>         intervals. Typical MUDs accept commands at 0.25 second intervals.\n"
		"\n"
		"<278>         <178>#map insert {direction} {flag}\n"
		"\n"
		"<278>         The insert command is useful for adding spacer rooms called void rooms.\n"
		"<278>         Often rooms overlap, and by adding void rooms you can stretch out\n"
		"<278>         exits. For example: #map insert north void. You cannot enter void rooms\n"
		"<278>         once they've been created, so you'll have to use #map info in an\n"
		"<278>         adjacent room to find the room vnum, then use #map goto {vnum} to\n"
		"<278>         visit.\n"
		"\n"
		"<278>         It's also possible to align rooms using void rooms. This is easily\n"
		"<278>         done using #map insert north void.\n"
		,
		"map path pathdir"
	},

	{
		"MATH",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #math <178>{<278>variable<178>} {<278>expression<178>}\n"
		"\n"
		"<278>         Performs math operations and stores the result in a variable.  The math\n"
		"<278>         follows a C-like precedence, as follows, with the top of the list\n"
		"<278>         having the highest priority.\n"
		"\n"
		"<278>         Operators       Priority     Function\n"
		"<278>         ------------------------------------------------\n"
		"<278>         !               0            logical not\n"
		"<278>         ~               0            bitwise not\n"
		"<278>         d               1            integer random dice\n"
		"<278>         *               2            integer multiply\n"
		"<278>         **              2            integer power\n"
		"<278>         /               2            integer divide\n"
		"<278>         //              2            integer sqrt // 2 or cbrt // 3\n"
		"<278>         %               2            integer modulo\n"
		"<278>         +               3            integer addition\n"
		"<278>         -               3            integer subtraction\n"
		"<278>         <<              4            bitwise shift\n"
		"<278>         >>              4            bitwise shift\n"
		"<278>         ..              4            integer range\n"
		"<278>         >               5            logical greater than\n"
		"<278>         >=              5            logical greater than or equal\n"
		"<278>         <               5            logical less than\n"
		"<278>         <=              5            logical less than or equal\n"
		"<278>         ==              6            logical equal (can use regex)\n"
		"<278>         ===             6            logical equal (never regex)\n"
		"<278>         !=              6            logical not equal (can use regex)\n"
		"<278>         !==             6            logical not equal (never regex)\n"
		"<278>          &              7            bitwise and\n"
		"<278>          ^              8            bitwise xor\n"
		"<278>          |              9            bitwise or\n"
		"<278>         &&             10            logical and\n"
		"<278>         ^^             11            logical xor\n"
		"<278>         ||             12            logical or\n"
		"<278>         ?              13            logical ternary if (unfinished code)\n"
		"<278>         :              14            logical ternary else \n"
		"\n"
		"<278>         True is any non-zero number, and False is zero.  Parentheses () have\n"
		"<278>         highest precedence, so inside the () is always evaluated first.\n"
		"\n"
		"<278>         Strings must be enclosed in \" \" or { } and in the case of an == or\n"
		"<278>         != operation a regex is performed with the regular expression in the\n"
		"<278>         right-hand string. In the case of a <= or >= operation the alphabetic\n"
		"<278>         order is compared.\n"
		"\n"
		"<278>         The #if and #switch commands use #math. Several commands accepting\n"
		"<278>         numeric input allow math operations as well, such as #delay.\n"
		"\n"
		"<278>         Floating point precision is added by using the decimal . operator or\n"
		"<278>         using #format with the %f flag character.\n"
		"\n"
		"<178>Example<278>: #math {heals} {$mana / 40}\n"
		"<278>         Assuming there is a variable $mana, divides its value by 40 and stores\n"
		"<278>         the result in $heals.\n"
		"\n"
		"<178>Example<278>: #action {^You receive %0 experience} {updatexp %0}\n"
		"<278>         #alias updatexp {#math {xpneed} {$xpneed - %0}\n"
		"<278>         Let's say you have a variable which stores xp needed for your next\n"
		"<278>         level.  The above will modify that variable after every kill, showing\n"
		"<278>         the amount still needed.\n"
		"\n"
		"<178>Example<278>: #action {%0 tells %1}\n"
		"<278>           {#if {{%0} == {Bubba} && $afk} {reply I'm away, my friend.}}\n"
		"<278>         When you are away from keyboard, it will only reply to your friend.\n"
		,
		"cat format function local mathematics replace script variable"
	},

	{
		"MATHEMATICS",
		TOKEN_TYPE_STRING,
		"<278>         <178>Number operations\n"
		"\n"
		"<278>         Operators       Priority     Function\n"
		"<278>         ------------------------------------------------\n"
		"<278>         !               0            logical not\n"
		"<278>         ~               0            bitwise not\n"
		"<278>         *               1            integer multiply\n"
		"<278>         **              1            integer power\n"
		"<278>         /               1            integer divide\n"
		"<278>         //              1            integer sqrt // 2 or cbrt // 3\n"
		"<278>         %               1            integer modulo\n"
		"<278>         d               1            integer random dice roll\n"
		"<278>         +               2            integer addition\n"
		"<278>         -               2            integer subtraction\n"
		"<278>         <<              3            bitwise shift\n"
		"<278>         >>              3            bitwise shift\n"
		"<278>         >               4            logical greater than\n"
		"<278>         >=              4            logical greater than or equal\n"
		"<278>         <               4            logical less than\n"
		"<278>         <=              4            logical less than or equal\n"
		"<278>         ==              5            logical equal\n"
		"<278>         !=              5            logical not equal\n"
		"<278>          &              6            bitwise and\n"
		"<278>          ^              7            bitwise xor\n"
		"<278>          |              8            bitwise or\n"
		"<278>         &&              9            logical and\n"
		"<278>         ^^             10            logical xor\n"
		"<278>         ||             11            logical or\n"
		"\n"
		"<278>         Operator priority can be ignored by using parentheses, for example\n"
		"<278>         (1 + 1) * 2 equals 4, while 1 + 1 * 2 equals 3.\n"
		"\n"
		"<278>         <178>String operations\n"
		"\n"
		"<278>         Operators       Priority     Function\n"
		"<278>         ------------------------------------------------\n"
		"<278>         >               4            alphabetical greater than\n"
		"<278>         >=              4            alphabetical greater than or equal\n"
		"<278>         <               4            alphabetical less than\n"
		"<278>         <=              4            alphabetical less than or equal\n"
		"<278>         ==              5            alphabetical equal using regex\n"
		"<278>         !=              5            alphabetical not equal using regex\n"
		"<278>         ===             5            alphabetical equal\n"
		"<278>         !==             5            alphabetical not equal\n"
		"\n"
		"<278>         Strings must be encased in double quotes or braces. The > >= < <=\n"
		"<278>         operators perform basic string comparisons. The == != operators perform\n"
		"<278>         regular expressions, with the argument on the left being the string,\n"
		"<278>         and the argument on the right being the regex. For example\n"
		"<278>         {bla} == {%*a} would evaluate as 1.\n"
		,
		"math regexp"
	},

	{
		"MESSAGE",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #message <178>{<278>listname<178>} {<278>on<178>|<278>off<178>}\n"
		"\n"
		"<278>         This will show the message status of all your lists if typed without an\n"
		"<278>         argument. If you set for example VARIABLES to OFF you will no longer be\n"
		"<278>         spammed when correctly using the #VARIABLE and #UNVARIABLE commands.\n"
		,
		"class debug ignore info kill"
	},

	{
		"METRIC SYSTEM",
		TOKEN_TYPE_STRING,
		"<278>         The #math command supports using 1K, 1M, 1m, and 1u to make large and\n"
		"<278>         small number handling a little easier. These are case sensitive. Only\n"
		"<278>         four symbols are supported to keep false positives to a minimum.\n"
		"\n"
		"<268>         ╭─────────┬────────┬─────────────────────────────────╮\n"
		"<268>         <268>│<178>    Name <268>│<178> Symbol <268>│<178>                           Factor<268>│\n"
		"<268>         ├─────────┼────────┼─────────────────────────────────┤\n"
//		"<268>         │<178>   Yotta <268>│<178>      Y <268>│<178>1 000 000 000 000 000 000 000 000<268>│\n"
//		"<268>         │<178>   Zetta <268>│<178>      Z <268>│<178>    1 000 000 000 000 000 000 000<268>│\n"
//		"<268>         │<178>     Exa <268>│<178>      E <268>│<178>        1 000 000 000 000 000 000<268>│\n"
//		"<268>         │<178>    Peta <268>│<178>      P <268>│<178>            1 000 000 000 000 000<268>│\n"
//		"<268>         │<178>    Tera <268>│<178>      T <268>│<178>                1 000 000 000 000<268>│\n"
//		"<268>         │<178>    Giga <268>│<178>      G <268>│<178>                    1 000 000 000<268>│\n"
		"<268>         │<178>    Mega <268>│<178>      M <268>│<178>                        1 000 000<268>│\n"
		"<268>         │<178>    Kilo <268>│<178>      K <268>│<178>                            1 000<268>│\n"
		"<268>         │<178>         <268>│<178>        <268>│<178>                                 <268>│\n"
		"<268>         │<178>   milli <268>│<178>      m <268>│<178>                            0.001<268>│\n"
		"<268>         │<178>   micro <268>│<178>      u <268>│<178>                        0.000 001<268>│\n"
//		"<268>         │<178>    nano <268>│<178>      n <268>│<178>                    0.000 000 001<268>│\n"
//		"<268>         │<178>    pico <268>│<178>      p <268>│<178>                0.000 000 000 001<268>│\n"
//		"<268>         │<178>   femto <268>│<178>      f <268>│<178>            0.000 000 000 000 001<268>│\n"
//		"<268>         │<178>    atto <268>│<178>      a <268>│<178>        0.000 000 000 000 000 001<268>│\n"
//		"<268>         │<178>   zepto <268>│<178>      z <268>│<178>    0.000 000 000 000 000 000 001<268>│\n"
//		"<268>         │<178>   yocto <268>│<178>      y <268>│<178>0.000 000 000 000 000 000 000 001<268>│\n"
		"<268>         ╰─────────┴────────┴─────────────────────────────────╯\n"
		,
		"echo format math"
	},

	{
		"MOUSE",
		TOKEN_TYPE_STRING,
		"\n"
		"<278>         To enable xterm mouse tracking use #CONFIG MOUSE ON.\n"
		"\n"
		"<278>         To see mouse events as they happen use #CONFIG MOUSE INFO. This\n"
		"<278>         information can then be used to create mouse events with the #event\n"
		"<278>         command and buttons with the #button command.\n"
		"\n"
		"<278>         Visual buttons and pop-ups can be drawn on the screen with the #draw\n"
		"<278>         command.\n"
		"\n"
		"<278>         The input field can be changed and renamed using #screen inputregion,\n"
		"<278>         which allows creating named events for enter handling.\n"
		"\n"
		"<278>         Links can be created using the MSLP protocol which will generate link\n"
		"<278>         specific events when clicked.\n"
		"\n",
		
		"button draw event MSLP"
	},

	{
		"MSDP",
		TOKEN_TYPE_STRING,
		"\n"
		"<278>         MSDP (Mud Server Data Protocol) is part of the #port functionality.\n"
		"<278>         See #help event for additional documentation as all MSDP events are\n"
		"<278>         available as regular events.\n"
		"\n"
		"<278>         Available MSDP events can be queried using the MSDP protocol\n"
		"<278>         as described in the specification.\n"
		"\n"
		"<278>         <168>https://tintin.sourceforge.io/protocols/msdp\n"
		,
		"event port"
	},

	{
		"MSLP",
		TOKEN_TYPE_STRING,
		"\n"
		"<278>         MSLP (Mud Server Link Protocol) requires enabling #config mouse on,\n"
		"<278>         and creating the appropriate LINK events.\n"
		"\n"
		"<278>         The simplest link can be created by surrounding a keyword with the\n"
		"<278>         \\e[4m and \\e[24m tags.\n"
		"\n"
		"<178>Example<278>: #substitute {\\b{n|e|s|w|u|d}\\b} {\\e[4m%1\\e[24m}\n"
		"\n"
		"<278>         This would display 'Exits: n, e, w.' as 'Exits: \e[4mn\e[24m, \e[4me\e[24m, \e[4mw\e[24m.'.\n"
		"\n"
		"<278>         When clicked this would trigger the PRESSED LINK MOUSE BUTTON ONE\n"
		"<278>         event of which %4 will hold the link command and %6 holds the\n"
		"<278>         link name, which in the case of a simple link will be empty.\n"
		"\n"
		"<178>Example<278>: #event {PRESSED LINK MOUSE BUTTON ONE} {#send {%4}}\n"
		"\n"
		"<278>         Keep in mind that if you change PRESSED to DOUBLE-CLICKED the link\n"
		"<278>         will only work if the text does not scroll in between clicks.\n"
		"\n"
		"<278>         If you want to create a complex link use an OSC code.\n"
		"\n"
		"<178>Example<278>: #sub {\\bsmurf\\b} {\\e]68;1;;say I hate smurfs!\\a\\e[4m%0\\e[24m}\n"
		"\n"
		"<278>         If you have the LINK event of the previous example set, the %4\n"
		"<278>         argument will contain 'say I hate smurfs!'.\n"
		"\n"
		"<178>Example<278>: #sub {\\bgoblin\\b} {\\e]68;1;SEND;kill goblin\\a\\e[4m%0\\e[24m}\n"
		"\n"
		"<278>         Notice the previous instance of ;; has been replaced with ;SEND;\n"
		"<278>         which will name the link. This will generate a named event.\n"
		"\n"
		"<178>Example<278>: #event {PRESSED LINK SEND MOUSE BUTTON ONE} {#send {%4}}\n"
		"\n"
		"<278>         By naming links you can organize things a little bit better instead\n"
		"<278>         of tunneling everything through the same event.\n"
		"\n"
		"<278>         Keep in mind that the server is allowed to use \\e]68;1;\\a as well,\n"
		"<278>         subsequently various security measures are in place.\n"
		"\n"
		"<278>         To create secure links, which are filtered out when send by a server,\n"
		"<278>         you need to use \\e]68;2;\\a, and they instead trigger the SECURE LINK\n"
		"<278>         event.\n"
		"\n"
		"<178>Example<278>: #sub {%* tells %*} {\\e]68;2;EXEC;#cursor set tell %1 \\a\\e[4m%0\\e[24m}\n"
		"<178>       <278>  #event {PRESSED SECURE LINK EXEC MOUSE BUTTON ONE} {%4}\n"
		"\n"
		"<278>         This would make you start a reply when clicking on a tell.\n"
		"\n"
		"<178>Website<278>: https://tintin.mudhalla.net/protocols/mslp\n"
		,
		"event port"
	},

	{
		"NOP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #nop <178>{<278>whatever<178>}\n"
		"\n"
		"<278>         Short for 'no operation', and is ignored by the client.  It is useful\n"
		"<278>         for commenting in your coms file, any text after the nop and before a\n"
		"<278>         semicolon or end of line is ignored. You shouldn't put braces { } in it\n"
		"<278>         though, unless you close them properly.\n"
		"\n"
		"<278>         A valid alternative for #nop is #0.\n"
		"\n"
		"<178>Comment<278>: By using braces you can comment out multiple lines of code in a script\n"
		"<278>         file.\n"
		"\n"
		"<278>         For commenting out an entire trigger and especially large sections of\n"
		"<278>         triggers you would want to use /* text */\n"
		"\n"
		"<178>Example<278>: #nop This is the start of my script file.\n"
		,
		"read"
	},
	{
		"PARSE",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #parse <178>{<278>string<178>} {<278>variable<178>} {<278>commands<178>}\n"
		"\n"
		"<278>         Like the loop statement, parse will loop from start to finish through\n"
		"<278>         the given string.  The value of the current character is stored in the\n"
		"<278>         provided variable.\n"
		"\n"
		"<178>Example<278>: #parse {hello world} {char} {#show $char}\n"
		,
		"break continue foreach list loop repeat return while"
	},
	{
		"PATH",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #path <178>{<278>option<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         create   Will clear the path and start path mapping.\n"
		"<278>         delete   Will delete the last move of the path.\n"
		"<278>         describe Describe the path and current position.\n"
		"<278>         destroy  Will clear the path and stop path mapping.\n"
		"<278>         get      Will get either the length or position.\n"
		"<278>         goto     Go the the start, end, or given position index.\n"
		"<278>         insert   Add the given argument to the path.\n"
		"<278>         load     Load the given variable as the new path.\n"
		"<278>         map      Display the map and the current position.\n"
		"<278>         move     Move the position forward or backward. If a number is given\n"
		"<278>                  the position is changed by the given number of steps.\n"
		"<278>         run      Execute the current path, with an optional floating point\n"
		"<278>                  delay in seconds as the second argument.\n"
		"<278>         save     Save the path to a variable. You must specify whether you\n"
		"<278>                  want to save the path 'forward' or 'backward'.\n"
		"<278>         start    Start path mapping.\n"
		"<278>         stop     Stop path mapping, can also abort #path run.\n"
		"<278>         swap     Switch the forward and backward path.\n"
		"<278>         unzip    Load the given speedwalk as the new path.\n"
		"<278>         walk     Take one step forward or backward.\n"
		"<278>         zip      Turn the path into a speedwalk.\n"
		"\n"
		"<178>Example<278>: #path ins {unlock n;open n} {unlock s;open s}\n"
		,
		"map pathdir speedwalk"
	},
	{
		"PATHDIR",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #pathdir <178>{<278>dir<178>} {<278>reversed dir<178>} {<278>coord<178>}\n"
		"\n"
		"<278>         By default tintin sets the most commonly used movement commands\n"
		"<278>         meaning you generally don't really have to bother with pathdirs.\n"
		"<278>         Pathdirs are used by the #path and #map commands.\n"
		"\n"
		"<278>         The first argument is a direction, the second argument is the reversed\n"
		"<278>         direction.  The reverse direction of north is south, etc.\n"
		"\n"
		"<278>         The third argument is a spatial coordinate which is a power of two.\n"
		"<278>         'n' is 1, 'e' is 2, 's' is 4, 'w' is '8', 'u' is 16, 'd' is 32. The\n"
		"<278>         exception is for compound directions, whose value should be the sum\n"
		"<278>         of the values of each cardinal direction it is composed of. For\n"
		"<278>         example, 'nw' is the sum of 'n' and 'w' which is 1 + 8, so 'nw'\n"
		"<278>         needs to be given the value of 9. This value is required for the\n"
		"<278>         #map functionality to work properly.\n"
		"\n"
		"<178>Example<278>: #pathdir {ue} {dw} {18}\n"
		"<278>         #pathdir {dw} {ue} {40}\n"
		"\n"
		"<178>Comment<278>: You can remove a pathdir with the #unpathdir command.\n",
		
		"map path"
	},
	{
		"PCRE",
		TOKEN_TYPE_STRING,
		"\n"
		"<278>         A regular expression, regex or regexp is a sequence of characters that\n"
		"<278>         defines a search pattern. Since the 1980s, different syntaxes for\n"
		"<278>         writing regular expressions exist, the two most widely used ones being\n"
		"<278>         the POSIX syntax and the similar but more advanced Perl standard.\n"
		"<278>         TinTin++ supports the Perl standard known as PCRE (Perl Compatible\n"
		"<278>         Regular Expressions).\n"
		"\n"
		"<278>         Regular expressions are an integral part of TinTin++, but keep in mind\n"
		"<278>         that tintin doesn't allow you to use regular expressions directly,\n"
		"<278>         instead it uses a simpler intermediate syntax that still allows more\n"
		"<278>         complex expressions when needed.\n"
		"\n"
		"<278>         Commands that utilize regular expressions are: action, alias, elseif,\n"
		"<278>         gag, grep, highlight, if, kill, local, math, prompt, regexp, replace,\n"
		"<278>         substitute, switch, variable and while. Several other commands use\n"
		"<278>         regular expressions in minor ways. Fortunately the basics are very\n"
		"<278>         easy to learn.\n"
		"\n"
		"<128>         TinTin++ Regular Expression\n"
		"\n"
		"<278>         The following support is available for regular expressions.\n"
		"\n"
		"<178>       ^ <278>match start of line.\n"
		"<178>       $ <278>match of end of line.\n"
		"<178>       \\ <278>escape one character.\n"
		"\n"
		"<178>  %1-%99 <278>match of any text, stored in the corresponding index.\n"
		"<178>      %0 <278>should be avoided in the regex, contains all matched text.\n"
		"<178>     { } <278>embed a perl compatible regular expression, matches are stored.\n"
		"<178>   %!{ } <278>embed a perl compatible regular expression, matches are not stored.\n"
		"\n"
		"<278>         [ ] . + | ( ) ? * are treated as normal text unless used within braces.\n"
		"<278>         Keep in mind that { } is replaced with ( ) automatically unless %!{ }\n"
		"<278>         is used.\n"
		"\n"
		"<178>TinTin++ <178>Description                                      POSIX\n"
		"<178>      %a <278>Match zero or more characters including newlines ([^\\n]*?)\n"       
		"<178>      %A <278>Match zero or more newlines                      ([\\n]*?)\n"
		"<178>      %c <278>Match zero or more ansi color codes              ((?:\\e\\[[0-9;]*m)*?)\n"
		"<178>      %d <278>Match zero or more digits                        ([0-9]*?)\n"
		"<178>      %D <278>Match zero or more non-digits                    ([^0-9]*?)\n"
		"<178>      %i <278>Matches become case insensitive                  (?i)\n"
		"<178>      %I <278>Matches become case sensitive (default)          (?-i)\n"
		"<178>      %s <278>Match zero or more spaces                        ([\\r\\n\\t ]*?)\n"
		"<178>      %w <278>Match zero or more word characters               ([A-Za-z0-9_]*?)\n"
		"<178>      %W <278>Match zero or more non-word characters           ([^A-Za-z0-9_]*?)\n"
		"<178>      %? <278>Match zero or one character                      (.\?\?)\n"
		"<178>      %. <278>Match one character                              (.)\n"
		"<178>      %+ <278>Match one or more characters                     (.+?)\n"
		"<178>      %* <278>Match zero or more characters excluding newlines (.*?)\n"
		"\n"
		"<278>         <128>Ranges\n"
		"\n"
		"<278>         If you want to match 1 digit use %+1d, if you want to match between 3\n"
		"<278>         and 5 spaces use %+3..5s, if you want to match 1 or more word\n"
		"<278>         characters use %+1..w, etc.\n"
		"\n"
		"<278>         <128>Variables\n"
		"\n"
		"<278>         If you use %1 in an action to perform a match the matched string is\n"
		"<278>         stored in the %1 variable which can be used in the action body.\n"
		"\n"
		"<178>Example<278>: #act {%1 says 'Tickle me'} {tickle %1}\n"
		"\n"
		"<278>         If you use %2 the match is stored in %2, etc. If you use an unnumbered\n"
		"<278>         match like %* or %S the match is stored at the last used index\n"
		"<278>         incremented by one.\n"
		"\n"
		"<178>Example<278>: #act {%3 says '%*'} {#if {\"%4\" == \"Tickle me\"} {tickle %3}}\n"
		"\n"
		"<278>         The maximum variable index is 99. If you begin an action with %* the\n"
		"<278>         match is stored in %1. You should never use %0 in the trigger part of\n"
		"<278>         an action, when used in the body of an action %0 contains all the parts\n"
		"<278>         of the string that were matched.\n"
		"\n"
		"<278>         To prevent a match from being stored use %!*, %!w, etc.\n"
		"\n"
		"<278>         <128>Perl Compatible Regular Expressions\n"
		"\n"
		"<278>         You can embed a PCRE (Perl Compatible Regular Expression) using curley\n"
		"<278>         braces { }, these braces are replaced with parentheses ( ) unless you\n"
		"<278>         use %!{ }.\n"
		"\n"
		"<278>         <128>Or\n"
		"\n"
		"<278>         You can separate alternatives within a PCRE using the | character.\n"
		"\n"
		"<178>Example<278>: #act {%* raises {his|her|its} eyebrows.} {say 42..}\n"
		"\n"
		"<278>         <128>Brackets\n"
		"\n"
		"<278>         You can group alternatives and ranges within a PCRE using brackets.\n"
		"\n"
		"<178>Example<278>: #act {%* says 'Who is number {[1-9]}?} {say $number[%2] is number %2}\n"
		"\n"
		"<278>         The example only triggers if someone provides a number between 1 and\n"
		"<278>         9. Any other character will cause the action to not trigger.\n"
		"\n"
		"<178>Example<278>: #act {%* says 'Set password to {[^0-9]*}$} {say The password must\n"
		"<278>           contain at least one number, not for security reasons, but just to\n"
		"<278>           annoy you.} {4}\n"
		"\n"
		"<278>         When the ^ character is used within brackets it creates an inverse\n"
		"<278>         search, [^0-9] matches every character except for a number between 0\n"
		"<278>         and 9.\n"
		"\n"
		"<278>         <128>Quantification\n"
		"\n"
		"<278>         A quantifier placed after a match specifies how often the match is\n"
		"<278>         allowed to occur.\n"
		"\n"
		"<178>       ? <278>repeat zero or one time.\n"
		"<178>       * <278>repeat zero or multiple times.\n"
		"<178>       + <278>repeat once or multiple times.\n"
		"<178>     {n} <278>repeat exactly n times, n must be a number.\n"
		"<178>    {n,} <278>repeat at least n times, n must be a number.\n"
		"<178>   {n,o} <278>repeat between n and o times, n and o must be a number.\n"
		"\n"
		"<178>Example<278>: #act {%* says 'Who is number {[1-9][0-9]{0,2}}?} {Say $number[%2] is\n"
		"<278>           number %2}\n"
		"\n"
		"<278>         The example only triggers if someone provides a number between 1 and\n"
		"<278>         999.\n"
		"\n"
		"<278>         <128>Parantheses\n"
		"\n"
		"<278>         TinTin Regular Expressions automatically add parenthesis, for example\n"
		"<278>         %* translates to (.*?) in PCRE unless the %* is found at the start or\n"
		"<278>         end of the line, in which cases it translates to (.*). Paranthesis in\n"
		"<278>         PCRE causes a change in execution priority similar to mathematical\n"
		"<278>         expressions, but parentheses also causes the match to be stored to a\n"
		"<278>         variable.\n"
		"\n"
		"<278>         When nesting multiple sets of parentheses each nest is assigned its\n"
		"<278>         numerical variable in order of appearance.\n"
		"\n"
		"<178>Example<278>: #act {%* chats '{Mu(ha)+}'} {chat %2ha!}\n"
		"\n"
		"<278>         If someone chats Muha you will chat Muhaha! If someone chats Muhaha\n"
		"<278>         you will chat Muhahaha!\n"
		"\n"
		"<278>         <128>Lazy vs Greedy\n"
		"\n"
		"<278>         By default regex matches are greedy, meaning {.*} will capture as much\n"
		"<278>         text as possible.\n"
		"\n"
		"<178>Example<278>: #regex {bli bla blo} {^{.*} {.*}$} {#show Arg1=(&1) Arg2=(&2)}\n"
		"\n"
		"<278>         This will display: Arg1=(bli bla) Arg2=(blo)\n"
		"\n"
		"<278>         By appending a ? behind a regex it becomes lazy, meaning {.*?} will\n"
		"<278>         capture as little text as possible.\n"
		"\n"
		"<178>Example<278>: #regex {bli bla blo} {^{.*?} {.*?}$} {#show Arg1=(&1) Arg2=(&2)}\n"
		"\n"
		"<278>         This will display: Arg1=(bli) Arg2=(bla blo).\n"
		"\n"
		"<278>         <128>Escape Codes\n"
		"\n"
		"<278>         PCRE support the following escape codes.\n"
		"\n"
		"<178>    PCRE Description                                    POSIX\n"
		"<178>      \\A <278>Match start of string                          ^\n"
		"<178>      \\b <278>Match word boundaries                          (^|\\r|\\n|\\t| |$)\n"
		"<178>      \\B <278>Match non-word boundaries                      [^\\r\\n\\t ]\n"
		"<178>      \\c <278>Insert control character                       \\c\n"
		"<178>      \\d <278>Match digits                                   [0-9]\n"
		"<178>      \\D <278>Match non-digits                               [^0-9]\n"
		"<178>      \\e <278>Insert escape character                        \\e\n"
		"<178>      \\f <278>Insert form feed character                     \\f\n"
		"<178>      \\n <278>Insert line feed character                     \\n\n"
		"<178>      \\r <278>Insert carriage return character               \\r\n"
		"<178>      \\s <278>Match spaces                                   [\\r\\n\\t ]\n"
		"<178>      \\S <278>Match non-spaces                               [^\\r\\n\\t ]\n"
		"<178>      \\t <278>Insert tab character                           \\t\n"
		"<178>      \\w <278>Match letters, numbers, and underscores        [A-Za-z0-9_]\n"
		"<178>      \\W <278>Match non-letters, numbers, and underscores    [^A-Za-z0-9_]\n"
		"<178>      \\x <278>Insert hex character                           \\x\n"
		"<178>      \\Z <278>Match end of string                            $\n"
		"\n"
		"<278>         \\s matches one space, \\s+ matches one or multiple spaces, the use\n"
		"<278>         of {\\s+} is required for this sequence to work in tintin, \\s by"
		"<278>         itself will work outside of a set of braces.\n"
		"\n"
		"<278>         <128>Color triggers\n"
		"\n"
		"<278>         To make matching easier text triggers (Actions, Gags, Highlights,\n"
		"<278>         Prompts, and Substitutes) have their color codes stripped. If you\n"
		"<278>         want to create a color trigger you must start the triggers with a ~\n"
		"<278>         (tilde). To make escape codes visible use #config {convert meta} on.\n"
		"\n"
		"<178>Example<278>: #action {~\\e[1;37m%1} {#var roomname %1}\n"
		"\n"
		"<278>         If the room name is the only line on the server in bright white\n"
		"<278>         white color trigger will save the roomname.\n"
		"\n"
		"\n"
		"<278>         This covers the basics. PCRE has more options, most of which are\n"
		"<278>         somewhat obscure, so you'll have to read a PCRE manual for additional\n"
		"<278>         information.\n"
		,
		"map path"
	},

	{
		"PORT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #port <178>{<278>option<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         <178>#port {init} {name} {port} {file}\n"
		"<278>           Initilize a port session.\n"
		"\n"
		"<278>         <178>#port {call} {address} {port}\n"
		"<278>           Connect to a remote socket.\n"
		"\n"
		"<278>         <178>#port {color} {color names}\n"
		"<278>           Set the default color of port messages.\n"
		"\n"
		"<278>         <178>#port {dnd}\n"
		"<278>           Do Not Disturb. Decline new connections\n"
		"\n"
		"<278>         <178>#port {group} {name} {group}\n"
		"<278>           Assign a socket group.\n"
		"\n"
		"<278>         <178>#port {ignore} {name}\n"
		"<278>           Ignore a socket\n"
		"\n"
		"<278>         <178>#port {info}\n"
		"<278>           Display information about the port session.\n"
		"\n"
		"<278>         <178>#port {name} {name}\n"
		"<278>           Change socket name.\n"
		"\n"
		"<278>         <178>#port {prefix} {text}\n"
		"<278>           Set prefix before each message.\n"
		"\n"
		"<278>         <178>#port {send} {name|all} {text}\n"
		"<278>           Send data to socket\n"
		"\n"
		"<278>         <178>#port {uninitialize}\n"
		"<278>           Uninitialize the port session.\n"
		"\n"
		"<278>         <178>#port {who}\n"
		"<278>           Show all connections\n"
		"\n"
		"<278>         <178>#port {zap} {name}\n"
		"<278>           Close a connection\n"
		"\n"
		"<278>         The port command is very similar to chat except that it creates a\n"
		"<278>         new session dedicated to receiving socket connections at the given\n"
		"<278>         port number without built-in support for a communication protocol.\n"
		"\n"
		"<278>         You can init with 0 as the port number to create a dummy session.\n"
		,
		"all chat run session sessionname snoop ssl zap"
	},

	{
		"PROMPT",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #prompt <178>{<278>text<178>} {<278>new text<178>} {<278>row #<178>} <178>{<278>col #<178>}\n"
		"\n"
		"<278>         Prompt is a feature for split window mode, which will capture a line\n"
		"<278>         received from the server and display it on the status bar of your\n"
		"<278>         split screen terminal. You would define <text> and <new text> the\n"
		"<278>         same way as you would with #substitute.\n"
		"\n"
		"<278>         The row number is optional and useful if you use a non standard split\n"
		"<278>         mode. A positive row number draws #row lines from the top while a\n"
		"<278>         negative number draws #row lines from the bottom. Without an argument\n"
		"<278>         #prompt will write to the default split line, which is one row above\n"
		"<278>         the input line, typically at row -2.\n"
		"\n"
		"<278>         If the row number is set to 0, #prompt will behave like #substitute.\n"
		"<278>         This is useful to let tintin know that a prompt was received so you\n"
		"<278>         can use #config packet_patch with minimal interference.\n"
		"\n"
		"<278>         The col number is optional and can be used to set the column index.\n"
		"<278>         A positive col number draws the given number of columns from the left,\n"
		"<278>         while a negative col number draws from the right. If you leave the\n"
		"<278>         col number empty tintin will clear the row before printing at the\n"
		"<278>         start of the row.\n"
		"\n"
		"<278>         The #show command takes a row and col argument as well so it's also\n"
		"<278>         possible to place text on your split lines using #show.\n"
		"\n"
		"<178>Comment<278>: See <178>#help split<278> for more information on split mode.\n"
		"\n"
		"<178>Comment<278>: See <178>#help substitute<278> for more information on text\n"
		"<278>         substitutions.\n"
		"\n"
		"<178>Comment<278>: You can remove a prompt with the #unprompt command.\n"
		,
		"action gag highlight substitute"
	},
	{
		"READ",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #read <178>{<278>filename<178>}\n"
		"\n"
		"<278>         Reads a commands file into memory.  The coms file is merged in with\n"
		"<278>         the currently loaded commands.  Duplicate commands are overwritten.\n"
		"\n"
		"<278>         If you uses braces, { and } you can use several lines for 1 commands.\n"
		"<278>         This however means you must always match every { with a } for the read\n"
		"<278>         command to work.\n"
		"\n"
		"<278>         You can comment out triggers using /* text */\n"
		,
		"log scan textin write"
	},
	{
		"REGEXP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #regexp <178>{<278>string<178>} {<278>expression<178>} {<278>true<178>} {<278>false<178>}\n"
		"\n"
		"<278>         Compares the string to the given regular expression.\n"
		"\n"
		"<278>         The expression can contain escapes, and if you want to match a literal\n"
		"<278>         \\ character you'll have to use \\\\ to match a single backslash.\n"
		"\n"
		"<278>         Variables are stored in &1 to &99 with &0 holding the matched\n"
		"<278>         substring.\n"
		"\n"
		"<278>         The #regex command is not a proper statement like #if, when using\n"
		"<278>         #return or #break in the {true} argument it won't terminate any loop\n"
		"<278>         the #regex command is nested within.\n"
		"\n"
		"<178>       ^ <278>force match of start of line.\n"
		"<178>       $ <278>force match of end of line.\n"
		"<178>       \\ <278>escape one character.\n"
		"<178>  %1-%99 <278>lazy match of any text, available at %1-%99.\n"
		"<178>      %0 <278>should be avoided in triggers, and if left alone lists all matches.\n"
		"<178>     { } <278>embed a raw regular expression, matches are stored to %1-%99.\n"
		"<178>   %!{ } <278>embed a raw regular expression, matches are not stored.\n"
		"<178>         <278>[ ] . + | ( ) ? * are treated as normal text unlessed used within\n"
		"<178>         <278>braces. Keep in mind that { } is replaced with ( ) automatically\n"
		"<178>         <278>unless %!{ } is used.\n"
		"\n"
		"<278>         Of the following the (lazy) match is available at %1-%99 + 1\n"
		"\n"
		"<178>      %a <278>match zero or more characters including newlines.\n"
		"<178>      %A <278>match zero or more newlines.\n"
		"<178>      %c <278>match zero or more ansi color codes.\n"
		"<178>      %d <278>match zero or more digits.\n"
		"<178>      %D <278>match zero or more non digits.\n"
		"<178>      %s <278>match zero or more spaces.\n"
		"<178>      %S <278>match zero or more non spaces.\n"
		"<178>      %w <278>match zero or more word characters.\n"
		"<178>      %W <278>match zero or more non word characters.\n"
		"\n"
		"      Experimental (subject to change) matches are:\n"
		"\n"
		"<178>      %p <278>match zero or more printable characters.\n"
		"<178>      %P <278>match zero or more non printable characters.\n"
		"<178>      %u <278>match zero or more unicode characters.\n"
		"<178>      %U <278>match zero or more non unicode characters.\n"
		"\n"
		"      If you want to match 1 digit use %+1d, if you want to match between 3\n"
		"      and 5 spaces use %+3..5s, if you want to match 0 or more word\n"
		"      characters use %+0..w, etc.\n"
		"\n"
		"<178>      %+ <278>match one or more characters.\n"
		"<178>      %? <278>match zero or one character.\n"
		"<178>      %. <278>match one character.\n"
		"<178>      %* <278>match zero or more characters.\n"
		"\n"
		"<178>      %i <278>matching becomes case insensitive.\n"
		"<178>      %I <278>matching becomes case sensitive (default).\n"
		"\n"
		"<278>         The match is automatically stored to a value between %1 and %99\n"
		"<278>         starting at %1 and incrementing by 1 for every regex. If you use\n"
		"<278>         %15 as a regular expression, the next unnumbered regular expression\n"
		"<278>         would be %16. To prevent a match from being stored use %!*, %!w, etc.\n"
		"\n"
		"<178>Example<278>: #regexp {bli bla blo} {bli {.*} blo} {#show &1}\n"
		"\n"
		"<178>Comment<278>: Like an alias or function #regex has its own scope.\n"
		,
		"pcre replace"
	},

	{
		"REPEAT",
		TOKEN_TYPE_STRING,
		"<178>Command<278>: #<178>[<078>number<178>] {<278>commands<178>}\n"
		"\n"
		"        Sometimes you want to repeat the same command multiple times. This is\n"
		"        the easiest way to accomplish that.\n"
		"\n"
		"<178>Example<278>: #10 {buy bread}\n",
		
		"mathematics statements"
	},
	{
		"REPLACE",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #replace <178>{<278>variable<178>} {<278>oldtext<178>} {<278>newtext<178>}\n"
		"\n"
		"<278>         Searches the given variable, replacing each occurrence of 'oldtext'\n"
		"<278>         with 'newtext'. The 'oldtext' argument is a regular expression.\n"
		"\n"
		"<278>         Variables are stored in &1 to &99 with &0 holding the entire matched\n"
		"<278>         substring.\n"
		"\n"
		"<178>Example<278>: #function rnd #math result 1d9;#replace test {%.} {@rnd{}}\n"
		,
		"cat format function local math script variable"
	},
	{
		"RETURN",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #return <178>{<278>text<178>}\n"
		"\n"
		"<278>         This command can be used to break out of a command string being\n"
		"<278>         executed.\n"
		"\n"
		"<278>         If used inside a #function you can use #return with an argument to both\n"
		"<278>         break out of the function and set the result variable.\n"
		,
		"break continue foreach list loop parse repeat while"
	},
	{
		"RUN",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #run <178>{<278>name<178>} {<278>shell command<178>} {<278>file<178>}\n"
		"\n"
		"<278>         The run command works much like the system command except that it\n"
		"<278>         runs the command in a pseudo terminal. The run command also creates\n"
		"<278>         a session that treats the given shell command as a server. This\n"
		"<278>         allows you to run ssh, as well as any other shell application, with\n"
		"<278>         full tintin scripting capabilities. If a file name is given the file\n"
		"<278>         is loaded prior to execution.\n"
		"\n"
		"<178>Example<278>: #run {somewhere} {ssh someone@somewhere.com}\n"
		"<178>Example<278>: #run {something} {tail -f chats.log}\n"
		,
		"all port session sessionname snoop ssl zap"
	},
	{
		"SCAN",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #scan <178>{<278>abort<178>|<278>csv<178><178>|<278>tsv<178><178>|<278>txt<178>} {<278>filename<178>}\n"
		"\n"
		"<278>         The scan command is a file reading utility.\n"
		"\n"
		"<278>         <178>#scan {abort}\n"
		"<278>           This command must be called from with a SCAN event and will\n"
		"<278>           abort the scan if one is in progress.\n"
		"\n"
		"<278>         <178>#scan {csv} <filename>\n"
		"<278>           The scan csv command reads in a comma separated value file\n"
		"<278>           without printing the content to the screen. Instead it triggers one\n"
		"<278>           of two events.\n"
		"\n"
		"<278>           The SCAN CSV HEADER event is triggered on the first line of the csv\n"
		"<278>           file. The SCAN CSV LINE event is triggered on the second and each\n"
		"<278>           subsequent line of the csv file. The %0 argument contains the entire\n"
		"<278>           line, with  %1 containing the first value, %2 the second value, etc,\n"
		"<278>           all the way up to %99.\n"
		"\n"
		"<278>           Values containing spaces must be surrounded with quotes, keep in mind\n"
		"<278>           newlines within quotes are not supported. Use two quotes to print one\n"
		"<278>           literal quote character.\n"
		"\n"
		"<178>         #scan {dir} <filename> <variable>\n"
		"\n"
		"<278>          The scan dir command will read the given filename or directory and\n"
		"<278>          store any gathered information into the provided variable.\n"
		"\n"
		"<278>         <178>#scan {tsv} <filename>\n"
		"\n"
		"<278>           The scan tsv <filename> command reads in a tab separated value file\n"
		"<278>           without printing the content to the screen. Instead it triggers the\n"
		"<278>           SCAN TSV HEADER event for the first line and SCAN TSV LINE for all\n"
		"<278>           subsequent lines.\n"
		"\n"
		"<278>         <178>#scan {file} <filename> {commands}\n"
		"\n"
		"<278>           The scan file command reads the given files and executes the\n"
		"<278>            commands argument. &0 contains the raw content of the file and\n"
		"<278>            &1 contains the plain content. &2 contains the raw byte size of the\n"
		"<278>            file and &3 the plain byte size. &5 contains the line count.\n"
		"\n"
		"<278>         <178>#scan {txt} <filename>\n"
		"\n"
		"<278>           The scan txt <filename> command reads in a file and sends its content\n"
		"<278>           to the screen as if it was send by a server. After using scan you can\n"
		"<278>           use page-up and down to view the file.\n"
		"\n"
		"<278>           This command is useful to convert ansi color files to html or viewing\n"
		"<278>           raw log files.\n"
		"\n"
		"<278>           Actions, highlights, and substitutions will trigger as normal, and it\n"
		"<278>           is possible to create an action to execute #scan abort to prematurely\n"
		"<278>           stop the scan.\n"
		,
		"read textin"
	},

	{
		"SCREEN",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #screen <178>{<278>option<178>}<178> {<278>argument<178>}\n"
		"\n"
		"<278>         The screen command offers a variety of screen manipulation\n"
		"<278>         commands and utilities.\n"
		"\n"
		"<278>         <178>#screen blur\n"
		"<278>           Move the terminal to the back of the stack.\n"
		"\n"
		"<278>         <178>#screen clear [all|scroll region|square] <args>\n"
		"<278>           Provide 4 arguments defining the top left and bottom right corner\n"
		"<278>         <888>  when erasing a square.\n"
		"\n"
		"<278>         <178>#screen focus\n"
		"<278>           Move the terminal to the front of the stack.\n"
		"\n"
		"<278>         <178>#screen fullscreen [on|off]\n"
		"<278>           Toggles fullscreen mode when used without an argument.\n"
		"\n"
		"<278>         <178>#screen get <option> <var>\n"
		"<278>           Get various screen options and save them to <var>. Use #screen\n"
		"<278>           get without an argument to see all available options.\n"
		"\n"
		"<278>         <178>#screen info\n"
		"<278>           Debugging information.\n"
		"\n"
		"<278>         <178>#screen inputregion <square> [name]\n"
		"<278>           Set the input region. The name argument is optional and can be\n"
		"<278>           used to create named RECEIVED INPUT [NAME] events.\n"
		"\n"
		"<278>         <178>#screen load <both|label|title>\n"
		"<278>           Reload the saved title, label, or both.\n"
		"\n"
		"<278>         <178>#screen minimize <on|off>\n"
		"<278>           Minimize with on, restore with off.\n"
		"\n"
		"<278>         <178>#screen maximize [on|off]\n"
		"<278>           Maximize with on, restore with off.\n"
		"\n"
		"<278>         <178>#screen move <height> <width>\n"
		"<278>           Move the upper left corner of the terminal to pixel coordinate.\n"
		"\n"
		"<278>         <178>#screen raise <event>\n"
		"<278>           This will raise several screen events with %1 and %2 arguments.\n"
		"\n"
		"<278>         <178>#screen refresh\n"
		"<278>           Terminal dependant, may do nothing.\n"
		"\n"
		"<278>         <178>#screen rescale <height> <width>\n"
		"<278>           Resize the screen to the given height and width in pixels.\n"
		"\n"
		"<278>         <178>#screen resize <rows> <cols>\n"
		"<278>           Resize the screen to the given height and width in characters.\n"
		"\n"
		"<278>         <178>#screen save <both|label|title>\n"
		"<278>           Save the title, label, or both.\n"
		"\n"
		"<278>         <178>#screen scroll <square>\n"
		"<278>           Set the scrolling region, changes the split setting.\n"
		"\n"
		"<278>         <178>#screen set <both|label|title>\n"
		"<278>           Set the title, label, or both. Only title works on Windows.\n"
		"\n"
		"<278>         <178>#screen swap\n"
		"<278>           Swap the input and scroll region.\n"
		,
		"bell"
	},

	{
		"SCREEN READER",
		TOKEN_TYPE_STRING,
		"<178>Command<278>: #config <178>{<278>SCREEN READER<178>} {<278>ON|OFF<178>}\n"
		"\n"
		"<278>         Screen reader mode is enabled by using #config screen on. One purpose\n"
		"<278>         of the screen reader mode is to report to servers that a screen reader\n"
		"<278>         is being used by utilizing the MTTS standard. The MTTS specification\n"
		"<278>         is available at:\n"
		"\n"
		"<278>         http://tintin.sourceforge.net/protocols/mtts\n"
		"\n"
		"<278>         With the screen reader mode enabled TinTin++ will try to remove or\n"
		"<278>         alter visual elements where possible.\n"
		,
		"config"
	},

	{
		"SCRIPT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #script <178>{<278>variable<178>}<178> {<278>shell command<178>}\n"
		"\n"
		"<278>         The script command works much like the system command except that it\n"
		"<278>         treats the generated echos as commands if no variable is provided.\n"
		"\n"
		"<278>         This is useful for running php, perl, ruby, and python scripts. You\n"
		"<278>         can run these scripts either from file or from within tintin if the\n"
		"<278>         scripting language allows this.\n"
		"\n"
		"<278>         If you provide a variable the output of the script is stored as a list.\n"
		"\n"
		"<178>Example<278>: #script {ruby -e 'print \"#show hello world\"'}\n"
		"<178>Example<278>: #script {python -c 'print \"#show hello world\"'}\n"
		"<178>Example<278>: #script {php -r 'echo \"#show hello world\"'}\n"
		"<178>Example<278>: #script {path} {pwd};#show The path is $path[1].\n"
		,
		"format function local math replace variable"
	},

	{
		"SEND",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #send <178>{<278>text<178>}\n"
		"\n"
		"<278>         Sends the text directly to the server, useful if you want to start\n"
		"<278>         with an escape code.\n"
		,
		"textin"
	},

	{
		"SESSION",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #session <178>{<278>name<178>} {<278>host<178>} {<278>port<178>} {<278>file<178>}\n"
		"\n"
		"<278>         Starts a telnet session with the given name, host, port, and optional\n"
		"<278>         file name. The name can be anything you want, except the name of an\n"
		"<278>         already existing session, a number, or the keywords '+' and '-'.\n"
		"\n"
		"<278>         If a file name is given the file is only read if the session\n"
		"<278>         succesfully connects.\n"
		"\n"
		"<278>         Without an argument #session shows the currently defined sessions.\n"
		"\n"
		"<278>         If you have more than one session, you can use the following commands:\n"
		"\n"
		"<278>         #session {-}        Switch to the previous session.\n"
		"<278>         #session {+}        Switch to the next session.\n"
		"<278>         #session {<number>} Switch to the given session. Session 0 is the\n"
		"<278>                             startup session, +1 the first, +2 the second, and\n"
		"<278>                             -1 is the last session. Sessions are (currently)\n"
		"<278>                             sorted in order of creation.\n"
		"<278>         #gts                Switch to the startup session. The name gts stands\n"
		"<278>                             for global tintin session.\n"
		"<278>         #ats                Switch to the active session. The name ats stands\n"
		"<278>                             for active tintin session.\n"
		"<278>                             not necessarily the calling session.\n"
		"<278>         #{name}             Activates to the session with the given name.\n"
		"<278>         #{name} {command}:  Executes a command with the given session without\n"
		"<278>                             changing the active session.\n"
		"<278>         @<name>{text}:      Parse text in the given session, substituting the\n"
		"<278>                             variables and functions, and print the result in\n"
		"<278>                             the current active session.\n"
		"\n"
		"<278>         The startup session is named 'gts' and can be used for relog scripts.\n"
		"<278>         Do keep in mind that tickers do not work in the startup session.\n"
		"\n"
		"<178>Example<278>: #event {SESSION DISCONNECTED} {#gts #delay 10 #ses %0 tintin.net 4321}\n"
		,
		"all port run sessionname snoop ssl zap"
	},

	{
		"SESSIONNAME",
		TOKEN_TYPE_STRING,
		"<178>Syntax<278>:  #[sessionname] <178>{<278>commands<178>}\n"
		"\n"
		"<278>         You can create multiple sessions with the #session command. By default\n"
		"<278>         only one session is active, meaning commands you input are executed in\n"
		"<278>         the active session. While all sessions receive output, only output sent\n"
		"<278>         to the active session is displayed.\n"
		"\n"
		"<278>         When you create a session with the #session command you must specify a\n"
		"<278>         session name, the session name, prepended with a hashtag, can be used\n"
		"<278>         to activate the session when used without an argument. If an argument\n"
		"<278>         is given it will be executed by that session as a command, the session\n"
		"<278>         will not be activated.\n"
		"\n"
		"<178>Example<278>: #ses one tintin.net 23;#ses two tintin.net 23;#one;#two grin\n"
		"\n"
		"<278>         This will create two sessions, the session that was created last (two\n"
		"<278>         in this case) will be automatically activated upon creation. Using\n"
		"<278>         #one, session one is activated. Using #two grin, the grin social will\n"
		"<278>         be executed by session two, session one will remain the active session.\n"
		"\n"
		"<278>         If you send a variable to another session it will be substituted before\n"
		"<278>         being passed. If you want the variable value of the receiving session\n"
		"<278>         to be used you need to use '$${variable}' to properly escape it.\n"
		"\n"
		"<178>Syntax<278>:  @[sessionname]<178>{<278>substitution<178>}\n"
		"\n"
		"<278>         If you want to pull the value of a variable from another session you\n"
		"<278>         can do so in a similar way as you would use a #function call. Using\n"
		"<278>         #showme {@two{$test}} in session one would print the value of $test,\n"
		"<278>         as defined by session two.\n"
		,
		"suspend"
	},

	{
		"SHOWME",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #show <178>{<278>string<178>} {<278>row<178>} <178>{<278>col<178>}\n"
		"\n"
		"<278>         Display the string to the terminal, do not send to the server.  Useful\n"
		"<278>         for status, warnings, etc.  The {row} and col number are optional and\n"
		"<278>         work the same way as the row number of the #prompt trigger.\n"
		"\n"
		"<278>         Actions can be triggered by the show command. If you want to avoid\n"
		"<278>         this from happening use: #line ignore #show {<string>}.\n"
		"\n"
		"<178>Example<278>: #tick {TICK} {#delay 50 #show 10 SECONDS TO TICK!!!} {60}\n"
		"\n"
		"<178>Comment<278>: The #prompt helpfile contains more information on using the\n"
		"<278>         option {row} and {col} arguments.\n"
		,
		"buffer draw echo grep prompt"
	},
	{
		"SNOOP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #snoop <178>{<278>session name<178>} <178>{<278>on<178>|<278>off<178>|<278>scroll<178>}\n"
		"\n"
		"<278>         If there are multiple sessions active, this command allows you to\n"
		"<278>         monitor what is going on in the sessions that are not currently active.\n"
		"<278>         The line of text from other sessions will be prefixed by the session's\n"
		"<278>         name.\n"
		"\n"
		"<278>         You can toggle off snoop mode by executing #snoop a second time.\n"
		"\n"
		"<278>         By using the scroll argument you will snoop the session's scroll\n"
		"<278>         region which will overwrite the display of whichever session is active.\n"
		"<278>         You can change the size and location of a session's scroll region by\n"
		"<278>         using the #split and #screen scrollregion commands.\n"
		,
		"all port run session sessionname ssl zap"
	},
	{
		"SPEEDWALK",
		TOKEN_TYPE_STRING,
		"<128>         SPEEDWALK V1\n"
		"\n"
		"<278>         Speedwalking allows you to enter multiple directions without using\n"
		"<278>         semicolons. Directions should be prefixed with a number and will be\n"
		"<278>         executed the given number of times.\n"
		"\n"
		"<278>         You can enable speedwalking with #CONFIG {SPEEDWALK} {ON}.\n"
		"\n"
		"<178>Example<278>: Without speedwalk, you have to type:\n"
		"<278>         <178>s;s;w;w;w;w;w;s;s;s;w;w;w;n;n;w\n"
		"<278>         With speedwalk, you only have to type:\n"
		"<278>         <178>2s5w3s3w2nw\n"
		"\n"
		"<278>         <128>SPEEDWALK V2\n"
		"\n"
		"<278>         Modern MUDs have increasingly adopted the use of diagonal exits, like\n"
		"<278>         ne, nw, sw, and se. To make accomodations for this the #map and #path\n"
		"<278>         command no longer interpret nesw as a speedwalk and require this to\n"
		"<278>         be written as 1n1e1s1w, which then allows 2ne2e to execute ne;ne;e;e.\n"
		"\n"
		"<278>         Speedwalks entered on the input line continue to use the v1 system.\n"
		"\n"
		"<278>         The #path load command is backward compatible with v1 speedwalks and\n"
		"<278>         to load v2 speedwalks the #path unzip command needs to be used, unless\n"
		"<278>         the speedwalk was saved using #path save in which case a v2 compatible\n"
		"<278>         format is used that can also contain timing data.\n"
		"\n"
		"<178>Example<278>: #path unzip 3n1e2nw\n"
		"<178>Example<278>: #map move 3ne1d\n",
		"keypad mapping repeat"
	},
	{
		"SPLIT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #split <178>{<278>top bar<178>} {<278>bottom bar<178>} {<278>left bar<178>} {<278>right bar<178>} {<278>input bar<178>}\n"
		"\n"
		"<278>         This option requires for your terminal to support VT100 emulation.\n"
		"\n"
		"<278>         #split allows the creation of a top status bar, a left and right status\n"
		"<278>         bar, a scrolling region, a bottom status bar, and an input line.\n"
		"\n"
		"<278>         <268>╭<268>──────<268>─<268>──────────────────<268>───────╮\n"
		"<278>         <268>│<178>      <178> <178>     top bar      <268>       │\n"
		"<278>         <268>├<268>──────<268>┬<268>──────────────────<268>┬──────┤\n"
		"<278>         <268>│<178> left <268>│<178>    scrolling     <268>│<178> right<268>│\n"
		"<278>         <268>│<178> bar  <268>│<178>     region       <268>│<178>  bar <268>│\n"
		"<278>         <268>├<268>──────<268>┴<268>──────────────────<268>┴──────┤\n"
		"<278>         <268>│<178>      <178> <178>    bottom bar    <268>       │\n"
		"<278>         <268>├<268>──────<268>─<268>──────────────────<268>───────┤\n"
		"<278>         <268>│<178>      <178> <178>    input bar     <268>       │\n"
		"<278>         <268>╰<268>──────<268>─<268>──────────────────<268>───────╯\n"
		"\n"
		"<278>         By default the bottom status bar is filled with dashes --- and\n"
		"<278>         subsequently it is also known as the split line. The scrolling\n"
		"<278>         region is also known as the main screen and this is where all\n"
		"<278>         incoming text is displayed by default.\n"
		"\n"
		"<278>         If you use #split without an argument it will set the height of the\n"
		"<278>         top status bar to 0 lines and the bottom status bar to 1 line.\n"
		"\n"
		"<278>         If you use #split with one argument it will set the height of the top\n"
		"<278>         status bar to the given number of lines and the bottom status bar will\n"
		"<278>         be set to 1 line.\n"
		"\n"
		"<278>         If you use two arguments the first argument is the height of the top\n"
		"<278>         status bar and the second argument the height of the bottom status bar.\n"
		"\n"
		"<278>         The third and fourth argument are optional and default to 0.\n"
		"\n"
		"<278>         The fifth argument is optional and sets the size of the input bar, it\n"
		"<278>         defaults to 1.\n"
		"\n"
		"<278>         It is possible to use negative arguments in which case the bar width\n"
		"<278>         defines the minimum width of the scrolling region.\n"
		"\n"
		"<178>Example<278>: #split 0 0\n"
		"<278>         This will create a split screen with just a scrolling region and an\n"
		"<278>         input line. Great for the minimalist.\n"
		"\n"
		"<178>Example<278>: #split 1 1 0 -80\n"
		"<278>         This will create a split screen with a single line top and bottom\n"
		"<278>         bar. The left bar has a width of 0 while the right bar will be of\n"
		"<278>         variable width. If for example the screen is 100 columns wide, 80\n"
		"<278>         columns will be used for the scrolling region, leaving a right bar\n"
		"<278>         with a width of 20 columns.\n"
		"\n"
		"<278>         To avoid displaying problems it's suggesed to use #prompt to capture\n"
		"<278>         the prompt sent by the MUD.\n"
		"\n"
		"<178>Comment<278>: You can display text on the split line(s) with the #prompt and\n"
		"<278>         #show {line} {row} commands.\n"
		"\n"
		"<178>Comment<278>: You can remove split mode with the #unsplit command.\n"
		,
		"echo prompt showme"
	},
	{
		"SSL",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #ssl <178>{<278>name<178>} {<278>host<178>} {<278>port<178>} {<278>file<178>}\n"
		"\n"
		"<278>         Starts a secure socket telnet session with the given name, host, port,\n"
		"<278>         and optional file name.\n"
		,
		"all port run sessionname snoop ssl zap"
	},
	{
		"STATEMENTS",
		TOKEN_TYPE_STRING,
		"\n"
		"<278>         TinTin++ knows the following statements.\n"
		"\n"
		"<278>         #break\n"
		"<278>         #case {value} {true}\n"
		"<278>         #continue\n"
		"<278>         #default {commands}\n"
		"<278>         #else {commands}\n"
		"<278>         #elseif {expression} {true}\n"
		"<278>         #foreach {list} {variable} {commands}\n"
		"<278>         #if {expression} {true}\n"
		"<278>         #loop {min} {max} {variable} {commands}\n"
		"<278>         #parse {string} {variable} {commands}\n"
		"<278>         #return {value}\n"
		"<278>         #switch {expression} {commands}\n"
		"<278>         #while {expression} {commands}\n"
		,
		"mathematics pcre repeat"
	},
	{
		"SUBSTITUTE",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #substitute <178>{<278>text<178>} {<278>new text<178>} {<278>priority<178>}\n"
		"\n"
		"<278>         Allows you to replace text from the server with the new text.\n"
		"\n"
		"<278>         The %1-%99 variables can be used to capture text and use it as part of\n"
		"<278>         the new output.\n"
		"\n"
		"<278>         Color codes can be used to color the new text, to restore the color to\n"
		"<278>         that of the original line the <<888>900> color code can be used.\n"
		"\n"
		"<278>         If only one argument is given, all active substitutions that match the\n"
		"<278>         argument are displayed.  Wildcards can be used, see '#help regex' for\n"
		"<278>         additional information on that subject.\n"
		"\n"
		"<278>         If no argument is given, all subs are displayed.\n"
		"\n"
		"<178>Example<278>: #sub {Zoe} {ZOE}\n"
		"<278>         Any instance of Zoe will be replaced with ZOE.\n"
		"\n"
		"<178>Example<278>: #sub {~\\e[0;34m} {\\e[1;34m}\n"
		"<278>         Replace generic dark blue color codes with bright blue ones.\n"
		"\n"
		"<178>Example<278>: #sub {%1massacres%2} {<<888>018>%1<<888>118>MASSACRES<<888>018>%2}\n"
		"<278>         Replaces all occurrences of 'massacres' with 'MASSACRES' in red.\n"
		"\n"
		"<178>Comment<278>: See '#help action', for more information about triggers.\n"
		"\n"
		"<178>Comment<278>: See '#help colors', for more information.\n"
		"\n"
		"<178>Comment<278>: You can remove a substitution with the #unsubstitute command.\n"
		,
		"action gag highlight prompt"
	},
	{
		"SUBSTITUTIONS",
		TOKEN_TYPE_STRING,
		"<278>          TinTin++ will perform various types of substitions as detailed below.\n"
		"\n"
		"<128>          Variables\n"
		"\n"
		"<178>$ & * @<278>   All variable and function names must begin with an alphabetic\n"
		"<278>          character, followed by any combination of alphanumeric characters and\n"
		"<278>          underscores.\n"
		"\n"
		"<178>$<278>         The dollar sign is used to retrieve the value of a variable.\n"
		"\n"
		"<178>&<278>         The ampersand sign is used to retrieve the index of a variable.\n"
		"\n"
		"<178>*<278>         The astrix sign is used to retrieve the name of a variable.\n"
		"\n"
		"<178>@<278>         The at sign is used for functions.\n"
		"\n"
		"<178>[ ]<278>       Brackets are used for nested variables which function as an\n"
		"<278>          associative array. Associative arrays are also known as tables and\n"
		"<278>          maps. Regex can be used within brackets to match multiple variables.\n"
		"\n"
		"<178>+ -<278>       The plus and minus signs are used to access variables by their index,\n"
		"<278>          with the first variable having index +1, and the last variable\n"
		"<278>          having index -1. Variables are ordered alphanumerically.\n"
		"\n"
		"<278>          All variables and functions can be escaped by doubling the sign,\n"
		"<278>          like $$variable_name or @@function_name. To escape a variable\n"
		"<278>          twice use $$$var_name. One escape is removed each time tintin\n"
		"<278>          needs to substitute a variable or function.\n"
		"\n"
		"<128>          Arguments\n"
		"\n"
		"<178>\%0 - \%99<278>  The percent sign followed by a number is used for arguments by the\n"
		"<278>          following triggers:\n"
		"\n"
		"<278>          alias, action, button, event, function, prompt, and substitute.\n"
		"\n"
		"<178>&0 - &99<278>  The ampersand sign followed by a number is used for arguments in the\n"
		"<278>          regex and replace commands.\n"
		"\n"
		"<278>          All trigger and command arguments can be escaped by doubling the\n"
		"<278>          sign like \%\%1 or &&1. One escape is removed each time tintin\n"
		"<278>          substitutes trigger or command arguments. To escape three times\n"
		"<278>          triple the sign like \%\%\%1, etc.\n"
		"\n"
		"<128>          Colors\n"
		"\n"
		"<178><<888>000><278>     Three alphanumeric characters encapsulated by the less- and greater-\n"
		"<278>          than signs are used for 4 and 8 bit color codes.\n"
		"\n"
		"<178><<888>0000><278>    Either a B (background) or F (foreground) followed by three\n"
		"<278>          hexadecimal characters encapsulated by < > signs are used for 12\n"
		"<278>          bit color codes. Requires truecolor capable terminal.\n"
		"\n"
		"<178><<888>0000000><278> Either a B (background) or F (foreground) followed by six\n"
		"<278>          hexadecimal characters encapsulated by < > signs are used for 24\n"
		"<278>          bit color codes. Requires truecolor capable terminal.\n"
		"\n"
		"<278>          More information is available at #help color.\n"
		"\n"
		"<128>          Escapes\n"
		"\n"
		"<178>\\ <278>        The back slash is used to escape a character. All available options\n"
		"<278>          are listed at #help escape. Escapes are typically escaped when text\n"
		"<278>          leaves the client, by being send to a server, the shell, being\n"
		"<278>          displayed on the screen, or being processed as part of a regex.\n"
		"<278>          Escapes try to mimic escapes in PCRE when possible.\n"
		,
		"characters colors escape_codes pcre",
	},
	{
		"SUSPEND",
		TOKEN_TYPE_STRING,
		"<178>Command<278>: #cursor suspend\n"
		"\n"
		"<278>         Temporarily suspends tintin and returns you to your shell.  To\n"
		"<278>         return to tintin, type 'fg' at the shell prompt.\n"
		"\n"
		"<278>         While suspended your tintin sessions will freeze. To keep a\n"
		"<278>         suspended session running use the #daemon command.\n"
		,
		"sessionname"
	},
	{
		"SWITCH",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #switch <178>{<278>conditional<178>} {<278>arguments<178>}\n"
		"\n"
		"<278>         The switch command works similar to the switch statement in other\n"
		"<278>         languages. When the 'switch' command is encountered its body is parsed\n"
		"<278>         and each 'case' command found will be compared to the conditional\n"
		"<278>         argument of the switch and executed if there is a match.\n"
		"\n"
		"<278>         When comparing strings both the switch and case arguments must be\n"
		"<278>         enclosed in quote characters.\n"
		"\n"
		"<278>         If the 'default' command is found and no 'case' statement has been\n"
		"<278>         matched the default command's argument is executed.\n"
		"\n"
		"<178>Example<278>: #switch {1d4} {#case 1 cackle;#case 2 smile;#default giggle}\n"
		,
		"statements"
	},
	{
		"SYSTEM",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #system <178>{<278>command<178>}\n"
		"\n"
		"<278>         Executes the command specified as a shell command.\n"
		,
		"detach script run"
	},
	{
		"TAB",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #tab <178>{<278>word<178>}\n"
		"\n"
		"<278>         Adds a word to the tab completion list, alphabetically sorted.\n"
		"\n"
		"<278>         If no tabs are defined tintin will use the scrollback buffer for auto\n"
		"<278>         tab completion.\n"
		"\n"
		"<278>         Tabbing behavior can be modified with the #cursor tab command which\n"
		"<278>         by default is bound to the tab key.\n"
		"\n"
		"<178>Example<278>: #macro \\t #cursor tab list scrollback caseless forward\n"
		"\n"
		"<178>Comment<278>: You can remove a tab with the #untab command.\n"
		,
		"alias cursor history keypad macro speedwalk"
	},
	{
		"TEXTIN",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #textin <178>{<278>filename<178>} {<278>delay<178>}\n"
		"\n"
		"<278>         Textin allows the user to read in a file, and send its contents\n"
		"<278>         directly to the server.  Useful for doing online creation, or message\n"
		"<278>         writing.\n"
		"\n"
		"<278>         The delay is in seconds and takes a floating point number which is\n"
		"<278>         cumulatively applied to each outgoing line.\n"
		,
		"scan send"
	},
	{
		"TICKER",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #ticker <178>{<278>name<178>} {<278>commands<178>} {<278>interval in seconds<178>}\n"
		"\n"
		"<278>         Executes given command every # of seconds. Floating point precision\n"
		"<278>         for the interval is allowed. A ticker cannot fire more often than\n"
		"<278>         10 times per second.\n"
		"\n"
		"<178>Comment<278>: Tickers don't work in the startup session.\n"
		"\n"
		"<178>Comment<278>: You can remove a ticker with the #unticker command.\n"
		,
		"delay event"
	},
	{
		"TIME",
		TOKEN_TYPE_STRING,
		"<178>Command<278>: #format <178>{<278>variable<178>} {<278>%t<178>} {<278>argument<178>}\n"
		"\n"
		"<278>         The %t format specifier of the #format command allows printing dates\n"
		"<278>         using the strftime() format specifiers. By default the time stamp used\n"
		"<278>         is the current time, if you want to print a past or future date use:\n"
		"\n"
		"<178>Command<278>: #format <178>{<278>variable<178>} {<278>%t<178>} {{<278>argument<178>} <178>{<278>epoch time<178>}}\n"
		"\n"
		"<278>         The current epoch time value is obtained using #format {time} {%T}.\n"
		"\n"
		"<278>         When using %t the argument should contain strftime format specifiers.\n"
		"<278>         Below are some common specifiers, see man strftime for the full list.\n"
		"\n"
		"<278>         %a  Abbreviated name of the day of the week (mon ... sun).\n"
		"<278>         %A  Full name of the day of the week. (Monday ... Sunday)\n"
		"<278>         %b  Abbreviated name of the month (Jan ... Dec)\n"
		"<278>         %B  Full name of the month. (January ... December)\n"
		"<278>         %C  2 digit numeric century. (19 ... 20)\n"
		"<278>         %d  2 digit numeric day of the month (01 ... 31)\n"
		"<278>         %H  2 digit numeric 24-hour clock hour. (00 ... 23)\n"
		"<278>         %I  2 digit numeric 12-hour clock hour. (01 ... 12)\n"
		"<278>         %j  3 digit numeric day of the year (001 ... 366)\n"
		"<278>         %m  2 digit numeric month of the year (01 ... 12)\n"
		"<278>         %M  2 digit numeric minute of the hour (00 ... 59)\n"
		"<278>         %p  Abbreviated 12 hour clock period (AM ... PM)\n"
		"<278>         %P  Abbreviated 12 hour clock period (am ... pm)\n"
		"<278>         %S  2 digit numeric second of the minute (00 ...59\n"
		"<278>         %u  1 digit numeric day of the week (1 ... 7)\n"
		"<278>         %U  2 digit numeric Sunday week of the year (00 ... 53\n"
		"<278>         %w  1 digit numeric day of the week (0 ... 6)\n"
		"<278>         %W  2 digit numeric Monday week of the year (00 ... 53\n"
		"<278>         %y  2 digit numeric year. (70 ... 38)\n"
		"<278>         %Y  4 digit numeric year. (1970 ... 2038)\n"
		"<278>         %z  5 digit timezone offset. (-1200 ... +1400)\n"
		"<278>         %Z  Abbreviated name of the time zone. (CET, GMT, etc)\n"
		,
		"echo event format"
	},
	{
		"TRIGGERS",
		TOKEN_TYPE_STRING,
		"<278>         All available triggers in TinTin++ are displayed when you use the #info\n"
		"<278>         command without an argument. All of them are written to file when you\n"
		"<278>         use the #write command, except commands, histories, and paths.\n"
		"\n"
		"<278>         Triggers can be disabled with the #ignore command. The #message\n"
		"<278>         command can be used to disable messages generated or related to the\n"
		"<278>         corresponding trigger, though this is generally not needed.\n"
		"\n"
		"<278>         The #debug command will generate useful debugging information for the\n"
		"<278>         corresponding trigger when enabled. The #info command can be used on\n"
		"<278>         triggers to generate additional information that might be of use.\n"
		"\n"
		"<178>Example<278>: #info event on\n"
		"\n"
		"<278>         When #info event is set to on you will see when most events are raised.\n"
		"<278>         Since this can get rather spammy some of the events won't generate\n"
		"<278>         messages, unless you have an event in the same category set already.\n"
		"\n"
		"<128>         Text triggers\n"
		"\n"
		"<278>         When a block of text arrives from the host it is split into individual\n"
		"<278>         lines, and all action, prompt, gag, substitute, and highlight triggers\n"
		"<278>         are checked for each line. Only one action can trigger per line, while\n"
		"<278>         the other triggers can trigger multiple times.\n"
		"\n"
		"<128>         Packet fragmentation\n"
		"\n"
		"<278>         MUDs that send long blurbs of text, don't have MCCP support, have a bad\n"
		"<278>         connection, or a combination of all three, will deliver broken packets.\n"
		"<278>         This can cause triggers to not fire, as well as displaying problems if\n"
		"<278>         #split is enabled.\n"
		"\n"
		"<278>         To mitigate this you can use <178>#config packet_patch 0.5<278>.\n"
		"\n"
		"<278>         TinTin++ will automatically enable packet patching if the IAC GA or IAC\n"
		"<278>         EOR telnet sequences are used to mark the end of the prompt. A MUD can\n"
		"<278>         negotiate the EOR option: https://tintin.mudhalla.net/protocols/eor\n"
		"\n"
		"<278>         In addition #prompt can be used to make packet patching less noticable.\n"
		"\n"
		"<128>         Color triggers\n"
		"\n"
		"<278>         By default most color, control, and vt100 codes are stripped from\n"
		"<278>         incoming text before being ran through the trigger engine. To create\n"
		"<278>         a trigger that runs on the unstripped text, the regular expression in\n"
		"<278>         the trigger should start with a ~.\n"
		"\n"
		"<278>         To view control codes you can use <178>#config convert_meta on<278> which will\n"
		"<278>         translate both input and output codes to PCRE escape sequences.\n"
		"\n"
		"<128>         Multi-line triggers\n"
		"\n"
		"<278>         If an action contains the \\n sequence it will be turned into a\n"
		"<278>         multi-line trigger. A multi-line action is executed on incoming blocks\n"
		"<278>         of text from the MUD, and they will not trigger if the regular\n"
		"<278>         expression spans more than one block. You can visualize incoming\n"
		"<278>         blocks by using #event {RECEIVED OUTPUT} {#echo <<888>058>%+80h BLOCK}\n"
		"\n"
		"<278>         Since the %* expression does not capture the \\n sequence it is required\n"
		"<278>         to use %a to capture multiple lines. To capture the start of the block\n"
		"<278>         use \\A and for the end use \\Z. You can use ^ and $ to capture the\n"
		"<278>         start and end of a line.\n"
		"\n"
		"<278>         Multi-line actions trigger before regular actions. Multiple\n"
		"<278>         multi-line actions can trigger per block, and each multi-line action\n"
		"<278>         can trigger multiple times per block. Packet fragmentation is not\n"
		"<278>         currently handled.\n"
		"\n"
		"<278>         Multi-line actions are experimental and subject to change.\n"
		"\n"
		"<128>         Input triggers\n"
		"\n"
		"<278>         The alias, history and pathdir triggers are checked for each line of\n"
		"<278>         input. The macro and tab triggers are checked for key presses.\n"
		"\n"
		"<128>         Time triggers\n"
		"\n"
		"<278>         The delay, path, and ticker triggers will execute at a set timed\n"
		"<278>         interval.\n"
		"\n"
		"<128>         Substitution triggers\n"
		"\n"
		"<278>         The function and variable triggers will generally execute right\n"
		"<278>         before the final processing of a line of text.\n"
		"\n"
		"<128>         Mouse triggers\n"
		"\n"
		"<278>         The button trigger is checked for each mouse input. #config mouse\n"
		"<278>         must be set to on to enable mouse tracking.\n"
		"\n"
		"<128>         Event triggers\n"
		"\n"
		"<278>         Events can be used for a wide variety of pre-defined triggers.\n"
		,
		"pcre substitutions escape_codes"
	},

	{
		"VARIABLE",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #variable <178>{<278>variable name<178>} {<278>text to fill variable<178>}\n"
		"\n"
		"<278>         Variables differ from the %0-99 arguments in the fact that you can\n"
		"<278>         specify a full word as a variable, and they stay in memory for the\n"
		"<278>         full session unless they are changed.  They can be saved in the\n"
		"<278>         coms file, and can be set to different values if you have two or\n"
		"<278>         more sessions running at the same time.  Variables are global for\n"
		"<278>         each session and can be accessed by adding a $ before the variable\n"
		"<278>         name.\n"
		"\n"
		"<178>Example<278>: #alias {target} {#var target %0}\n"
		"<278>         #alias {x}      {kick $target}\n"
		"\n"
		"<278>         The name of a variable must exist of only letters, numbers and\n"
		"<278>         underscores in order to be substituted.  If you do not meet these\n"
		"<278>         requirements do not panic, simply encapsulate the variable in braces:\n"
		"\n"
		"<178>Example<278>: #variable {cool website} {http://tintin.sourceforge.net}\n"
		"<278>         #chat I was on ${cool website} yesterday!.\n"
		"\n"
		"<278>         Variables can be escaped by adding additional $ signs.\n"
		"\n"
		"<178>Example<278>: #var test 42;#showme $$test\n"
		"\n"
		"<278>         Variables can be nested using brackets:\n"
		"\n"
		"<178>Example<278>: #var hp[self] 34;#var hp[target] 46\n"
		"\n"
		"<278>         You can see the first nest of a variable using $variable[+1] and the\n"
		"<278>         last nest using $variable[-1]. Using $variable[-2] will report the\n"
		"<278>         second last variable, and so on. To show all indices use *variable[].\n"
		"<278>         To show all values use $variable[]. To show all values from index 2\n"
		"<278>         through 4 use $variable[+2..4].\n"
		"\n"
		"<278>         Nested variables are also known as tables, table generally being used\n"
		"<278>         to refer to several variables nested within one specific variable.\n"
		"\n"
		"<278>         It's possible to use regular expressions.\n"
		"\n"
		"<178>Example<278>: #show {Targets starting with the letter A: $targets[A%*]\n"
		"\n"
		"<278>         To see the internal index of a variable use &<variable name>. To see\n"
		"<278>         the size of a table you would use: &targets[] or &targets[%*]. A non\n"
		"<278>         existent nested variable will report itself as 0.\n"
		"\n" 
		"<178>Example<278>: #show {Number of targets starting with A: &targets[A%*]\n"
		"\n"
		"<278>         In some scripts you need to know the name of a nested variable. This\n"
		"<278>         is also known as the key, and you can get it using *variable. For\n"
		"<278>         example *target[+1]. To get the first variable's name use *{+1}.\n"
		"\n"
		"<278>         It's also possible to declare a table using brace notation. Using\n"
		"<278>         #var hp[self] 34 is the equivalent of #var {hp} {{self}{34}}. This\n"
		"<278>         also allows merging tables. #var hp[self] 34;#var hp[target] 46 is\n"
		"<278>         the equivalent of #var {hp} {{self}{34} {target}{46}} as well as\n"
		"<278>         #var {hp} {{self}{34}} {{target}{46}} or if you want to get creative\n"
		"<278>         the equivalent of #var hp[self] 34;#var {hp} {$hp} {{target}{46}}.\n"
		"\n"
		"<178>Comment<278>: You can remove a variable with the #unvariable command.\n"
		,
		"cat format function local math replace script"
	},
	{
		"WHILE",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #while <178>{<278>conditional<178>} {<278>commands<178>}\n"
		"\n"
		"<278>         This command works similar to a 'while' statement in other languages.\n"
		"\n"
		"<278>         When a 'while' command is encourated, the conditional is evaluated,\n"
		"<278>         and if TRUE (any non-zero result) the commands are executed. The\n"
		"<278>         'while' loop will be repeated indefinitely until the conditional is\n"
		"<278>         FALSE or the #BREAK or #RETURN commands are found.\n"
		"\n"
		"<278>         The 'while' statement is only evaluated if it is read, so you must\n"
		"<278>         nest it inside a trigger, like an alias or action.\n"
		"\n"
		"<278>         The conditional is evaluated exactly the same as in the 'math' command.\n"
		"\n"
		"<178>Example<278>: #math cnt 0;#while {$cnt < 20} {#math cnt $cnt + 1;say $cnt}\n"
		"\n"
		"<178>Comment<278>: See '#help math', for more information.\n"
		,
		"statements"
	},
	{
		"WRITE",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #write <178>{<278><filename><178>} {<278>[FORCE]<178>}\n"
		"\n"
		"<278>         Writes all current actions, aliases, subs, highlights, and variables\n"
		"<278>         to a command file, specified by filename.\n"
		"\n"
		"<278>         By default you cannot write to .map files to prevent accidentally\n"
		"<278>         overwriting a map file. Use the FORCE argument to ignore this\n"
		"<278>         protection.\n"
		,
		"log read scan textin"
	},
	{
		"ZAP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #zap {[session]}\n"
		"\n"
		"<278>         Kill your current session.  If there is no current session, it will\n"
		"<278>         cause the program to terminate. If you provide an argument it'll zap\n"
		"<278>         the given session instead.\n"
		,
		"all port run session sessionname snoop ssl"
	},
	{
		"",
		TOKEN_TYPE_COMMAND,
		""
		,
		""
	}
};

size_t help_size()
{
	return sizeof(help_table) / sizeof(help_table[0]) - 1;
}
