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

// color subs


char *c256to16_fg[256] =
{
	"\e[22;30m", "\e[22;31m", "\e[22;32m", "\e[22;33m", "\e[22;34m", "\e[22;35m", "\e[22;36m", "\e[22;37m",
	 "\e[1;30m",  "\e[1;31m",  "\e[1;32m",  "\e[1;33m",  "\e[1;34m",  "\e[1;35m",  "\e[1;36m",  "\e[1;37m",

	"\e[22;30m", "\e[22;34m", "\e[22;34m", "\e[22;34m",  "\e[1;34m",  "\e[1;34m",
	"\e[22;32m", "\e[22;36m", "\e[22;36m", "\e[22;34m",  "\e[1;34m",  "\e[1;34m",
	"\e[22;32m", "\e[22;36m", "\e[22;36m", "\e[22;36m",  "\e[1;34m",  "\e[1;34m",
	"\e[22;32m", "\e[22;32m", "\e[22;36m", "\e[22;36m", "\e[22;36m",  "\e[1;36m",
	 "\e[1;32m",  "\e[1;32m",  "\e[1;32m", "\e[22;36m",  "\e[1;36m",  "\e[1;36m",
	 "\e[1;32m",  "\e[1;32m",  "\e[1;32m",  "\e[1;36m",  "\e[1;36m",  "\e[1;36m",

	"\e[22;31m", "\e[22;35m", "\e[22;35m", "\e[22;34m",  "\e[1;34m",  "\e[1;34m",
	"\e[22;33m",  "\e[1;30m", "\e[22;34m", "\e[22;34m",  "\e[1;34m",  "\e[1;34m",
	"\e[22;33m", "\e[22;32m", "\e[22;36m", "\e[22;36m",  "\e[1;34m",  "\e[1;34m",
	"\e[22;32m", "\e[22;32m", "\e[22;36m", "\e[22;36m", "\e[22;36m",  "\e[1;36m",
	 "\e[1;32m",  "\e[1;32m",  "\e[1;32m", "\e[22;36m",  "\e[1;36m",  "\e[1;36m",
	 "\e[1;32m",  "\e[1;32m",  "\e[1;32m",  "\e[1;36m",  "\e[1;36m",  "\e[1;36m",

	"\e[22;31m", "\e[22;35m", "\e[22;35m", "\e[22;35m",  "\e[1;34m",  "\e[1;34m",
	"\e[22;33m", "\e[22;31m", "\e[22;35m", "\e[22;35m",  "\e[1;34m",  "\e[1;34m",
	"\e[22;33m", "\e[22;33m", "\e[22;37m", "\e[22;34m",  "\e[1;34m",  "\e[1;34m",
	"\e[22;33m", "\e[22;33m", "\e[22;32m", "\e[22;36m", "\e[22;36m",  "\e[1;34m",
	 "\e[1;32m",  "\e[1;32m",  "\e[1;32m", "\e[22;36m",  "\e[1;36m",  "\e[1;36m",
	 "\e[1;32m",  "\e[1;32m",  "\e[1;32m",  "\e[1;32m",  "\e[1;36m",  "\e[1;36m",

	"\e[22;31m", "\e[22;31m", "\e[22;35m", "\e[22;35m", "\e[22;35m",  "\e[1;35m",
	"\e[22;31m", "\e[22;31m", "\e[22;35m", "\e[22;35m", "\e[22;35m",  "\e[1;35m",
	"\e[22;33m", "\e[22;33m", "\e[22;31m", "\e[22;35m", "\e[22;35m",  "\e[1;34m",
	"\e[22;33m", "\e[22;33m", "\e[22;33m", "\e[22;37m",  "\e[1;34m",  "\e[1;34m",
	"\e[22;33m", "\e[22;33m", "\e[22;33m",  "\e[1;32m",  "\e[1;36m",  "\e[1;36m",
	 "\e[1;33m",  "\e[1;33m",  "\e[1;32m",  "\e[1;32m",  "\e[1;36m",  "\e[1;36m",

	 "\e[1;31m",  "\e[1;31m",  "\e[1;31m", "\e[22;35m",  "\e[1;35m",  "\e[1;35m",
	 "\e[1;31m",  "\e[1;31m",  "\e[1;31m", "\e[22;35m",  "\e[1;35m",  "\e[1;35m",
	 "\e[1;31m",  "\e[1;31m",  "\e[1;31m", "\e[22;35m",  "\e[1;35m",  "\e[1;35m",
	"\e[22;33m", "\e[22;33m", "\e[22;33m",  "\e[1;31m",  "\e[1;35m",  "\e[1;35m",
	 "\e[1;33m",  "\e[1;33m",  "\e[1;33m",  "\e[1;33m",  "\e[1;37m",  "\e[1;37m",
	 "\e[1;33m",  "\e[1;33m",  "\e[1;33m",  "\e[1;33m",  "\e[1;37m",  "\e[1;37m",

	 "\e[1;31m",  "\e[1;31m",  "\e[1;31m",  "\e[1;35m",  "\e[1;35m",  "\e[1;35m",
	 "\e[1;31m",  "\e[1;31m",  "\e[1;31m",  "\e[1;35m",  "\e[1;35m",  "\e[1;35m",
	 "\e[1;31m",  "\e[1;31m",  "\e[1;31m",  "\e[1;31m",  "\e[1;35m",  "\e[1;35m",
	 "\e[1;33m",  "\e[1;33m",  "\e[1;31m",  "\e[1;31m",  "\e[1;35m",  "\e[1;35m",
	 "\e[1;33m",  "\e[1;33m",  "\e[1;33m",  "\e[1;33m",  "\e[1;37m",  "\e[1;37m",
	 "\e[1;33m",  "\e[1;33m",  "\e[1;33m",  "\e[1;33m",  "\e[1;37m",  "\e[1;37m",

	 "\e[1;30m",  "\e[1;30m",  "\e[1;30m",  "\e[1;30m",  "\e[1;30m",  "\e[1;30m",
	 "\e[1;30m",  "\e[1;30m",  "\e[1;30m",  "\e[1;30m",  "\e[1;30m",  "\e[1;30m",
	"\e[22;37m", "\e[22;37m", "\e[22;37m", "\e[22;37m", "\e[22;37m", "\e[22;37m",
	 "\e[1;37m",  "\e[1;37m",  "\e[1;37m",  "\e[1;37m",  "\e[1;37m",  "\e[1;37m"
};

char *c256to16_bg[256] =
{
	"\e[22;40m", "\e[22;41m", "\e[22;42m", "\e[22;43m", "\e[22;44m", "\e[22;45m", "\e[22;46m", "\e[22;47m",
	 "\e[1;40m",  "\e[1;41m",  "\e[1;42m",  "\e[1;43m",  "\e[1;44m",  "\e[1;45m",  "\e[1;46m",  "\e[1;47m",

	"\e[22;40m", "\e[22;44m", "\e[22;44m", "\e[22;44m",  "\e[1;44m",  "\e[1;44m",
	"\e[22;42m", "\e[22;46m", "\e[22;46m", "\e[22;44m",  "\e[1;44m",  "\e[1;44m",
	"\e[22;42m", "\e[22;46m", "\e[22;46m", "\e[22;46m",  "\e[1;44m",  "\e[1;44m",
	"\e[22;42m", "\e[22;42m", "\e[22;46m", "\e[22;46m", "\e[22;46m",  "\e[1;46m",
	 "\e[1;42m",  "\e[1;42m",  "\e[1;42m", "\e[22;46m",  "\e[1;46m",  "\e[1;46m",
	 "\e[1;42m",  "\e[1;42m",  "\e[1;42m",  "\e[1;46m",  "\e[1;46m",  "\e[1;46m",

	"\e[22;41m", "\e[22;45m", "\e[22;45m", "\e[22;44m",  "\e[1;44m",  "\e[1;44m",
	"\e[22;43m",  "\e[1;40m", "\e[22;44m", "\e[22;44m",  "\e[1;44m",  "\e[1;44m",
	"\e[22;43m", "\e[22;42m", "\e[22;46m", "\e[22;46m",  "\e[1;44m",  "\e[1;44m",
	"\e[22;42m", "\e[22;42m", "\e[22;46m", "\e[22;46m", "\e[22;46m",  "\e[1;46m",
	 "\e[1;42m",  "\e[1;42m",  "\e[1;42m", "\e[22;46m",  "\e[1;46m",  "\e[1;46m",
	 "\e[1;42m",  "\e[1;42m",  "\e[1;42m",  "\e[1;46m",  "\e[1;46m",  "\e[1;46m",

	"\e[22;41m", "\e[22;45m", "\e[22;45m", "\e[22;45m",  "\e[1;44m",  "\e[1;44m",
	"\e[22;43m", "\e[22;41m", "\e[22;45m", "\e[22;45m",  "\e[1;44m",  "\e[1;44m",
	"\e[22;43m", "\e[22;43m", "\e[22;47m", "\e[22;44m",  "\e[1;44m",  "\e[1;44m",
	"\e[22;43m", "\e[22;43m", "\e[22;42m", "\e[22;46m", "\e[22;46m",  "\e[1;44m",
	 "\e[1;42m",  "\e[1;42m",  "\e[1;42m", "\e[22;46m",  "\e[1;46m",  "\e[1;46m",
	 "\e[1;42m",  "\e[1;42m",  "\e[1;42m",  "\e[1;42m",  "\e[1;46m",  "\e[1;46m",

	"\e[22;41m", "\e[22;41m", "\e[22;45m", "\e[22;45m", "\e[22;45m",  "\e[1;45m",
	"\e[22;41m", "\e[22;41m", "\e[22;45m", "\e[22;45m", "\e[22;45m",  "\e[1;45m",
	"\e[22;43m", "\e[22;43m", "\e[22;41m", "\e[22;45m", "\e[22;45m",  "\e[1;44m",
	"\e[22;43m", "\e[22;43m", "\e[22;43m", "\e[22;47m",  "\e[1;44m",  "\e[1;44m",
	"\e[22;43m", "\e[22;43m", "\e[22;43m",  "\e[1;42m",  "\e[1;46m",  "\e[1;46m",
	 "\e[1;43m",  "\e[1;43m",  "\e[1;42m",  "\e[1;42m",  "\e[1;46m",  "\e[1;46m",

	 "\e[1;41m",  "\e[1;41m",  "\e[1;41m", "\e[22;45m",  "\e[1;45m",  "\e[1;45m",
	 "\e[1;41m",  "\e[1;41m",  "\e[1;41m", "\e[22;45m",  "\e[1;45m",  "\e[1;45m",
	 "\e[1;41m",  "\e[1;41m",  "\e[1;41m", "\e[22;45m",  "\e[1;45m",  "\e[1;45m",
	"\e[22;43m", "\e[22;43m", "\e[22;43m",  "\e[1;41m",  "\e[1;45m",  "\e[1;45m",
	 "\e[1;43m",  "\e[1;43m",  "\e[1;43m",  "\e[1;43m",  "\e[1;47m",  "\e[1;47m",
	 "\e[1;43m",  "\e[1;43m",  "\e[1;43m",  "\e[1;43m",  "\e[1;47m",  "\e[1;47m",

	 "\e[1;41m",  "\e[1;41m",  "\e[1;41m",  "\e[1;45m",  "\e[1;45m",  "\e[1;45m",
	 "\e[1;41m",  "\e[1;41m",  "\e[1;41m",  "\e[1;45m",  "\e[1;45m",  "\e[1;45m",
	 "\e[1;41m",  "\e[1;41m",  "\e[1;41m",  "\e[1;41m",  "\e[1;45m",  "\e[1;45m",
	 "\e[1;43m",  "\e[1;43m",  "\e[1;41m",  "\e[1;41m",  "\e[1;45m",  "\e[1;45m",
	 "\e[1;43m",  "\e[1;43m",  "\e[1;43m",  "\e[1;43m",  "\e[1;47m",  "\e[1;47m",
	 "\e[1;43m",  "\e[1;43m",  "\e[1;43m",  "\e[1;43m",  "\e[1;47m",  "\e[1;47m",

	 "\e[1;40m",  "\e[1;40m",  "\e[1;40m",  "\e[1;40m",  "\e[1;40m",  "\e[1;40m",
	 "\e[1;40m",  "\e[1;40m",  "\e[1;40m",  "\e[1;40m",  "\e[1;40m",  "\e[1;40m",
	"\e[22;47m", "\e[22;47m", "\e[22;47m", "\e[22;47m", "\e[22;47m", "\e[22;47m",
	 "\e[1;47m",  "\e[1;47m",  "\e[1;47m",  "\e[1;47m",  "\e[1;47m",  "\e[1;47m"
};


// input A to F

int c256_val(char chr)
{
	static unsigned char c256_val[256] =
	{
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   1,   2,    3,   4,   5,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   1,   2,    3,   4,   5,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0
	};

	return (int) c256_val[(unsigned char) chr];
}

// input 00 to FF

int c4096_val(char chr1, char chr2)
{
	static unsigned char c4096_val[256] =
	{
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   1,   2,   3,    4,   5,   6,   7,   8,   9,    0,   0,   0,   0,   0,   0,
		  0,  10,  11,  12,   13,  14,  15,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,  10,  11,  12,   13,  14,  15,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0
	};

	return (int) c4096_val[(unsigned char) chr1] * 16 + c4096_val[(unsigned char) chr2];
}

// input 00 to FF

int c4096_to_256_val(char chr1, char chr2)
{
	static unsigned char c4096_to_256[256] =
	{
		  0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
		  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
		  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
		  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
		  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
		  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
		  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
		  2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
		  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
		  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
		  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
		  4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
		  4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
		  4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
		  5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
	};

	return (int) c4096_to_256[c4096_val(chr1, chr2)];
}

char int_to_hex[16] =
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

char int_to_256[6] =
{
	'A', 'B', 'C', 'D', 'E', 'F'
};

void c4096_rnd(struct session *ses, char *str)
{
	sprintf(str, "%c%c%c", int_to_hex[generate_rand(ses) % 16], int_to_hex[generate_rand(ses) % 16], int_to_hex[generate_rand(ses) % 16]);
}

int is_c32(char chr)
{
	static unsigned char c32_lookup[256] =
	{
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  '?',
		  0,  'A',  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
		  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  'Z',  0,   0,   0,   0,   0,
		  0,  'a',  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
		  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  'z',  0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
	};

	return (int) c32_lookup[(unsigned char) chr];
}

char *c32_fg_dark[26] =
{
	"<f06b>", "<f00b>", "<f0bb>", "", "<f000>", "", "<f0b0>", "", "", "<f0b6>", "", "<f6b0>", "<fb0b>",
	"", "<fb60>", "<fb06>", "", "<fb00>", "<f888>", "<f860>", "", "<f60b>", "<fbbb>", "", "<fbb0>", ""
};

char *c32_fg_bold[26] =
{
	"<f08f>", "<f00f>", "<f0ff>", "", "<f666>", "", "<f0f0>", "", "", "<f0f8>", "", "<f8f0>", "<ff0f>",
	"", "<ff80>", "<ff08>", "", "<ff00>", "<fddd>", "<fdb0>", "", "<f80f>", "<ffff>", "", "<fff0>", ""
};

int valid_escape(struct session *ses, char *str)
{
	switch (*str)
	{
		case '0':
		case 'a':
		case 'c':
		case 'e':
		case 'f':
		case 'n':
		case 'r':
		case 't':
		case 'x':
		case 'u':
		case 'U':
		case 'v':
		case ';':
		case '$':
		case '@':
		case '*':
		case '&':
		case '\\':
			return TRUE;
	}
	return FALSE;
}

int is_variable(struct session *ses, char *str)
{
	struct listroot *root;
	char temp[BUFFER_SIZE], *ptt;
	int i = 1;

	while (str[i] == str[0])
	{
		i++;
	}

	if (str[i] == DEFAULT_OPEN)
	{
		return TRUE;
	}

	if (str[i] != '_' && isalpha((int) str[i]) == 0)
	{
		return FALSE;
	}

	ptt = temp;

	while (isalnum((int) str[i]) || str[i] == '_')
	{
		*ptt++ = str[i];

		i++;
	}
	*ptt = 0;

	root = local_list(ses);

	if (search_node_list(root, temp) == NULL)
	{
		root = ses->list[LIST_VARIABLE];

		if (search_node_list(root, temp) == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}

int is_function(struct session *ses, char *str)
{
	char temp[BUFFER_SIZE], *ptt;
	int i = 1;

	while (str[i] == str[0])
	{
		i++;
	}

	if (str[i] != '_' && isalpha((int) str[i]) == 0)
	{
		return FALSE;
	}

	ptt = temp;

	while (isalnum((int) str[i]) || str[i] == '_')
	{
		*ptt++ = str[i];

		i++;
	}
	*ptt = 0;

	if (str[i] != DEFAULT_OPEN)
	{
		return FALSE;
	}

	if (search_node_list(ses->list[LIST_FUNCTION], temp) == NULL)
	{
		if (find_session(temp) == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}



char *fuzzy_char(struct session *ses, char val1, char val2, int mode)
{
	static char out[10][3];
	static int cnt;
	int tmp;

	cnt = (cnt + 1) % 10;

	switch (mode)
	{
		case 8:
			tmp = c256_val(val2) - 1 + generate_rand(ses) % 3;

			tmp = URANGE(0, tmp, 5);

			if (isupper(val2))
			{
				sprintf(out[cnt], "%c", toupper(int_to_256[tmp]));
			}
			else
			{
				sprintf(out[cnt], "%c", tolower(int_to_256[tmp]));
			}
			break;

		case 12:
			tmp = c4096_val(0, val2);

			tmp = tmp - 2 + generate_rand(ses) % 4;

			tmp = URANGE(0, tmp, 15);

			sprintf(out[cnt], "%c", int_to_hex[tmp]);
			break;

		case 24:
			tmp = c4096_val(val1, val2);

			tmp = (tmp - 12 + generate_rand(ses) % 24);

			tmp = URANGE(0, tmp, 255);

			sprintf(out[cnt], "%c%c", int_to_hex[tmp / 16], int_to_hex[tmp % 16]);
			break;

		default:
			tintin_printf2(ses, "dim_char: invalid mode");
			break;
	}
	return out[cnt];
}

char *fuzzy_color_code(struct session *ses, char *in)
{
	static char *pto, out[10][100];
	static int cnt;
	char *pti, buf[100];
	int tmp;

	if (*in == 0 || strlen(in) > 50)
	{
		return "";
	}

	strcpy(buf, in);

	pti = buf;
	cnt = (cnt + 1) % 10;
	pto = out[cnt];

	while (*pti)
	{
		if (pti[0] == '<')
		{
			if (pti[1] == 0 || pti[2] == 0 || pti[3] == 0 || pti[4] == 0)
			{
				return out[cnt];
			}

			if (pti[4] == '>')
			{
				if (isdigit((int) pti[1]) && isdigit((int) pti[2]) && isdigit((int) pti[3]))
				{
					pto += sprintf(pto, "<%c%c%c>", pti[1], pti[2], pti[3]);
				}
				else if (pti[1] >= 'a' && pti[1] <= 'f' && pti[2] >= 'a' && pti[2] <= 'f' && pti[3] >= 'a' && pti[3] <= 'f')
				{
					pto += sprintf(pto, "<%s%s%s>", fuzzy_char(ses, 0, pti[1], 8), fuzzy_char(ses, 0, pti[2], 8), fuzzy_char(ses, 0, pti[3], 8));
				}
				else if (pti[1] >= 'A' && pti[1] <= 'F' && pti[2] >= 'A' && pti[2] <= 'F' && pti[3] >= 'A' && pti[3] <= 'F')
				{
					pto += sprintf(pto, "<%s%s%s>", fuzzy_char(ses, 0, pti[1], 8), fuzzy_char(ses, 0, pti[2], 8), fuzzy_char(ses, 0, pti[3], 8));
				}
				else if ((pti[1] == 'g' || pti[1] == 'G') && isdigit((int) pti[2]) && isdigit((int) pti[3]))
				{
					tmp = (pti[2] - '0') * 10 + (pti[3] - '0') - 3 + generate_rand(ses) % 7;

					pto += sprintf(pto, "<%c%02d>", pti[1], URANGE(0, tmp, 23));
				}
				else
				{
					return out[cnt];
				}
				pti += 5;
			}
			else if (toupper((int) pti[1]) == 'F')
			{
				if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && pti[5] == '>')
				{
					pto += sprintf(pto, "<F%s%s%s>", fuzzy_char(ses, 0, pti[2], 12), fuzzy_char(ses, 0, pti[3], 12), fuzzy_char(ses, 0, pti[4], 12));

					pti += 6;
				}
				else if (pti[2] == '?' && pti[3] == '?' && pti[4] == '?' && pti[5] == '>')
				{
					c4096_rnd(ses, &pti[2]);

					pto += sprintf(pto, "<F%c%c%c>", pti[2], pti[3], pti[4]);

					pti += 6;
				}
				else if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
				{
					pto += sprintf(pto, "<F%s%s%s>", fuzzy_char(ses, pti[3], pti[4], 24), fuzzy_char(ses, pti[4], pti[5], 24), fuzzy_char(ses, pti[7], pti[7], 24));

					pti += 9;
				}
				else
				{
					return out[cnt];
				}
			}
			else if (toupper((int) pti[1]) == 'B')
			{
				if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && pti[5] == '>')
				{
					pto += sprintf(pto, "<B%s%s%s>", fuzzy_char(ses, 0, pti[2], 12), fuzzy_char(ses, 0, pti[3], 12), fuzzy_char(ses, 0, pti[4], 12));

					pti += 6;
				}
				else if (pti[2] == '?' && pti[3] == '?' && pti[4] == '?' && pti[5] == '>')
				{
					c4096_rnd(ses, &pti[2]);

					pto += sprintf(pto, "<B%c%c%c>", pti[2], pti[3], pti[4]);

					pti += 6;
				}
				else if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
				{
					pto += sprintf(pto, "<B%s%s%s>", fuzzy_char(ses, pti[2], pti[3], 24), fuzzy_char(ses, pti[4], pti[5], 24), fuzzy_char(ses, pti[6], pti[7], 24));

					pti += 9;
				}
				else
				{
					return out[cnt];
				}
			}
			else
			{
				return out[cnt];
			}
		}
		else
		{
			return out[cnt];
		}
	}
	return out[cnt];
}

char *dim_char(struct session *ses, char val1, char val2, int mod, int mode)
{
	static char out[10][3];
	static int cnt;
	int tmp;

	cnt = (cnt + 1) % 10;

	switch (mode)
	{
		case 8:
			tmp = c256_val(val2) - mod;

			tmp = URANGE(0, tmp, 5);

			if (isupper(val2))
			{
				sprintf(out[cnt], "%c", toupper(int_to_256[tmp]));
			}
			else
			{
				sprintf(out[cnt], "%c", tolower(int_to_256[tmp]));
			}
			break;

		case 12:
			tmp = c4096_val(0, val2);

			tmp = tmp - mod - generate_rand(ses) % 3;

			tmp = URANGE(0, tmp, 15);

			sprintf(out[cnt], "%c", int_to_hex[tmp]);
			break;

		case 24:
			tmp = c4096_val(val1, val2);

			tmp = tmp - mod - generate_rand(ses) % 2;

			tmp = URANGE(0, tmp, 255);

			sprintf(out[cnt], "%c%c", int_to_hex[tmp / 16], int_to_hex[tmp % 16]);
			break;

		default:
			tintin_printf2(ses, "dim_char: invalid mode");
			break;
	}
	return out[cnt];
}

char *dim_color_code(struct session *ses, char *in, int mod)
{
	static char *pto, out[10][60], buf[60];
	char *pti;
	static int cnt;
	int tmp;

	if (*in == 0 || strlen(in) > 40)
	{
		return "";
	}

	strcpy(buf, in);

	pti = buf;
	cnt = (cnt + 1) % 10;
	pto = out[cnt];

	if (mod < 0)
	{
		strcpy(pto, buf);

		return out[cnt];
	}

	*pto = 0;

	while (*pti)
	{
		if (pti[0] == '<')
		{
			if (pti[1] == 0 || pti[2] == 0 || pti[3] == 0 || pti[4] == 0)
			{
				return out[cnt];
			}

			if (pti[4] == '>')
			{
				if (isdigit((int) pti[1]) && isdigit((int) pti[2]) && isdigit((int) pti[3]))
				{
					pto += sprintf(pto, "<%c%c%c>", pti[1], pti[2], pti[3]);
				}
				else if (pti[1] >= 'a' && pti[1] <= 'f' && pti[2] >= 'a' && pti[2] <= 'f' && pti[3] >= 'a' && pti[3] <= 'f')
				{
					pto += sprintf(pto, "<%s%s%s>", dim_char(ses, 0, pti[1], mod, 8), dim_char(ses, 0, pti[2], mod, 8), dim_char(ses, 0, pti[3], mod, 8));
				}
				else if (pti[1] >= 'A' && pti[1] <= 'F' && pti[2] >= 'A' && pti[2] <= 'F' && pti[3] >= 'A' && pti[3] <= 'F')
				{
					pto += sprintf(pto, "<%s%s%s>", dim_char(ses, 0, pti[1], mod, 8), dim_char(ses, 0, pti[2], mod, 8), dim_char(ses, 0, pti[3], mod, 8));
				}
				else if ((pti[1] == 'g' || pti[1] == 'G') && isdigit((int) pti[2]) && isdigit((int) pti[3]))
				{
					tmp = (pti[2] - '0') * 10 + (pti[3] - '0') - mod;

					pto += sprintf(pto, "<%c%02d>", pti[1], URANGE(0, tmp, 23));
				}
				else
				{
					return out[cnt];
				}
				pti += 5;
			}
			else if (toupper((int) pti[1]) == 'F')
			{
				if (pti[2] == '?' && pti[3] == '?' && pti[4] == '?' && pti[5] == '>')
				{
					c4096_rnd(ses, &pti[2]);
				}

				if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && pti[5] == '>')
				{
					pto += sprintf(pto, "<F%s%s%s>", dim_char(ses, 0, pti[2], mod, 12), dim_char(ses, 0, pti[3], mod, 12), dim_char(ses, 0, pti[4], mod, 12));

					pti += 6;
				}
				else if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
				{
					pto += sprintf(pto, "<F%s%s%s>", dim_char(ses, pti[2], pti[3], mod, 24), dim_char(ses, pti[4], pti[5], mod, 24), dim_char(ses, mod, pti[6], pti[7], 24));

					pti += 9;
				}
				else
				{
					return out[cnt];
				}
			}
			else if (toupper((int) pti[1]) == 'B')
			{
				if (pti[2] == '?' && pti[3] == '?' && pti[4] == '?' && pti[5] == '>')
				{
					c4096_rnd(ses, &pti[2]);
				}

				if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && pti[5] == '>')
				{
					pto += sprintf(pto, "<B%s%s%s>", dim_char(ses, 0, pti[2], mod, 12), dim_char(ses, 0, pti[3], mod, 12), dim_char(ses, 0, pti[4], mod, 12));

					pti += 6;
				}
				else if (toupper((int) pti[1]) == 'B' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
				{
					pto += sprintf(pto, "<B%s%s%s>", dim_char(ses, pti[2], pti[3], mod, 24), dim_char(ses, pti[4], pti[5], mod, 24), dim_char(ses, pti[6], pti[7], mod, 24));

					pti += 9;
				}
				else
				{
					return out[cnt];
				}
			}
			else
			{
				return out[cnt];
			}
		}
	}
	return out[cnt];

}

char lit_char(struct session *ses, char max, char val, int mod, int mode)
{
	int tmp;

	switch (mode)
	{
		case 8:
			tmp = val + mod;

			return UMIN(max, tmp);

		case 12:
			tmp = c4096_val(0, val);

			tmp = URANGE(0, tmp + mod, 15);

			return int_to_hex[tmp];
	}
	return val;
}

char *lit_color_code(struct session *ses, char *pti, int mod)
{
	static char fuzzy[10][20];
	static int cnt;
	int tmp;

	cnt = (cnt + 1) % 10;

	if (pti[0] == '<')
	{
		if (pti[1] == 0 || pti[2] == 0 || pti[3] == 0 || pti[4] == 0)
		{
			return "";
		}

		if (pti[4] == '>')
		{
			if (isdigit((int) pti[1]) && isdigit((int) pti[2]) && isdigit((int) pti[3]))
			{
				sprintf(fuzzy[cnt], "<%c%c%c>%.9s", pti[1], pti[2], pti[3], &pti[5]);

				return fuzzy[cnt];
			}

			if (pti[1] >= 'a' && pti[1] <= 'f' && pti[2] >= 'a' && pti[2] <= 'f' && pti[3] >= 'a' && pti[3] <= 'f')
			{
				sprintf(fuzzy[cnt], "<%c%c%c>%.9s", lit_char(ses, 'f', pti[1], mod, 8), lit_char(ses, 'f', pti[2], mod, 8), lit_char(ses, 'f', pti[3], mod, 8), &pti[5]);

				return fuzzy[cnt];
			}

			if (pti[1] >= 'A' && pti[1] <= 'F' && pti[2] >= 'A' && pti[2] <= 'F' && pti[3] >= 'A' && pti[3] <= 'F')
			{
				sprintf(fuzzy[cnt], "<%c%c%c>%.9s", lit_char(ses, 'F', pti[1], mod, 8), lit_char(ses, 'F', pti[2], mod, 8), lit_char(ses, 'F', pti[3], mod, 8), &pti[5]);

				return fuzzy[cnt];
			}
		}

		{
			if (pti[1] == 'g' || pti[1] == 'G')
			{
				if (isdigit((int) pti[2]) && isdigit((int) pti[3]))
				{
					tmp = (pti[2] - '0') * 10 + (pti[3] - '0');

					tmp -= mod;

					sprintf(fuzzy[cnt], "<%c%02d>%.9s", pti[1], URANGE(0, tmp, 23), &pti[5]);

					return fuzzy[cnt];
				}
				return "";
			}
		}

		if (pti[5] == 0)
		{
			return "";
		}

		if (toupper((int) pti[1]) == 'F')
		{
			if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && pti[5] == '>')
			{
				sprintf(fuzzy[cnt], "<F%c%c%c>%.9s", lit_char(ses, 'F', pti[2], mod, 12), lit_char(ses, 'F', pti[3], mod, 12), lit_char(ses, 'F', pti[4], mod, 12), &pti[6]);

				return fuzzy[cnt];
			}
			else if (pti[2] == '?' && pti[3] == '?' && pti[4] == '?' && pti[5] == '>')
			{
				sprintf(fuzzy[cnt], "<F??%c>%.9s", '?', &pti[6]);

				c4096_rnd(ses, &fuzzy[cnt][2]);

				return dim_color_code(ses, fuzzy[cnt], mod);
			}
			else if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
			{
				sprintf(fuzzy[cnt], "<F%c%c%c>%.9s", lit_char(ses, 'F', pti[2], mod, 12), lit_char(ses, 'F', pti[4], mod, 12), lit_char(ses, mod, pti[6], 'F', 12), &pti[9]);

				return fuzzy[cnt];
			}
			return "";
		}

		if (toupper((int) pti[1]) == 'B')
		{
			if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && pti[5] == '>')
			{
				sprintf(fuzzy[cnt], "<B%c%c%c>%.9s", lit_char(ses, 'F', pti[2], mod, 12), lit_char(ses, 'F', pti[3], mod, 12), lit_char(ses, 'F', pti[4], mod, 12), &pti[6]);

				return fuzzy[cnt];
			}
			if (toupper((int) pti[1]) == 'B' && pti[2] == '?' && pti[3] == '?' && pti[4] == '?' && pti[5] == '>')
			{
				sprintf(fuzzy[cnt], "<B??%c>%.9s", '?', &pti[6]);

				c4096_rnd(ses, &fuzzy[cnt][2]);

				return dim_color_code(ses, fuzzy[cnt], mod);
			}
			if (toupper((int) pti[1]) == 'B' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
			{
				sprintf(fuzzy[cnt], "<B%c%c%c>%.9s", lit_char(ses, 'F', pti[2], mod, 12), lit_char(ses, 'F', pti[4], mod, 12), lit_char(ses, mod, pti[6], 'F', 12), &pti[9]);

				return fuzzy[cnt];
			}
			return "";
		}
	}
	return "";
}

int substitute(struct session *ses, char *string, char *result, int flags)
{
	struct listnode *node;
	struct listroot *root;
	struct session *sesptr;
	char temp[BUFFER_SIZE], buf[BUFFER_SIZE], buffer[BUFFER_SIZE], *pti, *pto, *ptt, *str;
	char *pte, old[10] = { 0 };
	int i, skip, cnt, escape = FALSE, flags_neol = flags;

	push_call("substitute(%p,%p,%p,%d)",ses,string,result,flags);

	pti = string;
	pto = (string == result) ? buffer : result;

	DEL_BIT(flags_neol, SUB_EOL|SUB_LNF);

	while (TRUE)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		switch (*pti)
		{
			case '\0':
				if (HAS_BIT(flags, SUB_EOL))
				{
					if (HAS_BIT(ses->flags, SES_FLAG_RUN))
					{
						*pto++ = '\r';
					}
					else
					{
						*pto++ = '\r';
						*pto++ = '\n';
					}
				}

				if (HAS_BIT(flags, SUB_LNF))
				{
					*pto++ = '\n';
				}

				*pto = 0;

				if (string == result)
				{
					strcpy(result, buffer);

					pop_call();
					return pto - buffer;
				}
				else
				{
					pop_call();
					return pto - result;
				}
				break;

			case '@':
				if (HAS_BIT(flags, SUB_FUN) && !HAS_BIT(ses->list[LIST_FUNCTION]->flags, LIST_FLAG_IGNORE))
				{
					i = 1;
					escape = FALSE;
					sesptr = NULL;

					while (pti[i] == '@')
					{
						escape = TRUE;

						i++;
					}

					for (ptt = temp ; isalnum((int) pti[i]) || pti[i] == '_' ; i++)
					{
						*ptt++ = pti[i];
					}
					*ptt = 0;

					if (pti[i] != DEFAULT_OPEN)
					{
						while (*pti == '@')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					node = search_node_list(ses->list[LIST_FUNCTION], temp);

					if (node == NULL)
					{
						sesptr = find_session(temp);
					}

					if (sesptr == NULL && node == NULL)
					{
						while (*pti == '@')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (escape)
					{
						pti++;

						while (*pti == '@')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					pti = get_arg_in_braces(ses, &pti[i], temp, GET_ONE);

					if (sesptr)
					{
						substitute(sesptr, temp, pto, flags_neol);

						pto += strlen(pto);

						continue;
					}
					else
					{
						substitute(ses, temp, buf, flags_neol);
					}

					show_debug(ses, LIST_FUNCTION, "#DEBUG FUNCTION {%s}", node->arg1);

					RESTRING(gtd->vars[0], buf);

					pte = buf;

					for (i = 1 ; i < 100 ; i++)
					{
						pte = get_arg_in_braces(ses, pte, temp, GET_ALL);

						RESTRING(gtd->vars[i], temp);

						if (*pte == 0)
						{
							while (++i < 100)
							{
								if (*gtd->vars[i])
								{
									RESTRING(gtd->vars[i], "");
								}
							}
							break;
						}

						if (*pte == COMMAND_SEPARATOR)
						{
							pte++;
						}

					}

					substitute(ses, node->arg2, buf, SUB_ARG);

					if (node->shots && --node->shots == 0)
					{
						delete_node_list(ses, LIST_FUNCTION, node);
					}

					script_driver(ses, LIST_FUNCTION, buf);

					substitute(ses, "$result", pto, flags_neol|SUB_VAR);

					pto += strlen(pto);
				}
				else
				{
					if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG) && is_function(ses, pti))
					{
						*pto++ = '\\';
					}
					*pto++ = *pti++;
				}
				break;

			case '*':
				if (HAS_BIT(flags, SUB_VAR) && !HAS_BIT(ses->list[LIST_VARIABLE]->flags, LIST_FLAG_IGNORE) && pti[1])
				{
					int brace = FALSE;
					i = 1;
					escape = FALSE;

					while (pti[i] == '*')
					{
						escape = TRUE;

						i++;
					}

					if (pti[i] == DEFAULT_OPEN)
					{
						brace = TRUE;

						ptt = get_arg_in_braces(ses, &pti[i], buf, GET_ALL);

						i += strlen(buf) + 2;

						substitute(ses, buf, temp, flags_neol);
					}
					else
					{
						ptt = temp;

						while (isalnum((int) pti[i]) || pti[i] == '_')
						{
							*ptt++ = pti[i];

							i++;
						}
						*ptt = 0;
					}

					if (*temp)
					{
						root = local_list(ses);

						if ((node = search_node_list(root, temp)) == NULL)
						{
							root = ses->list[LIST_VARIABLE];

							node = search_node_list(root, temp);
						}
					}
					else
					{
						root = ses->list[LIST_VARIABLE];
						node = NULL;
					}

					if (brace == FALSE && node == NULL)
					{
						while (*pti == '*')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (escape)
					{
						pti++;

						while (*pti == '*')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (brace == FALSE)
					{
						pti = get_arg_at_brackets(ses, &pti[i], temp + strlen(temp));
					}
					else
					{
						pti = ptt;
					}

					substitute(ses, temp, buf, flags_neol);

					str = str_dup("");

					get_nest_node_key(root, buf, &str, brace);

					substitute(ses, str, pto, flags_neol - SUB_VAR);

					pto += strlen(pto);

					str_free(str);
				}
				else
				{
					if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG) && is_variable(ses, pti))
					{
						*pto++ = '\\';
					}
					*pto++ = *pti++;
				}
				break;

			case '$':
				if (HAS_BIT(flags, SUB_VAR) && !HAS_BIT(ses->list[LIST_VARIABLE]->flags, LIST_FLAG_IGNORE) && pti[1])
				{
					int brace = FALSE;
					i = 1;
					escape = FALSE;

					while (pti[i] == '$')
					{
						escape = TRUE;

						i++;
					}

					if (pti[i] == DEFAULT_OPEN)
					{
						brace = TRUE;

						ptt = get_arg_in_braces(ses, &pti[i], buf, GET_ALL);

						i += strlen(buf) + 2;

						substitute(ses, buf, temp, flags_neol);
					}
					else
					{
						ptt = temp;

						while (isalnum((int) pti[i]) || pti[i] == '_')
						{
							*ptt++ = pti[i];

							i++;
						}
						*ptt = 0;
					}

					if (*temp)
					{
						root = search_nest_base_ses(ses, temp);

						if (root)
						{
							node = search_node_list(root, temp);
						}
						else
						{
							root = ses->list[LIST_VARIABLE];
							node = NULL;
						}
					}
					else
					{
						root = ses->list[LIST_VARIABLE];
						node = NULL;
					}

					if (brace == FALSE && node == NULL)
					{
						while (*pti == '$')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (escape)
					{
						pti++;

						while (*pti == '$')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (brace == FALSE)
					{
						pti = get_arg_at_brackets(ses, &pti[i], temp + strlen(temp));
					}
					else
					{
						pti = ptt;
					}

					substitute(ses, temp, buf, flags_neol);

					str = str_dup("");

					get_nest_node_val(root, buf, &str, brace);

					substitute(ses, str, pto, flags_neol - SUB_VAR);

					pto += strlen(pto);

					str_free(str);
				}
				else
				{
					if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG) && is_variable(ses, pti))
					{
						*pto++ = '\\';
					}
					*pto++ = *pti++;
				}
				break;

			case '&':
				if (HAS_BIT(flags, SUB_CMD) && (isdigit((int) pti[1]) || pti[1] == '&'))
				{
					if (pti[1] == '&')
					{
						while (pti[1] == '&')
						{
							*pto++ = *pti++;
						}
						if (isdigit((int) pti[1]))
						{
							pti++;
						}
						else
						{
							*pto++ = *pti++;
						}
					}
					else
					{
						i = isdigit((int) pti[2]) ? (pti[1] - '0') * 10 + pti[2] - '0' : pti[1] - '0';

						for (cnt = 0 ; gtd->cmds[i][cnt] ; cnt++)
						{
							*pto++ = gtd->cmds[i][cnt];
						}
						pti += isdigit((int) pti[2]) ? 3 : 2;
					}
				}
				else if (HAS_BIT(flags, SUB_VAR) && !HAS_BIT(ses->list[LIST_VARIABLE]->flags, LIST_FLAG_IGNORE))
				{
					int brace = FALSE;
					i = 1;
					escape = FALSE;

					while (pti[i] == '&')
					{
						escape = TRUE;

						i++;
					}

					if (pti[i] == DEFAULT_OPEN)
					{
						brace = TRUE;

						ptt = get_arg_in_braces(ses, &pti[i], buf, GET_ALL);

						i += strlen(buf) + 2;

						substitute(ses, buf, temp, flags_neol);
					}
					else
					{
						ptt = temp;

						while (isalnum((int) pti[i]) || pti[i] == '_')
						{
							*ptt++ = pti[i];

							i++;
						}
						*ptt = 0;
					}

					if (*temp)
					{
						root = local_list(ses);

						if ((node = search_node_list(root, temp)) == NULL)
						{
							root = ses->list[LIST_VARIABLE];
							node = search_node_list(root, temp);
						}
					}
					else
					{
						root = ses->list[LIST_VARIABLE];
						node = NULL;
					}

					if (brace == FALSE && node == NULL)
					{
						while (*pti == '&')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (escape)
					{
						pti++;

						while (*pti == '&')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (brace == FALSE)
					{
						pti = get_arg_at_brackets(ses, &pti[i], temp + strlen(temp));
					}
					else
					{
						pti = ptt;
					}

					substitute(ses, temp, buf, flags_neol);

					str = str_dup("");

					get_nest_index(root, buf, &str, brace);

					substitute(ses, str, pto, flags_neol - SUB_VAR);

					pto += strlen(pto);

					str_free(str);
				}
				else
				{
					if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG) && is_variable(ses, pti))
					{
						*pto++ = '\\';
					}
					*pto++ = *pti++;
				}
				break;

			case '%':
				if (HAS_BIT(flags, SUB_ARG) && (isdigit((int) pti[1]) || pti[1] == '%'))
				{
					if (pti[1] == '%')
					{
						while (pti[1] == '%')
						{
							*pto++ = *pti++;
						}
						pti++;
					}
					else
					{
						i = isdigit((int) pti[2]) ? (pti[1] - '0') * 10 + pti[2] - '0' : pti[1] - '0';

						ptt = gtd->vars[i];

						while (*ptt)
						{
							if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, ptt))
							{
								*pto++ = *ptt++;
								*pto++ = *ptt++;
								continue;
							}

							if (HAS_BIT(flags, SUB_SEC))
							{
								switch (*ptt)
								{
									case '\\':
										*pto++ = '\\';
										*pto++ = '\\';
										break;

									case '{':
										*pto++ = '\\';
										*pto++ = 'x';
										*pto++ = '7';
										*pto++ = 'B';
										break;

									case '}':
										*pto++ = '\\';
										*pto++ = 'x';
										*pto++ = '7';
										*pto++ = 'D';
										break;

									case '$':
									case '&':
									case '*':
										if (is_variable(ses, ptt))
										{
											*pto++ = '\\';
											*pto++ = *ptt;
										}
										else
										{
											*pto++ = *ptt;
										}
										break;

									case '@':
										if (is_function(ses, ptt))
										{
											*pto++ = '\\';
											*pto++ = *ptt;
										}
										else
										{
											*pto++ = *ptt;
										}
										break;

									case COMMAND_SEPARATOR:
										*pto++ = '\\';
										*pto++ = COMMAND_SEPARATOR;
										break;

									default:
										*pto++ = *ptt;
										break;
								}
								ptt++;
							}
							else
							{
								*pto++ = *ptt++;
							}
						}
						pti += isdigit((int) pti[2]) ? 3 : 2;
					}
				}
				else if (pti[1] == '*') // avoid %*variable triggers
				{
					*pto++ = *pti++;
					*pto++ = *pti++;
				}
				else
				{
					*pto++ = *pti++;
				}
				break;

			case '<':
				if (HAS_BIT(flags, SUB_COL))
				{
					if (HAS_BIT(flags, SUB_CMP) && old[0] && !strncmp(old, pti, strlen(old)))
					{
						pti += strlen(old);
					}
					else if (pti[1] && pti[2] && pti[3] && pti[4] == '>')
					{
						if (isdigit((int) pti[1]) && isdigit((int) pti[2]) && isdigit((int) pti[3]))
						{
							if (pti[1] != '8' || pti[2] != '8' || pti[3] != '8')
							{
								*pto++ = ASCII_ESC;
								*pto++ = '[';

								switch (pti[1])
								{
									case '2':
										*pto++ = '2';
										*pto++ = '2';
										*pto++ = ';';
										break;
									case '8':
										break;
									default:
										*pto++ = pti[1];
										*pto++ = ';';
								}
								switch (pti[2])
								{
									case '8':
										break;
									default:
										*pto++ = '3';
										*pto++ = pti[2];
										*pto++ = ';';
										break;
								}
								switch (pti[3])
								{
									case '8':
										break;
									default:
										*pto++ = '4';
										*pto++ = pti[3];
										*pto++ = ';';
										break;
								}
								pto--;
								*pto++ = 'm';
							}
							pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
						}
						else if (pti[1] >= 'a' && pti[1] <= 'f' && pti[2] >= 'a' && pti[2] <= 'f' && pti[3] >= 'a' && pti[3] <= 'f' && pti[4] == '>')
						{
							cnt = 16 + (pti[1] - 'a') * 36 + (pti[2] - 'a') * 6 + (pti[3] - 'a');
							
							if (ses->color >= 256)
							{
								pto += sprintf(pto, "\e[38;5;%dm", cnt);
							}
							else if (ses->color == 16)
							{
								pto += sprintf(pto, "%s", c256to16_fg[cnt]);
							}
							pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
						}
						else if (pti[1] >= 'A' && pti[1] <= 'F' && pti[2] >= 'A' && pti[2] <= 'F' && pti[3] >= 'A' && pti[3] <= 'F' && pti[4] == '>')
						{
							cnt = 16 + (pti[1] - 'A') * 36 + (pti[2] - 'A') * 6 + (pti[3] - 'A');

							if (ses->color >= 256)
							{
								pto += sprintf(pto, "\e[48;5;%dm", cnt);
							}
							else if (ses->color == 16)
							{
								pto += sprintf(pto, "%s", c256to16_bg[cnt]);
							}
							pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
						}
						else if (pti[1] == 'g' && isdigit((int) pti[2]) && isdigit((int) pti[3]) && pti[4] == '>')
						{
							cnt = 232 + (pti[2] - '0') * 10 + (pti[3] - '0');

							if (ses->color >= 256)
							{
								pto += sprintf(pto, "\e[38;5;%dm", cnt);
							}
							else if (ses->color == 16)
							{
								pto += sprintf(pto, "%s", c256to16_fg[cnt]);
							}
							pti += sprintf(old, "<g%c%c>", pti[2], pti[3]);
						}
						else if (pti[1] == 'G' && isdigit((int) pti[2]) && isdigit((int) pti[3]) && pti[4] == '>')
						{
							cnt = 232 + (pti[2] - '0') * 10 + (pti[3] - '0');

							if (ses->color >= 256)
							{
								pto += sprintf(pto, "\e[48;5;%dm", cnt);
							}
							else if (ses->color == 16)
							{
								pto += sprintf(pto, "%s", c256to16_bg[cnt]);
							}
							pti += sprintf(old, "<G%c%c>", pti[2], pti[3]);
						}
						else
						{
							*pto++ = *pti++;
						}
					}
					else if (toupper((int) pti[1]) == 'F' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && pti[5] == '>')
					{
						if (ses->color == 4096)
						{
							pto += sprintf(pto, "\e[38;2;%d;%d;%dm", c4096_val(pti[2], pti[2]), c4096_val(pti[3], pti[3]), c4096_val(pti[4], pti[4]));
						}
						else if (ses->color == 256)
						{
							pto += sprintf(pto, "\033[38;5;%dm",  16 + c4096_to_256_val(pti[2], pti[2]) * 36 + c4096_to_256_val(pti[3], pti[3]) * 6 + c4096_to_256_val(pti[4], pti[4]));
						}
						else if (ses->color == 16)
						{
							pto += sprintf(pto, "%s", c256to16_fg[16 + c4096_to_256_val(pti[2], pti[2]) * 36 + c4096_to_256_val(pti[3], pti[3]) * 6 + c4096_to_256_val(pti[4], pti[4])]);
						}
						pti += sprintf(old, "<F%c%c%c>", pti[2], pti[3], pti[4]);
					}
					else if (toupper((int) pti[1]) == 'F' && pti[2] == '?' && pti[3] == '?' && pti[4] == '?' && pti[5] == '>')
					{
						c4096_rnd(ses, &pti[2]);

						if (ses->color == 4096)
						{
							pto += sprintf(pto, "\e[38;2;%d;%d;%dm", c4096_val(pti[2], pti[2]), c4096_val(pti[3], pti[3]), c4096_val(pti[4], pti[4]));
						}
						else if (ses->color == 256)
						{
							pto += sprintf(pto, "\033[38;5;%dm",  16 + c4096_to_256_val(pti[2], pti[2]) * 36 + c4096_to_256_val(pti[3], pti[3]) * 6 + c4096_to_256_val(pti[4], pti[4]));
						}
						else if (ses->color == 16)
						{
							pto += sprintf(pto, "%s", c256to16_fg[16 + c4096_to_256_val(pti[2], pti[2]) * 36 + c4096_to_256_val(pti[3], pti[3]) * 6 + c4096_to_256_val(pti[4], pti[4])]);
						}
						pti += sprintf(old, "<F%c%c%c>", pti[2], pti[3], pti[4]);
					}
					else if (toupper((int) pti[1]) == 'F' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
					{
						if (ses->color == 4096)
						{
							pto += sprintf(pto, "\e[38;2;%d;%d;%dm", c4096_val(pti[2], pti[3]), c4096_val(pti[4], pti[5]), c4096_val(pti[6], pti[7]));
						}
						else if (ses->color == 256)
						{
							pto += sprintf(pto, "\033[38;5;%dm",  16 + c4096_to_256_val(pti[2], pti[3]) * 36 + c4096_to_256_val(pti[4], pti[5]) * 6 + c4096_to_256_val(pti[6], pti[7]));
						}
						else if (ses->color == 16)
						{
							pto += sprintf(pto, "%s", c256to16_fg[16 + c4096_to_256_val(pti[2], pti[3]) * 36 + c4096_to_256_val(pti[4], pti[5]) * 6 + c4096_to_256_val(pti[6], pti[7])]);
						}
						pti += sprintf(old, "<F%c%c%c%c%c%c>", pti[2], pti[3], pti[4], pti[5], pti[6], pti[7]);
					}
					else if (toupper((int) pti[1]) == 'B' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && pti[5] == '>')
					{
						if (ses->color == 4096)
						{
							pto += sprintf(pto, "\e[48;2;%d;%d;%dm", c4096_val(pti[2], pti[2]), c4096_val(pti[3], pti[3]), c4096_val(pti[4], pti[4]));
						}
						else if (ses->color == 256)
						{
							pto += sprintf(pto, "\033[48;5;%dm",  16 + c4096_to_256_val(pti[2], pti[2]) * 36 + c4096_to_256_val(pti[3], pti[3]) * 6 + c4096_to_256_val(pti[4], pti[4]));
						}
						else if (ses->color == 16)
						{
							pto += sprintf(pto, "%s", c256to16_bg[16 + c4096_to_256_val(pti[2], pti[2]) * 36 + c4096_to_256_val(pti[3], pti[3]) * 6 + c4096_to_256_val(pti[4], pti[4])]);
						}
						pti += sprintf(old, "<B%c%c%c>", pti[2], pti[3], pti[4]);
					}
					else if (toupper((int) pti[1]) == 'B' && pti[2] == '?' && pti[3] == '?' && pti[4] == '?' && pti[5] == '>')
					{
						c4096_rnd(ses, &pti[2]);

						if (ses->color == 4096)
						{
							pto += sprintf(pto, "\e[48;2;%d;%d;%dm", c4096_val(pti[2], pti[2]), c4096_val(pti[3], pti[3]), c4096_val(pti[4], pti[4]));
						}
						else if (ses->color == 256)
						{
							pto += sprintf(pto, "\033[48;5;%dm",  16 + c4096_to_256_val(pti[2], pti[2]) * 36 + c4096_to_256_val(pti[3], pti[3]) * 6 + c4096_to_256_val(pti[4], pti[4]));
						}
						else if (ses->color == 16)
						{
							pto += sprintf(pto, "%s", c256to16_bg[16 + c4096_to_256_val(pti[2], pti[2]) * 36 + c4096_to_256_val(pti[3], pti[3]) * 6 + c4096_to_256_val(pti[4], pti[4])]);
						}
						pti += sprintf(old, "<F%c%c%c>", pti[2], pti[3], pti[4]);
					}
					else if (toupper((int) pti[1]) == 'B' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
					{
						if (ses->color == 4096)
						{
							pto += sprintf(pto, "\e[48;2;%d;%d;%dm", c4096_val(pti[2], pti[3]), c4096_val(pti[4], pti[5]), c4096_val(pti[6], pti[7]));
						}
						else if (ses->color == 256)
						{
							pto += sprintf(pto, "\033[48;5;%dm",  16 + c4096_to_256_val(pti[2], pti[3]) * 36 + c4096_to_256_val(pti[4], pti[5]) * 6 + c4096_to_256_val(pti[6], pti[7]));
						}
						else if (ses->color == 16)
						{
							pto += sprintf(pto, "%s", c256to16_bg[16 + c4096_to_256_val(pti[2], pti[3]) * 36 + c4096_to_256_val(pti[4], pti[5]) * 6 + c4096_to_256_val(pti[6], pti[7])]);
						}
						pti += sprintf(old, "<B%c%c%c%c%c%c>", pti[2], pti[3], pti[4], pti[5], pti[6], pti[7]);
					}
					else
					{
						*pto++ = *pti++;
					}
				}
				else
				{
					*pto++ = *pti++;
				}
				break;


			case '\\':
				if (HAS_BIT(flags, SUB_ESC))
				{
					pti++;

					switch (*pti)
					{
						case 'a':
							*pto++ = '\a';
							break;
						case 'b':
							*pto++ = '\b';
							break;
						case 'c':
							if (pti[1])
							{
								pti++;
								*pto++ = *pti % 32;
							}
							break;
						case 'e':
							*pto++ = '\e';
							break;
						case 'f':
							*pto++ = '\f';
							break;
						case 'n':
							*pto++ = '\n';
							break;
						case 'r':
							*pto++ = '\r';
							break;
						case 't':
							*pto++ = '\t';
							break;
						case 'x':
							if (pti[1] && pti[2])
							{
								if (pti[1] == '0' && pti[2] == '0' && pti[3] == 0)
								{
									pti += 2;
									DEL_BIT(flags, SUB_EOL);
									DEL_BIT(flags, SUB_LNF);
								}
								else
								{
									pti++;
									*pto++ = hex_number_8bit(pti);
									pti++;
								}
							}
							break;

						case 'u':
							if (pti[1] && pti[2] && pti[3] && pti[4])
							{
								pto += unicode_16_bit(&pti[1], pto);
								pti += 4;
							}
							break;

						case 'U':
							if (pti[1] && pti[2] && pti[3] && pti[4] && pti[5] && pti[6])
							{
								pto += unicode_21_bit(&pti[1], pto);
								pti += 6;
							}
							break;

						case 'v':
							*pto++ = '\v';
							break;
						case '0':
							if (pti[1] == 0)
							{
								DEL_BIT(flags, SUB_EOL);
								DEL_BIT(flags, SUB_LNF);
							}
							else if (pti[1] && pti[2])
							{
								pti++;
								*pto++ = oct_number(pti);
								pti++;
							}
							break;

						case '\0':
							DEL_BIT(flags, SUB_EOL);
							DEL_BIT(flags, SUB_LNF);
							continue;

						default:
							*pto++ = *pti;
							break;
					}
					pti++;
				}
				else if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG))
				{
					*pto++ = '\\';
					*pto++ = *pti++;
				}
				else if (HAS_BIT(flags, SUB_LIT))
				{
					*pto++ = *pti++;
				}
				else
				{
					*pto++ = *pti++;

					if (*pti)
					{
						*pto++ = *pti++;
					}
				}
				break;

			case ASCII_ESC:
				if (HAS_BIT(flags, SUB_COL) && ses->color == 0)
				{
					skip = find_color_code(pti);

					if (skip)
					{
						pti += skip;
					}
					else
					{
						*pto++ = *pti++;
					}
				}
				else
				{
					*pto++ = *pti++;
				}
				break;

			default:
				if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG))
				{
					switch (*pti)
					{
						case '{':
							*pto++ = '\\';
							*pto++ = 'x';
							*pto++ = '7';
							*pto++ = 'B';
							break;

						case '}':
							*pto++ = '\\';
							*pto++ = 'x';
							*pto++ = '7';
							*pto++ = 'D';
							break;

						case COMMAND_SEPARATOR:
							*pto++ = '\\';
							*pto++ = COMMAND_SEPARATOR;
							break;

						default:
							*pto++ = *pti;
							break;
					}
					pti++;
				}
				else
				{
					*pto++ = *pti++;
				}
				break;
		}
	}
	pop_call();
	return 0;
}

int is_color_code(char *pti)
{
	if (pti[0] == '<')
	{
		if (pti[1] == 0 || pti[2] == 0 || pti[3] == 0 || pti[4] == 0)
		{
			return 0;
		}

		if (pti[4] == '>')
		{

			if (isdigit((int) pti[1]) && isdigit((int) pti[2]) && isdigit((int) pti[3]))
			{
				return 5;
			}
			if (pti[1] >= 'a' && pti[1] <= 'f' && pti[2] >= 'a' && pti[2] <= 'f' && pti[3] >= 'a' && pti[3] <= 'f')
			{
				return 5;
			}
			if (pti[1] >= 'A' && pti[1] <= 'F' && pti[2] >= 'A' && pti[2] <= 'F' && pti[3] >= 'A' && pti[3] <= 'F')
			{
				return 5;
			}

			if (pti[1] == 'g' || pti[1] == 'G')
			{
				if (isdigit((int) pti[2]) && isdigit((int) pti[3]))
				{
					return 5;
				}
				return 0;
			}

			return 0;
		}

		if (pti[5] == 0)
		{
			return 0;
		}

		if (toupper((int) pti[1]) == 'F')
		{
			if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && pti[5] == '>')
			{
				return 6;
			}
			else if (pti[2] == '?' && pti[3] == '?' && pti[4] == '?' && pti[5] == '>')
			{
				return 6;
			}
			else if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
			{
				return 9;
			}
			return 0;
		}

		if (toupper((int) pti[1]) == 'B')
		{
			if (isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && pti[5] == '>')
			{
				return 6;
			}
			if (toupper((int) pti[1]) == 'B' && pti[2] == '?' && pti[3] == '?' && pti[4] == '?' && pti[5] == '>')
			{
				return 6;
			}
			if (toupper((int) pti[1]) == 'B' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
			{
				return 9;
			}
			return 0;
		}
	}
	return 0;
}

int is_color_name(char *string)
{
	int cnt, skip;

	while (*string)
	{
		switch (*string)
		{
			case ' ':
			case ';':
			case ',':
				string++;
				continue;

			case '<':
				skip = is_color_code(string);

				if (skip == 0)
				{
					return FALSE;
				}
				string += skip;
				continue;

			case '\\':
				skip = find_escaped_color_code(string);

				if (skip == 0)
				{
					return FALSE;
				}
				string += skip;
				continue;

			case '\e':
				skip = find_color_code(string);

				if (skip == 0)
				{
					return FALSE;
				}
				string += skip;
				continue;
		}

		if (isalpha((int) *string))
		{
			for (cnt = 0 ; *color_table[cnt].name ; cnt++)
			{
				if (!strncmp(color_table[cnt].name, string, color_table[cnt].len))
				{
					break;
				}
			}

			if (*color_table[cnt].name == 0)
			{
				for (cnt = 0 ; *color_table[cnt].name ; cnt++)
				{
					if (!strncasecmp(color_table[cnt].name, string, color_table[cnt].len))
					{
						break;
					}
				}

				if (*color_table[cnt].name == 0)
				{
					return FALSE;
				}
			}
			string += strlen(color_table[cnt].name);
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

int translate_color_names(struct session *ses, char *string, char *result)
{
	int cnt, skip;

	*result = 0;

	while (*string)
	{
		switch (*string)
		{
			case ' ':
			case ';':
			case ',':
				string++;
				continue;

			case '<':
				skip = is_color_code(string);

				if (skip == 0)
				{
					return FALSE;
				}
				result += sprintf(result, "%.*s", skip, string);

				string += skip;
				continue;

			case '\\':
				skip = find_escaped_color_code(string);

				if (skip == 0)
				{
					return FALSE;
				}
				result += sprintf(result, "%.*s", skip, string);

				string += skip;
				continue;

			case '\e':
				skip = find_color_code(string);

				if (skip == 0)
				{
					return FALSE;
				}
				result += sprintf(result, "%.*s", skip, string);

				string += skip;
				continue;
		}

		if (isalpha((int) *string))
		{
			for (cnt = 0 ; *color_table[cnt].name ; cnt++)
			{
				if (!strncmp(color_table[cnt].name, string, color_table[cnt].len))
				{
					result += sprintf(result, "%s", color_table[cnt].code);

					break;
				}
			}

			if (*color_table[cnt].name == 0)
			{
				for (cnt = 0 ; *color_table[cnt].name ; cnt++)
				{
					if (!strncasecmp(color_table[cnt].name, string, color_table[cnt].len))
					{
						result += sprintf(result, "%s", color_table[cnt].code);

						break;
					}
				}

				if (*color_table[cnt].name == 0)
				{
					return FALSE;
				}
			}
			string += strlen(color_table[cnt].name);
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

int get_color_names(struct session *ses, char *string, char *result)
{
	int cnt, skip;

	*result = 0;

	while (*string)
	{
		switch (*string)
		{
			case ' ':
			case ';':
			case ',':
				string++;
				continue;

			case '<':
				skip = is_color_code(string);

				if (skip == 0)
				{
					return FALSE;
				}
				string += sprintf(result, "%.*s", skip, string);

				result += substitute(ses, result, result, SUB_COL);

				continue;
			
			case '\\':
				skip = find_escaped_color_code(string);

				if (skip == 0)
				{
					string++;
					if (*string)
					{
						*result++ = *string++;
					}
				}
				string += sprintf(result, "%.*s", skip, string);

				result += substitute(ses, result, result, SUB_ESC);

				continue;

			case '\e':
				skip = find_color_code(string);

				if (skip == 0)
				{
					return FALSE;
				}
				result += sprintf(result, "%.*s", skip, string);

				string += skip;

				continue;
		}

		if (isalpha((int) *string))
		{
			for (cnt = 0 ; *color_table[cnt].name ; cnt++)
			{
				if (!strncmp(color_table[cnt].name, string, color_table[cnt].len))
				{
					substitute(ses, color_table[cnt].code, result, SUB_COL);

					result += strlen(result);

					break;
				}
			}

			if (*color_table[cnt].name == 0)
			{
				for (cnt = 0 ; *color_table[cnt].name ; cnt++)
				{
					if (!strncasecmp(color_table[cnt].name, string, color_table[cnt].len))
					{
						substitute(ses, color_table[cnt].code, result, SUB_COL);

						result += strlen(result);

						break;
					}
				}

				if (*color_table[cnt].name == 0)
				{
					return FALSE;
				}
			}
			string += strlen(color_table[cnt].name);
		}
		else
		{
			return FALSE;
		}
	}
	*result = 0;

	return TRUE;
}
