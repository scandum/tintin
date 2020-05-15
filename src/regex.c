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
	arg = get_arg_in_braces(ses, arg, arg4, GET_ALL);

	if (*arg3 == 0)
	{
		show_error(ses, LIST_COMMAND, "SYNTAX: #REGEXP {string} {expression} {true} {false}.");
	}
	else
	{
		if (tintin_regexp(ses, NULL, arg1, arg2, 0, REGEX_FLAG_CMD))
		{
			substitute(ses, arg3, arg3, SUB_CMD);

			ses = script_driver(ses, LIST_COMMAND, arg3);
		}
		else if (*arg4)
		{
			ses = script_driver(ses, LIST_COMMAND, arg4);
		}
	}
	return ses;
}

int regexp_compare(struct session *ses, pcre *nodepcre, char *str, char *exp, int option, int flag)
{
	pcre *regex;
	int i, j, matches, match[303];

	if (nodepcre == NULL)
	{
		regex = regexp_compile(ses, exp, option);
	}
	else
	{
		regex = nodepcre;
	}

	if (regex == NULL)
	{
		return FALSE;
	}

	matches = pcre_exec(regex, NULL, str, strlen(str), 0, 0, match, 303);

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
			for (i = 0 ; i < matches ; i++)
			{
				gtd->cmds[i] = restringf(gtd->cmds[i], "%.*s", match[i*2+1] - match[i*2], &str[match[i*2]]);
			}
			break;

		case REGEX_FLAG_CMD + REGEX_FLAG_FIX:
			for (i = 0 ; i < matches ; i++)
			{
				j = gtd->args[i];

				gtd->cmds[j] = restringf(gtd->cmds[j], "%.*s", match[i*2+1] - match[i*2], &str[match[i*2]]);
			}
			break;

		case REGEX_FLAG_ARG:
			for (i = 0 ; i < matches ; i++)
			{
				gtd->vars[i] = restringf(gtd->vars[i], "%.*s", match[i*2+1] - match[i*2], &str[match[i*2]]);
			}
			break;

		case REGEX_FLAG_ARG + REGEX_FLAG_FIX:
			for (i = 0 ; i < matches ; i++)
			{
				j = gtd->args[i];

				gtd->vars[j] = restringf(gtd->vars[j], "%.*s", match[i*2+1] - match[i*2], &str[match[i*2]]);
			}
			break;
	}

	if (nodepcre == NULL)
	{
		free(regex);
	}

	return TRUE;
}

pcre *regexp_compile(struct session *ses, char *exp, int option)
{
	const char *error;
	int i;
/*
	if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
	{
		option |= PCRE_UTF8|PCRE_NO_UTF8_CHECK;
	}
*/
	return pcre_compile(exp, option, &error, &i, NULL);
}




/******************************************************************************
* Calls tintin_regexp checking if the string matches, and automatically fills *
* in the text represented by the wildcards on success.                        *
******************************************************************************/

int check_one_regexp(struct session *ses, struct listnode *node, char *line, char *original, int option)
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

	if (*node->arg1 == '~')
	{
		exp++;
		str = original;
	}
	else
	{
		str = line;
	}	

	return tintin_regexp(ses, node->regex, str, exp, option, REGEX_FLAG_ARG);
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
				pto += sprintf(pto, "(.");
				break;
			case 'A':
				pto += sprintf(pto, "(\\n");
				break;
			case 'd':
				pto += sprintf(pto, "([0-9]");
				break;
			case 'D':
				pto += sprintf(pto, "([^0-9]");
				break;
			case 'p':
				pto += sprintf(pto, "([\\x20-\\xfe]");
				break;
			case 'P':
				pto += sprintf(pto, "([^\\x20-\\xfe]");
				break;
			case 's':
				pto += sprintf(pto, "(\\s");
				break;
			case 'S':
				pto += sprintf(pto, "(\\S");
				break;
			case 'u':
				pto += sprintf(pto, "((?:[\\xC0-\\xFE][\\x80-\\xC0]{1,3})");
				break;
			case 'U':
				pto += sprintf(pto, "([\\x00-\\x7F\\xFF]");
				break;
			case 'w':
				pto += sprintf(pto, "([a-zA-Z]");
				break;
			case 'W':
				pto += sprintf(pto, "([^a-zA-Z]");
				break;

			default:
				goto end;
		}
		*ptr = 0;
		pti++;

		pto += sprintf(pto, "{%s}%s", range, *pti ? "?)" : ")");

		return pti - in;
	}
	end:

	if (var)
	{
		gtd->args[next_arg(*var)] = next_arg(*arg);
	}
	strcpy(out, *pti ? "(.+?)" : "(.+)");

	return 0;
}


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

int tintin_regexp(struct session *ses, pcre *nodepcre, char *str, char *exp, int option, int flag)
{
	char out[BUFFER_SIZE], *pti, *pto;
	int arg = 1, var = 1, fix = 0;

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

			case '$':
				if (pti[1] != DEFAULT_OPEN && !isalnum((int) pti[1]))
				{
					int i = 0;

					while (pti[++i] == '$')
					{
						continue;
					}

					if (pti[i])
					{
						*pto++ = '\\';
					}
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
						arg = isdigit((int) pti[2]) ? (pti[1] - '0') * 10 + (pti[2] - '0') : pti[1] - '0';
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += isdigit((int) pti[2]) ? 3 : 2;
						strcpy(pto, *pti == 0 ? "(.*)" : "(.*?)");
						pto += strlen(pto);
						break;

					case 'a':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "([^\\n]*)" : "([^\\n]*?)");
						pto += strlen(pto);
						break;

					case 'A':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "(\\n*)" : "(\\n*?)");
						pto += strlen(pto);
						break;

					case 'd':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "([0-9]*)" : "([0-9]*?)");
						pto += strlen(pto);
						break;

					case 'D':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "([^0-9]*)" : "([^0-9]*?)");
						pto += strlen(pto);
						break;

					case 'i':
						pti += 2;
						strcpy(pto, "(?i)");
						pto += strlen(pto);
						break;

					case 'I':
						pti += 2;
						strcpy(pto, "(?-i)");
						pto += strlen(pto);
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
						strcpy(pto, *pti == 0 ? "(\\s*)" : "(\\s*?)");
						pto += strlen(pto);
						break;

					case 'S':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "(\\S*)" : "(\\S*?)");
						pto += strlen(pto);
						break;

					case 'u':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "((?:[\\x00-\\x7F|\\xC0-\\xFE][\\x80-\\xC0]{1,3})*)" : "((?:[\\xC0-\\xFE][\\x80-\\xC0]{1,3})*?)");
						pto += strlen(pto);
						break;

					case 'U':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "(^[\xFF]*)" : "([\\x00-\\x7F\\xFF]*?)");
						pto += strlen(pto);
						break;

					case 'w':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "([a-zA-Z]*)" : "([a-zA-Z]*?)");
						pto += strlen(pto);
						break;

					case 'W':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "([^a-zA-Z]*)" : "([^a-zA-Z]*?)");
						pto += strlen(pto);
						break;

					case '*':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "(.*)" : "(.*?)");
						pto += strlen(pto);
						break;

					case '+':
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
						strcpy(pto, "(.)");
						pto += strlen(pto);
						break;

					case '?':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "(.?)" : "(.?" "?)");
						pto += strlen(pto);
						break;

					case '!':
						switch (pti[2])
						{
							case 'a':
								gtd->args[next_arg(var)] = next_arg(arg);
								pti += 2;
								strcpy(pto, *pti == 0 ? "[^\\n]*" : "[^\\n]*?");
								pto += strlen(pto);
								break;

							case 'A':
								gtd->args[next_arg(var)] = next_arg(arg);
								pti += 2;
								strcpy(pto, *pti == 0 ? "\\n*" : "\\n*?");
								pto += strlen(pto);
								break;

							case 'd':
								pti += 3;
								strcpy(pto, *pti == 0 ? "[0-9]*" : "[0-9]*?");
								pto += strlen(pto);
								break;

							case 'D':
								pti += 3;
								strcpy(pto, *pti == 0 ? "[^0-9]*" : "[^0-9]*?");
									pto += strlen(pto);
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
								strcpy(pto, *pti == 0 ? "\\s*" : "\\s*?");
								pto += strlen(pto);
								break;

							case 'S':
								pti += 3;
								strcpy(pto, *pti == 0 ? "\\S*" : "\\S*?");
								pto += strlen(pto);
								break;

							case 'u':
								gtd->args[next_arg(var)] = next_arg(arg);
								pti += 3;
								strcpy(pto, *pti == 0 ? "(?:[\\xC0-\\xFE][\\x80-\\xC0]{1,3})*" : "(?:[\\xC0-\\xFE][\\x80-\\xC0]{1,3})*?");
								pto += strlen(pto);
								break;

							case 'U':
								gtd->args[next_arg(var)] = next_arg(arg);
								pti += 3;
								strcpy(pto, *pti == 0 ? "[\\x00-\\x7F\\xFF]*" : "[\\x00-\\x7F\\xFF]*?");
								pto += strlen(pto);
								break;

							case 'w':
								pti += 3;
								strcpy(pto, *pti == 0 ? "[a-zA-Z]*" : "[a-zA-Z]*?");
								pto += strlen(pto);
								break;

							case 'W':
								pti += 3;
								strcpy(pto, *pti == 0 ? "[^a-zA-Z]*" : "[^a-zA-Z]*?");
								pto += strlen(pto);
								break;

							case '?':
								pti += 3;
								strcpy(pto, *pti == 0 ? ".?" : ".?" "?");
								pto += strlen(pto);
								break;

							case '*':
								pti += 3;
								strcpy(pto, *pti == 0 ? ".*" : ".*?");
								pto += strlen(pto);
								break;

							case '+':
								pti += 3 + get_regex_range(&pti[3], pto, NULL, NULL);
								pto += strlen(pto);
								break;

							case '.':
								pti += 3;
								strcpy(pto, ".");
								pto += strlen(pto);
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

	return regexp_compare(ses, nodepcre, str, out, option, flag + fix);
}

pcre *tintin_regexp_compile(struct session *ses, struct listnode *node, char *exp, int option)
{
	char out[BUFFER_SIZE], *pti, *pto;

	pti = exp;
	pto = out;

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
						if (pto[1])
						{
							return NULL;
						}
					}
					pto++;
				}
				*pto++ = ')';
				break;

			case '&':
				if (pti[1] == DEFAULT_OPEN || isalnum((int) pti[1]) || pti[1] == '&')
				{
					return NULL;
				}
				*pto++ = *pti++;
				break;

			case '@':
				if (pti[1] == DEFAULT_OPEN || isalnum((int) pti[1]) || pti[1] == '@')
				{
					return NULL;
				}
				*pto++ = *pti++;
				break;

			case '$':
				if (pti[1] == DEFAULT_OPEN || isalnum((int) pti[1]))
				{
					return NULL;
				}
				{
					int i = 0;

					while (pti[++i] == '$')
					{
						continue;
					}

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
						pti += isdigit((int) pti[2]) ? 3 : 2;
						strcpy(pto, *pti == 0 ? "(.*)" : "(.*?)");
						pto += strlen(pto);
						break;

					case 'd':
						pti += 2;
						strcpy(pto, *pti == 0 ? "([0-9]*)" : "([0-9]*?)");
						pto += strlen(pto);
						break;

					case 'D':
						pti += 2;
						strcpy(pto, *pti == 0 ? "([^0-9]*)" : "([^0-9]*?)");
						pto += strlen(pto);
						break;

					case 'i':
						pti += 2;
						strcpy(pto, "(?i)");
						pto += strlen(pto);
						break;

					case 'I':
						pti += 2;
						strcpy(pto, "(?-i)");
						pto += strlen(pto);
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
						strcpy(pto, *pti == 0 ? "(\\s*)" : "(\\s*?)");
						pto += strlen(pto);
						break;

					case 'S':
						pti += 2;
						strcpy(pto, *pti == 0 ? "(\\S*)" : "(\\S*?)");
						pto += strlen(pto);
						break;

					case 'u':
						pti += 2;
						strcpy(pto, *pti == 0 ? "((?:[\\xC0-\\xFE][\\x80-\\xC0]{1,3})*)" : "((?:[\\xC0-\\xFE][\\x80-\\xC0]{1,3})*?)");
						pto += strlen(pto);
						break;

					case 'U':
						pti += 2;
						strcpy(pto, *pti == 0 ? "([\\x00-\\x7F\\xFF]*)" : "([\\x00-\\x7F\\xFF]*?)");
						pto += strlen(pto);
						break;

					case 'w':
						pti += 2;
						strcpy(pto, *pti == 0 ? "([a-zA-Z]*)" : "([a-zA-Z]*?)");
						pto += strlen(pto);
						break;

					case 'W':
						pti += 2;
						strcpy(pto, *pti == 0 ? "([^a-zA-Z]*)" : "([^a-zA-Z]*?)");
						pto += strlen(pto);
						break;

					case '?':
						pti += 2;
						strcpy(pto, *pti == 0 ? "(.?)" : "(.?" "?)");
						pto += strlen(pto);
						break;

					case '*':
						pti += 2;
						strcpy(pto, *pti == 0 ? "(.*)" : "(.*?)");
						pto += strlen(pto);
						break;

					case '+':
						pti += 2 + get_regex_range(&pti[2], pto, NULL, NULL);
						pto += strlen(pto);
						break;

					case '.':
						pti += 2;
						strcpy(pto, "(.)");
						pto += strlen(pto);
						break;

					case '%':
						*pto++ = *pti++;
						pti++;
						break;

					case '!':
						switch (pti[2])
						{
							case 'd':
								pti += 3;
								strcpy(pto, *pti == 0 ? "[0-9]*" : "[0-9]*?");
								pto += strlen(pto);
								break;

							case 'D':
								pti += 3;
								strcpy(pto, *pti == 0 ? "[^0-9]*" : "[^0-9]*?");
								pto += strlen(pto);
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
								strcpy(pto, *pti == 0 ? "\\s*" : "\\s*?");
								pto += strlen(pto);
								break;

							case 'S':
								pti += 3;
								strcpy(pto, *pti == 0 ? "\\S*" : "\\S*?");
								pto += strlen(pto);
								break;

							case 'w':
								pti += 3;
								strcpy(pto, *pti == 0 ? "[a-zA-Z]*" : "[a-zA-Z]*?");
								pto += strlen(pto);
								break;

							case 'W':
								pti += 3;
								strcpy(pto, *pti == 0 ? "[^a-zA-Z]*" : "[^a-zA-Z]*?");
								pto += strlen(pto);
								break;

							case '?':
								pti += 3;
								strcpy(pto, *pti == 0 ? ".?" : ".?" "?");
								pto += strlen(pto);
								break;

							case '*':
								pti += 3;
								strcpy(pto, *pti == 0 ? ".*" : ".*?");
								pto += strlen(pto);
								break;

							case '+':
								pti += 3 + get_regex_range(&pti[3], pto, NULL, NULL);
								pto += strlen(pto);
								break;

							case '.':
								pti += 3;
								strcpy(pto, ".");
								pto += strlen(pto);
								break;

							case '{':
								pti = get_arg_in_braces(ses, pti+2, pto, GET_ALL);

								while (*pto)
								{
									if (pto[0] == '$' || pto[0] == '@')
									{
										if (pto[1])
										{
											return NULL;
										}
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

	return regexp_compile(ses, out, option);
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
						*pti += 2;
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
							pto += unicode_21_bit(&pti[2], pto);
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
