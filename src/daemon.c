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
*                      coded by Igor van den Hoven 2019                       *
******************************************************************************/

#include "tintin.h"

#ifdef HAVE_PTY_H
#include <pty.h>
#else
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#endif
#include <fcntl.h>  
#include <dirent.h>
#include <termios.h>
#include <sys/un.h>

int get_daemon_dir(struct session *ses, char *filename);

DO_COMMAND(do_daemon)
{
	int cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		info:

		tintin_header(ses, " DAEMON OPTIONS ");

		for (cnt = 0 ; *daemon_table[cnt].fun != NULL ; cnt++)
		{
			if (*daemon_table[cnt].desc)
			{
				tintin_printf2(ses, "  [%-13s] %s", daemon_table[cnt].name, daemon_table[cnt].desc);
			}
		}
		tintin_header(ses, "");

		return ses;
	}
	else
	{
		for (cnt = 0 ; *daemon_table[cnt].name ; cnt++)
		{
			if (is_abbrev(arg1, daemon_table[cnt].name))
			{
				break;
			}
		}

		if (*daemon_table[cnt].name == 0)
		{
			goto info;
		}
		else
		{
			daemon_table[cnt].fun(ses, arg);
		}
	}

	return ses;
}


DO_DAEMON(daemon_attach)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], filename[BUFFER_SIZE], sock_file[BUFFER_SIZE];
	struct dirent **dirlist;
	struct sockaddr_un addr_un;
	int size, index, pid, error, repeat = 0;
	struct timeval timeout;
	fd_set wds, rds;

	timeout.tv_sec  = 0;
	timeout.tv_usec = 100000;

	if (gtd->attach_sock)
	{
		show_error(ses, LIST_COMMAND, "#DAEMON ATTACH: YOU ARE ALREADY ATTACHED TO {%s}.", gtd->attach_file);

		return;
	}
	sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (!get_daemon_dir(ses, filename))
	{
		return;
	}

	start:

	size = scandir(filename, &dirlist, 0, alphasort);

	if (size == -1)
	{
		syserr_printf(ses, "do_attach: scandir:");

		return;
	}

	for (*arg2 = index = pid = 0 ; index < size ; index++)
	{
		if (strlen(dirlist[index]->d_name) > 2)
		{
			if (*arg1)
			{
				if (!strstr(dirlist[index]->d_name, arg1))
				{
					continue;
				}
			}
			arg = strchr(dirlist[index]->d_name, '.');

			if (arg)
			{
				*arg = 0;

				strcpy(arg2, dirlist[index]->d_name);

				arg = strchr(dirlist[index]->d_name, '_');

				if (arg)
				{
					pid = atoi(arg + 1);
				}
				break;
			}
		}
	}

	for (index = 0 ; index < size ; index++)
	{
		free(dirlist[index]);
	}
	free(dirlist);

	if (pid == 0)
	{
		if (HAS_BIT(gtd->flags, TINTIN_FLAG_DAEMONIZE))
		{
			daemon_detach(ses, arg1);

			return;
		}

		if (*arg1 && ++repeat < 10)
		{
			usleep(2000);

			goto start;
		}

		if (*arg1)
		{
			show_message(ses, LIST_COMMAND, "#DAEMON ATTACH: UNABLE TO FIND DAEMON FILE {%s} IN {%s}.", arg1, filename);
		}
		else
		{
			show_message(ses, LIST_COMMAND, "#DAEMON ATTACH: NO AVAILABLE DAEMON FILES FOUND IN {%s}.", filename);
		}

		return;
	}

	sprintf(sock_file, "%.*s/%.*s.s", PATH_SIZE, filename, PATH_SIZE, arg2);

	if (access(sock_file, F_OK) == -1)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: DAEMON ATTACH: FILE {%s} CANNOT BE ACCESSED.", sock_file);

		return;
	}

	if (kill((pid_t) pid, 0) == -1)
	{
		show_error(ses, LIST_COMMAND, "#ERROR: DAEMON ATTACH: REMOVING INVALID DAEMON FILE {%s}.", sock_file);

		remove(sock_file);

		if (HAS_BIT(gtd->flags, TINTIN_FLAG_DAEMONIZE) || *arg1 == 0)
		{
			goto start;
		}
		return;
	}
	DEL_BIT(gtd->flags, TINTIN_FLAG_DAEMONIZE);

	memset(&addr_un, 0, sizeof(addr_un));

	if (strlen(sock_file) >= sizeof(addr_un.sun_path))
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #DAEMON ATTACH: {%s} FILENAME EXCEEDS MAXIMUM LENGTH OF %d.", filename, sizeof(addr_un.sun_path));

		return;
	}

	if (pid == getpid())
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #DAEMON ATTACH: {%s} CANNOT ATTACH TO ITSELF.", filename);
		
		return;
	}

	gtd->attach_file = restringf(gtd->attach_file, "%s", sock_file);
	gtd->attach_pid  = pid;
	gtd->attach_sock = socket(AF_UNIX, SOCK_STREAM, 0);

	if (gtd->attach_sock == -1)
	{
		syserr_printf(ses, "do_attach: %s: socket:");

		gtd->attach_sock = 0;

		return;
	}

	strcpy(addr_un.sun_path, sock_file);
	addr_un.sun_family = AF_UNIX;

	show_message(ses, LIST_COMMAND, "#DAEMON ATTACH: CONNECTING {%d} TO {%d} {%s}", getpid(), gtd->attach_pid, sock_file);

/*
	error = select(gtd->attach_sock, NULL, &wds, NULL, &timeout);

	if (error == -1)
	{
		syserr_printf(ses, "do_attach: %s: select:", sock_file);

		return;
	}
*/
	if (connect(gtd->attach_sock, (struct sockaddr *)&addr_un, sizeof(addr_un)) == -1)
	{
		syserr_printf(ses, "do_attach: %s: connect:", sock_file);

		gtd->attach_sock = close(gtd->attach_sock);

		return;
	}


	FD_ZERO(&wds);

	FD_SET(gtd->attach_sock, &wds);

	error = select(FD_SETSIZE, NULL, &wds, NULL, &timeout);

	if (error < 0)
	{
		syserr_printf(ses, "do_attach: select wds:");

		show_error(ses, LIST_COMMAND, "#ERROR: #DAEMON ATTACH: UNABLE TO WRITE TO {%s}.", sock_file);

		gtd->attach_sock = close(gtd->attach_sock);

		return;
	}

	if (!FD_ISSET(gtd->attach_sock, &wds))
	{
		show_error(ses, LIST_COMMAND, "#ERROR: #DAEMON ATTACH: UNABLE TO WRITE TO {%s}.", sock_file);

		gtd->attach_sock = close(gtd->attach_sock);

		return;
	}

	FD_ZERO(&rds);
	FD_SET(gtd->attach_sock, &rds);

	error = select(FD_SETSIZE, &rds, NULL, NULL, &timeout);

	if (error < 0)
	{
		syserr_printf(ses, "do_attach: select rds:");

		gtd->attach_sock = close(gtd->attach_sock);

		return;
	}

	if (error == 0)
	{
		tintin_printf2(ses, "do_attach: select rds: timeout");

		gtd->attach_sock = close(gtd->attach_sock);
		
		return;
	}

	return;
}

DO_DAEMON(daemon_detach)
{
	char arg1[BUFFER_SIZE], filename[BUFFER_SIZE];
	struct sockaddr_un addr_un;
	pid_t pid, sid;
	int dev_null;

	sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (gtd->detach_port)
	{
		if (gtd->detach_sock)
		{
			show_message(gtd->ses, LIST_COMMAND, "#DAEMON DETACH: DETACHING FROM {%s}", gtd->detach_file);

//			kill((pid_t) gtd->detach_sock, SIGTSTP);

//			print_stdout("%c", (char) 255);

			gtd->detach_sock = close(gtd->detach_sock);
		}
		else
		{
			show_error(gtd->ses, LIST_COMMAND, "#DAEMON DETACH: ALREADY FULLY DETACHED.");
		}
		return;
	}

	if (!get_daemon_dir(ses, filename))
	{
		return;
	}

	pid = fork();

	if (pid < 0)
	{
		syserr_printf(ses, "do_detatch: fork:");

		return;
	}

	if (pid > 0)
	{
		if (HAS_BIT(gtd->flags, TINTIN_FLAG_DAEMONIZE))
		{
			DEL_BIT(gtd->flags, TINTIN_FLAG_DAEMONIZE);

			usleep(2000);

			daemon_attach(ses, *arg1 ? arg1 : "pid");

			return;
		}
		reset_terminal(gtd->ses);

		print_stdout("\e[r\e[%d;%dH", gtd->screen->rows, 1);

		_exit(0);
	}

	DEL_BIT(gtd->flags, TINTIN_FLAG_DAEMONIZE);

	sid = setsid();

	if (sid < 0)
	{
		syserr_printf(ses, "do_detach: setsid:");
		return;
	}

	if (!get_daemon_dir(ses, filename))
	{
		return;
	}

	memset(&addr_un, 0, sizeof(addr_un));

	cat_sprintf(filename, "/%s_%d.s", *arg1 ? arg1 : "pid", getpid());

	if (strlen(filename) >= sizeof(addr_un.sun_path))
	{
		tintin_printf(ses, "#DAEMON DETACH: FILE NAME LENGTH OF {%s} EXCEEDS MAXIMUM OF %d.", filename, sizeof(addr_un.sun_path));

		return;
	}

	strcpy(addr_un.sun_path, filename);
	addr_un.sun_family = AF_UNIX;

	show_message(ses, LIST_COMMAND, "#DAEMON DETACH: DAEMONIZING PROCESS %d AS {%s}", getpid(), filename);

	gtd->detach_port = socket(AF_UNIX, SOCK_STREAM, 0);

	if (gtd->detach_port <= 0)
	{
		syserr_printf(ses, "do_detach: socket:");

		return;
	}

	if (bind(gtd->detach_port, (struct sockaddr *) &addr_un, sizeof(struct sockaddr_un)) < 0)
	{
		syserr_printf(ses, "do_detach: bind:");

		gtd->detach_port = close(gtd->detach_port);

		return;
	}

	if (listen(gtd->detach_port, 32) < 0)
	{
		syserr_printf(ses, "do_detach: listen:");

		gtd->detach_port = close(gtd->detach_port);

		return;
	}

	dev_null = open("/dev/null", O_RDWR, 0);

	if (dev_null == -1)
	{
		syserr_printf(ses, "daemon_detach: dev_null open:");

		return;
	}

	if (dup2(dev_null, STDIN_FILENO) == -1)
	{
		syserr_printf(ses, "daemon_detach: dup2 STDIN-fileno:");
	}

//	dup2(dev_null, STDOUT_FILENO);
//	dup2(dev_null, STDERR_FILENO);

	close(dev_null);

	gtd->detach_file = restringf(gtd->detach_file, "%s", filename);

	return;
}

DO_DAEMON(daemon_input)
{
	char arg1[BUFFER_SIZE], out[BUFFER_SIZE];
	int size;

	if (*arg == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #DAEMON INPUT <TEXT>");

		return;
	}

	if (gtd->attach_sock <= 0)
	{
		show_error(ses, LIST_COMMAND, "#DAEMON INPUT: YOU MUST BE ATTACHED TO A DAEMON TO SEND INPUT.");

		return;
	}

	get_arg_in_braces(ses, arg, arg1, GET_ALL);

	size = substitute(ses, arg1, out, SUB_VAR|SUB_FUN|SUB_COL|SUB_ESC|SUB_EOL);

	if (write(gtd->attach_sock, out, size) < 0)
	{
		gtd->attach_sock = close(gtd->attach_sock);

		show_message(gtd->ses, LIST_COMMAND, "#DAEMON INPUT: WRITE ERROR: UNATTACHING.");
	}
}

DO_DAEMON(daemon_kill)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], filename[BUFFER_SIZE], sock_file[BUFFER_SIZE];
	struct dirent **dirlist;
	int size, index, pid;

	sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (!get_daemon_dir(ses, filename))
	{
		return;
	}

	size = scandir(filename, &dirlist, 0, alphasort);

	if (size == -1)
	{
		syserr_printf(ses, "do_attach: scandir:");

		return;
	}

	for (*arg2 = index = pid = 0 ; index < size ; index++)
	{
		if (strlen(dirlist[index]->d_name) > 2)
		{
			tintin_printf2(ses, "#DAEMON KILL: CHECKING FILE {%s}", dirlist[index]->d_name);

			if (*arg1)
			{
				if (!strstr(dirlist[index]->d_name, arg1))
				{
					continue;
				}
			}
			arg = strchr(dirlist[index]->d_name, '.');

			if (arg)
			{
				*arg = 0;

				strcpy(arg2, dirlist[index]->d_name);

				arg = strchr(dirlist[index]->d_name, '_');

				if (arg)
				{
					pid = atoi(arg + 1);

					sprintf(sock_file, "%.*s/%.*s.s", PATH_SIZE, filename, PATH_SIZE, arg2);

					show_message(ses, LIST_COMMAND, "#DAEMON {%s} KILLED.", sock_file, pid);

					kill((pid_t) pid, SIGKILL);

					remove(sock_file);
				}
			}
		}
	}

	for (index = 0 ; index < size ; index++)
	{
		free(dirlist[index]);
	}
	free(dirlist);

	return;
}

DO_DAEMON(daemon_list)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], filename[BUFFER_SIZE], sock_file[BUFFER_SIZE];
	struct dirent **dirlist;
	int size, index, pid;

	sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (!get_daemon_dir(ses, filename))
	{
		return;
	}

	size = scandir(filename, &dirlist, 0, alphasort);

	if (size == -1)
	{
		syserr_printf(ses, "do_attach: scandir:");

		return;
	}

	tintin_printf2(ses, "#THESE DAEMONS HAVE BEEN DEFINED:");

	for (*arg2 = index = pid = 0 ; index < size ; index++)
	{
		if (strlen(dirlist[index]->d_name) > 2)
		{
			if (*arg1)
			{
				if (!strstr(dirlist[index]->d_name, arg1))
				{
					continue;
				}
			}
			arg = strchr(dirlist[index]->d_name, '.');

			if (arg)
			{
				*arg = 0;

				strcpy(arg2, dirlist[index]->d_name);

				arg = strchr(dirlist[index]->d_name, '_');

				if (arg)
				{
					pid = atoi(arg + 1);

					sprintf(sock_file, "%.*s/%.*s.s", PATH_SIZE, filename, PATH_SIZE, arg2);

					tintin_printf2(ses, "%-40s [%6d]", sock_file, pid);
				}
			}
		}
	}
	for (index = 0 ; index < size ; index++)
	{
		free(dirlist[index]);
	}
	free(dirlist);

	return;
}

int get_daemon_dir(struct session *ses, char *filename)
{
	sprintf(filename, "%s/%s", gtd->home, TINTIN_DIR);

	if (mkdir(filename, 0755) && errno != EEXIST)
	{
		show_error(ses, LIST_COMMAND, "#DAEMON CHECK DIR: FAILED TO CREATE TINTIN DIR %s (%s)", filename, strerror(errno));

		return 0;
	}

	sprintf(filename, "%s/%s/%s", gtd->home, TINTIN_DIR, DAEMON_DIR);

	if (mkdir(filename, 0755) && errno != EEXIST)
	{
		show_error(ses, LIST_COMMAND, "#DAEMON CHECK DIR: CANNOT CREATE DAEMON DIR %s (%s)", filename, strerror(errno));

		return 0;
	}
	return 1;
}

void reset_daemon()
{
	if (gtd->detach_sock > 0)
	{
//		print_stdout("removing(%s)\n", gtd->detach_file);

		remove(gtd->detach_file);
	}
/*
	if (gtd->attach_sock > 0)
	{
		print_stdout("unlinking(%s)\n", gtd->attach_file);

		unlink(gtd->attach_file);
	}
*/
}

void winch_daemon()
{
	if (gtd->attach_sock)
	{
		kill((pid_t) gtd->attach_pid, SIGWINCH);
	}
}
