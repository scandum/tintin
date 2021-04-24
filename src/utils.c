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


int hex_digit(char *str)
{
	if (is_digit(*str))
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
		if (!is_hex(str[len]))
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

unsigned int hex_number_32bit(char *str)
{
	unsigned long long len, mul, val = 0;

	for (len = 0 ; len < 8 ; len++)
	{
		if (!is_hex(str[len]))
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

	if (*str)
	{
		value += 16 * hex_digit(str++);
	}

	if (*str)
	{
		value += hex_digit(str++);
	}

	return value;
}

int oct_number(char *str)
{
	int value = 0;

	if (*str)
	{
		if (is_digit(*str))
		{
			value += 8 * (*str - '0');
		}
		str++;
	}

	if (*str)
	{
		if (is_digit(*str))
		{
			value += *str - '0';
		}
		str++;
	}

	return value;
}

int unicode_8_bit(char *str, char *out)
{
	int val = 0;
	unsigned char *pto = (unsigned char *) out;

	val += 16 * hex_digit(str++);
	val += hex_digit(str++);

	if (val < 128)
	{
		*pto++ = val;
		*pto++ = 0;
		return 1;
	}
	else
	{
		*pto++ = 192 + val / 64;
		*pto++ = 128 + val % 64;
		*pto++ = 0;
		return 2;
	}
}

int unicode_12_bit(char *str, char *out)
{
	int val = 0;
	unsigned char *pto = (unsigned char *) out;

	val += 256 * hex_digit(str++);
	val += 16 * hex_digit(str++);
	val += hex_digit(str++);

	if (val < 128)
	{
		*pto++ = val;
		*pto++ = 0;
		return 1;
	}
	else
	{
		*pto++ = 192 + val / 64;
		*pto++ = 128 + val % 64;
		*pto++ = 0;
		return 2;
	}
}


int unicode_16_bit(char *str, char *out)
{
	int val = 0;
	unsigned char *pto = (unsigned char *) out;

	val += 4096 * hex_digit(str++);
	val += 256 * hex_digit(str++);
	val += 16 * hex_digit(str++);
	val += hex_digit(str++);

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

int unicode_20_bit(char *str, char *out)
{
	int val = 0;
	unsigned char *pto = (unsigned char *) out;

	val += 65536 * hex_digit(str++);
	val += 4096 * hex_digit(str++);
	val += 256 * hex_digit(str++);
	val += 16 * hex_digit(str++);
	val += hex_digit(str++);

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
	else
	{
		*pto++ = 240 + val / 262144;
		*pto++ = 128 + val / 4096 % 64;
		*pto++ = 128 + val / 64 % 64;
	        *pto++ = 128 + val % 64;
		*pto++ = 0;
	        return 4;
	}
}

int unicode_21_bit(char *str, char *out)
{
	int val = 0;
	unsigned char *pto = (unsigned char *) out;

	val += 1048576 * hex_digit(str++);
	val += 65536 * hex_digit(str++);
	val += 4096 * hex_digit(str++);
	val += 256 * hex_digit(str++);
	val += 16 * hex_digit(str++);
	val += hex_digit(str++);

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
	unsigned long long utime;

	gettimeofday(&now_time, NULL);

	utime = now_time.tv_sec * 1000000ULL + now_time.tv_usec;

	if (gtd->utime < utime)
	{
		gtd->utime = utime;
	}

	return gtd->utime++;
}

time_t get_time(struct session *ses, char *str)
{
	unsigned long long time = get_ulong(ses, str);

	if (time >= 1000000000LL * 1000000LL)
	{
		time /= 1000000;
	}

	return time;
}

char *str_time(struct session *ses, char *format, time_t time)
{
	static char buf[10][NAME_SIZE];
	static int cnt;
	struct tm timeval_tm;

	cnt = (cnt + 1) % 10;

	timeval_tm = *localtime(&time);

	strftime(buf[cnt], NAME_SIZE, format, &timeval_tm);

	return buf[cnt];
}
	
void seed_rand(struct session *ses, unsigned long long seed)
{
	ses->rand = seed % 4294967291ULL;
	ses->rkey = seed % 5;

	srand(ses->rand);
}

unsigned long long generate_rand(struct session *ses)
{
	static unsigned long long primes[] = {26196137413795067, 1062272168593625449, 5189794811, 237506310434573, 212938855558633, 51741641338759 };

	return rand();

/*	if (ses->rkey % 3 == 1)
	{
		ses->rand += 316595909ULL + primes[++ses->rkey % 5];
	}
	else*/
	{
		ses->rand += primes[++ses->rkey % 5];
	}

	return (unsigned int) ses->rand;

//	ses->rand = (ses->rand + 260854879ULL) % 4294967291ULL;
//	return ses->rand % 1000000000ULL;

}

/*
uint32_t lcg_rand(uint32_t *state)
{
    return *state = (uint64_t)*state * 279470273u % 0xfffffffb;
}
	ses->rand = 6364136223846793005ULL * ses->rand + 1ULL;

	return ses->rand;
}
*/
char *capitalize(char *str)
{
	char *outbuf = str_alloc_stack(0);
	int cnt;

	for (cnt = 0 ; str[cnt] != 0 ; cnt++)
	{
		outbuf[cnt] = toupper((int) str[cnt]);
	}
	outbuf[cnt] = 0;

	return outbuf;
}

char *ftos(float number)
{
	static char outbuf[10][NUMBER_SIZE];
	static int cnt;
	int len;

	cnt = (cnt + 1) % 10;

	sprintf(outbuf[cnt], "%f", number);

	for (len = strlen(outbuf[cnt]) - 1 ; len ; len--)
	{
		if (outbuf[cnt][len] == '0')
		{
			outbuf[cnt][len] = 0;
		}
		else
		{
			if (outbuf[cnt][len] == '.')
			{
				outbuf[cnt][len] = 0;
			}
			break;
		}
	}
	return outbuf[cnt];
}

char *ntos(long long number)
{
	static char outbuf[10][NUMBER_SIZE];
	static int cnt;

	cnt = (cnt + 1) % 10;

	sprintf(outbuf[cnt], "%lld", number);

	return outbuf[cnt];
}

char *indent_one(int len)
{
	static char outbuf[10][STACK_SIZE];
	static int cnt;

	cnt = (cnt + 1) % 10;

	memset(outbuf[cnt], ' ', UMAX(1, len));

	outbuf[cnt][len] = 0;

	return outbuf[cnt];
}

char *indent(int len)
{
	static char outbuf[21][101];

	len = URANGE(0, len, 20);

	if (outbuf[len][0] == 0)
	{
		sprintf(outbuf[len], "%*s", len * 4, "");
	}

	return outbuf[len];
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

void ins_cpy(char *dest, char *str)
{
	char tmp[STRING_SIZE];

	strcpy(tmp, dest);
	strcpy(dest, str);
	strcat(dest, tmp);
}

// unused, also needs testing

void ins_sprintf(char *dest, char *fmt, ...)
{
	char tmp[STRING_SIZE];
	int len;
	va_list args;

	strcpy(tmp, dest);

	va_start(args, fmt);
	len = vsprintf(dest, fmt, args);
	va_end(args);

	strcpy(dest + len, tmp);
}

int is_suffix(char *str1, char *str2)
{
	int len1, len2;

	len1 = strlen(str1);
	len2 = strlen(str2);

	if (len1 >= len2)
	{
		if (!strcasecmp(str1 + len1 - len2, str2))
		{
			return TRUE;
		}
	}
	return FALSE;
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

void telnet_printf(struct session *ses, int length, char *format, ...)
{
	size_t size;

	char buf[STRING_SIZE];
	va_list args;

	va_start(args, format);
	size = vsprintf(buf, format, args);
	va_end(args);

	if (length != -1 && size != length && HAS_BIT(ses->telopts, TELOPT_FLAG_DEBUG))
	{
		tintin_printf(ses, "DEBUG TELNET: telnet_printf size difference: %d vs %d", size, length);
	}

	if (HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
	{
		SET_BIT(ses->telopts, TELOPT_FLAG_TELNET);

		write_line_mud(ses, buf, size);

		DEL_BIT(ses->telopts, TELOPT_FLAG_TELNET);
	}
}
