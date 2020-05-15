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

int str_len_str(struct session *ses, char *str, int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt, width, raw_len, tmp_cnt;

	raw_cnt = 0;
	str_cnt = 0;
	ret_cnt = 0;

	raw_len = str_len(str);

	while (raw_cnt < raw_len)
	{
		if (end >= 0 && str_cnt >= end)
		{
			break;
		}

		tmp_cnt = skip_vt102_codes(&str[raw_cnt]);

		if (tmp_cnt)
		{
			raw_cnt += tmp_cnt;
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(&str[raw_cnt]))
		{
			raw_cnt += get_utf8_width(&str[raw_cnt], &width);

			if (str_cnt >= start)
			{
				ret_cnt += width;
			}
			str_cnt += width;
		}
		else
		{
			if (str_cnt >= start)
			{
				ret_cnt++;
			}
			raw_cnt++;
			str_cnt++;
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
			width = 0;
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
	if (start >= end)
	{
		return 0;
	}
	return end - start;
}

char *str_ins_str(struct session *ses, char **str, char *ins, int str_start, int str_end)
{
	int len, raw_start, raw_end, raw_len, ins_raw_len;

	len = str_len_str(ses, *str, 0, str_end);

	if (len < str_end)
	{
		str_cat_printf(str, "%*s", str_end - len, "");
	}

	ins_raw_len = raw_len_str(ses, ins, 0, str_end - str_start);

	raw_start = raw_len_str(ses, *str, 0, str_start);
	raw_len   = raw_len_str(ses, *str, 0, -1);
	raw_end   = raw_len_str(ses, *str, 0, str_end);

	if (raw_len < raw_end + ins_raw_len)
	{
		str_resize(str, ins_raw_len);

		memmove(*str + raw_start + ins_raw_len, *str + raw_end, raw_len - raw_end + 1);

		memcpy(*str + raw_start, ins, ins_raw_len);
	}
	else
	{
		memcpy(*str + raw_start, ins, ins_raw_len);

		(*str)[raw_start + ins_raw_len] = 0;
	}
	str_fix(*str);

	return *str;
}
