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

#define EXP_NUMBER             0
#define EXP_STRING             1
#define EXP_BRACE              2
#define EXP_OPERATOR           3
#define EXP_PARANTHESES        4

#define EXP_PR_CONSTANT        0
#define EXP_PR_DICE            1
#define EXP_PR_INTMUL          2
#define EXP_PR_INTADD          3
#define EXP_PR_BITSHIFT        4
#define EXP_PR_LOGLTGT         5
#define EXP_PR_LOGCOMP         6
#define EXP_PR_BITAND          7
#define EXP_PR_BITXOR          8
#define EXP_PR_BITOR           9
#define EXP_PR_LOGAND         10
#define EXP_PR_LOGXOR         11
#define EXP_PR_LOGOR          12
#define EXP_PR_TERNARY        13
#define EXP_PR_VAR            14
#define EXP_PR_LVL            15


#define EXP_OP_MULTIPLY        '*'
#define EXP_OP_POWER           '*' + 128 * '*'
#define EXP_OP_DIVIDE          '/'
#define EXP_OP_ROOT            '/' + 128 * '/'
#define EXP_OP_MODULO          '%'
#define EXP_OP_DICE            'd'
#define EXP_OP_ADDITION        '+'
#define EXP_OP_SUBTRACTION     '-'
#define EXP_OP_LEFT_SHIFT      '<' + 128 * '<'
#define EXP_OP_RIGHT_SHIFT     '>' + 128 * '>'
#define EXP_OP_ELLIPSIS        '.' + 128 * '.'
#define EXP_OP_GREATER         '>'
#define EXP_OP_GREATER_EQUAL   '>' + 128 * '='
#define EXP_OP_LESSER          '<'
#define EXP_OP_LESSER_EQUAL    '<' + 128 * '='
#define EXP_OP_EQUAL           '=' + 128 * '='
#define EXP_OP_COMPARE         '=' + 128 * '=' + 128 * 128 * '='
#define EXP_OP_NOT_EQUAL       '!' + 128 * '='
#define EXP_OP_NOT_COMPARE     '!' + 128 * '=' + 128 * 128 * '='
#define EXP_OP_AND             '&'
#define EXP_OP_XOR             '^'
#define EXP_OP_OR              '|'
#define EXP_OP_LOGICAL_AND     '&' + 128 * '&'
#define EXP_OP_LOGICAL_XOR     '^' + 128 * '^'
#define EXP_OP_LOGICAL_OR      '|' + 128 * '|'
#define EXP_OP_TERNARY_IF      '?'
#define EXP_OP_TERNARY_ELSE    ':'

struct math_node
{
	struct math_node     * next;
	struct math_node     * prev;
	unsigned short         level;
	unsigned char          priority;
	unsigned char          type;
	long double            val;
	char                 * str3;
};

struct math_node *math_head;
struct math_node *math_tail;
struct math_node *mathnode_s;
struct math_node *mathnode_e;

int precision, wonky;

//long double mathexp(struct session *ses, char *str, char *result, int seed);
void mathexp_level(struct session *ses, struct math_node *node);
void mathexp_compute(struct session *ses, struct math_node *node);
int mathexp_tokenize(struct session *ses, char *str, int seed, int debug);
long double tinternary(struct math_node *left, struct math_node *right);
long double tincmp(struct math_node *left, struct math_node *right);
long double tineval(struct session *ses, struct math_node *left, struct math_node *right);
long double tindice(struct session *ses, struct math_node *left, struct math_node *right);

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

	val = mathexp(ses, str, result, 0);

	DEL_BIT(gtd->flags, TINTIN_FLAG_GETNUMBER);

	return val;
}

unsigned long long get_ulong(struct session *ses, char *str)
{
	unsigned long long val;
	char result[BUFFER_SIZE];

	val = (unsigned long long) mathexp(ses, str, result, 0);

	return val;
}

int get_ellipsis(struct session *ses, unsigned int size, char *name, int *min, int *max)
{
	size_t len;
	unsigned long long range;

	push_call("get_ellipsis(%p,%d,%p,%p,%p)",ses,size,name,min,max);

	len = strlen(name);

	if (name[len - 1] == '.')
	{
		strcpy(name + len, "-1");
	}

	range = get_ulong(ses, name);

	*min = (int) (HAS_BIT(range, 0x00000000FFFFFFFFLL));
	*max = (int) (HAS_BIT(range, 0xFFFFFFFF00000000ULL) >> 32ULL);

	*min = *min > 0 ? *min - 1 : size + *min;
	*max = *max > 0 ? *max - 1 : size + *max;

	*min = URANGE(0, *min, size - 1);
	*max = URANGE(0, *max, size - 1);

	pop_call();
	return 1 + (*min < *max ? *max - *min : *min - *max);
}

long double get_root(long double value, long double power)
{
	long double bot, mid, top, sum;

	bot = 0;
	top = 2;

	while (pow(top, power) < value)
	{
		top *= 2;
	}

	while (top > 0.000000000001)
	{
		mid = top / 2;
		sum = bot + mid;
		
		if (powl(sum, power) <= value)
		{
			bot += mid;
		}
		top -= mid;
	}
	return bot;
}

long double get_double(struct session *ses, char *str)
{
	long double val;
	char result[BUFFER_SIZE];

	val = mathexp(ses, str, result, 1);

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

long double mathexp(struct session *ses, char *str, char *result, int seed)
{
	struct math_node *node;

	substitute(ses, str, result, SUB_VAR|SUB_FUN);

	if (mathexp_tokenize(ses, result, seed, TRUE) == FALSE)
	{
		return 0;
	}

	node = math_head;

	while (node->prev || node->next)
	{
		if (node->next == NULL || node->next->level < node->level)
		{
			mathexp_level(ses, node);

			node = math_head;
		}
		else
		{
			node = node->next;
		}
	}

	if (node->type != EXP_STRING)
	{
		sprintf(result, "%.*Lf", precision, node->val);
	}
	return node->val;
}

#define MATH_NODE(type, priority, newstatus)                    \
{                                                               \
	*pta = 0;                                               \
	add_math_node(type, level, priority, buf3);             \
	status = newstatus;                                     \
	pta = buf3;                                             \
	point = -1;                                             \
}

void add_math_node(int type, int level, int priority, char *arg3)
{
	struct math_node *link;

	link = (struct math_node *) calloc(1, sizeof(struct math_node));

	link->level = level;
	link->priority = priority;
	link->type = type;

	switch (type)
	{
		case EXP_NUMBER:
			if (wonky)
			{
				link->val = --wonky;
			}
			else
			{
				link->val = tintoi(arg3);
			}
			break;

		case EXP_OPERATOR:
			link->val = (int) arg3[0];
			if (arg3[1])
			{
				link->val += (int) 128 * arg3[1];
				if (arg3[2])
				{
					link->val += (int) 128 * 128 * arg3[2];
				}
			}
			break;

//		case EXP_STRING:
//		case EXP_BRACE:
//			link->str3 = strdup(arg3);
//			break;
	}

	link->str3 = strdup(arg3);

	LINK(link, math_head, math_tail);
}

void del_math_node(struct math_node *node)
{
	UNLINK(node, math_head, math_tail);

//	switch (node->type)
//	{
//		case EXP_STRING:
//		case EXP_BRACE:
//			free(node->str3);
//	}
	free(node->str3);
	free(node);
}

int mathexp_tokenize(struct session *ses, char *str, int seed, int debug)
{
	char *buf3, *pti, *pta;
	int level, status, point, nest;

	push_call("mathexp_tokenize(%p,%s,%d,%d)",ses,str,seed,debug);

	buf3 = str_alloc_stack(0);

	nest      = 0;
	level     = 0;
	wonky     = 0;
	point     = -1;
	status    = EXP_NUMBER;
	precision = seed;

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
			case EXP_NUMBER:
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
						if (pta != buf3)
						{
							MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							add_math_node(EXP_NUMBER, level, EXP_PR_VAR, "0");
							add_math_node(EXP_OPERATOR, level, EXP_PR_CONSTANT, "==");

							*pta++ = *pti++;

							pta = buf3;
						}
						break;

					case '~':
						if (pta != buf3)
						{
							MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							add_math_node(EXP_NUMBER, level, EXP_PR_VAR, "-1");
							add_math_node(EXP_OPERATOR, level, EXP_PR_INTADD, "-");

							*pta++ = *pti++;

							pta = buf3;
						}
						break;

					case '+':
						if (pta != buf3)
						{
							MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
//							add_math_node(EXP_NUMBER, level, EXP_PR_VAR, "1");
//							add_math_node(EXP_OPERATOR, level, EXP_PR_INTMUL, "*");
//							*pta++ = *pti++;

							pti++;

							pta = buf3;
						}
						break;

					case '-':
						if (pta != buf3)
						{
							MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							add_math_node(EXP_NUMBER, level, EXP_PR_VAR, "-1");
							add_math_node(EXP_OPERATOR, level, EXP_PR_INTMUL, "*");

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
						pti++;
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
						pti++;
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
							MATH_NODE(EXP_PARANTHESES, EXP_PR_LVL, EXP_NUMBER);
						}
						level++;
						break;

					case ',':
						pti++;
						break;

					case ':':
						if (debug && wonky == 0)
						{
							show_error(gtd->ses, LIST_COMMAND, "\e[1;31m#WARNING: THE : TIME OPERATOR IN #MATH WILL BE REMOVED IN FUTURE RELEASES.");
						}
						*pta++ = *pti++;
						break;
						
					case '.':
						if (pti[1] == '.')
						{
							if (pta == buf3)
							{
								*pta++ = '1';
							}

							MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);

							if (pti[2] == 0)
							{
								*pta++ = *pti++;
								*pta++ = *pti++;

								MATH_NODE(EXP_OPERATOR, EXP_PR_LOGCOMP, EXP_NUMBER);

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
							MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);
							if (*pti == '?')
							{
								wonky = 1;
							}
						}
						else
						{
							*pta++ = *pti++;
							*pta = 0;

							if (debug)
							{
								show_debug(ses, LIST_VARIABLE, "#MATH EXP {%s}: FOUND OPERATOR %s WHILE EXPECTING A VALUE.", str, buf3);
							}
							pop_call();
							return FALSE;
						}
						break;

					case 'K':
					case 'M':
					case 'G':
					case 'T':
//					case 'P':
//					case 'E':
//					case 'Z':
//					case 'Y':
						if (pta == buf3)
						{
							*pta++ = *pti++;
							*pta = 0;

							if (debug)
							{
								show_debug(ses, LIST_VARIABLE, "#MATH EXP {%s}: INVALID NUMBER %s.", str, buf3);
							}

							pop_call();
							return FALSE;
						}
						else
						{
							MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);

							*pta++ = '*';
							MATH_NODE(EXP_OPERATOR, EXP_PR_CONSTANT, EXP_NUMBER);

							switch (*pti++)
							{
								case 'K': pta += sprintf(pta, "1000"); break;
								case 'M': pta += sprintf(pta, "1000000"); break;
								case 'G': pta += sprintf(pta, "1000000000"); break;
								case 'T': pta += sprintf(pta, "1000000000000"); break;
//								case 'P': pta += sprintf(pta, "1000000000000000"); break;
//								case 'E': pta += sprintf(pta, "1000000000000000000"); break;
//								case 'Z': pta += sprintf(pta, "1000000000000000000000"); break;
//								case 'Y': pta += sprintf(pta, "1000000000000000000000000"); break;
							}
							MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);
						}
						break;

					case 'm':
					case 'u':
					case 'n':
					case 'p':
//					case 'f':
//					case 'a':
//					case 'z':
//					case 'y':
						if (pta == buf3)
						{
							*pta++ = *pti++;
							*pta = 0;



							pop_call();
							return FALSE;
						}
						else
						{
							MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);

							*pta++ = '/';

							MATH_NODE(EXP_OPERATOR, EXP_PR_CONSTANT, EXP_NUMBER);

							pta = buf3;

							switch (*pti++)
							{
								case 'm': pta += sprintf(pta, "1000"); break;
								case 'u': pta += sprintf(pta, "1000000"); break;
								case 'n': pta += sprintf(pta, "1000000000"); break;
								case 'p': pta += sprintf(pta, "1000000000000"); break;
//								case 'f': pta += sprintf(pta, "1000000000000000"); break;
//								case 'a': pta += sprintf(pta, "1000000000000000000"); break;
//								case 'z': pta += sprintf(pta, "1000000000000000000000"); break;
//								case 'y': pta += sprintf(pta, "1000000000000000000000000"); break;
							}
							precision = UMAX(precision, pta - buf3 - 1);

							MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);
						}
						break;

					default:
						*pta++ = *pti++;
						*pta = 0;

						if (debug)
						{
							show_debug(ses, LIST_VARIABLE, "#MATH EXP {%s}: INVALID NUMBER %s.", str, buf3);
						}
						pop_call();
						return FALSE;
				}
				break;

			case EXP_STRING:
				switch (*pti)
				{
					case '"':
						if (--nest == 0)
						{
							pti++;
							MATH_NODE(EXP_STRING, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							*pta++ = *pti++;
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
						if (--nest == 0)
						{
							pti++;
							MATH_NODE(EXP_STRING, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							*pta++ = *pti++;
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

							MATH_NODE(EXP_OPERATOR, EXP_PR_LOGCOMP, EXP_NUMBER);
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
						MATH_NODE(EXP_PARANTHESES, EXP_PR_LVL, EXP_OPERATOR);
						break;

					case '?':
						*pta++ = *pti++;
						MATH_NODE(EXP_OPERATOR, EXP_PR_TERNARY, EXP_NUMBER);
						break;

					case 'd':
						*pta++ = *pti++;
						MATH_NODE(EXP_OPERATOR, EXP_PR_DICE, EXP_NUMBER);
						break;

					case '*':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '*':
								*pta++ = *pti++;
								MATH_NODE(EXP_OPERATOR, EXP_PR_INTMUL, EXP_NUMBER);
								break;
							
							default:
								MATH_NODE(EXP_OPERATOR, EXP_PR_INTMUL, EXP_NUMBER);
								break;
						}
						break;
	
					case '/':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '/':
								*pta++ = *pti++;
								MATH_NODE(EXP_OPERATOR, EXP_PR_INTMUL, EXP_NUMBER);
								break;
							default:
								MATH_NODE(EXP_OPERATOR, EXP_PR_INTMUL, EXP_NUMBER);
								break;
						}
						break;

					case '%':
						*pta++ = *pti++;
						MATH_NODE(EXP_OPERATOR, EXP_PR_INTMUL, EXP_NUMBER);
						break;

					case '+':
					case '-':
						*pta++ = *pti++;
						MATH_NODE(EXP_OPERATOR, EXP_PR_INTADD, EXP_NUMBER);
						break;

					case '<':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '<':
								*pta++ = *pti++;
								MATH_NODE(EXP_OPERATOR, EXP_PR_BITSHIFT, EXP_NUMBER);
								break;

							case '=':
								*pta++ = *pti++;
								MATH_NODE(EXP_OPERATOR, EXP_PR_LOGLTGT, EXP_NUMBER);
								break;

							default:
								MATH_NODE(EXP_OPERATOR, EXP_PR_LOGLTGT, EXP_NUMBER);
								break;
						}
						break;

					case '>':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '>':
								*pta++ = *pti++;
								MATH_NODE(EXP_OPERATOR, EXP_PR_BITSHIFT, EXP_NUMBER);
								break;

							case '=':
								*pta++ = *pti++;
								MATH_NODE(EXP_OPERATOR, EXP_PR_LOGLTGT, EXP_NUMBER);
								break;

							default:
								MATH_NODE(EXP_OPERATOR, EXP_PR_LOGLTGT, EXP_NUMBER);
								break;
						}
						break;

					case '&':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '&':
								*pta++ = *pti++;
								MATH_NODE(EXP_OPERATOR, EXP_PR_LOGAND, EXP_NUMBER);
								break;

							default:
								MATH_NODE(EXP_OPERATOR, EXP_PR_BITAND, EXP_NUMBER);
								break;
						}
						break;

					case '^':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '^':
								*pta++ = *pti++;
								MATH_NODE(EXP_OPERATOR, EXP_PR_LOGXOR, EXP_NUMBER);
								break;

							default:
								MATH_NODE(EXP_OPERATOR, EXP_PR_BITXOR, EXP_NUMBER);
								break;

						}
						break;

					case '|':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '|':
								*pta++ = *pti++;
								MATH_NODE(EXP_OPERATOR, EXP_PR_LOGOR, EXP_NUMBER);
								break;

							default:
								MATH_NODE(EXP_OPERATOR, EXP_PR_BITOR, EXP_NUMBER);
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
								MATH_NODE(EXP_OPERATOR, EXP_PR_LOGCOMP, EXP_NUMBER);
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
		if (pta == buf3)
		{
			pop_call();
			return FALSE;
		}
		MATH_NODE(EXP_NUMBER, EXP_PR_VAR, EXP_OPERATOR);
	}

	pop_call();
	return TRUE;
}


void mathexp_level(struct session *ses, struct math_node *node)
{
	int priority, lowest;

	mathnode_e = node;

	while (node->prev)
	{
		if (node->prev->level < node->level)
		{
			break;
		}
		node = node->prev;
	}

	mathnode_s = node;

	for (priority = 0 ; priority < EXP_PR_VAR ; priority = lowest)
	{
		lowest = EXP_PR_VAR;

		for (node = mathnode_s ; node ; node = node->next)
		{
			if (node->priority == priority)
			{
				mathexp_compute(ses, node);
			}
			else if (node->priority < lowest)
			{
				lowest = node->priority;
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
		if (node->prev->priority == EXP_PR_LVL && node->next->priority == EXP_PR_LVL)
		{
			node->level = node->next->level;

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

void mathexp_compute(struct session *ses, struct math_node *node)
{
	int integer64 = 0;
	long double value = 0;
	unsigned long long min = 0, max = 0;
	unsigned long long value64 = 0;

	switch ((int) node->val)
	{
		case EXP_OP_ELLIPSIS:
			integer64 = 1;

                        SET_BIT(max, (unsigned int) node->next->val);
                        max = max << 32ULL;
                        SET_BIT(min, (unsigned int) node->prev->val);
                        value64 = max | min;
                        break;

		case EXP_OP_DICE:
			if (node->next->val <= 0)
			{
				show_debug(ses, LIST_VARIABLE, "#MATHEXP: INVALID DICE: %lld", (long long) node->next->val);
				value = 0;
			}
			else
			{
				value = tindice(ses, node->prev, node->next);
			}
			break;

		case EXP_OP_TERNARY_IF:
			value = tinternary(node->prev, node->next);
			break;

		case EXP_OP_MULTIPLY:
			value = node->prev->val * node->next->val;
			break;

		case EXP_OP_POWER:
			value = pow(node->prev->val, node->next->val);
			break;

		case EXP_OP_DIVIDE:
			if (node->next->val == 0)
			{
				show_debug(ses, LIST_VARIABLE, "#MATH ERROR: DIVISION ZERO.");
				value = 0;
				precision = 0;
			}
			else
			{
				if (precision)
				{
					value = node->prev->val / node->next->val;
				}
				else
				{
					value = (long long) node->prev->val / (long long) node->next->val;
				}
			}
			break;

		case EXP_OP_ROOT:
			if (node->next->val == 2)
			{
				value = sqrt(node->prev->val);
			}
			else
			{
				value = get_root(node->prev->val, node->next->val);
			}
			break;
		case EXP_OP_MODULO:
			value = fmod(node->prev->val, node->next->val);
			break;
		case EXP_OP_ADDITION:
			value = node->prev->val + node->next->val;
			break;
		case EXP_OP_SUBTRACTION:
			value = node->prev->val - node->next->val;
			break;

		case EXP_OP_LESSER:
			value = tincmp(node->prev, node->next) < 0;
			break;

		case EXP_OP_LESSER_EQUAL:
			value = tincmp(node->prev, node->next) <= 0;
			break;

		case EXP_OP_LEFT_SHIFT:
			value = (long long) node->prev->val << (long long) node->next->val;
			break;

		case EXP_OP_GREATER:
			value = tincmp(node->prev, node->next) > 0;
			break;

		case EXP_OP_GREATER_EQUAL:
			value = tincmp(node->prev, node->next) >= 0;
			break;

		case EXP_OP_RIGHT_SHIFT:
			value = (long long) node->prev->val >> (long long) node->next->val;
			break;

		case EXP_OP_AND:
			value = (long long) node->prev->val & (long long) node->next->val;
			break;

		case EXP_OP_LOGICAL_AND:
			value = node->prev->val && node->next->val;
			break;

		case EXP_OP_XOR:
			value = (long long) node->prev->val ^ (long long) node->next->val;
			break;

		case EXP_OP_LOGICAL_XOR:
			value = ((node->prev->val || node->next->val) && (!node->prev->val != !node->next->val));
			break;

		case EXP_OP_OR:
			value = (long long) node->prev->val | (long long) node->next->val;
			break;

		case EXP_OP_LOGICAL_OR:
			value = node->prev->val || node->next->val;
			break;

		case EXP_OP_EQUAL:
			value = tineval(ses, node->prev, node->next) != 0;
			break;
		
		case EXP_OP_COMPARE:
			value = tincmp(node->prev, node->next) == 0;
			break;

		case EXP_OP_NOT_EQUAL:
			value = tineval(ses, node->prev, node->next) == 0;
			break;
		
		case EXP_OP_NOT_COMPARE:
			value = tincmp(node->prev, node->next) != 0;
			break;

		default:
			show_debug(ses, LIST_VARIABLE, "#MATH COMPUTE EXP: UNKNOWN OPERATOR: %c%c%c", (int) node->val % 128, (int) node->val % 16384 / 128, (int) node->val % 2097152 / 16384);
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

	node->priority = EXP_PR_VAR;
	node->type = EXP_NUMBER;

	if (integer64)
	{
		node->val = (long double) value64;
	}
	else
	{
		node->val = value;
	}
}

long double tinternary(struct math_node *left, struct math_node *right)
{
	char *arg3 = strchr(right->str3, ':');

	if (arg3 == NULL)
	{
		return 0;
	}
	*arg3++ = 0;

	if (left->val)
	{
		return tintoi(right->str3);
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
				value = 0;
				m = 1;
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

long double tincmp(struct math_node *left, struct math_node *right)
{
	if (left->type != right->type)
	{
		show_debug(NULL, LIST_VARIABLE, "#MATH COMPUTE: COMPARING STRING WITH A NUMBER.");

		return 0;
	}

	switch (left->type)
	{
		case EXP_STRING:
			return strcmp(left->str3, right->str3);

		default:
			return left->val - right->val;
	}
}

long double tineval(struct session *ses, struct math_node *left, struct math_node *right)
{
	if (left->type != right->type)
	{
		show_debug(ses, LIST_VARIABLE, "#MATH COMPUTE: COMPARING %s WITH %s.", left->type == EXP_NUMBER ? "NUMBER" : "STRING", right->type == EXP_NUMBER ? "NUMBER" : "STRING");

		return 0;
	}

	switch (left->type)
	{
		case EXP_STRING:
			return match(ses, left->str3, right->str3, SUB_NONE);

		default:
			return left->val == right->val;
	}
}

long double tindice(struct session *ses, struct math_node *left, struct math_node *right)
{
	unsigned long long cnt, numdice, sizedice, sum;
	long double estimate;

	numdice  = (unsigned long long) left->val;
	sizedice = (unsigned long long) right->val;

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
