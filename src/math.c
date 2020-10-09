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

#include "tintin.h"

#define EXP_VARIABLE         0
#define EXP_STRING           1
#define EXP_BRACE            2
#define EXP_OPERATOR         3

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
#define EXP_PR_TERNARY      12
#define EXP_PR_VAR          13
#define EXP_PR_LVL          14

struct link_data *math_head;
struct link_data *math_tail;
struct link_data *mathnode_s;
struct link_data *mathnode_e;

int precision;

DO_COMMAND(do_math)
{
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

		node = set_nest_node_ses(ses, arg1, "%.*Lf", precision, result);

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

	SET_BIT(gtd->flags, TINTIN_FLAG_GETNUMBER);

	mathexp(ses, str, result, 0);

	DEL_BIT(gtd->flags, TINTIN_FLAG_GETNUMBER);

	val = tintoi(result);

	return val;
}

unsigned long long get_ulong(struct session *ses, char *str)
{
	unsigned long long val;
	char result[BUFFER_SIZE];

	mathexp(ses, str, result, 0);

	val = tintou(result);

	return val;
}

int get_ellipsis(struct listroot *root, char *name, int *min, int *max)
{
	size_t len;
	unsigned long long range;

	push_call("get_ellipsis(%p,%p,%p,%p)",root,name,min,max);

	len = strlen(name);

	if (name[len - 1] == '.')
	{
		strcpy(name + len, "-1");
	}

	range = get_ulong(root->ses, name);

	*min = (int) (HAS_BIT(range, 0x00000000FFFFFFFFLL));
	*max = (int) (HAS_BIT(range, 0xFFFFFFFF00000000ULL) >> 32ULL);

	*min = *min > 0 ? *min - 1 : root->used + *min;
	*max = *max > 0 ? *max - 1 : root->used + *max;

	*min = URANGE(0, *min, root->used - 1);
	*max = URANGE(0, *max, root->used - 1);

	pop_call();
	return 1 + (*min < *max ? *max - *min : *min - *max);
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
	long double val = get_number(ses, str);

	sprintf(result, "%.*Lf", precision, val);
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
		pop_call( );                                    \
		return FALSE;                                   \
	}                                                       \
	if (badnumber && debug)                                 \
	{                                                       \
		badnumber = 0;                                  \
		precision = 0;                                  \
		show_debug(ses, LIST_VARIABLE, "#MATH EXP {%s}: INVALID NUMBER %s.", str, buf3); \
	}                                                       \
	*pta = 0;                                               \
	sprintf(buf1, "%02d", level);                           \
	sprintf(buf2, "%02d", priority);                        \
	add_math_node(buf1, buf2, buf3);                        \
	status = newstatus;                                     \
	pta = buf3;                                             \
	point = -1;                                             \
	metric = 0;                                             \
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
	char *buf1, *buf2, *buf3, *pti, *pta;
	int level, status, point, badnumber, metric, nest;

	push_call("mathexp_tokenize(%p,%s,%d,%d)",ses,str,seed,debug);

	buf1 = str_alloc_stack(0);
	buf2 = str_alloc_stack(0);
	buf3 = str_alloc_stack(0);

	nest      = 0;
	level     = 0;
	metric    = 0;
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

						if (metric)
						{
							badnumber = 1;

							if (debug == 0)
							{
								pop_call();
								return FALSE;
							}
						}

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
						if (pta != buf3)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							add_math_node(ntos(level), ntos(EXP_PR_VAR), "0");
							add_math_node(ntos(level), ntos(EXP_PR_LOGCOMP), "==");

							*pta++ = *pti++;

							pta = buf3;
						}
						break;

					case '~':
						if (pta != buf3)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							add_math_node(ntos(level), ntos(EXP_PR_VAR), "-1");
							add_math_node(ntos(level), ntos(EXP_PR_INTADD), "-");

							*pta++ = *pti++;

							pta = buf3;
						}
						break;

					case '+':
						if (pta != buf3)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							add_math_node(ntos(level), ntos(EXP_PR_VAR), "1");
							add_math_node(ntos(level), ntos(EXP_PR_INTMUL), "*");

							*pta++ = *pti++;

							pta = buf3;
						}
						break;

					case '-':
						if (pta != buf3)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							add_math_node(ntos(level), ntos(EXP_PR_VAR), "-1");
							add_math_node(ntos(level), ntos(EXP_PR_INTMUL), "*");

							*pta++ = *pti++;

							pta = buf3;
						}
						break;

					case '{':
						if (pta != buf3)
						{
							if (debug)
							{
								show_debug(ses, LIST_VARIABLE, "MATH EXP: { FOUND INSIDE A NUMBER");
							}
							pop_call();
							return FALSE;
						}
						*pta++ = *pti++;
						status = EXP_BRACE;
						nest++;
						break;

					case '"':
						if (pta != buf3)
						{
							if (debug)
							{
								show_debug(ses, LIST_VARIABLE, "MATH EXP: \" FOUND INSIDE A NUMBER");
							}
							pop_call();
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
							pop_call();
							return FALSE;
						}
						else
						{
							*pta++ = *pti++;
							MATH_NODE(FALSE, EXP_PR_LVL, EXP_VARIABLE);
						}
						level++;
						break;

					case ',':
						pti++;
						break;

					case ':':
						*pta++ = *pti++;
						break;
						
					case '.':
						if (pti[1] == '.')
						{
							if (pta == buf3)
							{
								*pta++ = '1';
							}

							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);

							if (pti[2] == 0)
							{
								*pta++ = *pti++;
								*pta++ = *pti++;

								MATH_NODE(FALSE, EXP_PR_LOGCOMP, EXP_VARIABLE);
								
								*pta++ = '-';
								*pta++ = '1';
							}
						}
						else
						{
							*pta++ = *pti++;
							if (point >= 0)
							{
								if (debug)
								{
									show_debug(ses, LIST_VARIABLE, "#MATH EXP: MORE THAN ONE POINT FOUND INSIDE A NUMBER");
								}
								precision = 0;
								pop_call();
								return FALSE;
							}
							point++;
						}
						break;

					case ' ':
					case '\t':
						pti++;
						break;

					case 'd':
					case ')':
					case '*':
					case '/':
					case '%':
					case '<':
					case '>':
					case '&':
					case '^':
					case '|':
					case '=':
					case '?':
						if (pti == str)
						{
							if (debug)
							{
								show_debug(ses, LIST_VARIABLE, "#MATH EXP: EXPRESSION STARTED WITH AN OPERATOR.");
							}
							pop_call();
							return FALSE;
						}

						if (pta != buf3)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							*pta++ = *pti++;
						}
						break;

					case 'K':
					case 'M':
//					case 'G':
//					case 'T':
//					case 'P':
//					case 'E':
//					case 'Z':
//					case 'Y':
						if (pta == buf3 || metric == 1)
						{
							badnumber = 1;

							if (debug == 0)
							{
								pop_call();
								return FALSE;
							}
							*pta++ = *pti++;
							*pta = 0;
						}
						else if (badnumber == 0)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);

							*pta++ = '*';
							MATH_NODE(FALSE, EXP_PR_INTMUL, EXP_VARIABLE);

							switch (*pti++)
							{
								case 'K': pta += sprintf(pta, "1000"); break;
								case 'M': pta += sprintf(pta, "1000000"); break;
								case 'G': pta += sprintf(pta, "1000000000"); break;
								case 'T': pta += sprintf(pta, "1000000000000"); break;
								case 'P': pta += sprintf(pta, "1000000000000000"); break;
								case 'E': pta += sprintf(pta, "1000000000000000000"); break;
								case 'Z': pta += sprintf(pta, "1000000000000000000000"); break;
								case 'Y': pta += sprintf(pta, "1000000000000000000000000"); break;
							}
							metric = 1;
						}
						else
						{
							*pta++ = *pti++;
						}
						break;

					case 'm':
					case 'u':
//					case 'n':
//					case 'p':
//					case 'f':
//					case 'a':
//					case 'z':
//					case 'y':
						if (pta == buf3 || metric == 1)
						{
							badnumber = 1;

							if (debug == 0)
							{
								pop_call();
								return FALSE;
							}
							*pta++ = *pti++;
							*pta = 0;
						}
						else if (badnumber == 0)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);

							*pta++ = '/';

							MATH_NODE(FALSE, EXP_PR_INTMUL, EXP_VARIABLE);

							switch (*pti++)
							{
								case 'm': pta += sprintf(pta, "1000"); break;
								case 'u': pta += sprintf(pta, "1000000"); break;
								case 'n': pta += sprintf(pta, "1000000000"); break;
								case 'p': pta += sprintf(pta, "1000000000000"); break;
								case 'f': pta += sprintf(pta, "1000000000000000"); break;
								case 'a': pta += sprintf(pta, "1000000000000000000"); break;
								case 'z': pta += sprintf(pta, "1000000000000000000000"); break;
								case 'y': pta += sprintf(pta, "1000000000000000000000000"); break;
							}
							metric = 1;
							precision = UMAX(precision, strlen(buf3) -1);
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
							pop_call();
							return FALSE;
						}
						break;
				}
				break;

			case EXP_STRING:
				switch (*pti)
				{
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

			case EXP_BRACE:
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

					case '.':
						if (pti[1] == '.')
						{
							*pta++ = *pti++;
							*pta++ = *pti++;

							MATH_NODE(FALSE, EXP_PR_LOGCOMP, EXP_VARIABLE);
							break;
						}
						else
						{
							if (debug)
							{
								show_debug(ses, LIST_VARIABLE, "#MATH EXP: UNKNOWN OPERATOR: %c%c", pti[0], pti[1]);
							}
							pop_call();
							return FALSE;
						}
						break;

					case ')':
						*pta++ = *pti++;
						level--;
						MATH_NODE(FALSE, EXP_PR_LVL, EXP_OPERATOR);
						break;

					case '?':
						*pta++ = *pti++;
						MATH_NODE(FALSE, EXP_PR_TERNARY, EXP_VARIABLE);
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
								if (*pti == '=')
								{
									*pta++ = *pti++;
								}
								MATH_NODE(FALSE, EXP_PR_LOGCOMP, EXP_VARIABLE);
								break;

							default:
								if (debug)
								{
									show_debug(ses, LIST_VARIABLE, "#MATH EXP: UNKNOWN OPERATOR: %c%c", pti[-1], pti[0]);
								}
								pop_call();
								return FALSE;
						}
						break;

					default:
						if (debug)
						{
							show_debug(ses, LIST_VARIABLE, "#MATH EXP: UNKNOWN OPERATOR: %c", *pti);
						}
						pop_call();
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
		pop_call();
		return FALSE;
	}

	if (status != EXP_OPERATOR)
	{
		MATH_NODE(TRUE, EXP_PR_VAR, EXP_OPERATOR);
	}

	pop_call();
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
	int integer64 = 0;
	long double value = 0;
	unsigned long long min = 0, max = 0;
	unsigned long long value64 = 0;

	switch (node->str3[0])
	{
		case '.':
			integer64 = 1;

                        SET_BIT(max, (unsigned int) tintoi(node->next->str3));
                        max = max << 32ULL;
                        SET_BIT(min, (unsigned int) tintoi(node->prev->str3));
                        value64 = max | min;
                        break;

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

		case '?':
			value = tinternary(node->prev->str3, node->next->str3);
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
						value = 0;
						precision = 0;
//						value = tintoi(node->prev->str3);
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
				integer64 = 1;

				value64 = tintou(node->prev->str3) % tintou(node->next->str3);
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
			if (node->str3[1] == '=' && node->str3[2] == '=')
			{
				
				value = tincmp(node->prev->str3, node->next->str3) == 0;
			}
			else
			{
				value = tineval(ses, node->prev->str3, node->next->str3) != 0;
			}
			break;
		case '!':
			if (node->str3[1] == '=' && node->str3[2] == '=')
			{
				value = tincmp(node->prev->str3, node->next->str3) != 0;
			}
			else
			{
				value = tineval(ses, node->prev->str3, node->next->str3) == 0;
			}
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

	if (integer64)
	{
		sprintf(temp, "%llu", value64);
	}
	else
	{
//		sprintf(temp, "%.*Lf", precision, value);
		sprintf(temp, "%Lf", value);
	}
	free(node->str3);
	node->str3 = strdup(temp);
}

long double tinternary(char *arg1, char *arg2)
{
	char *arg3 = strchr(arg2, ':');

	if (arg3 == NULL)
	{
		return 0;
	}
	*arg3++ = 0;

	if (tintoi(arg1))
	{
		return tintoi(arg2);
	}
	else
	{
		return tintoi(arg3);
	}
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
//				show_error(gtd->ses, LIST_COMMAND, "\e[1;31m#WARNING: THE : TIME OPERATOR IN #MATH WILL BE REMOVED IN FUTURE RELEASES.");

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
}

unsigned long long tintou(char *str)
{
	char *ptr = str;
	unsigned long long value = 0, m = 1;

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

	while (ptr != str)
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
				value += (*ptr - '0') * m;
				m *= 10;
				break;

			case '.':
				*ptr = 0;
				break;

			case ':':
				return 0;
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
	}

	switch (str[0])
	{
		case '!':
			return !value;

		case '~':
			return ~value;

		case '+':
			return +value;

		case '-':
			return -value;

		default:
			return value;
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
	long double estimate;

	numdice  = (unsigned long long) tintoi(left);
	sizedice = (unsigned long long) tintoi(right);

	if (sizedice == 0)
	{
		return 0;
	}

	if (numdice > 100)
	{
		estimate = numdice / 100.0;
		numdice = 100;
	}
	else
	{
		estimate = 1;
	}

	for (cnt = sum = 0 ; cnt < numdice ; cnt++)
	{
		sum += generate_rand(ses) % sizedice + 1;
	}

	sum *= estimate;

	return (long double) sum;
}
