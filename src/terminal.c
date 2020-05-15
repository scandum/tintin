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
*                      coded by Igor van den Hoven 2006                       *
******************************************************************************/


#include "tintin.h"

#ifdef HAVE_SYS_IOCTL_H
  #include <sys/ioctl.h>
#endif
#include <termios.h>

void init_terminal(struct session *ses)
{
	struct termios io;

	if (tcgetattr(0, &gtd->old_terminal))
	{
		syserr_fatal(-1, "init_terminal: tcgetattr 1");
	}

	io = gtd->old_terminal;

	/*
		Canonical mode off
	*/

	DEL_BIT(io.c_lflag, ICANON);

	io.c_cc[VMIN]   = 1;
	io.c_cc[VTIME]  = 0;
	io.c_cc[VSTART] = 255;
	io.c_cc[VSTOP]  = 255;
	io.c_cc[VINTR]  = 4; // ctrl-d

	/*
		Make the terminalal as raw as possible
	*/

/*
	DEL_BIT(io.c_iflag, IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	DEL_BIT(io.c_oflag, OPOST);
	DEL_BIT(io.c_cflag, CSIZE|PARENB);
*/

	DEL_BIT(io.c_lflag, ECHO|ECHONL|IEXTEN|ISIG);
//	DEL_BIT(io.c_lflag, ECHO|ECHONL|IEXTEN|ISIG);

	SET_BIT(io.c_cflag, CS8);

	if (tcsetattr(0, TCSANOW, &io))
	{
		syserr_printf(ses, "init_terminal: tcsetattr");
	}

	if (tcgetattr(0, &gts->cur_terminal))
	{
		syserr_fatal(-1, "init_terminal: tcgetattr 2");
	}

	print_stdout("\e=");
	print_stdout("\e[>4;1m");
}

void reset_terminal(struct session *ses)
{
	if (gtd->detach_port == 0)
	{
		if (tcsetattr(0, TCSANOW, &gtd->old_terminal))
		{
			syserr_printf(ses, "reset_terminal: tcsetattr");
		}
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING))
	{
		print_stdout("\e[?1000l\e[?1002l\e[?1004l\e[?1006l");
	}
	print_stdout("\e[?25h");
	print_stdout("\e[23t");
	print_stdout("\e[>4n");
}


void save_session_terminal(struct session *ses)
{
	tcgetattr(0, &ses->cur_terminal);
}

void refresh_session_terminal(struct session *ses)
{
//	tcsetattr(0, TCSANOW, &ses->cur_terminal);
}

void echo_off(struct session *ses)
{
	struct termios io;

	tcgetattr(STDIN_FILENO, &io);

	DEL_BIT(io.c_lflag, ECHO|ECHONL);

	tcsetattr(STDIN_FILENO, TCSADRAIN, &io);
}

void echo_on(struct session *ses)
{
	struct termios io;

	tcgetattr(STDIN_FILENO, &io);

	SET_BIT(io.c_lflag, ECHO|ECHONL);

	tcsetattr(STDIN_FILENO, TCSADRAIN, &io);
}

void init_terminal_size(struct session *ses)
{
	struct winsize screen;
	static int old_rows, old_cols;

	push_call("init_terminal_size(%p)",ses);

	if (ses == gts)
	{
		old_rows = gtd->screen->rows;
		old_cols = gtd->screen->cols;

		if (ioctl(1, TIOCGWINSZ, &screen) >= 0)
		{
			init_screen(screen.ws_row, screen.ws_col, screen.ws_ypixel, screen.ws_xpixel);

			if (gtd->attach_sock)
			{
				char buf[100];
				sprintf(buf, "\e[8;%d;%dt\e[4;%d;%dt\e[7t", screen.ws_row, screen.ws_col, screen.ws_ypixel, screen.ws_xpixel);
				write(gtd->attach_sock, buf, strlen(buf));
			}
		}
	}

	if (ses->scroll)
	{
		SET_BIT(ses->scroll->flags, SCROLL_FLAG_RESIZE);
	}

	if (ses->map)
	{
		SET_BIT(ses->map->flags, MAP_FLAG_RESIZE);
	}

	init_split(ses, ses->split->sav_top_row, ses->split->sav_top_col, ses->split->sav_bot_row, ses->split->sav_bot_col);

	check_all_events(ses, SUB_ARG, 0, 4, "SCREEN RESIZE", ntos(gtd->screen->rows), ntos(gtd->screen->cols), ntos(gtd->screen->height), ntos(gtd->screen->width));

	if (old_rows <= old_cols / 2 && gtd->screen->rows > gtd->screen->cols / 2)
	{
		check_all_events(ses, SUB_ARG, 0, 4, "SCREEN ROTATE PORTRAIT", ntos(gtd->screen->rows), ntos(gtd->screen->cols), ntos(gtd->screen->height), ntos(gtd->screen->width));
	}
	else if (old_rows >= old_cols / 2 && gtd->screen->rows < gtd->screen->cols / 2)
	{
		check_all_events(ses, SUB_ARG, 0, 4, "SCREEN ROTATE LANDSCAPE", ntos(gtd->screen->rows), ntos(gtd->screen->cols), ntos(gtd->screen->height), ntos(gtd->screen->width));
	}

	msdp_update_all("SCREEN_ROWS",   "%d", gtd->screen->rows);
	msdp_update_all("SCREEN_COLS",   "%d", gtd->screen->cols);
	msdp_update_all("SCREEN_HEIGHT", "%d", gtd->screen->height);
	msdp_update_all("SCREEN_WIDTH",  "%d", gtd->screen->width);

	pop_call();
	return;
}

int get_scroll_rows(struct session *ses)
{
	return (ses->split->bot_row - ses->split->top_row);
}

int get_scroll_cols(struct session *ses)
{
	return ses->wrap;
}

char *get_charset(struct session *ses)
{
	switch (HAS_BIT(ses->charset, CHARSET_FLAG_ALL))
	{
		case CHARSET_FLAG_BIG5:
			return "BIG-5";

		case CHARSET_FLAG_GBK1:
			return "GBK-1";

		case CHARSET_FLAG_UTF8:
			return "UTF-8";

		case CHARSET_FLAG_UTF8|CHARSET_FLAG_BIG5TOUTF8:
			return "BIG5TOUTF8";

	        case CHARSET_FLAG_UTF8|CHARSET_FLAG_CP1251TOUTF8:
	        	return "CP1251TOUTF8";

		case CHARSET_FLAG_UTF8|CHARSET_FLAG_FANSITOUTF8:
			return "FANSI";
		
		case CHARSET_FLAG_UTF8|CHARSET_FLAG_GBK1TOUTF8:
			return "GBK1TOUTF8";

		case CHARSET_FLAG_UTF8|CHARSET_FLAG_KOI8TOUTF8:
			return "KOI8TOUTF8";

		case CHARSET_FLAG_UTF8|CHARSET_FLAG_ISO1TOUTF8:
			return "ISO1TOUTF8";

		case CHARSET_FLAG_UTF8|CHARSET_FLAG_ISO2TOUTF8:
			return "ISO2TOUTF8";

		default:
			return "ASCII";
	}
}
