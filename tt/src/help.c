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
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"

struct help_type
{
	char                  * name;
	char                  * text;
	char                  * also;
};

struct help_type help_table[];

char *help_related(struct session *ses, int index, int html)
{
	char *arg;
	char tmp[BUFFER_SIZE], link[BUFFER_SIZE];
	static char buf[BUFFER_SIZE];

	arg = help_table[index].also;

	buf[0] = 0;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, tmp, GET_ONE);

		if (html)
		{
			sprintf(link, "\\c<a href='%s.php'\\c>%s\\c</a\\c>", tmp, tmp);
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
				cat_sprintf(buf, " and %s.");
			}
		}
	}
	return buf;
}

DO_COMMAND(do_help)
{
	char arg1[BUFFER_SIZE], buf[BUFFER_SIZE];
	int cnt, found;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	if (*arg1 == 0)
	{
		*buf = 0;

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			if (strlen(buf) + 19 > gtd->screen->cols)
			{
				show_lines(ses, SUB_COL, "<088>%s<088>\n", buf);

				*buf = 0;
			}
			cat_sprintf(buf, "%19s ", help_table[cnt].name);
		}

		if (*buf)
		{
			show_lines(ses, SUB_COL, "<088>%s<088>\n", buf);
		}
	}
	else if (!strcasecmp(arg1, "dump"))
	{
		FILE *logfile = fopen("../docs/help.html", "w");

		do_configure(ses, "{log} {html}");

		if (HAS_BIT(ses->flags, SES_FLAG_LOGHTML))
		{
			write_html_header(ses, logfile);
		}

		*buf = 0;

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
//			printf("debug: %s\n", help_table[cnt].name);

			if (cnt && cnt % 4 == 0)
			{
				substitute(ses, buf, buf, SUB_ESC|SUB_COL);

				logit(ses, buf, logfile, LOG_FLAG_LINEFEED);

				*buf = 0;
			}
			cat_sprintf(buf, "     \\c<a href='#%s'\\c>%15s\\c</a\\c>", help_table[cnt].name, help_table[cnt].name);
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
				substitute(ses, help_related(ses, cnt, 0), buf, SUB_COL);

				logit(ses, buf, logfile, LOG_FLAG_LINEFEED);
			}
		}
		fclose(logfile);
	}
	else if (!strcasecmp(arg1, "dump.php"))
	{
		FILE *logfile;

		do_configure(ses, "{log} {html}");

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
				show_lines(ses, SUB_COL, "%s<088>\n", help_table[cnt].text);
				
				if (*help_table[cnt].also)
				{
					show_lines(ses, SUB_COL, "%s<088>\n\n", help_related(ses, cnt, 0));
				}
				return ses;
			}
		}
		found = FALSE;

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			if (match(ses, help_table[cnt].name, arg1, SUB_VAR|SUB_FUN))
			{
				show_lines(ses, SUB_COL, "%s<088>\n", help_table[cnt].text);

				if (*help_table[cnt].also)
				{
					show_lines(ses, SUB_COL, "%s<088>\n\n", help_related(ses, cnt, 0));
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
		"         Actions can be triggered by the showme command and certain system\n"
		"         messages.\n"
		"\n"
		"         Actions can be triggered by the #showme command. If you don't want a\n"
		"         #showme to get triggered use: #line ignore #showme {text}\n"
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

		"<178>Command<278>: #alias <178>{<278>name<178>} {<278>commands<178>}<278>\n"
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
		"<178>Example<278>: #alias {%*} {#showme You wrote: %0}\n"
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
		"<178>Command<278>: #bell\n"
		"\n"
		"         The #bell command will ring the terminal bell.\n"
		"\n"
		"<178>Example<278>: #action {Bubba tells you} {#bell}\n"
		"\n"
		"         If you aren't watching the screen this could be useful if you don't\n"
		"         want to miss out on a conversation with Bubba. Alternatively you can\n"
		"         use #system to play a sound file.\n"
		"\n"
		"         Some terminals will allow you to use VT100 Operating System Commands\n"
		"         to change the terminal's title which can be used as a visual alert.\n"
		"\n"
		"<178>Example<278>: #action {Bubba tells you} {#screen save title;#screen set title Tell!;\n"
		"           #bell;#delay 10 #screen load title}\n"
		"\n"
		"         The above example will save your window title, change the title to\n"
		"         'Tell!', next reset the window title after 10 seconds.\n"
		"\n"
		"         It's possible to set the terminal to pop to the foreground upon\n"
		"         ringing of the alarm bell.\n"
		"\n"
		"<178>Example<278>: #showme {pop up alarm: \\e[?1043h\\a\\e[?1043l}\n"
		"\n"
		"         It's possible to adjust the alarm bell volume on some terminals.\n"
		"\n"
		"<178>Example<278>: #loop {1} {8} {cnt} {#line substitute variables\n"
		"           #delay {$cnt} #showme {Volume $cnt: \\e[$cnt t};#bell}\n",

		"screen"
	},
	{
		"BREAK",

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
		"CASE",

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
		"CHARACTERS",

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
		"{ }      Curly brackets aka braces are used for seperating multi word command\n"
		"         arguments, nesting commands, and nesting variables. Braces cannot\n"
		"         easily be escaped and must always be used in pairs.\n"
		"\n"
		"\" \"      Quote characters are used for strings in the #math, #if, #switch,\n"
		"         and #case commands. It is however suggested to use an extra\n"
		"         set of braces { } to define strings.\n"
		"\n"
		"!        The exclamation sign is used to repeat commands, see #help history.\n"
		"         The character can be redefined using #config.\n"
		"\n"
		"\\        An input line starting with a backslash is send verbatim if you are\n"
		"         connected to a server. This character can be configured with\n"
		"         #config.\n",

		"colors escape mathematics pcre"
	},
	{
		"CHAT",

		"<178>Command<278>: #chat <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         #chat {init}       {port}             Initilizes a chat port.\n"
		"         #chat {name}       {name}             Sets your chat name.\n"
		"         #chat {message}    {buddy|all} {text} Sends a chat message\n"
		"\n"
		"         #chat {accept}     {buddy} {boost}    Accept a file transfer\n"
		"         #chat {call}       {address} {port}   Connect to a buddy\n"
		"         #chat {cancel}     {buddy}            Cancel a file transfer\n"
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
		"         #chat {who}                           Show all connections\n"
		"         #chat {zap}        {buddy}            Close a connection\n",
		
		"port"
	},
	{
		"CLASS",

		"<178>Command<278>: #class <178>{<278>name<178>} {<278>open<178>|<278>close<178>|<278>list<178>|<278>read<178>|<278>size<178>|<278>write<178>|<278>kill<178>} {<278>arg<178>}<278>\n"
		"\n"
		"         The {open} option will open a class, closing a previously opened\n"
		"         class. All triggers added afterwards are assigned to this class.\n"
		"         The {close} option will close the given class.\n"
		"         The {list} option will show the given list of the class.\n"
		"         The {read} option will open the class, read, and close afterwards.\n"
		"         The {size} option will store the size of the class in a variable.\n"
		"         The {write} option will write all triggers of the given class to file.\n"
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

		"<178>Syntax<278>:  <<888>xyz>  with x, y, z being parameters\n"
		"\n"
		"         Parameter 'x': VT100 code\n"
		"\n"
		"         0 - Reset all colors and codes to default\n"
		"         1 - Bold\n"
		"         2 - Dim\n"
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
		"<178>Example<278>: #showme <<888>acf>Azure    <<888>afc>Jade     <<888>caf>Violet\n"
		"<178>Example<278>: #showme <<888>cfa>Lime     <<888>fac>Pink     <<888>fca>Orange\n"
		"\n"
		"         For 12 bit truecolor use <<888>F000> to <<888>FFFF> for foreground colors and\n"
		"         <<888>B000> to <<888>BFFF> for background colors.\n"
		"\n"
		"         For 24 bit truecolor use \\e[38;2;R;G;Bm where R G B are red/green/blue\n"
		"         intensities between 0 and 255. For example: \\e[37;2;50;100;150m. Use\n"
		"         \\e[48;2;R;G;Bm for background colors.\n",
		
		"characters coordinates escape mathematics pcre"
	},
	{
		"COORDINATES",

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

		"<178>Command<278>: #config <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         This allows you to configure various settings, the settings can be\n"
		"         written to file with the #write or #writesession command.\n"
		"\n"
		"         If you configure the global session (the one you see as you start up\n"
		"         tintin) all sessions started will inherite these settings.\n"
		"\n"
		"         It's advised to make a configuration file to read on startup if you\n"
		"         do not like the default settings.\n"
		"\n"
		"         Config options which aren't listed by default:\n"
		"\n"
		"         #CONFIG {CHILD LOCK}   {ON|OFF} Enable or disable command input.\n"
		"         #CONFIG {CONVERT META} {ON|OFF} Shows color codes and key bindings.\n"
		"         #CONFIG {DEBUG TELNET} {ON|OFF} Shows telnet negotiations y/n.\n"
		"         #CONFIG {LOG LEVEL}  {LOW|HIGH} LOW logs mud output before triggers.\n"
		"         #CONFIG {INHERITANCE}  {ON|OFF} Session trigger inheritance y/n.\n"
		"         #CONFIG {MCCP}         {ON|OFF} Enable or disable MCCP support.\n",
		
		"class line"
	},
	{
		"CONTINUE",

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

		"<178>Command<278>: #cursor <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         Typing #cursor without an option will show all available cursor\n"
		"         options, their default binding, and an explanation of their function.\n"
		"\n"
		"         The cursor command's primarly goal is adding customizable input editing\n"
		"         with macros. Subsequently many cursor commands only work properly when\n"
		"         used within a macro or event.\n",
		
		"alias history keypad macro speedwalk tab"
	},
	{
		"DEBUG",

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

		"<178>Command<278>: #default <178>{<278>commands<178>}<278>\n"
		"\n"
		"         The default command can only be used within the switch command. When\n"
		"         the conditional argument of non of the case commands matches the switch\n"
		"         command's conditional statement the default command is executed.\n",
		
		"case default else elseif if switch regex"
	},
	{
		"DELAY",

		"<178>Command<278>: #delay <178>{<278>seconds<178>} {<278>command<178>}<278>\n"
		"<178>Command<278>: #delay <178>{<278>name<178>} {<278>command<178>} {<278>seconds<178>}<278> \n"
		"\n"
		"         Delay allows you to have tintin wait the given amount of seconds\n"
		"         before executing the given command. tintin won't wait before\n"
		"         executing following input commands if any.\n"
		"\n"
		"         Floating point precision for miliseconds is possible.\n"
		"\n"
		"<178>Example<278>: #showme first;#delay {1} {#showme last}\n"
		"         This will print 'first', and 'last' around one second later.\n"
		"\n"
		"<178>Comment<278>: If you want to remove a delay with the #undelay command you can add\n"
		"         a name as the first argument, be aware this changes the syntax. If\n"
		"         the name is a number keep in mind that delays with the same numeric\n"
		"         name will not be overwritten\n",
		
		"event ticker"
	},
	{
		"ECHO",

		"<178>Command<278>: #echo <178>{<278>format<178>} {<278>argument1<178>} {<278>argument2<178>} {<278>etc<178>}<278>\n"
		"\n"
		"         Echo command displays text on the screen with formatting options. See\n"
		"         the help file for the format command for more informations.\n"
		"\n"
		"         The echo command does not trigger actions.\n"
		"\n"
		"         As with the #showme command you can split the {format} argument up into\n"
		"         two braced arguments, in which case the 2nd argument is the row number.\n"
		"\n"
		"<178>Example<278>: #echo {The current date is %t.} {%Y-%m-%d %H:%M:%S}\n"
		"         #echo {[%38s][%-38s]} {Hello World} {Hello World}\n"
		"         #echo {{this is %s on the top row} {-1}} {printed}\n",
		
		"buffer grep showme"
	},
	{
		"ELSE",

		"<178>Command<278>: #else <178>{<278>commands<178>}<278>\n"
		"\n"
		"         The else statement should follow an #IF or #ELSEIF statement and is\n"
		"         only called if the proceeding #IF or #ELSEIF is false.\n"
		"\n"
		"<178>Example<278>: #if {1d2 == 1} {smile};#else {grin}\n",
		
		"case default elseif if switch regex"
	},
	{
		"ELSEIF",

		"<178>Command<278>: #elseif <178>{<278>conditional<178>} {<278>commands<178>}<278>\n"
		"\n"
		"         The elseif statement should follow an #IF or #ELSEIF statement and is\n"
		"         only called when the statement is true and the proceeding #IF and\n"
		"         #ELSEIF statements are false.\n"
		"\n"
		"<178>Example<278>: #if {1d3 == 1} {smirk};#elseif {1d2 == 1} {snicker}\n",
		
		"case default else if switch regex"
	},
	{
		"END",

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

		"<278>         You may use the escape character \\ for various special characters.\n"
		"\n"
		"         \\a    beep the terminal.\n"
		"         \\c    send a control character, \\ca for ctrl-a.\n"
		"         \\e    start an escape sequence.\n"
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

		"<178>Command<278>: #event <178>{<278>event type<178>}<278>\n"
		"\n"
		"         Events allow you to create triggers for predetermined client events.\n"
		"\n"
		"         Use #event without an argument to see a list of possible events with\n"
		"         a brief description. Use #event %* to see the current list of defined\n"
		"         events. Use #info {events} {on} to see events get thrown.\n"
		"\n"
		"         Some events can be prefixed with CATCH to interrupt default behavior.\n"
		"\n"
		"         CATCH <EVENT>\n"
		"         CHAT MESSAGE          %0 default %1 plain\n"
		"         CLASS ACTIVATED       %0 class name\n"
		"         CLASS DEACTIVATED     %0 class name\n"
		"         DATE                  %1 month - %3 day   %4 hour : %5 minute\n"
		"         DAY <DAY>             %3 day of the month\n"
		"         DOUBLE-CLICKED <VAR>  %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         END OF PATH\n"
		"         HOUR                  %4 hour\n"
		"         IAC <VAR> <VAR>\n"
		"         IAC SB GMCP <MODULE>  %0 data     %1 raw data\n"
		"         IAC SB MSSP           %0 variable %1 value\n"
		"         IAC SB MSDP           %0 variable %1 value\n"
		"         IAC SB MSDP <VAR>     %1 value\n"
		"         IAC SB NEW-ENVIRON    %0 variable %1 value\n"
		"         IAC SB ZMP <VAR>      %0 value\n"
		"         IAC SB <VAR>          %0 raw text %1 raw data\n"
		"         LONG-CLICKED <VAR>    %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         MAP ENTER MAP         %0 new vnum\n"
		"         MAP ENTER ROOM        %0 new vnum %1 old vnum\n"
		"         MAP ENTER ROOM <VAR>  %0 new vnum %1 old vnum\n"
		"         MAP EXIT MAP          %0 old vnum\n"
		"         MAP EXIT ROOM         %0 old vnum %1 new vnum\n"
		"         MAP EXIT ROOM <VAR>   %0 old vnum %1 new vnum\n"
		"         MAP FOLLOW MAP        %0 old vnum %1 new vnum %2 exit name\n"
		"         MAP UPDATED VTMAP\n"
		"         MINUTE                %5 minute\n"
		"         MONTH                 %1 month\n"
		"         MOVED <VAR>           %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         PORT CONNECTION       %0 name %1 ip %2 port\n"
		"         PORT DISCONNECTION    %0 name %1 ip %2 port\n"
		"         PORT MESSAGE          %0 data %1 plain data\n"
		"         PORT LOG MESSAGE      %0 name %1 ip %2 port %3 data %4 plain data\n"
		"         PORT RECEIVED MESSAGE %0 name %1 ip %2 port %3 data %4 plain data\n"
		"         PRESSED <VAR>         %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         PROGRAM START         %0 startup arguments\n"
		"         PROGRAM TERMINATION   %0 goodbye message\n"
		"         RECEIVED INPUT        %0 raw text\n"
		"         RECEIVED KEYPRESS     %0 raw text %1 unicode index\n"
		"         RECEIVED LINE         %0 raw text %1 plain text\n"
		"         RECEIVED OUTPUT       %0 raw text\n"
		"         RECEIVED PROMPT       %0 raw text %1 plain text\n"
		"         RELEASED <VAR>        %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         SCAN CSV HEADER       %0 all args %1 arg1 %2 arg2 .. %99 arg99\n"
		"         SCAN CSV LINE         %0 all args %1 arg1 %2 arg3 .. %99 arg99\n"
		"         SCAN TSV HEADER       %0 all args %1 arg1 %2 arg3 .. %99 arg99\n"
		"         SCAN TSV LINE         %0 all args %1 arg1 %2 arg3 .. %99 arg99\n"
		"         SCREEN RESIZE         %0 rows %1 cols\n"
		"         SCROLLED <VAR>        %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         SECOND                %6 second\n"
		"         SEND OUTPUT           %0 raw text %1 size\n"
		"         SENT OUTPUT           %0 raw text %1 size\n"
		"         SESSION ACTIVATED     %0 name\n"
		"         SESSION CONNECTED     %0 name %1 host %2 ip %3 port\n"
		"         SESSION CREATED       %0 name %1 host %2 ip %3 port\n"
		"         SESSION DEACTIVATED   %0 name\n"
		"         SESSION DISCONNECTED  %0 name %1 host %2 ip %3 port\n"
		"         SESSION TIMED OUT     %0 name %1 host %2 ip %3 port\n"
		"         SHORT-CLICKED <VAR>   %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         SYSTEM ERROR          %0 name %1 system msg %2 error %3 error msg\n"
		"         TIME                  %4 hour : %5 minute : %6 second\n"
		"         TRIPLE-CLICKED <VAR>  %0 row %1 col %2 -row %3 -col %4 word %5 line\n"
		"         UNKNOWN COMMAND       %0 raw text\n"
		"         VARIABLE UPDATE <VAR> %0 name %1 value\n"
		"         VT100 SCROLL REGION   %0 top row %1 bot row\n"
		"         WEEK <DAY>            %2 day of the week\n"
		"         WINDOW FOCUS IN       %0 name\n"
		"         WINDOW FOCUS OUT      %0 name\n"
		"         YEAR                  %0 year\n"
		"\n"
		"<178>Example<278>: #event {SESSION CONNECTED} {#read mychar.tin}\n"
		"\n"
		"<178>Comment<278>: You can remove an event with the #unevent command.\n",
		
		"delay ticker"
	},
	{
		"FORALL",

		"<178>This command is obsolete, please use foreach instead.\n",

		"cr"
	},
	{
		"FOREACH",

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

		"<178>Command<278>: #format <178>{<278>variable<178>} {<278>format<178>} {<278>argument1<178>} {<278>argument2<178>} {<278>etc<178>}<278>\n"
		"\n"
		"         Allows you to store a string into a variable in the exact same way\n"
		"         C's sprintf works with a few enhancements and limitations such as\n"
		"         no integer operations and a maximum of 30 arguments.\n"
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
		"         #format {list} {%w}   {string}  store wordwrapped text in {list}\n"
		"                                         optional {{string}{width}} syntax\n"
		"         #format {test} {%x}      {hex}  print corresponding charset character\n"
		"         #format {test} {%A}     {char}  store corresponding character value\n"
		"         #format {cols} {%C}         {}  store the screen width in {cols}\n"
		"         #format {test} {%D}      {hex}  convert hex to decimal in {test}\n"
		"         #format {hash} {%H}   {string}  store a 64 bit string hash in {hash}\n"
		"         #format {test} {%L}   {string}  store the string length in {test}\n"
		"         #format {rows} {%R}         {}  store the screen height in {rows}\n"
		"         #format {name} {%S}         {}  store the session name in {name}\n"
		"         #format {time} {%T}         {}  store the epoch time in {time}\n"
		"         #format {time} {%U}         {}  store the micro epoch time in {time}\n"
		"         #format {test} {%X}      {dec}  convert dec to hexadecimal in {test}\n\n"
		"         #format {test} {%%}             a literal % character\n"
		"\n"
		"<178>Comment<278>: See #help TIME for help on the %t argument.\n",
		
		"echo function local math replace script time variable"
	},

	{
		"FUNCTION",

		"<178>Command<278>: #function <178>{<278>name<178>} {<278>operation<178>}<278>\n"
		"\n"
		"         Functions allow you to execute a script within a line of text, and\n"
		"         replace the function call with the line of text generated by the\n"
		"         function.\n"
		"\n"
		"         Be aware that each function should set the $result variable at the\n"
		"         end of the function, or call #return with the given result.\n"
		"\n"
		"         To use a function use the @ character before the function name.\n"
		"         The function arguments should be placed between braces behind the\n"
		"         function name with argument separated by semicolons.\n"
		"\n"
		"         The function itself can use the provided arguments which are stored\n"
		"         in %1 to %9, with %0 holding all arguments.\n"
		"\n"
		"<178>Example<278>: #function {rnd} {#math {result} {1 d (%2 - %1 + 1) + %1 - 1}}\n"
		"         #showme A random number between 100 and 200: @rnd{100;200}\n"
		"\n"
		"<178>Example<278>: #function gettime {#format result %t %H:%M}\n"
		"         #showme The current time is @gettime{}\n"
		"\n"
		"<178>Comment<278>: You can remove a function with the #unfunction command.\n",
		
		"format local math replace script variable"
	},
	{
		"GAG",

		"<178>Command<278>: #gag <178>{<278>string<178>}<278>\n"
		"\n"
		"         Removes any line that contains the string.\n"
		"\n"
		"<178>Comment<278>: See '#help action', for more information about triggers.\n"
		"\n"
		"<178>Comment<278>: You can remove a gag with the #ungag command.\n",
		
		"action highlight prompt substitute"
	},
	{
		"GREETING",

		"<268>      #<268>##################################################################<268>#\n"
		"<268>      #<278>                     T I N T I N + +   "CLIENT_VERSION"                   <268>#\n"
		"<268>      #<278>                                                                  <268>#\n"
		"<268>      #<278>                 <268>T<278>he K<268>i<278>cki<268>n<278> <268>T<278>ickin D<268>i<278>kuMUD Clie<268>n<278>t <268>                #\n"
		"<268>      #<278>                                                                  <268>#\n"
		"<268>      #<278>      Code by Peter Unold, Bill Reis, and Igor van den Hoven      <268>#\n"
		"<268>      #<268>##################################################################<268>#<288>\n",
		
		""
	},
	{
		"GREP",

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

		"<178>Command<278>: #help <178>{<278>subject<178>}<278>\n"
		"\n"
		"         Without an argument #help will list all available help subjects.\n"
		"\n"
		"         Using #help %* will display all help entries.\n",
		
		"debug ignore info message"
	},
	{
		"HIGHLIGHT",

		"<178>Command<278>: #highlight <178>{<278>string<178>} {<278>color names<178>}<278>\n"
		"\n"
		"         The highlight command is used to allow you to highlight strings of text\n"
		"         from the mud.  Available ANSI color names are:\n"
		"\n"
		"         reset, light, dark, underscore, blink, reverse\n"
		"\n"
		"         black, red, green, yellow, blue, magenta, cyan, white,\n"
		"         b black, b red, b green, b yellow, b blue, b magenta, b cyan, b white\n"
		"\n"
		"         Available XTERM 256 color names are:\n"
		"\n"
		"         azure, ebony, jade, lime, orange, pink, silver, tan, violet,\n"
		"         light azure, light ebony, light jade, light lime, light orange,\n"
		"         light pink, light silver, light tan, light violet.\n"
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
		"<178>Example<278>: #high {Valgar} {reverse}\n"
		"         Prints every occurrence of 'Valgar' in reverse video.\n"
		"\n"
		"<178>Example<278>: #high {^You %1} {bold cyan}\n"
		"         Boldfaces any line that starts with 'You' in cyan.\n"
		"\n"
		"<178>Example<278>: #high {Bubba} {red underscore blink}\n"
		"         Highlights the name Bubba as blinking, red, underscored text\n"
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
		"         search mode, or by issueing #cursor {history search}.\n"
		"\n"
		"         TinTin++ tries to bind the arrow up and down keys to scroll through\n"
		"         the history list by default. You can bind these with a macro yourself\n"
		"         using #cursor {history next} and #cursor {history prev}. Many #cursor\n"
		"         commands only work properly when bound with a macro.\n",
		
		"alias cursor keypad macro speedwalk tab"
	},
	{
		"IF",

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
		
		"case default else elseif switch regex"
	},
	{
		"IGNORE",

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

		"<278>"
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
		"<178>Example<278>: #help -- #help is a mud client command, and isn't send to the mud\n"
		"         server.\n"
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
		"         say Hello \\;) -- The escape character can esape 1 letter.\n"
		"         #config verbatim on -- Everything is send as is except '#' commands.\n"
		"<128>\n"
		"         Connecting to a mud\n"
		"<178>\n"
		"Command<278>: #session <178>{<278>session name<178>} {<278>mud address<178>} {<278>port<178>}<278>\n"
		"\n"
		"         Example: #session someone tintin.sourceforge.net 4321\n"
		"\n"
		"         You can have more than one session, in which case you can switch\n"
		"         between sessions typing #<session name>.\n"
		"\n"
		"         You can get a list of all sessions by typing: #session. The current\n"
		"         active session is marked with (active). Snooped sessions with\n"
		"         (snooped). MCCP sessions (mud client compression protocol) with\n"
		"         (mccp 2).\n"
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
		"         to substitute text from the mud with color you provide. This command\n"
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
		"         Every 60 seconds on a standard dikumud a so called tick occures. You\n"
		"         regenerate hp/mana/move faster if you're sleeping/resting during a\n"
		"         tick. So it's pretty nice to know when the next tick occures. TinTin++\n"
		"         helps you with that.\n"
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
		"         login to a mud (name, password etc..) and basically all kinds of\n"
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
		"         your screen where a map drawn using unicode characters is displayed. \n"
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

		""
	},
	{
		"INFO",

		"<178>Command<278>: #info <178>{<278>listname<178>} {<278>LIST<178>|<278>ON<178>|<278>OFF<178>|<278>SAVE<178>}<278>\n"
		"\n"
		"         Without an argument info displays the settings of every tintin list.\n"
		"\n"
		"         By providing the name of a list and the LIST option it shows all\n"
		"         triggers/variables associated with that list. With the SAVE option\n"
		"         This data is written to the info variable.\n"
		"\n"
		"         #info cpu will show information about tintin's cpu usage.\n"
		"         #info system will show some system information.\n",

		"class debug ignore kill message"
	},
	{
		"KEYPAD",

		"<278>When TinTin++ starts up it sends \\e= to the terminal to enable the terminal's\n"
		"application keypad mode, which can be disabled using #showme {\\e>}\n"
		"\n"
		"<178>     Configuration A            Configuration B            Configuration C<278>\n"
		"+-----+-----+-----+-----+  +-----+-----+-----+-----+  +-----+-----+-----+-----+\n"
		"|Num  |/    |*    |-    |  |Num  |/    |*    |-    |  |Num  |nkp/ |nkp* |nkp- |\n"
		"+-----+-----+-----+-----+  +-----+-----+-----+-----+  +-----+-----+-----+-----+\n"
		"|7    |8    |9    |     |  |Home |Up   |PgUp |     |  |nkp7 |nkp8 |nkp9 |     |\n"
		"+-----+-----+-----+     |  +-----+-----+-----+     |  +-----+-----+-----+     |\n"
		"|4    |5    |6    |+    |  |Left |Centr|Right|+    |  |nkp4 |nkp5 |nkp6 |nkp+ |\n"
		"+-----+-----+-----+-----+  +-----+-----+-----+-----+  +-----+-----+-----+-----+\n"
		"|1    |2    |3    |     |  |End  |Down |PgDn |     |  |nkp1 |nkp2 |nkp3 |     |\n"
		"+-----+-----+-----+     |  +-----+-----+-----+     |  +-----+-----+-----+     |\n"
		"|0          |.    |Enter|  |Ins        |Del  |Enter|  |nkp0       |nkp. |nkpEn|\n"
		"+-----------+-----+-----+  +-----------+-----+-----+  +-----------+-----+-----+\n"
		"\n"
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

		"<178>Command<278>: #line <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         #line log {filename} {[text]}          Log the current or given line to\n"
		"                                                file.\n"
		"\n"
		"         #line logverbatim {filename} {[text]}  Log text without variable\n"
		"                                                substitution.\n"
		"\n"
		"         #line gag                              Gag the next line.\n"
		"\n"
		"         #line ignore {argument}                Argument is executed without\n"
		"                                                any triggers being checked.\n"
		"\n"
		"         #line quiet {argument}                 Argument is executed with\n"
		"                                                suppression of system messages.\n"
		"\n"
		"         #line strip {argument}                 Strips the argument of color\n"
		"                                                codes next executes it as a\n"
		"                                                command.\n"
		"\n"
		"         #line substitute {options} {argument}  Substitutes the given options:\n"
		"                                                variables, functions, colors,\n"
		"                                                escapes, secure, in the given\n"
		"                                                argument next executes it as a\n"
		"                                                command.\n"
		"\n"
		"         #line verbatim {argument}              Argument is executed verbatim.\n"
		"\n"
		"         #line verbose {argument}               Argument is executed verbose.\n"
		"\n"
		"         When using #line log and logging in html format use \\c< \\c> \\c& \\c\" to\n"
		"         log a literal < > & and \".\n",

		"class config"
	},
	{
		"LIST",

		"<178>Command<278>: #list <178>{<278>variable<178>} {<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         #list {var} {add} {item}               Add {item} to the list\n"
		"         #list {var} {clear}                    Empty the given list\n"
		"         #list {var} {create} {item}            Create a list using {items}\n"
		"         #list {var} {delete} {index} {number}  Delete the item at {index},\n"
		"                                                the {number} is optional.\n"
		"         #list {var} {insert} {index} {string}  Insert {string} at given index\n"
		"         #list {var} {find} {string} {variable} Return the found index\n"
		"         #list {var} {get} {index} {variable}   Copy an item to {variable}\n"
		"         #list {var} {set} {index} {string}     Change the item at {index}\n"
		"         #list {var} {simplify} {variable}      Copy simple list to {variable}\n"
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
		"LOCAL",

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
		"<178>Example<278>: #alias {swap} {#local x %0;#replace x {e} {u};#showme $x}\n",

		"format function math replace script variable"
	},

	{
		"LOG",

		"<178>Command<278>: #log <178>{<278>append<178>|<278>overwrite<178>} {<278>filename<178>}<278>\n"
		"\n"
		"         Logs session to a file, you can set the data type to either plain,\n"
		"         raw, or html with the config command.\n",
		
		"read scan textin write"
	},

	{
		"LOOP",

		"<178>Command<278>: #loop <178>{<278><start><178>} {<278><finish><178>} {<278><variable><178>} {<278>commands<178>}<278>\n"
		"\n"
		"         Like a for statement, loop will loop from start to finish incrementing\n"
		"         or decrementing by 1 each time through.  The value of the loop counter\n"
		"         is stored in the provided variable, which you can use in the commands.\n"
		"\n"
		"<178>Example<278>: #loop 1 3 loop {get all $loop\\.corpse}\n"
		"         This equals 'get all 1.corpse;get all 2.corpse;get all 3.corpse'.\n"
		"\n"
		"         The . needs to be escaped so it's not treated as part of the variable.\n"
		"\n"
		"<178>Example<278>: #loop 3 1 cnt {drop $cnt\\.key}\n"
		"         This equals 'drop 3.key;drop 2.key;drop 1.key'.\n",

		"break continue foreach list parse repeat return while"
	},
	{
		"MACRO",

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
		"<178>Example<278>: #macro {(press ctrl-v)(press F1)} {#showme \\e[2J;#buffer lock}\n"
		"         Clear the screen and lock the window when you press F1, useful when the\n"
		"         boss is near.\n"
		"\n"
		"<178>Example<278>: #macro {\\eOM} {#cursor enter}\n"
		"         Makes the keypad's enter key work as an enter in keypad mode.\n"
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

		"<178>Command<278>: #map\n"
		"\n"
		"         The map command is the backbone of the auto mapping feature.\n"
		"\n"
		"         <178>#map at <location> <command>\n"
		"         <278>  Execute the command at the location.\n"
		"\n"
		"         <178>#map color <field> [value]\n"
		"         <278>  Sets the map color for the given color field.\n"
		"\n"
		"         <178>#map create <size>\n"
		"         <278>  Creates a new map and room 1. The default size is 50000 rooms.\n"
		"\n"
		"         <178>#map destroy\n"
		"         <278>  Deletes the map.\n"
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
		"         <178>#map exit <exit> <option> <arg>\n"
		"         <278>  Set the exit data. Useful with a closed door where you can\n"
		"         <278>  set the exit command: '#map exit e command {open east;e}'.\n"
		"         <278>  Use #map exit <exit> for a list of available options.\n"
		"\n"
		"         <178>#map exitflag <exit> <HIDE|AVOID> [on|off]\n"
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
		"         <278>  These options are also available to the at, delete, goto\n\n"
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
		"         <178>#map goto <room vnum> [dig]\n"
		"         <278>  Takes you to the given room vnum, with the\n"
		"         <278>  dig argument a new room will be created if non exists.\n"
		"\n"
		"         <178>#map goto <name> <exits> <desc> <area> <note> <terrain>\n"
		"         <278>  Takes you to\n"
		"         <278>  the given room name, if you provide exits those must match.\n"
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
		"         <178>#map info\n"
		"         <278>  Gives information about the map and room you are in.\n"
		"\n"
		"         <178>#map insert <direction> [roomflag]\n"
		"         <278>  Insert a room in the given\n"
		"         <278>  direction. Most useful for inserting void rooms.\n"
		"\n"
		"         <178>#map jump <x> <y> <z>\n"
		"         <278>  Jump to the given coordinate, which is relative\n"
		"         <278>  to your current room.\n"
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
		"         <278>  Lists all matching rooms and their distance.\n"
		"\n"
		"         <278>  Use {variable} {<variable>} to save the output to a variable.\n"
		"         <278>  {roomname} {<name>}, {roomarea} {<area>}, etc, are valid too.\n"
		"\n"
		"         <178>#map map <rows> <cols> <append|overwrite|list|variable> <name>\n"
		"         <278>  Display a drawing of the map of the given height and width.\n"
		"         <278>  All arguments are optional. If {rows} or {cols} are set to {}\n"
		"         <278>  or {0} they will use the scrolling window size as the default.\n"
		"         <278>  If {rows} or {cols} are a negative number this number is\n"
		"         <278>  subtracted from the scrolling window size.\n"
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
		"         <178>#map roomflag avoid\n"
		"         <278>  When set, '#map find' will avoid a route leading\n"
		"         <278>  through that room. Useful when you want to avoid death traps.\n"
		"         <178>#map roomflag hide\n"
		"         <278>  When set, '#map' will not display the map beyond\n"
		"         <278>  this room. When mapping overlapping areas or areas that aren't\n"
		"         <278>  build consistently you need this flag as well to stop\n"
		"         <278>  auto-linking, unless you use void rooms.\n"
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
		"         <278>  Calculates the shortest path to the\n"
		"         <278>  destination and walks you there. The delay is optional and\n"
		"         <278>  requires using braces. Besides the room name a list of\n"
		"         <278>  exits can be provided for more precise matching.\n"
		"\n"
		"         <178>#map set <option> <value> [vnum]\n"
		"         <278>  Set a map value for your current\n"
		"         <278>  room, or given room if a room vnum is provided.\n"
		"\n"
		"         <178>#map travel <direction> <delay>\n"
		"         <278>  Follows the direction until a dead end\n"
		"         <278>  or an intersection is found. Use braces around the direction\n"
		"         <278>  if you use the delay, which will add the given delay between\n"
		"         <278>  movements\n"
		"         <278>  Use #undelay PATH %* to abort delayed movement.\n"
		"\n"
		"         <178>#map undo\n"
		"         <278>  Will undo your last move. If this created a room or a link\n"
		"         <278>  they will be deleted, otherwise you'll simply move back a\n"
		"         <278>  room. Useful if you walked into a non existant direction.\n"
		"\n"
		"         <178>#map uninsert <direction>\n"
		"         <278>  Exact opposite of the insert command.\n"
		"\n"
		"         <178>#map unlink <direction> [both]\n"
		"         <278>  Will remove the exit, this isn't two\n"
		"         <278>  way so you can have the map properly display no exit rooms and\n"
		"         <278>  mazes.\n"
		"         <278>  If you use the both argument the exit is removed two-ways.\n"
		"\n"
		"         <178>#map update\n"
		"         <278>  Sets the vtmap to update within the next 0.1 seconds.\n"
		"\n"
		"         <178>#map vnum <low> [high]\n"
		"         <278>  Change the room vnum to the given number, if\n"
		"         <278>  a range is provided the first available room in that range\n"
		"         <278>  is selected.\n"
		"\n"
		"         <178>#map write <filename> [force]\n"
		"         <278>  Will save the map, if you want to save a map to a .tin file\n"
		"         <278>  you must provide the {force} argument.\n",

		"path pathdir"
	},
	{
		"MATH",

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
		"         >               4            logical greater than\n"
		"         >=              4            logical greater than or equal\n"
		"         <               4            logical less than\n"
		"         <=              4            logical less than or equal\n"
		"         ==              5            logical equal (can use regex)\n"
		"         !=              5            logical not equal (can use regex)\n"
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
		"         Time in seconds is calculated using [day]:[hour]:<minute>:<second>.\n"
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

		"format function local mathematics replace script variable"
	},
	{
		"MATHEMATICS",

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
		"Operator priority can be ignored by using paranthesis, for example (1 + 1) * 2\n"
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
		"         ==              5            alphabetical equal (can use regex)\n"
		"         !=              5            alphabetical not equal (can use regex)\n"
		"\n"
		"Strings must be encased in double quotes or braces. The > >= < <= operators\n"
		"perform basic string comparisons. The == != operators perform regular\n"
		"expressions, with the argument on the left being the string, and the argument\n"
		"on the right being the regex. For example {bla} == {%*a} would evaluate as 1.\n",

		"math"
	},

	{
		"MESSAGE",

		"<178>Command<278>: #message <178>{<278>listname<178>} {<278>on<178>|<278>off<178>}<278>\n"
		"\n"
		"         This will show the message status of all your lists if typed without an\n"
		"         argument. If you set for example VARIABLES to OFF you will no longer be\n"
		"         spammed when correctly using the #VARIABLE and #UNVARIABLE commands.\n",

		"class debug ignore info kill"
	},
	{
		"MSDP",

		"<278>\n"
		"         MSDP is part of the #port functionality. See #help event for\n"
		"         additional documentation as all MSDP events are available as\n"
		"         regular events.\n"
		"\n"
		"         Available MSDP events can be queried using the MSDP protocol\n"
		"         as described in the specification.\n"
		"<178>\n"
		"         https://tintin.sourceforge.io/protocols/msdp\n",

		"event port"
	},
	{
		"NOP",

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

		"<178>Command<278>: #parse <178>{<278>string<178>} {<278>variable<178>} {<278>commands<178>}<278>\n"
		"\n"
		"         Like the loop statement, parse will loop from start to finish through\n"
		"         the given string.  The value of the current character is stored in the\n"
		"         provided variable.\n"
		"\n"
		"<178>Example<278>: #parse {hello world} {char} {#showme $char}\n",

		"break continue foreach list loop repeat return while"
	},
	{
		"PATH",

		"<178>Command<278>: #path <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         create   Will clear the path and start path mapping.\n"
		"         delete   Will delete the last move of the path.\n"
		"         describe Describe the path and current position.\n"
		"         destroy  Will clear the path and stop path mapping.\n"
		"         goto     Go the the start, end, or given position index.\n"
		"         insert   Add the given argument to the path.\n"
		"         load     Load the given variable as the new path.\n"
		"         map      Display the map and the current position.\n"
		"         move     Move the position forward or backward. If a number is given\n"
		"                  the position is changed by the given number of steps.\n"
		"         run      Execute the current path, with an optional floating point\n"
		"                  delay in seconds as the second argument.\n"
		"         save     Save the path to a variable. You must specify whether you\n"
		"                  want to save the path 'forward' or 'backward'. If you use\n"
		"                  the 'length' or 'position' keywords the current length or\n"
		"                  position is saved.\n"
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
		"         braces { }, these braces are replaced with paranthesis ( ) unless you\n"
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
		"         <178>Paranthesis<278>\n"
		"\n"
		"         TinTin Regular Expressions automatically add parenthesis, for example\n"
		"         %* translates to (.*?) in PCRE unless the %* is found at the start or\n"
		"         end of the line, in which cases it translates to (.*). Paranthesis in\n"
		"         PCRE causes a change in execution priority similar to mathematical\n"
		"         expressions, but paranthesis also causes the match to be stored to a\n"
		"         variable.\n"
		"\n"
		"         When nesting multiple sets of paranthesis each nest is assigned its\n"
		"         numercial variable in order of appearance.\n"
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
		"Example: #regex {bli bla blo} {^{.*} {.*}$} {#showme Arg1=(&1) Arg2=(&2)}\n"
		"\n"
		"         This will display: Arg1=(bli bla) Arg2=(blo)\n"
		"\n"
		"         By appending a ? behind a regex it becomes lazy, meaning {.*?} will\n"
		"         capture as little text as possible.\n"
		"\n"
		"Example: #regex {bli bla blo} {^{.*?} {.*?}$} {#showme Arg1=(&1) Arg2=(&2)}\n"
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
		"         (tilda). To make escape codes visible use #config {convert meta} on.\n"
		"\n"
		"Example: #action {~\\e[1;37m%1} {#var roomname %1}\n"
		"\n"
		"         If the room name is the only line on the mud in bright white this\n"
		"         color trigger will save the roomname.\n"
		"\n"
		"\n"
		"         This covers the basics. PCRE has more options, most of which are\n"
		"         somewhat obscure, so you'll have to read a PCRE manual for additional\n"
		"         information.\n",

		"map path"
	},

	{
		"PORT",

		"<178>Command<278>: #port <178>{<278>option<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         #port {init} {name} {port} {file}     Initilize a port session.\n"
		"\n"
		"         #port {call}       {address} {port}   Connect to a remote socket\n"
		"         #port {color}      {color names}      Set the default color\n"
		"         #port {dnd}                           Decline new connections\n"
		"         #port {group}      {name} {group}     Assign a socket group\n"
		"         #port {ignore}     {name}             Ignore a socket\n"
		"         #port {info}                          Display your info\n"
		"         #port {name}       {name}             Change socket name.\n"		
		"         #port {prefix}     {text}             Set prefix before each message.\n"		
		"         #port {send}       {name|all} {text}  Send data to socket\n"
		"         #port {uninitialize}                  Unitialize the port session.\n"
		"         #port {who}                           Show all connections\n"
		"         #port {zap}        {name}             Close a connection\n"
		"\n"
		"         The port command is very similar to chat except that it creates a\n"
		"         new session dedicated to receiving socket connections at the given\n"
		"         port number without built-in support for a communication protocol.\n",

		"all chat run session sessionname snoop ssl zap"
	},
	{
		"PROMPT",

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
		"         #prompt will write to the default split line, which is at row -2.\n"
		"\n"
		"         The col number is optional and can be used to set the column index.\n"
		"         A positive col number draws the given number of columns from the left,\n"
		"         while a negative col number draws from the right. If you leave the\n"
		"         column argument empty tintin will clear the row before printing at\n"
		"         the start of the row.\n"
		"\n"
		"         The #showme command takes a row and col argument as well so it's also\n"
		"         possible to place text on your split lines using #showme.\n"
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

		"<178>Command<278>: #regexp <178>{<278>string<178>} {<278>expression<178>} {<278>true<178>} {<278>false<178>}<278>\n"
		"\n"
		"         Compares the string to the given regular expression.\n"
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
		"      %w match zero to any number of word characters.\n"
		"      %W match zero to any number of non word characters.\n"
		"      %d match zero to any number of digits.\n"
		"      %D match zero to any number of non digits.\n"
		"      %s match zero to any number of spaces.\n"
		"      %S match zero to any number of non spaces.\n"
		"\n"
		"      %? match zero or one character.\n"
		"      %. match one character.\n"
		"      %+ match one to any number of characters.\n"
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
		"<178>Example<278>: #regexp {bli bla blo} {bli {.*} blo} {#showme &1}\n",

		"case default else elseif if switch"
	},

	{
		"REPEAT",

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

		"<178>Command<278>: #replace <178>{<278>variable<178>} {<278>oldtext<178>} {<278>newtext<178>}<278>\n"
		"\n"
		"         Searches the variable text replacing each occurance of 'oldtext' with\n"
		"         'newtext'.\n",

		"format function local math script variable"
	},
	{
		"RETURN",

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

		"<178>Command<278>: #run <178>{<278>name<178>} {<278>shell command<178>} {<278>file<178>}<278>\n"
		"\n"
		"         The run command works much like the system command except that it\n"
		"         runs the command in a pseudo terminal. The run command also creates\n"
		"         a session that treats the given shell command as a mud server. This\n"
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

		"<178>Command<278>: #scan <178>{<278>abort<178>|<278>csv<178><178>|<278>tsv<178><178>|<278>txt<178>} {<278>filename<178>}<278>\n"
		"\n"
		"         The scan txt <filename> command reads in a file and sends its content\n"
		"         to the screen as if it was send by a mud. After using scan you can use\n"
		"         page-up and down to view the file.\n"
		"\n"
		"         This command is useful to convert ansi color files to html or viewing\n"
		"         raw log files.\n"
		"\n"
		"         Actions, highlights, and substitutions will trigger as normal, and it\n"
		"         is possible to create an action to execute #scan abort to prematurely\n"
		"         stop the scan.\n"
		"\n"
		"         The scan csv <filename> command reads in a comma separated value file\n"
		"         without printing the content to the screen. Instead it triggers one of\n"
		"         two events.\n"
		"\n"
		"         The SCAN CSV HEADER event is triggered on the first line of the csv\n"
		"         file. The SCAN CSV LINE event is triggered on the second and subsequent\n"
		"         lines of the csv file. The %0 argument contains the entire line, with\n"
		"         %1 containing the first value, %2 the second value, etc, all the way up\n"
		"         to %99.\n"
		"\n"
		"         Values containing spaces must be surrounded with quotes, keep in mind\n"
		"         newlines within quotes are not supported. Use two quotes to print one\n"
		"         literal quote character.\n"
		"\n"
		"         The scan tsv <filename> command reads in a tab separated value file\n"
		"         without printing the content to the screen. Instead it triggers the\n"
		"         SCAN TSV HEADER event for the first line and SCAN TSV LINE for all\n"
		"         subsequent lines.\n",

		"read textin"
	},

	{
		"SCREEN",

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
		"         <178>#screen get <rows|cols|height|width> <var>\n"
		"         <278>  Get the rows/cols size in characters or height/width in pixels.\n"
		"\n"
		"         <178>#screen get <top_row|bot_row|top_split|bot_split> <var>\n"
		"         <278>  Get the top and bot row of the scrolling region or the height\n"
		"         <888>  of the top and bot split bars.\n"
		"\n"
		"         <178>#screen info\n"
		"         <278>  Debugging information.\n"
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
		"         <178>#screen set <both|label|title>\n"
		"         <278>  Set the title, label, or both. Only title works on Windows.\n",

		"bell"
	},

	{
		"SCREEN READER",

		"<178>Command<278>: #config <178>{<278>SCREEN READER<178>} {<278>ON|OFF<178>}<278>\n"
		"\n"
		"         Screen reader mode is enabled by using #config screen on. The main\n"
		"         purpose of the screen reader mode is to tell MUDs that a screen reader\n"
		"         is being used by using the MTTS standard. The MTTS specification is\n"
		"         available at:\n"
		"\n"
		"         http://tintin.sourceforge.net/protocols/mtts\n"
		"\n"
		"         With the screen reader mode enabled TinTin++ will try to remove visual\n"
		"         elements where possible.\n",

		"config"
	},

	{
		"SCRIPT",

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
		"<178>Example<278>: #script {ruby -e 'print \"#showme hello world\"'}\n"
		"<178>Example<278>: #script {python -c 'print \"#showme hello world\"'}\n"
		"<178>Example<278>: #script {php -r 'echo \"#showme hello world\"'}\n"
		"<178>Example<278>: #script {path} {pwd};#showme The path is $path[1].\n",

		"format function local math replace variable"
	},

	{
		"SEND",

		"<178>Command<278>: #send <178>{<278>text<178>}<278>\n"
		"\n"
		"         Sends the text directly to the MUD, useful if you want to start with an\n"
		"         escape code.\n",

		"textin"
	},
	{
		"SESSION",

		"<178>Command<278>: #session <178>{<278>name<178>} {<278>host<178>} {<278>port<178>} {<278>file<178>}<278>\n"
		"\n"
		"         Starts a telnet session with the given name, host, port, and optional\n"
		"         file name. The name can be anything you want, except the name of an\n"
		"         already existant session, a number, or the keywords '+', '-' and 'self'.\n"
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
		"         The startup session is named 'gts' and can be used for relog scripts. Do\n"
		"         keep in mind that tickers do not work in the startup session.\n"
		"\n"
		"<178>Example<278>: #event {SESSION DISCONNECTED} {#gts #delay 10 #ses %0 mymud.com 4321}\n",

		"all port run sessionname snoop ssl zap"
	},
	{
		"SHOWME",

		"<178>Command<278>: #showme <178>{<278>string<178>} {<278>row<178>} <178>{<278>col<178>}<278>\n"
		"\n"
		"         Display the string to the terminal, do not send to the mud.  Useful for\n"
		"         status, warnings, etc.  The {row} and col number are optional and work\n"
		"         the same way as the row number of the #prompt trigger.\n"
		"\n"
		"         Actions can be triggered by the showme command. If you want to avoid\n"
		"         this from happening use: #line ignore #showme {<string>}.\n"
		"\n"
		"<178>Example<278>: #tick {TICK} {#delay 50 #showme 10 SECONDS TO TICK!!!} {60}\n"
		"\n"
		"<178>Comment<278>: The #prompt helpfile contains more information on using the\n"
		"         option {row} and {col} arguments.\n",

		"buffer echo grep"
		
	},
	{
		"SNOOP",

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

		"         <278>Speedwalking allows you to type multiple directions not separated by\n"
		"         semicolons, and now it lets you prefix a direction with a number, to\n"
		"         signify how many times to go that direction. You can turn it on/off\n"
		"         with #config.\n"
		"\n"
		"<178>Example<278>: Without speedwalk, you have to type:\n"
		"         s;s;w;w;w;w;w;s;s;s;w;w;w;n;n;w\n"
		"         With speedwalk, you only have to type:\n"
		"         2s5w3s3w2nw\n",

		"alias cursor history keypad macro tab"
	},
	{
		"SPLIT",

		"<178>Command<278>: #split <178>{<278>top status bar height<178>} {<278>bottom status bar height<178>}<278>\n"
		"\n"
		"         This option requires for your terminal to support VT100 emulation.\n"
		"\n"
		"         #split allows the creation of an input line, a bottom status bar, a\n"
		"         top status bar, and a scrolling text region.\n"
		"\n"
		"         By default the bottom status bar is filled with dashes --- and\n"
		"         subsequently it is also known as the split line. The scrolling\n"
		"         text region is also known as the main screen and this is where\n"
		"         all incoming text is displayed by default.\n"
		"\n"
		"         If you use #split without an argument it will set the height of the\n"
		"         top status bar to 0 lines and the bottom status bar to 1 line.\n"
		"\n"
		"         If you use #split with one argument it will set the height of the top\n"
		"         status bar to 0 lines and the bottom status bar will be set to 1 line.\n"
		"\n"
		"         If you use two arguments the first argument is the height of the top\n"
		"         status bar and the second argument the height of the bottom status bar.\n"
		"\n"
		"         <178>--top status bar--------\n"
		"\n"
		"         <278>  scrolling text region\n"
		"\n"
		"         <178>--bottom status bar----------\n"
		"         <278>  input line\n"
		"\n"
		"<178>Example<278>: #split 0 0\n"
		"         If tintin has determined that you have a screen of 30 rows, it will\n"
		"         set the scroll text region line 1 to line 29. With this example you\n"
		"         will have no status bars, but you will have an input bar, meaning\n"
		"         that if there is incoming text it won't overwrite what you are typing.\n"
		"\n"
		"<178>Comment<278>: You can display text on the split line(s) with the #prompt and\n"
		"         #showme {line} {row} commands.\n"
		"\n"
		"<178>Comment<278>: You can remove split mode with the #unsplit command.\n",

		"echo prompt showme"
	},
	{
		"SSL",

		"<178>Command<278>: #ssl <178>{<278>name<178>} {<278>host<178>} {<278>port<178>} {<278>file<178>}\n"
		"\n"
		"         Starts a secure socket telnet session with the given name, host, port,\n"
		"         and optional file name.\n",

		"all port run sessionname snoop ssl zap"
	},
	{
		"STATEMENTS",

		"         TinTin++ knows the following statements.\n"
		"<278>\n"
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

		"break case continue default else elseif foreach if loop parse return switch while"
	},
	{
		"SUBSTITUTE",

		"<178>Command<278>: #substitute <178>{<278>text<178>} {<278>new text<178>}<278>\n"
		"\n"
		"         Allows you to replace original text from the mud with different text.\n"
		"         This is helpful for complex coloring and making things more readable.\n"
		"         The %1-%99 variables can be used to capture text and use it as part of\n"
		"         the new output, and the ^ char is valid to only check the beginning of\n"
		"         the line for the text specified.\n"
		"\n"
		"         If only one argument is given, all active substitutions that match the\n"
		"         strings are displayed.  The '%*' char is valid in this instance.  See\n"
		"         '#help regex', for advanced wildcard information.\n"
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

		"<178>Command<278>: #suspend\n"
		"\n"
		"         Temporarily suspends tintin and returns you to your shell.  The\n"
		"         effect of this command is exactly as if you had typed control-z.\n"
		"         To return to tintin, type 'fg' at the shell prompt.\n"
		"\n"
		"         While suspended your tintin sessions will freeze. To keep a\n"
		"         suspended session running use the screen utility program and\n"
		"         have it detach the session.\n",

		"end"
	},
	{
		"SWITCH",

		"<178>Command<278>: #switch <178>{<278>conditional<178>} {<278>arguments<178>}<278>\n"
		"\n"
		"         The switch command works similar to the switch statement in other\n"
		"         languages. When the 'switch' command is encountered its body is parsed\n"
		"         and each 'case' command found will be compared to the conditional\n"
		"         argument of the switch and executed if there is a match.\n"
		"\n"
		"         When comparing strings the switch and case arguments must be enclosed\n"
		"         in quote characters.\n"
		"\n"
		"         If the 'default' command is found and no 'case' statement has been\n"
		"         matched the default command's argument is executed.\n"
		"\n"
		"<178>Example<278>: #switch {1d4} {#case 1 cackle;#case 2 smile;#default giggle}\n",

		"statements"
	},
	{
		"SYSTEM",

		"<178>Command<278>: #system <178>{<278>command<178>}<278>\n"
		"\n"
		"         Executes the command specified as a shell command.\n",

		"script run"
	},
	{
		"TAB",

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

		"<178>Command<278>: #textin <178>{<278>filename<178>} {<278>delay<178>}<278>\n"
		"\n"
		"         Textin allows the user to read in a file, and send its contents\n"
		"         directly to the mud.  Useful for doing online creation, or message\n"
		"         writing.\n"
		"\n"
		"         The delay is in seconds and takes a floating point number which is\n"
		"         cumulatively applied to each outgoing line.\n",

		"scan send"
	},
	{
		"TICKER",

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

		"<178>Command<278>: #format <178>{<278>variable<178>} {<278>%t<178>} {<278>argument<178>}<278>\n"
		"\n"
		"         The %t format specifier of the #format command allows printing dates\n"
		"         using the strftime() format specifiers. By default the time stamp used\n"
		"         is the current time, if you want to print a past or future date use:\n"
		"\n"
		"<178>Command<278>: #format <178>{<278>variable<178>} {<278>%t<178>} {{<278>argument<178>} <178>{{<278>epoch time<178>}}<278>\n"
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

		"echo format"
	},
	{
		"VARIABLE",

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
		"         Variables can be nested using brackets or dots:\n"
		"\n"
		"<178>Example<278>: #var hp[self] 34;#var hp[target] 46\n"
		"\n"
		"         You can see the first nest of a variable using $variable[+1] and the\n"
		"         last nest using $variable[-1]. Using $variable[-2] will report the\n"
		"         second last variable, and so on. To show all indices use $variable[].\n"
		"         To show all values use $variable[%*] or a less generic regex.\n"
		"\n"
		"         Nested variables are also known as tables, table generally being used\n"
		"         to refer to several variables nested within one specific variable.\n"
		"\n"
		"<178>Example<278>: #showme {Targets starting with the letter A: $targets[A%*]\n"
		"\n"
		"         To see the internal index of a variable use &<variable name>. To see\n"
		"         the size of a table you would use: &targets[] or &targets[%*]. A non\n"
		"         existent nested variable will report itself as 0.\n"
		"\n" 
		"<178>Example<278>: #showme {Number of targets starting with A: &targets[A%*]\n"
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

		"format function local math replace script"
	},
	{
		"WHILE",

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

		"<178>Command<278>: #zap {[session]}\n"
		"\n"
		"         Kill your current session.  If there is no current session, it will\n"
		"         cause the program to terminate. If you provide an argument it'll zap\n"
		"         the given session instead.\n",

		"all port run session sessionname snoop ssl"
	},
	{
		"",
		"",
		""
	}
};
