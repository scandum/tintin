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
*                          coded by Bill Reiss 1993                           *
*                     recoded by Igor van den Hoven 2004                      *
******************************************************************************/

#include "tintin.h"

DO_COMMAND(do_variable)
{
	char *str;
	struct listroot *root = ses->list[LIST_VARIABLE];
	struct listnode *node;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_list(root, 0);
	}
	else if (*arg == 0)
	{
		node = search_nest_node(root, arg1);

		if (node)
		{
			if (node->root)
			{
				char *str_result;

				str_result = str_dup("");

				view_nest_node(node, &str_result, 0, 1);

				print_lines(ses, SUB_NONE, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\n{\n" COLOR_STRING "%s" COLOR_BRACE "}" COLOR_RESET "\n", gtd->tintin_char, list_table[LIST_VARIABLE].name, node->arg1, str_result);

				str_free(str_result);
			}
			else
			{
				tintin_printf2(ses, COLOR_TINTIN "%c" COLOR_COMMAND "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}" COLOR_RESET "\n", gtd->tintin_char, list_table[LIST_VARIABLE].name, node->arg1, node->arg2);
			}
		}
		else if (show_node_with_wild(ses, arg1, ses->list[LIST_VARIABLE]) == FALSE)
		{
			show_message(ses, LIST_VARIABLE, "#VARIABLE: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		if (!valid_variable(ses, arg1))
		{
			show_message(ses, LIST_VARIABLE, "#VARIABLE: INVALID VARIALBE NAME {%s}.", arg1);
			return ses;
		}
		str = str_alloc(UMAX(strlen(arg), BUFFER_SIZE));

		arg = sub_arg_in_braces(ses, arg, str, GET_ALL, SUB_VAR|SUB_FUN);

		node = set_nest_node(root, arg1, "%s", str);

		while (*arg)
		{
			arg = sub_arg_in_braces(ses, arg, str, GET_ALL, SUB_VAR|SUB_FUN);

			if (*str)
			{
				add_nest_node(root, arg1, "%s", str);
			}
		}

		show_nest_node(node, &str, 1);

		show_message(ses, LIST_VARIABLE, "#OK. VARIABLE {%s} HAS BEEN SET TO {%s}.", arg1, str);

		str_free(str);
	}
	return ses;
}

DO_COMMAND(do_local)
{
	char *str;
	struct listroot *root;
	struct listnode *node;

	root = local_list(ses);

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_list(root, 0);
	}
	else if (*arg1 && *arg == 0)
	{
		root = search_nest_base_ses(ses, arg1);

		if (root)
		{
			node = search_nest_node_ses(ses, arg1);
		}
		else
		{
			root = local_list(ses);
			node = NULL;
		}

		if (node)
		{
			show_node(root, node, 0);
		}
		else if (show_node_with_wild(ses, arg1, root) == FALSE)
		{
			show_message(ses, LIST_VARIABLE, "#LOCAL: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		str = str_alloc(UMAX(strlen(arg), BUFFER_SIZE));

		arg = sub_arg_in_braces(ses, arg, str, GET_ALL, SUB_VAR|SUB_FUN);

		DEL_BIT(gtd->flags, TINTIN_FLAG_LOCAL);

		node = set_nest_node(root, arg1, "%s", str);

		SET_BIT(gtd->flags, TINTIN_FLAG_LOCAL);

		while (*arg)
		{
			arg = sub_arg_in_braces(ses, arg, str, GET_ALL, SUB_VAR|SUB_FUN);

			if (*str)
			{
				add_nest_node(root, arg1, "%s", str);
			}
		}

		show_nest_node(node, &str, 1);

		show_message(ses, LIST_VARIABLE, "#OK. LOCAL VARIABLE {%s} HAS BEEN SET TO {%s}.", arg1, str);

		str_free(str);
	}
	return ses;
}

DO_COMMAND(do_unvariable)
{
	arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);

	do
	{
		if (delete_nest_node(ses->list[LIST_VARIABLE], arg1))
		{
			show_message(ses, LIST_VARIABLE, "#OK. {%s} IS NO LONGER A VARIABLE.", arg1);
		}
		else
		{
			delete_node_with_wild(ses, LIST_VARIABLE, arg1);
		}
		arg = sub_arg_in_braces(ses, arg, arg1, GET_ALL, SUB_VAR|SUB_FUN);
	}
	while (*arg1);

	return ses;
}

DO_COMMAND(do_cat)
{
	char *str;
	struct listnode *node;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 || *arg == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: CAT {<VARIABLE>} {<ARGUMENT>}");
	}
	else
	{
		str = str_alloc(UMAX(strlen(arg), BUFFER_SIZE));

		if ((node = search_nest_node_ses(ses, arg1)) == NULL)
		{
			arg = sub_arg_in_braces(ses, arg, str, GET_ALL, SUB_VAR|SUB_FUN);

			node = set_nest_node(ses->list[LIST_VARIABLE], arg1, "%s", str);
		}

		while (*arg)
		{
			arg = sub_arg_in_braces(ses, arg, str, GET_ALL, SUB_VAR|SUB_FUN);

			check_all_events(ses, SUB_ARG, 1, 2, "VARIABLE UPDATE %s", arg1, arg1, str);

			if (*str)
			{
				str_cat(&node->arg2, str);
			}
		}

		check_all_events(ses, SUB_ARG, 1, 1, "VARIABLE UPDATED %s", arg1, arg1, str);

		show_message(ses, LIST_VARIABLE, "#CAT: VARIABLE {%s} HAS BEEN SET TO {%s}.", arg1, node->arg2);

		str_free(str);
	}
	return ses;
}

DO_COMMAND(do_replace)
{
	char tmp[BUFFER_SIZE], *pti, *ptm, *str;
	struct listnode *node;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 || *arg2 == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #REPLACE {VARIABLE} {OLD TEXT} {NEW TEXT}");

		return ses;
	}

	if ((node = search_nest_node_ses(ses, arg1)) == NULL)
	{
		show_error(ses, LIST_VARIABLE, "#REPLACE: VARIABLE {%s} NOT FOUND.", arg1);

		return ses;
	}

	if (tintin_regexp(ses, NULL, node->arg2, arg2, 0, REGEX_FLAG_CMD) == FALSE)
	{
		show_message(ses, LIST_VARIABLE, "#REPLACE: {%s} NOT FOUND IN {%s}.", arg2, node->arg2);
	}
	else
	{
		pti = node->arg2;
		str = str_dup("");

		do
		{
			if (*gtd->cmds[0] == 0) // Set by tintin_regexp
			{
				break;
			}

			ptm = strstr(pti, gtd->cmds[0]);

			if (ptm == NULL)
			{
				break;
			}

			*ptm = 0;

			substitute(ses, arg3, tmp, SUB_CMD);

			str_cat_printf(&str, "%s%s", pti, tmp);

			pti = ptm + strlen(gtd->cmds[0]);
		}
		while (tintin_regexp(ses, NULL, pti, arg2, 0, REGEX_FLAG_CMD));

		str_cat(&str, pti);

		str_cpy(&node->arg2, str);

		str_free(str);
	}
	return ses;
}

int valid_variable(struct session *ses, char *arg)
{
	if (*arg == 0)
	{
		return FALSE;
	}

	if (is_math(ses, arg))
	{
		return FALSE;
	}

	return TRUE;
}

/*
	support routines for #format
*/

void stringtobase(char *str, char *base)
{
	char *buf;

	push_call("stringtobase(%p,%p)",str,base);

	buf = strdup(str);

	switch (atoi(base))
	{
		case 64:
			str_to_base64(buf, str, strlen(str));
			break;

		case 252:
			str_to_base252(buf, str, strlen(str));
			break;

		default:
			tintin_printf2(gtd->ses, "#FORMAT: Unknown base conversion {%s}.", base);
			break;
	}
	free(buf);

	pop_call();
	return;
}

void basetostring(char *str, char *base)
{
	char *buf;

	push_call("basetostring(%p,%p)",str,base);

	buf = strdup(str);

	switch (atoi(base))
	{
		case 64:
			base64_to_str(buf, str, strlen(str));
			break;

		case 252:
			base252_to_str(buf, str, strlen(str));
			break;

		default:
			tintin_printf2(gtd->ses, "#FORMAT: Unknown base conversion {%s}.", base);
			break;
	}
	pop_call();
	return;
}

void stringtobasez(char *str, char *base)
{
	char *buf;

	push_call("stringtobase(%p,%p)",str,base);

	buf = strdup(str);

	switch (atoi(base))
	{
		case 64:
			str_to_base64(buf, str, strlen(str));
			break;

		case 252:
			str_to_base252z(buf, str, strlen(str));
			break;

		default:
			tintin_printf2(gtd->ses, "#FORMAT: Unknown base conversion {%s}.", base);
			break;
	}
	free(buf);

	pop_call();
	return;
}

void basetostringz(char *str, char *base)
{
	char *buf;

	push_call("basetostring(%p,%p)",str,base);

	buf = strdup(str);

	switch (atoi(base))
	{
		case 64:
			base64_to_str(buf, str, strlen(str));
			break;

		case 252:
			base252z_to_str(buf, str, strlen(str));
			break;

		default:
			tintin_printf2(gtd->ses, "#FORMAT: Unknown base conversion {%s}.", base);
			break;
	}
	pop_call();
	return;
}

unsigned long long generate_hash_key(char *str)
{
	unsigned long long len, h = 4321;

	for (len = 0 ; *str != 0 ; str++, len++)
	{
		h = ((h << 5) + h) + *str;
	}

	h += len;

	return h;
}

void numbertocharacter(struct session *ses, char *str)
{
	if (get_number(ses, str) < 256)
	{
		sprintf(str, "%c", (int) get_number(ses, str));
	}
	else if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC))
	{
		sprintf(str, "%c%c", (unsigned int) get_number(ses, str) % 256, (unsigned int) get_number(ses, str) / 256);
	}
	else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
	{
		unicode_to_utf8((int) get_number(ses, str), str);
	}
	else
	{
		sprintf(str, "%c", (int) get_number(ses, str));
	}
}

void charactertonumber(struct session *ses, char *str)
{
	int result;

	if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, str))
	{
		if (get_euc_size(ses, str) == 4)
		{
			result = (unsigned char) str[0] + (unsigned char) str[1] * 256 + (unsigned char) str[2] * 256 * 256 + (unsigned char) str[3] * 256 * 256 * 256;
		}
		else
		{
			result = (unsigned char) str[0] + (unsigned char) str[1] * 256;
		}
	}
	else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(str))
	{
		get_utf8_index(str, &result);
	}
	else
	{
		result = (unsigned char) str[0];
	}
	sprintf(str, "%d", result);
}

void charactertohex(struct session *ses, char *str)
{
	if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, str))
	{
		if (get_euc_size(ses, str) == 4)
		{
			sprintf(str, "%u", (unsigned char) str[0] + (unsigned char) str[1] * 256 + (unsigned char) str[2] * 256 * 256 + (unsigned char) str[3] * 256 * 256 * 256);
		}
		else
		{
			sprintf(str, "%u", (unsigned char) str[0] + (unsigned char) str[1] * 256);
		}
	}
	else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(str))
	{
		int result;

		get_utf8_index(str, &result);

		sprintf(str, "%u", result);
	}
	else if (!is_math(ses, str))
	{
		sprintf(str, "%u", (unsigned int) str[0]);
	}
}


void colorstring(struct session *ses, char *str)
{
	char result[BUFFER_SIZE];

	get_color_names(ses, str, result);

	strcpy(str, result);
}

void headerstring(struct session *ses, char *str, char *columns)
{
	char buf[BUFFER_SIZE], fill[BUFFER_SIZE];
	int len, max;

	len = string_raw_str_len(ses, str, 0, BUFFER_SIZE);

	max = *columns ? atoi(columns) : get_scroll_cols(ses);

	if (len > max - 2)
	{
		str[max] = 0;

		return;
	}

	memset(fill, '#', max);

	sprintf(buf, "%.*s%s%.*s%s", (max - len) / 2, fill, str, (max - len) / 2, fill, (max - len) % 2 ? "#" : "");

	strcpy(str, buf);
}

void lowerstring(char *str)
{
	char *pts;

	for (pts = str ; *pts ; pts++)
	{
		*pts = tolower((int) *pts);
	}
}

void upperstring(char *str)
{
	char *pts;

	for (pts = str ; *pts ; pts++)
	{
		*pts = toupper((int) *pts);
	}
}

void hexstring(char *str)
{
	unsigned long long result = hex_number_64bit(str);

	unicode_to_utf8(result, str);
}

void reversestring(char *str)
{
	char t;
	int a = 0, z = strlen(str) - 1;

	while (z > a)
	{
		t = str[z];
		str[z--] = str[a];
		str[a++] = t;
	}

	z = strlen(str) - 1;

	for (a = 1 ; a < z ; a++)
	{
		if (str[a] == '\\' && str[a + 1] != '\\')
		{
			str[a] = str[a - 1];
			str[a - 1] = '\\';
		}
	}

}

void mathstring(struct session *ses, char *str)
{
	get_number_string(ses, str, str);
}

void thousandgroupingstring(struct session *ses, char *str)
{
	char result[BUFFER_SIZE], strold[BUFFER_SIZE];
	int cnt1, cnt2, cnt3, cnt4;

	get_number_string(ses, str, strold);

	cnt1 = strlen(strold);
	cnt2 = BUFFER_SIZE / 2;
	cnt4 = strchr(strold, '.') ? 1 : 0;

	result[cnt2+1] = 0;

	for (cnt3 = 0 ; cnt1 >= 0 ; cnt1--, cnt2--)
	{
		if (cnt3++ % 3 == 0 && cnt3 != 1 && cnt4 == 0 && isdigit((int) strold[cnt1]))
		{
			result[cnt2--] = ',';
		}

		result[cnt2] = strold[cnt1];

		if (!isdigit((int) result[cnt2]))
		{
			cnt4 = 0;
			cnt3 = 0;
			continue;
		}
	}

	strcpy(str, result + cnt2 + 1);
}

void chronosgroupingstring(struct session *ses, char *str)
{
	char *sign = "-";
	long long val = (long long) get_number(ses, str);
	int days, hours, minutes, seconds;

	if (val < 0)
	{
		val *= -1;
	}
	else
	{
		sign = "";
	}

	seconds = val % 60;
	val /= 60;

	minutes = val % 60;
	val /= 60;

	hours = val % 24;
	val /= 24;

	days = val;

	if (days)
	{
		sprintf(str, "%s%d:%02d:%02d:%02d", sign, days, hours, minutes, seconds);
	}
	else if (hours)
	{
		sprintf(str, "%s%d:%02d:%02d", sign, hours, minutes, seconds);
	}
	else
	{
		sprintf(str, "%s%d:%02d", sign, minutes, seconds);
	}
}

void metricgroupingstring(struct session *ses, char *str)
{
	char big[]   = {' ', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y', '?', '?', '?', '?', '?', '?', '?', '?'};
	char small[] = {' ', 'm', 'u', 'n', 'p', 'f', 'a', 'z', 'y', '?', '?', '?', '?', '?', '?', '?', '?'};
	char tmp[NUMBER_SIZE];
	long double val = get_number(ses, str);
	int index = 0;

	if (val >= 1000)
	{
                while (val >= 1000)
                {
                        val = val / 1000;
                        index++;
                }
                if (val >= 100)
                {
                	sprintf(tmp, " %Lf", val);
		}
		else
		{
                	sprintf(tmp, "%Lf", val);
		}
                sprintf(str, "%.4s%c", tmp, big[index]);
	}
	else if (val > 0 && val < 0.01)
	{
		while (val < 0.01)
		{
			val = val * 1000;
			index++;
		}
		sprintf(tmp, "%Lf", val);
		sprintf(str, "%.4s%c", tmp, small[index]);
	}
	else if (val >= 0)
	{
		if (val >= 100)
		{
			sprintf(tmp, " %Lf", val);
		}
		else
		{
			sprintf(tmp, "%Lf", val);
		}
		sprintf(str, "%.4s%c", tmp, big[index]);
	}
	else if (val <= -0.01 && val > -1000)
	{
		if (val <= -100)
		{
			sprintf(tmp, " %Lf", val);
		}
		else
		{
			sprintf(tmp, "%Lf", val);
		} 
		sprintf(str, "%.5s%c", tmp, small[index]);
	}
	else if (val <= -1000)
	{
                while (val <= -100)
                {
                        if (val <= -10000)
                        {
                                val = (long double) ((long long) val / 100LL * 100LL);
                        }
                        val = val / 1000;
                        index++;
                }
                sprintf(tmp, "%Lf", val);
                sprintf(str, "%.5s%c", tmp, big[index]);
	}

	else if (val < 0 /*&& val > -0.01*/)
	{
		while (val > -0.01)
		{
			val = val * 1000;
			index++;
		}
		sprintf(tmp, "%Lf", val);
		sprintf(str, "%.5s%c", tmp, small[index]);
	}
}

void stripspaces(char *str)
{
	int cnt;

	for (cnt = strlen(str) - 1 ; cnt >= 0 ; cnt--)
	{
		if (!isspace((int) str[cnt]))
		{
			break;
		}
		str[cnt] = 0;
	}

	for (cnt = 0 ; str[cnt] != 0 ; cnt++)
	{
		if (!isspace((int) str[cnt]))
		{
			break;
		}
	}
	memmove(str, &str[cnt], strlen(&str[cnt]) + 1);
//	strcpy(str, &str[cnt]);
}

void wrapstring(struct session *ses, char *str, char *wrap)
{
	char  arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	char *pts, *pte, *arg;
	int cnt, width, height;

	push_call("wrapstring(%p,%p,%p)",ses,str,wrap);

	arg = sub_arg_in_braces(ses, str, arg1, GET_ALL, SUB_COL);

	if (*arg == COMMAND_SEPARATOR)
	{
		arg++;
	}

	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg2)
	{
		cnt = get_number(ses, arg2);
	}
	else if (*wrap)
	{
		cnt = atoi(wrap);
	}
	else
	{
		cnt = get_scroll_cols(ses);
	}

	if (cnt <= 0)
	{
		cnt = get_scroll_cols(ses) + cnt;

		if (cnt <= 0)
		{
			show_error(ses, LIST_VARIABLE, "#FORMAT %w: INVALID LENTGH {%s}", arg2);

			pop_call();
			return;
		}
	}

	word_wrap_split(ses, arg1, arg2, cnt, 0, 0, 0, &height, &width);

	pts = pte = arg2;

	str[0] = cnt = 0;

	while (*pte != 0)
	{
		if (*pte == '\n')
		{
			*pte++ = 0;

			cat_sprintf(str, "{%d}{%s}", ++cnt, pts);

			pts = pte;
		}
		else
		{
			pte++;
		}
	}
	cat_sprintf(str, "{%d}{%s}", ++cnt, pts);

	pop_call();
	return;
}

int stringlength(struct session *ses, char *str)
{
	char temp[BUFFER_SIZE];

	substitute(ses, str, temp, SUB_COL|SUB_ESC);

	return strip_vt102_strlen(ses, temp);
}


// stripped range raw return

int string_str_raw_len(struct session *ses, char *str, int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt, tmp_cnt, tot_len, width, col_len;

	raw_cnt = str_cnt = ret_cnt = 0;

	tot_len = strlen(str);

	while (raw_cnt < tot_len)
	{
		if (skip_vt102_codes(&str[raw_cnt]))
		{
			ret_cnt += (str_cnt >= start) ? skip_vt102_codes(&str[raw_cnt]) : 0;
			raw_cnt += skip_vt102_codes(&str[raw_cnt]);

			continue;
		}

		col_len = is_color_code(&str[raw_cnt]);
		
		if (col_len)
		{
			ret_cnt += (str_cnt >= start) ? col_len : 0;
			raw_cnt += col_len;

			continue;
		}

		if (str_cnt >= end)
		{
			break;
		}

		if (str[raw_cnt] == '\\')
		{
			ret_cnt += (str_cnt >= start) ? 1 : 0;
			raw_cnt++;
			
			if (str[raw_cnt] == '\\')
			{
				ret_cnt += (str_cnt >= start) ? 1 : 0;
				raw_cnt++;
				str_cnt++;
			}
			continue;
		}

		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, &str[raw_cnt]))
		{
			tmp_cnt = get_euc_width(ses, &str[raw_cnt], &width);

			if (str_cnt >= start)
			{
				ret_cnt += tmp_cnt;
			}
			raw_cnt += tmp_cnt;
			str_cnt += width;
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&str[raw_cnt]))
		{
			tmp_cnt = get_utf8_width(&str[raw_cnt], &width);

			if (str_cnt >= start)
			{
				ret_cnt += tmp_cnt;
			}
			raw_cnt += tmp_cnt;
			str_cnt += width;
		}
		else
		{
			ret_cnt += (str_cnt >= start) ? 1 : 0;
			raw_cnt++;
			str_cnt++;
		}
	}
	return ret_cnt;
}

// raw range stripped return

int string_raw_str_len(struct session *ses, char *str, int raw_start, int raw_end)
{
	int raw_cnt, ret_cnt, tot_len, width, col_len;

	raw_cnt = raw_start;
	ret_cnt = 0;
	tot_len = strlen(str);

	while (raw_cnt < tot_len)
	{
		if (raw_cnt >= raw_end)
		{
			break;
		}

		if (skip_vt102_codes(&str[raw_cnt]))
		{
			raw_cnt += skip_vt102_codes(&str[raw_cnt]);

			continue;
		}

		col_len = is_color_code(&str[raw_cnt]);
		
		if (col_len)
		{
			raw_cnt += col_len;

			continue;
		}

		if (str[raw_cnt] == '\\')
		{
			raw_cnt++;

			if (valid_escape(ses, &str[raw_cnt]))
			{
				raw_cnt++;
				ret_cnt++;
			}
			else
			{
				ret_cnt++;
			}
			continue;
		}

		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC)  && is_euc_head(ses, &str[raw_cnt]))
		{
			raw_cnt += get_euc_width(ses, &str[raw_cnt], &width);

			ret_cnt += width;
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&str[raw_cnt]))
		{
			raw_cnt += get_utf8_width(&str[raw_cnt], &width);

			ret_cnt += width;
		}
		else
		{
			raw_cnt++;
			ret_cnt++;
		}
	}
	return ret_cnt;
}

void timestring(struct session *ses, char *str)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], *arg;

	struct tm timeval_tm;
	time_t    timeval_t;

	arg = get_arg_in_braces(ses, str, arg1, GET_ALL);

	if (*arg == COMMAND_SEPARATOR)
	{
		arg++;
	}
	arg = get_arg_in_braces(ses, arg, arg2, GET_ALL);

	if (*arg2)
	{
		timeval_t = (time_t) get_number(ses, arg2);
	}
	else
	{
		timeval_t = gtd->time;
	}

	timeval_tm = *localtime(&timeval_t);

	strftime(str, BUFFER_SIZE, arg1, &timeval_tm);
}

void justify_string(struct session *ses, char *in, char *out, int align, int cut)
{
	char temp[BUFFER_SIZE];

	if (align < 0)
	{
		sprintf(temp, "%%%d.%ds", align - ((int) strlen(in) - string_raw_str_len(ses, in, 0, BUFFER_SIZE)), string_str_raw_len(ses, in, 0, cut));
	}
	else
	{
		sprintf(temp, "%%%d.%ds", align + ((int) strlen(in) - string_raw_str_len(ses, in, 0, BUFFER_SIZE)), string_str_raw_len(ses, in, 0, cut));
	}

	sprintf(out, temp, in);
}

void format_string(struct session *ses, char *format, char *arg, char *out)
{
	char *arglist[30];
	char argformat[BUFFER_SIZE], newformat[BUFFER_SIZE], *ptf, *ptt, *pts, *ptn;
	struct tm timeval_tm;
	time_t    timeval_t;
	int i, max;

	push_call("format_string(%p,%p,%p,%p)",ses,format,arg,out);

	for (max = 0 ; max < 4 ; max++)
	{
		arglist[max] = str_alloc_stack();

		arg = sub_arg_in_braces(ses, arg, arglist[max], GET_ONE, SUB_VAR|SUB_FUN);
	}

	for (max = 4 ; *arg && max < 30 ; max++)
	{
		arglist[max] = str_alloc_stack();

		arg = sub_arg_in_braces(ses, arg, arglist[max], GET_ONE, SUB_VAR|SUB_FUN);
	}

	i = 0;

	ptf = format;
	ptn = newformat;

	while (*ptf)
	{
		if (*ptf == '%')
		{
			if (i >= max)
			{
				*ptn++ = *ptf++;
				*ptn++ = '%';
				i++;
				continue;
			}
			pts = ptn;

			*ptn++ = *ptf++;

			if (*ptf == 0)
			{
				*ptn++ = '%';
				break;
			}
			else if (*ptf == '%')
			{
				*ptn++ = *ptf++;
			}
			else if (*ptf == ' ')
			{
				*ptn++ = '%';
			}
			else
			{
				while (!isalpha((int) *ptf))
				{
					if (*ptf == 0)
					{
						break;
					}
					*ptn++ = *ptf++;
				}

				*ptn = 0;

				switch (*ptf)
				{
					case 0:
						show_error(ses, LIST_VARIABLE, "#FORMAT STRING: UNKNOWN ARGUMENT {%s}.", pts);
						continue;

					case 'd':
					case 'f':
					case 'X':
						strcpy(argformat, pts);

						ptn = pts + 1;
						*ptn = 0;
						break;

					case 'b':
					case 'B':
					case 'h':
					case 'w':
					case 'z':
					case 'Z':
						strcpy(argformat, pts+1);
						ptn = pts + 1;
						*ptn = 0;
						break;

					default:
						if (pts[1])
						{
							char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

							ptt = arg1;
							ptn = pts + 1;

							while (*ptn && *ptn != '.')
							{
								*ptt++ = *ptn++;
							}

							*ptt = 0;

							if (*ptn == 0)
							{
								if (atoi(arg1) < 0)
								{
									sprintf(argformat, "%%%d", atoi(arg1) - ((int) strlen(arglist[i]) - string_raw_str_len(ses, arglist[i], 0, BUFFER_SIZE)));
								}
								else
								{
									sprintf(argformat, "%%%d", atoi(arg1) + ((int) strlen(arglist[i]) - string_raw_str_len(ses, arglist[i], 0, BUFFER_SIZE)));
								}
							}
							else
							{
								ptt = arg2;
								ptn = ptn + 1;
		
								while (*ptn)
								{
									*ptt++ = *ptn++;
								}
		
								*ptt = 0;

								if (atoi(arg1) < 0)
								{
									sprintf(argformat, "%%%d.%d",
										atoi(arg1) - ((int) strlen(arglist[i]) - string_raw_str_len(ses, arglist[i], 0, BUFFER_SIZE)),
										string_str_raw_len(ses, arglist[i], 0, atoi(arg2)));
								}
								else
								{
									sprintf(argformat, "%%%d.%d",
										atoi(arg1) + ((int) strlen(arglist[i]) - string_raw_str_len(ses, arglist[i], 0, BUFFER_SIZE)),
										string_str_raw_len(ses, arglist[i], 0, atoi(arg2)));
								}
							}
		
							ptt = argformat;
							ptn = pts;
		
							while (*ptt)
							{
								*ptn++ = *ptt++;
							}
		
							*ptn = 0;
						}
				}

				switch (*ptf)
				{
					case 'a':
						numbertocharacter(ses, arglist[i]);
//						sprintf(arglist[i], "%c", (char) get_number(ses, arglist[i]));
						break;

					case 'b':
						substitute(ses, arglist[i], arglist[i], SUB_VAR|SUB_FUN);
						stringtobase(arglist[i], argformat);
						break;

					case 'c':
						colorstring(ses, arglist[i]);
						break;

					case 'd':
						strcat(argformat, "lld");
						sprintf(arglist[i], argformat, (long long) get_number(ses, arglist[i]));
						break;

					case 'f':
						strcat(argformat, "Lf");
						sprintf(arglist[i], argformat, get_double(ses, arglist[i]));
						break;

					case 'g':
						thousandgroupingstring(ses, arglist[i]);
						break;

					case 'h':
						substitute(ses, arglist[i], arglist[i], SUB_VAR|SUB_FUN);
						headerstring(ses, arglist[i], argformat);
						break;

					case 'l':
						lowerstring(arglist[i]);
						break;

					case 'm':
						mathstring(ses, arglist[i]);
						break;

					case 'n':
						arglist[i][0] = toupper((int) arglist[i][0]);
						break;

					case 'p':
						stripspaces(arglist[i]);
						break;

					case 'r':
						reversestring(arglist[i]);
						break;

					case 's':
						break;

					case 't':
						timestring(ses, arglist[i]);
						break;

					case 'u':
						upperstring(arglist[i]);
						break;

					case 'w':
						substitute(ses, arglist[i], arglist[i], SUB_VAR|SUB_FUN);
						wrapstring(ses, arglist[i], argformat);
						break;

					case 'x':
						hexstring(arglist[i]);
						break;

					case 'z':
						substitute(ses, arglist[i], arglist[i], SUB_VAR|SUB_FUN);
						stringtobasez(arglist[i], argformat);
						break;

					case 'A':
						charactertonumber(ses, arglist[i]);
						break;

					case 'B':
						substitute(ses, arglist[i], arglist[i], SUB_VAR|SUB_FUN);
						basetostring(arglist[i], argformat);
						break;

					case 'C':
						chronosgroupingstring(ses, arglist[i]);
						break;

					case 'D':
						sprintf(arglist[i], "%llu", hex_number_64bit(arglist[i]));

						break;

					case 'G':
						thousandgroupingstring(ses, arglist[i]);
						break;

					case 'H':
						sprintf(arglist[i], "%llu", generate_hash_key(arglist[i]));
						break;

					case 'L':
						sprintf(arglist[i], "%d", stringlength(ses, arglist[i]));
						break;

					case 'M':
						metricgroupingstring(ses, arglist[i]);
						break;

					case 'R':
						tintin_printf2(ses, "\e[1;31m#echo/#format %%R please use #screen {get} {rows} to get screen height.");
						sprintf(arglist[i], "%d", gtd->screen->rows);
						break;

					case 'S':
						sprintf(arglist[i], "%d", spellcheck_count(ses, arglist[i]));
						break;

					case 'T':
						sprintf(arglist[i], "%ld", gtd->time);
						break;

					case 'U':
						sprintf(arglist[i], "%lld", utime());
						break;

					case 'X':
						strcat(argformat, "llX");
						charactertohex(ses, arglist[i]);
						sprintf(arglist[i], argformat, (unsigned long long) get_number(ses, arglist[i]));
						break;

					// undocumented
					case 'Y': // print the year, experimental
						timeval_t  = (time_t) *arglist[i] ? atoll(arglist[i]) : gtd->time;
						timeval_tm = *localtime(&timeval_t);
						strftime(arglist[i], BUFFER_SIZE, "%Y", &timeval_tm);
						break;

					case 'Z':
						substitute(ses, arglist[i], arglist[i], SUB_VAR|SUB_FUN);
						basetostringz(arglist[i], argformat);
						break;

					default:
						show_error(ses, LIST_VARIABLE, "#FORMAT STRING: UNKNOWN ARGUMENT {%s%c}.", pts, *ptf);
						break;
				}
				*ptn++ = 's';
				i++;
				ptf++;
			}
		}
		else
		{
			*ptn++ = *ptf++;
		}
	}
	*ptn = 0;

	snprintf(out, BUFFER_SIZE - 1, newformat, arglist[0], arglist[1], arglist[2], arglist[3], arglist[4], arglist[5], arglist[6], arglist[7], arglist[8], arglist[9], arglist[10], arglist[11], arglist[12], arglist[13], arglist[14], arglist[15], arglist[16], arglist[17], arglist[18], arglist[19], arglist[20], arglist[21], arglist[22], arglist[23], arglist[24], arglist[25], arglist[26], arglist[27], arglist[28], arglist[29]);

	pop_call();
	return;
}

DO_COMMAND(do_format)
{
	char destvar[BUFFER_SIZE], format[BUFFER_SIZE], result[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, destvar,  GET_NST, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, format,   GET_ONE, SUB_VAR|SUB_FUN);

	if (*destvar == 0)
	{
		show_error(ses, LIST_VARIABLE, "#SYNTAX: #format {variable} {format} {arg1} {arg2}");

		return ses;
	}

	format_string(ses, format, arg, result);

	set_nest_node_ses(ses, destvar, "%s", result);

	show_message(ses, LIST_VARIABLE, "#OK. VARIABLE {%s} HAS BEEN SET TO {%s}.", destvar, result);

	return ses;
}
