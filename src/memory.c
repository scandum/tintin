/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright 2004-2026 Igor van den Hoven                                    *
*                                                                             *
*   SPDX-License-Identifier: LGPL-2.1-or-later                                *
******************************************************************************/

/******************************************************************************
*                               T I N T I N + +                               *
*                                                                             *
*                      coded by Igor van den Hoven 2007                       *
******************************************************************************/

#include "tintin.h"

#ifdef __GLIBC__
#include <malloc.h>
#endif

struct str_data *str_alloc_list(int size);

// freed strings kept for reuse, the older ones are given back to malloc

#define STR_POOL_KEEP                 4096
#define STR_POOL_COMPACT_MIN          65536

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

void init_memory(void)
{
	gtd->memory            = calloc(1, sizeof(struct memory_data));

	gtd->memory->debug     = calloc(1, sizeof(struct stack_data *));

	gtd->memory->stack     = calloc(1, sizeof(struct str_data *));
	gtd->memory->stack_max = 1;

	gtd->memory->list      = calloc(1, sizeof(struct str_data *));
	gtd->memory->list_max  = 1;

	gtd->memory->free      = calloc(1, sizeof(int));
	gtd->memory->free_max  = 1;
}


struct str_data *str_ptr_alloc(int size)
{
	return str_alloc_list(size);
/*
	struct str_data *str_ptr = (struct str_data *) calloc(1, sizeof(struct str_data) + size + 1);

	str_ptr->max = size + 1;

	return str_ptr;
*/
}

struct str_data *get_str_ptr(char *str)
{
	return (struct str_data *) (str - sizeof(struct str_data));
}

char *get_str_str(struct str_data *str_ptr)
{
	return (char *) str_ptr + sizeof(struct str_data);
}

struct str_data *str_ptr_realloc(struct str_data *str_ptr, int size)
{
//	if (str_ptr->max <= size)
	if (size != str_ptr->max - 1)
	{
		str_ptr = (struct str_data *) realloc(str_ptr, sizeof(struct str_data) + size + 1);

		switch (str_ptr->flags)
		{
			case STR_FLAG_STACK:
				gtd->memory->stack[str_ptr->index] = str_ptr;
				break;

			case STR_FLAG_LIST:
				gtd->memory->list[str_ptr->index] = str_ptr;
				break;

			default:
				printf("\e[1;35mstr_ptr_realloc: unknown memory type (%d)", str_ptr->flags);
				break;
		}

		str_ptr->max = size + 1;
	}
	else
	{
		printf("\e[1;35mstr_ptr_realloc: shrink error max=%d len=%d\n", str_ptr->max, str_ptr->len);
	}

	return str_ptr;
}


char *str_alloc(int size)
{
	struct str_data *str_ptr;

	str_ptr = str_ptr_alloc(size);

	return get_str_str(str_ptr);
}

struct str_data *str_ptr_resize(struct str_data *str_ptr, int add)
{
	int len = str_ptr->len;

	if (str_ptr->max <= len + add)
	{
		str_ptr = str_ptr_realloc(str_ptr, len + add + 1);
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

// call after a non str_ function alters *str to set the correct length.

int str_fix(char *original)
{
	struct str_data *str_ptr = get_str_ptr(original);

	str_ptr->len = strlen(original);

	return str_ptr->len;
}

int str_fix_len(char *original, int len)
{
	struct str_data *str_ptr = get_str_ptr(original);

	str_ptr->len = len;

	original[len] = 0;

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
		clo_ptr = str_ptr_realloc(clo_ptr, len * 2);

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

	get_str_ptr(dup)->len = len;

	return dup;
}


char *str_dup(char *original)
{
	char *dup;

	if (*original == 0)
	{
		return str_alloc(0);
	}
	dup = str_alloc(strlen(original));

	str_cpy(&dup, original);

	return dup;
}

char *str_dup_printf(char *fmt, ...)
{
	char *str, *ptv;
	int len;
	va_list args;

	va_start(args, fmt);

	len = vasprintf(&ptv, fmt, args);

	va_end(args);

	str = str_alloc(len);

	memcpy(str, ptv, len + 1);

	free(ptv);

	return str;
}

char *str_cpy(char **str, char *buf)
{
	int buf_len;
	struct str_data *str_ptr;

	buf_len = strlen(buf);

	str_ptr = get_str_ptr(*str);

	if (str_ptr->max <= buf_len)
	{
		str_ptr = str_ptr_realloc(str_ptr, buf_len);

		*str = get_str_str(str_ptr);
	}
	str_ptr->len = buf_len;

	strcpy(*str, buf);

	return *str;
}

char *str_cpy_printf(char **str, char *fmt, ...)
{
	struct str_data *str_ptr;
	char *ptv;
	va_list args;
	int len;

	va_start(args, fmt);

	len = vasprintf(&ptv, fmt, args);

	va_end(args);
/*
	str_cpy(str, ptv);

	return *str;
*/
	str_ptr = get_str_ptr(*str);

	if (str_ptr->max <= len)
	{
		str_ptr = str_ptr_realloc(str_ptr, len);

		*str = get_str_str(str_ptr);
	}

	memcpy(*str, ptv, len + 1);

	str_ptr->len = len;

	free(ptv);

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
		str_ptr = str_ptr_realloc(str_ptr, len);

		*str = get_str_str(str_ptr);
	}

	str_ptr->len = UMIN(buf_len, len);

	strncpy(*str, buf, len);

	(*str)[len] = 0;

	return *str;
}


char *str_cat_len(char **str, char *arg, int len)
{
	struct str_data *str_ptr;

	str_ptr = get_str_ptr(*str);

	if (str_ptr->max <= str_ptr->len + len)
	{
		str_ptr = str_ptr_resize(str_ptr, len);

		*str = get_str_str(str_ptr);
	}

	strcpy(&(*str)[str_ptr->len], arg);

	str_ptr->len += len;

	return *str;
}

char *str_cat(char **str, char *arg)
{
	return str_cat_len(str, arg, strlen(arg));
}

char *str_cat_chr(char **str, char chr)
{
	struct str_data *str_ptr;

	str_ptr = get_str_ptr(*str);

	if (str_ptr->max <= str_ptr->len + 1)
	{
		str_ptr = str_ptr_realloc(str_ptr, str_ptr->max + 10);

		*str = get_str_str(str_ptr);
	}

	(*str)[str_ptr->len++] = chr;

	(*str)[str_ptr->len] = 0;

	return *str;
}

char *str_cat_printf(char **str, char *fmt, ...)
{
	char *arg;
	va_list args;
	int len;

	va_start(args, fmt);
	
	len = vasprintf(&arg, fmt, args);

	va_end(args);

	str_cat_len(str, arg, len);

	free(arg);

	return *str;
}


char *str_cap(char **str, int index, char *buf)
{
	int buf_len;
	struct str_data *str_ptr;

	buf_len = strlen(buf);

	str_ptr = get_str_ptr(*str);

	if (str_ptr->max <= index + buf_len)
	{
		str_ptr = str_ptr_resize(str_ptr, buf_len);

		*str = get_str_str(str_ptr);
	}

	if (index <= str_ptr->len)
	{
		strcpy(&(*str)[index], buf);
	}
	else
	{
		tintin_printf2(gtd->ses, "debug: str_cap: index=%d str_len=%d cap=%s", index, str_ptr->len, buf);
	}

	str_ptr->len = index + buf_len;

	return *str;
}


char *str_ins_len(char **str, int index, char *buf, int buf_len)
{
	struct str_data *str_ptr;

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

		pta = &(*str)[str_ptr->len];
		ptz = &(*str)[str_ptr->len + buf_len];

		for (cnt = 0 ; cnt <= str_ptr->len - index ; cnt++)
		{
			*ptz-- = *pta--;
		}

		pta = &(*str)[index];
		ptz = buf;

		while (*ptz)
		{
			*pta++ = *ptz++;
		}
	}

	str_ptr->len += buf_len;

	return *str;
}

char *str_ins(char **str, int index, char *buf)
{
	return str_ins_len(str, index, buf, strlen(buf));
}

char *str_ins_printf(char **str, int index, char *fmt, ...)
{
	int len;
	char *arg;
	va_list args;

	va_start(args, fmt);

	len = vasprintf(&arg, fmt, args);

	va_end(args);

	str_ins_len(str, index, arg, len);

	free(arg);

	return *str;
}

char *str_mov(char **str, int dst, int src)
{
	struct str_data *str_ptr;
	char *ptm;

	if (dst >= src)
	{
		show_error(gtd->ses, LIST_COMMAND, "str_mov: dst (%d) >= src (%d)", dst, src);

		return *str;
	}

	str_ptr = get_str_ptr(*str);

	if (src > str_ptr->len)
	{
		show_error(gtd->ses, LIST_COMMAND, "str_mov: src (%d) >= len (%d)", src, str_ptr->len);

		return *str;
	}

	ptm = &(*str)[src];

	str_ptr->len -= (src - dst);

	while (*ptm)
	{
		(*str)[dst++] = *ptm++;
	}
	(*str)[dst++] = 0;

	return *str;
}

void str_alloc_free(struct str_data *str_ptr)
{
	if (HAS_BIT(str_ptr->flags, STR_FLAG_STACK|STR_FLAG_FREE))
	{
		tintin_printf2(gtd->ses, "\e[1;31mstr_alloc_free: trying to free invalid memory: %d", str_ptr->flags);
		dump_stack();
		return;
	}

	if (gtd->memory->free_len == gtd->memory->free_max)
	{
		gtd->memory->free_max *= 2;

		gtd->memory->free = (int *) realloc(gtd->memory->free, sizeof(int) * gtd->memory->free_max);
	}
	SET_BIT(str_ptr->flags, STR_FLAG_FREE);

	gtd->memory->free[gtd->memory->free_len++] = str_ptr->index;

	gtd->memory->free_full++;
}

void str_free(char *str)
{
//	free(get_str_ptr(str));

	str_alloc_free(get_str_ptr(str));
}

// stack handling

char *str_alloc_stack(int size)
{
	struct str_data *str_ptr;
	char *str;

	if (size < BUFFER_SIZE)
	{
		size = BUFFER_SIZE;
	}

	if (gtd->memory->stack_len == gtd->memory->stack_cap)
	{
		gtd->memory->stack_cap++;

		if (gtd->memory->stack_cap == gtd->memory->stack_max)
		{
			gtd->memory->stack_max *= 2;

			gtd->memory->stack = (struct str_data **) realloc(gtd->memory->stack, sizeof(struct str_data *) * gtd->memory->stack_max);
		}
		str_ptr = (struct str_data *) calloc(1, sizeof(struct str_data) + size + 1);

		str_ptr->max   = size + 1;
		str_ptr->flags = STR_FLAG_STACK;
		str_ptr->index = gtd->memory->stack_len++;

		gtd->memory->stack[str_ptr->index] = str_ptr;
	}
	else
	{
		str_ptr = gtd->memory->stack[gtd->memory->stack_len++];
	}

	if (str_ptr->max < size)
	{
		str_ptr = str_ptr_realloc(str_ptr, size);
	}

	str_ptr->len = 0;

	str = get_str_str(str_ptr);

	*str = 0;

	return str;
}

struct str_data *str_alloc_list(int size)
{
	struct str_data *str_ptr;
	char *str;

	if (size < 0)
	{
		tintin_printf2(gtd->ses, "str_alloc_list: negative size: %d", size);
		dump_stack();
		size = BUFFER_SIZE;
	}

	if (gtd->memory->free_len)
	{
		int index = gtd->memory->free[--gtd->memory->free_len];

		if (gtd->memory->free_low > gtd->memory->free_len)
		{
			gtd->memory->free_low = gtd->memory->free_len;
		}

		str_ptr = gtd->memory->list[index];

		if (str_ptr == NULL)
		{
			// empty slot left by str_pool_release

			str_ptr = (struct str_data *) calloc(1, sizeof(struct str_data) + size + 1);

			str_ptr->max   = size + 1;
			str_ptr->flags = STR_FLAG_LIST;
			str_ptr->index = index;

			gtd->memory->list[index] = str_ptr;

			gtd->memory->empty--;

			return str_ptr;
		}

		gtd->memory->free_full--;

		DEL_BIT(str_ptr->flags, STR_FLAG_FREE);

//		if (size >= str_ptr->max)
		if (size != str_ptr->max - 1)
		{
			str_ptr = str_ptr_realloc(str_ptr, size);
		}
		str_ptr->len = 0;
	}
	else
	{
		if (gtd->memory->list_len + 1 >= gtd->memory->list_max)
		{
			gtd->memory->list_max *= 2;

			gtd->memory->list = (struct str_data **) realloc(gtd->memory->list, sizeof(struct str_data *) * gtd->memory->list_max);
		}
		str_ptr = (struct str_data *) calloc(1, sizeof(struct str_data) + size + 1);

		str_ptr->max   = size + 1;
		str_ptr->flags = STR_FLAG_LIST;
		str_ptr->index = gtd->memory->list_len++;

		gtd->memory->list[str_ptr->index] = str_ptr;
	}

	str = get_str_str(str_ptr);

	*str = 0;

	return str_ptr;
}

// Freed strings stay in the pool for reuse, without a limit a burst of garbage
// inside one dispose window raises memory use for the life of the process.
// Called from memory_update() after the dispose list has been emptied.

static int str_pool_size(int len)
{
	int size = 2048;

	while (size <= len)
	{
		size *= 2;
	}
	return size;
}

static void str_pool_compact(void)
{
	struct memory_data *mem = gtd->memory;
	int i, j, k;

	for (i = j = 0 ; i < mem->list_len ; i++)
	{
		if (mem->list[i])
		{
			mem->list[j] = mem->list[i];
			mem->list[j]->index = j;
			j++;
		}
	}
	mem->list_len = j;

	for (i = k = 0 ; i < mem->list_len ; i++)
	{
		if (HAS_BIT(mem->list[i]->flags, STR_FLAG_FREE))
		{
			mem->free[k++] = i;
		}
	}
	mem->free_len  = k;
	mem->free_full = k;
	mem->free_low  = 0;
	mem->empty     = 0;

	mem->list_max = str_pool_size(mem->list_len + 1);
	mem->list     = (struct str_data **) realloc(mem->list, sizeof(struct str_data *) * mem->list_max);

	mem->free_max = str_pool_size(mem->free_len + 1);
	mem->free     = (int *) realloc(mem->free, sizeof(int) * mem->free_max);

	mem->compacted++;
}

void str_pool_release(void)
{
	struct memory_data *mem = gtd->memory;
	int index, top, released = 0;
	time_t now;

	if (mem->free_full <= STR_POOL_KEEP)
	{
		return;
	}

	// keep the top STR_POOL_KEEP entries (reused first), release the older ones

	top = mem->free_len - STR_POOL_KEEP;

	while (mem->free_low < top)
	{
		index = mem->free[mem->free_low++];

		if (mem->list[index])
		{
			free(mem->list[index]);

			mem->list[index] = NULL;

			mem->free_full--;
			mem->empty++;
			released++;
		}
	}

	if (released == 0)
	{
		return;
	}

	mem->released += released;

	now = time(NULL);

	if (mem->empty >= STR_POOL_COMPACT_MIN && mem->empty > mem->list_len / 2 && now - mem->last_compact >= 10)
	{
		mem->last_compact = now;

		str_pool_compact();
	}

#ifdef __GLIBC__
	if (now - mem->last_trim >= 1)
	{
		mem->last_trim = now;

		malloc_trim(0);
	}
#endif
}
