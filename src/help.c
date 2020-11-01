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

char *help_related(struct session *ses, int index, int html)
{
	char *arg, *tmp, *link;
	static char buf[INPUT_SIZE];

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
			sprintf(tmp, "%s", link);
		}
		else if (html == 2)
		{
			sprintf(link, "\\c<a href='#%s'\\c>%s\\c</a\\c>", capitalize(tmp), tmp);
			sprintf(tmp, "%s", link);
		}
		else if (HAS_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING))
		{
			sprintf(link, "\e]68;6;;%s\a\e[4m%s\e[24m", tmp, tmp);
			sprintf(tmp, "%s", link);
		}

		if (*buf == 0)
		{
			sprintf(buf, "<178>Related<278>: %s", tmp);
		}
		else
		{
			if (*arg)
			{
				cat_sprintf(buf, ", %s", tmp);
			}
			else
			{
				cat_sprintf(buf, " and %s.", tmp);
			}
		}
	}
	pop_call();
	return buf;
}

DO_COMMAND(do_help)
{
	char buf[BUFFER_SIZE], tmp[BUFFER_SIZE], color[COLOR_SIZE];
	int cnt, found;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		tintin_header(ses, 0, " %s ", "HELP");

		*buf = 0;

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			switch (help_table[cnt].type)
			{
				case TOKEN_TYPE_STATEMENT:
//					strcpy(color, COLOR_STATEMENT);
//					break;
				case TOKEN_TYPE_CONFIG:
//					strcpy(color, COLOR_CONFIG);
//					break;
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
				sprintf(tmp, "%.*s\e]68;6;;%s\a\e[4m%s%s\e[24m", 15 - (int) strlen(help_table[cnt].name), "                ", help_table[cnt].name, color, help_table[cnt].name);
			}
			else
			{
				sprintf(tmp, "%s%15s", color, help_table[cnt].name);
			}

			if (strip_vt102_strlen(ses, buf) + 15 > ses->wrap)
			{
				print_lines(ses, SUB_COL, "<088>%s<088>\n", buf);

				*buf = 0;
			}
			cat_sprintf(buf, "%s ", tmp);
		}

		if (*buf)
		{
			print_lines(ses, SUB_COL, "<088>%s<088>\n", buf);
		}
		tintin_header(ses, 0, "");
	}
	else if (!strcasecmp(arg1, "dump"))
	{
		FILE *logfile = fopen("../docs/help.html", "w");

		script_driver(ses, LIST_COMMAND, "#config {log} {html}");

		if (HAS_BIT(ses->logmode, LOG_FLAG_HTML))
		{
			write_html_header(ses, logfile);
		}

		*buf = 0;

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			if (cnt && cnt % 5 == 0)
			{
				substitute(ses, buf, buf, SUB_ESC|SUB_COL);

				logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

				*buf = 0;
			}
			cat_sprintf(buf, " \\c<a href='#%s'\\c>%15s\\c</a\\c>", help_table[cnt].name, help_table[cnt].name);
		}

		cat_sprintf(buf, "\n\n");

		substitute(ses, buf, buf, SUB_ESC|SUB_COL);

		logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			sprintf(buf, "\\c<a name='%s'\\c>\\c</a\\c>\n", help_table[cnt].name);

			substitute(ses, buf, buf, SUB_ESC|SUB_COL);

			logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

			sprintf(buf, "<128>         %s\n", help_table[cnt].name);

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
				print_lines(ses, SUB_COL, "%s<088>\n", help_table[cnt].text);

				if (*help_table[cnt].also)
				{
					print_lines(ses, SUB_COL, "%s<088>\n\n", help_related(ses, cnt, 0));
				}
				return ses;
			}
		}
		found = FALSE;

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			if (match(ses, help_table[cnt].name, arg1, SUB_VAR|SUB_FUN))
			{
				print_lines(ses, SUB_COL, "%s<088>\n", help_table[cnt].text);

				if (*help_table[cnt].also)
				{
					print_lines(ses, SUB_COL, "%s<088>\n\n", help_related(ses, cnt, 0));
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



/*
	This help table is a mess, but I got better things to do - Igor
*/

struct help_type help_table[] =
{
	{
		"ACTION",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #action <178>{<278>message<178>} {<278>commands<178>} {<278>priority<178>}<278>\n"
		"\n"
		"         The #action command can be used to respond with one or several\n"
		"         commands to a specific message send by the server. The %1-%99\n"
		"         variables are substituted from the message and can be used in the\n"
		"         command part of the action.\n"
		"\n"
		"         The priority part is optional and determines the priority of the\n"
		"         action, it defaults to 5.\n"
		"\n"
		"         If the message starts with a ~ color codes must be matched. You can\n"
		"         enable #config {convert meta} on to display meta characters.\n"
		"\n"
		"         For more information on pattern matching see the section on PCRE.\n"
		"\n"
		"<178>Example<278>: #action {%1 tells you '%2'} {tell %1 I'm afk.}\n"
		"\n"
		"         Actions can be triggered by the show command and certain system\n"
		"         messages.\n"
		"\n"
		"         Actions can be triggered by the #show command. If you don't want a\n"
		"         #show to get triggered use: #line ignore #show {text}\n"
		"\n"
		"         Actions are ordered alphabetically and only one action can trigger at\n"
		"         a time. To change the order you can assign a priority, which defaults\n"
		"         to 5, with a lower number indicating a higher priority. The priority\n"
		"         can be a floating point number.\n"
		"\n"
		"         To remove action with %* as the message, use #unaction {%%*} or\n"
		"         #unaction {\%*}. Alternatively you could wrap the action inside a\n"
		"         class, and kill that class when you no longer need the action.\n"
		"\n"
		"<178>Comment<278>: You can remove an action with the #unaction command.\n",

		"pcre gag highlight prompt substitute"
	},
	{
		"ALIAS",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #alias <178>{<278>name<178>} {<278>commands<178>} {<278>priority<178>}<278>\n"
		"\n"
		"         The #alias command can be used to shorten up long or oftenly used\n"
		"         commands. The %1-99 variables are substituted from the arguments when\n"
		"         using an alias and represent the 1st till 99th word which can be used\n"
		"         in the commands part of the alias. If %0 is used it will contain all\n"
		"         arguments. The priority part is optional and determines the priority\n"
		"         of the alias, it defaults to 5.\n"
		"\n"
		"<178>Example<278>: #alias {k} {kill %1;kick}\n"
		"\n"
		"         Typing 'k orc' would result in attacking the orc followed by a kick.\n"
		"\n"
		"         You can create multi-word aliases by using variables in the name\n"
		"         section.\n"
		"\n"
		"<178>Example<278>: #alias {k %1 with %2} {draw %2;attack %1;slash %1 with %2;\n"
		"           kick at %2;strike %1 with %2}\n"
		"\n"
		"         Using the above alias you could type k blue smurf with battle axe\n"
		"\n"
		"         To have an alias that matches all user input, use %* as the name.\n"
		"\n"
		"<178>Example<278>: #alias {%*} {#show You wrote: %0}\n"
		"\n"
		"         Aliases are ordered alphabetically and only one alias can trigger at\n"
		"         a time. To change the order you can assign a priority, which defaults\n"
		"         to 5, with a lower number indicating a higher priority. The priority\n"
		"         can be a floating point number.\n"
		"\n"
		"         To remove an alias with %* as the name, use #unalias {%%*} or #unalias\n"
		"         {\%*}. Alternatively you can wrap the alias inside a class, and kill\n"
		"         that class when you no longer need the alias.\n"
		"\n"
		"         For more information on pattern matching see the section on PCRE.\n"
		"\n"
		"<178>Comment<278>: You can remove an alias with the #unalias command.\n",

		"cursor history keypad macro speedwalk tab"
	},
	{
		"ALL",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #all <178>{<278>string<178>}<278>\n"
		"\n"
		"         If you have multiple sessions in one terminal you can use #all to\n"
		"         execute the command with all sessions, excluding the startup session.\n"
		"\n"
		"<178>Example<278>: #all quit\n"
		"\n"
		"         Sends 'quit' to all sessions.\n",

		"port run session sessionname snoop ssl zap"
	},
	{
		"BELL",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #bell <178>{<278>flash<178>|<278>focus<178>|<278>margin<178>|<278>ring<178>|<278>volume<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         The #bell command without an argument will ring the terminal bell.\n"
		"\n"
		"<178>Example<278>: #action {Bubba tells you} {#bell}\n"
		"\n"
		"         If you aren't watching the screen this could be useful if you don't\n"
		"         want to miss out on a conversation with Bubba. Alternatively you can\n"
		"         use #system to play a sound file.\n"
		"\n"
		"         Some terminals will allow you to use VT100 Operating System Commands\n"
		"         to change the terminal's bell behavior which can be used to flash the\n"
		"         taskbar icon and or focus the window on receival of a bell.\n"
		"\n"
		"<178>Example<278>: #action {Bubba tells you} {#screen save title;#screen set title Tell!;\n"
		"           #bell ring;#delay 10 #screen load title}\n"
		"\n"
		"         The above example will save your window title, change the title to\n"
		"         'Tell!', ring the bell, next reset the window title after 10 seconds.\n"
		"\n"
		"         It's possible to set the terminal to pop to the foreground upon\n"
		"         ringing of the alarm bell.\n"
		"\n"
		"<178>Example<278>: #bell focus on;#bell ring;#bell focus off\n"
		"\n"
		"         It's possible to adjust the alarm bell volume on some terminals.\n"
		"\n"
		"<178>Example<278>: #loop {1} {8} {cnt} {#line substitute variables\n"
		"           #delay {$cnt} {#show Volume $cnt: #bell volume $cnt;#bell}\n",

		"screen"
	},
	{
		"BREAK",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #break\n"
		"\n"
		"         The break command can be used inside the #foreach, #loop, #parse,\n"
		"         #while and #switch statements. When #break is found, tintin will stop\n"
		"         executing the statement it is currently in and move on to the next.\n"
		"\n"
		"<178>Example<278>: #while {1} {#math cnt $cnt + 1;#if {$cnt == 20} {#break}}\n",
		
		"statements"
	},
	{
		"BUFFER",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #buffer <178>{<278>home<178>|<278>up<178>|<278>down<178>|<278>end<178>|<278>lock<178>|<278>find<178>|<278>get<178>|<278>clear<178>}<278>\n"
		"\n"
		"         The buffer command has various options to manipulate your scrollback\n"
		"         buffer.\n"
		"\n"
		"         <178>#buffer {home}\n"
		"<278>\n"
		"         Moves you to the top of your scrollback buffer and displays the page.\n"
		"         Enables scroll lock mode. Most useful when used in a #macro.\n"
		"\n"
		"         <178>#buffer {up} [lines]\n"
		"<278>\n"
		"         Moves your scrollback buffer up one page and displays the page.\n"
		"         Enables scroll lock mode. Most useful when used in a #macro. You\n"
		"         can use #buffer {up} {1} to move the scrollback buffer up 1 line.\n"
		"\n"
		"         <178>#buffer {down} [lines]\n"
		"<278>\n"
		"         Moves your scrollback buffer down one page and displays the page.\n"
		"         Enables scroll lock mode unless at the end. Most useful when used in\n"
		"         a #macro.\n"
		"\n"
		"         <178>#buffer {end}\n"
		"<278>\n"
		"         Moves you to the end of your scrollback buffer and displays the page.\n"
		"         Disables scroll lock mode. Most useful when used in a #macro.\n"
		"\n"
		"         <178>#buffer {find} {[number]} {<string>}\n"
		"<278>\n"
		"         Moves the buffer to the given string which can contain a regular\n"
		"         expression. Optionally you can provide the number of matches to skip,\n"
		"         allowing you to jump further back in the buffer.\n"
		"\n"
		"         <178>#buffer {get} {<variable>} {<lower bound>} {[upper bound]}\n"
		"<278>\n"
		"         Allows you to store one or several lines from your scrollback buffer\n"
		"         (including color codes) into a variable. The lower and upper bound\n"
		"         must be between 1 and the size of the buffer. If the upper bound is\n"
		"         omitted the given line is stored as a standard variable. If an upper\n"
		"         bound is given the lines between the two bounds are stored as a list.\n"
		"\n"
		"         <178>#buffer {info} {[save]} {[variable]}\n"
		"<278>\n"
		"         Display buffer info, optionally save the data to a variable.\n"
		"\n"
		"         <178>#buffer {lock} {on|off}\n"
		"<278>\n"
		"         Toggles the lock on the scrollback buffer. When locked, newly incoming\n"
		"         text won't be displayed, any command will disable the lock, though\n"
		"         several buffer commands will re-enable the lock. When unlocking it'll\n"
		"         move you to the end of your scrollback buffer and display the page.\n"
		"\n"
		"         <178>#buffer {write} {<filename>}\n"
		"<278>\n"
		"         Writes the scrollback buffer to the given file.\n"
		"\n"
		"<178>Example<278>: #macro {\\e[F} {#buffer end}\n",

		"echo grep macro showme screen"
	},
	{
		"BUTTON",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #button <178>{<278>square<178>} {<278>commands<178>} {<278>priority<178>}<278>\n"
		"\n"
		"         The #button command can be used to respond with one or several\n"
		"         commands to a mouse click received within the specified square.\n"
		"         The click coordinates are stored in %0-%3 and can be used in the\n"
		"         command part of the button.\n"
		"\n"
		"         The square part should exists of two coordinates defining the\n"
		"         upper left and bottom right corner using row, col, row, col syntax.\n"
		"         The square arguments should be separated by spaces, semi-colons or\n"
		"         braces.\n"
		"\n"
		"         By default the button is set to respond to a mouse button press, to\n"
		"         respond to other button presses you must add a 5th argument to the\n"
		"         square that defines the button press type. You can enable #info\n"
		"         button on to see button events and their type as they happen.\n"
		"\n"
		"         The priority part is optional and determines the priority of the\n"
		"         button, it defaults to 5.\n"
		"\n"
		"         You must enable #config {mouse tracking} on for buttons to work.\n"
		"\n"
		"         This command draws no visible button, you'll have to do so separately\n"
		"         if needed.\n"
		"\n"
		"<178>Example<278>: #button {1;1;2;2} {#show You clicked the upper left corner.}\n"
		"\n"
		"         Buttons are ordered alphabetically and only one button can trigger at\n"
		"         a time. To change the order you can assign a priority, which defaults\n"
		"         to 5, with a lower number indicating a higher priority. The priority\n"
		"         can be a floating point number.\n"
		"\n"
		"<178>Comment<278>: To see button clicks trigger use #info button on.\n"
		"\n"
		"<178>Comment<278>: You can remove a button with the #unbutton command.\n",

		"delay event ticker"
	},
	{
		"CASE",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #case <178>{<278>conditional<178>} {<278>arguments<178>}<278>\n"
		"\n"
		"         The case command must be used within the #switch command. When the\n"
		"         conditional argument of the case command matches the conditional\n"
		"         argument of the switch command the body of the case is executed.\n"
		"\n"
		"         When comparing strings both the switch and case arguments must be\n"
		"         surrounded in quotes.\n"
		"\n"
		"<178>Example<278>:\n"
		"\n"
		"         #function {reverse_direction}\n"
		"         {\n"
		"             #switch {\"%1\"}\n"
		"             {\n"
		"                 #case {\"north\"} {#return south};\n"
		"                 #case {\"east\"}  {#return west};\n"
		"                 #case {\"south\"} {#return north};\n"
		"                 #case {\"west\"}  {#return east};\n"
		"                 #case {\"up\"}    {#return down};\n"
		"                 #case {\"down\"}  {#return up}\n"
		"             }\n"
		"         }\n"
		"\n"
		"         This function returns the reverse direction. @reverse_direction{north}\n"
		"         would return south.\n",

		"default statements switch"
	},
	{
		"CAT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #cat <178>{<278>variable<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         The cat command will concatinate the argument to the given variable.\n",
		
		"format function local math replace script variable"
	},

	{
		"CHARACTERS",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"         The following special characters are defined:\n"
		"\n"

		"#        The hashtag is the default character for starting a command and is\n"
		"         subsequently known as the command character or tintin character.\n"
		"         When loading a command file the command character is set to the\n"
		"         first character in the file. The character can also be redefined\n"
		"         using #config.\n"
		"\n"
		";        The semi-colon is used as the command separator and can be used to\n"
		"         separate two commands. Multiple commands can be strung together as\n"
		"         well. Trailing semi-colons are ignored when reading a script file\n"
		"         as this is a common error.\n"
		"\n"
		"{ }      Curly brackets aka braces are used for separating multi word command\n"
		"         arguments, nesting commands, and nesting variables. Braces cannot\n"
		"         easily be escaped and must always be used in pairs.\n"
		"\n"
		"\" \"      Quote characters are used for strings in the #math, #if, #switch,\n"
		"         and #case commands. It is however suggested to use a set of braces\n"
		"         { } to define strings instead, particularly when checking strings\n"
		"         that may contain quotes.\n"
		"\n"
		"!        The exclamation sign is used to repeat commands, see #help history.\n"
		"         The character can be redefined using #config.\n"
		"\n"
		"\\        An input line starting with a backslash is send verbatim if you are\n"
		"         connected to a server. This character can be configured with\n"
		"         #config, and is itself send verbatim when the verbatim config mode\n"
		"         is enabled.\n",

		"colors escape function mathematics pcre variable"
	},
	{
		"CHAT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #chat <178>{<278>option<178>} {<278>argument<178>}\n"
		"<278>\n"
		"         The #chat command is used to create peer to peer connections to other\n"
		"         clients, typically for the purpose of chatting and sending files.\n"
		"         This is a decentralized chat system, meaning you have to exchange ip\n"
		"         addresses and port numbers with other users in order to connect to\n"
		"         them.\n"
		"\n"
		"         <178>#chat {init} {port}\n"
		"         <278>  #chat initialize launches your chat server. The port number is\n"
		"           optional, and by default 4050 is used as your port. After using\n"
		"           this command other people can connect to your chat server using\n"
		"           your ip address and port number, and in turn you can connect to\n"
		"           other people.\n"
		"         <178>#chat {name} {name}\n"
		"         <278>  By default your name is set to TinTin, but most servers will\n"
		"           reject you if there is already someone with the name TinTin\n"
		"           connected, so one of the first things you'd want to do is\n"
		"           change your chat name. Your name can include color codes. Some\n"
		"           names aren't accepted by tt++ chat servers, like the name 'all'\n"
		"           and names longer than 20 characters.\n"
		"         <178>#chat {message} {buddy|all} {text}\n"
		"         <278>  This is the main command used for communication. If you use\n"
		"           #chat message all, the message is marked as public and send to\n"
		"           everyone you are connected to.\n"
		"         <178>#chat {accept} {buddy} {boost}\n"
		"         <278>  Accept a file transfer from a buddy. The boost is optional and\n"
		"           must be a value between 1 and 1000.\n"
		"         <178>#chat {call}       {address} {port}\n"
		"         <278>  #chat call is used to connect to another chat server. If you\n"
		"           omit the port argument the default port (4050) is used.\n"
		"         <178>#chat {cancel}     {buddy}            Cancel a file transfer\n"
		"         #chat {color}      {color names}      Set the default color\n"
		"         #chat {decline}    {buddy}            Decline a file transfer\n"
		"         #chat {dnd}                           Decline new connections\n"
		"         #chat {download}   {directory}        Set your download directory\n"
		"         #chat {emote}      {buddy|all} {text} Send an emote message\n"
		"         #chat {forward}    {buddy}            Forward all chat messages\n"
		"         #chat {forwardall} {buddy}            Forward all session output\n"
		"         #chat {filestat}   {buddy}            Show file transfer data\n"
		"         #chat {group}      {buddy} {name}     Assign a chat group\n"
		"         #chat {ignore}     {buddy}            Ignores someone\n"
		"         #chat {info}                          Displays your info\n"
		"         #chat {ip}         {address}          Changes your IP address\n"
		"         #chat {paste}      {buddy|all} {text} Pastes a block of text\n"
		"         #chat {peek}       {buddy}            Show one's public connections\n"
		"         #chat {ping}       {buddy}            Display response time\n"
		"         #chat {private}    {buddy|all}        Make a connection private\n"
		"         #chat {public}     {buddy|all}        Make a connection public\n"
		"         #chat {reply}      {text}             Reply to last private message\n"
		"         #chat {request}    {buddy}            Request one's public connections\n"
		"         #chat {send}       {buddy|all} {text} Sends a raw data string\n"
		"         #chat {sendfile}   {buddy} {filename} Start a file transfer\n"
		"         #chat {serve}      {buddy}            Forward all public chat messages\n"
		"         #chat {uninitialize}                  Uninitialize the chat port.\n"
		"         <178>#chat {who}                           Show all connections\n"
		"         <278>  #chat who shows all people you are connected to. The first\n"
		"           column shows a reference number for the connection, which can be\n"
		"           used instead of the connection's name when sending someone a message\n"
		"           The second column shows the connection's name. The third column\n"
		"           shows flags set for the connection, (P)rivate, (I)gnore, (S)erve,\n"
		"           (F)orward to user, and (f)orward from user. The next columns show\n"
		"           ip, port, and client name.\n"
		"         <178>#chat {zap}        {buddy}            Close a connection\n",
		
		"port"
	},
	{
		"CLASS",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #class <178>{<278>name<178>} {<278>optionkill<178>} {<278>arg<178>}<278>\n"
		"\n"
		"         <178>#class {<name>} {assign} {<argument>}\n"
		"         <278>  Will execute argument with the given class opened.\n"
		"         <178>#class {<name>} {clear}\n"
		"         <278>  Will delete all triggers associated with the given class.\n"
		"         <178>#class {<name>} {close}\n"
		"         <278>  Close the given class, opening the last open class, if any.\n"
		"         <178>#class {<name>} {kill}\n"
		"         <278>  Will clear, close, and remove the class.\n"
		"         <178>#class {<name>} {list}\n"
		"         <278>  List all triggers associated with the given class.\n"
		"         <178>#class {<name>} {load}\n"
		"         <278>  Will load the saved copy of the class from memory.\n"
		"         <178>#class {<name>} {open}\n"
		"         <278>  Open a class, closing a previously opened class. All triggers\n"
		"         <278>  added afterwards are assigned to this class.\n"
		"         <178>#class {<name>} {read} {<filename>\n"
		"         <278>  Will open the class, read the file, and close afterwards.\n"
		"         <178>#class {<name>} {save}\n"
		"         <278>  Will save all triggers of the given class to memory.\n"
		"         <178>#class {<name>} {size} {<variable>}\n"
		"         <278>  Will store the size of the class in a variable.\n"
		"         <178>#class {<name>} {write} {<filename>}\n"
		"         <278>  Will write all triggers of the given class to file.\n"
		"         The {kill} option will delete all triggers of the given class.\n"
		"\n"
		"         Keep in mind that the kill and read option are very fast allowing\n"
		"         them to be used to enable and disable classes.\n"
		"\n"
		"<178>Example<278>: #class extra kill;#class extra read extra.tin\n"
		"         Deletes all triggers of 'extra' class if any. Read 'extra.tin' file,\n"
		"         all triggers loaded will be assigned to the fresh new 'extra' class.\n",
		
		"config debug ignore info kill line message"
	},
	{
		"COLORS",
		TOKEN_TYPE_STRING,
		"<178>Syntax<278>:  <<888>xyz>  with x, y, z being parameters\n"
		"\n"
		"         Parameter 'x': VT100 code\n"
		"\n"
		"         0 - Reset all colors and codes to default\n"
		"         1 - Bold\n"
		"         2 - Dim\n"
		"         3 - Italic\n"
		"         4 - Underscore\n"
		"         5 - Blink\n"
		"         7 - Reverse\n"
		"         8 - Skip (use previous code)\n"
		"\n"
		"         Parameter 'y':  Foreground color\n"
		"         Parameter 'z':  Background color\n"
		"\n"
		"         0 - Black                5 - Magenta\n"
		"         1 - Red                  6 - Cyan\n"
		"         2 - Green                7 - White\n"
		"         3 - Yellow               8 - Skip\n"
		"         4 - Blue                 9 - Default\n"
		"\n"
		"         For xterm 256 colors support use <<888>aaa> to <<888>fff> for RGB foreground\n"
		"         colors and <<888>AAA> to <<888>FFF> for RGB background colors. For the grayscale\n"
		"         foreground colors use <<888>g00> to <<888>g23>, for grayscale background colors\n"
		"         use <<888>G00> to <<888>G23>.\n"
		"\n"
		"         The tertiary colors are as follows:\n"
		"\n"
		"         <<888>acf> - Azure            <<888>afc> - Jade\n"
		"         <<888>caf> - Violet           <<888>cfa> - Lime\n"
		"         <<888>fac> - Pink             <<888>fca> - Orange\n"
		"\n"
		"<178>Example<278>: #show <<888>acf>Azure    <<888>afc>Jade     <<888>caf>Violet\n"
		"<178>Example<278>: #show <<888>cfa>Lime     <<888>fac>Pink     <<888>fca>Orange\n"
		"\n"
		"         For 12 bit truecolor use <<888>F000> to <<888>FFFF> for foreground colors and\n"
		"         <<888>B000> to <<888>BFFF> for background colors.\n"
		"\n"
		"         For 24 bit truecolor use <<888>F000000> to <<888>FFFFFFF> for foreground\n"
		"         colors and <<888>B000000> to <<888>BFFFFFF> for background colors.\n"
		"\n"
		"         If the color code exceeds your configured color mode it will be\n"
		"         downgraded to the closest match.\n",

		"characters coordinates escape mathematics pcre"
	},
	{
		"COMMANDS",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #commands <178>{<278>regex<178>}\n"
		"<278>\n"
		"         Shows all commands or all commands matching the given search\n"
		"         string.\n",
		
		"help info statements"
	},

	{
		"COORDINATES",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"         When the 0,0 coordinate is in the upper left corner TinTin++ uses\n"
		"         a y,x / rows,cols notation. When the 0,0 coordinate is in the\n"
		"         bottom left corner tintin uses a x,y / cols/rows notation.\n"
		"\n"
		"         When a square is defined this is done by specifying the upper left\n"
		"         and bottom right corner of the square using four coordinates.\n"
		"\n"
		"         The vast majority of tintin commands use row,col notation.\n",
		
		"characters colors escape mathematics pcre"
	},
	{
		"CONFIG",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #config <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         This allows you to configure various settings, the settings can be\n"
		"         written to file with the #write command.\n"
		"\n"
		"         If you configure the global session (the one you see as you start up\n"
		"         tintin) all sessions started will inherite these settings.\n"
		"\n"
		"         It's advised to make a configuration file to read on startup if you\n"
		"         do not like the default settings.\n"
		"\n"
		"         Use #config without an argument to see your current configuration as\n"
		"         well as a brief explanation of each config option.\n"
		"\n"
		"         The following config options are not listed by default:\n"
		"\n"
		"         #CONFIG {CHILD LOCK}   {ON|OFF} Enable or disable command input.\n"
		"         #CONFIG {CONVERT META} {ON|OFF} Shows color codes and key bindings.\n"
		"         #CONFIG {DEBUG TELNET} {ON|OFF} Shows telnet negotiations y/n.\n"
		"         #CONFIG {LOG LEVEL}  {LOW|HIGH} LOW logs server output before triggers.\n"
		"         #CONFIG {INHERITANCE}  {ON|OFF} Session trigger inheritance y/n.\n"
		"         #CONFIG {MCCP}         {ON|OFF} Enable or disable MCCP support.\n",

		"class line"
	},
	{
		"CONTINUE",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #continue\n"
		"\n"
		"         The continue command can be used inside the #FOREACH, #LOOP, #PARSE,\n"
		"         #WHILE and #SWITCH commands. When #CONTINUE is found, tintin will go\n"
		"         to the end of the command and proceed as normal, which may be to\n"
		"         reiterate the command.\n"
		"\n"
		"<178>Example<278>: #loop 1 10 cnt {#if {$cnt % 2 == 0} {#continue} {say $cnt}}\n",
		
		"break foreach list loop parse repeat return while"
	},
	{
		"CR",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #cr\n"
		"\n"
		"         Sends a carriage return to the session.  Useful for aliases that need\n"
		"         extra carriage returns.\n"
		"\n"
		"         This command is obsolete as you can accomplish the same using #send\n"
		"         without an argument or #send {}.\n",

		"forall"
	},
	{
		"CURSOR",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #cursor <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         Typing #cursor without an option will show all available cursor\n"
		"         options, their default binding, and an explanation of their function.\n"
		"\n"
		"         The cursor command's primarly goal is adding customizable input editing\n"
		"         with macros. Subsequently many cursor commands only work properly when\n"
		"         used within a macro or event.\n"
		"\n"
		"         <178>#cursor tab <list;scrollback> <backward|forward>\n"
		"         <278>  Tab through the given option(s) going forward or backward.\n"
		,
		"alias history keypad macro speedwalk tab"
	},
	{
		"DAEMON",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #daemon <178>{<278>attach<178>|<278>detach<178>|<278>kill<178>|<278>list<178>} <178>[<278>name<178>]\n"
		"\n"
		"         <278>#daemon provides functionality similar to that of the screen and tmux\n"
		"         utilities.\n"
		"\n"
		"         <178>#daemon attach [name]\n"
		"         <278>  The attach option will try to find a daemonized tintin instance and\n"
		"           take over control. The name argument is optional.\n"
		"\n"
		"         <178>#daemon detach [name]\n"
		"         <278>  The detach option will daemonize tintin, turning it into a background\n"
		"           process. The name argument is optional and is useful if you have\n"
		"           several daemonized tt++ instances running so you can keep them apart.\n"
		"\n"
		"         <178>#daemon kill [name]\n"
		"         <278>  Kills all daemons or daemons with matching name.\n"
		"\n"
		"         <178>#daemon list [name]\n"
		"         <278>  List all daemons or daemons with matching name.\n",
		
		"script system run"
	},
	{
		"DEBUG",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #debug <178>{<278>listname<178>} {<278>on<178>|<278>off<178>|<278>log<178>}<278>\n"
		"\n"
		"         Toggles a list on or off. With no argument it shows your current\n"
		"         settings, as well as the list names that you can debug.\n"
		"\n"
		"         If you for example set ACTIONS to ON you will get debug information\n"
		"         whenever an action is triggered.\n"
		"\n"
		"         #debug {listname} {log} will silently write debugging information to\n"
		"         the log file, you must be logging in order for this to work.\n"
		"\n"
		"         Not every list has debug support yet.\n",
		
		"class ignore info kill message"
	},
	{
		"DEFAULT",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #default <178>{<278>commands<178>}<278>\n"
		"\n"
		"         The default command can only be used within the switch command. When\n"
		"         the conditional argument of non of the case commands matches the switch\n"
		"         command's conditional statement the default command is executed.\n",
		
		"case default else elseif if switch regexp"
	},
	{
		"DELAY",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #delay <178>{<278>seconds<178>} {<278>command<178>}<278>\n"
		"<178>Command<278>: #delay <178>{<278>name<178>} {<278>command<178>} {<278>seconds<178>}<278>\n"
		"\n"
		"         Delay allows you to have tintin wait the given amount of seconds\n"
		"         before executing the given command. tintin won't wait before\n"
		"         executing following input commands if any.\n"
		"\n"
		"         Floating point precision for milliseconds is possible.\n"
		"\n"
		"<178>Example<278>: #show first;#delay {1} {#show last}\n"
		"         This will print 'first', and 'last' around one second later.\n"
		"\n"
		"<178>Comment<278>: If you want to remove a delay with the #undelay command you can add\n"
		"         a name as the first argument, be aware this changes the syntax. If\n"
		"         the name is a number keep in mind that delays with the same numeric\n"
		"         name will not be overwritten\n",
		
		"event ticker"
	},

	{
		"DRAW",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #draw <178>[<278>color<178>] <178>[<278>options<178>] <178><<278>type<178>> <<278>square<178>> {<278>text<178>}\n"
		"<278>\n"
		"         The draw commands allows you to draw various types of lines and shapes\n"
		"         on the screen. Common options and types with a brief description are\n"
		"         provided when you type #draw without an argument.\n"
		"\n"
		"         The square arguments should exists of two coordinates defining the\n"
		"         upper left and bottom right corner using row, col, row, col syntax.\n"
		"\n"
		"         The square arguments can be negative, in which case the coordinates\n"
		"         are calculated from the opposite side of the screen. In the case the\n"
		"         screen is 80 columns wide using #draw box 1 60 10 70 will be the\n"
		"         equivalent of #draw box 1 -21 10 -11, but with different screen\n"
		"         widths the boxes would be drawn in different places.\n"
		"\n"
		"         You can prefix the option with a color code or color name to color the\n"
		"         lines and shapes.\n"
		"\n"
		"         You can further prefix the option as following:\n"
		"\n"
		"         ASCII      will draw in ASCII mode.\n"
		"         BLANKED    will blank the lines and corners.\n"
		"         BOTTOM     will draw on the bottom side if possible.\n"
		"         BOXED      will draw a box along the square.\n"
		"         BUMPED     will precede the draw with an enter.\n"
		"         CALIGN     will center text.\n"
		"         CIRCLED    will circle the corners.\n"
		"         CONVERT    will draw text with meta conversion.\n"
		"         CROSSED    will cross the corners.\n"
		"         CURSIVE    will draw text with cursive letters.\n"
		"         FAT        will draw text with fat letters.\n"
		"         FILLED     will fill circles and jewels.\n"
		"         FOREGROUND will draw even if session is not active.\n"
		"         GRID       will draw TABLE as a grid.\n"
		"         HORIZONTAL will draw horizontal if possible.\n"
		"         HUGE       will draw text in huge letters.\n"
		"         JEWELED    will diamond the corners.\n"
		"         JOINTED    will draw corners.\n"
		"         LALIGN     will left align text.\n"
		"         LEFT       will draw on the left side if possible.\n"
		"         NUMBERED   will draw numbers instead of lines.\n"
		"         PRUNED     will prune the corners.\n"
		"         RALIGN     will right align text.\n"
		"         RIGHT      will draw on the right side if possible.\n"
		"         ROUNDED    will round the corners.\n"
		"         SANSSERIF  will draw text with sansserif letters.\n"
		"         SCALED     will fit the square to the text size.\n"
		"         SCROLL     will draw in the scrolling region.\n"
		"         SHADOWED   will shadow HUGE text.\n"
		"         TALIGN     will top align text too large to fit.\n"
		"         TEED       will tee the corners.\n"
		"         TOP        will draw on the top side if possible.\n"
		"         TRACED     will trace HUGE text.\n"
		"         TUBED      will draw tubes instead of lines.\n"
		"         UALIGN     will unwrap and rewrap text.\n"
		"         UNICODE    will draw in unicode mode.\n"
		"         VERTICAL   will draw vertical if possible.\n"
		"\n"
		"         The following types are available.\n"
		"\n"
		"         <178>[ASCII|UNICODE|HUGE] BOX {[TEXT1]} {[TEXT2]}\n"
		"         <278>  will draw a box.\n"
		"         <178>[BOXED|FOREGROUND] BUFFER\n"
		"         <278>  will draw the scrollback buffer.\n"
		"         <178>[BLANKED|CIRCLED|CROSSED|JEWELED|ROUNDED|TEED|PRUNED] CORNER\n"
		"         <278>  will draw a corner.\n"
		"         <178>[BLANKED|HORIZONTAL|NUMBERED|TUBED|VERTICAL] LINE {[TEXT]}\n"
		"         <278>  will draw a line.\n"
		"         <178>[BOXED] MAP\n"
		"         <278>  will draw the map\n"
		"         <178>RAIN {<VARIABLE>} {[SPAWN]} {[FADE]} {[LEGEND]}\n"
		"         <278>  will draw digital rain.\n"
		"         <178>[JOINTED|TOP|LEFT|BOTTOM|RIGHT] SIDE\n"
		"         <278>  will draw one or more sides of a box.\n"
		"         <178>[GRID] TABLE {[LIST1]} {[LIST2]}\n"
		"         <278> will draw a table.\n"
		"         <178>[HUGE] TILE {[TEXT1]} {[TEXT2]}\n"
		"         <278>  will draw a tile\n"
		"\n"
		"         All draw types take an optional text argument as long as a valid\n"
		"         square with enough space has been defined. Text is automatically\n"
		"         word wrapped and text formatting can be customized with the\n"
		"         CALIGN, LALIGN, RALIGN, and UALIGN options.\n"
		"\n"
		"<178>Example<278>: #draw Blue box 1 1 3 20 {Hello world!}\n",

		"buffer echo grep showme"
	},

	{
		"ECHO",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #echo <178>{<278>format<178>} {<278>argument1<178>} {<278>argument2<178>} {<278>etc<178>}<278>\n"
		"\n"
		"         Echo command displays text on the screen with formatting options. See\n"
		"         the help file for the format command for more information.\n"
		"\n"
		"         The echo command does not trigger actions.\n"
		"\n"
		"         As with the #show command you can split the {format} argument up into\n"
		"         two braced arguments, in which case the 2nd argument is the row number.\n"
		"\n"
		"<178>Example<278>: #echo {The current date is %t.} {%Y-%m-%d %H:%M:%S}\n"
		"         #echo {[%38s][%-38s]} {Hello World} {Hello World}\n"
		"         #echo {{this is %s on the top row} {-1}} {printed}\n",
		
		"buffer format grep showme"
	},
	{
		"EDIT",
		TOKEN_TYPE_COMMAND,
		"#edit <178>{<278>option<178>} <178>[<278>argument<178>]<278>\n"
		"\n"
		"         The edit command can be used to turn the default line editor into a\n"
		"         text editor.\n"
		"\n"
		"         <178>#edit create <arguments><278>\n"
		"         <278>  Create an editor, initialize using the provided arguments.\n"
		"\n"
		"         <178>#edit load <variable><278>\n"
		"         <278>  Create an editor, initialize using the provided list variable.\n"
		"\n"
		"         <178>#edit read <filename><278>\n"
		"         <278>  Create an editor, initialize using the provided file.\n"
		"\n"
		"         <178>#edit resume<278>\n"
		"         <278>  Resume editing after a suspension.\n"
		"\n"
		"         <178>#edit save <variable><278>\n"
		"         <278>  Save the editor to the provided variable.\n"
		"\n"
		"         <178>#edit suspend<278>\n"
		"         <278>  Suspend editing, similar to pressing enter except that no\n"
		"         <278>  events are triggered.\n"
		"\n"
		"         <178>#edit write <filename<278>\n"
		"         <278>  Write the editor content to file.\n"
		"\n"
		"<178>Example<278>: #edit create {bli}{bla}{blo}\n",
		
		"cursor macro"
	},
	{
		"EDITING",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"
		"<268>│<178>alt b                    <268>││<178>cursor backward word                        <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>alt f                    <268>││<178>cursor forward word                         <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n"

		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"
		"<268>│<178>ctrl a                   <268>││<178>cursor home                                 <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl b                   <268>││<178>cursor backward                             <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl c                   <268>││<178>clear line                                  <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl d                   <268>││<178>delete or exit                              <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl e                   <268>││<178>cursor end                                  <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl f                   <268>││<178>cursor forward                              <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl g                   <268>││<178>                                            <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl h                   <268>││<178>backspace                                   <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl i                   <268>││<178>tab                                         <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl j                   <268>││<178>enter                                       <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl k                   <268>││<178>clear line right                            <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl l                   <268>││<178>redraw input                                <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl m                   <268>││<178>enter                                       <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl n                   <268>││<178>input history next                          <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl o                   <268>││<178>                                            <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl p                   <268>││<178>input history prev                          <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl q                   <268>││<178>                                            <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl r                   <268>││<178>input history search                        <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl s                   <268>││<178>                                            <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl t                   <268>││<178>scroll buffer lock                          <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl u                   <268>││<178>clear line left                             <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl v                   <268>││<178>convert meta characters                     <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl w                   <268>││<178>delete word left                            <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl x                   <268>││<178>                                            <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl y                   <268>││<178>paste                                       <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl z                   <268>││<178>suspend                                     <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n"

		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"
		"<268>│<178>arrow left               <268>││<178>cursor left                                 <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>arrow right              <268>││<178>cursor right                                <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>arrow up                 <268>││<178>previous input line                         <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>arrow down               <268>││<178>next input line                             <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"		
		"<268>│<178>ctrl arrow left          <268>││<178>cursor left word                            <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl arrow right         <268>││<178>cursor right word                           <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"
		"<268>│<178>backspace                <268>││<178>backspace                                   <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>alt backspace            <268>││<178>clear line left                             <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl backspace           <268>││<178>clear line                                  <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"
		"<268>│<178>delete                   <268>││<178>delete                                      <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl delete              <268>││<178>delete word right                           <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"		
		"<268>│<178>end                      <268>││<178>cursor end                                  <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl end                 <268>││<178>scroll buffer end                           <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n"

		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"		
		"<268>│<178>enter                    <268>││<178>enter                                       <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>shift-enter              <268>││<178>soft enter                                  <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"
		"<268>│<178>home                     <268>││<178>cursor home                                 <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>ctrl home                <268>││<178>scroll buffer home                          <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n"
		
		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"		
		"<268>│<178>page up                  <268>││<178>scroll buffer up                            <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>page down                <268>││<178>scroll buffer down                          <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n"

		"<268>┌─────────────────────────┐┌────────────────────────────────────────────┐<278>\n"		
		"<268>│<178>tab                      <268>││<178>complete word forward                       <268>│<278>\n"
		"<268>├─────────────────────────┤├────────────────────────────────────────────┤<278>\n"
		"<268>│<178>shift-tab                <268>││<178>complete word backward                      <268>│<278>\n"
		"<268>└─────────────────────────┘└────────────────────────────────────────────┘<278>\n",

		"cursor edit macro"
	},

	{
		"ELSE",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #else <178>{<278>commands<178>}<278>\n"
		"\n"
		"         The else statement should follow an #IF or #ELSEIF statement and is\n"
		"         only called if the proceeding #IF or #ELSEIF is false.\n"
		"\n"
		"<178>Example<278>: #if {1d2 == 1} {smile};#else {grin}\n",
		
		"case default elseif if switch regexp"
	},
	{
		"ELSEIF",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #elseif <178>{<278>conditional<178>} {<278>commands<178>}<278>\n"
		"\n"
		"         The elseif statement should follow an #IF or #ELSEIF statement and is\n"
		"         only called when the statement is true and the proceeding #IF and\n"
		"         #ELSEIF statements are false.\n"
		"\n"
		"<178>Example<278>: #if {1d3 == 1} {smirk};#elseif {1d2 == 1} {snicker}\n",
		
		"case default else if switch regexp"
	},
	{
		"END",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #end {<message>}\n"
		"\n"
		"         Terminates tintin and return to unix.  On most systems, ctrl-c has\n"
		"         the same result.\n"
		"\n"
		"         The message is optional and is printed before tintin exits. When\n"
		"         using #end {\\} tintin will terminate silently.\n",
		
		"zap"
	},
	{
		"ESCAPE CODES",
		TOKEN_TYPE_STRING,
		"<278>         You may use the escape character \\ for various special characters.\n"
		"\n"
		"         \\a    beep the terminal.\n"
		"         \\c    send a control character, \\ca for ctrl-a.\n"
		"         \\e    start an escape sequence.\n"
		"         \\f    send a form feed.\n"
		"         \\n    send a line feed.\n"
		"         \\r    send a carriage return.\n"
		"         \\t    send a horizontal tab.\n"
		"         \\x    print an 8 bit character using hexadecimal, \\xFF for example.\n"
		"         \\x7B  send the '{' character.\n"
		"         \\x7D  send the '}' character.\n"
		"         \\u    print a 16 bit unicode character, \\uFFFD for example.\n"
		"         \\U    print a 21 bit unicode character, \\U02AF21 for example.\n"
		"         \\v    send a vertical tab\n"
		"\n"
		"         Ending a line with \\ will stop tintin from appending a line feed.\n"
		"         To escape arguments in an alias or action use %%0 %%1 %%2 etc.\n",
		
		"characters colors coordinates mathematics pcre"
	},
	{
		"EVENT",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #event <178>{<278>event type<178>}<278>\n"
		"\n"
		"         Events allow you to create triggers for predetermined client events.\n"
		"\n"
		"         Use #event without an argument to see a list of possible events with\n"
		"         a brief description. Use #event %* to see the current list of defined\n"
		"         events. Use #info {events} {on} to see events get thrown.\n"
		"\n"
		"         Events, like triggers in general, are case sensitive and event names\n"
		"         must be defined using all upper case letters.\n"
		"\n"
		"         To enable mouse events use #config mouse_tracking on, to see mouse\n"
		"         events use #config mouse_tracking info.\n"
		"\n"
		"         <128>CATCH EVENTS\n"
		"\n"
		"         <178>CATCH <EVENT>\n"
		"         <278>  Some events can be prefixed with CATCH to interrupt default\n"
		"         <278>  behavior.\n"
		"\n"
		"         <128>CLASS EVENTS\n"
		"\n"
		"         <178>CLASS ACTIVATED [CLASS],  CLASS_CLEAR [CLASS],  CLASS CREATED [CLASS],\n"
		"         <178>CLASS DEACTIVATED [CLASS],  CLASS DESTROYED [CLASS],\n"
		"         <178>CLASS_LOAD [CLASS]\n"
		"         <278>  %0 class name\n"
		"\n"

		"         <128>GAG EVENTS\n"
		"\n"
		"         <178>GAG <EVENT>\n"
		"         <278>  Some events can be prefixed with GAG to gag default system\n"
		"         <278>  messages.\n"
		"\n"
		"         <128>INPUT EVENTS<278>\n"
		"\n"
		"         <178>EDIT STARTED, EDIT FINISHED\n"
		"         <278>  %0 name  %1 lines %2 size %3 data\n"
		"\n"
		"         <178>RECEIVED KEYPRESS, PROCESSED KEYPRESS\n"
		"         <278>  %0 character  %1 unicode index  %2 edit row  %3 edit column\n"
		"\n"
		"         <178>RECEIVED INPUT [NAME]\n"
		"         <278>  %0 raw text\n"
		"\n"
		"         SEND OUTPUT            %0 raw text %1 size\n"
		"         SENT OUTPUT            %0 raw text %1 size\n"
		"\n"
		"         <128>MAP EVENTS\n"
		"\n"
		"         <178>END OF PATH,  END OF RUN, MAP UPDATED VTMAP\n"
		"         <278>  These events have no additional arguments.\n"
		"\n"
		"         <178>MAP CREATE EXIT, MAP DELETE EXIT\n"
		"         <278>  %0 vnum  %1 exit name  %2 exit cmd  %3 exit vnum\n"
		"\n"
		"         <178>MAP CREATE ROOM, MAP DELETE ROOM\n"
		"         <278>  %0 vnum  %1 name\n"
		"\n"
		"         <178>MAP ENTER MAP, MAP EXIT MAP\n"
		"         <278>  %0 vnum\n"
		"\n"
		"         <178>MAP ENTER ROOM [VNUM]\n"
		"         <278>  %0 new vnum  %1 old vnum %2 direction\n"
		"\n"
		"         <178>MAP EXIT ROOM [VNUM]\n"
		"         <278>  %0 old vnum  %1 new vnum %2 direction\n"
		"\n"
		"         <178>MAP FOLLOW MAP\n"
		"         <278>  %0 old vnum  %1 new vnum  %2 exit name\n"
		"\n"
		"         <178>MAP REGION <MOUSE>, MAP ROOM <MOUSE>\n"
		"         <278>  %0 row  %1 col  %2 -row  %3 -col  %5 vnum  %6 info\n"
		"\n"
		"         <128>MOUSE EVENTS<278>\n"
		"\n"
		"         DOUBLE-CLICKED <MOUSE> %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         LONG-CLICKED <MOUSE>   %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         MOVED <MOUSE>          %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         PRESSED <MOUSE>        %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         SHORT-CLICKED <MOUSE>  %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         RELEASED <MOUSE>       %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         SCROLLED <MOUSE>       %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         TRIPLE-CLICKED <MOUSE> %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"\n"
		"         <178>MAP <MOUSE EVENT>\n"
		"         <278>  Mouse events can be prefixed with MAP to only trigger when the mouse\n"
		"         <278>  event occurs inside the VT100 map region.\n"
		"\n"
		"         <178>SWIPED [DIR]\n"
		"         <278>  %0 dir  %1 button  %2 row  %3 col  %4 -row  %5 -col\n"
		"         <278>                     %6 row  %7 col  %8 -row  %9 -col %10 rows %11 cols\n"
		"\n"
		"         <128>OUTPUT EVENTS\n"
		"\n"
		"         <178>BUFFER UPDATE,  DISPLAY UPDATE\n"
		"         <278>  These events have no additional arguments.\n"
		"\n"
		"         RECEIVED LINE          %0 raw text %1 plain text\n"
		"         RECEIVED OUTPUT        %0 raw text\n"
		"         RECEIVED PROMPT        %0 raw text %1 plain text\n"
		"\n"
		"         <128>PORT EVENTS\n"
		"\n"
		"         <178>CHAT MESSAGE,  PORT MESSAGE\n"
		"         <278>  %0 raw text  %1 plain text\n"
		"\n"
		"         <178>PORT CONNECTION        <278>%0 name %1 ip %2 port\n"
		"         <178>PORT DISCONNECTION     <278>%0 name %1 ip %2 port\n"
		"         <178>PORT LOG MESSAGE       <278>%0 name %1 ip %2 port %3 data %4 plain data\n"
		"         <178>PORT RECEIVED MESSAGE  <278>%0 name %1 ip %2 port %3 data %4 plain data\n"
		"\n"
		"         <128>SYSTEM EVENTS<278>\n"
		"\n"
		"         DAEMON ATTACHED        %0 file %1 pid\n"
		"         DAEMON DETACHED        %0 file %1 pid\n"
		"         PROGRAM START          %0 startup arguments\n"
		"         PROGRAM TERMINATION    %0 goodbye message\n"
		"\n"
		"         READ ERROR             %0 filename %1 error message\n"
		"         READ FILE              %0 filename\n"
		"         WRITE ERROR            %0 filename %1 error message\n"
		"         WRITE FILE             %0 filename\n"
		"\n"
		"         SYSTEM CRASH           %0 message\n"
		"         SYSTEM ERROR           %0 name %1 system msg %2 error %3 error msg\n"
		"         UNKNOWN COMMAND        %0 raw text\n"

		"\n"
		"         <128>TELNET EVENTS\n"
		"\n"
		"         <178>IAC <EVENT>\n"
		"         <278>  IAC TELNET events are made visable using #config telnet info.\n"
		"\n"
		"         <178>IAC SB GMCP [MODULE]   %0 module    %1 data  %1 plain data\n"
		"         <178>IAC SB MSSP            %0 variable  %1 data\n"
		"         <178>IAC SB MSDP            %0 variable  %1 data  %2 plain data\n"
		"         <178>IAC SB MSDP [VAR]      %0 variable  %1 data  %2 plain data\n"
		"         <178>IAC SB NEW-ENVIRON     %0 variable  %1 data  %2 plain data\n"
		"         <178>IAC SB ZMP <VAR>       %0 variable  %1 data\n"
		"         <178>IAC SB <VAR>           %0 variable  %1 raw data  %2 plain data\n"
		"\n"
		"         <128>TIME EVENTS\n"
		"\n"
		"         <178>DATE <MONTH-DAY OF MONTH> [HOUR:MINUTE], DAY [DAY OF MONTH],\n"
		"         <178>HOUR [HOUR], MONTH [DAY OF MONTH], TIME <HOUR:MINUTE>[:SECOND],\n"
		"         <178>WEEK [DAY OF WEEK], YEAR [YEAR]\n"
		"         <278>  %0 year  %1 month  %2 day of week  %3 day of month  %4 hour\n"
		"         <278>  %5 minute  %6 second\n"
		"\n"
		"         <128>SCAN EVENTS<278>\n"
		"\n"
		"         SCAN CSV HEADER        %0 all args %1 arg1 %2 arg2 .. %99 arg99\n"
		"         SCAN CSV LINE          %0 all args %1 arg1 %2 arg3 .. %99 arg99\n"
		"         SCAN TSV HEADER        %0 all args %1 arg1 %2 arg3 .. %99 arg99\n"
		"         SCAN TSV LINE          %0 all args %1 arg1 %2 arg3 .. %99 arg99\n"
		"\n"
		"         <128>SCREEN EVENTS<278>\n"
		"\n"
		"         SCREEN FOCUS           %0 focus (0 or 1)\n"
		"         SCREEN LOCATION        %0 rows %1 cols  %2 height %3 width\n"
		"\n"
		"         <178>SCREEN MOUSE LOCATION\n"
		"         <278>  %0 row  %1 col  %2 -row  %3 -col  %4 pix row  %5 pix col\n"
		"         <278>  %6 -pix row  %7 -pix col  %8 location\n"
		"\n"
		"         SCREEN RESIZE          %0 rows %1 cols %2 height %3 width\n"
		"         SCREEN SPLIT           %0 top row %1 top col %2 bot row %3 bot col\n"
		"         SCREEN UNSPLIT         %0 top row %1 top col %2 bot row %3 bot col\n"
		"\n"
		"         <128>SESSION EVENTS<278>\n"
		"\n"
		"         SESSION ACTIVATED      %0 name\n"
		"         SESSION CONNECTED      %0 name %1 host %2 ip %3 port\n"
		"         SESSION CREATED        %0 name %1 host %2 ip %3 port\n"
		"         SESSION DEACTIVATED    %0 name\n"
		"         SESSION DISCONNECTED   %0 name %1 host %2 ip %3 port\n"
		"         SESSION TIMED OUT      %0 name %1 host %2 ip %3 port\n"
		"\n"
		"         <128>VARIABLE EVENTS<278>\n"
		"\n"
		"         VARIABLE UPDATE <VAR>  %0 name %1 new value %2 path\n"
		"         VARIABLE UPDATED <VAR> %0 name %1 new value %2 path\n"
		"\n"
		"         <128>VT100 EVENTS<278>\n"
		"\n"
		"         VT100 SCROLL REGION    %0 top row %1 bot row %2 rows %3 cols %4 wrap\n"
		"\n"
		"         To see all events trigger use #event info on. Since this can get\n"
		"         rather spammy it's possible to gag event info messages.\n"
		"\n"
		"<178>Example<278>: #event {SESSION CONNECTED} {#read mychar.tin}\n"
		"\n"
		"<178>Comment<278>: You can remove an event with the #unevent command.\n",
		
		"button delay ticker"
	},
	{
		"FOREACH",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #foreach <178>{<278>list<178>} {<278>variable<178>} {<278>commands<178>}<278>\n"
		"\n"
		"         For each item in the provided list the foreach statement will update\n"
		"         the given variable and execute the command part of the statement. List\n"
		"         elements must be separated by braces or semicolons.\n"
		"\n"
		"<178>Example<278>: #foreach {bob;tim;kim} {name} {tell $name Hello}\n"
		"<178>Example<278>: #foreach {{bob}{tim}{kim}} {name} {tell $name Hello}\n",
		
		"break continue list loop parse repeat return while"
	},
	{
		"FORMAT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #format <178>{<278>variable<178>} {<278>format<178>} {<278>argument1<178>} {<278>argument2<178>} {<278>etc<178>}<278>\n"
		"\n"
		"         Allows you to store a string into a variable in the exact same way\n"
		"         C's sprintf works with a few enhancements and limitations like a\n"
		"         maximum of 30 arguments.\n"
		"\n"
		"         If you use #format inside an alias or action you must escape %1s as\n"
		"         %+1s or %%1s or %\\1s so the %1 isn't substituted by the trigger.\n"
		"\n"
		"         #format {test} {%+9s} {string}  pad string with up to 9 spaces\n"
		"         #format {test} {%-9s} {string}  post pad string with up to 9 spaces\n"
		"         #format {test} {%.8s} {string}  copy at most 8 characters\n"
		"         #format {test} {%a}   {number}  print corresponding charset character\n"
		"         #format {test} {%c}   {string}  use a highlight color name\n"
		"         #format {test} {%d}   {number}  print a number with integer formatting\n"
		"         #format {test} {%f}   {string}  perform floating point math\n"
		"         #format {test} {%g}   {number}  perform thousand grouping on {number}\n"
		"         #format {test} {%h}   {string}  turn text into a header line\n"
		"         #format {test} {%l}   {string}  lowercase text\n"
		"         #format {test} {%m}   {string}  perform mathematical calculation\n"
		"         #format {test} {%n}     {name}  capitalize the first letter\n"
		"         #format {test} {%p}   {string}  strip leading and trailing spaces\n"
		"         #format {test} {%r}   {string}  reverse text, hiya = ayih\n"
		"         #format {test} {%s}   {string}  print given string\n"
		"         #format {test} {%t}   {format}  display time with strftime format\n"
		"                                         optional {{format}{time}} syntax\n"
		"         #format {test} {%u}   {string}  uppercase text\n"
		"         #format {list} {%w}   {string}  store word wrapped text in {list}\n"
		"                                         optional {{string}{width}} syntax\n"
		"         #format {test} {%x}      {hex}  print corresponding charset character\n"
		"         #format {test} {%A}     {char}  store corresponding character value\n"
		"         #format {test} {%C}   {number}  store number in chronological notation\n"
		"         #format {test} {%D}      {hex}  convert hex to decimal in {test}\n"
		"         #format {hash} {%H}   {string}  store a 64 bit string hash in {hash}\n"
		"         #format {test} {%L}   {string}  store the string length in {test}\n"
		"         #format {test} {%M}   {number}  convert number to metric in {test}\n"
		"         #format {test} {%S}   {string}  store the number of spelling errors\n"
		"         #format {time} {%T}         {}  store the epoch time in {time}\n"
		"         #format {time} {%U}         {}  store the micro epoch time in {time}\n"
		"         #format {test} {%X}      {dec}  convert dec to hexadecimal in {test}\n\n"
		"         #format {test} {%%}             a literal % character\n"
		"\n"
		"<178>Comment<278>: See #help TIME for help on the %t argument.\n",
		
		"cat echo function local math replace script time variable"
	},

	{
		"FUNCTION",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #function <178>{<278>name<178>} {<278>operation<178>}<278>\n"
		"<278>\n"
		"         Functions allow you to execute a script within a line of text, and\n"
		"         replace the function call with the line of text generated by the\n"
		"         function.\n"
		"<278>\n"
		"         Be aware that each function should use the #return command at the\n"
		"         end of the function with the result, or set the {result} variable.\n"
		"<278>\n"
		"         To use a function use the @ character before the function name.\n"
		"         The function arguments should be placed between braces behind the\n"
		"         function name with argument separated by semicolons.\n"
		"<278>\n"
		"         Functions can be escaped by adding additional @ signs.\n"
		"\n"
		"<178>Example<278>: #function test #return 42;#showme @@test{}\n"
		"<278>\n"
		"         The function itself can use the provided arguments which are stored\n"
		"         in %1 to %99, with %0 holding all arguments.\n"
		"\n"
		"<178>Example<278>: #function {rnd} {#math {result} {1 d (%2 - %1 + 1) + %1 - 1}}\n"
		"         #show A random number between 100 and 200: @rnd{100;200}\n"
		"\n"
		"<178>Example<278>: #function gettime {#format result %t %H:%M}\n"
		"         #show The current time is @gettime{}\n"
		"\n"
		"<178>Comment<278>: You can remove a function with the #unfunction command.\n",
		
		"format local math replace script variable"
	},
	{
		"GAG",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #gag <178>{<278>string<178>}<278>\n"
		"\n"
		"         Removes any line that contains the string.\n"
		"\n"
		"<178>Comment<278>: See '#help action', for more information about triggers.\n"
		"\n"
		"         There are a system messages that can be gagged using gag events.\n"
		"\n"
		"<178>Comment<278>: You can remove a gag with the #ungag command.\n",
		
		"action highlight prompt substitute"
	},
	{
		"GREETING",
		TOKEN_TYPE_STRING,
		"<268>      #<268>##################################################################<268>#\n"
		"<268>      #<278>                                                                  <268>#\n"
		"<268>      #<278>                    T I N T I N + +   "CLIENT_VERSION"                    <268>#\n"
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
		"<178>Command<278>: #grep <178>[<278>page<178>] {<278>search string<178>}<278>\n"
		"\n"
		"         This command allows you to search for matching lines in your scroll\n"
		"         back buffer. The amount of matches shown equals your screen size. If\n"
		"         you want to search back further use the optional page number. You can\n"
		"         use wildcards for better search results. Be aware the search string\n"
		"         is case sensitive, which can be disabled by using %i.\n"
		"\n"
		"         By default grep searches from the end of the scrollback buffer to the\n"
		"         beginning, this can be reversed by using a negative page number.\n"
		"\n"
		"<178>Example<278>: #grep Bubba tells you\n"
		"         This will show all occasions where bubba tells you something.\n",
		
		"buffer echo showme"
	},
	{
		"HELP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #help <178>{<278>subject<178>}<278>\n"
		"\n"
		"         Without an argument #help will list all available help subjects.\n"
		"\n"
		"         Using #help %* will display all help entries.\n",
		
		"commands debug ignore info message statements"
	},
	{
		"HIGHLIGHT",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #highlight <178>{<278>string<178>} {<278>color names<178>} {<278>priority<178>}<278>\n"
		"\n"
		"         The highlight command is used to allow you to highlight strings of text.\n"
		"\n"
		"         Available color options are:\n"
		"\n"
		"         reset      - resets the color state to default\n"
		"         light      - turns the color light in 16 color mode.\n"
		"         dark       - turns the color dark in 16 color mode.\n"
		"         underscore - underscores the text.\n"
		"         blink      - makes the text blink.\n"
		"         reverse    - reverse foreground and background color.\n"
		"         b          - makes next color the background color.\n"
		"\n"
		"         Available color names are:\n"
		"\n"
		"         <<888>F06B> - azure                 <<888>F08F> - Azure\n"
		"         <<888>F00B> - blue                  <<888>F00F> - Blue\n"
		"         <<888>F0BB> - cyan                  <<888>F0FF> - Cyan\n"
		"         <<888>F000> - ebony                 <<888>F666> - Ebony\n"
		"         <<888>F0B0> - green                 <<888>F0F0> - Green\n"
		"         <<888>F0B6> - jade                  <<888>F0F8> - Jade\n"
		"         <<888>F6B0> - lime                  <<888>F8F0> - Lime\n"
		"         <<888>FB0B> - magenta               <<888>FF0F> - Magenta\n"
		"         <<888>FB60> - orange                <<888>FF80> - Orange\n"
		"         <<888>FB06> - pink                  <<888>FF08> - Pink\n"
		"         <<888>FB00> - red                   <<888>FF00> - Red\n"
		"         <<888>F888> - silver                <<888>FDDD> - Silver\n"
		"         <<888>F860> - tan                   <<888>FDB0> - Tan\n"
		"         <<888>F60B> - violet                <<888>F80F> - Violet\n"
		"         <<888>FBBB> - white                 <<888>FFFF> - White\n"
		"         <<888>FBB0> - yellow                <<888>FFF0> - Yellow\n"
		"\n"

		"         The %1-99 variables can be used as 'wildcards' that will match with any\n"
		"         text. They are useful for highlighting a complete line. The %0 variable\n"
		"         should never be used in highlights.\n"
		"\n"
		"         You may start the string to highlight with a ^ to only highlight text\n"
		"         if it begins the line.\n"
		"\n"
		"         Besides color names also <<888>abc> color codes can be used.\n"
		"\n"
		"<178>Example<278>: #high {Valgar} {reverse blink}\n"
		"         Prints every occurrence of 'Valgar' in blinking reverse video.\n"
		"\n"
		"<178>Example<278>: #high {^You %1} {bold cyan}\n"
		"         Boldfaces any line that starts with 'You' in cyan.\n"
		"\n"
		"<178>Example<278>: #high {Bubba} {red underscore b green}\n"
		"         Highlights the name Bubba as red underscored text on green background.\n"
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
		"         #history <178>{<278>insert<178>}    {<278>command<178>}<278>    Insert a command.\n"
		"         #history <178>{<278>list<178>}<278>                   Display the entire command history.\n"
		"         #history <178>{<278>read<178>}      {<278>filename<178>}<278>   Read a command history from file.\n"
		"         #history <178>{<278>write<178>}     {<278>filename<178>}<278>   Write a command history to file.\n"
		"\n"
		"         Without an argument all available options are shown.\n"
		"\n"
		"         By default all commands are saved to the history list and the history\n"
		"         list is saved between sessions in the ~/.tintin/history.txt file.\n"
		"\n"
		"         You can set the character to repeat a command in the history with the\n"
		"         #config {REPEAT CHAR} {<character>} configuration option, by default\n"
		"         this is set to the exclamation mark.\n"
		"\n"
		"         You can use ! by itself to repeat the last command, or !<text> to\n"
		"         repeat the last command starting with the given text.\n"
		"\n"
		"         You can use #config {REPEAT ENTER} {ON} to repeat the last command\n"
		"         when you press enter on an empty line.\n"
		"\n"
		"         You can press ctrl-r to enter an interactive regex enabled history\n"
		"         search mode, or by issuing #cursor {history search}.\n"
		"\n"
		"         TinTin++ tries to bind the arrow up and down keys to scroll through\n"
		"         the history list by default. You can bind these with a macro yourself\n"
		"         using #cursor {history next} and #cursor {history prev}. Many #cursor\n"
		"         commands only work properly when bound with a macro.\n",
		
		"alias cursor keypad macro speedwalk tab"
	},
	{
		"IF",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #if <178>{<278>conditional<178>} {<278>commands if true<178>} {<278>commands if false<178>}<278>\n"
		"\n"
		"         The 'if' command is one of the most powerful commands added since\n"
		"         TINTIN III. It works similar to an 'if' statement in other languages,\n"
		"         and is strictly based on the way C handles its conditional statements.\n"
		"         When an 'if' command is encountered, the conditional statement is\n"
		"         evaluated, and if TRUE (any non-zero result) the commands are executed.\n"
		"\n"
		"         The 'if' statement is only evaluated if it is read, so you must nest\n"
		"         the 'if' statement inside another statement (most likely an 'action'\n"
		"         command). The conditional is evaluated exactly the same as in the\n"
		"         'math' command only instead of storing the result, the result is used\n"
		"         to determine whether to execute the commands.\n"
		"\n"
		"<178>Example<278>: #action {%0 gives you %1 gold coins.} {#if {%1>5000} {thank %0}}\n"
		"         If someone gives you more than 5000 coins, thank them.\n"
		"\n"
		"<178>Comment<278>: See '#help math', for more information.\n",
		
		"case default else elseif switch regexp"
	},
	{
		"IGNORE",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #ignore <178>{<278>listname<178>} {<278>on<178>|<278>off<178>}<278>\n"
		"\n"
		"         Toggles a list on or off. With no arguments it shows your current\n"
		"         settings, as well as the list names that you can ignore.\n"
		"\n"
		"         If you for example set ACTIONS to OFF actions will no longer trigger.\n"
		"         Not every list can be ignored.\n",

		"class debug info kill message"
	},
	{
		"INDEX",
		TOKEN_TYPE_STRING,
		"<128>         INDEX\n"
		"<278>\n"
		"         On this page you'll find an introduction to using TinTin++. Additional\n"
		"         information can be found in the individual help sections.\n"
		"<128>\n"
		"         Starting and Ending\n"
		"<278>\n"
		"         The syntax for starting TinTin++ is: ./tt++ [command file]\n"
		"\n"
		"         Read more about the command file in the files section below. Remember\n"
		"         one thing though. All actions, aliases, substitutions, etc, defined\n"
		"         when starting up TinTin++ are inherited by all sessions.\n"
		"\n"
		"         If you want to exit TinTin++ type '#end' or press ctrl-d on an empty\n"
		"         line.\n"
		"\n"
		"         For the WinTin++ users, if you want to paste text use shift-insert,\n"
		"         text is automatically copied upon selection. This is typical Linux\n"
		"         behavior, but it can take some getting used to.\n"
		"\n"
		"<128>\n"
		"         Basic features\n"
		"<278>\n"
		"         I'll start by explaining some of the very basic and important features:\n"
		"\n"
		"         All TinTin++ commands starts with a '#'.\n"
		"\n"
		"<178>Example<278>: #help -- #help is a client command, and isn't send to the server.\n"
		"\n"
		"         All TinTin++ commands can be abbreviated when typed.\n"
		"\n"
		"         #he -- Typing #he is the same as typing #help though it's suggested to\n"
		"         use at least 3 letter abbreviations just in case another command is\n"
		"         added that starts with 'he'.\n"
		"\n"
		"         All commands can be separated with a ';'.\n"
		"\n"
		"         n;l green;s;say Dan Dare is back! -- do these 4 commands\n"
		"         There are 3 ways ';'s can be overruled.\n"
		"\n"
		"         \\say Hello ;) -- Lines starting with a '\\' aren't parsed by TinTin++.\n"
		"         say Hello \\;) -- The escape character can escape 1 letter.\n"
		"         #config verbatim on -- Everything is send as is except '#' commands.\n"
		"<128>\n"
		"         Connecting to a server\n"
		"<178>\n"
		"Command<278>: #session <178>{<278>session name<178>} {<278>server address<178>} {<278>port<178>}<278>\n"
		"\n"
		"         Example: #session someone tintin.sourceforge.net 4321\n"
		"\n"
		"         You can have more than one session, in which case you can switch\n"
		"         between sessions typing #<session name>.\n"
		"\n"
		"         You can get a list of all sessions by typing: #session. The current\n"
		"         active session is marked with (active). Snooped sessions with\n"
		"         (snooped). MCCP sessions (compression) with (mccp 2) and (mccp 3).\n"
		"\n"
		"<128>\n"
		"         Split\n"
		"<178>\n"
		"Command<278>: #split\n"
		"\n"
		"         The split command will create a separated input and output area.\n"
		"\n"
		"         Using the #prompt command you can capture the prompt and place it on\n"
		"         the split line. To get rid of the split interface you can use #unsplit\n"
		"         which will restore the terminal settings to default.\n"
		"\n"
		"<128>\n"
		"         Alias\n"
		"<178>\n"
		"Command<278>: #alias <178>{<278>name<178>} {<278>commands<178>}<278>\n"
		"\n"
		"         The syntax of the #alias command is almost like alias in csh.\n"
		"         Use this command to define aliases. The variables %0, %1.. %9 contain\n"
		"         the arguments to the aliased command as follows:\n"
		"         the %0 variable contains ALL the arguments.\n"
		"         the %1 variable contains the 1st argument\n"
		"         ....\n"
		"         the %9 variable contains the 9th argument\n"
		"\n"
		"<178>Example<278>: #alias greet say Greetings, most honorable %1\n"
		"\n"
		"         If there are no variables on the right-side of the alias definition,\n"
		"         any arguments following the aliases-command will be appended to the\n"
		"         command string.\n"
		"\n"
		"<178>Example<278>: #alias ff cast 'fireball' -- 'ff bob' equals: cast 'fireball' bob\n"
		"\n"
		"         If you want an alias to execute more commands, you must use braces.\n"
		"\n"
		"<178>Example<278>: #alias ws <178>{<278>wake;stand<178>}<278>\n"
		"\n"
		"         To delete an alias use the #unalias command.\n"
		"\n"
		"         WARNING! TinTin++ doesn't baby sit, and hence does not check for\n"
		"         recursive aliases! You can avoid recursion by escaping the entire\n"
		"         line.\n"
		"\n"
		"<178>Example<278>: #alias put \\put %1 in %2\n"
		"\n"
		"         Or by using the send command.\n"
		"\n"
		"<178>Example<278>: #send put %1 in %2\n"
		"\n"
		"\n"
		"<128>         Action\n"
		"\n"
		"<178>Command<278>: #action <178>{<278>action-text<178>} {<278>commands<178>}<278>\n"
		"\n"
		"         Use this command to define an action to take place when a particular\n"
		"         text appears on your screen. There are 99 variables you can use as\n"
		"         wildcards in the action-text.\n"
		"\n"
		"         These variables are %1, %2, %3 .... %9, %10, %11 ... %97, %98, %99.\n"
		"\n"
		"<178>Example<278>: #action <178>{<278>You are hungry<178>} {<278>get bread bag;eat bread<178>}<278>\n"
		"\n"
		"<178>Example<278>: #action <178>{<278>%1 has arrived.<178>}<278> shake %1 -- shake hands with people arriving.\n"
		"\n"
		"<178>Example<278>: #action <178>{<278>%1 tells you '%2'<178>}\n"
		"                   {<278>tell bob %1 told me '%2'<178>}<278> -- forward tells.\n"
		"\n"
		"<178>Example<278>: #action <178>{<278>tells you<178>}<278> #bell -- beep on tell.\n"
		"\n"
		"         You can have TinTin++ ignore actions if you type '#ignore actions on'.\n"
		"\n"
		"         You can see what commands TinTin++ executes when an action triggers\n"
		"         by typing '#debug actions on'.\n"
		"\n"
		"         You can remove actions with the #unaction command.\n"
		"\n"
		"<128>\n"
		"         Highlight\n"
		"\n"
		"<178>Command<278>: #highlight <178>{<278>text<178>} {<278>color<178>}<278>\n"
		"\n"
		"         This command works a bit like #action. The purpose of this command is\n"
		"         to substitute text from the server with color you provide. This command\n"
		"         is a simplified version of the #substitute command.\n"
		"\n"
		"<178>Example<278>: #high <178>{<278>Snowy<178>} {<278>light yellow<178>}<278>\n"
		"\n"
		"<178>Example<278>: #high <178>{<278>%*Snowy%*<178>} {<278>light yellow<178>}<278>\n"
		"\n"
		"         Use #unhigh to delete highlights.\n"
		"\n"
		"\n"
		"         Speedwalk\n"
		"\n"
		"         If you type a command consisting ONLY of letters and numbers n, e, s,\n"
		"         w, u, d - then this command can be interpreted as a serie of movement\n"
		"         commands.\n"
		"\n"
		"<178>Example<278>: ssw2n -- go south, south, west, north, north\n"
		"\n"
		"         If you have problems with typing some commands that actually ONLY\n"
		"         consists of these letters, then type them in CAPS. For example when\n"
		"         checking the NEWS or when asked to enter NEW as your name.\n"
		"\n"
		"         You must enable speedwalking with: #config speedwalk on/off.\n"
		"\n"
		"<128>\n"
		"         Ticker\n"
		"\n"
		"<178>Command<278>: #ticker <178>{<278>name<178>} {<278>commands<178>} {<278>seconds<178>}<278>\n"
		"\n"
		"         The name can be whatever you want it to be, and is only required for\n"
		"         the unticker command. The commands will be executed every x amount of\n"
		"         seconds, which is specified in the interval part.\n"
		"\n"
		"<178>Example<278>: #tick <178>{<278>tick<178>} {<278>#delay 50 #show 10 SECONDS TO TICK!;#show TICK!!!<178>} {<278>60<178>}<278>\n"
		"\n"
		"         This creates a ticker with the name <178>{<278>tick<178>}<278> which will print TICK!!!,\n"
		"         as well as print a warning when the next tick will occure.\n"
		"\n"
		"         You can remove tickers with #untick\n"
		"\n"
		"<128>\n"
		"         Command files\n"
		"<278>\n"
		"         When you order TinTin++ to read a command file, it parses all the text\n"
		"         in the file. You can use command files to keep aliases/actions in,\n"
		"         login to a server (name, password etc..) and basically all kinds of\n"
		"         commands.\n"
		"\n"
		"         You can make the command files with either a text editor (suggested),\n"
		"         or use the #write command to write out a file.\n"
		"\n"
		"         Commands for files:\n"
		"\n"
		"         #read filename -- read and execute the file.\n"
		"\n"
		"         #write filename -- write all actions/aliases/substitutes/etc known for\n"
		"         the current session to a file.\n"
		"\n"
		"<128>\n"
		"         Repeating Commands\n"
		"<278>\n"
		"         You can repeat a command, the syntax is: #number command\n"
		"\n"
		"<178>Example<278>: #5 cackle -- if you just killed bob the wizard.\n"
		"<178>Example<278>: #10 <178>{<278>buy bread;put bread bag<178>}<278> -- repeat these 2 commands 10 times.\n"
		"<178>Example<278>: #100 ooc w00t w00t!!!!! -- nochannel yourself.\n"
		"\n"
		"<128>\n"
		"         History\n"
		"<278>\n"
		"         TinTin++ has a limited subset of the csh history features.\n"
		"\n"
		"         ! -- repeat the last command\n"
		"         !cast -- repeat the last command starting with cast\n"
		"         ctrl-r -- enter the reverse history search mode.\n"
		"\n"
		"<128>\n"
		"         Map commands\n"
		"<278>\n"
		"         TinTin++ has a powerful highly configurable automapper. Whenever\n"
		"         you type n/ne/e/se/s/sw/w/nw/n/u/d tt++ tries to keep track of your\n"
		"         movement.\n"
		"\n"
		"         Commands for map:\n"
		"\n"
		"         #map create -- create a map.\n"
		"         #map goto 1 -- go to the first room in the map, created by default.\n"
		"         #map map -- display the map.\n"
		"         #map undo -- undo your last map alteration.\n"
		"         #map write <filename> -- save the map to file.\n"
		"         #map read <filename> -- load a map from file.\n"
		"\n"
		"         There are many other map options and it's beyond the scope of this\n"
		"         help section to explain everything there is to know, but I'll give\n"
		"         a set of commands that will get most people started.\n"
		"\n"
		"         #map create\n"
		"         #split 12 1\n"
		"         #map flag unicode on\n"
		"         #map flag vt on\n"
		"         #map goto 1\n"
		"\n"
		"         These commands will create a 12 row vt100 split section at the top of\n"
		"         your screen where a map drawn using unicode characters is displayed.\n"
		"\n"
		"<178>Example<278>: #action <178>{<278>There is no exit in that direction.<178>} {<278>#map undo<178>}<278>\n"
		"\n"
		"         The map will be automatically created as you move around.\n"
		"\n"
		"<128>\n"
		"         Help\n"
		"\n"
		"<178>Command<278>: #help <178>{<278>subject<178>}<278>\n"
		"\n"
		"         The help command is your friend and contains the same helpfiles\n"
		"         inside TinTin++ as are available on the website. If you type #help\n"
		"         without an argument you will see the various available help subjects\n"
		"         which try to explain the TinTin++ commands and features in greater\n"
		"         detail.\n"
		"\n"
		"<128>\n"
		"         Enjoy<278>\n",

		"characters colors coordinates editing escape_codes greeting keypad lists mapping mathematics screen_reader sessionname speedwalk statements suspend time"
	},
	{
		"INFO",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #info <178>{<278>listname<178>} {<278>LIST<178>|<278>ON<178>|<278>OFF<178>|<278>SAVE<178>}<278>\n"
		"\n"
		"         Without an argument info displays the settings of every tintin list.\n"
		"\n"
		"         By providing the name of a list and the LIST option it shows all\n"
		"         triggers/variables associated with that list. With the SAVE option\n"
		"         This data is written to the info variable.\n"
		"\n"
		"         #info cpu will show information about tintin's cpu usage.\n"
		"         #info mccp will show information about data compression.\n"
		"         #info stack will show the low level debugging stack.\n"
		"         #info session will show some session information.\n"
		"         #info system will show some system information.\n"
		"         #info unicode will show information on the provided character.\n",

		"class debug ignore kill message"
	},
	{
		"KEYPAD",
		TOKEN_TYPE_STRING,
		"<278>When TinTin++ starts up it sends \\e= to the terminal to enable the terminal's\n"
		"application keypad mode, which can be disabled using #show {\\e>}\n"
		"\n"
		"<178>      Configuration A           Configuration B           Configuration C<268>\n"
		" ╭─────┬─────┬─────┬─────╮ ╭─────┬─────┬─────┬─────╮ ╭─────┬─────┬─────┬─────╮\n"
		" │<178>num<268>  │<178>/<268>    │<178>*<268>    │<178>-<268>    │ │<178>num<268>  │<178>/<268>    │<178>*<268>    │<178>-<268>    │ │<178>Num<268>  │<178>nkp/<268> │<178>nkp*<268> │<178>nkp-<268> │\n"
		" ├─────┼─────┼─────┼─────┤ ├─────┼─────┼─────┼─────┤ ├─────┼─────┼─────┼─────┤\n"
		" │<178>7<268>    │<178>8<268>    │<178>9<268>    │<178>+<268>    │ │<178>Home<268> │<178>Up<268>   │<178>PgUp<268> │<178>+<268>    │ │<178>nkp7<268> │<178>nkp8<268> │<178>nkp9<268> │<178>nkp+<268> │\n"
		" ├─────┼─────┼─────┤     │ ├─────┼─────┼─────┤     │ ├─────┼─────┼─────┤     │\n"
		" │<178>4<268>    │<178>5<268>    │<178>6<268>    │     │ │<178>Left<268> │<178>Cntr<268> │<178>Right<268>│     │ │<178>nkp4<268> │<178>nkp5<268> │<178>nkp6<268> │     │\n"
		" ├─────┼─────┼─────┼─────┤ ├─────┼─────┼─────┼─────┤ ├─────┼─────┼─────┼─────┤\n"
		" │<178>1<268>    │<178>2<268>    │<178>3<268>    │<178>Enter<268>│ │<178>End<268>  │<178>Down<268> │<178>PgDn<268> │<178>Enter<268>│ │<178>nkp1<268> │<178>nkp2<268> │<178>nkp3<268> │<178>nkpEn<268>│\n"
		" ├─────┴─────┼─────┤     │ ├─────┴─────┼─────┤     │ ├─────┴─────┼─────┤     │\n"
		" │<178>0<268>          │<178>.<268>    │     │ │<178>Ins<268>        │<178>Del<268><268>  │     │ │<178>nkp0<268>       │<178>nkp.<268> │     │\n"
		" ╰───────────┴─────┴─────╯ ╰───────────┴─────┴─────╯ ╰───────────┴─────┴─────╯\n"
		"<278>\n"
		"With keypad mode disabled numlock on will give you configuration A, and numlock\n"
		"off will give you configuration B. With keypad mode enabled you'll get\n"
		"configuration C.\n"
		"\n"
		"<178>Terminals that support keypad mode"
		"\n"
		"<278>Linux Console, PuTTY, Eterm, aterm.\n"
		"\n"
		"<178>Terminals that do not support keypad mode\n"
		"\n"
		"<278>RXVT on Cygwin, Windows Console, Gnome Terminal, Konsole.\n"
		"\n"
		"<178>Peculiar Terminals\n"
		"\n"
		"<278>RXVT requires turning off numlock to enable configuration C.\n"
		"\n"
		"Xterm may require disabling Alt/NumLock Modifiers (num-lock) in the ctrl-left\n"
		"click menu. Or edit ~/.Xresources and add XTerm*VT100.numLock:false\n"
		"\n"
		"Mac OS X Terminal requires enabling 'strict vt100 keypad behavior' in\n"
		"Terminal -> Window Settings -> Emulation.\n",
		
		"colors coordinates escape mathematics pcre"
	},
	{
		"KILL",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #kill <178>{<278>list<178><178>} {<278>pattern<178>}<278>\n"
		"\n"
		"         Without an argument, the kill command clears all lists.  Useful if\n"
		"         you don't want to exit tintin to reload your command files.\n"
		"\n"
		"         With one argument a specific list can be cleared.\n"
		"\n"
		"         With two arguments the triggers in the chosen list that match the\n"
		"         given pattern will be removed.\n"
		"\n"
		"<178>Example<278>: #kill alias %*test*\n",

		"class debug ignore info message"
	},
	{
		"LINE",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #line <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"<128>\n"
		"         Line sub commands that alter the argument.\n"
		"<278>\n"
		"         <178>#line strip <argument>\n"
		"         <278>  Argument is executed with all color codes stripped.\n"
		"\n"
		"         <178>#line substitute <options> <argument>\n"
		"         <278>  Argument is executed using the provided substitutions, available\n"
		"         <278>  options are: arguments, colors, escapes, functions, secure, and\n"
		"         <278>  variables.\n"
		"<128>\n"
		"         Line sub commands that alter how the line is executed.\n"
		"<278>\n"
		"         <178>#line background <argument>\n"
		"         <278>  Prevent new session activation.\n"
		"\n"
		"         <178>#line capture <variable> <argument.\n"
		"         <278>  Argument is executed and output stored in <variable>.\n"
		"\n"
		"         <178>#line convert <argument>\n"
		"         <278>  Argument is executed with escaped meta characters.\n"
		"\n"
		"         <178>#line debug <argument>\n"
		"         <278>  Argument is executed in debug mode.\n"
		"\n"
		"         <178>#line gag\n"
		"         <278>  Gag the next line.\n"
		"\n"
		"         <178>#line ignore {argument}\n"
		"         <278>  Argument is executed without any triggers being checked.\n"
		"\n"
		"         <178>#line local {argument}\n"
		"         <278>  Argument is executed with all newly and indirectly\n"
		"         <278>  created variables being local.\n"
		"\n"
		"         <178>#line log <filename> [text]\n"
		"         <278>  Log the next line to file unless the [text] argument is\n"
		"         <278>  provided.\n"
		"\n"
		"         <178>#line logmode <option> <argument>\n"
		"         <278>  Argument is executed using the provided logmode, available\n"
		"         <278>  modes are: html, plain, and raw.\n"
		"\n"
		"         <178>#line msdp <argument>\n"
		"         <278>  Turn the argument into an msdp telnet sequence, starting at the\n"
		"         <278>  first opening brace. Will turn tintin tables into msdp tables,\n"
		"         <278>  with semicolons being used to create msdp arrays.\n"
		"\n"
		"         <178>#line multishot <number> <argument>\n"
		"         <278>  Argument is executed in multishot mode, all triggers created\n"
		"         <278>  will only fire the given number of times.\n"
		"\n"
		"         <178>#line oneshot <argument>\n"
		"         <278>  Argument is executed in oneshot mode, all triggers created will\n"
		"         <278>  only fire once.\n"
		"\n"
		"         <178>#line quiet <argument>\n"
		"         <278>  Argument is executed with suppression of most system messages.\n"
		"\n"
		"         <178>#line verbatim <argument>\n"
		"         <278>  Argument is executed verbatim, prohibiting variable and function\n"
		"         <278>  substitutions.\n"
		"\n"
		"         <178>#line verbose <argument>\n"
		"         <278>  Argument is executed with most system messages enabled.\n"
		"\n"
		"         When using #line log and logging in html format use \\c< \\c> \\c& \\c\" to\n"
		"         log a literal < > & and \".\n",

		"class config"
	},
	{
		"LIST",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #list <178>{<278>variable<178>} {<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         #list {var} {add} {item}               Add {item} to the list\n"
		"         #list {var} {clear}                    Empty the given list\n"
		"         #list {var} {collapse}                 Turn list into a variable\n"
		"         #list {var} {create} {item}            Create a list using {items}\n"
		"         #list {var} {delete} {index} {number}  Delete the item at {index},\n"
		"                                                the {number} is optional.\n"
		"         #list {var} {explode}                  Turn list into a character list\n"
		"         #list {var} {indexate}                 Index a list table for sorting\n"
		"         #list {var} {insert} {index} {string}  Insert {string} at given index\n"
		"         #list {var} {find} {string} {variable} Return the found index\n"
		"         #list {var} {get} {index} {variable}   Copy an item to {variable}\n"
		"         #list {var} {order} {string}           Insert item in numerical order\n"
		"         #list {var} {shuffle}                  Shuffle the list\n"
		"         #list {var} {set} {index} {string}     Change the item at {index}\n"
		"         #list {var} {simplify} {string}        Turn list into a simple list\n"
		"         #list {var} {size} {variable}          Copy list size to {variable}\n"
		"         #list {var} {sort} {string}            Insert item in alphabetic order\n"
		"         #list {var} {tokenize} {string}        Create a character list\n"
		"\n"
		"         The index should be between 1 and the list's length. You can also give\n"
		"         a negative value, in which case -1 equals the last item in the list, -2\n"
		"         the second last, etc.\n"
		"\n"
		"         When inserting an item a positive index will prepend the item at the\n"
		"         given index, while a negative index will append the item.\n"
		"\n"
		"         The add and create options allow using multiple items, as well\n"
		"         as semicolon separated items.\n"
		"\n"
		"         A length of 0 is returned for an empty or non existant list.\n"
		"\n"
		"         You can directly access elements in a list variable using $var[1],\n"
		"         $var[2], $var[-1], etc.\n",

		"break continue foreach loop parse repeat return while"
	},

	{
		"LISTS",
		TOKEN_TYPE_STRING,
		"<278>         There are several different types of lists in tintin which behave in a\n"
		"         fairly universal manner. To properly explain lists it's easiest to\n"
		"         explain the most basic variable type first before discussing more\n"
		"         complex types.\n"
		"\n"
		"       - Basic variable: The standard key = value variable.\n"
		"\n"
		"       - Simple list: A string that contains semicolon delimited fields.\n"
		"         {a;b;c}. Can be saved as a variable.\n"
		"\n"
		"       - Brace list: A string in which fields are delimited with braces.\n"
		"         {a}{b}{c}. Brace lists cannot be stored as a variable because tables\n"
		"         use braces as well, they must be stored as a simple list instead.\n"
		"\n"
		"       - Table: Think of this as variables nested within another variable. Or\n"
		"          as variables contained within another variable.\n"
		"\n"
		"       - List: A table that uses integers for its indexes. Also known as an\n"
		"         array. The #list command is a utility command for using tables as\n"
		"         arrays.\n"
		"<128>\n"
		"         Simple Variables\n"
		"<278>\n"
		"<178>Example:<278>\n"
		"         #variable {simple} {Hello World!}\n"
		"         #show $simple\n"
		"\n"
		"         To see if the 'simple' variable exists you can use &simple which will\n"
		"         display 0 if the variable does not exist, or the variable's index if\n"
		"         it exists.\n"
		"\n"
		"         If you have multiple variables they are sorted alphabetically and\n"
		"         numerically. While it's not all that relevant for simple variables,\n"
		"         the first variable has index 1, the second variable index 2, and so\n"
		"         on.\n"
		"\n"
		"         Variable names need to start with a letter and only exist of letters,\n"
		"         numbers, and underscores. If you need to use a non standard variable\n"
		"         name this is possible using braces.\n"
		"\n"
		"<178>Example: <278>#variable {:)} {Happy Happy!};#show ${:)}\n"
		"\n"
		"         Variables can be accessed using their index. While primarily useful\n"
		"         for tables it is possible to do this for simple variables. Use +1 for\n"
		"         the first variable, +2 for the second variable, etc. Use -1 for the\n"
		"         last variable, -2 for the second last variable, etc.\n"
		"\n"
		"<178>Example:<278> #show The first variable is: ${+1}\n"
		"<128>\n"
		"         Removing Variables\n"
		"<278>\n"
		"         To remove a variable, use #unvariable or #unvar (every command can be\n"
		"         abbreviated). It's possible to remove multiple variables at once\n"
		"         using #unvar {var 1} {var 2} {etc}\n"
		"\n"
		"         Variables are unique to each session, so if you have multiple\n"
		"         sessions, removing a variable from one session won't remove it from\n"
		"         other sessions.\n"
		"\n"
		"         If you remove a table variable, all variables contained within that\n"
		"         table variable are removed as well.\n"
		"<128>\n"
		"         Simple Lists\n"
		"<278>\n"
		"         A simple list is a string that contains semicolon delimited fields.\n"
		"         Commands can be entered as simple lists, for example:\n"
		"         #show {a};#show {b} will execute a single line as two commands.\n"
		"\n"
		"         Several commands take a simple list as their input, these are:\n"
		"         #foreach, #line substitute, #path load, #list create, and #highlight.\n"
		"<128>\n"
		"         Brace Lists\n"
		"<278>\n"
		"         A brace list is a string in which fields are delimited with braces.\n"
		"         Most commands take a brace list for their arguments, for example:\n"
		"         #session {x} {mud.com} {1234} {mud.tin}. The session command takes\n"
		"         4 arguments, the 4th argument (command file) is optional.\n"
		"\n"
		"         Commands that take a simple list as their input will also accept a\n"
		"         brace list, keep in mind you'll have to embed the brace list in an\n"
		"         extra set of braces, for example: #path load {{n}{s}{w}{w}}, which is\n"
		"         identical to: #path load {n;s;w;w}.\n"
		"\n"
		"         Brace lists cannot be stored as variables because TinTin++ will\n"
		"         confuse them with tables. You can convert a brace list to a table\n"
		"         variable using: #list {bracelist} {create} {{a}{b}{c}} this will look\n"
		"         internally as: {{1}{a}{2}{b}{3}{c}}. You can then convert this table\n"
		"         to a simple list using: #list {bracelist} {simplify} {simplelist}\n"
		"         which will store {a;b;c} in the $simplelist variable.\n"
		"\n"
		"         Braces cannot easily be escaped in TinTin++. Using \\{ or \\} will not\n"
		"         work. The reason for this is due to several factors, but primarily\n"
		"         backward compatibility. To escape braces you must define them using\n"
		"         hexadecimal notation using \\x7B and \\x7D. See #help escape for a list\n"
		"         of escape options, and the help file will also remind you of how to\n"
		"         escape braces.\n"
		"<128>\n"
		"         Tables\n"
		"<278>\n"
		"         Tables are key/value pairs stored within a variable. Tables are also\n"
		"         known as associative arrays, dictionaries, maps, nested variables,\n"
		"         structures, and probably a couple of other names. There are several\n"
		"         ways to create and access tables.\n"
		"\n"
		"<178>Example:<278> #variable {friendlist} {{bob}{bob@mail.com} {bubba}{sunset@gmail.com}}\n"
		"\n"
		"         This will create a friendlist with two entries, the key is the name of\n"
		"         the friend, the value is the email address of the friend. You can see\n"
		"         the email address of bob using: #show {$friendlist[bob]}. You can\n"
		"         also define this table as following:\n"
		"\n"
		"<178>Example:<278>\n"
		"         #variable {friendlist[bob]} {bob@mail.com}\n"
		"         #variable {friendlist[bubba]} {sunset@gmail.com}\n"
		"\n"
		"         This would create the exact same table as the single line declaration\n"
		"         used previously. To see the first key in the table use:\n"
		"         *friendlist[+1], to see the first value in the table use:\n"
		"         $friendlist[+1]. To see the size of the table use &friendlist[]. To\n"
		"         print a bracelist of all friends use *friendlist[%*], to print a\n"
		"         bracelist of all friends whose name starts with the letter 'a' you\n"
		"         would use: *friendlist[a%*]. Similarly to see the number of friends\n"
		"         you have whose name ends with the letter 'b' you would use:\n"
		"         &friendlist[%*b].\n"
		"\n"
		"         See #help regexp for a brief overview of regular expression options.\n"
		"         While TinTin++ supports PCRE (perl-compatible regular expressions), it\n"
		"         embeds them within its own regular expression syntax that is simpler\n"
		"         and less invasive, while still allowing the full power of PCRE for\n"
		"         those who need it.\n"
		"\n"
		"<178>Example:<278> #unvariable {friendlist[bubba]}\n"
		"\n"
		"         This would remove {bubba} from the friendlist. To remove the entire\n"
		"         friendlist you would use: #unvariable {friendlist}.\n"
		"\n"
		"<178>Example:<278> #variable {friendlist} {{bob} {{email}{bob@ma.il} {phone}{123456789}}}\n"
		"\n"
		"         There is no limit to the number of nests, simply add more braces. To\n"
		"         see Bob's email in this example you would use:\n"
		"         #show {$friendlist[bob][email]}.\n"
		"<278>\n"
		"         Lists\n"
		"\n"
		"         Tables are sorted alphabetically with the exception of numbers which\n"
		"         are sorted numerically. If you want to determine the sorting order\n"
		"         yourself you can use use the #list command which helps you to use\n"
		"         tables as arrays.\n"
		"\n"
		"<178>Example:<278> #action {%1 chats %2} {#list chats add {%0}}\n"
		"\n"
		"         Each time a chat is received it's added to the end of the 'chats' list\n"
		"         variable. If you type #variable chats this might look like:\n"
		"\n"
		"         <138>#<168>VARIABLE <258>{<178>chats<258>}\n"
		"         {\n"
		"                 {<178>1<258>} {<178>Bubba chats Hi<258>}\n"
		"                 {<178>2<258>} {<178>Bob chats Hi bub<258>}\n"
		"                 {<178>3<258>} {<178>Bubba chats Bye<258>}\n"
		"                 {<178>4<258>} {<178>Bob chats bub bye<258>}\n"
		"         }\n"
		"<128>\n"
		"         Parsing\n"
		"<278>\n"
		"         There are various ways to parse lists and tables, using either #loop,\n"
		"         #foreach, #while, or #<number>.\n"
		"\n"
		"         #loop takes two numeric arguments, incrementing or decrementing the\n"
		"         first number until it matches the second number. The value of the loop\n"
		"         counter is stored in the provided variable.\n"
		"\n"
		"         #foreach takes either a simple list or a brace list as its first\n"
		"         argument. Foreach will go through each item in the list and store the\n"
		"         value in the provided variable.\n"
		"\n"
		"         #while will perform an if check on the first argument, if the result\n"
		"         is true it will execute the commands in the second argument. Then it\n"
		"         performs an if check on the first argument again. It will continue to\n"
		"         repeat until the if check returns 0 or the loop is interrupted with a\n"
		"         control flow command. It takes special care to avoid infinite loops.\n"
		"\n"
		"         #<number> will execute the provided argument 'number' times. For\n"
		"         example: #4 {#show beep! \\a}\n"
		"\n"
		"         Here are some examples.\n"
		"\n"
		"<178>Example:<278> #list friends create {bob;bubba;zorro}\n"
		"\n"
		"         Internally this looks like {{1}{bob}{2}{bubba}{3}{zorro}} and the\n"
		"         list can be parsed in various ways.\n"
		"\n"
		"<178>Example:<278> #foreach {$friends[%*]} {name} {#show $name}\n"
		"\n"
		"<178>Example:<278> #foreach {*friends[%*]} {i} {#show $friends[$i]}\n"
		"\n"
		"<178>Example:<278> #loop {1} {&friends[]} {i} {#show $friends[+$i]}\n"
		"\n"
		"<178>Example:<278> #math i 1;#while {&friends[+$i]} {#show $friends[+$i];\n"
		"         #math i $i + 1}\n"
		"\n"
		"<178>Example:<278> #math i 1;#&friends[] {#show $friends[+$i];#math i $i + 1}\n"
		"\n"
		"         Each of the five examples above performs the same task; printing the\n"
		"         three names in the friends list.\n"
		"\n"
		"         If you want to get a better look at what goes on behind the scenes\n"
		"         while executing scripts you can use '#debug all on'. To stop seeing\n"
		"         debug information use '#debug all off'.\n"
		"<128>\n"
		"         Optimization\n"
		"<278>\n"
		"         TinTin++ tables are exceptionally fast while they remain under 100\n"
		"         items. Once a table grows beyond 10000 items there can be performance\n"
		"         issues when inserting and removing items in the beginning or middle of\n"
		"         the table.\n"
		"\n"
		"         The plan is to eventually implement an indexable and flexible data\n"
		"         structure for large tables.\n"
		"\n"
		"         If you load a large table from file it's important to make sure it's\n"
		"         sorted, when using #write to save a table it's automatically sorted.\n"
		"\n"
		"         If you notice performance issues on large tables it's relatively easy\n"
		"         to create a hash table.\n"
		"\n"
		"<178>Example:<278>\n"
		"\n"
		"         #alias {sethash}\n"
		"         {\n"
		"         	#format hash %H %1;\n"
		"         	#math hash1 $hash % 100;\n"
		"         	#math hash2 $hash / 100 % 100;\n"
		"         	#var hashtable[$hash1][$hash2][%1] %2\n"
		"         }\n"
		"\n"
		"         #function {gethash}\n"
		"         {\n"
		"         	#format hash %H %1;\n"
		"         	#math hash1 $hash % 100;\n"
		"         	#math hash2 $hash / 100 % 100;\n"
		"         	#return $hashtable[$hash1][$hash2][%1]\n"
		"         }\n"
		"\n"
		"         #alias {test}\n"
		"         {\n"
		"         	sethash bli hey;\n"
		"         	sethash bla hi;\n"
		"         	sethash blo hello;\n"
		"         	#show The value of bla is: @gethash{bla}\n"
		"         }\n"
		"\n"
		"         The above script will rapidly store and retrieve over 1 million items.\n"
		"         Looping through a hash table is relatively easy as well.\n"
		"\n"
		"<178>Example:<278>\n"
		"\n"
		"         #alias {showhash}\n"
		"         {\n"
		"         	#foreach {*hashtable[%*]} {hash1}\n"
		"         	{\n"
		"         		#foreach {*hashtable[$hash1][%*]} {hash2}\n"
		"         		{\n"
		"         			#echo {%-20s = %s}\n"
		"                                        {hashtable[$hash1][$hash2]}\n"
		"                                        {$hashtable[$hash1][$hash2]}\n"
		"         		}\n"
		"         	}\n"
		"        }\n",
		
		"break continue foreach loop parse repeat return while"
	},
	
	{
		"LOCAL",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #local <178>{<278>variable name<178>} {<278>text to fill variable<178>}<278>\n"
		"\n"
		"         The local command sets a local variable. Unlike a regular variable\n"
		"         a local variable will only stay in memory for the duration of the\n"
		"         event that created it. They are accessed in the same way as a\n"
		"         regular variable.\n"
		"\n"
		"         Commands that store information to a variable will use a local variable\n"
		"         if it exists.\n"
		"\n"
		"         Avoid setting the result variable as local in a function.\n"
		"\n"
		"<178>Example<278>: #alias {swap} {#local x %0;#replace x {e} {u};#show $x}\n"
		"\n"
		"<178>Comment<278>: You can remove a local variable with the #unlocal command.\n",

		"format function math replace script variable"
	},

	{
		"LOG",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #log <178>{<278>append<178>|<278>overwrite<178>|<278>off<178>} {<278>[filename]<178>}<278>\n"
		"\n"
		"         Logs session output to a file, you can set the data type to either\n"
		"         plain, raw, or html with the config command.\n",
		
		"read scan textin write"
	},

	{
		"LOOP",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #loop <178>{<278><start><178>} {<278><finish><178>} {<278><variable><178>} {<278>commands<178>}<278>\n"
		"\n"
		"         Like a for statement, loop will loop from start to finish incrementing\n"
		"         or decrementing by 1 each time through.  The value of the loop counter\n"
		"         is stored in the provided variable, which you can use in the commands.\n"
		"\n"
		"<178>Example<278>: #loop 1 3 loop {get all $loop.corpse}\n"
		"         This equals 'get all 1.corpse;get all 2.corpse;get all 3.corpse'.\n"
		"\n"
		"<178>Example<278>: #loop 3 1 cnt {drop $cnt\\.key}\n"
		"         This equals 'drop 3.key;drop 2.key;drop 1.key'.\n",

		"break continue foreach list parse repeat return while"
	},
	{
		"MACRO",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #macro <178>{<278>key sequence<178>} {<278>commands<178>}<278>\n"
		"\n"
		"         Macros allow you to make tintin respond to function keys.\n"
		"\n"
		"         The key sequence send to the terminal when pressing a function key\n"
		"         differs for every OS and terminal. To find out what sequence is send\n"
		"         you can enable the CONVERT META config option.\n"
		"\n"
		"         Another option is pressing ctrl-v, which will enable CONVERT META for\n"
		"         the next key pressed.\n"
		"\n"
		"         If you only want a key sequence to trigger at the start of an input\n"
		"         line prefix the key sequence with ^.\n"
		"\n"
		"<178>Example<278>: #macro {(press ctrl-v)(press F1)} {#show \\e[2J;#buffer lock}\n"
		"         Clear the screen and lock the window when you press F1, useful when the\n"
		"         boss is near.\n"
		"\n"
		"<178>Example<278>: #macro {\\eOM} {#cursor enter}\n"
		"         Makes the keypad's enter key work as an enter in keypad mode.\n"
		"\n"
		"<178>Example<278>: #macro {^nn} {n}\n"
		"         Makes pressing n twice on an empty line execute north.\n"
		"\n"
		"<178>Comment<278>: Not all terminals properly initialize the keypad key sequences.\n"
		"         If this is the case you can still use the keypad, but instead of the\n"
		"         arrow keys use ctrl b, f, p, and n.\n"
		"\n"
		"<178>Comment<278>: You can remove a macro with the #unmacro command.\n",

		"alias cursor history keypad speedwalk tab"
	},
	{
		"MAP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #map\n"
		"\n"
		"         The map command is the backbone of the auto mapping feature.\n"
		"\n"
		"         <178>#map at <exit|vnum> <command>\n"
		"         <278>  Execute the command at the given exit or vnum.\n"
		"\n"
		"         <178>#map center <x> <y> <z>\n"
		"         <278>  Sets displaying center of the map viewer, default is 0 0 0.\n"
		"\n"
		"         <178>#map color <field> [value]\n"
		"         <278>  Sets the map color for the given color field.\n"
		"\n"
		"         <178>#map create <size>\n"
		"         <278>  Creates a new map and room 1. The default size is 50000 rooms.\n"
		"\n"
		"         <178>#map destroy {area|world} <name>\n"
		"         <278>  Deletes the map or given area.\n"
		"\n"
		"         <178>#map delete <exit|vnum>\n"
		"         <278>  Deletes the room for the given exit or vnum.\n"
		"\n"
		"         <178>#map dig <exit|vnum> [new|<vnum>]\n"
		"         <278>  Creates an exit for the given exit name. If no valid exit name\n"
		"         <278>  is given or no existing room is found a new room is created.\n"
		"         <278>  Useful for portal links and other alternative forms of\n"
		"         <278>  transportation. If the 'new' argument is provided all existing\n"
		"         <278>  rooms are ignored and a new room is created. If a room vnum is\n"
		"         <278>  given as the second argument an exit will be created leading\n"
		"         <278>  to the given room vnum. If the room vnum doesn't exist a new\n"
		"         <278>  room is created.\n"
		"\n"
		"         <178>#map entrance <exit> [option] [arg]\n"
		"         <278>  Set the entrance data for the given exit. You must specify a\n"
		"         <278>  valid two-way exit for this to work.\n"
		"\n"
		"         <178>#map exit <exit> <option> <arg>\n"
		"         <278>  Set the exit data. Useful with a closed door where you can\n"
		"         <278>  set the exit command: '#map exit e command {open east;e}'.\n"
		"         <278>  Use #map exit <exit> for a list of available options.\n"
		"         <278>  Use #map exit <eixt> save to save the exit data.\n"
		"\n"
		"         <178>#map exitflag <exit> <AVOID|BLOCK|HIDE|INVIS> [on|off]\n"
		"         <278>  Set exit flags. See #map roomflag for more info.\n"
		"\n"
		"         <178>#map explore <exit>\n"
		"         <278>  Explores the given exit until a dead end or an\n"
		"         <278>  intersection is found. The route is stored in #path and can\n"
		"         <278>  subsequently be used with #walk. Useful for long roads.\n"
		"\n"
		"         <178>#map find <name> <exits> <desc> <area> <note> <terrain> <flag>\n"
		"         <278>  searches for the given room name. If found the shortest path\n"
		"         <278>  from your current location to the destination is calculated.\n"
		"         <278>  The route is stored in #path and can subsequently be used with\n"
		"         <278>  the various #path commands. If <exits> is provided all exits\n"
		"         <278>  must be matched, if <roomdesc>, <roomarea> or <roomnote> or\n"
		"         <278>  <roomterrain> or <roomflag> is provided these are matched as\n"
		"         <278>  well against the room to be found.\n"
		"         <278>  These options are also available to the at, delete, goto\n"
		"         <278>  link, list and run commands.\n"
		"\n"
		"         <178>#map flag asciigraphics\n"
		"         <278>  Takes up more space but draws a more detailed\n"
		"         <278>  map that displays the ne se sw nw exits and room symbols.\n"
		"\n"
		"         <178>#map flag asciivnums\n"
		"         <278>  Display room vnums if asciigraphics is enabled.\n"
		"\n"
		"         <178>#map flag nofollow\n"
		"         <278>  When you enter movement commands the map will no longer\n"
		"         <278>  automatically follow along. Useful for MSDP and GMCP\n"
		"         <278>  automapping scripts.\n"
		"\n"
		"         <178>#map flag static\n"
		"         <278>  Will make the map static so new rooms are no longer\n"
		"         <278>  created when walking into an unmapped direction. Useful when\n"
		"         <278>  you're done mapping and regularly bump into walls accidentally\n"
		"         <278>  creating a new room. #map dig etc will still work.\n"
		"\n"
		"         <178>#map flag vtgraphics\n"
		"         <278>  Enables vt line drawing on some terminals\n"
		"\n"
		"         <178>#map flag vtmap\n"
		"         <278>  Will enable the vtmap which is shown in the top split\n"
		"         <278>  screen if you have one. You can create a 16 rows high top\n"
		"         <278>  screen by using '#split 16 1'.\n"
		"\n"
		"         <178>#map get <option> <variable> [vnum]\n"
		"         <278>  Store a map value into a variable, if no vnum is given the\n"
		"         <278>  current room is used. Use 'all' as the option to store all\n"
		"         <278>  values as a table.\n"
		"\n"
		"         <178>#map get roomexits <variable>\n"
		"         <278>  Store all room exits into variable.\n"
		"\n"
		"         <178>#map global <room vnum>\n"
		"         <278>  Set the vnum of a room that contains global\n"
		"         <278>  exits, for example an exit named 'recall' that leads to the\n"
		"         <278>  recall location. The room can contain multiple exits, in case\n"
		"         <278>  there are multiple commands that are similar to recall.\n"
		"\n"
		"         <178>#map goto <room vnum> [dig]\n"
		"         <278>  Takes you to the given room vnum, with the\n"
		"         <278>  dig argument a new room will be created if none exists.\n"
		"\n"
		"         <178>#map goto <name> <exits> <desc> <area> <note> <terrain>\n"
		"         <278>  Takes you to the given room name, if you provide exits those\n"
		"         <278>  must match.\n"
		"\n"
		"         <178>#map info\n"
		"         <278>  Gives information about the map and room you are in.\n"
		"\n"
		"         <178>#map insert <direction> [roomflag]\n"
		"         <278>  Insert a room in the given direction. Most useful for inserting\n"
		"         <278>  void rooms.\n"
		"\n"
		"         <178>#map jump <x> <y> <z>\n"
		"         <278>  Jump to the given coordinate, which is relative\n"
		"         <278>  to your current room.\n"
		"\n"
		"         <178>#map landmark <name> <vnum> [description] [size]\n"
		"         <278>  Creates an alias to target the provided room vnum. The\n"
		"         <278>  description is optional and should be brief. The size\n"
		"         <278>  determines from how many rooms away the landmark can be\n"
		"         <278>  seen.\n"
		"\n"
		"         <178>#map leave\n"
		"         <278>  Makes you leave the map. Useful when entering a maze. You\n"
		"         <278>  can return to your last known room using #map return.\n"
		"\n"
		"         <178>#map legend <legend> [symbols|reset]\n"
		"         <178>#map legend <legend> <index> [symbol]\n"
		"         <278>  There are several legends and sub-legends available for\n"
		"         <278>  drawing maps to suit personal preference and character sets.\n"
		"         <278>  Use #map legend all to see the legend as currently defined.\n"
		"         <278>  Use #map legend <legend> <reset> to set the default legend.\n"
		"         <278>  Use #map legend <legend> <character list> to create a custom\n"
		"         <278>  legend. Custom legends are automatically saved and loaded by\n"
		"         <278>  using #map read and #map write.\n"
		"\n"
		"         <178>#map link <direction> <room name> [both]\n"
		"         <278>  Links two rooms. If the both\n"
		"         <278>  argument and a valid direction is given the link is two ways.\n"
		"\n"
		"         <178>#map list <name> <exits> <desc> <area> <note> <terrain>\n"
		"         <278>  Lists all matching rooms and their distance. The following\n"
		"         <278>  search keywords are supported.\n"
		"\n"
		"         <278>  {roomarea}    <arg> will list rooms with matching area name.\n"
		"         <278>  {roomdesc}    <arg> will list rooms with matching room desc.\n"
		"         <278>  {roomexits}   <arg> will list rooms with identical room exits.\n"
		"         <278>  {roomflag}    <arg> will list rooms with matching room flags.\n"
		"         <278>  {roomid}      <arg> will list rooms with identical id name.\n"
		"         <278>  {roomname}    <arg> will list rooms with matching room name.\n"
		"         <278>  {roomnote}    <arg> will list rooms with matching room note.\n"
		"         <278>  {roomterrain} <arg> will list rooms with matching room terrain.\n"
		"         <278>  {variable}    <arg> will save the output to given variable.\n"
		"\n"
		"         <178>#map map <rows> <cols> <append|overwrite|list|variable> <name>\n"
		"         <278>  Display a drawing of the map of the given height and width.\n"
		"         <278>  All arguments are optional. If {rows} or {cols} are set to {}\n"
		"         <278>  or {0} they will use the scrolling window size as the default.\n"
		"         <278>  If {rows} or {cols} are a negative number this number is\n"
		"         <278>  subtracted from the scrolling window size.\n"
		"\n"
		"         <178>#map map <rows> <cols> draw <square>\n"
		"         <278>  Display a drawing of the map of the given height and width.\n"
		"         <278>  The square argument exists of 4 numbers formulating the top\n"
		"         <278>  left corner and bottom right corner of a square.\n"
		"\n"
		"         <278>  If you use {append|overwrite} the map is written to the specified\n"
		"         <278>  file name which must be given as the 4th argument.\n"
		"         <278>  If you use {list|variable} the map is saved to the specified\n"
		"         <278>  variable name.\n"
		"\n"
		"         <178>#map move <direction>\n"
		"         <278>  This does the same as an actual movement\n"
		"         <278>  command, updating your location on the map and creating new\n"
		"         <278>  rooms. Useful when you are following someone and want the map\n"
		"         <278>  to follow. You will need to create actions using '#map move',\n"
		"         <278>  for this to work.\n"
		"\n"
		"         <178>#map offset <row> <col> <row> <col>\n"
		"         <278>  Define the offset of the vtmap as a square. Without an argument\n"
		"         <278>  it defaults to the entire top split region.\n"
		"\n"
		"         <178>#map read <filename>\n"
		"         <278>  Will load the given map file.\n"
		"\n"
		"         <178>#map resize <size>\n"
		"         <278>  Resize the map, setting the maximum number of rooms.\n"
		"\n"
		"         <178>#map return\n"
		"         <278>  Returns you to your last known room after leaving the map\n"
		"         <278>  or loading a map.\n"
		"\n"
		"         <178>#map roomflag <flags> <get|on|off>\n"
		"         <278>\n"
		"         <178>#map roomflag avoid\n"
		"         <278>  When set, '#map find' will avoid a route leading\n"
		"         <278>  through that room. Useful for locked doors, etc.\n"
		"         <178>#map roomflag block\n"
		"         <278>  When set the automapper will prevent movement into or through\n"
		"         <278>  the room. Useful for death traps.\n"
		"         <178>#map roomflag hide\n"
		"         <278>  When set, '#map' will not display the map beyond\n"
		"         <278>  this room. When mapping overlapping areas or areas that aren't\n"
		"         <278>  build consistently you need this flag as well to stop\n"
		"         <278>  auto-linking, unless you use void rooms.\n"
		"         <178>#map roomflag invis\n"
		"         <278>  When set the room will be colored with the INVIS color.\n"
		"         <178>#map roomflag leave\n"
		"         <278>  When entering a room with this flag, you will\n"
		"         <278>  automatically leave the map. Useful when set at the entrance\n"
		"         <278>  of an unmappable maze.\n"
		"         <178>#map roomflag noglobal\n"
		"         <278>  This marks a room as not allowing global\n"
		"         <278>  transportation, like norecall rooms that block recall.\n"
		"         <178>#map roomflag void\n"
		"         <278>  When set the room becomes a spacing room that can\n"
		"         <278>  be used to connect otherwise overlapping areas. A void room\n"
		"         <278>  should only have two exits. When entering a void room you are\n"
		"         <278>  moved to the connecting room until you enter a non void room.\n"
		"         <178>#map roomflag static\n"
		"         <278>  When set the room will no longer be autolinked\n"
		"         <278>  when walking around. Useful for mapping mazes.\n"
		"\n"
		"         <178>#map run <room name> [delay]\n"
		"         <278>  Calculates the shortest path to the destination and walks you\n"
		"         <278>  there. The delay is optional and requires using braces. Besides\n"
		"         <278>  the room name a list of exits can be provided for more precise\n"
		"         <278>  matching.\n"
		"\n"
		"         <178>#map set <option> <value> [vnum]\n"
		"         <278>  Set a map value for your current room, or given room if a room\n"
		"         <278>  vnum is provided.\n"
		"\n"
		"         <178>#map sync <filename>\n"
		"         <278>  Similar to #map read except the current map won't be unloaded\n"
		"         <278>  or overwritten.\n"
		"\n"
		"         <178>#map terrain <name> <symbol> [flag]\n"
		"         <278>  Set the terrain symbol and flag.\n"
		"\n"
		"         <178>#map terrain <name> <symbol> [DENSE|SPARSE|SCANT]\n"
		"         <278>  Determine symbol density, omit for the default.\n"
		"\n"
		"         <178>#map terrain <name> <symbol> [NARROW|WIDE|VAST]\n"
		"         <278>  Determine symbol spread range, omit for the default.\n"
		"\n"
		"         <178>#map terrain <name> <symbol> [FADEIN|FADEOUT]\n"
		"         <278>  Determine symbol spread density, omit for the default.\n"
		"\n"
		"         <178>#map terrain <name> <symbol> [DOUBLE]\n"
		"         <278>  You're using two characters for the symbol.\n"
		"\n"
		"         <178>#map travel <direction> <delay>\n"
		"         <278>  Follows the direction until a dead end or an intersection is\n"
		"         <278>  found. Use braces around the direction if you use the delay,\n"
		"         <278>  which will add the given delay between movements.\n"
		"         <278>  Use #path stop to stop a delayed run.\n"
		"\n"
		"         <178>#map undo\n"
		"         <278>  Will undo your last move. If this created a room or a link\n"
		"         <278>  they will be deleted, otherwise you'll simply move back a\n"
		"         <278>  room. Useful if you walked into a non existant direction.\n"
		"\n"
		"         <178>#map uninsert <direction>\n"
		"         <278>  Exact opposite of the insert command.\n"
		"\n"
		"         <178>#map unlandmark <name>\n"
		"         <278>  Removes a landmark.\n"
		"\n"
		"         <178>#map unlink <direction> [both]\n"
		"         <278>  Will remove the exit, this isn't two way so you can have the\n"
		"         <278>  properly display no exit rooms and mazes.\n"
		"         <278>  If you use the both argument the exit is removed two-ways.\n"
		"\n"
		"         <178>#map unterrain <name>\n"
		"         <278>  Removes a terrain.\n"
		"\n"
		"         <178>#map update [now]\n"
		"         <278>  Sets the vtmap to update within the next 0.1 seconds, or\n"
		"         <278>  instantly with the now argument.\n"
		"\n"
		"         <178>#map vnum <low> [high]\n"
		"         <278>  Change the room vnum to the given number, if a range is\n"
		"         <278>  provided the first available room in that range is selected.\n"
		"\n"
		"         <178>#map write <filename> [force]\n"
		"         <278>  Will save the map, if you want to save a map to a .tin file\n"
		"         <278>  you must provide the {force} argument.\n",

		"path pathdir"
	},

	{
		"MAPPING",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"         TinTin++ has a powerful automapper that uses a room system similar to\n"
		"         Diku MUDs which means that odd map layouts and weird exit\n"
		"         configurations aren't a problem. The mapper provides tools to improve\n"
		"         the visual map display. For basic path tracking see #help PATH.\n"
		"\n"
		"<178>         #map create [size]\n"
		"<278>\n"
		"         This command creates the initial map. The size is 50,000 by default\n"
		"         and can be changed at any time with the #map resize command. If you\n"
		"         play a MUD that uses MSDP or GMCP to provide room numbers you'll have\n"
		"         to increase it to the highest reported room number. Increasing the\n"
		"         size of the map doesn't decrease performance.\n"
		"\n"
		"<178>         #map goto <location>\n"
		"<278>\n"
		"         When you create the map you are not automatically inside the map. By\n"
		"         default room number (vnum) 1 is created, so you can go to it using\n"
		"         #map goto 1. Once you are inside the map new rooms are automatically\n"
		"         created as you move around. Movement commands are defined with the\n"
		"         pathdir command. By default n, ne, e, se, s, sw, w, nw, u, d are\n"
		"         defined.\n"
		"<178>\n"
		"         #map map <rows> <cols> <append|overwrite|list|variable> <name>\n"
		"<278>\n"
		"         To see the map you can use #map map. It's annoying to have to\n"
		"         constantly type #map map however. Instead it's possible to use #split\n"
		"         to display a vt100 map. To do so execute:\n"
		"         <178>#split 16 1\n"
		"         #map flag vtmap on<278>\n"
		"         The first command sets the top split lines to 16 and the bottom split\n"
		"         line to 1. If you want a smaller or larger map display you can use a\n"
		"         different value than 16.\n"
		"\n"
		"         If you don't need to display diagonal exits and prefer a more compact\n"
		"         look you can use #map flag AsciiGraphics off. This will enable the\n"
		"         standard display which uses UTF-8 box drawing characters, results may\n"
		"         vary depending on the font used.\n"
		"\n"
		"         If your terminal supports UTF-8 you can also give #Map flag unicode on\n"
		"         a try.\n"
		"\n"
		"         If you want to display the map in a different location of the screen\n"
		"         use something like:\n"
		"         <178>#split 0 1 0 -80\n"
		"         #map offset 1 81 -4 -1<278>\n"
		"         This will display the map on the right side of the screen, if the\n"
		"         width of the screen is wide enough.\n"
		"<178>\n"
		"         #map undo\n"
		"<278>\n"
		"         If you accidentally walk into the wall on your MUD the mapper will\n"
		"         still create a new room. You can easily fix this mistake by using\n"
		"         #map undo. If you want to move around on the map without moving around\n"
		"         on the MUD you can use: #map move {direction}. To delete a room\n"
		"         manually you can use: #map delete {direction}. To create a room\n"
		"         manually you can use: #map dig {direction}.\n"
		"<178>\n"
		"         #map write <filename>\n"
		"<278>\n"
		"         You can save your map using #map write, to load a map you can use\n"
		"         #map read <filename>.\n"
		"<178>\n"
		"         #map set <option> <value>\n"
		"<278>\n"
		"         You can set the room name using #map set roomname <name>. You either\n"
		"         have to do this manually or create triggers to set the room name\n"
		"         automatically. Once the room name is set you can use #map goto with\n"
		"         the room name to visit it. If there are two rooms with the same name\n"
		"         #map goto will go to the most nearby room. If you want to always go\n"
		"         to the same room you should memorize the room number. You can further\n"
		"         narrow down the matches by providing additional arguments, for example:\n"
		"<178>\n"
		"         #map goto {dark alley} {roomexits} {n;e} {roomarea} {Haddock Ville}\n"
		"<278>\n"
		"         You can set the room weight using #map set roomweight {value}. The\n"
		"         weight by default is set to 1.0 and it represents the difficulty of\n"
		"         traversing the room. If you have a lake as an alternative route, and\n"
		"         traversing water rooms is 4 times slower than regular rooms, then you\n"
		"         could set the weight of the lake rooms to 4.0. If the lake is 3 rooms\n"
		"         wide the total weight is 12. If walking around the lake has a weight\n"
		"         less than 12 the mapper will go around the lake, if the weight is\n"
		"         greater than 12 the mapper will take a route through the lake.\n"
		"\n"
		"         You can set the room symbol using #map set roomsymbol {value}. The\n"
		"         symbol should be one, two, or three characters, which can be\n"
		"         colorized. You can for example mark shops with an 'S' and colorize the\n"
		"         'S' depending on what type of shop it is.\n"
		"<178>\n"
		"         #map run <location> <delay>\n"
		"<278>\n"
		"         The run command will have tintin find the shortest path to the given\n"
		"         location and execute the movement commands to get there. You can\n"
		"         provide a delay in seconds with floating point precision, for example:\n"
		"         <178>#map run {dark alley} {0.5}<278>\n"
		"<178>\n"
		"         #map insert {direction} {flag}\n"
		"<278>\n"
		"         The insert command is useful for adding spacer rooms called void rooms.\n"
		"         Often rooms overlap, and by adding void rooms you can stretch out\n"
		"         exits. For example: #map insert north void. You cannot enter void rooms\n"
		"         once they've been created, so you'll have to use #map info in an\n"
		"         adjacent room to find the room vnum, then use #map goto {vnum} to\n"
		"         visit.\n"
		"\n"
		"         It's also possible to align rooms using void rooms. This is easily\n"
		"         done using #map insert north void.\n",

		"map path pathdir"
	},

	{
		"MATH",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #math <178>{<278>variable<178>} {<278>expression<178>}<278>\n"
		"\n"
		"         Performs math operations and stores the result in a variable.  The math\n"
		"         follows a C-like precedence, as follows, with the top of the list\n"
		"         having the highest priority.\n"
		"\n"
		"         Operators       Priority     Function\n"
		"         ------------------------------------------------\n"
		"         !               0            logical not\n"
		"         ~               0            bitwise not\n"
		"         *               1            integer multiply\n"
		"         **              1            integer power\n"
		"         /               1            integer divide\n"
		"         //              1            integer sqrt // 2 or cbrt // 3\n"
		"         %               1            integer modulo\n"
		"         d               1            integer random dice roll\n"
		"         +               2            integer addition\n"
		"         -               2            integer subtraction\n"
		"         <<              3            bitwise shift\n"
		"         >>              3            bitwise shift\n"
		"         ..              3            bitwise ellipsis\n"
		"         >               4            logical greater than\n"
		"         >=              4            logical greater than or equal\n"
		"         <               4            logical less than\n"
		"         <=              4            logical less than or equal\n"
		"         ==              5            logical equal (can use regex)\n"
		"         ===             5            logical equal (never regex)\n"
		"         !=              5            logical not equal (can use regex)\n"
		"         !==             5            logical not equal (never regex)\n"
		"          &              6            bitwise and\n"
		"          ^              7            bitwise xor\n"
		"          |              8            bitwise or\n"
		"         &&              9            logical and\n"
		"         ^^             10            logical xor\n"
		"         ||             11            logical or\n"
		"\n"
		"         True is any non-zero number, and False is zero.  Parentheses () have\n"
		"         highest precedence, so inside the () is always evaluated first.\n"
		"         Strings must be enclosed in { } and use regex with == and !=,\n"
		"         in the case of <= and >= the alphabetic order is compared.\n"
		"\n"
		"         The #if and #switch commands use #math. Several commands accepting\n"
		"         integer input allow math operations as well.\n"
		"\n"
		"         Floating point precision is added by using the decimal . operator.\n"
		"\n"
		"<178>Example<278>: #math {heals} {$mana / 40}\n"
		"         Assuming there is a variable $mana, divides its value by 40 and stores\n"
		"         the result in $heals.\n"
		"\n"
		"<178>Example<278>: #action {^You receive %0 experience} {updatexp %0}\n"
		"         #alias updatexp {#math {xpneed} {$xpneed - %0}\n"
		"         Let's say you have a variable which stores xp needed for your next\n"
		"         level.  The above will modify that variable after every kill, showing\n"
		"         the amount still needed.\n"
		"\n"
		"<178>Example<278>: #action {%0 tells %1}\n"
		"           {#if {{%0} == {Bubba} && $afk} {reply I'm away, my friend.}}\n"
		"         When you are away from keyboard, it will only reply to your friend.\n",

		"cat format function local mathematics replace script variable"
	},

	{
		"MATHEMATICS",
		TOKEN_TYPE_STRING,
		"<178>Number operations\n"
		"<278>"
		"         Operators       Priority     Function\n"
		"         ------------------------------------------------\n"
		"         !               0            logical not\n"
		"         ~               0            bitwise not\n"
		"         *               1            integer multiply\n"
		"         **              1            integer power\n"
		"         /               1            integer divide\n"
		"         //              1            integer sqrt // 2 or cbrt // 3\n"
		"         %               1            integer modulo\n"
		"         d               1            integer random dice roll\n"
		"         +               2            integer addition\n"
		"         -               2            integer subtraction\n"
		"         <<              3            bitwise shift\n"
		"         >>              3            bitwise shift\n"
		"         >               4            logical greater than\n"
		"         >=              4            logical greater than or equal\n"
		"         <               4            logical less than\n"
		"         <=              4            logical less than or equal\n"
		"         ==              5            logical equal\n"
		"         !=              5            logical not equal\n"
		"          &              6            bitwise and\n"
		"          ^              7            bitwise xor\n"
		"          |              8            bitwise or\n"
		"         &&              9            logical and\n"
		"         ^^             10            logical xor\n"
		"         ||             11            logical or\n"
		"\n"
		"Operator priority can be ignored by using parentheses, for example (1 + 1) * 2\n"
		"equals 4, while 1 + 1 * 2 equals 3.\n"
		"\n"
		"<178>String operations<278>\n"
		"<278>"
		"         Operators       Priority     Function\n"
		"         ------------------------------------------------\n"
		"         >               4            alphabetical greater than\n"
		"         >=              4            alphabetical greater than or equal\n"
		"         <               4            alphabetical less than\n"
		"         <=              4            alphabetical less than or equal\n"
		"         ==              5            alphabetical equal using regex\n"
		"         !=              5            alphabetical not equal using regex\n"
		"         ===             5            alphabetical equal\n"
		"         !==             5            alphabetical not equal\n"
		"\n"
		"Strings must be encased in double quotes or braces. The > >= < <= operators\n"
		"perform basic string comparisons. The == != operators perform regular\n"
		"expressions, with the argument on the left being the string, and the argument\n"
		"on the right being the regex. For example {bla} == {%*a} would evaluate as 1.\n",

		"math regexp"
	},

	{
		"MESSAGE",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #message <178>{<278>listname<178>} {<278>on<178>|<278>off<178>}<278>\n"
		"\n"
		"         This will show the message status of all your lists if typed without an\n"
		"         argument. If you set for example VARIABLES to OFF you will no longer be\n"
		"         spammed when correctly using the #VARIABLE and #UNVARIABLE commands.\n",

		"class debug ignore info kill"
	},

	{
		"METRIC SYSTEM",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"             Name  Symbol                              Factor\n"
		"           --------------------------------------------------\n"
//		"            Yotta       Y   1 000 000 000 000 000 000 000 000\n"
//		"            Zetta       Z       1 000 000 000 000 000 000 000\n"
//		"              Exa       E           1 000 000 000 000 000 000\n"
//		"             Peta       P               1 000 000 000 000 000\n"
//		"             Tera       T                   1 000 000 000 000\n"
//		"             Giga       G                       1 000 000 000\n"
		"             Mega       M                           1 000 000\n"
		"             Kilo       K                               1 000\n"
		"\n"
		"            milli       m                               0.001\n"
		"            micro       u                           0.000 001\n",
//		"             nano       n                       0.000 000 001\n"
//		"             pico       p                   0.000 000 000 001\n"
//		"            femto       f               0.000 000 000 000 001\n"
//		"             atto       a           0.000 000 000 000 000 001\n"
//		"            zepto       z       0.000 000 000 000 000 000 001\n"
//		"            yocto       y   0.000 000 000 000 000 000 000 001\n",
		
		"echo format math"
	},

	{
		"MOUSE",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"         To enable xterm mouse tracking use #CONFIG MOUSE ON.\n"
		"\n"
		"         To see mouse events as they happen use #CONFIG MOUSE INFO. This\n"
		"         information can then be used to create mouse events with the #event\n"
		"         command and buttons with the #button command.\n"
		"\n"
		"         Visual buttons and pop-ups can be drawn on the screen with the #draw\n"
		"         command.\n"
		"\n"
		"         The input field can be changed and renamed using #screen inputregion,\n"
		"         which allows creating named events for enter handling.\n"
		"\n"
		"         Links can be created using the MSLP protocol which will generate link\n"
		"         specific events when clicked.\n"
		"<278>\n",
		
		"button draw event MSLP"
	},

	{
		"MSDP",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"         MSDP (Mud Server Data Protocol) is part of the #port functionality.\n"
		"         See #help event for additional documentation as all MSDP events are\n"
		"         available as regular events.\n"
		"\n"
		"         Available MSDP events can be queried using the MSDP protocol\n"
		"         as described in the specification.\n"
		"<178>\n"
		"         https://tintin.sourceforge.io/protocols/msdp\n",

		"event port"
	},

	{
		"MSLP",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"         MSLP (Mud Server Link Protocol) requires enabling #config mouse on,\n"
		"         and creating the appropriate LINK events.\n"
		"<278>\n"
		"         The simplest link can be created by surrounding a keyword with the\n"
		"         \\e[4m and \\e[24m tags.\n"
		"\n"
		"<178>Example<278>: #substitute {\\b{n|e|s|w|u|d}\\b} {\\e[4m%1\\[24m}\n"
		"\n"
		"         This would display 'Exits: n, e, w.' as 'Exits: \e[4mn\e[24m, \e[4me\e[24m, \e[4mw\e[24m.'.\n"
		"\n"
		"         When clicked this would trigger the PRESSED LINK MOUSE BUTTON ONE\n"
		"         event of which %4 will hold the link command and %6 holds the\n"
		"         link name, which in the case of a simple link will be empty.\n"
		"\n"
		"<178>Example<278>: #event {PRESSED LINK MOUSE BUTTON ONE} {#send {%4}}\n"
		"\n"
		"         Keep in mind that if you change PRESSED to DOUBLE-CLICKED the link\n"
		"         will only work if the text does not scroll in between clicks.\n"
		"\n"
		"         If you want to create a complex link use an OSC code.\n"
		"\n"
		"<178>Example<278>: #sub {\\bsmurf\\b} {\\e]68;1;;say I hate smurfs!\\a\\e[4m%0\\e[24m}\n"
		"\n"
		"         If you have the LINK event of the previous example set, the %4\n"
		"         argument will contain 'say I hate smurfs!'.\n"
		"\n"
		"<178>Example<278>: #sub {\\bgoblin\\b} {\\e]68;1;SEND;kill goblin\\a\\e[4m%0\\e[24m}\n"
		"\n"
		"         Notice the previous instance of ;; has been replaced with ;SEND;\n"
		"         which will name the link. This will generate a named event.\n"
		"\n"
		"<178>Example<278>: #event {PRESSED LINK SEND MOUSE BUTTON ONE} {#send {%4}}\n"
		"\n"
		"         By naming links you can organize things a little bit better instead\n"
		"         of tunneling everything through the same event.\n"
		"<278>\n"
		"         Keep in mind that the server is allowed to use \\e]68;1;\\a as well,\n"
		"         subsequently various security measures are in place.\n"
		"\n"
		"         To create secure links, which are filtered out when send by a server,\n"
		"         you need to use \\e]68;2;\\a, and they instead trigger the SECURE LINK\n"
		"         event.\n"
		"\n"
		"<178>Example<278>: #sub {%* tells %*} {\\e]68;2;EXEC;#cursor set tell %1 \\a\\e[4m%0\\e[24m}\n"
		"<178>       <278>  #event {PRESSED SECURE LINK EXEC MOUSE BUTTON ONE} {%4}\n"
		"\n"
		"         This would make you start a reply when clicking on a tell.\n"
		"\n"
		"Website: https://tintin.mudhalla.net/protocols/mslp\n",

		"event port"
	},

	{
		"NOP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #nop <178>{<278>whatever<178>}<278>\n"
		"\n"
		"         Short for 'no operation', and is ignored by the client.  It is useful\n"
		"         for commenting in your coms file, any text after the nop and before a\n"
		"         semicolon or end of line is ignored. You shouldn't put braces { } in it\n"
		"         though, unless you close them properly.\n"
		"\n"
		"<178>Comment<278>: By using braces you can comment out multiple lines of code in a script\n"
		"         file.\n"
		"\n"
		"         For commenting out an entire trigger and especially large sections of\n"
		"         triggers you would want to use /* text */\n"
		"\n"
		"<178>Example<278>: #nop This is the start of my script file.\n",

		"read"
	},
	{
		"PARSE",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #parse <178>{<278>string<178>} {<278>variable<178>} {<278>commands<178>}<278>\n"
		"\n"
		"         Like the loop statement, parse will loop from start to finish through\n"
		"         the given string.  The value of the current character is stored in the\n"
		"         provided variable.\n"
		"\n"
		"<178>Example<278>: #parse {hello world} {char} {#show $char}\n",

		"break continue foreach list loop repeat return while"
	},
	{
		"PATH",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #path <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         create   Will clear the path and start path mapping.\n"
		"         delete   Will delete the last move of the path.\n"
		"         describe Describe the path and current position.\n"
		"         destroy  Will clear the path and stop path mapping.\n"
		"         get      Will get either the length or position.\n"
		"         goto     Go the the start, end, or given position index.\n"
		"         insert   Add the given argument to the path.\n"
		"         load     Load the given variable as the new path.\n"
		"         map      Display the map and the current position.\n"
		"         move     Move the position forward or backward. If a number is given\n"
		"                  the position is changed by the given number of steps.\n"
		"         run      Execute the current path, with an optional floating point\n"
		"                  delay in seconds as the second argument.\n"
		"         save     Save the path to a variable. You must specify whether you\n"
		"                  want to save the path 'forward' or 'backward'.\n"
		"         swap     Switch the forward and backward path.\n"
		"         unzip    Load the given speedwalk as the new path.\n"
		"         walk     Take one step forward or backward.\n"
		"         zip      Turn the path into a speedwalk.\n"
		"\n"
		"<178>Example<278>: #path ins {unlock n;open n} {unlock s;open s}\n",

		"map pathdir"
	},
	{
		"PATHDIR",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #pathdir <178>{<278>dir<178>} {<278>reversed dir<178>} {<278>coord<178>}<278>\n"
		"\n"
		"         By default tintin sets the most commonly used movement commands\n"
		"         meaning you generally don't really have to bother with pathdirs.\n"
		"         Pathdirs are used by the #path and #map commands.\n"
		"\n"
		"         The first argument is a direction, the second argument is the reversed\n"
		"         direction.  The reverse direction of north is south, etc.\n"
		"\n"
		"         The third argument is a spatial coordinate which is a power of two.\n"
		"         'n' is 1, 'e' is 2, 's' is 4, 'w' is '8', 'u' is 16, 'd' is 32. The\n"
		"         exception is for compound directions, whose value should be the sum\n"
		"         of the values of each cardinal direction it is composed of. For\n"
		"         example, 'nw' is the sum of 'n' and 'w' which is 1 + 8, so 'nw'\n"
		"         needs to be given the value of 9. This value is required for the\n"
		"         #map functionality to work properly.\n"
		"\n"
		"<178>Example<278>: #pathdir {ue} {dw} {18}\n"
		"         #pathdir {dw} {ue} {40}\n"
		"\n"
		"<178>Comment<278>: You can remove a pathdir with the #unpathdir command.\n",
		
		"map path"
	},
	{
		"PCRE",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"         A regular expression, regex or regexp is a sequence of characters that\n"
		"         defines a search pattern. Since the 1980s, different syntaxes for\n"
		"         writing regular expressions exist, the two most widely used ones being\n"
		"         the POSIX syntax and the similar but more advanced Perl standard.\n"
		"         TinTin++ supports the Perl standard known as PCRE (Perl Compatible\n"
		"         Regular Expressions).\n"
		"\n"
		"         Regular expressions are an integral part of TinTin++, but keep in mind\n"
		"         that tintin doesn't allow you to use regular expressions directly,\n"
		"         instead it uses a simpler intermediate syntax that still allows more\n"
		"         complex expressions when needed.\n"
		"\n"
		"         Commands that utilize regular expressions are: action, alias, elseif,\n"
		"         gag, grep, highlight, if, kill, local, math, prompt, regexp, replace,\n"
		"         substitute, switch, variable and while. Several other commands use\n"
		"         regular expressions in minor ways. Fortunately the basics are very\n"
		"         easy to learn.\n"
		"\n"
		"<178>         TinTin++ Regular Expression<278>\n"
		"\n"
		"         The following support is available for regular expressions.\n"
		"\n"
		"       ^ match start of line.\n"
		"       $ match of end of line.\n"
		"       \\ escape one character.\n"
		"\n"
		"  %1-%99 match of any text, stored in the corresponding index.\n"
		"      %0 should be avoided in the regex, contains all matched text.\n"
		"     { } embed a perl compatible regular expression, matches are stored.\n"
		"   %!{ } embed a perc compatible regular expression, matches are not stored.\n"
		"\n"
		"[ ] . + | ( ) ? * are treated as normal text unless used within braces. Keep in\n"
		"mind that { } is replaced with ( ) automatically unless %!{ } is used.\n"
		"<178>\n"
		"         TinTin++  Description                            POSIX<278>\n"
		"      %d Match zero to any number of digits               ([0-9]*?)\n"
		"      %D Match zero to any number of non-digits           ([^0-9]*?)\n"
		"      %i Matches become case insensitive                  (?i)\n"
		"      %I Matches become case sensitive (default)          (?-i)\n"
		"      %s Match zero to any number of spaces               ([\\r\\n\\t ]*?)\n"
		"      %w Match zero to any number of word characters      ([A-Za-z0-9_]*?)\n"
		"      %W Match zero to any number of non-word characters  ([^A-Za-z0-9_]*?)\n"
		"      %? Match zero or one character                      (.\?\?)\n"
		"      %. Match one character                              (.)\n"
		"      %+ Match one to any number of characters            (.+?)\n"
		"      %* Match zero to any number of characters           (.*?)\n"
		"<178>\n"
		"         Variables<278>\n"
		"\n"
		"         If you use %1 in an action to perform a match the matched string is\n"
		"         stored in the %1 variable which can be used in the action body.\n"
		"\n"
		"Example: %1 says 'Tickle me'} {tickle %1}\n"
		"\n"
		"         If you use %2 the match is stored in %2, etc. If you use an unnumbered\n"
		"         match like %* or %S the match is stored at the last used index\n"
		"         incremented by one.\n"
		"\n"
		"Example: %3 says '%*'} {#if {\"%4\" == \"Tickle me\"} {tickle %3}}\n"
		"\n"
		"         The maximum variable index is 99. If you begin an action with %* the\n"
		"         match is stored in %1. You should never use %0 in the trigger part of\n"
		"         an action, when used in the body of an action %0 contains all the parts\n"
		"         of the string that were matched.\n"
		"\n"
		"         To prevent a match from being stored use %!*, %!w, etc.\n"
		"<178>\n"
		"         Perl Compatible Regular Expressions<278>\n"
		"\n"
		"         You can embed a PCRE (Perl Compatible Regular Expression) using curley\n"
		"         braces { }, these braces are replaced with parentheses ( ) unless you\n"
		"         use %!{ }.\n"
		"<178>\n"
		"         Or<278>\n"
		"\n"
		"         You can separate alternatives within a PCRE using the | character.\n"
		"\n"
		"Example: #act {%* raises {his|her|its} eyebrows.} {say 42..}\n"
		"<178>\n"
		"         Brackets<278>\n"
		"\n"
		"         You can group alternatives and ranges within a PCRE using brackets.\n"
		"\n"
		"Example: #act {%* says 'Who is number {[1-9]}} {say $number[%2] is number %2}\n"
		"\n"
		"         The example only triggers if someone provides a number between 1 and\n"
		"         9. Any other character will cause the action to not trigger.\n"
		"\n"
		"Example: #act {%* says 'Set password to {[^0-9]*}$} {say The password must\n"
		"           contain at least one number, not for security reasons, but just to\n"
		"           annoy you.} {4}\n"
		"\n"
		"         When the ^ character is used within brackets it creates an inverse\n"
		"         search, [^0-9] matches every character except for a number between 0\n"
		"         and 9.\n"
		"<178>\n"
		"         Quantification<278>\n"
		"\n"
		"         A quantifier placed after a match specifies how often the match is\n"
		"         allowed to occur.\n"
		"\n"
		"       ? repeat zero or one time.\n"
		"       * repeat zero or multiple times.\n"
		"       + repeat once or multiple times.\n"
		"     {n} repeat exactly n times, n must be a number.\n"
		"    {n,} repeat at least n times, n must be a number.\n"
		"   {n,o} repeat between n and o times, n and o must be a number.\n"
		"\n"
		"Example: #act {%* says 'Who is number {[1-9][0-9]{0,2}} {Say $number[%2] is\n"
		"           number %2}\n"
		"\n"
		"         The example only triggers if someone provides a number between 1 and\n"
		"         999.\n"
		"\n"
		"         <178>Parantheses<278>\n"
		"\n"
		"         TinTin Regular Expressions automatically add parenthesis, for example\n"
		"         %* translates to (.*?) in PCRE unless the %* is found at the start or\n"
		"         end of the line, in which cases it translates to (.*). Paranthesis in\n"
		"         PCRE causes a change in execution priority similar to mathematical\n"
		"         expressions, but parentheses also causes the match to be stored to a\n"
		"         variable.\n"
		"\n"
		"         When nesting multiple sets of parentheses each nest is assigned its\n"
		"         numerical variable in order of appearance.\n"
		"\n"
		"Example: #act {%* chats '{Mu(ha)+}'} {chat %2ha!}\n"
		"\n"
		"         If someone chats Muha you will chat Muhaha! If someone chats Muhaha\n"
		"         you will chat Muhahaha!\n"
		"\n"
		"         <178>Lazy vs Greedy<278>\n"
		"\n"
		"         By default regex matches are greedy, meaning {.*} will capture as much\n"
		"         text as possible.\n"
		"\n"
		"Example: #regex {bli bla blo} {^{.*} {.*}$} {#show Arg1=(&1) Arg2=(&2)}\n"
		"\n"
		"         This will display: Arg1=(bli bla) Arg2=(blo)\n"
		"\n"
		"         By appending a ? behind a regex it becomes lazy, meaning {.*?} will\n"
		"         capture as little text as possible.\n"
		"\n"
		"Example: #regex {bli bla blo} {^{.*?} {.*?}$} {#show Arg1=(&1) Arg2=(&2)}\n"
		"\n"
		"         This will display: Arg1=(bli) Arg2=(bla blo).\n"
		"\n"
		"         <178>Escape Codes<278>\n"
		"\n"
		"         PCRE support the following escape codes.\n"
		"<178>\n"
		"    PCRE Description                                    POSIX<278>\n"
		"      \\A Match start of string                          ^\n"
		"      \\b Match word boundaries                          (^|\\r|\\n|\\t| |$)\n"
		"      \\B Match non-word boundaries                      [^\\r\\n\\t ]\n"
		"      \\c Insert control character                       \\c\n"
		"      \\d Match digits                                   [0-9]\n"
		"      \\D Match non-digits                               [^0-9]\n"
		"      \\e Insert escape character                        \\e\n"
		"      \\f Insert form feed character                     \\f\n"
		"      \\n Insert line feed character                     \\n\n"
		"      \\r Insert carriage return character               \\r\n"
		"      \\s Match spaces                                   [\\r\\n\\t ]\n"
		"      \\S Match non-spaces                               [^\\r\\n\\t ]\n"
		"      \\t Insert tab character                           \\t\n"
		"      \\w Match letters, numbers, and underscores        [A-Za-z0-9_]\n"
		"      \\W Match non-letters, numbers, and underscores    [^A-Za-z0-9_]\n"
		"      \\x Insert hex character                           \\x\n"
		"      \\Z Match end of string                            $\n"
		"\n"
		"         \\s matches one space, \\s+ matches one or multiple spaces.\n"
		"\n"
		"         <178>Color triggers<278>\n"
		"\n"
		"         To make matching easier text triggers (Actions, Gags, Highlights,\n"
		"         Prompts, and Substitutes) have their color codes stripped. If you\n"
		"         want to create a color trigger you must start the triggers with a ~\n"
		"         (tilde). To make escape codes visible use #config {convert meta} on.\n"
		"\n"
		"Example: #action {~\\e[1;37m%1} {#var roomname %1}\n"
		"\n"
		"         If the room name is the only line on the server in bright white\n"
		"         white color trigger will save the roomname.\n"
		"\n"
		"\n"
		"         This covers the basics. PCRE has more options, most of which are\n"
		"         somewhat obscure, so you'll have to read a PCRE manual for additional\n"
		"         information.\n",

		"map path"
	},

	{
		"PORT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #port <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         <178>#port {init} {name} {port} {file}\n"
		"         <278>  Initilize a port session.\n"
		"\n"
		"         <178>#port {call} {address} {port}\n"
		"         <278>  Connect to a remote socket.\n"
		"\n"
		"         <178>#port {color} {color names}\n"
		"         <278>  Set the default color of port messages.\n"
		"\n"
		"         <178>#port {dnd}\n"
		"         <278>  Do Not Disturb. Decline new connections\n"
		"\n"
		"         <178>#port {group} {name} {group}\n"
		"         <278>  Assign a socket group.\n"
		"\n"
		"         <178>#port {ignore} {name}\n"
		"         <278>  Ignore a socket\n"
		"\n"
		"         <178>#port {info}\n"
		"         <278>  Display information about the port session.\n"
		"\n"
		"         <178>#port {name} {name}\n"
		"         <278>  Change socket name.\n"
		"\n"
		"         <178>#port {prefix} {text}\n"
		"         <278>  Set prefix before each message.\n"
		"\n"
		"         <178>#port {send} {name|all} {text}\n"
		"         <278>  Send data to socket\n"
		"\n"
		"         <178>#port {uninitialize}\n"
		"         <278>  Uninitialize the port session.\n"
		"\n"
		"         <178>#port {who}\n"
		"         <278>  Show all connections\n"
		"\n"
		"         <178>#port {zap} {name}\n"
		"         <278>  Close a connection\n"
		"\n"
		"         The port command is very similar to chat except that it creates a\n"
		"         new session dedicated to receiving socket connections at the given\n"
		"         port number without built-in support for a communication protocol.\n"
		"\n"
		"         You can init with 0 as the port number to create a dummy session.\n",

		"all chat run session sessionname snoop ssl zap"
	},

	{
		"PROMPT",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #prompt <178>{<278>text<178>} {<278>new text<178>} {<278>row #<178>} <178>{<278>col #<178>}<278>\n"
		"\n"
		"         Prompt is a feature for split window mode, which will capture a line\n"
		"         received from the server and display it on the status bar of your\n"
		"         split screen terminal. You would define <text> and <new text> the\n"
		"         same way as with a substitution.\n"
		"\n"
		"         The row number is optional and useful if you use a non standard split\n"
		"         mode. A positive row number draws #row lines from the top while a\n"
		"         negative number draws #row lines from the bottom. Without an argument\n"
		"         #prompt will write to the default split line, which is one row above\n"
		"         the input line, typically at row -2.\n"
		"\n"
		"         The col number is optional and can be used to set the column index.\n"
		"         A positive col number draws the given number of columns from the left,\n"
		"         while a negative col number draws from the right. If you leave the\n"
		"         column argument empty tintin will clear the row before printing at\n"
		"         the start of the row.\n"
		"\n"
		"         The #show command takes a row and col argument as well so it's also\n"
		"         possible to place text on your split lines using #show.\n"
		"\n"
		"<178>Comment<278>: See <178>#help split<278> for more information on split mode.\n"
		"\n"
		"<178>Comment<278>: See <178>#help substitute<278> for more information on text\n"
		"         substitutions.\n"
		"\n"
		"<178>Comment<278>: You can remove a prompt with the #unprompt command.\n",

		"action gag highlight substitute"
	},
	{
		"READ",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #read <178>{<278>filename<178>}<278>\n"
		"\n"
		"         Reads a commands file into memory.  The coms file is merged in with\n"
		"         the currently loaded commands.  Duplicate commands are overwritten.\n"
		"\n"
		"         If you uses braces, { and } you can use several lines for 1 commands.\n"
		"         This however means you must always match every { with a } for the read\n"
		"         command to work.\n"
		"\n"
		"         You can comment out triggers using /* text */\n",

		"log scan textin write"
	},
	{
		"REGEXP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #regexp <178>{<278>string<178>} {<278>expression<178>} {<278>true<178>} {<278>false<178>}<278>\n"
		"\n"
		"         Compares the string to the given regular expression.\n"
		"\n"
		"         The expression can contain escapes, and if you want to match a literal\n"
		"         \\ character you'll have to use \\\\ to match a single backslash.\n"
		"\n"
		"         Variables are stored in &1 to &99 with &0 holding the matched substring.\n"
		"\n"
		"       ^ force match of start of line.\n"
		"       $ force match of end of line.\n"
		"       \\ escape one character.\n"
		"  %1-%99 lazy match of any text, available at %1-%99.\n"
		"      %0 should be avoided in triggers, and if left alone lists all matches.\n"
		"     { } embed a raw regular expression, matches are stored to %1-%99.\n"
		"   %!{ } embed a raw regular expression, matches are not stored.\n"
		"         [ ] . + | ( ) ? * are treated as normal text unlessed used within\n"
		"         braces. Keep in mind that { } is replaced with ( ) automatically\n"
		"         unless %!{ } is used.\n"
		"\n"
		"         Of the following the (lazy) match is available at %1-%99 + 1\n"
		"\n"
		"      %a match zero to any number of characters including newlines.\n"
		"      %A match zero to any number of newlines.\n"
		"      %d match zero to any number of digits.\n"
		"      %D match zero to any number of non digits.\n"
		"      %p match zero to any number of printable characters.\n"
		"      %P match zero to any number of non printable characters.\n"
		"      %s match zero to any number of spaces.\n"
		"      %S match zero to any number of non spaces.\n"
		"      %u match zero to any number of unicode characters.\n"
		"      %U match zero to any number of non unicode characters.\n"
		"      %w match zero to any number of word characters.\n"
		"      %W match zero to any number of non word characters.\n"
		"\n"
		"      If you want to match 1 digit use %+1d, if you want to match between 3\n"
		"      and 5 spaces use %+3..5s, if you want to match 0 or more word\n"
		"      characters use %+0..w, etc.\n"
		"\n"
		"      %+ match one to any number of characters.\n"
		"      %? match zero or one character.\n"
		"      %. match one character.\n"
		"      %* match zero to any number of characters.\n"
		"\n"
		"      %i matching becomes case insensitive.\n"
		"      %I matching becomes case sensitive (default).\n"
		"\n"
		"         The match is automatically stored to a value between %1 and %99\n"
		"         starting at %1 and incrementing by 1 for every regex. If you use\n"
		"         %15 as a regular expression, the next unnumbered regular expression\n"
		"         would be %16. To prevent a match from being stored use %!*, %!w, etc.\n"
		"\n"
		"<178>Example<278>: #regexp {bli bla blo} {bli {.*} blo} {#show &1}\n",

		"case default else elseif if switch"
	},

	{
		"REPEAT",
		TOKEN_TYPE_STRING,
		"<178>Command<278>: #<178>[<078>number<178>] {<278>commands<178>}<278>\n"
		"\n"
		"Sometimes you want to repeat the same command multiple times. This is the\n"
		"easiest way to accomplish that.\n"
		"\n"
		"<178>Example<278>: #10 {buy bread}\n",
		
		"break continue foreach list loop parse return while"
	},
	{
		"REPLACE",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #replace <178>{<278>variable<178>} {<278>oldtext<178>} {<278>newtext<178>}<278>\n"
		"\n"
		"         Searches the variable text replacing each occurrence of 'oldtext' with\n"
		"         'newtext'. The 'newtext' argument can be a regular expression.\n"
		"\n"
		"         Variables are stored in &1 to &99 with &0 holding the matched substring.\n",

		"cat format function local math script variable"
	},
	{
		"RETURN",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #return <178>{<278>text<178>}<278>\n"
		"\n"
		"         This command can be used to break out of a command string being\n"
		"         executed.\n"
		"\n"
		"         If used inside a #function you can use #return with an argument to both\n"
		"         break out of the function and set the result variable.\n",

		"break continue foreach list loop parse repeat while"
	},
	{
		"RUN",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #run <178>{<278>name<178>} {<278>shell command<178>} {<278>file<178>}<278>\n"
		"\n"
		"         The run command works much like the system command except that it\n"
		"         runs the command in a pseudo terminal. The run command also creates\n"
		"         a session that treats the given shell command as a server. This\n"
		"         allows you to run ssh, as well as any other shell application, with\n"
		"         full tintin scripting capabilities. If a file name is given the file\n"
		"         is loaded prior to execution.\n"
		"\n"
		"<178>Example<278>: #run {somewhere} {ssh someone@somewhere.com}\n"
		"<178>Example<278>: #run {something} {tail -f chats.log}\n",

		"all port session sessionname snoop ssl zap"
	},
	{
		"SCAN",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #scan <178>{<278>abort<178>|<278>csv<178><178>|<278>tsv<178><178>|<278>txt<178>} {<278>filename<178>}<278>\n"
		"\n"
		"         The scan command is a file reading utility.\n"
		"\n"
		"         <178>#scan {abort}\n"
		"         <278>  This command must be called from with a SCAN event and will\n"
		"         <278>  abort the scan if one is in progress.\n"
		"\n"
		"         <178>#scan {csv} <filename>\n"
		"         <278>  The scan csv command reads in a comma separated value file\n"
		"           without printing the content to the screen. Instead it triggers one\n"
		"           of two events.\n"
		"\n"
		"           The SCAN CSV HEADER event is triggered on the first line of the csv\n"
		"           file. The SCAN CSV LINE event is triggered on the second and each\n"
		"           subsequent line of the csv file. The %0 argument contains the entire\n"
		"           line, with  %1 containing the first value, %2 the second value, etc,\n"
		"           all the way up to %99.\n"
		"\n"
		"           Values containing spaces must be surrounded with quotes, keep in mind\n"
		"           newlines within quotes are not supported. Use two quotes to print one\n"
		"           literal quote character.\n"
		"\n"
		"<178>         #scan {dir} <filename> <variable>\n"
		"<278>\n"
		"          The scan dir command will read the given filename or directory and\n"
		"          store any gathered information into the provided variable.\n"
		"\n"
		"         <178>#scan {tsv} <filename>\n"
		"\n"
		"         <278>  The scan tsv <filename> command reads in a tab separated value file\n"
		"           without printing the content to the screen. Instead it triggers the\n"
		"           SCAN TSV HEADER event for the first line and SCAN TSV LINE for all\n"
		"           subsequent lines.\n"
		"\n"
		"         <178>#scan {file} <filename> {commands}\n"
		"\n"
		"         <278>  The scan file command reads the given files and executes the\n"
		"            commands argument. &0 contains the raw content of the file and\n"
		"            &1 contains the plain content. &2 contains the raw byte size of the\n"
		"            file and &3 the plain byte size. &5 contains the line count.\n"
		"\n"
		"         <178>#scan {txt} <filename>\n"
		"\n"
		"         <278>  The scan txt <filename> command reads in a file and sends its content\n"
		"           to the screen as if it was send by a server. After using scan you can\n"
		"           use page-up and down to view the file.\n"
		"\n"
		"           This command is useful to convert ansi color files to html or viewing\n"
		"           raw log files.\n"
		"\n"
		"           Actions, highlights, and substitutions will trigger as normal, and it\n"
		"           is possible to create an action to execute #scan abort to prematurely\n"
		"           stop the scan.\n",

		"read textin"
	},

	{
		"SCREEN",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #screen <178>{<278>option<178>}<178> {<278>argument<178>}\n"
		"\n"
		"         <278>The screen command offers a variety of screen manipulation\n"
		"         commands and utilities.\n"
		"\n"
		"         <178>#screen blur\n"
		"         <278>  Move the terminal to the back of the stack.\n"
		"\n"
		"         <178>#screen clear [all|scroll region|square] <args>\n"
		"         <278>  Provide 4 arguments defining the top left and bottom right corner\n"
		"         <888>  when erasing a square.\n"
		"\n"
		"         <178>#screen focus\n"
		"         <278>  Move the terminal to the front of the stack.\n"
		"\n"
		"         <178>#screen fullscreen [on|off]\n"
		"         <278>  Toggles fullscreen mode when used without an argument.\n"
		"\n"
		"         <178>#screen get <option> <var>\n"
		"         <278>  Get various screen options and save them to <var>. Use #screen\n"
		"         <278>  get without an argument to see all available options.\n"
		"\n"
		"         <178>#screen info\n"
		"         <278>  Debugging information.\n"
		"\n"
		"         <178>#screen inputregion <square> [name]\n"
		"         <278>  Set the input region. The name argument is optional and can be\n"
		"         <278>  used to create named RECEIVED INPUT [NAME] events.\n"
		"\n"
		"         <178>#screen load <both|label|title>\n"
		"         <278>  Reload the saved title, label, or both.\n"
		"\n"
		"         <178>#screen minimize <on|off>\n"
		"         <278>  Minimize with on, restore with off.\n"
		"\n"
		"         <178>#screen maximize [on|off]\n"
		"         <278>  Maximize with on, restore with off.\n"
		"\n"
		"         <178>#screen move <height> <width>\n"
		"         <278>  Move the upper left corner of the terminal to pixel coordinate.\n"
		"\n"
		"         <178>#screen raise <event>\n"
		"         <278>  This will raise several screen events with %1 and %2 arguments.\n"
		"\n"
		"         <178>#screen refresh\n"
		"         <278>  Terminal dependant, may do nothing.\n"
		"\n"
		"         <178>#screen rescale <height> <width>\n"
		"         <278>  Resize the screen to the given height and width in pixels.\n"
		"\n"
		"         <178>#screen resize <rows> <cols>\n"
		"         <278>  Resize the screen to the given height and width in characters.\n"
		"\n"
		"         <178>#screen save <both|label|title>\n"
		"         <278>  Save the title, label, or both.\n"
		"\n"
		"         <178>#screen scroll <square>\n"
		"         <278>  Set the scrolling region, changes the split setting.\n"
		"\n"
		"         <178>#screen set <both|label|title>\n"
		"         <278>  Set the title, label, or both. Only title works on Windows.\n"
		"\n"
		"         <178>#screen swap\n"
		"         <278>  Swap the input and scroll region.\n",

		"bell"
	},

	{
		"SCREEN READER",
		TOKEN_TYPE_STRING,
		"<178>Command<278>: #config <178>{<278>SCREEN READER<178>} {<278>ON|OFF<178>}<278>\n"
		"\n"
		"         Screen reader mode is enabled by using #config screen on.  The main\n"
		"         purpose of the screen reader mode is to report to servers that a\n"
		"         screen reader is being used by utilizing the MTTS standard.  The MTTS\n"
		"         specification is available at:\n"
		"\n"
		"         http://tintin.sourceforge.net/protocols/mtts\n"
		"\n"
		"         With the screen reader mode enabled TinTin++ will try to remove visual\n"
		"         elements where possible.\n",

		"config"
	},

	{
		"SCRIPT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #script <178>{<278>variable<178>}<178> {<278>shell command<178>}<278>\n"
		"\n"
		"         The script command works much like the system command except that it\n"
		"         treats the generated echos as commands if no variable is provided.\n"
		"\n"
		"         This is useful for running php, perl, ruby, and python scripts. You\n"
		"         can run these scrips either from file or from within tintin if the\n"
		"         scripting language allows this.\n"
		"\n"
		"         If you provide a variable the output of the script is stored as a list.\n"
		"\n"
		"<178>Example<278>: #script {ruby -e 'print \"#show hello world\"'}\n"
		"<178>Example<278>: #script {python -c 'print \"#show hello world\"'}\n"
		"<178>Example<278>: #script {php -r 'echo \"#show hello world\"'}\n"
		"<178>Example<278>: #script {path} {pwd};#show The path is $path[1].\n",

		"format function local math replace variable"
	},

	{
		"SEND",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #send <178>{<278>text<178>}<278>\n"
		"\n"
		"         Sends the text directly to the server, useful if you want to start\n"
		"         with an escape code.\n",

		"textin"
	},

	{
		"SESSION",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #session <178>{<278>name<178>} {<278>host<178>} {<278>port<178>} {<278>file<178>}<278>\n"
		"\n"
		"         Starts a telnet session with the given name, host, port, and optional\n"
		"         file name. The name can be anything you want, except the name of an\n"
		"         already existant session, a number, or the keywords '+' and '-'.\n"
		"\n"
		"         If a file name is given the file is only read if the session succesfully\n"
		"         connects.\n"
		"\n"
		"         Without an argument #session shows the currently defined sessions.\n"
		"\n"
		"         If you have more than one session, you can use the following commands:\n"
		"\n"
		"         #session {-}        Switch to the previous session.\n"
		"         #session {+}        Switch to the next session.\n"
		"         #session {<number>} Switch to the given session. Session 0 is the\n"
		"                             startup session, +1 the first, +2 the second, and\n"
		"                             -1 is the last session. Sessions are (currently)\n"
		"                             sorted in order of creation.\n"
		"         #gts                Switch to the startup session. The name gts stands\n"
		"                             for global tintin session.\n"
		"         #ats                Switch to the active session. The name ats stands\n"
		"                             for active tintin session.\n"
		"                             not necessarily the calling session.\n"
		"         #{name}             Activates to the session with the given name.\n"
		"         #{name} {command}:  Executes a command with the given session without\n"
		"                             changing the active session.\n"
		"         @<name>{text}:      Parse text in the given session, substituting the\n"
		"                             variables and functions, and print the result in\n"
		"                             the current active session.\n"
		"\n"
		"         The startup session is named 'gts' and can be used for relog scripts.\n"
		"         Do keep in mind that tickers do not work in the startup session.\n"
		"\n"
		"<178>Example<278>: #event {SESSION DISCONNECTED} {#gts #delay 10 #ses %0 tintin.net 4321}\n",

		"all port run sessionname snoop ssl zap"
	},

	{
		"SESSIONNAME",
		TOKEN_TYPE_STRING,
		"<178>Syntax<278>: #[sessionname] <178>{<278>commands<178>}<278>\n"
		"\n"
		"You can create multiple sessions with the #session command. By default only one\n"
		"session is active, meaning commands you input are executed in the active\n"
		"session. While all sessions receive output, only output sent to the active\n"
		"session is displayed.\n"
		"\n"
		"When you create a session with the #session command you must specify a session\n"
		"name, the session name, prepended with a hashtag, can be used to activate the\n"
		"session when used without an argument. If an argument is given it will be\n"
		"executed by that session as a command, the session will not be activated.\n"
		"\n"
		"<178>Example<278>: #ses one tintin.net 23;#ses two tintin.net 23;#one;#two grin\n"
		"\n"
		"This will create two sessions, the session that was created last (two in this\n"
		"case) will be automatically activated upon creation. Using #one, session one is\n"
		"activated. Using #two grin, the grin social will be executed by session two,\n"
		"session one will remain the active session.\n",
		
		"all port run session snoop ssl zap"
	},

	{
		"SHOWME",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #show <178>{<278>string<178>} {<278>row<178>} <178>{<278>col<178>}<278>\n"
		"\n"
		"         Display the string to the terminal, do not send to the server.  Useful\n"
		"         for status, warnings, etc.  The {row} and col number are optional and\n"
		"         work the same way as the row number of the #prompt trigger.\n"
		"\n"
		"         Actions can be triggered by the show command. If you want to avoid\n"
		"         this from happening use: #line ignore #show {<string>}.\n"
		"\n"
		"<178>Example<278>: #tick {TICK} {#delay 50 #show 10 SECONDS TO TICK!!!} {60}\n"
		"\n"
		"<178>Comment<278>: The #prompt helpfile contains more information on using the\n"
		"         option {row} and {col} arguments.\n",

		"buffer draw echo grep prompt"
	},
	{
		"SNOOP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #snoop <178>{<278>session name<178>} <178>{<278>on<178>|<278>off<178>}<278>\n"
		"\n"
		"         If there are multiple sessions active, this command allows you to monitor\n"
		"         what is going on in the sessions that are not currently active.  The\n"
		"         line of text from other sessions will be prefixed by the session's name.\n"
		"\n"
		"         You can toggle off snoop mode by executing #snoop a second time.\n",

		"all port run session sessionname ssl zap"
	},
	{
		"SPEEDWALK",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"         Speedwalking allows you to enter multiple directions without using\n"
		"         semicolons. Directions should be prefixed with a number and will be\n"
		"         executed the given number of times.\n"
		"\n"
		"         You can enable speedwalking with #CONFIG {SPEEDWALK} {ON}.\n"
		"\n"
		"<178>Example<278>: Without speedwalk, you have to type:\n"
		"         <178>s;s;w;w;w;w;w;s;s;s;w;w;w;n;n;w\n"
		"         <278>With speedwalk, you only have to type:\n"
		"         <178>2s5w3s3w2n1w\n",

		"alias cursor history keypad macro tab"
	},
	{
		"SPLIT",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #split <178>{<278>top bar<178>} {<278>bottom bar<178>} {<278>left bar<178>} {<278>right bar<178>} {<278>input bar<178>}\n"
		"<278>\n"
		"         This option requires for your terminal to support VT100 emulation.\n"
		"<278>\n"
		"         #split allows the creation of a top status bar, a left and right status\n"
		"         bar, a scrolling region, a bottom status bar, and an input line.\n"
		"\n"
		"         <268>╭<268>──────<268>─<268>──────────────────<268>───────╮\n"
		"         <268>│<178>      <178> <178>     top bar      <268>       │\n"
		"         <268>├<268>──────<268>┬<268>──────────────────<268>┬──────┤\n"
		"         <268>│<178> left <268>│<178>    scrolling     <268>│<178> right<268>│\n"
		"         <268>│<178> bar  <268>│<178>     region       <268>│<178>  bar <268>│\n"
		"         <268>├<268>──────<268>┴<268>──────────────────<268>┴──────┤\n"
		"         <268>│<178>      <178> <178>    bottom bar    <268>       │\n"
		"         <268>├<268>──────<268>─<268>──────────────────<268>───────┤\n"
		"         <268>│<178>      <178> <178>    input bar     <268>       │\n"
		"         <268>╰<268>──────<268>─<268>──────────────────<268>───────╯\n"
		"<278>\n"
		"         By default the bottom status bar is filled with dashes --- and\n"
		"         subsequently it is also known as the split line. The scrolling\n"
		"         region is also known as the main screen and this is where all\n"
		"         incoming text is displayed by default.\n"
		"\n"
		"         If you use #split without an argument it will set the height of the\n"
		"         top status bar to 0 lines and the bottom status bar to 1 line.\n"
		"\n"
		"         If you use #split with one argument it will set the height of the top\n"
		"         status bar to the given number of lines and the bottom status bar will\n"
		"         be set to 1 line.\n"
		"\n"
		"         If you use two arguments the first argument is the height of the top\n"
		"         status bar and the second argument the height of the bottom status bar.\n"
		"\n"
		"         The third and fourth argument are optional and default to 0.\n"
		"\n"
		"         The fifth argument is optional and sets the size of the input bar, it\n"
		"         defaults to 1.\n"
		"\n"
		"         It is possible to use negative arguments in which case the bar width\n"
		"         defines the minimum width of the scrolling region.\n"
		"\n"
		"<178>Example<278>: #split 0 0\n"
		"         This will create a split screen with just a scrolling region and an\n"
		"         input line. Great for the minimalist.\n"
		"\n"
		"<178>Example<278>: #split 1 1 0 -80\n"
		"         This will create a split screen with a single line top and bottom\n"
		"         bar. The left bar has a width of 0 while the right bar will be of\n"
		"         variable width. If for example the screen is 100 columns wide, 80\n"
		"         columns will be used for the scrolling region, leaving a right bar\n"
		"         with a width of 20 columns.\n"
		"\n"
		"<178>Comment<278>: You can display text on the split line(s) with the #prompt and\n"
		"         #show {line} {row} commands.\n"
		"\n"
		"<178>Comment<278>: You can remove split mode with the #unsplit command.\n",

		"echo prompt showme"
	},
	{
		"SSL",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #ssl <178>{<278>name<178>} {<278>host<178>} {<278>port<178>} {<278>file<178>}\n"
		"\n"
		"         Starts a secure socket telnet session with the given name, host, port,\n"
		"         and optional file name.\n",

		"all port run sessionname snoop ssl zap"
	},
	{
		"STATEMENTS",
		TOKEN_TYPE_STRING,
		"<278>\n"
		"         TinTin++ knows the following statements.\n"
		"\n"
		"         #break\n"
		"         #case {value} {true}\n"
		"         #continue\n"
		"         #default {commands}\n"
		"         #else {commands}\n"
		"         #elseif {expression} {true}\n"
		"         #foreach {list} {variable} {commands}\n"
		"         #if {expression} {true}\n"
		"         #loop {min} {max} {variable} {commands}\n"
		"         #parse {string} {variable} {commands}\n"
		"         #return {value}\n"
		"         #switch {expression} {commands}\n"
		"         #while {expression} {commands}\n",

		"commands help info"
	},
	{
		"SUBSTITUTE",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #substitute <178>{<278>text<178>} {<278>new text<178>} {<278>priority<178>}<278>\n"
		"\n"
		"         Allows you to replace text from the server with the new text.\n"
		"<278>\n"
		"         The %1-%99 variables can be used to capture text and use it as part of\n"
		"         the new output.\n"
		"<278>\n"
		"         Color codes can be used to color the new text, to restore the color to\n"
		"         that of the original line the <<888>900> color code can be used.\n"
		"<278>\n"
		"         If only one argument is given, all active substitutions that match the\n"
		"         argument are displayed.  Wildcards can be used, see '#help regex' for\n"
		"         additional information on that subject.\n"
		"\n"
		"         If no argument is given, all subs are displayed.\n"
		"\n"
		"<178>Example<278>: #sub {Zoe} {ZOE}\n"
		"         Any instance of Zoe will be replaced with ZOE.\n"
		"\n"
		"<178>Example<278>: #sub {~\\e[0;34m} {\\e[1;34m}\n"
		"         Replace generic dark blue color codes with bright blue ones.\n"
		"\n"
		"<178>Example<278>: #sub {%1massacres%2} {<<888>018>%1<<888>118>MASSACRES<<888>018>%2}\n"
		"         Replaces all occurrences of 'massacres' with 'MASSACRES' in red.\n"
		"\n"
		"<178>Comment<278>: See '#help action', for more information about triggers.\n"
		"\n"
		"<178>Comment<278>: See '#help colors', for more information.\n"
		"\n"
		"<178>Comment<278>: You can remove a substitution with the #unsubstitute command.\n",

		"action gag highlight prompt"
	},
	{
		"SUSPEND",
		TOKEN_TYPE_STRING,
		"<178>Command<278>: #cursor suspend\n"
		"\n"
		"         Temporarily suspends tintin and returns you to your shell.  To\n"
		"         return to tintin, type 'fg' at the shell prompt.\n"
		"\n"
		"         While suspended your tintin sessions will freeze. To keep a\n"
		"         suspended session running use the #detach command.\n",

		"end"
	},
	{
		"SWITCH",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #switch <178>{<278>conditional<178>} {<278>arguments<178>}<278>\n"
		"\n"
		"         The switch command works similar to the switch statement in other\n"
		"         languages. When the 'switch' command is encountered its body is parsed\n"
		"         and each 'case' command found will be compared to the conditional\n"
		"         argument of the switch and executed if there is a match.\n"
		"\n"
		"         When comparing strings both the switch and case arguments must be\n"
		"         enclosed in quote characters.\n"
		"\n"
		"         If the 'default' command is found and no 'case' statement has been\n"
		"         matched the default command's argument is executed.\n"
		"\n"
		"<178>Example<278>: #switch {1d4} {#case 1 cackle;#case 2 smile;#default giggle}\n",

		"statements"
	},
	{
		"SYSTEM",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #system <178>{<278>command<178>}<278>\n"
		"\n"
		"         Executes the command specified as a shell command.\n",

		"detach script run"
	},
	{
		"TAB",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #tab <178>{<278>word<178>}<278>\n"
		"\n"
		"         Adds a word to the tab completion list, alphabetically sorted.\n"
		"\n"
		"         If no tabs are defined tintin will use the scrollback buffer\n"
		"         for auto tab completion.\n"
		"\n"
		"<178>Comment<278>: You can remove a tab with the #untab command.\n",

		"alias cursor history keypad macro speedwalk"
	},
	{
		"TEXTIN",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #textin <178>{<278>filename<178>} {<278>delay<178>}<278>\n"
		"\n"
		"         Textin allows the user to read in a file, and send its contents\n"
		"         directly to the server.  Useful for doing online creation, or message\n"
		"         writing.\n"
		"\n"
		"         The delay is in seconds and takes a floating point number which is\n"
		"         cumulatively applied to each outgoing line.\n",

		"scan send"
	},
	{
		"TICKER",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #ticker <178>{<278>name<178>} {<278>commands<178>} {<278>interval in seconds<178>}<278>\n"
		"\n"
		"         Executes given command every # of seconds.\n"
		"\n"
		"<178>Comment<278>: Tickers don't work in the startup session.\n"
		"\n"
		"<178>Comment<278>: You can remove a ticker with the #unticker command.\n",

		"delay event"
	},
	{
		"TIME",
		TOKEN_TYPE_STRING,
		"<178>Command<278>: #format <178>{<278>variable<178>} {<278>%t<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         The %t format specifier of the #format command allows printing dates\n"
		"         using the strftime() format specifiers. By default the time stamp used\n"
		"         is the current time, if you want to print a past or future date use:\n"
		"\n"
		"<178>Command<278>: #format <178>{<278>variable<178>} {<278>%t<178>} {{<278>argument<178>} <178>{<278>epoch time<178>}}<278>\n"
		"\n"
		"         The current epoch time value is obtained using #format {time} {%T}.\n"
		"\n"
		"         When using %t the argument should contain strftime format specifiers.\n"
		"         The output may differ depending on your locale.\n"
		"\n"
		"         %a  Abbreviated name of the day of the week (mon ... sun).\n"
		"         %A  Full name of the day of the week. (Monday ... Sunday)\n"
		"         %b  Abbreviated name of the month (Jan ... Dec)\n"
		"         %B  Full name of the month. (January ... December)\n"
		"         %C  2 digit numeric century. (19 ... 20)\n"
		"         %d  2 digit numeric day of the month (01 ... 31)\n"
		"         %H  2 digit numeric 24-hour clock hour. (00 ... 23)\n"
		"         %I  2 digit numeric 12-hour clock hour. (01 ... 12)\n"
		"         %j  3 digit numeric day of the year (001 ... 366)\n"
		"         %m  2 digit numeric month of the year (01 ... 12)\n"
		"         %M  2 digit numeric minute of the hour (00 ... 59)\n"
		"         %p  Abbreviated 12 hour clock period (AM ... PM)\n"
		"         %P  Abbreviated 12 hour clock period (am ... pm)\n"
		"         %S  2 digit numeric second of the minute (00 ...59\n"
		"         %u  1 digit numeric day of the week (1 ... 7)\n"
		"         %U  2 digit numeric Sunday week of the year (00 ... 53\n"
		"         %w  1 digit numeric day of the week (0 ... 6)\n"
		"         %W  2 digit numeric Monday week of the year (00 ... 53\n"
		"         %y  2 digit numeric year. (70 ... 38)\n"
		"         %Y  4 digit numeric year. (1970 ... 2038)\n"
		"         %z  5 digit timezone offset. (-1200 ... +1400)\n"
		"         %Z  Abbreviated name of the time zone.\n",

		"echo event format"
	},
	{
		"VARIABLE",
		TOKEN_TYPE_CONFIG,
		"<178>Command<278>: #variable <178>{<278>variable name<178>} {<278>text to fill variable<178>}<278>\n"
		"\n"
		"         Variables differ from the %0-99 arguments in the fact that you can\n"
		"         specify a full word as a variable, and they stay in memory for the\n"
		"         full session unless they are changed.  They can be saved in the\n"
		"         coms file, and can be set to different values if you have two or\n"
		"         more sessions running at the same time.  Variables are global for\n"
		"         each session and can be accessed by adding a $ before the variable\n"
		"         name.\n"
		"\n"
		"<178>Example<278>: #alias {target} {#var target %0}\n"
		"         #alias {x}      {kick $target}\n"
		"\n"
		"         The name of a variable must exist of only letters, numbers and\n"
		"         underscores in order to be substituted.  If you do not meet these\n"
		"         requirements do not panic, simply encapsulate the variable in braces:\n"
		"\n"
		"<178>Example<278>: #variable {cool website} {http://tintin.sourceforge.net}\n"
		"         #chat I was on ${cool website} yesterday!.\n"
		"\n"
		"         Variables can be escaped by adding additional $ signs.\n"
		"\n"
		"<178>Example<278>: #var test 42;#showme $$test\n"
		"\n"
		"         Variables can be nested using brackets:\n"
		"\n"
		"<178>Example<278>: #var hp[self] 34;#var hp[target] 46\n"
		"\n"
		"         You can see the first nest of a variable using $variable[+1] and the\n"
		"         last nest using $variable[-1]. Using $variable[-2] will report the\n"
		"         second last variable, and so on. To show all indices use *variable[].\n"
		"         To show all values use $variable[%*] or a less generic regex. To show\n"
		"         all values from index 2 through 4 use $variable[+2..4].\n"
		"\n"
		"         Nested variables are also known as tables, table generally being used\n"
		"         to refer to several variables nested within one specific variable.\n"
		"\n"
		"<178>Example<278>: #show {Targets starting with the letter A: $targets[A%*]\n"
		"\n"
		"         To see the internal index of a variable use &<variable name>. To see\n"
		"         the size of a table you would use: &targets[] or &targets[%*]. A non\n"
		"         existent nested variable will report itself as 0.\n"
		"\n" 
		"<178>Example<278>: #show {Number of targets starting with A: &targets[A%*]\n"
		"\n"
		"         In some scripts you need to know the name of a nested variable. This\n"
		"         is also known as the key, and you can get it using *variable. For\n"
		"         example *target[+1]. To get the first variable's name use *{+1}.\n"
		"\n"
		"         It's also possible to declare a table using brace notation. Using\n"
		"         #var hp[self] 34 is the equivalent of #var {hp} {{self}{34}}. This\n"
		"         also allows merging tables. #var hp[self] 34;#var hp[target] 46 is\n"
		"         the equivalent of #var {hp} {{self}{34} {target}{46}} as well as\n"
		"         #var {hp} {{self}{34}} {{target}{46}} or if you want to get creative\n"
		"         the equivalent of #var hp[self] 34;#var {hp} {$hp} {{target}{46}}.\n"
		"\n"
		"<178>Comment<278>: You can remove a variable with the #unvariable command.\n",

		"cat format function local math replace script"
	},
	{
		"WHILE",
		TOKEN_TYPE_STATEMENT,
		"<178>Command<278>: #while <178>{<278>conditional<178>} {<278>commands<178>}<278>\n"
		"\n"
		"         This command works similar to a 'while' statement in other languages.\n"
		"\n"
		"         When a 'while' command is encourated, the conditional is evaluated,\n"
		"         and if TRUE (any non-zero result) the commands are executed. The\n"
		"         'while' loop will be repeated indefinitely until the conditional is\n"
		"         FALSE or the #BREAK or #RETURN commands are found.\n"
		"\n"
		"         The 'while' statement is only evaluated if it is read, so you must\n"
		"         nest it inside a trigger, like an alias or action.\n"
		"\n"
		"         The conditional is evaluated exactly the same as in the 'math' command.\n"
                "\n"
                "<178>Example<278>: #math cnt 0;#while {$cnt < 20} {#math cnt $cnt + 1;say $cnt}\n"
		"\n"
		"<178>Comment<278>: See '#help math', for more information.\n",

		"statements"
	},
                                                                                                   
	{
		"WRITE",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #write <178>{<278><filename><178>} {<278>[FORCE]<178>}<278>\n"
		"\n"
		"         Writes all current actions, aliases, subs, highlights, and variables\n"
		"         to a command file, specified by filename.\n"
		"\n"
		"         By default you cannot write to .map files to prevent accidentally\n"
		"         overwriting a map file. Use the FORCE argument to ignore this\n"
		"         protection.\n",

		"log read scan textin"
	},
	{
		"ZAP",
		TOKEN_TYPE_COMMAND,
		"<178>Command<278>: #zap {[session]}\n"
		"\n"
		"         Kill your current session.  If there is no current session, it will\n"
		"         cause the program to terminate. If you provide an argument it'll zap\n"
		"         the given session instead.\n",

		"all port run session sessionname snoop ssl"
	},
	{
		"",
		TOKEN_TYPE_COMMAND,
		"",

		""
	}
};
