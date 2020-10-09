/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright (C) 2004-2020 Igor van den Hoven                                *
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
*                      coded by Igor van den Hoven 2020                       *
******************************************************************************/

#include "tintin.h"

char base64_table[64] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

char str64_table[256] =
{ 
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,
	 0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0,
	 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,	 
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0,	 

	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	 
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	 
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	 
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

int compress_data(char *in, size_t size_in, char *out, size_t size_out)
{
	z_stream *stream;
	int size;

	push_call("compress_data(%p,%zu,%p,%zu)",in,size_in,out,size_out);

	stream              = calloc(1, sizeof(z_stream));

	stream->next_in     = (unsigned char *) in;
	stream->avail_in    = size_in;

	stream->next_out    = (unsigned char *) out;
	stream->avail_out   = size_out;

	stream->data_type   = Z_ASCII;
	stream->zalloc      = zlib_alloc;
	stream->zfree       = zlib_free;
	stream->opaque      = Z_NULL;

	if (deflateInit(stream, Z_BEST_COMPRESSION) != Z_OK)
	{
		tintin_printf2(gtd->ses, "compress: failed deflateInit2");

		free(stream);

		pop_call();
		return 0;
	}

	if (deflate(stream, Z_FINISH) != Z_STREAM_END)
	{
		tintin_printf2(gtd->ses, "compress: failed deflate");

		free(stream);

		pop_call();
		return 0;
	}

	size = size_out - stream->avail_out;

	deflateEnd(stream);

	free(stream);

	pop_call();
	return size;
}

int decompress_data(char *in, size_t size_in, char *out, size_t size_out)
{
	z_stream *stream;
	int size;

	stream              = calloc(1, sizeof(z_stream));

	stream->data_type   = Z_ASCII;
	stream->zalloc      = zlib_alloc;
	stream->zfree       = zlib_free;
	stream->opaque      = Z_NULL;

	if (inflateInit(stream) != Z_OK)
	{
		tintin_printf2(gtd->ses, "decompress_data: failed inflateInit");

		free(stream);

		return 0;
	}

	stream->next_in     = (unsigned char *) in;
	stream->avail_in    = size_in;

	stream->next_out    = (unsigned char *) out;
	stream->avail_out   = size_out;

	if (inflate(stream, Z_SYNC_FLUSH) == Z_BUF_ERROR)
	{
		tintin_printf2(gtd->ses, "decompress_data: failed inflate");
	}

	size = size_out - stream->avail_out;

	inflateEnd(stream);

	free(stream);

	return size;
}

int str_to_base64(char *in, char *out, size_t size)
{
	char *pto;
	int cnt;

	push_call("str_to_base64(%p,%p,%zu)",in,out,size);

	pto = out;

	for (cnt = 0 ; cnt + 3 < size ; cnt += 3)
	{
		*pto++ = base64_table[(unsigned char) in[cnt + 0] % 64];
		*pto++ = base64_table[(unsigned char) in[cnt + 0] / 64 + (unsigned char) in[cnt + 1] % 16 * 4];
		*pto++ = base64_table[(unsigned char) in[cnt + 1] / 16 + (unsigned char) in[cnt + 2] % 4 * 16];
		*pto++ = base64_table[(unsigned char) in[cnt + 2] /  4];
	}

	switch (size - cnt)
	{
		case 1:
			*pto++ = base64_table[(unsigned char) in[cnt + 0] % 64];
			*pto++ = base64_table[(unsigned char) in[cnt + 0] / 64];
			*pto++ = '=';
			*pto++ = '=';
			break;
		case 2:
			*pto++ = base64_table[(unsigned char) in[cnt + 0] % 64];
			*pto++ = base64_table[(unsigned char) in[cnt + 0] / 64 + (unsigned char) in[cnt + 1] % 16 * 4];
			*pto++ = base64_table[(unsigned char) in[cnt + 1] / 16];
			*pto++ = '=';
			break;
	}


	*pto++ = 0;

	pop_call();
	return pto - out;
}

int base64_to_str(char *in, char *out, size_t size)
{
	char *pto;
	int cnt;

	pto = out;

	for (cnt = 0 ; cnt + 4 <= size ; cnt += 4)
	{
		*pto++ = str64_table[(unsigned char) in[cnt + 0]] + str64_table[(unsigned char) in[cnt + 1]] % 4 * 64;

		if (in[cnt + 1] == '=')
		{
			break;
		}

		*pto++ = str64_table[(unsigned char) in[cnt + 1]] / 4 + str64_table[(unsigned char) in[cnt + 2]] % 16 * 16;

		if (in[cnt + 2] == '=')
		{
			break;
		}

		*pto++ = str64_table[(unsigned char) in[cnt + 2]] / 16 + str64_table[(unsigned char) in[cnt + 3]] * 4;
	}
	*pto++ = 0;

	return pto - out;
}


int str_to_base252(char *in, char *out, size_t size)
{
	char *pto;
	int cnt;

	push_call("str_to_base252(%p,%p,%zu)",in,out,size);

	pto = out;

	for (cnt = 0 ; cnt < size ; cnt++)
	{
		switch ((unsigned char) in[cnt])
		{
			case 0:
			case '{':
			case '}':
				*pto++ = 245;
				*pto++ = 128 + (unsigned char) in[cnt] % 64;
				break;

			case '\\':
				*pto++ = 246 + (unsigned char) in[cnt] / 64;
				*pto++ = 128 + (unsigned char) in[cnt] % 64;
				break;

			case 245:
			case 246:
			case 247:
			case 248:
			case 255:
				*pto++ = 248;
				*pto++ = 128 + (unsigned char) in[cnt] % 64;
				break;

			default:
				*pto++ = in[cnt];
				break;
		}
	}

	*pto++ = 0;

	pop_call();
	return pto - out;
}

int base252_to_str(char *in, char *out, size_t size)
{
	char *pto;
	int cnt;

	pto = out;

	for (cnt = 0 ; cnt < size ; cnt++)
	{
		switch ((unsigned char) in[cnt])
		{
			case 245:
				cnt++;
				*pto++ =   0 + (unsigned char) in[cnt] % 64;
				break;

			case 246:
				cnt++;
				*pto++ =  64 + (unsigned char) in[cnt] % 64;
				break;

			case 247:
				cnt++;
				*pto++ = 128 + (unsigned char) in[cnt] % 64;
				break;

			case 248:
				cnt++;
				*pto++ = 192 + (unsigned char) in[cnt] % 64;
				break;

			default:
				*pto++ = in[cnt];
				break;
		}
	}
	*pto++ = 0;

	return pto - out;
}

void str_to_base64z(char *in, char *out, size_t size)
{
	char buf[BUFFER_SIZE];
	int len;

	len = compress_data(in, size + 1, buf, BUFFER_SIZE);

	if (len)
	{
		str_to_base64(buf, out, len);
	}
}

void base64z_to_str(char *in, char *out, size_t size)
{
	char buf[BUFFER_SIZE];
	int len;

	len = base64_to_str(in, buf, size);

	decompress_data(buf, len, out, BUFFER_SIZE);
}

void str_to_base252z(char *in, char *out, size_t size)
{
	char buf[BUFFER_SIZE];
	int len;

	len = compress_data(in, size + 1, buf, BUFFER_SIZE);

	if (len)
	{
		str_to_base252(buf, out, len);
	}
}

void base252z_to_str(char *in, char *out, size_t size)
{
	char buf[BUFFER_SIZE];
	int len;

	len = base252_to_str(in, buf, size);

	decompress_data(buf, len, out, BUFFER_SIZE);
}
