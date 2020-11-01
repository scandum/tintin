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
*                    recoded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"

#include <signal.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/*************** globals ******************/

struct session *gts;
struct tintin_data *gtd;

void pipe_handler(int signal)
{
	syserr_printf(gtd->ses, "pipe_handler");
}

void xfsz_handler(int signal)
{
	syserr_printf(gtd->ses, "xfsz_handler");
}

void hub_handler(int signal)
{
	syserr_printf(gtd->ses, "hub_handler");
}

void ttin_handler(int signal)
{
	syserr_printf(gtd->ses, "ttin_handler");
}

void ttou_handler(int signal)
{
	syserr_printf(gtd->ses, "ttou_handler");
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
			SET_BIT(ses->telopts, TELOPT_FLAG_UPDATENAWS);
		}
	}

	winch_daemon();
}


void abort_handler(int signal)
{
	syserr_fatal(signal, "abort_handler");
}

void child_handler(int signal)
{
	return;
	syserr_printf(gtd->ses, "child_handler");

//	syserr_fatal(signal, "child_handler");
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
	else if (gtd->attach_sock)
	{
		gtd->attach_sock = close(gtd->attach_sock);

		show_message(gtd->ses, LIST_COMMAND, "#REDETACHING PROCESS TO {%s}", gtd->attach_file);
	}
	else
	{
		cursor_delete_or_exit(gtd->ses, "");
	}
}

void suspend_handler(int signal)
{
	show_message(gtd->ses, LIST_COMMAND, "#SIGNAL: SIGTSTP");

	if (gtd->attach_sock)
	{
		show_message(gtd->ses, LIST_COMMAND, "#DAEMON {%s} WANTS TO DETACH.", gtd->attach_file);

		gtd->attach_sock = close(gtd->attach_sock);

		return;
	}
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

/*	if (signal(SIGCHLD, child_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGCHLD");
	}
*/
/*
	if (signal(SIGINT, interrupt_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGINT");
	}
*/

	if (signal(SIGTSTP, suspend_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGTSTP");
	}
/*
	if (signal(SIGPIPE, pipe_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGPIPE");
	}

	if (signal(SIGXFSZ, xfsz_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGXFSZ");
	}

	if (signal(SIGHUP, hub_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGHUP");
	}

	if (signal(SIGTTIN, hub_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGTTIN");
	}

	if (signal(SIGTTOU, hub_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGTTOU");
	}
*/
	if (signal(SIGWINCH, winch_handler) == BADSIG)
	{
		syserr_fatal(-1, "signal SIGWINCH");
	}

	signal(SIGPIPE, SIG_IGN);

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
		while ((c = getopt(argc, argv, "a: e: g G h M:: r: R:: s t: T v V")) != EOF)
		{
			switch (c)
			{
				case 'h':
					printf("Usage: %s [OPTION]... [FILE]...\n", argv[0]);
					printf("\n");
					printf("  -a  Set argument for PROGRAM START event.\n");
					printf("  -e  Execute given command.\n");
					printf("  -g  Enable the startup user interface.\n");
					printf("  -G  Don't show the greeting screen.\n");
					printf("  -h  This help section.\n");
					printf("  -M  Matrix Digital Rain.\n");
					printf("  -r  Read given file.\n");
					printf("  -R  Reattach to daemonized process.\n");
					printf("  -s  Enable screen reader mode.\n");
					printf("  -t  Set given title.\n");
					printf("  -T  Don't set the default title.\n");
					printf("  -v  Enable verbose mode.\n");
					printf("  -V  Show version information.\n");

					exit(1);

				case 'M':
				case 'G':
					SET_BIT(greeting, STARTUP_FLAG_NOGREETING);
					break;

				case 's':
					SET_BIT(greeting, STARTUP_FLAG_SCREENREADER);
					break;

				case 'v':
					SET_BIT(greeting, STARTUP_FLAG_VERBOSE);
					break;

				case 'V':
					printf("\nTinTin++ " CLIENT_VERSION "\n");
					printf("\n(C) 2004-2019 Igor van den Hoven\n");
					printf("\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\n");
					exit(1);
			}
		}
	}

	init_tintin(greeting);


	RESTRING(gtd->vars[1], argv[0]);

	if (argc > 1)
	{
		optind = 1;

		RESTRING(gtd->vars[2], argv[1]);

		while ((c = getopt(argc, argv, "a: e: g G h M:: r: R:: s t: T v")) != EOF)
		{
			switch (c)
			{
				case 'a':
					RESTRING(gtd->vars[0], argv[2]);
					SET_BIT(greeting, STARTUP_FLAG_ARGUMENT);
					break;

				case 'e':
					gtd->level->input++;
					gtd->ses = script_driver(gtd->ses, LIST_COMMAND, optarg);
					gtd->level->input--;
					break;

				case 'g':
					gtd->ses = command(gtd->ses, do_banner, "gui");
					break;

				case 'G':
					break;

				case 'M':
					command(gts, do_test, "rain %s", optarg ? optarg : "");
					break;

				case 'r':
//					gtd->level->input++;
					gtd->ses = command(gtd->ses, do_read, "%s", optarg);
//					gtd->level->input--;
					break;

				case 'R':
					SET_BIT(gtd->flags, TINTIN_FLAG_DAEMONIZE);
					command(gtd->ses, do_daemon, "attach {%s}", optarg ? optarg : "");
					break;

				case 's':
					break;

				case 't':
					SET_BIT(greeting, STARTUP_FLAG_NOTITLE);
					print_stdout(0, 0, "\e]0;%s\007", optarg);
					break;

				case 'T':
					SET_BIT(greeting, STARTUP_FLAG_NOTITLE);
					break;

				case 'v':
//					do_configure(gtd->ses, "{VERBOSE} {ON}");
					break;

				default:
//					tintin_printf2(NULL, "Unknown option '%c'.", c);
					break;;
			}
		}
	}

	if (!HAS_BIT(greeting, STARTUP_FLAG_NOTITLE))
	{
		command(gts, do_screen, "LOAD BOTH");
		command(gts, do_screen, "SAVE BOTH");
		command(gts, do_screen, "SET BOTH TinTin++");
	}

	gtd->system->exec = strdup(argv[0]);

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
		if (!strncasecmp(argv[optind], "telnet://", 9))
		{
			gtd->ses = command(gtd->ses, do_session, "%s", argv[optind]);
		}
		else
		{
			gtd->level->input++;

			gtd->ses = command(gtd->ses, do_read, "%s", argv[optind]);

			gtd->level->input--;
		}
	}

	check_all_events(gts, EVENT_FLAG_SYSTEM, 0, 0, "PROGRAM START");

	mainloop();

	return 0;
}

void init_tintin(int greeting)
{
	char filename[PATH_SIZE];
	int index;

	gtd                 = (struct tintin_data *) calloc(1, sizeof(struct tintin_data));

	init_memory();

	push_call("init_tintin(%d)",greeting);

	gtd->level          = (struct level_data *) calloc(1, sizeof(struct level_data));

	gtd->buf            = str_alloc(BUFFER_SIZE);
	gtd->out            = str_alloc(BUFFER_SIZE);

	gtd->flags          = TINTIN_FLAG_INHERITANCE;

	gtd->mccp_len       = 10000;
	gtd->mccp_buf       = (unsigned char *) calloc(1, gtd->mccp_len);

	gtd->mud_output_max = 16384;
	gtd->mud_output_buf = (char *) calloc(1, gtd->mud_output_max);

	// WSL: /proc/sys/kernel/osrelease

	gtd->system         = (struct system_data *) calloc(1, sizeof(struct system_data));

	gtd->system->os     = strdup(getenv("OS")   ? getenv("OS")   : "UNKNOWN");
	gtd->system->home   = strdup(getenv("HOME") ? getenv("HOME") : "~/");
	gtd->system->lang   = strdup(getenv("LANG") ? getenv("LANG") : "UNKNOWN");
	gtd->system->term   = strdup(getenv("TERM") ? getenv("TERM") : "UNKNOWN");

	gtd->system->tt_dir = restringf(gtd->system->tt_dir, "%s/%s", gtd->system->home, TINTIN_DIR);

	if (mkdir(gtd->system->tt_dir, 0755) && errno != EEXIST)
	{
		gtd->system->tt_dir = restringf(gtd->system->tt_dir, "%s", TINTIN_DIR);

		mkdir(gtd->system->tt_dir, 0755);
	}

	gtd->detach_file    = strdup("");
	gtd->attach_file    = strdup("");

	gtd->tintin_char    = '#';

	gtd->time           = time(NULL);

	for (index = 0 ; index < 100 ; index++)
	{
		gtd->vars[index] = strdup("");
		gtd->cmds[index] = strdup("");
	}

	for (index = 1 ; index ; index++)
	{
		if (*event_table[index].name == 0)
		{
			break;
		}

		if (strcmp(event_table[index - 1].name, event_table[index].name) > 0)
		{
			print_stdout(0, 0, "\e[1;31minit_tintin() unsorted event table %s vs %s.", event_table[index - 1].name, event_table[index].name);

			break;
		}
	}

	init_commands();

	gtd->screen = calloc(1, sizeof(struct screen_data));

	gtd->screen->rows   = SCREEN_HEIGHT;
	gtd->screen->cols   = SCREEN_WIDTH;
	gtd->screen->height = SCREEN_HEIGHT * 16;
	gtd->screen->width  = SCREEN_WIDTH * 10;
	gtd->screen->focus  = 1;

	gtd->dispose_list   = init_list(NULL, 0, 32);

	init_msdp_table();

	// global tintin session

	gts = (struct session *) calloc(1, sizeof(struct session));

	gts->created        = gtd->time;
	gts->name           = strdup("gts");
	gts->group          = strdup("");
	gts->session_host   = strdup("");
	gts->session_ip     = strdup("");
	gts->session_port   = strdup("");
	gts->cmd_color      = strdup("");
	gts->telopts        = TELOPT_FLAG_ECHO;
	gts->config_flags   = CONFIG_FLAG_MCCP;
	gts->socket         = 1;
	gts->read_max       = 16384;
	gts->logname        = strdup("");
	gts->lognext_name   = strdup("");
	gts->logline_name   = strdup("");

	gts->more_output    = str_dup("");

	gtd->ses = gts;

	for (index = 0 ; index < LIST_MAX ; index++)
	{
		gts->list[index] = init_list(gts, index, 32);
	}

	gtd->banner_list    = init_list(gts, LIST_CONFIG, 32);

	gts->split          = calloc(1, sizeof(struct split_data));
	gts->scroll         = calloc(1, sizeof(struct scroll_data));
	gts->input          = calloc(1, sizeof(struct input_data));

	init_local(gts);

	init_terminal_size(gts);

	init_input(gts, 0, 0, 0, 0);

	init_buffer(gts, 10000);

	if (HAS_BIT(greeting,  STARTUP_FLAG_VERBOSE))
	{
		gtd->level->verbose++;
	}

	gtd->level->input++;

	command(gts, do_configure, "{AUTO TAB}         {5000}");
	command(gts, do_configure, "{BUFFER SIZE}     {10000}");
	command(gts, do_configure, "{COLOR MODE}         {ON}");
	command(gts, do_configure, "{COLOR PATCH}       {OFF}");
	command(gts, do_configure, "{COMMAND COLOR}   {<078>}");
	command(gts, do_configure, "{COMMAND ECHO}       {ON}");
	command(gts, do_configure, "{CONNECT RETRY}       {0}");
	command(gts, do_configure, "{CHARSET}          {AUTO}");
	command(gts, do_configure, "{HISTORY SIZE}     {1000}");
	command(gts, do_configure, "{LOG MODE}          {RAW}");
	command(gts, do_configure, "{MOUSE}             {OFF}");
	command(gts, do_configure, "{PACKET PATCH}     {AUTO}");
	command(gts, do_configure, "{RANDOM SEED}      {AUTO}");
	command(gts, do_configure, "{REPEAT CHAR}         {!}");
	command(gts, do_configure, "{REPEAT ENTER}      {OFF}");
	command(gts, do_configure, "{SCREEN READER}      {%s}", HAS_BIT(greeting, STARTUP_FLAG_SCREENREADER) ? "ON" : "OFF");
	command(gts, do_configure, "{SCROLL LOCK}        {ON}");
	command(gts, do_configure, "{SPEEDWALK}         {OFF}");
	command(gts, do_configure, "{TAB WIDTH}        {AUTO}");
	command(gts, do_configure, "{TELNET}             {ON}");
	command(gts, do_configure, "{TINTIN CHAR}         {#}");
	command(gts, do_configure, "{VERBATIM}          {OFF}");
	command(gts, do_configure, "{VERBATIM CHAR}      {\\}");
	command(gts, do_configure, "{VERBOSE}            {%s}", HAS_BIT(greeting, STARTUP_FLAG_VERBOSE) ? "ON" : "OFF");
	command(gts, do_configure, "{WORDWRAP}           {ON}");

	command(gts, do_pathdir, " n  s  1");
	command(gts, do_pathdir, " e  w  2");
	command(gts, do_pathdir, " s  n  4");
	command(gts, do_pathdir, " w  e  8");
	command(gts, do_pathdir, " u  d 16");
	command(gts, do_pathdir, " d  u 32");
	command(gts, do_pathdir, "ne sw  3");
	command(gts, do_pathdir, "nw se  9");
	command(gts, do_pathdir, "se nw  6");
	command(gts, do_pathdir, "sw ne 12");

	sprintf(filename, "%s/%s", gtd->system->tt_dir, HISTORY_FILE);

	if (access(filename, F_OK ) != -1)
	{
		command(gts, do_history, "read %s", filename);
	}

	gtd->level->input--;

	if (HAS_BIT(greeting,  STARTUP_FLAG_VERBOSE))
	{
		gtd->level->verbose--;
	}

	init_terminal(gts);

	reset_screen(gts);

	command(gts, do_banner, "INIT");

	if (!HAS_BIT(greeting, STARTUP_FLAG_NOGREETING))
	{
		if (HAS_BIT(greeting, STARTUP_FLAG_SCREENREADER))
		{
			tintin_printf2(gts, "Welcome to TinTin Plus Plus. Don't know which MUD to play? How about the following MUD.");

			command(gts, do_banner, "RANDOM");

			tintin_printf2(gts, "You're using TinTin Plus Plus version %s written by Peter Unold, Bill Reis, and Igor van den Hoven.", CLIENT_VERSION);

			tintin_printf2(gts, "");

			tintin_printf2(gts, "For help and requests visit the Discord channel at https://discord.gg/gv7a37n");
		}
		else
		{
			command(gts, do_banner, "RANDOM");

			if (gtd->screen->cols >= 80)
			{
				command(gts, do_help, "GREETING");
			}
			else
			{
				tintin_printf2(gts, "\e[0;37mT I N T I N + +   %s", CLIENT_VERSION);

				tintin_printf2(gts, "");

//				tintin_printf2(gts, "\e[0;36mT\e[0;37mhe K\e[0;36mi\e[0;37mcki\e[0;36mn\e[0;37m \e[0;36mT\e[0;37mickin D\e[0;36mi\e[0;37mkuMUD Clie\e[0;36mn\e[0;37mt");

				tintin_printf2(gts, "Code by Peter Unold, Bill Reis, and Igor van den Hoven");
			}
		}
	}
	pop_call();
	return;
}


void quitmsg(char *message)
{
	struct session *ses;
	static char crashed = FALSE;

	if (crashed++)
	{
		print_stdout(0, 0, "quitmsg(crashed)\n");

		fflush(NULL);

		exit(-1);
	}

	SET_BIT(gtd->flags, TINTIN_FLAG_TERMINATE);

	for (ses = gts->next ; ses ; ses = gts->next)
	{
		cleanup_session(ses);
	}

	if (gtd->chat)
	{
		chat_uninitialize("", "");
	}

	check_all_events(gts, EVENT_FLAG_SYSTEM, 0, 1, "PROGRAM TERMINATION", message ? message : "");

	if (gtd->history_size)
	{
		command(gts, do_history, "write %s/%s", gtd->system->tt_dir, HISTORY_FILE);
	}

	reset_daemon();

	reset_terminal(gts);

	reset_screen(gts);

	if (message == NULL || *message)
	{
		if (message)
		{
			print_stdout(0, 0, "\n\e[0m%s", message);
		}
		print_stdout(0, 0, "\nGoodbye from TinTin++\n\n");
	}
	fflush(stdout);

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

	check_all_events(ses, SUB_SEC|EVENT_FLAG_SYSTEM, 0, 4, "SYSTEM ERROR", name, buf, ntos(errno), errstr);

	if (!check_all_events(ses, SUB_SEC|EVENT_FLAG_SYSTEM, 0, 4, "CATCH SYSTEM ERROR", name, buf, ntos(errno), errstr))
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
		print_stdout(0, 0, "syserr_signal(crashed)\n");

		fflush(NULL);

		exit(-1);
	}

	reset_terminal(gts);

	reset_screen(gts);

	fflush(NULL);

	dump_stack();

	sprintf(buf, "\e[1;31mFATAL SIGNAL FROM (%s): %s\e[0m\n", msg, strsignal(signal));

	print_stdout(0, 0, "%s", buf);

	fflush(NULL);

	abort();
}

void syserr_fatal(int signal, char *msg)
{
	char buf[256], errstr[128];
	static char crashed = FALSE;

	if (crashed++)
	{
		print_stdout(0, 0, "syserr_fatal(crashed)");

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

	check_all_events(NULL, SUB_SEC|EVENT_FLAG_SYSTEM, 0, 1, "SYSTEM CRASH", errstr);

	if (gtd->level->quiet)
	{
		gtd->level->quiet = 0;
	}

	reset_terminal(gts);

	reset_screen(gts);

	print_stdout(0, 0, "\e[?1049l\e[r");

	dump_stack();

	sprintf(buf, "\n\e[1;31mFATAL ERROR \e[1;32m%s %s\e[0m\n", msg, errstr);

	print_stdout(0, 0, "%s", buf);

	reset_daemon();

	fflush(NULL);

	abort();
}
