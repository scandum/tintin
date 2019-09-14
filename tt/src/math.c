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

#define EXP_VARIABLE         0
#define EXP_STRING           1
#define EXP_OPERATOR         2

#define EXP_PR_DICE          0
#define EXP_PR_INTMUL        1
#define EXP_PR_INTADD        2
#define EXP_PR_BITSHIFT      3
#define EXP_PR_LOGLTGT       4
#define EXP_PR_LOGCOMP       5
#define EXP_PR_BITAND        6
#define EXP_PR_BITXOR        7
#define EXP_PR_BITOR         8
#define EXP_PR_LOGAND        9
#define EXP_PR_LOGXOR       10
#define EXP_PR_LOGOR        11
#define EXP_PR_VAR          12
#define EXP_PR_LVL          13

struct link_data *math_head;
struct link_data *math_tail;
struct link_data *mathnode_s;
struct link_data *mathnode_e;

int precision;

DO_COMMAND(do_math)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	struct listnode *node;
	long double result;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_VAR|SUB_FUN);

	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg1 == 0 || *arg2 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #MATH {variable} {expression}.");
	}
	else
	{
		result = get_number(ses, arg2);

		node = set_nest_node(ses->list[LIST_VARIABLE], arg1, "%.*Lf", precision, result);

		show_message(ses, LIST_VARIABLE, "#MATH: VARIABLE {%s} HAS BEEN SET TO {%s}.", arg1, node->arg2);
	}

	return ses;
}

int is_math(struct session *ses, char *str)
{
	return mathexp_tokenize(ses, str, 0, 0);
}

long double get_number(struct session *ses, char *str)
{
	long double val;
	char result[BUFFER_SIZE];

	mathexp(ses, str, result, 0);

	val = tintoi(result);

	return val;
}

long double get_double(struct session *ses, char *str)
{
	long double val;
	char result[BUFFER_SIZE];

	mathexp(ses, str, result, 1);

	val = tintoi(result);

	return val;
}
	
void get_number_string(struct session *ses, char *str, char *result)
{
	sprintf(result, "%.*Lf", precision, get_number(ses, str));
}

long double mathswitch(struct session *ses, char *left, char *right)
{
	char shift[BUFFER_SIZE];

	sprintf(shift, "%s == %s", left, right);

	return get_number(ses, shift);
}

/*
	Flexible tokenized mathematical expression interpreter
	
	If seed is set it forces floating point math
*/

void mathexp(struct session *ses, char *str, char *result, int seed)
{
	struct link_data *node;

	substitute(ses, str, result, SUB_VAR|SUB_FUN);

	if (mathexp_tokenize(ses, result, seed, TRUE) == FALSE)
	{
		return;
	}

	node = math_head;

	while (node->prev || node->next)
	{
		if (node->next == NULL || atoi(node->next->str1) < atoi(node->str1))
		{
			mathexp_level(ses, node);

			node = math_head;
		}
		else
		{
			node = node->next;
		}
	}

	strcpy(result, node->str3);
}

#define MATH_NODE(buffercheck, priority, newstatus)             \
{                                                               \
	if (buffercheck && pta == buf3)                         \
	{                                                       \
		return FALSE;                                   \
	}                                                       \
	if (badnumber && debug)                                 \
	{                                                       \
		badnumber = 0;                                  \
		show_debug(ses, LIST_VARIABLE, "#MATH EXP: INVALID NUMBER %s.", buf3); \
	}                                                       \
	*pta = 0;                                               \
	sprintf(buf1, "%02d", level);                           \
	sprintf(buf2, "%02d", priority);                        \
	add_math_node(buf1, buf2, buf3);                        \
	status = newstatus;                                     \
	pta = buf3;                                             \
	point = -1;                                             \
}

void add_math_node(char *arg1, char *arg2, char *arg3)
{
	struct link_data *link;

	link = (struct link_data *) calloc(1, sizeof(struct link_data));

	link->str1 = strdup(arg1);
	link->str2 = strdup(arg2);
	link->str3 = strdup(arg3);

	LINK(link, math_head, math_tail);
}

void del_math_node(struct link_data *node)
{
	UNLINK(node, math_head, math_tail);

	free(node->str1);
	free(node->str2);
	free(node->str3);

	free(node);
}

int mathexp_tokenize(struct session *ses, char *str, int seed, int debug)
{
	char buf1[BUFFER_SIZE], buf2[BUFFER_SIZE], buf3[STRING_SIZE], *pti, *pta;
	int level, status, point, badnumber, nest;

	nest      = 0;
	level     = 0;
	point     = -1;
	status    = EXP_VARIABLE;
	precision = seed;
	badnumber = 0;

	pta = buf3;
	pti = str;

	while (math_head)
	{
		del_math_node(math_head);
	}

	while (*pti)
	{
		switch (status)
		{
			case EXP_VARIABLE:
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
						*pta++ = *pti++;

						if (point >= 0)
						{
							point++;

							if (precision < point)
							{
								precision = point;
							}
						}
						break;

					case '!':
					case '~':
					case '+':
					case '-':
						if (pta != buf3)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							*pta++ = *pti++;
						}
						break;

					case '{':
						if (pta != buf3)
						{
							if (debug)
							{
								show_debug(ses, LIST_VARIABLE, "MATH EXP: { FOUND INSIDE A NUMBER");
							}
							return FALSE;
						}
						*pta++ = *pti++;
						status = EXP_STRING;
						nest++;
						break;
					case '"':
						if (pta != buf3)
						{
							if (debug)
							{
								show_debug(ses, LIST_VARIABLE, "MATH EXP: \" FOUND INSIDE A NUMBER");
							}
							return FALSE;
						}
						*pta++ = *pti++;
						nest++;
						status = EXP_STRING;
						break;

					case '(':
						if (pta != buf3)
						{
							if (debug)
							{
								show_debug(ses, LIST_VARIABLE, "#MATH EXP: PARANTESES FOUND INSIDE A NUMBER");
							}
							return FALSE;
						}
						*pta++ = *pti++;
						MATH_NODE(FALSE, EXP_PR_LVL, EXP_VARIABLE);
						level++;
						break;

					case ',':
						pti++;
						break;

					case ':':
						*pta++ = *pti++;
						break;
						
					case '.':
						*pta++ = *pti++;
						if (point >= 0)
						{
							if (debug)
							{
								show_debug(ses, LIST_VARIABLE, "#MATH EXP: MORE THAN ONE POINT FOUND INSIDE A NUMBER");
							}
							precision = 0;
							return FALSE;
						}
						point++;
						break;

					case ' ':
					case '\t':
						pti++;
						break;

					case ')':
					case 'd':
					case '*':
					case '/':
					case '%':
					case '<':
					case '>':
					case '&':
					case '^':
					case '|':
					case '=':
						if (pta != buf3)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							*pta++ = *pti++;
						}
						break;

					default:
						*pta++ = *pti++;
						*pta = 0;

						badnumber = 1;

						if (debug == 0)
						{
							return FALSE;
						}
						break;
				}
				break;

			case EXP_STRING:
				switch (*pti)
				{
					case '{':
						*pta++ = *pti++;
						nest++;
						break;

					case '}':
						*pta++ = *pti++;
						nest--;
						if (nest == 0)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						}
						break;

					case '"':
						*pta++ = *pti++;
						nest--;
						if (nest == 0)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						}
						break;

					default:
						*pta++ = *pti++;
						break;
				}
				break;

			case EXP_OPERATOR:
				switch (*pti)
				{
					case ' ':
						pti++;
						break;

					case ')':
						*pta++ = *pti++;
						level--;
						MATH_NODE(FALSE, EXP_PR_LVL, EXP_OPERATOR);
						break;

					case 'd':
						*pta++ = *pti++;
						MATH_NODE(FALSE, EXP_PR_DICE, EXP_VARIABLE);
						break;

					case '*':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '*':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_INTMUL, EXP_VARIABLE);
								break;
							
							default:
								MATH_NODE(FALSE, EXP_PR_INTMUL, EXP_VARIABLE);
								break;
						}
						break;
	
					case '/':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '/':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_INTMUL, EXP_VARIABLE);
								break;
							default:
								MATH_NODE(FALSE, EXP_PR_INTMUL, EXP_VARIABLE);
								break;
						}
						break;

					case '%':
						*pta++ = *pti++;
						MATH_NODE(FALSE, EXP_PR_INTMUL, EXP_VARIABLE);
						break;

					case '+':
					case '-':
						*pta++ = *pti++;
						MATH_NODE(FALSE, EXP_PR_INTADD, EXP_VARIABLE);
						break;

					case '<':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '<':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_BITSHIFT, EXP_VARIABLE);
								break;

							case '=':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGLTGT, EXP_VARIABLE);
								break;

							default:
								MATH_NODE(FALSE, EXP_PR_LOGLTGT, EXP_VARIABLE);
								break;
						}
						break;

					case '>':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '>':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_BITSHIFT, EXP_VARIABLE);
								break;

							case '=':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGLTGT, EXP_VARIABLE);
								break;

							default:
								MATH_NODE(FALSE, EXP_PR_LOGLTGT, EXP_VARIABLE);
								break;
						}
						break;

					case '&':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '&':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGAND, EXP_VARIABLE);
								break;

							default:
								MATH_NODE(FALSE, EXP_PR_BITAND, EXP_VARIABLE);
								break;
						}
						break;

					case '^':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '^':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGXOR, EXP_VARIABLE);
								break;

							default:
								MATH_NODE(FALSE, EXP_PR_BITXOR, EXP_VARIABLE);
								break;

						}
						break;

					case '|':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '|':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGOR, EXP_VARIABLE);
								break;

							default:
								MATH_NODE(FALSE, EXP_PR_BITOR, EXP_VARIABLE);
								break;
						}
						break;

					case '=':
					case '!':
						*pta++ = *pti++;
						switch (*pti)
						{
							case '=':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGCOMP, EXP_VARIABLE);
								break;

							default:
								if (debug)
								{
									show_debug(ses, LIST_VARIABLE, "#MATH EXP: UNKNOWN OPERATOR: %c%c", pti[-1], pti[0]);
								}
								return FALSE;
						}
						break;

					default:
						if (debug)
						{
							show_debug(ses, LIST_VARIABLE, "#MATH EXP: UNKNOWN OPERATOR: %c", *pti);
						}
						return FALSE;
				}
				break;
		}
	}

	if (level != 0)
	{
		if (debug)
		{
			show_debug(ses, LIST_VARIABLE, "#MATH EXP: UNMATCHED PARENTHESES, LEVEL: %d", level);
		}
		return FALSE;
	}

	if (status != EXP_OPERATOR)
	{
		MATH_NODE(TRUE, EXP_PR_VAR, EXP_OPERATOR);
	}

	return TRUE;
}


void mathexp_level(struct session *ses, struct link_data *node)
{
	int priority;

	mathnode_e = node;

	while (node->prev)
	{
		if (atoi(node->prev->str1) < atoi(node->str1))
		{
			break;
		}
		node = node->prev;
	}

	mathnode_s = node;

	for (priority = 0 ; priority < EXP_PR_VAR ; priority++)
	{
		for (node = mathnode_s ; node ; node = node->next)
		{
			if (atoi(node->str2) == priority)
			{
				mathexp_compute(ses, node);
			}
			if (node == mathnode_e)
			{
				break;
			}
		}
	}

	node = mathnode_s;

	while (node->prev && node->next)
	{
		if (atoi(node->prev->str2) == EXP_PR_LVL && atoi(node->next->str2) == EXP_PR_LVL)
		{
			free(node->str1);
			node->str1 = strdup(node->next->str1);

			del_math_node(node->next);
			del_math_node(node->prev);
		}
		else
		{
			break;
		}
	}
	return;
}

void mathexp_compute(struct session *ses, struct link_data *node)
{
	char temp[BUFFER_SIZE];
	long double value;

	switch (node->str3[0])
	{
		case 'd':
			if (tintoi(node->next->str3) <= 0)
			{
				show_debug(ses, LIST_VARIABLE, "#MATHEXP: INVALID DICE: %s", node->next->str3);
				value = 0;
			}
			else
			{
				value = tindice(ses, node->prev->str3, node->next->str3);
			}
			break;
		case '*':
			switch (node->str3[1])
			{
				case '*':
					value = pow(tintoi(node->prev->str3), tintoi(node->next->str3));
					break;
				default:
					value = tintoi(node->prev->str3) * tintoi(node->next->str3);
					break;
			}
			break;
		case '/':
			switch (node->str3[1])
			{
				case '/':
					if (tintoi(node->next->str3) == 3)
					{
						value = cbrt(tintoi(node->prev->str3));
					}
					else
					{
						value = sqrt(tintoi(node->prev->str3));
					}
					break;
				default:
					if (tintoi(node->next->str3) == 0)
					{
						show_debug(ses, LIST_VARIABLE, "#MATH ERROR: DIVISION ZERO.");
						value = tintoi(node->prev->str3);
					}
					else
					{
						if (precision)
						{
							value = tintoi(node->prev->str3) / tintoi(node->next->str3);
						}
						else
						{
							value = (long long) tintoi(node->prev->str3) / (long long) tintoi(node->next->str3);
						}
					}
					break;
			}
			break;
		case '%':
			if (tintoi(node->next->str3) == 0)
			{
				show_debug(ses, LIST_VARIABLE, "#MATH ERROR: MODULO ZERO.");
				value = tintoi(node->prev->str3);
			}
			else
			{
				// Can't perform modulo on doubles

				value = (long long) tintoi(node->prev->str3) % (long long) tintoi(node->next->str3);
			}
			break;
		case '+':
			value = tintoi(node->prev->str3) + tintoi(node->next->str3);
			break;
		case '-':
			value = tintoi(node->prev->str3) - tintoi(node->next->str3);
			break;
		case '<':
			switch (node->str3[1])
			{
				case '=':
					value = tincmp(node->prev->str3, node->next->str3) <= 0;
					break;
				case '<':
					value = (long long) tintoi(node->prev->str3) << (long long) tintoi(node->next->str3);
					break;
				default:
					value = tincmp(node->prev->str3, node->next->str3) < 0;
					break;
			}
			break;
		case '>':
			switch (node->str3[1])
			{
				case '=':
					value = tincmp(node->prev->str3, node->next->str3) >= 0;
					break;
				case '>':
					value = (long long) tintoi(node->prev->str3) >> (long long) tintoi(node->next->str3);
					break;
				default:
					value = tincmp(node->prev->str3, node->next->str3) > 0;
					break;
			}
			break;

		case '&':
			switch (node->str3[1])
			{
				case '&':
					value = tintoi(node->prev->str3) && tintoi(node->next->str3);
					break;
				default:
					value = (long long) tintoi(node->prev->str3) & (long long) tintoi(node->next->str3);
					break;
			}
			break;
		case '^':
			switch (node->str3[1])
			{
				case '^':
					value = ((tintoi(node->prev->str3) || tintoi(node->next->str3)) && (!tintoi(node->prev->str3) != !tintoi(node->next->str3)));
					break;

				default:
					value = (long long) tintoi(node->prev->str3) ^ (long long) tintoi(node->next->str3);
					break;
			}
			break;
		case '|':
			switch (node->str3[1])
			{
				case '|':
					value = tintoi(node->prev->str3) || tintoi(node->next->str3);
					break;

				default:
					value = (long long) tintoi(node->prev->str3) | (long long) tintoi(node->next->str3);
					break;
			}
			break;
		case '=':
			value = (tineval(ses, node->prev->str3, node->next->str3) != 0);
			break;
		case '!':
			value = (tineval(ses, node->prev->str3, node->next->str3) == 0);
			break;
		default:
			show_debug(ses, LIST_VARIABLE, "#COMPUTE EXP: UNKNOWN OPERATOR: %c%c", node->str3[0], node->str3[1]);
			value = 0;
			break;
	}

	if (node->prev == mathnode_s)
	{
		mathnode_s = node;
	}

	if (node->next == mathnode_e)
	{
		mathnode_e = node;
	}

	del_math_node(node->next);
	del_math_node(node->prev);

	sprintf(temp, "%d", EXP_PR_VAR);
	free(node->str2);
	node->str2 = strdup(temp);

	sprintf(temp, "%.*Lf", precision, value);
	free(node->str3);
	node->str3 = strdup(temp);
}

/*
	Keep synced with is_number()
*/

long double tintoi(char *str)
{
	char *ptr = str;
	long double values[5] = {0, 0, 0, 0, 0}, m = 1;
	int i = 1, d = 0;

	if (*ptr == 0)
	{
		return 0;
	}

	switch (*ptr)
	{
		case '!':
		case '~':
		case '+':
		case '-':
			ptr++;
			break;
	}

	ptr = str + strlen(str);

	while (TRUE)
	{
		ptr--;

		switch (*ptr)
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
				values[i] += (*ptr - '0') * m;
				m *= 10;
				break;

			case '.':
				if (d)
				{
					return 0;
				}
				d = 1;

				values[0] = values[1] / m;
				values[1] = 0;
				m = 1;
				break;

			case ':':
				switch (i)
				{
					case 2:
						values[i] *= 60;
						break;
					case 3:
						values[i] *= 60 * 60;
						break;
					case 4:
						return 0;
				}
				i++;
				m = 1;
				break;

			case '!':
			case '~':
			case '+':
			case '-':
				if (ptr == str)
				{
					break;
				}
				return 0;

			default:
				return 0;
		}

		if (ptr == str)
		{
			switch (i)
			{
				case 2:
					values[i] *= 60;
					break;
				case 3:
					values[i] *= 60 * 60;
					break;
				case 4:
					values[i] *= 60 * 60 * 24;
					break;
			}
			break;
		}
	}

	switch (*str)
	{
		case '!':
			return !(values[0] + values[1] + values[2] + values[3] + values[4]);
		case '~':
			return ~ (long long) (values[0] + values[1] + values[2] + values[3] + values[4]);
		case '+':
			return +(values[0] + values[1] + values[2] + values[3] + values[4]);
		case '-':
			return -(values[0] + values[1] + values[2] + values[3] + values[4]);
		default:
			return (values[0] + values[1] + values[2] + values[3] + values[4]);
	}

	switch (str[0])
	{
		case '!':
			return !atof(&str[1]);

		case '~':
			return (long double) ~atoll(&str[1]);

		case '+':
			return +atof(&str[1]);

		case '-':
			return -atof(&str[1]);

		default:
			return atof(str);
	}

	while (*ptr)
	{
		if (!isdigit((int) *ptr))
		{
			if (*ptr != '.')
			{
				return 0;
			}
			ptr++;

			while (*ptr)
			{
				if (!isdigit((int) *ptr))
				{
					return 0;
				}
				ptr++;
			}
		}
		else
		{
			ptr++;
		}
	}

	switch (str[0])
	{
		case '!':
			return !atof(&str[1]);

		case '~':
			return (long double) ~atoll(&str[1]);

		case '+':
			return +atof(&str[1]);

		case '-':
			return -atof(&str[1]);

		default:
			return atof(str);
	}
}

/*
	Keep synched with tintoi() and is_math()
*/

int is_number(char *str)
{
	char *ptr = str;
	int i = 1, d = 0, valid = 0;

	if (*ptr == 0)
	{
		return FALSE;
	}

	ptr = str + strlen(str);

	while (TRUE)
	{
		ptr--;

		switch (*ptr)
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
				valid = TRUE;
				break;

			case '.':
				if (d)
				{
					return FALSE;
				}
				d = 1;
				valid = FALSE;
				break;

			case ':':
				if (i == 4)
				{
					return FALSE;
				}
				i++;
				valid = FALSE;
				break;

			case '!':
			case '~':
			case '+':
			case '-':
				if (ptr != str)
				{
					return FALSE;
				}
				break;

			default:
				return FALSE;
		}

		if (ptr == str)
		{
			break;
		}
	}
	return valid;
}

long double tincmp(char *left, char *right)
{
	long double left_val, right_val;

	switch (left[0])
	{
		case '{':
			return strcmp(left, right);

		case '"':
			return strcmp(left, right);

		default:
			left_val = tintoi(left);
			right_val = tintoi(right);

			return left_val - right_val;
	}
}

long double tineval(struct session *ses, char *left, char *right)
{
	long double left_val, right_val;

	switch (left[0])
	{
		case '{':
			get_arg_in_braces(ses, left, left, GET_ALL);
			get_arg_in_braces(ses, right, right, GET_ALL);

			return match(ses, left, right, SUB_NONE);

		case '"':
			return match(ses, left, right, SUB_NONE);

		default:
			left_val = tintoi(left);
			right_val = tintoi(right);

			return left_val == right_val;
	}
}

long double tindice(struct session *ses, char *left, char *right)
{
	unsigned long long cnt, numdice, sizedice, sum;

	numdice  = (unsigned long long) tintoi(left);
	sizedice = (unsigned long long) tintoi(right);

	if (sizedice == 0)
	{
		return 0;
	}

	for (cnt = sum = 0 ; cnt < numdice ; cnt++)
	{
		sum += generate_rand(ses) % sizedice + 1;
	}

	return (long double) sum;
}
