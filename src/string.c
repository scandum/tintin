/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright (C) 2004-2020 Igor van den Hoven                                *
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
*                      coded by Igor van den Hoven 2020                       *
******************************************************************************/

#include "tintin.h"

int get_raw_off_str_range_raw_width(struct session *ses, char *str, int start, int end, int *raw_width)
{
	int raw_cnt, str_cnt, raw_off, ret_raw, raw_len, skip, width;

	raw_off = 0;
	raw_cnt = 0;
	str_cnt = 0;
	ret_raw = 0;

	raw_len = str_len(str);

	while (raw_cnt < raw_len)
	{
		skip = get_vt102_width(ses, &str[raw_cnt], &width);

		if (str_cnt >= start)
		{
			ret_raw += skip;
		}
		else
		{
			raw_off += skip;
		}

		raw_cnt += skip;

		if (end >= 0 && str_cnt + width > end)
		{
			break;
		}
		str_cnt += width;
	}
	*raw_width = ret_raw;

	return raw_off;
}

int str_len_str(struct session *ses, char *str, int start, int end)
{
	int str_cnt, ret_cnt, width, tmp_cnt;

	str_cnt = 0;
	ret_cnt = 0;

	while (*str)
	{
		if (end >= 0 && str_cnt >= end)
		{
			break;
		}

		tmp_cnt = skip_vt102_codes(str);

		if (tmp_cnt)
		{
			str += tmp_cnt;
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(str))
		{
			tmp_cnt = get_utf8_width(str, &width);

			if (str_cnt >= start)
			{
				ret_cnt += width;
			}
			str_cnt += width;

			str += tmp_cnt;
		}
		else
		{
			if (str_cnt >= start)
			{
				ret_cnt++;
			}
			str_cnt++;

			str++;
		}
	}
	return ret_cnt;
}

int str_len_raw(struct session *ses, char *str, int start, int end)
{
	int raw_cnt, ret_cnt, width, raw_len, tmp_cnt;

	raw_cnt = start;
	ret_cnt = 0;

	raw_len = str_len(str);

	while (raw_cnt < raw_len)
	{
		if (raw_cnt >= end)
		{
			break;
		}

		tmp_cnt = skip_vt102_codes(&str[raw_cnt]);

		if (tmp_cnt)
		{
			// can go past end, but shouldn't be an issue
			raw_cnt += tmp_cnt;
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

int raw_len_str(struct session *ses, char *str, int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt, width, tmp_cnt, raw_len;

	raw_cnt = 0;
	str_cnt = 0;
	ret_cnt = 0;
	raw_len = strlen(str);

	while (raw_cnt < raw_len)
	{
		tmp_cnt = skip_vt102_codes(&str[raw_cnt]);

		if (tmp_cnt)
		{
			raw_cnt += tmp_cnt;

			if (str_cnt >= start)
			{
				ret_cnt += tmp_cnt;
			}
			continue;
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&str[raw_cnt]))
		{ 
			tmp_cnt = get_utf8_width(&str[raw_cnt], &width);

			if (str_cnt >= start)
			{
				ret_cnt += tmp_cnt;
			}
			raw_cnt += tmp_cnt;
		}
		else
		{
			if (str_cnt >= start)
			{
				ret_cnt++;
			}
			raw_cnt++;
			width = 1;
		}

		if (end >= 0 && str_cnt + width > end)
		{
			break;
		}
		str_cnt += width;
	}
	return ret_cnt;
}

// minimum string length

int raw_len_str_min(struct session *ses, char *str, int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt, width, tmp_cnt, raw_len;

	raw_cnt = 0;
	str_cnt = 0;
	ret_cnt = 0;
	raw_len = strlen(str);

	while (raw_cnt < raw_len)
	{
		if (str_cnt >= end)
		{
			break;
		}

		tmp_cnt = skip_vt102_codes(&str[raw_cnt]);

		if (tmp_cnt)
		{
			raw_cnt += tmp_cnt;

			if (str_cnt >= start)
			{
				ret_cnt += tmp_cnt;
			}
			continue;
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&str[raw_cnt]))
		{    
			tmp_cnt = get_utf8_width(&str[raw_cnt], &width);

			if (str_cnt >= start)
			{
				ret_cnt += tmp_cnt;
			}
			raw_cnt += tmp_cnt;
		}
		else
		{
			if (str_cnt >= start)
			{
				ret_cnt++;
			}
			raw_cnt++;
			width = 1;
		}
		if (end >= 0 && str_cnt + width > end)
		{
			break;
		}
		str_cnt += width;
	}
	return ret_cnt;
}

int raw_len_raw(struct session *ses, char *str, int start, int end)
{
	if (end == -1)
	{
		return strlen(str) - start;
	}

	if (start >= end)
	{
		return 0;
	}

	return end - start;
}

char *str_ins_str(struct session *ses, char **str, char *ins, int str_start, int str_end)
{
	char old[COLOR_SIZE], tmp;
	int len, raw_start, raw_end, raw_len, ins_raw_len, col_len;

	if (str_end == -1)
	{
		str_end = str_start + strip_vt102_strlen(ses, ins);
	}

	len = str_len_str(ses, *str, 0, str_end);

	if (len < str_end)
	{
		str_cat_printf(str, "%*s", str_end - len, "");
	}

	ins_raw_len = raw_len_str(ses, ins, 0, str_end - str_start);

	raw_start = raw_len_str_min(ses, *str, 0, str_start);
	raw_len   = str_len(*str);
	raw_end   = raw_len_str_min(ses, *str, 0, str_end);

	tmp = (*str)[raw_end];

	*old = (*str)[raw_end] = 0;

	get_color_codes(old, *str, old, GET_ALL);

	(*str)[raw_end] = tmp;

	col_len = strlen(old);

	str_resize(str, ins_raw_len + col_len + 1);

	if (raw_len < raw_end + ins_raw_len || raw_len > raw_end)
	{
		memmove(*str + raw_start + ins_raw_len + col_len, *str + raw_end, raw_len - raw_end + 1);

		memcpy(*str + raw_start + ins_raw_len, old, col_len);

		memcpy(*str + raw_start, ins, ins_raw_len);
	}
	else if (raw_len > raw_end)
	{
		memmove(*str + raw_start + ins_raw_len, *str + raw_end, raw_len - raw_end + 1);

		memcpy(*str + raw_start, ins, ins_raw_len);
	}
	else
	{
		memcpy(*str + raw_start, ins, ins_raw_len);

		if (len < str_end)
		{
			(*str)[raw_start + ins_raw_len] = 0;
		}
	}
	str_fix(*str);

	return *str;
}

char *calign(struct session *ses, char *in, char *out, int width)
{
	int width_in;

	in = space_out(in);

	if (*in)
	{
		int len = strlen(in) - 1;

		while (is_space(in[len]))
		{
			in[len--] = 0;
		}
	}

	strip_vt102_width(ses, in, &width_in);

	width = UMAX(0, width - width_in);

	sprintf(out, "%*s%s%*s", width / 2, "", in, width - width / 2, "");

	return out;
}

char *lalign(struct session *ses, char *in, char *out, int width)
{
	int width_in;

	in = space_out(in);

	if (*in)
	{
		int len = strlen(in) - 1;

		while (is_space(in[len]))
		{
			in[len--] = 0;
		}
	}

	strip_vt102_width(ses, in, &width_in);

	width = UMAX(0, width - width_in);

	sprintf(out, "%s%*s", in, width, "");

	return out;
}

char *ralign(struct session *ses, char *in, char *out, int width)
{
	int width_in;

	in = space_out(in);

	if (*in)
	{
		int len = strlen(in) - 1;

		while (is_space(in[len]))
		{
			in[len--] = 0;
		}
	}

	strip_vt102_width(ses, in, &width_in);

	width = UMAX(0, width - width_in);

	sprintf(out, "%*s%s", width, "", in);

	return out;
}


char *ualign(struct session *ses, char *in, char *out, int width)
{
	char *pti, *pto;

	pti = in;
	pto = out;

	while (*pti)
	{
		if (*pti == '\n')
		{
			switch (pti[1])
			{
				case '\0':
					*pto++ = *pti++;
					break;

				case '\n':
					while (*pti == '\n')
					{
						*pto++ = *pti++;
					}
					break;

				default:
					pti++;
					*pto++ = ' ';
					break;
			}
		}
		else
		{
			*pto++ = *pti++;
		}
	}
	*pto = 0;

	return out;
}

// unused

char char_cmp(char left, char right)
{
	return left / 64 == right / 64 && left % 32 == right % 32;
}

char is_alnum(char input)
{
	return HAS_BIT(character_table[(unsigned char) input], CHAR_FLAG_ALPHA|CHAR_FLAG_DIGIT);
}

char is_alpha(char input)
{
	return HAS_BIT(character_table[(unsigned char) input], CHAR_FLAG_ALPHA);
}

char is_digit(char input)
{
	return HAS_BIT(character_table[(unsigned char) input], CHAR_FLAG_DIGIT);
}

char is_hex(char input)
{
	return HAS_BIT(character_table[(unsigned char) input], CHAR_FLAG_HEX);
}

char is_space(char input)
{
	return HAS_BIT(character_table[(unsigned char) input], CHAR_FLAG_SPACE);
}

char is_varchar(char input)
{
	return HAS_BIT(character_table[(unsigned char) input], CHAR_FLAG_VAR);
}

char is_csichar(char input)
{
	return HAS_BIT(character_table[(unsigned char) input], CHAR_FLAG_CSI);
}

char is_print(char input)
{
	return HAS_BIT(character_table[(unsigned char) input], CHAR_FLAG_PRINT);
}
