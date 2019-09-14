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
*                      coded by Igor van den Hoven 2007                       *
******************************************************************************/

#include "tintin.h"

char *restring(char *point, char *string)
{
	if (point)
	{
		free(point);
	}

	return strdup(string);
}

char *restringf(char *point, char *fmt, ...)
{
	char string[STRING_SIZE];
	va_list args;

	va_start(args, fmt);
	vsprintf(string, fmt, args);
	va_end(args);

	if (point)
	{
		free(point);
	}

	return strdup(string);
}


/*
	str_ functions
*/

char *str_alloc(int size)
{
	char *str;

	struct str_data *str_ptr = (struct str_data *) calloc(1, gtd->str_size + size + 1);

	str_ptr->max = size + 1;
	str_ptr->len = 0;

	str = (char *) str_ptr + gtd->str_size;

	*str = 0;

	return str;
}

struct str_data *str_realloc(struct str_data *str_ptr, int size)
{
	if (str_ptr->max <= size)
	{
		int len = str_ptr->len;

		str_ptr = (struct str_data *) realloc(str_ptr, gtd->str_size + size + 1);

		str_ptr->max = size + 1;
		str_ptr->len = len;
	}
	return str_ptr;
}

struct str_data *str_resize(struct str_data *str_ptr, int add)
{
	int len = str_ptr->len;

	if (str_ptr->max <= len + add)
	{
		str_ptr = (struct str_data *) realloc(str_ptr, gtd->str_size + (len + 1) * 2 + add + 1);

		str_ptr->max = (len + 1) * 2 + add + 1;
		str_ptr->len = len;
	}
	return str_ptr;
}

// like str_dup but return an empty string

char *str_mim(char *original)
{
	char *string = str_alloc(strlen(original));

	str_cpy(&string, "");

	return string;
}

// give **clone the same max length as *original.

void str_clone(char **clone, char *original)
{
	struct str_data *clo_ptr = (struct str_data *) (*clone - gtd->str_size);
	int len = str_len(original);

	if (clo_ptr->max < len)
	{
		clo_ptr = str_realloc(clo_ptr, len * 2);

		*clone = (char *) clo_ptr + gtd->str_size;
	}
}

// call after a non str_ function alters *str to set the correct length.

void str_fix(char *original)
{
	struct str_data *str_ptr = (struct str_data *) (original - gtd->str_size);

	str_ptr->len = strlen(original);
}

int str_len(char *str)
{
	struct str_data *str_ptr = (struct str_data *) (str - gtd->str_size);

	return str_ptr->len;
}

char *str_dup(char *original)
{
	char *dup = str_alloc(strlen(original));

	str_cpy(&dup, original);

	return dup;
}

char *str_dup_printf(char *fmt, ...)
{
	char *str, *ptr;
	va_list args;

	push_call("str_dup_printf(%s)", fmt);

	va_start(args, fmt);
	vasprintf(&str, fmt, args);
	va_end(args);

	ptr = str_dup(str);

	free(str);

	pop_call();
	return ptr;
}

char *str_cpy(char **str, char *buf)
{
	int buf_len;
	struct str_data *str_ptr;

	buf_len = strlen(buf);

	str_ptr = (struct str_data *) (*str - gtd->str_size);

	if (str_ptr->max <= buf_len)
	{
		str_ptr = str_realloc(str_ptr, buf_len);

		*str = (char *) str_ptr + gtd->str_size;
	}
	str_ptr->len = buf_len;

	strcpy(*str, buf);

	return *str;
}

char *str_cpy_printf(char **ptr, char *fmt, ...)
{
	char *str;
	va_list args;

	va_start(args, fmt);
	vasprintf(&str, fmt, args);
	va_end(args);

	str_cpy(ptr, str);

	free(str);

	return *ptr;
}

// unused

char *str_ndup(char *buf, int len)
{
	char *dup = str_alloc(len + 1);

	str_ncpy(&dup, buf, len);

	return dup;
}

// Like strncpy but handles the string terminator properly

char *str_ncpy(char **str, char *buf, int len)
{
	int buf_len;
	struct str_data *str_ptr;

	buf_len = strlen(buf);

	if (buf_len > len)
	{
		buf_len = len;
	}

	str_ptr = (struct str_data *) (*str - gtd->str_size);

	if (str_ptr->max <= buf_len)
	{
		str_ptr = str_realloc(str_ptr, len);

		*str = (char *) str_ptr + gtd->str_size;
	}

	str_ptr->len = UMIN(buf_len, len);

	strncpy(*str, buf, len);

	(*str)[len] = 0;

	return *str;
}

char *str_cat(char **str, char *buf)
{
	int buf_len;
	struct str_data *str_ptr;

	buf_len = strlen(buf);

	str_ptr = (struct str_data *) (*str - gtd->str_size);

	if (str_ptr->max <= str_ptr->len + buf_len)
	{
		str_ptr = str_resize(str_ptr, buf_len);

		*str = (char *) str_ptr + gtd->str_size;
	}

	strcpy(&(*str)[str_ptr->len], buf);

	str_ptr->len += buf_len;

	return *str;
}

// Unused

char *str_cat_chr(char **ptr, char chr)
{
	struct str_data *str_ptr;

	str_ptr = (struct str_data *) (*ptr - gtd->str_size);

	if (str_ptr->max <= str_ptr->len + 1)
	{
		str_ptr = str_resize(str_ptr, 1);

		*ptr = (char *) str_ptr + gtd->str_size;
	}

	(*ptr)[str_ptr->len++] = chr;

	(*ptr)[str_ptr->len] = 0;

	return *ptr;
}
	

char *str_cat_printf(char **ptr, char *fmt, ...)
{
	char *str;
	va_list args;

	va_start(args, fmt);
	vasprintf(&str, fmt, args);
	va_end(args);

	str_cat(ptr, str);

	free(str);

	return *ptr;
}

void str_free(char *ptr)
{
	free((struct str_data *) (ptr - gtd->str_size));
}
