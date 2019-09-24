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

#include <signal.h>
#include <sys/socket.h>

/*************** globals ******************/

struct session *gts;
struct tintin_data *gtd;

void pipe_handler(int signal)
{
	syserr_printf(gtd->ses, "pipe_handler");
}

/*
	when the screen size changes, take note of it
*/

void winch_handler(int signal)
{
	struct session *ses;

	init_terminal_size(gts);

	for (ses = gts->next ; ses ; ses = ses->next)
	{
		init_terminal_size(ses);

		if (HAS_BIT(ses->telopts, TELOPT_FLAG_NAWS))
		{
			client_send_sb_naws(ses, 0, NULL);
		}
	}
}


void abort_handler(int signal)
{
	syserr_fatal(signal, "abort_handler");
}

void interrupt_handler(int signal)
{
	if (gtd->ses->connect_retry > utime())
	{
		gtd->ses->connect_retry = 0;
	}
	else if (HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_SGA) && !HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_ECHO))
	{
		socket_printf(gtd->ses, 1, "%c", 4);
	}
	else
	{
		cursor_delete_or_exit(gtd->ses, "");
	}
}

void suspend_handler(int signal)
{
	printf("\e[r\e[%d;%dH", gtd->screen->rows, 1);

	fflush(NULL);

	reset_terminal(gtd->ses);

	kill(0, SIGSTOP);

	init_terminal(gtd->ses);

	dirty_screen(gtd->ses);

	tintin_puts(NULL, "#RETURNING BACK TO TINTIN++.");
}

void trap_handler(int signal)
{

	syserr_fatal(signal, "trap_handler");
}


/****************************************************************************/
/* main() - show title - setup signals - init lists - readcoms - mainloop() */
/****************************************************************************/


int main(int argc, char **argv)
{
	int c, i = 0, greeting = 0;
	char filename[256];
	char arg[BUFFER_SIZE];

	#ifdef SOCKS
		SOCKSinit(argv[0]);
	#endif

	if (signal(SIGTERM, trap_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGTERM");
	}

	if (signal(SIGSEGV, trap_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGSEGV");
	}

	if (signal(SIGHUP, trap_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGHUP");
	}

	if (signal(SIGABRT, abort_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGTERM");
	}
/*
	if (signal(SIGINT, interrupt_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGINT");
	}

	if (signal(SIGTSTP, suspend_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGSTOP");
	}
*/
	if (signal(SIGPIPE, pipe_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGPIPE");
	}

	if (signal(SIGWINCH, winch_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGWINCH");
	}

	for (c = 0 ; c < argc ; c++)
	{
		if (c)
		{
			cat_sprintf(arg, " %s", argv[c]);
		}
		{
			strcpy(arg, argv[0]);
		}
	}

	if (argc > 1)
	{
		while ((c = getopt(argc, argv, "a: e: G h r: s t: v")) != EOF)
		{
			switch (c)
			{
				case 'h':
					init_tintin(STARTUP_FLAG_NOGREETING|STARTUP_FLAG_NORESET);

					tintin_printf(NULL, "Usage: %s [OPTION]... [FILE]...", argv[0]);
					tintin_printf(NULL, "");
					tintin_printf(NULL, "  -a  Set argument for PROGRAM START event.");
					tintin_printf(NULL, "  -e  Execute given command.");
					tintin_printf(NULL, "  -G  Don't show the greeting screen.");
					tintin_printf(NULL, "  -h  This help section.");
					tintin_printf(NULL, "  -r  Read given file.");
					tintin_printf(NULL, "  -s  Enable screen reader mode.");
					tintin_printf(NULL, "  -t  Set given title.");
					tintin_printf(NULL, "  -v  Enable verbose mode.");

					reset_terminal(gtd->ses);
					exit(1);
					break;

				case 'G':
					SET_BIT(greeting, STARTUP_FLAG_NOGREETING);
					break;

				case 's':
					SET_BIT(greeting, STARTUP_FLAG_SCREENREADER);
					break;
			}
		}
	}

	init_tintin(greeting);

	sprintf(filename, "%s/%s", gtd->home, TINTIN_DIR);

	if (mkdir(filename, 0777) || errno == EEXIST)
	{
		sprintf(filename, "%s/%s/%s", gtd->home, TINTIN_DIR, HISTORY_FILE);

		if (access(filename, F_OK ) != -1)
		{
			history_read(gts, filename);
		}
	}

	RESTRING(gtd->vars[1], argv[0]);

	if (argc > 1)
	{
		optind = 1;

		RESTRING(gtd->vars[2], argv[1]);

		while ((c = getopt(argc, argv, "a: e: G h r: s t: v")) != EOF)
		{
			switch (c)
			{
				case 'a':
					RESTRING(gtd->vars[0], argv[2]);
					SET_BIT(greeting, STARTUP_FLAG_ARGUMENT);
					break;

				case 'e':
					gtd->quiet_level++;
					gtd->ses = script_driver(gtd->ses, LIST_COMMAND, optarg);
					gtd->quiet_level--;
					break;

				case 'G':
					break;

				case 'r':
					gtd->ses = do_read(gtd->ses, optarg);
					break;

				case 's':
					break;

				case 't':
					printf("\e]0;%s\007", optarg);
					break;

				case 'v':
					do_configure(gtd->ses, "{VERBOSE} {ON}");
					break;

				default:
					tintin_printf(NULL, "Unknown option '%c'.", c);
					break;
			}
		}

		if (argc > 2)
		{
			RESTRING(gtd->vars[3], argv[2]);

			for (i = 3 ; i <= optind ; i++)
			{
				RESTRING(gtd->vars[i+1], argv[i] ? argv[i] : "");
			}

			arg[0] = 0;

			for (i = optind + 1 ; i < argc ; i++)
			{
				if (*arg)
				{
					strcat(arg, " ");
				}
				strcat(arg, argv[i]);

				if (i < 100)
				{
					RESTRING(gtd->vars[i+1], argv[i]);
				}
			}

			if (!HAS_BIT(greeting, STARTUP_FLAG_ARGUMENT))
			{
				RESTRING(gtd->vars[0], arg);
			}
		}

		if (argv[optind] != NULL)
		{
			gtd->ses = do_read(gtd->ses, argv[optind]);
		}
	}

	check_all_events(gts, SUB_ARG, 0, 0, "PROGRAM START");

	mainloop();

	return 0;
}

void init_tintin(int greeting)
{
	int ref, index;

	gtd                 = (struct tintin_data *) calloc(1, sizeof(struct tintin_data));

	gtd->ses = gts      = (struct session *) calloc(1, sizeof(struct session));
	gtd->str_size       = sizeof(struct str_data);
	gtd->str_hash_size  = sizeof(struct str_hash_data);
	gtd->buf            = str_alloc(STRING_SIZE);
	gtd->out            = str_alloc(STRING_SIZE);

	for (index = 0 ; index < LIST_MAX ; index++)
	{
		gts->list[index] = init_list(gts, index, 32);
	}

	gts->name           = strdup("gts");
	gts->group          = strdup("");
	gts->session_host   = strdup("");
	gts->session_port   = strdup("");
	gts->cmd_color      = strdup("");
	gts->telopts        = TELOPT_FLAG_ECHO;
	gts->flags          = SES_FLAG_MCCP;
	gts->socket         = 1;
	gts->read_max       = 16384;
	gts->lognext_name   = strdup("");
	gts->logline_name   = strdup("");

	gtd->flags          = TINTIN_FLAG_INHERITANCE;

	gtd->mccp_len       = 10000;
	gtd->mccp_buf       = (unsigned char *) calloc(1, gtd->mccp_len);

	gtd->mud_output_max = 16384;
	gtd->mud_output_buf = (char *) calloc(1, gtd->mud_output_max);

	gtd->input_off      = 1;

	gtd->os             = strdup(getenv("OS") ? getenv("OS") : "UNKNOWN");
	gtd->home           = strdup(getenv("HOME") ? getenv("HOME") : "~/");
	gtd->lang           = strdup(getenv("LANG") ? getenv("LANG") : "UNKNOWN");
	gtd->term           = strdup(getenv("TERM") ? getenv("TERM") : "UNKNOWN");

	gtd->time           = time(NULL);
	gtd->calendar       = *localtime(&gtd->time);

	for (index = 0 ; index < 100 ; index++)
	{
		gtd->vars[index] = strdup("");
		gtd->cmds[index] = strdup("");
	}

	for (ref = 0 ; ref < 26 ; ref++)
	{
		for (index = 0 ; *command_table[index].name != 0 ; index++)
		{
			if (index && strcmp(command_table[index - 1].name, command_table[index].name) > 0)
			{
				printf("\e[1;31minit_tintin() unsorted command table %s vs %s.", command_table[index - 1].name, command_table[index].name);
			}

			if (*command_table[index].name == 'a' + ref)
			{
				gtd->command_ref[ref] = index;
				break;
			}
		}
	}

	for (index = 1 ; index ; index++)
	{
		if (*event_table[index].name == 0)
		{
			break;
		}

		if (strcmp(event_table[index - 1].name, event_table[index].name) > 0)
		{
			printf("\e[1;31minit_tintin() unsorted event table %s vs %s.", event_table[index - 1].name, event_table[index].name);

			break;
		}
	}
	init_terminal_size(gts);

	init_msdp_table();

	init_local(gts);

	printf("\e="); // set application keypad mode

	gtd->input_level++;

	do_configure(gts, "{AUTO TAB}         {5000}");
	do_configure(gts, "{BUFFER SIZE}     {20000}");
	do_configure(gts, "{COLOR MODE}       {AUTO}");
	do_configure(gts, "{COLOR PATCH}       {OFF}");
	do_configure(gts, "{COMMAND COLOR}   {<078>}");
	do_configure(gts, "{COMMAND ECHO}       {ON}");
	do_configure(gts, "{CONNECT RETRY}       {0}");
	do_configure(gts, "{CHARSET}          {AUTO}");
	do_configure(gts, "{HISTORY SIZE}     {1000}");
	do_configure(gts, "{LOG MODE}          {RAW}");
	do_configure(gts, "{MOUSE TRACKING}    {OFF}");
	do_configure(gts, "{PACKET PATCH}     {AUTO}");
	do_configure(gts, "{RANDOM SEED}      {AUTO}");
	do_configure(gts, "{REPEAT CHAR}         {!}");
	do_configure(gts, "{REPEAT ENTER}      {OFF}");

	if (HAS_BIT(greeting, STARTUP_FLAG_SCREENREADER))
	{
		do_configure(gts, "{SCREEN READER}     {ON}");
	}
	else
	{
		do_configure(gts, "{SCREEN READER}     {OFF}");
	}
	do_configure(gts, "{SCROLL LOCK}        {ON}");
	do_configure(gts, "{SPEEDWALK}         {OFF}");
	do_configure(gts, "{TELNET}             {ON}");
	do_configure(gts, "{TINTIN CHAR}         {#}");
	do_configure(gts, "{VERBATIM}          {OFF}");
	do_configure(gts, "{VERBATIM CHAR}      {\\}");
	do_configure(gts, "{VERBOSE}           {OFF}");
	do_configure(gts, "{WORDWRAP}           {ON}");

	gtd->input_level--;

	insert_node_list(gts->list[LIST_PATHDIR],  "n",  "s",  "1", "");
	insert_node_list(gts->list[LIST_PATHDIR],  "e",  "w",  "2", "");
	insert_node_list(gts->list[LIST_PATHDIR],  "s",  "n",  "4", "");
	insert_node_list(gts->list[LIST_PATHDIR],  "w",  "e",  "8", "");
	insert_node_list(gts->list[LIST_PATHDIR],  "u",  "d", "16", "");
	insert_node_list(gts->list[LIST_PATHDIR],  "d",  "u", "32", "");

	insert_node_list(gts->list[LIST_PATHDIR], "ne", "sw",  "3", "");
	insert_node_list(gts->list[LIST_PATHDIR], "nw", "se",  "9", "");
	insert_node_list(gts->list[LIST_PATHDIR], "se", "nw",  "6", "");
	insert_node_list(gts->list[LIST_PATHDIR], "sw", "ne", "12", "");

	init_terminal(gts);

	if (!HAS_BIT(greeting, STARTUP_FLAG_NORESET))
	{
		reset_screen(gts);
	}

	if (!HAS_BIT(greeting, STARTUP_FLAG_NOGREETING))
	{
		if (HAS_BIT(greeting, STARTUP_FLAG_SCREENREADER))
		{
			tintin_printf2(gts, "Welcome to TinTin Plus Plus. Don't know which MUD to play? How about the following MUD.");

			do_advertise(gts, "");

			tintin_printf2(gts, "You're using TinTin Plus Plus written by Peter Unold, Bill Reis, and Igor van den Hoven.", CLIENT_VERSION);

			tintin_printf2(gts, "For help and requests visit tintin.sourceforge.io/forum the captcha answer is 3671.");
		}
		else
		{
			do_advertise(gts, "");

			if (gtd->screen->cols >= 80)
			{
				do_help(gts, "GREETING");
			}
			else
			{
				tintin_printf2(gts,
					"\e[0;37mT I N T I N + +   %s"
					"\n\n\e[0;36mT\e[0;37mhe K\e[0;36mi\e[0;37mcki\e[0;36mn\e[0;37m \e[0;36mT\e[0;37mickin D\e[0;36mi\e[0;37mkuMUD Clie\e[0;36mn\e[0;37mt\n\n"
					"Code by Peter Unold, Bill Reis, and Igor van den Hoven\n",
					CLIENT_VERSION);
			}
		}
	}
}


void quitmsg(char *message)
{
	struct session *ses;
	static char crashed = FALSE;

	if (crashed++)
	{
		printf("quitmsg(crashed)\n");

		fflush(NULL);

		exit(-1);
	}

	SET_BIT(gtd->flags, TINTIN_FLAG_TERMINATE);

	while ((ses = gts->next) != NULL)
	{
		cleanup_session(ses);
	}

	if (gtd->chat)
	{
		chat_uninitialize("", "");
	}

	check_all_events(gts, SUB_ARG, 0, 1, "PROGRAM TERMINATION", message ? message : "");

	if (gtd->history_size)
	{
		char filename[BUFFER_SIZE];

		sprintf(filename, "%s/%s/%s", gtd->home, TINTIN_DIR, HISTORY_FILE);

		history_write(gts, filename);
	}

	reset_terminal(gts);

	reset_screen(gts);

	if (message == NULL || *message)
	{
		if (message)
		{
			printf("\n\e[0m%s", message);
		}
		printf("\nGoodbye from TinTin++\n\n");
	}
	fflush(NULL);

	exit(0);
}

void syserr_printf(struct session *ses, char *fmt, ...)
{
	char buf[BUFFER_SIZE], name[BUFFER_SIZE], *errstr;

	errstr = strerror(errno);
	
	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	if (ses)
	{
		sprintf(name, "(%s)", ses->name);
	}
	else
	{
		sprintf(name, "(null)");
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "SYSTEM ERROR", name, buf, ntos(errno), errstr);

	if (!check_all_events(ses, SUB_ARG|SUB_SEC, 0, 0, "CATCH SYSTEM ERROR", name, buf, ntos(errno), errstr))
	{
		if (gts)
		{
			tintin_printf2(gts, "#SYSTEM ERROR %s %s (%d: %s)\e[0m", name, buf, errno, errstr);
		}

		if (ses && ses != gts)
		{
			tintin_printf2(ses, "#SYSTEM ERROR: %s %s (%d: %s)\e[0m", name, buf, errno, errstr);		
		}

		if (ses && gtd->ses != ses && gtd->ses != gts)
		{
			tintin_printf2(gtd->ses, "#SYSTEM ERROR: %s %s (%d: %s)\e[0m", name, buf, errno, errstr);
		}
	}
}

void syserr_signal(int signal, char *msg)
{
	char buf[256];
	static char crashed = FALSE;

	if (crashed++)
	{
		printf("syserr_signal(crashed)\n");

		fflush(NULL);

		exit(-1);
	}

	reset_terminal(gts);

	reset_screen(gts);

	dump_stack_fatal();

	sprintf(buf, "\e[1;31mFATAL SIGNAL FROM (%s): %s\e[0m\n", msg, strsignal(signal));

	printf("%s", buf);

	fflush(NULL);

	exit(0);
}

void syserr_fatal(int signal, char *msg)
{
	char buf[256], errstr[128];
	static char crashed = FALSE;

	if (crashed++)
	{
		printf("syserr_fatal(crashed)");

		fflush(NULL);

		exit(-1);
	}

	if (signal <= 0)
	{
		sprintf(errstr, "(error %d: %s", errno, strerror(errno));
	}
	else
	{
		sprintf(errstr, "(signal %d: %s)", signal, strsignal(signal));
	}

	if (gtd->quiet_level)
	{
		gtd->quiet_level = 0;
	}

	reset_terminal(gts);

	reset_screen(gts);

	dump_stack_fatal();

	sprintf(buf, "\n\e[1;31mFATAL ERROR \e[1;32m%s %s\e[0m\n", msg, errstr);

	printf("%s", buf);

	fflush(NULL);

	exit(0);
}
