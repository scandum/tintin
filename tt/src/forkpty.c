#include "config.h"

#if !defined(HAVE_PTY_H) && defined(HAVE__DEV_PTMX) && !defined(HAVE_FORKPTY)

/*
 * Substitute for the nonstandard BSD/GNU extension 'forkpty' using
 * SysV STREAMS (the /dev/ptmx pseudoterminal multiplexer).
 *
 * dgc@uchicago.edu
 */

#ifdef HAVE_SYS_IOCTL_H
  #include <sys/ioctl.h>
#endif

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h> /* PATH_MAX */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <stropts.h>

#define DEV_PTMX "/dev/ptmx"

enum
{
	EPTMX_OK = 0,
	EPTMX_OPEN,
	EPTMX_GRANT,
	EPTMX_UNLOCK,
	EPTMX_NAME,
	EPTMX_FIND,
	EPTMX_PUSH_PTEM,
	EPTMX_PUSH_LDTERM,
	EPTMX_PUSH_TTCOMPAT,
	EPTMX_END
} ptmx_errs;

int open_master(char *name, int sz)
{
	char *sname;
	int fd;

	strncpy(name, DEV_PTMX, sz);
	name[sz-1] = '\0';

	fd = open(name, O_RDWR);
	if (fd < 0)
		return EPTMX_OPEN;

	if (grantpt(fd) < 0)
	{
		close(fd);
		return EPTMX_GRANT;
	}

	if (unlockpt(fd) < 0)
	{
		close(fd);
		return EPTMX_UNLOCK;
	}

	sname = ptsname(fd);
	if (sname == NULL)
	{
		close(fd);
		return EPTMX_NAME;
	}

	strncpy(name, sname, sz);
	name[sz-1] = '\0';
	return fd;
}

int open_slave(char *name)
{
	int fd;
	int status;

	fd = open(name, O_RDWR);
	if (fd < 0)
		return EPTMX_OPEN;

	status = ioctl(fd, I_FIND, "ldterm");
	if (status < 0)
	{
		close(fd);
		return EPTMX_FIND;
	}

	if (status > 0)
		return fd;

	if (ioctl(fd, I_PUSH, "ptem") < 0)
	{
		close(fd);
		return EPTMX_PUSH_PTEM;
	}

	if (ioctl(fd, I_PUSH, "ldterm") < 0)
	{
		close(fd);
		return EPTMX_PUSH_LDTERM;
	}

	if (ioctl(fd, I_PUSH, "ttcompat") < 0)
	{
		close(fd);
		return EPTMX_PUSH_TTCOMPAT;
	}

	return fd;
}


int login_tty(int fd)
{
	setsid();
	if (ioctl(fd, TIOCSCTTY, NULL) == -1)
		return -1;
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);
	if (fd > 2)
		close(fd);
	return 0;
}


pid_t forkpty(int *masterp, char *name, struct termios *termp, struct winsize *winp)
{
	int master, slave;
	char ptname[PATH_MAX];
	pid_t pid;

	master = open_master(ptname, sizeof(ptname));
	if (master < 0)
	{
		return -1;
	}

	slave = open_slave(ptname);
	if (slave < 0)
	{
		close(master);
		return -1;
	}

	if (name)
		strcpy(name, ptname);

	if (termp)
		tcsetattr(slave, TCSAFLUSH, termp);
	if (winp)
		ioctl(slave, TIOCSWINSZ, winp);

	pid = fork();
	if (pid < 0)
	{
		close(slave);
		close(master);
		return -1;
	}
	else if (pid == 0)
	{
		/* child/slave */
		close(master);
		login_tty(slave);
		return 0;
	}

	/* parent/master */
	*masterp = master;
	close(slave);
	return pid;
}

#endif
