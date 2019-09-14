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

#include <sys/types.h>
#include <pcre.h>

#include "tintin.h"


int match(struct session *ses, char *str, char *exp, int flags)
{
	char expbuf[BUFFER_SIZE];

	sprintf(expbuf, "\\A%s\\Z", exp);

	substitute(ses, expbuf, expbuf, flags);

	return tintin_regexp(ses, NULL, str, expbuf, 0, 0);
}

int find(struct session *ses, char *str, char *exp, int sub)
{
	char expbuf[BUFFER_SIZE], strbuf[BUFFER_SIZE];

	if (ses)
	{
		substitute(ses, str, strbuf, SUB_VAR|SUB_FUN);
		substitute(ses, exp, expbuf, SUB_VAR|SUB_FUN);

		return tintin_regexp(ses, NULL, strbuf, expbuf, 0, sub);
	}
	else
	{
		return tintin_regexp(ses, NULL, str, exp, 0, sub);
	}
}

DO_COMMAND(do_regexp)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], is_t[BUFFER_SIZE], is_f[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, is_t, GET_ALL);
	arg = get_arg_in_braces(ses, arg, is_f, GET_ALL);

	if (*is_t == 0)
	{
		show_error(ses, LIST_COMMAND, "SYNTAX: #REGEXP {string} {expression} {true} {false}.");
	}
	else
	{
		if (tintin_regexp(ses, NULL, arg1, arg2, 0, SUB_CMD))
		{
			substitute(ses, is_t, is_t, SUB_CMD);

			ses = script_driver(ses, LIST_COMMAND, is_t);
		}
		else if (*is_f)
		{
			ses = script_driver(ses, LIST_COMMAND, is_f);
		}
	}
	return ses;
}

int regexp_compare(pcre *nodepcre, char *str, char *exp, int option, int flag)
{
	pcre *regex;
	const char *error;
	int i, j, matches, match[303];

	if (nodepcre == NULL)
	{
		regex = pcre_compile(exp, option, &error, &i, NULL);
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

	// SUB_FIX handles %1 to %99 usage. Backward compatibility.

	switch (flag)
	{
		case SUB_CMD:
			for (i = 0 ; i < matches ; i++)
			{
				gtd->cmds[i] = restringf(gtd->cmds[i], "%.*s", match[i*2+1] - match[i*2], &str[match[i*2]]);
			}
			break;

		case SUB_CMD + SUB_FIX:
			for (i = 0 ; i < matches ; i++)
			{
				j = gtd->args[i];

				gtd->cmds[j] = restringf(gtd->cmds[j], "%.*s", match[i*2+1] - match[i*2], &str[match[i*2]]);
			}
			break;

		case SUB_ARG:
			for (i = 0 ; i < matches ; i++)
			{
				gtd->vars[i] = restringf(gtd->vars[i], "%.*s", match[i*2+1] - match[i*2], &str[match[i*2]]);
			}
			break;

		case SUB_ARG + SUB_FIX:
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

pcre *regexp_compile(char *exp, int option)
{
	const char *error;
	int i;

	return pcre_compile(exp, option, &error, &i, NULL);
}


int is_variable(struct session *ses, char *str)
{
	struct listroot *root;
	char temp[BUFFER_SIZE], *ptt;
	int i = 1;

	while (str[i] == str[0])
	{
		i++;
	}

	if (str[i] == DEFAULT_OPEN)
	{
		return TRUE;
	}

	if (str[i] != '_' && isalpha((int) str[i]) == 0)
	{
		return FALSE;
	}

	ptt = temp;

	while (isalnum((int) str[i]) || str[i] == '_')
	{
		*ptt++ = str[i];

		i++;
	}
	*ptt = 0;

	root = local_list(ses);

	if (search_node_list(root, temp) == NULL)
	{
		root = ses->list[LIST_VARIABLE];

		if (search_node_list(root, temp) == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}

int is_function(struct session *ses, char *str)
{
	char temp[BUFFER_SIZE], *ptt;
	int i = 1;

	while (str[i] == str[0])
	{
		i++;
	}

	if (str[i] != '_' && isalpha((int) str[i]) == 0)
	{
		return FALSE;
	}

	ptt = temp;

	while (isalnum((int) str[i]) || str[i] == '_')
	{
		*ptt++ = str[i];

		i++;
	}
	*ptt = 0;

	if (str[i] != DEFAULT_OPEN)
	{
		return FALSE;
	}

	if (search_node_list(ses->list[LIST_FUNCTION], temp) == NULL)
	{
		if (find_session(temp) == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}

int substitute(struct session *ses, char *string, char *result, int flags)
{
	struct listnode *node;
	struct listroot *root;
	struct session *sesptr;
	char temp[BUFFER_SIZE], buf[BUFFER_SIZE], buffer[BUFFER_SIZE], *pti, *pto, *ptt, *str;
	char *pte, old[10] = { 0 };
	int i, cnt, escape = FALSE, flags_neol = flags;

	push_call("substitute(%p,%p,%p,%d)",ses,string,result,flags);

	pti = string;
	pto = (string == result) ? buffer : result;

	DEL_BIT(flags_neol, SUB_EOL|SUB_LNF);

	while (TRUE)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		switch (*pti)
		{
			case '\0':
				if (HAS_BIT(flags, SUB_EOL))
				{
					if (HAS_BIT(ses->flags, SES_FLAG_RUN))
					{
						*pto++ = '\r';
					}
					else
					{
						*pto++ = '\r';
						*pto++ = '\n';
					}
				}

				if (HAS_BIT(flags, SUB_LNF))
				{
					*pto++ = '\n';
				}

				*pto = 0;

				if (string == result)
				{
					strcpy(result, buffer);

					pop_call();
					return pto - buffer;
				}
				else
				{
					pop_call();
					return pto - result;
				}
				break;

			case '@':
				if (HAS_BIT(flags, SUB_FUN) && !HAS_BIT(ses->list[LIST_FUNCTION]->flags, LIST_FLAG_IGNORE))
				{
					i = 1;
					escape = FALSE;
					sesptr = NULL;

					while (pti[i] == '@')
					{
						escape = TRUE;

						i++;
					}

					for (ptt = temp ; isalnum((int) pti[i]) || pti[i] == '_' ; i++)
					{
						*ptt++ = pti[i];
					}
					*ptt = 0;

					if (pti[i] != DEFAULT_OPEN)
					{
						while (*pti == '@')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					node = search_node_list(ses->list[LIST_FUNCTION], temp);

					if (node == NULL)
					{
						sesptr = find_session(temp);
					}

					if (sesptr == NULL && node == NULL)
					{
						while (*pti == '@')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (escape)
					{
						pti++;

						while (*pti == '@')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					pti = get_arg_in_braces(ses, &pti[i], temp, GET_ONE);

					if (sesptr)
					{
						substitute(sesptr, temp, pto, flags_neol);

						pto += strlen(pto);
						
						continue;
					}
					else
					{
						substitute(ses, temp, buf, flags_neol);
					}

					show_debug(ses, LIST_FUNCTION, "#DEBUG FUNCTION {%s}", node->arg1);

					RESTRING(gtd->vars[0], buf);

					pte = buf;

					for (i = 1 ; i < 100 ; i++)
					{
						pte = get_arg_in_braces(ses, pte, temp, GET_ALL);

						RESTRING(gtd->vars[i], temp);

						if (*pte == 0)
						{
							while (++i < 100)
							{
								if (*gtd->vars[i])
								{
									RESTRING(gtd->vars[i], "");
								}
							}
							break;
						}

						if (*pte == COMMAND_SEPARATOR)
						{
							pte++;
						}

					}

					substitute(ses, node->arg2, buf, SUB_ARG);

					script_driver(ses, LIST_FUNCTION, buf);

					substitute(ses, "$result", pto, flags_neol|SUB_VAR);

					pto += strlen(pto);
				}
				else
				{
					if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG) && is_function(ses, pti))
					{
						*pto++ = '\\';
					}
					*pto++ = *pti++;
				}
				break;

			case '*':
				if (HAS_BIT(flags, SUB_VAR) && !HAS_BIT(ses->list[LIST_VARIABLE]->flags, LIST_FLAG_IGNORE) && pti[1])
				{
					int brace = FALSE;
					i = 1;
					escape = FALSE;

					while (pti[i] == '*')
					{
						escape = TRUE;

						i++;
					}

					if (pti[i] == DEFAULT_OPEN)
					{
						brace = TRUE;

						ptt = get_arg_in_braces(ses, &pti[i], buf, GET_ALL);

						i += strlen(buf) + 2;

						substitute(ses, buf, temp, flags_neol);
					}
					else
					{
						ptt = temp;

						while (isalnum((int) pti[i]) || pti[i] == '_')
						{
							*ptt++ = pti[i];

							i++;
						}
						*ptt = 0;
					}

					if (*temp)
					{
						root = local_list(ses);

						if ((node = search_node_list(root, temp)) == NULL)
						{
							root = ses->list[LIST_VARIABLE];

							node = search_node_list(root, temp);
						}
					}
					else
					{
						root = ses->list[LIST_VARIABLE];
						node = NULL;
					}

					if (brace == FALSE && node == NULL)
					{
						while (*pti == '*')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (escape)
					{
						pti++;

						while (*pti == '*')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					pti = get_arg_at_brackets(ses, &pti[i], temp + strlen(temp));

					substitute(ses, temp, buf, flags_neol);

					str = str_dup("");

					get_nest_node_key(root, buf, &str, brace);

					substitute(ses, str, pto, flags_neol - SUB_VAR);

					pto += strlen(pto);

					str_free(str);
				}
				else
				{
					if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG) && is_variable(ses, pti))
					{
						*pto++ = '\\';
					}
					*pto++ = *pti++;
				}
				break;

			case '$':
				if (HAS_BIT(flags, SUB_VAR) && !HAS_BIT(ses->list[LIST_VARIABLE]->flags, LIST_FLAG_IGNORE) && pti[1])
				{
					int brace = FALSE;
					i = 1;
					escape = FALSE;

					while (pti[i] == '$')
					{
						escape = TRUE;

						i++;
					}

					if (pti[i] == DEFAULT_OPEN)
					{
						brace = TRUE;

						ptt = get_arg_in_braces(ses, &pti[i], buf, GET_ALL);

						i += strlen(buf) + 2;

						substitute(ses, buf, temp, flags_neol);
					}
					else
					{
						ptt = temp;

						while (isalnum((int) pti[i]) || pti[i] == '_')
						{
							*ptt++ = pti[i];

							i++;
						}
						*ptt = 0;
					}

					if (*temp)
					{
						root = local_list(ses);

						if ((node = search_node_list(root, temp)) == NULL)
						{
							root = ses->list[LIST_VARIABLE];

							node = search_node_list(root, temp);
						}
					}
					else
					{
						root = ses->list[LIST_VARIABLE];
						node = NULL;
					}

					if (brace == FALSE && node == NULL)
					{
						while (*pti == '$')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (escape)
					{
						pti++;

						while (*pti == '$')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					pti = get_arg_at_brackets(ses, &pti[i], temp + strlen(temp));

					substitute(ses, temp, buf, flags_neol);

					str = str_dup("");

					get_nest_node_val(root, buf, &str, brace);

					substitute(ses, str, pto, flags_neol - SUB_VAR);

					pto += strlen(pto);

					str_free(str);
				}
				else
				{
					if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG) && is_variable(ses, pti))
					{
						*pto++ = '\\';
					}
					*pto++ = *pti++;
				}
				break;

			case '&':
				if (HAS_BIT(flags, SUB_CMD) && (isdigit((int) pti[1]) || pti[1] == '&'))
				{
					if (pti[1] == '&')
					{
						while (pti[1] == '&')
						{
							*pto++ = *pti++;
						}
						if (isdigit((int) pti[1]))
						{
							pti++;
						}
						else
						{
							*pto++ = *pti++;
						}
					}
					else
					{
						i = isdigit((int) pti[2]) ? (pti[1] - '0') * 10 + pti[2] - '0' : pti[1] - '0';

						for (cnt = 0 ; gtd->cmds[i][cnt] ; cnt++)
						{
							*pto++ = gtd->cmds[i][cnt];
						}
						pti += isdigit((int) pti[2]) ? 3 : 2;
					}
				}
				else if (HAS_BIT(flags, SUB_VAR) && !HAS_BIT(ses->list[LIST_VARIABLE]->flags, LIST_FLAG_IGNORE))
				{
					int brace = FALSE;
					i = 1;
					escape = FALSE;

					while (pti[i] == '&')
					{
						escape = TRUE;

						i++;
					}

					if (pti[i] == DEFAULT_OPEN)
					{
						brace = TRUE;

						ptt = get_arg_in_braces(ses, &pti[i], buf, GET_ALL);

						i += strlen(buf) + 2;

						substitute(ses, buf, temp, flags_neol);
					}
					else
					{
						ptt = temp;

						while (isalnum((int) pti[i]) || pti[i] == '_')
						{
							*ptt++ = pti[i];

							i++;
						}
						*ptt = 0;
					}

					if (*temp)
					{
						root = local_list(ses);

						if ((node = search_node_list(root, temp)) == NULL)
						{
							root = ses->list[LIST_VARIABLE];
							node = search_node_list(root, temp);
						}
					}
					else
					{
						root = ses->list[LIST_VARIABLE];
						node = NULL;
					}

					if (brace == FALSE && node == NULL)
					{
						while (*pti == '&')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					if (escape)
					{
						pti++;

						while (*pti == '&')
						{
							*pto++ = *pti++;
						}
						continue;
					}

					pti = get_arg_at_brackets(ses, &pti[i], temp + strlen(temp));

					substitute(ses, temp, buf, flags_neol);

					str = str_dup("");

					get_nest_index(root, buf, &str, brace);

					substitute(ses, str, pto, flags_neol - SUB_VAR);

					pto += strlen(pto);

					str_free(str);
				}
				else
				{
					if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG) && is_variable(ses, pti))
					{
						*pto++ = '\\';
					}
					*pto++ = *pti++;
				}
				break;

			case '%':
				if (HAS_BIT(flags, SUB_ARG) && (isdigit((int) pti[1]) || pti[1] == '%'))
				{
					if (pti[1] == '%')
					{
						while (pti[1] == '%')
						{
							*pto++ = *pti++;
						}
						pti++;
					}
					else
					{
						i = isdigit((int) pti[2]) ? (pti[1] - '0') * 10 + pti[2] - '0' : pti[1] - '0';

						ptt = gtd->vars[i];

						while (*ptt)
						{
							if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *ptt & 128 && ptt[1] != 0)
							{
								*pto++ = *ptt++;
								*pto++ = *ptt++;
								continue;
							}

							if (HAS_BIT(flags, SUB_SEC))
							{
								switch (*ptt)
								{
									case '\\':
										*pto++ = '\\';
										*pto++ = '\\';
										break;

									case '{':
										*pto++ = '\\';
										*pto++ = 'x';
										*pto++ = '7';
										*pto++ = 'B';
										break;

									case '}':
										*pto++ = '\\';
										*pto++ = 'x';
										*pto++ = '7';
										*pto++ = 'D';
										break;

									case '$':
									case '&':
									case '*':
										if (is_variable(ses, ptt))
										{
											*pto++ = '\\';
											*pto++ = *ptt;
										}
										else
										{
											*pto++ = *ptt;
										}
										break;

									case '@':
										if (is_function(ses, ptt))
										{
											*pto++ = '\\';
											*pto++ = *ptt;
										}
										else
										{
											*pto++ = *ptt;
										}
										break;

									case COMMAND_SEPARATOR:
										*pto++ = '\\';
										*pto++ = COMMAND_SEPARATOR;
										break;

									default:
										*pto++ = *ptt;
										break;
								}
								ptt++;
							}
							else
							{
								*pto++ = *ptt++;
							}
						}
						pti += isdigit((int) pti[2]) ? 3 : 2;
					}
				}
				else
				{
					*pto++ = *pti++;
				}
				break;

			case '<':
				if (HAS_BIT(flags, SUB_COL) && isalnum((int) pti[1]))
				{
					if (HAS_BIT(flags, SUB_CMP) && old[0] && !strncmp(old, pti, strlen(old)))
					{
						pti += strlen(old);
					}
					else if (isdigit((int) pti[1]) && isdigit((int) pti[2]) && isdigit((int) pti[3]) && pti[4] == '>')
					{
						if (pti[1] != '8' || pti[2] != '8' || pti[3] != '8')
						{
							*pto++ = ESCAPE;
							*pto++ = '[';

							switch (pti[1])
							{
								case '2':
									*pto++ = '2';
									*pto++ = '2';
									*pto++ = ';';
									break;
								case '8':
									break;
								default:
									*pto++ = pti[1];
									*pto++ = ';';
							}
							switch (pti[2])
							{
								case '8':
									break;
								default:
									*pto++ = '3';
									*pto++ = pti[2];
									*pto++ = ';';
									break;
							}
							switch (pti[3])
							{
								case '8':
									break;
								default:
									*pto++ = '4';
									*pto++ = pti[3];
									*pto++ = ';';
									break;
							}
							pto--;
							*pto++ = 'm';
						}
						pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
					}
					else if (pti[1] >= 'a' && pti[1] <= 'f' && pti[2] >= 'a' && pti[2] <= 'f' && pti[3] >= 'a' && pti[3] <= 'f' && pti[4] == '>')
					{
						*pto++ = ESCAPE;
						*pto++ = '[';
						*pto++ = '3';
						*pto++ = '8';
						*pto++ = ';';
						*pto++ = '5';
						*pto++ = ';';
						cnt = 16 + (pti[1] - 'a') * 36 + (pti[2] - 'a') * 6 + (pti[3] - 'a');
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = 'm';
						pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
					}
					else if (pti[1] >= 'A' && pti[1] <= 'F' && pti[2] >= 'A' && pti[2] <= 'F' && pti[3] >= 'A' && pti[3] <= 'F' && pti[4] == '>')
					{
						*pto++ = ESCAPE;
						*pto++ = '[';
						*pto++ = '4';
						*pto++ = '8';
						*pto++ = ';';
						*pto++ = '5';
						*pto++ = ';';
						cnt = 16 + (pti[1] - 'A') * 36 + (pti[2] - 'A') * 6 + (pti[3] - 'A');
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = 'm';
						pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
					}
					else if (pti[1] == 'g' && isdigit((int) pti[2]) && isdigit((int) pti[3]) && pti[4] == '>')
					{
						*pto++ = ESCAPE;
						*pto++ = '[';
						*pto++ = '3';
						*pto++ = '8';
						*pto++ = ';';
						*pto++ = '5';
						*pto++ = ';';
						cnt = 232 + (pti[2] - '0') * 10 + (pti[3] - '0');
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = 'm';
						pti += sprintf(old, "<g%c%c>", pti[2], pti[3]);
					}
					else if (pti[1] == 'G' && isdigit((int) pti[2]) && isdigit((int) pti[3]) && pti[4] == '>')
					{
						*pto++ = ESCAPE;
						*pto++ = '[';
						*pto++ = '4';
						*pto++ = '8';
						*pto++ = ';';
						*pto++ = '5';
						*pto++ = ';';
						cnt = 232 + (pti[2] - '0') * 10 + (pti[3] - '0');
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = 'm';
						pti += sprintf(old, "<G%c%c>", pti[2], pti[3]);
					}
					else if (toupper((int) pti[1]) == 'F' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[3]) && pti[5] == '>')
					{
						*pto++ = ESCAPE;
						*pto++ = '[';
						*pto++ = '3';
						*pto++ = '8';
						*pto++ = ';';
						*pto++ = '2';
						*pto++ = ';';
						cnt  = isdigit(pti[2]) ? (pti[2] - '0') : (pti[2] - 'A' + 10);
						cnt += cnt * 16;
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = ';';
						cnt  = isdigit(pti[3]) ? (pti[3] - '0') : (pti[3] - 'A' + 10);
						cnt += cnt * 16;
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = ';';
						cnt  = isdigit(pti[4]) ? (pti[4] - '0') : (pti[4] - 'A' + 10);
						cnt += cnt * 16;
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = 'm';
						pti += sprintf(old, "<F%c%c%c>", pti[2], pti[3], pti[4]);
					}
					else if (toupper((int) pti[1]) == 'F' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
					{
						*pto++ = ESCAPE;
						*pto++ = '[';
						*pto++ = '3';
						*pto++ = '8';
						*pto++ = ';';
						*pto++ = '2';
						*pto++ = ';';
						cnt  = isdigit(pti[2]) ? 16 * (pti[2] - '0') : 16 * (pti[2] - 'A' + 10);
						cnt += isdigit(pti[3]) ?  1 * (pti[3] - '0') :  1 * (pti[3] - 'A' + 10);
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = ';';
						cnt  = isdigit(pti[4]) ? 16 * (pti[4] - '0') : 16 * (pti[4] - 'A' + 10);
						cnt += isdigit(pti[5]) ?  1 * (pti[5] - '0') :  1 * (pti[5] - 'A' + 10);
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = ';';
						cnt  = isdigit(pti[6]) ? 16 * (pti[6] - '0') : 16 * (pti[6] - 'A' + 10);
						cnt += isdigit(pti[7]) ?  1 * (pti[7] - '0') :  1 * (pti[7] - 'A' + 10);
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = 'm';
						pti += sprintf(old, "<F%c%c%c%c%c%c>", pti[2], pti[3], pti[4], pti[5], pti[6], pti[7]);
					}
					else if (toupper((int) pti[1]) == 'B' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[3]) && pti[5] == '>')
					{
						*pto++ = ESCAPE;
						*pto++ = '[';
						*pto++ = '4';
						*pto++ = '8';
						*pto++ = ';';
						*pto++ = '2';
						*pto++ = ';';
						cnt  = isdigit(pti[2]) ? (pti[2] - '0') : (pti[2] - 'A' + 10);
						cnt += cnt * 16;
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = ';';
						cnt  = isdigit(pti[3]) ? (pti[3] - '0') : (pti[3] - 'A' + 10);
						cnt += cnt * 16;
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = ';';
						cnt  = isdigit(pti[4]) ? (pti[4] - '0') : (pti[4] - 'A' + 10);
						cnt += cnt * 16;
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = 'm';
						pti += sprintf(old, "<B%c%c%c>", pti[2], pti[3], pti[4]);
					}
					else if (toupper((int) pti[1]) == 'B' && isxdigit((int) pti[2]) && isxdigit((int) pti[3]) && isxdigit((int) pti[4]) && isxdigit((int) pti[5]) && isxdigit((int) pti[6]) && isxdigit((int) pti[7]) && pti[8] == '>')
					{
						*pto++ = ESCAPE;
						*pto++ = '[';
						*pto++ = '4';
						*pto++ = '8';
						*pto++ = ';';
						*pto++ = '2';
						*pto++ = ';';
						cnt  = isdigit(pti[2]) ? 16 * (pti[2] - '0') : 16 * (pti[2] - 'A' + 10);
						cnt += isdigit(pti[3]) ?  1 * (pti[3] - '0') :  1 * (pti[3] - 'A' + 10);
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = ';';
						cnt  = isdigit(pti[4]) ? 16 * (pti[4] - '0') : 16 * (pti[4] - 'A' + 10);
						cnt += isdigit(pti[5]) ?  1 * (pti[5] - '0') :  1 * (pti[5] - 'A' + 10);
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = ';';
						cnt  = isdigit(pti[6]) ? 16 * (pti[6] - '0') : 16 * (pti[6] - 'A' + 10);
						cnt += isdigit(pti[7]) ?  1 * (pti[7] - '0') :  1 * (pti[7] - 'A' + 10);
						*pto++ = '0' + cnt / 100;
						*pto++ = '0' + cnt % 100 / 10;
						*pto++ = '0' + cnt % 10;
						*pto++ = 'm';
						pti += sprintf(old, "<B%c%c%c%c%c%c>", pti[2], pti[3], pti[4], pti[5], pti[6], pti[7]);
					}
					else
					{
						*pto++ = *pti++;
					}
				}
				else
				{
					*pto++ = *pti++;
				}
				break;


			case '\\':
				if (HAS_BIT(flags, SUB_ESC))
				{
					pti++;

					switch (*pti)
					{
						case 'a':
							*pto++ = '\a';
							break;
						case 'b':
							*pto++ = '\b';
							break;
						case 'c':
							if (pti[1])
							{
								pti++;
								*pto++ = *pti % 32;
							}
							break;
						case 'e':
							*pto++ = '\e';
							break;
						case 'f':
							*pto++ = '\f';
							break;
						case 'n':
							*pto++ = '\n';
							break;
						case 'r':
							*pto++ = '\r';
							break;
						case 't':
							*pto++ = '\t';
							break;
						case 'x':
							if (pti[1] && pti[2])
							{
								if (pti[1] == '0' && pti[2] == '0' && pti[3] == 0)
								{
									pti += 2;
									DEL_BIT(flags, SUB_EOL);
									DEL_BIT(flags, SUB_LNF);
								}
								else
								{
									pti++;
									*pto++ = hex_number_8bit(pti);
									pti++;
								}
							}
							break;

						case 'u':
							if (pti[1] && pti[2] && pti[3] && pti[4])
							{
								pto += unicode_16_bit(&pti[1], pto);
								pti += 4;
							}
							break;
						case 'U':
							if (pti[1] && pti[2] && pti[3] && pti[4] && pti[5] && pti[6])
							{
								pto += unicode_21_bit(&pti[1], pto);
								pti += 6;
							}
							break;

						case 'v':
							*pto++ = '\v';
							break;
						case '0':
							if (pti[1] == 0)
							{
								DEL_BIT(flags, SUB_EOL);
								DEL_BIT(flags, SUB_LNF);
							}
							else if (pti[1] && pti[2])
							{
								pti++;
								*pto++ = oct_number(pti);
								pti++;
							}
							break;

						case '\0':
							DEL_BIT(flags, SUB_EOL);
							DEL_BIT(flags, SUB_LNF);
							continue;

						default:
							*pto++ = *pti;
							break;
					}
					pti++;
				}
				else if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG))
				{
					*pto++ = '\\';
					*pto++ = *pti++;
				}
				else if (HAS_BIT(flags, SUB_LIT))
				{
					*pto++ = *pti++;
				}
				else
				{
					*pto++ = *pti++;

					if (*pti)
					{
						*pto++ = *pti++;
					}
				}
				break;

			default:
				if (HAS_BIT(flags, SUB_SEC) && !HAS_BIT(flags, SUB_ARG))
				{
					switch (*pti)
					{
						case '{':
							*pto++ = '\\';
							*pto++ = 'x';
							*pto++ = '7';
							*pto++ = 'B';
							break;

						case '}':
							*pto++ = '\\';
							*pto++ = 'x';
							*pto++ = '7';
							*pto++ = 'D';
							break;

						case COMMAND_SEPARATOR:
							*pto++ = '\\';
							*pto++ = COMMAND_SEPARATOR;
							break;

						default:
							*pto++ = *pti;
							break;
					}
					pti++;
				}
				else
				{
					*pto++ = *pti++;
				}
				break;
		}
	}
	pop_call();
	return 0;
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

	return tintin_regexp(ses, node->regex, str, exp, option, SUB_ARG);
}

/*
	Keep synched with tintin_regexp and tintin_regexp_compile
*/

int tintin_regexp_check(struct session *ses, char *exp)
{
	if (*exp == '^')
	{
		return TRUE;
	}

	while (*exp)
	{

		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *exp & 128 && exp[1] != 0)
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
					case 'd':
					case 'D':
					case 'i':
					case 'I':
					case 's':
					case 'S':
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
							case 'd':
							case 'D':
							case 's':
							case 'S':
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
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
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
						fix = SUB_FIX;
						arg = isdigit((int) pti[2]) ? (pti[1] - '0') * 10 + (pti[2] - '0') : pti[1] - '0';
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += isdigit((int) pti[2]) ? 3 : 2;
						strcpy(pto, *pti == 0 ? "(.*)" : "(.*?)");
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

					case '?':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "(.?)" : "(.?" "?)");
						pto += strlen(pto);
						break;

					case '*':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "(.*)" : "(.*?)");
						pto += strlen(pto);
						break;

					case '+':
						gtd->args[next_arg(var)] = next_arg(arg);
						pti += 2;
						strcpy(pto, *pti == 0 ? "(.+)" : "(.+?)");
						pto += strlen(pto);
						break;

					case '.':
						gtd->args[next_arg(var)] = next_arg(arg);
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
								pti += 3;
								strcpy(pto, *pti == 0 ? ".+" : ".+?");
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

	return regexp_compare(nodepcre, str, out, option, flag + fix);
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
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
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
						pti += 2;
						strcpy(pto, *pti == 0 ? "(.+)" : "(.+?)");
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
								pti += 3;
								strcpy(pto, *pti == 0 ? ".+" : ".+?");
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

//	printf("debug regex compile (%s)\n", out);

	return regexp_compile(out, option);
}
