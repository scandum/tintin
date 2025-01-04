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
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include <sys/types.h>
#include <pcre.h>

#include "tintin.h"


int match(struct session *ses, char *str, char *exp, int sub)
{
	char expbuf[BUFFER_SIZE];

	sprintf(expbuf, "\\A%s\\Z", exp);

	substitute(ses, expbuf, expbuf, sub);

	return tintin_regexp(ses, NULL, str, expbuf, 0, 0);
}

int find(struct session *ses, char *str, char *exp, int sub, int flag)
{
	if (HAS_BIT(sub, SUB_VAR|SUB_FUN))
	{
		char expbuf[BUFFER_SIZE], strbuf[BUFFER_SIZE];

		substitute(ses, str, strbuf, SUB_VAR|SUB_FUN);
		substitute(ses, exp, expbuf, SUB_VAR|SUB_FUN);

		return tintin_regexp(ses, NULL, strbuf, expbuf, 0, flag);
	}
	else
	{
		return tintin_regexp(ses, NULL, str, exp, 0, flag);
	}
}

DO_COMMAND(do_regexp)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg3 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #REGEXP <TEXT> <EXPRESSION> <TRUE> [FALSE]");
	}
	else
	{
		if (tintin_regexp(ses, NULL, arg1, arg2, 0, REGEX_FLAG_CMD))
		{
			substitute(ses, arg3, arg3, SUB_CMD);

			ses = script_driver(ses, LIST_COMMAND, arg3);
		}
		else
		{
			arg4 = str_alloc_stack(0);

			arg = get_arg_in_braces(ses, arg, arg4, GET_ALL);

			if (*arg4)
			{
				ses = script_driver(ses, LIST_COMMAND, arg4);
			}
		}
	}
	return ses;
}

int regexp_compare(struct session *ses, pcre *nodepcre, char *str, char *exp, int comp_option, int flag)
{
	pcre *regex;
	int i, j, matches;

	if (nodepcre == NULL)
	{
		regex = regexp_compile(ses, exp, comp_option);
	}
	else
	{
		regex = nodepcre;
	}

	if (regex == NULL)
	{
		return FALSE;
	}

	matches = pcre_exec(regex, NULL, str, strlen(str), 0, 0, gtd->match, 303);

	if (matches <= 0)
	{
		if (nodepcre == NULL)
		{
			free(regex);
		}
		return FALSE;
	}

	// REGEX_FLAG_FIX handles %1 to %99 usage. Backward compatibility.

	switch (flag)
	{
		case REGEX_FLAG_CMD:
			for (i = matches ; i < gtd->cmdc ; i++)
			{
				*gtd->cmds[i] = 0;
			}

			for (i = 0 ; i < matches ; i++)
			{
				gtd->cmds[i] = restringf(gtd->cmds[i], "%.*s", gtd->match[i*2+1] - gtd->match[i*2], &str[gtd->match[i*2]]);
			}
			gtd->cmdc = matches;
			break;

		case REGEX_FLAG_CMD + REGEX_FLAG_FIX:
			for (i = matches ; i < gtd->cmdc ; i++)
			{
				*gtd->cmds[i] = 0;
			}

			for (i = 0 ; i < matches ; i++)
			{
				j = gtd->args[i];

				gtd->cmds[j] = restringf(gtd->cmds[j], "%.*s", gtd->match[i*2+1] - gtd->match[i*2], &str[gtd->match[i*2]]);
			}
			gtd->cmdc = matches;
			break;

		case REGEX_FLAG_ARG:
			for (i = matches ; i < gtd->varc ; i++)
			{
				*gtd->vars[i] = 0;
			}

			for (i = 0 ; i < matches ; i++)
			{
				gtd->vars[i] = restringf(gtd->vars[i], "%.*s", gtd->match[i*2+1] - gtd->match[i*2], &str[gtd->match[i*2]]);
			}
			gtd->varc = matches;
			break;

		case REGEX_FLAG_ARG + REGEX_FLAG_FIX:
			for (i = matches ; i < gtd->varc ; i++)
			{
				*gtd->vars[i] = 0;
			}

			for (i = 0 ; i < matches ; i++)
			{
				j = gtd->args[i];

				gtd->vars[j] = restringf(gtd->vars[j], "%.*s", gtd->match[i*2+1] - gtd->match[i*2], &str[gtd->match[i*2]]);
			}
			gtd->varc = matches;
			break;
	}

	if (nodepcre == NULL)
	{
		free(regex);
	}

	return TRUE;
}

pcre *regexp_compile(struct session *ses, char *exp, int comp_option)
{
	const char *error;
	int i;
/*
	if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
	{
		comp_option |= PCRE_UTF8|PCRE_NO_UTF8_CHECK;
	}
*/
	return pcre_compile(exp, comp_option, &error, &i, NULL);
}




/******************************************************************************
* Calls tintin_regexp checking if the string matches, and automatically fills *
* in the text represented by the wildcards on success.                        *
******************************************************************************/

int check_one_regexp(struct session *ses, struct listnode *node, char *line, char *original, int comp_option)
{
	char *exp, *str;

	if (node->regex == NULL)
	{
		char result[BUFFER_SIZE];

		substitute(ses, node->arg1, result, SUB_VAR|SUB_FUN);

		exp = result;
	}
	else
	{
		exp = node->arg1;
	}

	if (*exp == '~')
	{
		exp++;
		str = original;
	}
	else
	{
		str = line;
	}	

	return tintin_regexp(ses, node->regex, str, exp, comp_option, REGEX_FLAG_ARG);
}

/*
	Keep synched with tintin_regexp and tintin_regexp_compile
*/

int get_regex_range(char *in, char *out, int *var, int *arg)
{
	char *pti, *pto, *ptr, range[BUFFER_SIZE];

	pto = out;
	pti = in;
	ptr = range;

	if (in[-2] != '!')
	{
		*pto++ = '(';
	}

	if (*pti < '0' || *pti > '9')
	{
		goto end;
	}

	while (*pti)
	{
		switch (*pti)
		{
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
				*ptr++ = *pti++;
				continue;

			case '.':
				if (pti[1] != '.')
				{
					goto end;
				}
				if (ptr == range)
				{
					*ptr++ = '0';
				}
				*ptr++ = ',';
				pti += 2;
				continue;

			case 'a':
				pto += sprintf(pto, "%s", "[^\\0]");
				break;
			case 'A':
				pto += sprintf(pto, "%s", "\\n");
				break;
			case 'c':
				pto += sprintf(pto, "%s", "(?:\\e\\[[0-9;]*m)");
				break;
			case 'd':
				pto += sprintf(pto, "%s", "[0-9]");
				break;
			case 'D':
				pto += sprintf(pto, "%s", "[^0-9]");
				break;
			case 'p':
				pto += sprintf(pto, "%s", "[\\x20-\\xfe]");
				break;
			case 'P':
				pto += sprintf(pto, "%s", "[^\\x20-\\xfe]");
				break;
			case 's':
				pto += sprintf(pto, "%s", "\\s");
				break;
			case 'S':
				pto += sprintf(pto, "%s", "\\S");
				break;
			case 'u':
				pto += sprintf(pto, "%s", "(?:[\\x00-\\x7F]|[\\xC0-\\xF4][\\x80-\\xC0]{1,3})");
				break;
			case 'U':
				pto += sprintf(pto, "%s", "[\\xF5-\\xFF]");
				break;
			case 'w':
				pto += sprintf(pto, "%s", "\\w");
				break;
			case 'W':
				pto += sprintf(pto, "%s", "\\W");
				break;
			case '*':
				pto += sprintf(pto, "%s", ".");
				break;

			default:
				goto end;
		}
		*ptr = 0;
		pti++;

		pto += sprintf(pto, "{%s}%s%s", range, *pti ? "?" : "", in[-2] != '!' ? ")" : "");

		return pti - in;
	}
	end:

/*	if (var)
	{
		gtd->args[next_arg(*var)] = next_arg(*arg);
	}*/

	pto += sprintf(pto, "%s%s", *in ? ".+?" : ".+", in[-2] != '!' ? ")" : "");

	return 0;
}


// check if a table key is a regex

int tintin_regexp_check(struct session *ses, char *exp)
{
	if (*exp == '^')
	{
		return TRUE;
	}

	while (*exp)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, exp))
		{
			exp += 2;
			continue;
		}

		switch (exp[0])
		{
			case '\\':
			case '{':
				return TRUE;

			case '$':
				if (exp[1] == 0)
				{
					return TRUE;
				}
				break;

			case '%':
				switch (exp[1])
				{
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

					case 'a':
					case 'A':
					case 'c':
					case 'd':
					case 'D':
					case 'i':
					case 'I':
					case 'p':
					case 'P':
					case 's':
					case 'S':
					case 'u':
					case 'U':
					case 'w':
					case 'W':
					case '?':
					case '*':
					case '+':
					case '.':
					case '%':
						return TRUE;

					case '!':
						switch (exp[2])
						{
							case 'a':
							case 'A':
							case 'c':
							case 'd':
							case 'D':
							case 'p':
							case 'P':
							case 's':
							case 'S':
							case 'u':
							case 'U':
							case 'w':
							case 'W':
							case '?':
							case '*':
							case '+':
							case '.':
							case '{':
								return TRUE;
						}
						break;
				}
				break;
		}
		exp++;
	}
	return FALSE;
}

// 1. convert tinexp to pcre
// 2. store tintin %00-99 index in gtd->args that links to pcre index in gtd->vars or gtd->args
// 3. call regexp_compare
//    3.1 compile the pcre if not already compiled
//    3.2 run the pcre and return FALSE if there's not a match
//    3.3 set gtd->vars or gtd->args to the pcre index, unless %00-%99 was found and FIX flag is set
//    3.4 If FIX is set the gtd->args index is used and valid tinexp is assumed
//    3.4 return TRUE

int tintin_regexp(struct session *ses, pcre *nodepcre, char *str, char *exp, int comp_option, int flag)
{
	char out[BUFFER_SIZE], *pti, *pto;
	int i, arg = 1, var = 1, fix = 0;

	pti = exp;
	pto = out;

	while (*pti == '^')
	{
		*pto++ = *pti++;
	}

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;

			switch (*pti)
			{
				case '\\':
				case '[':
				case ']':
				case '(':
				case ')':
				case '|':
				case '.':
				case '?':
				case '+':
				case '*':
				case '$':
				case '^':
					*pto++ = '\\';
					break;
			}
			*pto++ = *pti++;
			continue;
		}

		switch (pti[0])
		{
			case '\\':
				if (pti[1] == 'n')
				{
					SET_BIT(comp_option, PCRE_MULTILINE);
				}
				else if (pti[1] == 0)
				{
					pti++;
					*pto++ = '\\';
					*pto++ = 'z';
					break;
				}
				*pto++ = *pti++;
				*pto++ = *pti++;
				break;

			case '{':
				gtd->args[next_arg(var)] = next_arg(arg);
				*pto++ = '(';
				pti = get_arg_in_braces(ses, pti, pto, GET_ALL);
				pto += strlen(pto);
				*pto++ = ')';
				break;

			case '[':
			case ']':
			case '(':
			case ')':
			case '|':
			case '.':
			case '?':
			case '+':
			case '*':
			case '^':
				*pto++ = '\\';
				*pto++ = *pti++;
				break;

			// variables should already have been substituted, check eol marker.

			case '$':
				for (i = 1 ; pti[i] == '$' ; i++)
				{
					continue;
				}

				if (pti[i] != 0 && pti[i] != '\n')
				{
					*pto++ = '\\';
				}
				*pto++ = *pti++;
				break;

			case '%':
				switch (pti[1])
				{
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
						fix = REGEX_FLAG_FIX;
						arg = is_digit(pti[2]) ? (pti[1] - '0') * 10 + (pti[2] - '0') : pti[1] - '0';
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += is_digit(pti[2]) ? 3 : 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(.*)" : "(.*?)");
						break;

					case 'a':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([^\\0]*)" : "([^\\0]*?)");
						break;

					case 'A':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(\\n*)" : "(\\n*?)");
						break;

					case 'c':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "((?:\\e\\[[0-9;]*m)*)" : "((?:\\e\\[[0-9;]*m)*?)");
						break;

					case 'd':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([0-9]*)" : "([0-9]*?)");
						break;

					case 'D':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([^0-9]*)" : "([^0-9]*?)");
						break;

					case 'i':
						pti += 2;
						pto += sprintf(pto, "%s", "(?i)");
						break;

					case 'I':
						pti += 2;
						pto += sprintf(pto, "%s", "(?-i)");
						break;

					case 'p':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([\\x20-\\xfe]*)" : "([\\x20-\\xfe]*?)");
						break;

					case 'P':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([^\\x20-\\xfe]*)" : "([^\\x20-\\xfe]*?)");
						break;

					case 's':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(\\s*)" : "(\\s*?)");
						break;

					case 'S':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(\\S*)" : "(\\S*?)");
						break;

					case 'u':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "((?:[\\x00-\\x7F]|[\\xC0-\\xF4][\\x80-\\xC0]{1,3})*)" : "((?:[\\x00-\\x7F]|[\\xC0-\\xF4][\\x80-\\xC0]{1,3})*?)");
//						pto += sprintf(pto, "%s", *pti == 0 ? "((?:[\\x00-\\x7F|\\xC0-\\xF4][\\x80-\\xC0]{1,3})*)" : "((?:[\\x00-\\x7F|\\xC0-\\xF4][\\x80-\\xC0]{1,3})*?)");
						break;

					case 'U':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(^[\\xF5-\\xFF]*)" : "([\\xF5-\\xFF]*?)");
						break;


					case 'w':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(\\w*)" : "(\\w*?)");
						break;

					case 'W':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(\\W*)" : "(\\W*?)");
						break;

					case '*':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(.*)" : "(.*?)");
						break;

					case '+':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2 + get_regex_range(&pti[2], pto, &var, &arg);
						pto += strlen(pto);
						break;

					case '%':
						*pto++ = *pti++;
						pti++;
						break;

					case '.':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", "(.)");
						break;

					case '?':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(.?)" : "(.?" "?)");
						break;

					case '!':
						switch (pti[2])
						{
							case 'a':
								gtd->args[next_arg(var)] = next_arg(arg);
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[^\\0]*" : "[^\\0]*?");
								break;

							case 'A':
								gtd->args[next_arg(var)] = next_arg(arg);
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "\\n*" : "\\n*?");
								break;

							case 'c':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "(?:\\e\\[[0-9;]*m)*" : "(?:\\e\\[[0-9;]*m)*?");
								break;

							case 'd':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[0-9]*" : "[0-9]*?");
								break;

							case 'D':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[^0-9]*" : "[^0-9]*?");
								break;

							case 'p':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[\\x20-\\xfe]*" : "[\\x20-\\xfe]*?");
								break;

							case 'P':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[^\\x20-\\xfe]*" : "[^\\x20-\\xfe]*?");
								break;

							case 's':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "\\s*" : "\\s*?");
								break;

							case 'S':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "\\S*" : "\\S*?");
								break;

							case 'u':
								gtd->args[next_arg(var)] = next_arg(arg);
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "(?:[\\x00-\\x7F]|[\\xC0-\\xF4][\\x80-\\xC0]{1,3})*" : "(?:[\\x00-\\x7F]|[\\xC0-\\xF4][\\x80-\\xC0]{1,3})*?");
								break;

							case 'U':
								gtd->args[next_arg(var)] = next_arg(arg);
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[\\xF5-\\xFF]*" : "[\\xF5-\\xFF]*?");
								break;

							case 'w':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "\\w*" : "\\w*?");
								break;

							case 'W':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "\\W*" : "\\W*?");
								break;

							case '?':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? ".?" : ".?" "?");
								break;

							case '*':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? ".*" : ".*?");
								break;

							case '+':
								pti += 3 + get_regex_range(pti + 3, pto, NULL, NULL);
								pto += strlen(pto);
								break;

							case '.':
								pti += 3;
								pto += sprintf(pto, "%s", ".");
								break;

							case '{':
								pti = get_arg_in_braces(ses, pti+2, pto, GET_ALL);
								pto += strlen(pto);
								break;

							default:
								*pto++ = *pti++;
								break;
						}
						break;

					default:
						*pto++ = *pti++;
						break;
				}
				break;

			default:
				*pto++ = *pti++;
				break;
		}
	}
	*pto = 0;

	return regexp_compare(ses, nodepcre, str, out, comp_option, flag + fix);
}

pcre *tintin_regexp_compile(struct session *ses, struct listnode *node, char *exp, int comp_option)
{
	char out[BUFFER_SIZE], *pti, *pto;

	pti = exp;
	pto = out;

	node->flags = 0;

	if (*pti == '~')
	{
		pti++;
	}

	while (*pti == '^')
	{
		*pto++ = *pti++;
	}

	while (*pti)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pti))
		{
			*pto++ = *pti++;

			switch (*pti)
			{
				case '\\':
				case '[':
				case ']':
				case '(':
				case ')':
				case '|':
				case '.':
				case '?':
				case '+':
				case '*':
				case '$':
				case '^':
					*pto++ = '\\';
					break;
			}
			*pto++ = *pti++;
			continue;
		}

		switch (pti[0])
		{
			case '\\':
				if (pti[1] == 'e')
				{
					SET_BIT(node->flags, NODE_FLAG_COLOR);
				}
				else if (pti[1] == 'n')
				{
					SET_BIT(comp_option, PCRE_MULTILINE);
					SET_BIT(node->flags, NODE_FLAG_MULTI);
				}
				else if (pti[1] == 0)
				{
					pti++;
					*pto++ = '\\';
					*pto++ = 'z';
					break;
				}
				*pto++ = *pti++;
				*pto++ = *pti++;
				break;

			case '{':
				*pto++ = '(';
				pti = get_arg_in_braces(ses, pti, pto, GET_ALL);
				while (*pto)
				{
					if (pto[0] == '$' || pto[0] == '@')
					{
						if (pto[1] == DEFAULT_OPEN || is_alnum(pto[1]) || pto[0] == pto[1])
						{
							return NULL;
						}
					}
					if (pto[0] == '\\' && pto[1] == 'n')
					{
						SET_BIT(node->flags, NODE_FLAG_MULTI);
					}
					pto++;
				}
				*pto++ = ')';
				break;

			case '&':
				if (pti[1] == DEFAULT_OPEN)
				{
					return NULL;
				}
				*pto++ = *pti++;
				break;

			case '@':
				if (pti[1] == DEFAULT_OPEN || is_alnum(pti[1]))
				{
					return NULL;
				}
				*pto++ = *pti++;
				break;

			case '$':
				if (pti[1] == DEFAULT_OPEN || is_alnum(pti[1]))
				{
					return NULL;
				}
				{
					int i = 1;
	
					while (pti[i] == '$') i++;

					if (pti[i])
					{
						*pto++ = '\\';
					}
				}
				*pto++ = *pti++;
				break;

			case '[':
			case ']':
			case '(':
			case ')':
			case '|':
			case '.':
			case '?':
			case '+':
			case '*':
			case '^':
				*pto++ = '\\';
				*pto++ = *pti++;
				break;

			case '%':
				switch (pti[1])
				{
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
						pti += is_digit(pti[2]) ? 3 : 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(.*)" : "(.*?)");
						break;

					case 'a':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([^\\0]*)" : "([^\\0]*?)");
						break;

					case 'c':
						SET_BIT(node->flags, NODE_FLAG_COLOR);
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "((?:\\e\\[[0-9;]*m)*)" : "((?:\\e\\[[0-9;]*m)*?)");
						break;

					case 'd':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([0-9]*)" : "([0-9]*?)");
						break;

					case 'D':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([^0-9]*)" : "([^0-9]*?)");
						break;

					case 'i':
						pti += 2;
						pto += sprintf(pto, "%s", "(?i)");
						break;

					case 'I':
						pti += 2;
						pto += sprintf(pto, "%s", "(?-i)");
						break;

					case 'p':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([\\x20-\\xfe]*)" : "([\\x20-\\xfe]*?)");
						break;

					case 'P':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([^\\x20-\\xfe]*)" : "([^\\x20-\\xfe]*?)");
						break;
						
					case 's':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(\\s*)" : "(\\s*?)");
						break;

					case 'S':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(\\S*)" : "(\\S*?)");
						break;

					case 'u':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "((?:[\\x00-\\x7F]|[\\xC0-\\xF4][\\x80-\\xC0]{1,3})*)" : "((?:[\\x00-\\x7F]|[\\xC0-\\xF4][\\x80-\\xC0]{1,3})*?)");
						break;

					case 'U':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "([\\xF5-\\xFF]*)" : "([\\xF5-\\xFF]*?)");
						break;

					case 'w':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(\\w*)" : "(\\w*?)");
						break;

					case 'W':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(\\W*)" : "(\\W*?)");
						break;

					case '?':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(.?)" : "(.?" "?)");
						break;

					case '*':
						pti += 2;
						pto += sprintf(pto, "%s", *pti == 0 ? "(.*)" : "(.*?)");
						break;

					case '+':
						pti += 2 + get_regex_range(&pti[2], pto, NULL, NULL);
						pto += strlen(pto);
						break;

					case '.':
						pti += 2;
						pto += sprintf(pto, "%s", "(.)");
						break;

					case '%':
						*pto++ = *pti++;
						pti++;
						break;

					case '!':
						switch (pti[2])
						{
							case 'a':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[^\\0]*" : "[^\\0]*?");
								break;

							case 'c':
								SET_BIT(node->flags, NODE_FLAG_COLOR);
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "(?:\\e\\[[0-9;]*m)*" : "(?:\\e\\[[0-9;]*m)*?");
								break;

							case 'd':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[0-9]*" : "[0-9]*?");
								break;

							case 'D':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[^0-9]*" : "[^0-9]*?");
								break;

							case 'p':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[\\x21-\\x7E]*" : "[\\x21-\\x7E]?*");
								break;

							case 'P':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "[^\\x20-\\xfe]*" : "[^\\x20-\\xfe]*?");
								break;

							case 's':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "\\s*" : "\\s*?");
								break;

							case 'S':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "\\S*" : "\\S*?");
								break;

							case 'w':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "\\w*" : "\\w*?");
								break;

							case 'W':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? "\\W*" : "\\W*?");
								break;

							case '?':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? ".?" : ".?" "?");
								break;

							case '*':
								pti += 3;
								pto += sprintf(pto, "%s", *pti == 0 ? ".*" : ".*?");
								break;

							case '+':
								pti += 3 + get_regex_range(&pti[3], pto, NULL, NULL);
								pto += strlen(pto);
								break;

							case '.':
								pti += 3;
								pto += sprintf(pto, "%s", ".");
								break;

							case '{':
								pti = get_arg_in_braces(ses, pti+2, pto, GET_ALL);

								while (*pto)
								{
									if (pto[0] == '$' || pto[0] == '@')
									{
										if (pto[1] == DEFAULT_OPEN || is_alnum(pto[1]) || pto[0] == pto[1])
										{
											return NULL;
										}
									}
									if (pto[0] == '\\' && pto[1] == 'n')
									{
										SET_BIT(node->flags, NODE_FLAG_MULTI);
									}
									pto++;
								}
								break;

							default:
								*pto++ = *pti++;
								break;
						}
						break;

					default:
						*pto++ = *pti++;
						break;
				}
				break;

			default:
				*pto++ = *pti++;
				break;
		}
	}
	*pto = 0;

	if (HAS_BIT(node->flags, NODE_FLAG_COLOR) && *exp != '~')
	{
		show_error(ses, LIST_COMMAND, "#WARNING: REGEX {%s} MATCHES ESCAPE CODES BUT DOES NOT START WITH A '~'.", exp);
	}
	return regexp_compile(ses, out, comp_option);
}

void tintin_macro_compile(char *input, char *output)
{
	char *pti, *pto;

	pti = input;
	pto = output;

	if (*pti == '^')
	{
		pti++;
	}

	while (*pti)
	{
		switch (pti[0])
		{
			case '\\':
				switch (pti[1])
				{
					case 'C':
						if (pti[2] == '-' && pti[3])
						{
							*pto++  = pti[3] - 'a' + 1;
							pti    += 4;
						}
						else
						{
							*pto++ = *pti++;
						}
						break;

					case 'c':
						*pto++ = pti[2] % 32;
						pti += 3;
						break;

					case 'a':
						*pto++  = ASCII_BEL;
						pti += 2;
						break;

					case 'b':
						*pto++  = 127;
						pti    += 2;
						break;

					case 'e':
						*pto++  = ASCII_ESC;
						pti    += 2;
						break;

					case 'n':
						*pto++ = ASCII_LF;
						pti  += 2;
						break;

					case 'r':
						*pto++ = ASCII_CR;
						pti   += 2;
						break;

					case 't':
						*pto++  = ASCII_HTAB;
						pti    += 2;
						break;

					case 'x':
						if (pti[2] && pti[3])
						{
							*pto++ = hex_number_8bit(&pti[2]);
							pti += 4;
						}
						else
						{
							*pto++ = *pti++;
						}
						break;

					case 'u':
						if (pti[2] && pti[3] && pti[4] && pti[5])
						{
							pto += unicode_16_bit(&pti[2], pto);
							pti += 6;
						}
						else
						{
							*pto++ = *pti++;
						}
						break;

					case 'U':
						if (pti[2] && pti[3] && pti[4] && pti[5] && pti[6] && pti[7])
						{
							pto += unicode_21_bit(pti + 2, pto);
							pti += 8;
						}
						else
						{
							*pto++ = *pti++;
						}
						break;

					case 'v':
						*pto++ = ASCII_VTAB;
						pti   += 2;
						break;

					default:
						*pto++ = *pti++;
						break;
				}
				break;

			default:
				*pto++ = *pti++;
				break;
		}
	}
	*pto = 0;
}
