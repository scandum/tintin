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

struct str_data *str_calloc(int size)
{
	struct str_data *str_ptr = (struct str_data *) calloc(1, sizeof(struct str_data) + size + 1);

	LINK(str_ptr, gtd->memory->alloc->next, gtd->memory->alloc->prev);

	str_ptr->max = size + 1;
	str_ptr->len = 0;

	return str_ptr;
}

char *str_alloc(int size)
{
	struct str_data *str_ptr;
	char *str;

	str_ptr = str_calloc(size);

	str = (char *) str_ptr + sizeof(struct str_data);

	return str;
}

struct str_data *str_realloc(struct str_data *str_ptr, int size)
{
	struct str_data *new_ptr;

	push_call("str_realloc(%p,%d) (%d,%d,%p,%p)",str_ptr,size, str_ptr->len, str_ptr->max, str_ptr->next, str_ptr->prev);

	if (str_ptr->max <= str_ptr->len)
	{
		tintin_printf2(gtd->ses, "\e[1;35mstr_realloc: we got problem\n");
	}

	if (str_ptr->max <= size)
	{
//		int len = str_ptr->len;

		UNLINK(str_ptr, gtd->memory->alloc->next, gtd->memory->alloc->prev);

		new_ptr = (struct str_data *) realloc(str_ptr, sizeof(struct str_data) + size + 1);

		if (new_ptr == NULL)
		{
			printf("str_realloc: failed to realloc mem.\n");
		}
		else
		{
			str_ptr = new_ptr;
		}

		LINK(str_ptr, gtd->memory->alloc->next, gtd->memory->alloc->prev);

		str_ptr->max = size + 1;
//		str_ptr->len = len;
	}
	pop_call();
	return str_ptr;
}

struct str_data *get_str_ptr(char *str)
{
	return (struct str_data *) (str - sizeof(struct str_data));
}

char *get_str_str(struct str_data *str_ptr)
{
	return (char *) str_ptr + sizeof(struct str_data);
}

struct str_data *str_ptr_resize(struct str_data *str_ptr, int add)
{
	int len = str_ptr->len;

	if (str_ptr->max <= len + add)
	{
		str_ptr = str_realloc(str_ptr, len * 2 + add);
	}
	return str_ptr;
}

char *str_resize(char **str, int add)
{
	struct str_data *str_ptr = get_str_ptr(*str);

	str_ptr = str_ptr_resize(str_ptr, add);

	*str = get_str_str(str_ptr);

	return *str;
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
	struct str_data *clo_ptr = get_str_ptr(*clone);

	int len = str_len(original);

	if (clo_ptr->max < len)
	{
		clo_ptr = str_realloc(clo_ptr, len * 2);

		*clone = get_str_str(clo_ptr);
	}
}

char *str_dup_clone(char *original)
{
	char *dup;
	int len;

	len = str_len(original);
	dup = str_alloc(len);

	memcpy(dup, original, len + 1);

	return dup;
}

// call after a non str_ function alters *str to set the correct length.

int str_fix(char *original)
{
	struct str_data *str_ptr = get_str_ptr(original);

	str_ptr->len = strlen(original);

	return str_ptr->len;
}

int str_len(char *str)
{
	return get_str_ptr(str)->len;
}

int str_max(char *str)
{
	return get_str_ptr(str)->max;
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

	str_ptr = get_str_ptr(*str);

	if (str_ptr->max <= buf_len)
	{
		str_ptr = str_realloc(str_ptr, buf_len);

		*str = get_str_str(str_ptr);
	}
	str_ptr->len = buf_len;

	strcpy(*str, buf);

	return *str;
}

char *str_cpy_printf(char **str, char *fmt, ...)
{
	char *arg;
	va_list args;

	va_start(args, fmt);
	vasprintf(&arg, fmt, args);
	va_end(args);

	str_cpy(str, arg);

	free(arg);

	return *str;
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

	buf_len = strnlen(buf, len);

	str_ptr = get_str_ptr(*str);

	if (str_ptr->max <= buf_len)
	{
		str_ptr = str_realloc(str_ptr, len);

		*str = get_str_str(str_ptr);
	}

	str_ptr->len = UMIN(buf_len, len);

	strncpy(*str, buf, len);

	(*str)[len] = 0;

	return *str;
}

char *str_cat(char **str, char *arg)
{
	int arg_len;
	struct str_data *str_ptr;

	arg_len = strlen(arg);

	str_ptr = get_str_ptr(*str);

	if (str_ptr->max <= str_ptr->len + arg_len)
	{
		str_ptr = str_ptr_resize(str_ptr, arg_len);

		*str = get_str_str(str_ptr);
	}

	strcpy(&(*str)[str_ptr->len], arg);

	str_ptr->len += arg_len;

	return *str;
}

char *str_cat_chr(char **ptr, char chr)
{
	struct str_data *str_ptr;

	str_ptr = get_str_ptr(*ptr);

	if (str_ptr->max <= str_ptr->len + 1)
	{
		str_ptr = str_ptr_resize(str_ptr, 1);

		*ptr = get_str_str(str_ptr);
	}

	(*ptr)[str_ptr->len++] = chr;

	(*ptr)[str_ptr->len] = 0;

	return *ptr;
}
	

char *str_cat_printf(char **str, char *fmt, ...)
{
	char *arg;
	va_list args;

	va_start(args, fmt);
	vasprintf(&arg, fmt, args);
	va_end(args);

	str_cat(str, arg);

	free(arg);

	return *str;
}

char *str_ins(char **str, int index, char *buf)
{
	int buf_len;
	struct str_data *str_ptr;

	buf_len = strlen(buf);

	str_ptr = get_str_ptr(*str);

	if (str_ptr->max <= str_ptr->len + buf_len)
	{
		str_ptr = str_ptr_resize(str_ptr, buf_len);

		*str = get_str_str(str_ptr);
	}

	if (index >= str_ptr->len)
	{
		strcpy(&(*str)[str_ptr->len], buf);
	}
	else
	{
		int cnt;
		char *pta, *ptz;

		pta = &(*str)[str_ptr->len + 1];
		ptz = &(*str)[str_ptr->len + 1 + buf_len];

		for (cnt = 0 ; cnt < buf_len + 1 ; cnt++)
		{
			*ptz-- = *pta--;
		}

		pta = &(*str)[index];
		ptz = buf;

		for (cnt = 0 ; cnt < buf_len ; cnt++)
		{
			*pta++ = *ptz++;
		}
	}

	str_ptr->len += buf_len;

	return *str;
}

void str_free(char *ptr)
{
	struct str_data *str_ptr = get_str_ptr(ptr);

	UNLINK(str_ptr, gtd->memory->alloc->next, gtd->memory->alloc->prev);

	free(str_ptr);
}

// stack handling

char *str_alloc_stack()
{
	struct str_data *str_ptr;

	if (gtd->memory->stack_len == gtd->memory->stack_max)
	{
		gtd->memory->stack_max++;

		gtd->memory->stack = (struct str_data **) realloc(gtd->memory->stack, sizeof(struct str_data *) * gtd->memory->stack_max);

		gtd->memory->stack[gtd->memory->stack_len] = str_calloc(BUFFER_SIZE);
	}
	str_ptr = gtd->memory->stack[gtd->memory->stack_len++];

	str_ptr->len = 0;

	return get_str_str(str_ptr);
}
