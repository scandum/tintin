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

struct str_hash_index_data str_hash_index[MAX_STR_HASH];

int str_hash_len;

/*
	Performs as well as Jenkin's one-at-a-time hash.- Igor
*/

int generate_str_hash_key(char *str)
{
	unsigned int h = 4321;

	for (str_hash_len = 0 ; *str != 0 ; str++, str_hash_len++)
	{
		h = ((h << 5) + h) + *str;
	}

	h += str_hash_len;

	return h % MAX_STR_HASH;
}

unsigned long long generate_hash_key(char *str)
{
	unsigned long long len, h = 4321;

	for (len = 0 ; *str != 0 ; str++, len++)
	{
		h = ((h << 5) + h) + *str;
	}

	h += len;

	return h;
}

char *str_hash(char *str, int lines)
{
	int hash;
	struct str_hash_data *hash_ptr;

	hash = generate_str_hash_key(str);

	for (hash_ptr = str_hash_index[hash].f_node ; hash_ptr ; hash_ptr = hash_ptr->next)
	{
		if (str_hash_len == hash_ptr->length && !strcmp(str, (char *) hash_ptr + gtd->str_hash_size))
		{
			hash_ptr->count++;

			return (char *) hash_ptr + gtd->str_hash_size;
		}
	}
	hash_ptr = (struct str_hash_data *) calloc(1, str_hash_len + 1 + gtd->str_hash_size);

	hash_ptr->count  = 1;
	hash_ptr->lines  = lines;
	hash_ptr->length = str_hash_len;
	hash_ptr->hash   = hash;
	strcpy((char *) hash_ptr + gtd->str_hash_size, str);

	LINK(hash_ptr, str_hash_index[hash].f_node, str_hash_index[hash].l_node);

	return (char *) hash_ptr + gtd->str_hash_size;
}


char *str_unhash(char *str)
{
	struct str_hash_data *hash_ptr;

	hash_ptr = (struct str_hash_data *) (str - gtd->str_hash_size);

	if (--hash_ptr->count == 0)
	{
		UNLINK(hash_ptr, str_hash_index[hash_ptr->hash].f_node, str_hash_index[hash_ptr->hash].l_node);
		free(hash_ptr);
	}
	return NULL;
}


int str_hash_lines(char *str)
{
	return ((struct str_hash_data *) (str - gtd->str_hash_size))->lines;
}

int str_hash_length(char *str)
{
	return ((struct str_hash_data *) (str - gtd->str_hash_size))->length;
}

int str_hash_grep(char *str, int write)
{
	struct str_hash_data *hash_ptr;

	hash_ptr = ((struct str_hash_data *) (str - gtd->str_hash_size));

	if (write)
	{
		SET_BIT(hash_ptr->flags, STR_HASH_FLAG_NOGREP);
	}

	return HAS_BIT(hash_ptr->flags, STR_HASH_FLAG_NOGREP);
}

void reset_hash_table(void)
{
	char temp[STRING_SIZE];
	struct str_hash_data *hash_ptr;
	int hash;

	push_call("reset_hash_table()");

	for (hash = 0 ; hash < MAX_STR_HASH ; hash++)
	{
		for (hash_ptr = str_hash_index[hash].f_node ; hash_ptr ; hash_ptr = hash_ptr->next)
		{
			if (hash_ptr->length < gtd->screen->cols)
			{
				hash_ptr->lines = 1;
			}
			else
			{
				hash_ptr->lines = word_wrap(gtd->ses, (char *) hash_ptr + gtd->str_hash_size, temp, FALSE);
			}
		}
	}
	pop_call();
	return;
}

void str_hash_info(struct session *ses)
{
	struct str_hash_data *hash_ptr;
	int hash, hash_cnt, hash_max, index_cnt, string_cnt, pointer_cnt, hashed_size, unhashed_size;

	hash = hash_cnt = hash_max = index_cnt = string_cnt = pointer_cnt = hashed_size = unhashed_size = 0;

	for (hash = 0 ; hash < MAX_STR_HASH ; hash++)
	{
		if (str_hash_index[hash].f_node)
		{
			index_cnt++;
		}

		for (hash_cnt = 0, hash_ptr = str_hash_index[hash].f_node ; hash_ptr ; hash_ptr = hash_ptr->next)
		{
			if (hash != generate_str_hash_key((char *) hash_ptr + gtd->str_hash_size))
			{
				tintin_printf2(ses, "corrupted hash node: %s", (char *) hash_ptr + gtd->str_hash_size);
			}
			hash_cnt          += 1;
			string_cnt        += 1;
			pointer_cnt       += hash_ptr->count;
			hashed_size       += gtd->str_hash_size + strlen((char *) hash_ptr + gtd->str_hash_size);
			unhashed_size     += hash_ptr->count    * strlen((char *) hash_ptr + gtd->str_hash_size);
		}
		if (hash_cnt > hash_max)
		{
			hash_max = hash_cnt;
		}
	}
	tintin_printf2(ses, "Total string count:   %8d", string_cnt);
	tintin_printf2(ses, "Total pointer count:  %8d", pointer_cnt);
	tintin_printf2(ses, "Total buckets count:  %8d", index_cnt);
	tintin_printf2(ses, "Fullest bucket count: %8d", hash_max);
	tintin_printf2(ses, "Average bucket usage: %8.2f", (float) string_cnt / index_cnt);
	tintin_printf2(ses, "Total memory usage:   %8d", hashed_size + MAX_STR_HASH * sizeof(struct str_hash_index_data));
	tintin_printf2(ses, "Total memory saved:   %8d", unhashed_size - hashed_size - MAX_STR_HASH * sizeof(struct str_hash_index_data));
}
