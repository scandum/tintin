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
*                         coded by Peter Unold 1992                           *
*                     recoded by Igor van den Hoven 2005                      *
******************************************************************************/

#include "tintin.h"

// whether str1 is an abbreviation of str2

int case_table[256] =
{
	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
	 95,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
	 64,
	 65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,
	 91,  92,  93,  94,
	 95,  96,
	 65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,
	123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

int is_abbrev(char *str1, char *str2)
{
	char buf[NUMBER_SIZE], *str3;

	str3 = buf;

	if (*str1 == 0)
	{
		return FALSE;
	}

	if (*str2 == 0)
	{
		tintin_printf2(gtd->ses, "\e[1;31mis_abbrev(%s,%s)", str1, str2);

		dump_stack();

		return FALSE;
	}

	while (TRUE)
	{
		if (*str1 == 0)
		{
			*str3 = 0;

			strcpy(gtd->is_result, buf);

			return TRUE;
		}

		if (case_table[(int) *str1] != case_table[(int) *str2])
		{
			return FALSE;
		}
		str1++;

		*str3++ = *str2++;
	}
}

int is_member(char *str1, char *str2)
{
	char *pt1, *pt2;

	if (*str1 == 0)
	{
		return FALSE;
	}

	if (*str2 == 0)
	{
		tintin_printf2(gtd->ses, "\e[1;31mis_member(%s,%s)", str1, str2);

		dump_stack();

		return FALSE;
	}

	while (*str1)
	{
		if (case_table[(int) *str1] == case_table[(int) *str2])
		{
			pt1 = str1;
			pt2 = str2;

			while (case_table[(int) *pt1] == case_table[(int) *pt2])
			{
				pt1++;
				pt2++;

				if (*pt1 == ' ' || *pt1 == 0)
				{
					return TRUE;
				}
			}
		}

		while (*str1 && *str1 != ' ')
		{
			str1++;
		}

		if (*str1)
		{
			str1++;
		}
	}
	return FALSE;
}

void filename_string(char *input, char *output)
{
	while (*input)
	{
		*output++ = (char) case_table[(int) *input++];
	}
	*output = 0;
}

int is_vowel(char *str)
{
	switch (case_table[(int) *str])
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
			return TRUE;
	}
	return FALSE;
}


struct session *parse_input(struct session *ses, char *input)
{
	char *line;

	push_call("parse_input(%s,%s)",ses->name,input);

	if (*input == 0)
	{
		write_mud(ses, input, SUB_EOL);

		pop_call();
		return ses;
	}

	line = str_alloc_stack(0);

	if (VERBATIM(ses))
	{
		sub_arg_all(ses, input, line, 1, SUB_SEC);

		if (check_all_aliases(ses, line))
		{
			ses = script_driver(ses, LIST_ALIAS, line);
		}
		else if (HAS_BIT(ses->config_flags, CONFIG_FLAG_SPEEDWALK) && is_speedwalk(ses, line))
		{
			process_speedwalk(ses, line);
		}
		else
		{
			write_mud(ses, line, SUB_EOL|SUB_ESC);
		}

		pop_call();
		return ses;
	}

	if (*input == gtd->verbatim_char)
	{
		write_mud(ses, input+1, SUB_EOL);

		pop_call();
		return ses;
	}

	while (*input)
	{
		input = space_out(input);

		input = get_arg_all(ses, input, line, 0);

		if (parse_command(ses, line))
		{
			ses = script_driver(ses, LIST_COMMAND, line);
		}
		else if (check_all_aliases(ses, line))
		{
			ses = script_driver(ses, LIST_ALIAS, line);
		}
		else if (HAS_BIT(ses->config_flags, CONFIG_FLAG_SPEEDWALK) && is_speedwalk(ses, line))
		{
			process_speedwalk(ses, line);
		}
		else
		{
			write_mud(ses, line, SUB_VAR|SUB_FUN|SUB_ESC|SUB_EOL);
		}

		if (*input == COMMAND_SEPARATOR)
		{
			input++;
		}
	}

	pop_call();
	return ses;
}

/*
	Deal with variables and functions used as commands.
*/

struct session *parse_command(struct session *ses, char *input)
{
	char *arg, *arg1;

	push_call("parse_command(%p,%p)",ses,input);

	arg1 = str_alloc_stack(0);

	arg = sub_arg_stop_spaces(ses, input, arg1, GET_ONE, SUB_VAR|SUB_FUN);

	if (!strncmp(input, arg1, strlen(arg1)))
	{
		pop_call();
		return NULL;
	}

	if (*arg)
	{
		cat_sprintf(arg1, " %s", arg);
	}
	strcpy(input, arg1);

	pop_call();
	return ses;
}

char *substitute_speedwalk(struct session *ses, char *input, char *output)
{
	char num[NUMBER_SIZE], name[BUFFER_SIZE], *pti, *ptn, *pto;
	int cnt, max;

	pti = input;
	pto = output;

	while (*pti && pto - output < INPUT_SIZE)
	{
		if (is_space(*pti))
		{
			return input;
		}

		if (is_digit(*pti))
		{
			ptn = num;

			while (is_digit(*pti))
			{
				if (ptn - num < 4)
				{
					*ptn++ = *pti++;
				}
				else
				{
					pti++;
				}
			}
			*ptn = 0;

			max = atoi(num);

			if (*pti == 0)
			{
				return input;
			}
		}
		else
		{
			max = 1;
		}

		pti = get_arg_stop_digits(ses, pti, name, GET_ONE);

		if (*name == 0 || !is_pathdir(ses, name))
		{
			return input;
		}

		for (cnt = 0 ; cnt < max ; cnt++)
		{
			if (output != pto)
			{
				*pto++ = COMMAND_SEPARATOR;
			}
			pto += sprintf(pto, "%s", name);
		}

		if (*pti == COMMAND_SEPARATOR)
		{
			pti++;
		}
	}
	*pto = 0;

	return output;
}

int is_speedwalk(struct session *ses, char *input)
{
	int digit = 0, flag = FALSE;

	while (*input)
	{
		switch (*input)
		{
			case 'n':
			case 'e':
			case 's':
			case 'w':
			case 'u':
			case 'd':
				if (digit > 3)
				{
					return FALSE;
				}
				digit = 0;
				flag  = TRUE;
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				digit++;
				flag = FALSE;
				break;

			default:
				return FALSE;
		}
		input++;
	}
	return flag;
}


void process_speedwalk(struct session *ses, char *input)
{
	char dir[2];
	int cnt, i;

	for (dir[1] = 0 ; *input ; input++)
	{
		if (is_digit(*input))
		{
			sscanf(input, "%d%c", &cnt, dir);

			for (i = 0 ; i < cnt ; i++)
			{
				write_mud(ses, dir, SUB_EOL);
			}

			while (*input != dir[0])
			{
				input++;
			}
		}
		else
		{
			dir[0] = *input;

			write_mud(ses, dir, SUB_EOL);
		}
	}
	return;
}

/*
	Deals with all # stuff
*/

struct session *parse_tintin_command(struct session *ses, char *input)
{
	char line[BUFFER_SIZE];
	struct session *sesptr;

	input = sub_arg_in_braces(ses, input, line, GET_ONE, SUB_VAR|SUB_FUN);

	if (is_number(line))
	{
		int cnt = atoi(line);

		input = get_arg_in_braces(ses, input, line, GET_ALL);

		while (cnt-- > 0)
		{
			ses = script_driver(ses, LIST_COMMAND, line);
		}
		return ses;
	}

	sesptr = find_session(line);

	if (sesptr)
	{
		if (*input)
		{
			input = get_arg_in_braces(ses, input, line, GET_ALL);

			substitute(ses, line, line, SUB_VAR|SUB_FUN);

			script_driver(sesptr, LIST_COMMAND, line);

			return ses;
		}
		else
		{
			return activate_session(sesptr);
		}
	}

	if (*line == '!')
	{
		show_error(ses, LIST_COMMAND, "#!%s %s", line + 1, input);

		return ses;
	}

	tintin_printf2(ses, "#ERROR: #UNKNOWN TINTIN-COMMAND '%s'.", line);

	check_all_events(ses, SUB_SEC|EVENT_FLAG_SYSTEM, 0, 1, "UNKNOWN COMMAND", line);

	return ses;
}


int cnt_arg_all(struct session *ses, char *string, int flag)
{
	char *arg, tmp[BUFFER_SIZE];
	int cnt;

	arg = string;
	cnt = 0;

	while (*arg)
	{
		cnt++;

		arg = get_arg_in_braces(ses, arg, tmp, flag);

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}
	return cnt;
}

/*
	get all arguments - only check for unescaped command separators
*/

char *get_arg_all(struct session *ses, char *string, char *result, int verbatim)
{
	char *pto, *pti;
	int skip, nest = 0;

	pti = string;
	pto = result;

	if (*pti == gtd->verbatim_char)
	{
		while (*pti)
		{
			*pto++ = *pti++;
		}
		*pto = 0;

		return pti;
	}

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		skip = find_secure_color_code(pti);

		if (skip)
		{
			while (skip--)
			{
				*pto++ = *pti++;
			}
			continue;
		}

		if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR)
		{
			*pto++ = *pti++;
		}
		else if (*pti == COMMAND_SEPARATOR && nest == 0 && !verbatim)
		{
			break;
		}
		else if (*pti == DEFAULT_OPEN)
		{
			nest++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest--;
		}
		*pto++ = *pti++;

		if (pto - result >= BUFFER_SIZE - 3)
		{
			tintin_printf2(ses, "#ERROR: INPUT BUFFER OVERFLOW.");

			pto--;

			break;
		}
	}
	*pto = '\0'; 

	return pti;
}

char *sub_arg_all(struct session *ses, char *string, char *result, int verbatim, int sub)
{
	char *buffer;

	if (*string == 0)
	{
		*result = 0;

		return string;
	}

	push_call("sub_arg_all(%p,%p,%p,%d,%d)",ses,string,result,verbatim,sub);

	buffer = str_alloc_stack(strlen(string));

	string = get_arg_all(ses, string, buffer, verbatim);

	substitute(ses, buffer, result, sub);

	pop_call();
	return string;
}

/*
	Braces are stripped in braced arguments leaving all else as is.
*/

char *get_arg_in_braces(struct session *ses, char *string, char *result, int flag)
{
	char *pti, *pto;
	int skip, nest = 1;

	pti = space_out(string);
	pto = result;

	if (*pti != DEFAULT_OPEN)
	{
		if (!HAS_BIT(flag, GET_ALL))
		{
			pti = get_arg_stop_spaces(ses, pti, result, flag);
		}
		else
		{
			pti = get_arg_with_spaces(ses, pti, result, flag);
		}
		return pti;
	}

	pti++;

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		skip = find_secure_color_code(pti);

		if (skip)
		{
			while (skip--)
			{
				*pto++ = *pti++;
			}
			continue;
		}

		if (*pti == DEFAULT_OPEN)
		{
			nest++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest--;

			if (nest == 0)
			{
				break;
			}
		}
		*pto++ = *pti++;
	}

	if (*pti == 0)
	{
		tintin_printf2(ses, "#ERROR: GET BRACED ARGUMENT: UNMATCHED BRACE.");
	}
	else
	{
		pti++;
	}
	*pto = '\0';

	return pti;
}

char *sub_arg_in_braces(struct session *ses, char *string, char *result, int flag, int sub)
{
	char *buffer;

	if (*string == 0)
	{
		*result = 0;
		
		return string;
	}

	push_call("sub_arg_in_braces(%p,%p,%p,%d,%d)",ses,string,result,flag,sub);

	buffer = str_alloc_stack(strlen(string) * 2);

	string = get_arg_in_braces(ses, string, buffer, flag);

	substitute(ses, buffer, result, sub);

	pop_call();
	return string;
}

/*
	get all arguments
*/

char *get_arg_with_spaces(struct session *ses, char *string, char *result, int flag)
{
	char *pto, *pti;
	int skip, nest = 0;

	pti = space_out(string);
	pto = result;

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		skip = find_secure_color_code(pti);

		if (skip)
		{
			while (skip--)
			{
				*pto++ = *pti++;
			}
			continue;
		}

		if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR)
		{
			*pto++ = *pti++;
		}
		else if (*pti == COMMAND_SEPARATOR && nest == 0)
		{
			break;
		}
		else if (*pti == DEFAULT_OPEN)
		{
			nest++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest--;
		}
		*pto++ = *pti++;
	}
	*pto = '\0'; 

	return pti;
}

/*
	get one arg, stop at spaces
*/

char *get_arg_stop_spaces(struct session *ses, char *string, char *result, int flag)
{
	char *pto, *pti;
	int skip, nest = 0;

	pti = space_out(string);
	pto = result;

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		skip = find_secure_color_code(pti);

		if (skip)
		{
			while (skip--)
			{
				*pto++ = *pti++;
			}
			continue;
		}


		if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR)
		{
			*pto++ = *pti++;
		}
		else if (*pti == COMMAND_SEPARATOR && nest == 0)
		{
			break;
		}
		else if (is_space(*pti) && nest == 0)
		{
			pti++;
			break;
		}
		else if (*pti == DEFAULT_OPEN)
		{
			nest++;
		}
		else if (*pti == '[' && HAS_BIT(flag, GET_NST))
		{
			nest++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest--;
		}
		else if (*pti == ']' && HAS_BIT(flag, GET_NST))
		{
			nest--;
		}
		*pto++ = *pti++;
	}
	*pto = '\0';

	return pti;
}

char *sub_arg_stop_spaces(struct session *ses, char *string, char *result, int flag, int sub)
{
	char *buffer;

	if (*string == 0)
	{
		*result = 0;

		return string;
	}

	push_call("sub_arg_stop_braces(%p,%p,%p,%d,%d)",ses,string,result,flag,sub);

	buffer = str_alloc_stack(strlen(string) * 2);

	string = get_arg_stop_spaces(ses, string, buffer, flag);

	substitute(ses, buffer, result, sub);

	pop_call();
	return string;
}

// Get one arg, stop at numbers, used for speedwalks

char *get_arg_stop_digits(struct session *ses, char *string, char *result, int flag)
{
	char *pto, *pti;
	int nest = 0;

	pti = space_out(string);
	pto = result;

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR)
		{
			*pto++ = *pti++;
		}
		else if (*pti == COMMAND_SEPARATOR && nest == 0)
		{
			break;
		}
		else if (is_digit(*pti) && nest == 0)
		{
			break;
		}
		else if (*pti == DEFAULT_OPEN)
		{
			nest++;
		}
		else if (*pti == '[' && HAS_BIT(flag, GET_NST))
		{
			nest++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest--;
		}
		else if (*pti == ']' && HAS_BIT(flag, GET_NST))
		{
			nest--;
		}
		*pto++ = *pti++;
	}
	*pto = '\0';

	return pti;
}

/*
	advance ptr to next none-space
*/

char *space_out(char *string)
{
	while (is_space(*string))
	{
		string++;
	}
	return string;
}

/*
	For variable handling
*/

char *get_arg_to_brackets(struct session *ses, char *string, char *result)
{
	char *pti, *pto, *ptii, *ptoo;
	int nest1 = 0, nest2 = 0, nest3 = 0;

	pti = space_out(string);
	pto = result;
	ptii = ptoo = NULL;

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '[')
		{
			nest2++;

			if (nest1 == 0 && ptii == NULL)
			{
				ptii = pti;
				ptoo = pto;
			}
		}
		else if (*pti == ']')
		{
			if (nest2)
			{
				nest2--;
			}
			else
			{
				nest3 = 1;
			}

			if (*(pti+1) == 0 && ptii && nest1 == 0 && nest2 == 0 && nest3 == 0)
			{
				*ptoo = 0;

				return ptii;
			}
		}
		else if (*pti == DEFAULT_OPEN)
		{
			nest1++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest1--;
		}
		*pto++ = *pti++;
	}
	*pto = 0;

	return pti;
}

char *get_arg_at_brackets(struct session *ses, char *string, char *result)
{
	char *pti, *pto;
	int nest = 0;

	pti = string;
	pto = result;

	if (*pti != '[')
	{
		*pto = 0;

		return pti;
	}

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '[')
		{
			nest++;
		}
		else if (*pti == ']')
		{
			if (nest)
			{
				nest--;
			}
			else
			{
				break;
			}
		}
		else if (nest == 0)
		{
			break;
		}
		*pto++ = *pti++;
	}

	if (nest)
	{
		tintin_printf2(NULL, "#ERROR: GET BRACKETED VARIABLE: UNMATCHED BRACKET.");
	}
	*pto = 0;

	return pti;
}

char *get_arg_in_brackets(struct session *ses, char *string, char *result)
{
	char *pti, *pto;
	int nest = 1;

	pti = string;
	pto = result;

	if (*pti != '[')
	{
		*pto = 0;

		return pti;
	}

	pti++;

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '[')
		{
			nest++;
		}
		else if (*pti == ']')
		{
			nest--;

			if (nest == 0)
			{
				break;
			}
		}
		*pto++ = *pti++;
	}

	if (*pti == 0)
	{
		tintin_printf2(NULL, "#ERROR: GET BRACKETED ARGUMENT: UNMATCHED BRACKET.");
	}
	else
	{
		pti++;
	}
	*pto = 0;

	return pti;
}

char *get_char(struct session *ses, char *string, char *result)
{
	char *pti = string;

	if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
	{
		pti += sprintf(result, "%c%c", pti[0], pti[1]);
	}
	else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(pti))
	{
		pti += sprintf(result, "%.*s", get_utf8_size(pti), pti);
	}
	else
	{
		pti += sprintf(result, "%c", pti[0]);
	}

	return pti;
}

/*
	send command to the mud
*/

void write_mud(struct session *ses, char *command, int flags)
{
	char output[BUFFER_SIZE];
	int size;

	size = substitute(ses, command, output, flags);

	if (gtd->level->ignore == 0 && HAS_BIT(ses->flags, SES_FLAG_PATHMAPPING))
	{
		if (ses->map == NULL || ses->map->nofollow == 0)
		{
			check_append_path(ses, command, NULL, 0.0, 1);
		}
	}

	if (gtd->level->ignore == 0 && ses->map && ses->map->in_room && ses->map->nofollow == 0)
	{
		int quiet, follow;

		quiet = HAS_BIT(ses->map->flags, MAP_FLAG_QUIET);

		gtd->level->input += quiet;

		follow = follow_map(ses, command);

		gtd->level->input -= quiet;

		if (follow)
		{
			return;
		}
	}

	write_line_mud(ses, output, size);
}


/*
	Check line for triggers
*/

void do_one_line(char *line, struct session *ses)
{
	char *strip, *buf;

	if (gtd->level->ignore)
	{
		return;
	}

	push_call("do_one_line(%s,%p)",ses->name,line);

	push_script_stack(ses, LIST_VARIABLE);

	strip = str_alloc_stack(0);
	buf   = str_alloc_stack(0);

	strip_vt102_codes(line, strip);

	if (!HAS_BIT(ses->list[LIST_ACTION]->flags, LIST_FLAG_IGNORE))
	{
		check_all_actions(ses, line, strip, buf);
	}

	if (!HAS_BIT(ses->list[LIST_PROMPT]->flags, LIST_FLAG_IGNORE))
	{
		check_all_prompts(ses, line, strip);
	}

	if (!HAS_BIT(ses->list[LIST_GAG]->flags, LIST_FLAG_IGNORE))
	{
		check_all_gags(ses, line, strip);
	}

	if (!HAS_BIT(ses->list[LIST_SUBSTITUTE]->flags, LIST_FLAG_IGNORE))
	{
		check_all_substitutions(ses, line, strip);
	}

	if (!HAS_BIT(ses->list[LIST_HIGHLIGHT]->flags, LIST_FLAG_IGNORE))
	{
		check_all_highlights(ses, line, strip);
	}

	if (HAS_BIT(ses->logmode, LOG_FLAG_NEXT))
	{
		logit(ses, line, ses->lognext_file, LOG_FLAG_LINEFEED);

		DEL_BIT(ses->logmode, LOG_FLAG_NEXT);
	}

	pop_script_stack();

	pop_call();
	return;
}
