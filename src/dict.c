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
*                      coded by Igor van den Hoven 2019                       *
******************************************************************************/

#include "tintin.h"

#include "dict.h"

struct dictionary_data
{
	unsigned int  * wordindex[26];
	int             listsize[26];
};

struct dictionary_data *dictionary;

void dictionary_init()
{
	char *pta;
	int index, hash;

	dictionary = calloc(1, sizeof(struct dictionary_data));

	for (hash = 0 ; hash < 26 ; hash++)
	{
		index = 1;

		pta = wordlist[hash];

		do
		{
			pta++;

			if (*pta == 0)
			{
				index++;
				pta++;
			}
		}
		while (*pta);

		dictionary->listsize[hash] = index;

		dictionary->wordindex[hash] = calloc(index, sizeof(int));
	}

	for (hash = 0 ; hash < 26 ; hash++)
	{
		index = 1;

		pta = wordlist[hash] + 1;

		do
		{
			dictionary->wordindex[hash][index++] = pta - wordlist[hash];

			while (*pta)
			{
				pta++;
			}
			pta++;
		}
		while (*pta);
	}

//	for (hash = 0 ; hash < 26 ; hash++)
//	{
//		printf("hash %2d = %d\n", hash, dictionary->listsize[hash]);
//	}
}

int dictionary_search(int hash, char *key)
{
	register int mid, i, bot;
	register char val;

	bot = 0;
	i = dictionary->listsize[hash] - 1;
	mid = i / 2;

	while (mid)
	{
		val = strcmp(key, wordlist[hash] + dictionary->wordindex[hash][i - mid]);

//		printf("debug: %5d '%c' %s\n", i - mid, 'a' + hash, wordlist[hash] + dictionary->wordindex[hash][i - mid]);

		if (val < 0)
		{
			i -= mid + 1;
		}
		else if (val > 0)
		{
			bot = i - mid + 1;
		}
		else
		{
			return i - mid;
		}
		mid = (i - bot) / 2;
	}

	if (i > bot)
	{
		val = strcmp(key, wordlist[hash] + dictionary->wordindex[hash][i]);

		if (val > 0)
		{
			return -1;
		}
		else if (val < 0)
		{
			--i;
		}
		else
		{
			return i;
		}
	}

	if (!strcmp(key, wordlist[hash] + dictionary->wordindex[hash][i]))
	{
		return i;
	}
	return -1;
}

void dictionary_lowerstring(char *in, char *out)
{
	char *pti, *pto;

	pti = in;
	pto = out;

	while (*pti)
	{
		if (is_alpha(*pti))
		{
			*pto++ = tolower(*pti++);
		}
		else
		{
			pti++;
		}
	}
	*pto = 0;
}

int spellcheck_count(struct session *ses, char *in)
{
	char *arg, arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int cnt, hash, index;

	if (dictionary == NULL)
	{
		dictionary_init();
	}

	cnt = 0;
	arg = in;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

		dictionary_lowerstring(arg1, arg2);

		if (is_alpha(*arg2))
		{
			hash = *arg2 - 'a';

			index = dictionary_search(hash, arg2 + 1);

			if (index == -1)
			{
				cnt++;
			}
		}

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}
	return cnt;
}

DO_COMMAND(do_dictionary)
{
	int hash, index;

	if (dictionary == NULL)
	{
		dictionary_init();
	}

	sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 || !is_alpha(*arg1))
	{
		show_message(ses, LIST_COMMAND, "#SYNTAX: #DICTIONARY {WORD}");

		return ses;
	}

	arg = arg1;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, arg2, GET_ONE);

		dictionary_lowerstring(arg2, arg3);

		if (is_alpha(*arg3))
		{
			hash = *arg3 - 'a';

			index = dictionary_search(hash, arg3 + 1);

			if (index == -1)
			{
				tintin_printf2(ses, "\e[1;31m%s", arg2);
			}
			else
			{
				tintin_printf2(ses, "\e[1;32m%s", arg2);
			}
		}

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}
	return ses;
}
