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

#include "tintin.h"


int match(struct session *ses, char *str, char *exp, int sub)
{
/*
	sprintf(expbuf, "\\A%s\\Z", exp);

	substitute(ses, expbuf, expbuf, sub);

	return tintin_regex_compare(ses, NULL, str, expbuf, 0, 0);
*/
	if (sub)
	{
		char expbuf[BUFFER_SIZE];

		substitute(ses, exp, expbuf, sub);

		return tintin_regex_compare(ses, NULL, str, expbuf, PCRE2_ANCHORED|PCRE2_ENDANCHORED, 0);
	}

	return tintin_regex_compare(ses, NULL, str, exp, PCRE2_ANCHORED|PCRE2_ENDANCHORED, 0);
}

int find(struct session *ses, char *str, char *exp, int sub, int flag)
{
	if (HAS_BIT(sub, SUB_VAR|SUB_FUN))
	{
		char expbuf[BUFFER_SIZE], strbuf[BUFFER_SIZE];

		substitute(ses, str, strbuf, SUB_VAR|SUB_FUN);
		substitute(ses, exp, expbuf, SUB_VAR|SUB_FUN);

		if (flag)
		{
			return tintin_regex_match(ses, NULL, strbuf, expbuf, 0, flag);
		}
		return tintin_regex_compare(ses, NULL, strbuf, expbuf, 0, 0);
	}

	if (flag)
	{
		return tintin_regex_match(ses, NULL, str, exp, 0, flag);
	}
	return tintin_regex_compare(ses, NULL, str, exp, 0, 0);
}

// This code is never called as it's handled in tokenize.c

DO_COMMAND(do_regexp)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg3, GET_ALL);

	if (*arg3 == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #REGEX <TEXT> <EXPRESSION> <TRUE> [FALSE]");
	}
	else
	{
		if (tintin_regex_match(ses, NULL, arg1, arg2, 0, REGEX_FLAG_CMD))
		{
			substitute(ses, arg3, arg3, SUB_CMD);

			ses = script_driver(ses, LIST_COMMAND, NULL, arg3);
		}
		else
		{
			arg4 = str_alloc_stack(0);

			arg = get_arg_in_braces(ses, arg, arg4, GET_ALL);

			if (*arg4)
			{
				ses = script_driver(ses, LIST_COMMAND, NULL, arg4);
			}
		}
	}
	return ses;
}

int tintin_regex_compare(struct session *ses, pcre2_code *nodepcre, char *str, char *exp, int comp_option, int flag)
{
	pcre2_code *regex;
	int matches;

	if (nodepcre == NULL)
	{
		regex = tintin_regex_compile(ses, NULL, exp, comp_option);
	}
	else
	{
		regex = nodepcre;
	}

	if (regex == NULL)
	{
		return FALSE;
	}

	matches = pcre2_match(regex, (PCRE2_SPTR) str, PCRE2_ZERO_TERMINATED, 0, 0, gtd->match_data, gtd->match_context);

	if (nodepcre == NULL)
	{
		pcre2_code_free(regex);
	}

	return matches > 0;
}


int tintin_regex_match(struct session *ses, pcre2_code *nodepcre, char *str, char *exp, int comp_option, int flag)
{
	pcre2_code *regex;
	int i, j, matches;

	if (nodepcre == NULL)
	{
		regex = tintin_regex_compile(ses, NULL, exp, comp_option);
	}
	else
	{
		regex = nodepcre;
	}

	if (regex == NULL)
	{
		return FALSE;
	}

	matches = pcre2_match(regex, (PCRE2_SPTR) str, PCRE2_ZERO_TERMINATED, 0, 0, gtd->match_data, gtd->match_context);

	if (nodepcre == NULL)
	{
		pcre2_code_free(regex);
	}

	if (matches <= 0)
	{
		return FALSE;
	}

	flag += tintin_match_data(ses, exp);

	PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(gtd->match_data);

	memcpy(gtd->match, ovector, matches * 2 * sizeof(PCRE2_SIZE));

	// REGEX_FLAG_FIX handles %1 to %99 usage. Backward compatible.

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

	return TRUE;
}


// Used by triggers

int check_one_regex(struct session *ses, struct listnode *node, char *line, char *original, int comp_option, int flag)
{
	char *exp, *str, result[BUFFER_SIZE];

	if (node->regex == NULL)
	{
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

	if (flag)
	{
		return tintin_regex_match(ses, node->regex, str, exp, comp_option, flag);
	}

	return tintin_regex_compare(ses, node->regex, str, exp, comp_option, 0);
}


// check if a table key is a regex

int tintin_regex_check(struct session *ses, char *exp)
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

// keep synched with tintin_regex_compile

int tintin_match_data(struct session *ses, char *exp)
{
	char tmp[BUFFER_SIZE], *pte;
	int arg = 1, var = 1, flag = 0;

	pte = exp;

	while (*pte == '^')
	{
		pte++;
	}

	while (*pte)
	{
		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, pte))
		{
			pte += 2;
			continue;
		}

		switch (pte[0])
		{
			case '\\':
				if (pte[1] == 0)
				{
					pte++;
					break;
				}
				pte += 2;
				break;

			case '{':
				gtd->args[next_arg(var)] = next_arg(arg);
				pte = get_arg_in_braces(ses, pte, tmp, GET_ALL);
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
				pte++;
				break;

			// variables should already have been substituted

			case '$':
				pte++;
				break;

			case '%':
				switch (pte[1])
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
						flag = REGEX_FLAG_FIX;
						arg = is_digit(pte[2]) ? (pte[1] - '0') * 10 + (pte[2] - '0') : pte[1] - '0';
						gtd->args[next_arg(var)] = next_arg(arg);
						pte += is_digit(pte[2]) ? 3 : 2;
						break;

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
					case '*':
					case '+':
					case '.':
					case '?':
						gtd->args[next_arg(var)] = next_arg(arg);
						pte += 2;
						break;

					case 'i':
					case 'I':
					case '%':
						pte += 2;
						break;

					case '!':
						switch (pte[2])
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
								pte += 3;
								break;

							case '{':
								pte = get_arg_in_braces(ses, pte+2, tmp, GET_ALL);
								break;

							default:
								pte++;
								break;
						}
						break;

					default:
						pte++;
						break;
				}
				break;

			default:
				pte++;
				break;
		}
	}
	return flag;
}


// Keep synched with tintin_regex_compile

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


pcre2_code *tintin_regex_compile(struct session *ses, struct listnode *node, char *exp, int comp_option)
{
	char out[BUFFER_SIZE], *pti, *pto;
	pcre2_code *regex;
	PCRE2_SIZE erroroffset;
	int i, errorcode;

	pti = exp;
	pto = out;

	if (node)
	{
		node->flags = 0;
	}

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
				if (pti[1] == 'e' && node)
				{
					SET_BIT(node->flags, NODE_FLAG_COLOR);
				}
				else if (pti[1] == 'n')
				{
					if (node)
					{
						SET_BIT(node->flags, NODE_FLAG_MULTI);
					}
					SET_BIT(comp_option, PCRE2_MULTILINE);
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
					if (node && (pto[0] == '$' || pto[0] == '@'))
					{
						if (pto[1] == DEFAULT_OPEN || is_alnum(pto[1]) || pto[0] == pto[1])
						{
							return NULL;
						}
					}
					if (pto[0] == '\\' && pto[1] == 'n')
					{
						if (node)
						{
							SET_BIT(node->flags, NODE_FLAG_MULTI);
						}
						SET_BIT(comp_option, PCRE2_MULTILINE);
					}
					pto++;
				}
				*pto++ = ')';
				break;

			case '&':
				if (node && pti[1] == DEFAULT_OPEN)
				{
					return NULL;
				}
				*pto++ = *pti++;
				break;

			case '@':
				if (node && (pti[1] == DEFAULT_OPEN || is_alnum(pti[1])))
				{
					return NULL;
				}
				*pto++ = *pti++;
				break;

			case '$':
				if (node && (pti[1] == DEFAULT_OPEN || is_alnum(pti[1])))
				{
					return NULL;
				}
				for (i = 1 ; pti[i] == '$' ; i++) continue;

				if (pti[i])
				{
					*pto++ = '\\';
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
						if (node)
						{
							SET_BIT(node->flags, NODE_FLAG_COLOR);
						}
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
								if (node)
								{
									SET_BIT(node->flags, NODE_FLAG_COLOR);
								}
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
									if (node && (pto[0] == '$' || pto[0] == '@'))
									{
										if (pto[1] == DEFAULT_OPEN || is_alnum(pto[1]) || pto[0] == pto[1])
										{
											return NULL;
										}
									}
									if (pto[0] == '\\' && pto[1] == 'n')
									{
										if (node)
										{
											SET_BIT(node->flags, NODE_FLAG_MULTI);
										}
										SET_BIT(comp_option, PCRE2_MULTILINE);
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

	if (node && HAS_BIT(node->flags, NODE_FLAG_COLOR) && *exp != '~')
	{
		show_error(ses, LIST_COMMAND, "#WARNING: REGEX {%s} MATCHES ESCAPE CODES BUT DOES NOT START WITH A '~'.", exp);
	}

//	if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
//	{
//		comp_option |= PCRE2_UTF|PCRE2_NO_UTF_CHECK|PCRE2_UCP;
//	}

	regex = pcre2_compile((PCRE2_SPTR) out, PCRE2_ZERO_TERMINATED, comp_option, &errorcode, &erroroffset, NULL);

	if (node && regex)
	{
		pcre2_jit_compile(regex, PCRE2_JIT_COMPLETE);
	}
	return regex;
}

void tintin_regex_free(struct listnode *node)
{
	if (node->regex)
	{
		pcre2_code_free(node->regex);
		node->regex = NULL;
	}
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
