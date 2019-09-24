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

int is_color_code(char *str)
{
	if (str[0] == '<')
	{
		if (!isalnum((int) str[1]) || !isalnum((int) str[2]) || !isalnum((int) str[3]) || str[4] != '>')
		{
			return FALSE;
		}
		else if (str[1] >= '0' && str[1] <= '9' && str[2] >= '0' && str[2] <= '9' && str[3] >= '0' && str[3] <= '9')
		{
			return TRUE;
		}
		else if (str[1] >= 'a' && str[1] <= 'f' && str[2] >= 'a' && str[2] <= 'f' && str[3] >= 'a' && str[3] <= 'f')
		{
			return TRUE;
		}
		else if (str[1] >= 'A' && str[1] <= 'F' && str[2] >= 'A' && str[2] <= 'F' && str[3] >= 'A' && str[3] <= 'F')
		{
			return TRUE;
		}
		else if (str[1] == 'g' && str[2] >= '0' && str[2] <= '9' && str[3] >= '0' && str[3] <= '9')
		{
			return TRUE;
		}
		else if (str[1] == 'G' && str[2] >= '0' && str[2] <= '9' && str[3] >= '0' && str[3] <= '9')
		{
			return TRUE;
		}
	}
	return FALSE;
}

int hex_digit(char *str)
{
	if (isdigit((int) *str))
	{
		return *str - '0';
	}
	else
	{
		return toupper((int) *str) - 'A' + 10;
	}
}

unsigned long long hex_number_64bit(char *str)
{
	unsigned long long len, mul, val = 0;

	for (len = 0 ; len < 16 ; len++)
	{
		if (!isxdigit((int) str[len]))
		{
			break;
		}
	}

	for (mul = 1 ; len > 0 ; mul *= 16)
	{
		val += mul * hex_digit(str + --len);
	}

	return val;
}

int hex_number_8bit(char *str)
{
	int value = 0;

	if (str)
	{
		if (isdigit((int) *str))
		{
			value += 16 * (*str - '0');
		}
		else
		{
			value += 16 * (toupper((int) *str) - 'A' + 10);
		}
		str++;
	}

	if (str)
	{
		if (isdigit((int) *str))
		{
			value += *str - '0';
		}
		else
		{
			value += toupper((int) *str) - 'A' + 10;
		}
		str++;
	}

	return value;
}

int oct_number(char *str)
{
	int value = 0;

	if (str)
	{
		if (isdigit((int) *str))
		{
			value += 8 * (*str - '0');
		}
		str++;
	}

	if (str)
	{
		if (isdigit((int) *str))
		{
			value += *str - '0';
		}
		str++;
	}

	return value;
}

int unicode_16_bit(char *str, char *out)
{
	int val = 0;
	unsigned char *pto = (unsigned char *) out;

	if (isdigit((int) *str))
	{
		val += 4096 * (*str - '0');
	}
	else
	{
		val += 4096 * (toupper((int) *str) - 'A' + 10);
	}
	str++;

	if (isdigit((int) *str))
	{
		val += 256 * (*str - '0');
	}
	else
	{
		val += 256 * (toupper((int) *str) - 'A' + 10);
	}
	str++;

	if (isdigit((int) *str))
	{
		val += 16 * (*str - '0');
	}
	else
	{
		val += 16 * (toupper((int) *str) - 'A' + 10);
	}
	str++;

	if (isdigit((int) *str))
	{
		val += (*str - '0');
	}
	else
	{
		val += (toupper((int) *str) - 'A' + 10);
	}
	str++;

	if (val < 128)
	{
		*pto++ = val;
		*pto++ = 0;
		return 1;
	}
	else if (val < 4096)
	{
		*pto++ = 192 + val / 64;
		*pto++ = 128 + val % 64;
		*pto++ = 0;
		return 2;
	}
	else
	{
		*pto++ = 224 + val / 4096;
		*pto++ = 128 + val / 64 % 64;
		*pto++ = 128 + val % 64;
		*pto++ = 0;
		return 3;
	}
}

int unicode_21_bit(char *str, char *out)
{
	int val = 0;
	unsigned char *pto = (unsigned char *) out;

	if (str)
	{
		if (isdigit((int) *str))
		{
			val += 1048576 * (*str - '0');
		}
		else
		{
			val += 1048576 * (toupper((int) *str) - 'A' + 10);
		}
		str++;
	}

	if (str)
	{
		if (isdigit((int) *str))
		{
			val += 65536 * (*str - '0');
		}
		else
		{
			val += 65536 * (toupper((int) *str) - 'A' + 10);
		}
		str++;
	}

	if (str)
	{
		if (isdigit((int) *str))
		{
			val += 4096 * (*str - '0');
		}
		else
		{
			val += 4096 * (toupper((int) *str) - 'A' + 10);
		}
		str++;
	}

	if (str)
	{
		if (isdigit((int) *str))
		{
			val += 256 * (*str - '0');
		}
		else
		{
			val += 256 * (toupper((int) *str) - 'A' + 10);
		}
		str++;
	}

	if (str)
	{
		if (isdigit((int) *str))
		{
			val += 16 * (*str - '0');
		}
		else
		{
			val += 16 * (toupper((int) *str) - 'A' + 10);
		}
		str++;
	}

	if (str)
	{
		if (isdigit((int) *str))
		{
			val += (*str - '0');
		}
		else
		{
			val += (toupper((int) *str) - 'A' + 10);
		}
		str++;
	}

	if (val < 128)
	{
		*pto++ = val;
		return 1;
	}
	else if (val < 4096)
	{
		*pto++ = 192 + val / 64;
		*pto++ = 128 + val % 64;
		*pto++ = 0;
		return 2;
	}
	else if (val < 65536)
	{
		*pto++ = 224 + val / 4096;
		*pto++ = 128 + val / 64 % 64;
		*pto++ = 128 + val % 64;
		*pto++ = 0;
		return 3;
	}
	else if (val < 1114112)
	{
		*pto++ = 240 + val / 262144;
		*pto++ = 128 + val / 4096 % 64;
		*pto++ = 128 + val / 64 % 64;
	        *pto++ = 128 + val % 64;
		*pto++ = 0;
	        return 4;
	}
	else
	{
		*pto++ = 239;
		*pto++ = 191;
		*pto++ = 189;
		*pto++ = 0;
		return 3;
	}
}

unsigned long long utime()
{
	struct timeval now_time;

	gettimeofday(&now_time, NULL);

	if (gtd->utime >= now_time.tv_sec * 1000000ULL + now_time.tv_usec)
	{
		gtd->utime++;
	}
	else
	{
		gtd->utime = now_time.tv_sec * 1000000ULL + now_time.tv_usec;
	}
	return gtd->utime;
}

void seed_rand(struct session *ses, unsigned long long seed)
{
	ses->rand = seed;
}

unsigned long long generate_rand(struct session *ses)
{
	ses->rand = 6364136223846793005ULL * ses->rand + 1ULL;

	return ses->rand;
}

char *capitalize(char *str)
{
	static char outbuf[BUFFER_SIZE];
	int cnt;

	for (cnt = 0 ; str[cnt] != 0 ; cnt++)
	{
		outbuf[cnt] = toupper((int) str[cnt]);
	}
	outbuf[cnt] = 0;

	return outbuf;
}

char *ntos(long long number)
{
	static char outbuf[100][NUMBER_SIZE];
	static int cnt;

	cnt = (cnt + 1) % 100;

	sprintf(outbuf[cnt], "%lld", number);

	return outbuf[cnt];
}

char *indent(int len)
{
	static char outbuf[10][STACK_SIZE];
	static int cnt;

	cnt = (cnt + 1) % 10;

	memset(outbuf[cnt], ' ', len * 5);

	outbuf[cnt][len * 5] = 0;

	return outbuf[cnt];
}

int cat_sprintf(char *dest, char *fmt, ...)
{
	char buf[STRING_SIZE];
	int size;

	va_list args;

	va_start(args, fmt);
	size = vsprintf(buf, fmt, args);
	va_end(args);

	strcat(dest, buf);

	return size;
}

void ins_sprintf(char *dest, char *fmt, ...)
{
	char buf[STRING_SIZE], tmp[STRING_SIZE];

	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	strcpy(tmp, dest);
	strcpy(dest, buf);
	strcat(dest, tmp);
}

int str_suffix(char *str1, char *str2)
{
	int len1, len2;

	len1 = strlen(str1);
	len2 = strlen(str2);

	if (len1 >= len2)
	{
		if (!strcasecmp(str1 + len1 - len2, str2))
		{
			return FALSE;
		}
	}
	return TRUE;
}

void socket_printf(struct session *ses, size_t length, char *format, ...)
{
	size_t size;

	char buf[STRING_SIZE];
	va_list args;

	va_start(args, format);
	size = vsprintf(buf, format, args);
	va_end(args);

	if (size != length && HAS_BIT(ses->telopts, TELOPT_FLAG_DEBUG))
	{
		tintin_printf(ses, "DEBUG TELNET: socket_printf size difference: %d vs %d", size, length);
	}

	if (HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
	{
		write_line_mud(ses, buf, length);
	}
}

void telnet_printf(struct session *ses, size_t length, char *format, ...)
{
	size_t size;

	char buf[STRING_SIZE];
	va_list args;

	va_start(args, format);
	size = vsprintf(buf, format, args);
	va_end(args);

	if (size != length && HAS_BIT(ses->telopts, TELOPT_FLAG_DEBUG))
	{
		tintin_printf(ses, "DEBUG TELNET: telnet_printf size difference: %d vs %d", size, length);
	}

	if (HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
	{
		SET_BIT(ses->telopts, TELOPT_FLAG_TELNET);

		write_line_mud(ses, buf, length);

		DEL_BIT(ses->telopts, TELOPT_FLAG_TELNET);
	}
}
