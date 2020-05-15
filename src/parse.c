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
	char *str3 = gtd->is_result;

	if (*str1 == 0)
	{
		return false;
	}

	if (*str2 == 0)
	{
		tintin_printf2(gtd->ses, "\e[1;31mis_abbrev(%s,%s)", str1, str2);

		dump_stack();

		return false;
	}

	while (true)
	{
		if (*str1 == 0)
		{
			strcpy(str3, str2);

			return true;
		}

		if (case_table[(int) *str1] != case_table[(int) *str2])
		{
			return false;
		}
		str1++;
		*str3++ = *str2++;
	}
}

void filename_string(char *input, char *output)
{
	while (*input)
	{
		*output++ = (char) case_table[(int) *input++];
	}
	*output = 0;
}

int is_suffix(char *str1, char *str2)
{
	int len1, len2;

	len1 = strlen(str1);
	len2 = strlen(str2);

	if (len1 >= len2)
	{
		if (is_abbrev(str1 + len1 - len2, str2))
		{
			return TRUE;
		}
	}
	return FALSE;
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

struct session *execute(struct session *ses, char *format, ...)
{
	char *buffer;
	va_list args;

	va_start(args, format);
	vasprintf(&buffer, format, args);
	va_end(args);

	if (*buffer)
	{
		if (*buffer != gtd->tintin_char)
		{
			*buffer = gtd->tintin_char;
		}
		get_arg_all(ses, buffer, buffer, FALSE);
	}

	ses = script_driver(ses, LIST_COMMAND, buffer);

	free(buffer);

	return ses;
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

	if (VERBATIM(ses))
	{
		line = (char *) malloc(BUFFER_SIZE);

		sub_arg_all(ses, input, line, 1, SUB_SEC);

		if (check_all_aliases(ses, line))
		{
			ses = script_driver(ses, LIST_ALIAS, line);
		}
		else if (HAS_BIT(ses->flags, SES_FLAG_SPEEDWALK) && is_speedwalk(ses, line))
		{
			process_speedwalk(ses, line);
		}
		else
		{
			write_mud(ses, line, SUB_EOL);
		}

		free(line);

		pop_call();
		return ses;
	}

	if (*input == gtd->verbatim_char)
	{
		write_mud(ses, input+1, SUB_EOL);

		pop_call();
		return ses;
	}

	line = (char *) malloc(BUFFER_SIZE);

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
		else if (HAS_BIT(ses->flags, SES_FLAG_SPEEDWALK) && is_speedwalk(ses, line))
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

	free(line);

	pop_call();
	return ses;
}

/*
	Deal with variables and functions used as commands.
*/

struct session *parse_command(struct session *ses, char *input)
{
	char *arg, line[BUFFER_SIZE], cmd1[BUFFER_SIZE], cmd2[BUFFER_SIZE];

	push_call("parse_command(%p,%p)",ses,input);

	arg = get_arg_stop_spaces(ses, input, cmd1, GET_ONE);

	substitute(ses, cmd1, cmd2, SUB_VAR|SUB_FUN);

	if (!strcmp(cmd1, cmd2))
	{
		pop_call();
		return NULL;
	}

	sprintf(line, "%s%s%s", cmd2, *arg ? " " : "", arg);

	strcpy(input, line);

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
		while (isspace((int) *pti))
		{
			pti++;
		}

		if (isdigit((int) *pti))
		{
			ptn = num;

			while (isdigit((int) *pti))
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
		}
		else
		{
			max = 1;
		}

		pti = get_arg_stop_digits(ses, pti, name, GET_ONE);

		if (*name == 0)
		{
			break;
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
		if (isdigit((int) *input))
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

	input = get_arg_stop_spaces(ses, input, line, GET_ONE);

	substitute(ses, line, line, SUB_VAR|SUB_FUN);

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

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "UNKNOWN COMMAND", line);

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
	char *buffer = str_alloc(UMAX(strlen(string), BUFFER_SIZE));

	string = get_arg_all(ses, string, buffer, verbatim);

	substitute(ses, buffer, result, sub);

	str_free(buffer);

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
	char *buffer = str_alloc(strlen(string) + BUFFER_SIZE);

	string = get_arg_in_braces(ses, string, buffer, flag);

	substitute(ses, buffer, result, sub);

	str_free(buffer);

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
		else if (isspace((int) *pti) && nest == 0)
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
		else if (isdigit((int) *pti) && nest == 0)
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
	while (isspace((int) *string))
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
			check_append_path(ses, command, NULL, 1);
		}
	}

	if (gtd->level->ignore == 0 && ses->map && ses->map->in_room && ses->map->nofollow == 0)
	{
		if (follow_map(ses, command))
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
	char *strip;

	push_call("do_one_line(%s,%p)",ses->name,line);

	if (gtd->level->ignore)
	{
		pop_call();
		return;
	}

	strip = str_alloc_stack();

	strip_vt102_codes(line, strip);

	if (!HAS_BIT(ses->list[LIST_ACTION]->flags, LIST_FLAG_IGNORE))
	{
		check_all_actions(ses, line, strip);
	}

	if (!HAS_BIT(ses->list[LIST_PROMPT]->flags, LIST_FLAG_IGNORE))
	{
		check_all_prompts(ses, line, strip, TRUE);
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

	pop_call();
	return;
}
