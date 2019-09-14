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


void print_line(struct session *ses, char **str, int prompt)
{
	char *out;

	push_call("print_line(%p,%p,%d)",ses,*str,prompt);

	if (ses->scroll->line != -1 && HAS_BIT(ses->flags, SES_FLAG_SCROLLLOCK))
	{
		pop_call();
		return;
	}

	if (HAS_BIT(ses->flags, SES_FLAG_SCAN) && gtd->verbose_level == 0)
	{
		pop_call();
		return;
	}

	out = str_alloc(100 + strlen(*str) * 2 + 2);

	if (HAS_BIT(ses->flags, SES_FLAG_CONVERTMETA))
	{
		convert_meta(*str, out, TRUE);

		str_cpy(str, out);
	}

	if (HAS_BIT(ses->flags, SES_FLAG_WORDWRAP))
	{
		word_wrap(ses, *str, out, TRUE);
	}
	else
	{
		strcpy(out, *str);
	}

	if (prompt)
	{
		printf("%s", out);
	}
	else
	{
		printf("%s\n", out);
	}
	add_line_screen(out);

	str_free(out);

	pop_call();
	return;
}

/*
	Word wrapper, only wraps scrolling region
*/

int word_wrap(struct session *ses, char *textin, char *textout, int display)
{
	char *pti, *pto, *lis, *los, *chi, *cho;
	int width, size, skip = 0, cnt = 0;

	push_call("word_wrap(%s,%p,%p)",ses->name, textin,textout);

	pti = chi = lis = textin;
	pto = cho = los = textout;

	ses->cur_col = 1;

	while (*pti != 0)
	{
		if (skip_vt102_codes(pti))
		{
			if (display)
			{
				interpret_vt102_codes(ses, pti, TRUE);
			}

			for (skip = skip_vt102_codes(pti) ; skip > 0 ; skip--)
			{
				*pto++ = *pti++;
			}
			continue;
		}

		if (*pti == '\n')
		{
			*pto++ = *pti++;
			cnt = cnt + 1;
			los = pto;
			lis = pti;

			ses->cur_col = 1;

			continue;
		}

		if (*pti == ' ')
		{
			los = pto;
			lis = pti;
		}

		if (ses->cur_col > gtd->screen->cols)
		{
			cnt++;
			ses->cur_col = 1;

			if (HAS_BIT(ses->flags, SES_FLAG_WORDWRAP))
			{
				if (pto - los > 15 || !SCROLL(ses))
				{
					*pto++ = '\n';
					los = pto;
					lis = pti;
				}
				else if (lis != chi) // infinite VT loop detection
				{
					pto = los;
					*pto++ = '\n';
					pti = chi = lis;
					pti++;
				}
				else if (los != cho)
				{
					pto = cho = los;
					pto++;
					pti = chi = lis;
					pti++;
				}
			}
		}
		else
		{
			if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
			{
				*pto++ = *pti++;
				*pto++ = *pti++;

				ses->cur_col++;
			}
			else if (HAS_BIT(ses->flags, SES_FLAG_UTF8) && is_utf8_head(pti))
			{
				size = get_utf8_width(pti, &width);

				while (size--)
				{
					*pto++ = *pti++;
				}
				ses->cur_col += width;
			}
			else
			{
				*pto++ = *pti++;
				ses->cur_col++;
			}
		}
	}
	*pto = 0;

	pop_call();
	return (cnt + 1);
}

// store whatever falls inbetween skip and keep. Used by #buffer

int word_wrap_split(struct session *ses, char *textin, char *textout, int display, int start, int end)
{
	char *pti, *pto, *lis, *los, *chi, *cho, *ptb;
	int width, size, i = 0, cnt = 0;

	push_call("word_wrap_split(%s,%p,%p,%d,%d)",ses->name, textin,textout, start, end);

	pti = chi = lis = textin;
	pto = cho = los = textout;

	ses->cur_col = 1;

	while (*pti != 0)
	{
		if (skip_vt102_codes(pti))
		{
//			if (cnt >= start && cnt < end)
			{
				for (i = skip_vt102_codes(pti) ; i > 0 ; i--)
				{
					*pto++ = *pti++;
				}
			}
/*			else
			{
				pti += skip_vt102_codes(pti);
			}*/
			continue;
		}

		if (*pti == '\n')
		{
			if (cnt++ >= start && cnt < end)
			{
				*pto++ = *pti++;
			}
			else
			{
				pti++;
			}
			los = pto;
			lis = pti;

			ses->cur_col = 1;

			continue;
		}

		if (*pti == ' ')
		{
			los = pto;
			lis = pti;
		}

		if (ses->cur_col > gtd->screen->cols)
		{
			cnt++;
			ses->cur_col = 1;

			if (HAS_BIT(ses->flags, SES_FLAG_WORDWRAP))
			{
				if (pto - los > 15 || !SCROLL(ses))
				{
					if (cnt >= start && cnt < end)
					{
						*pto++ = '\n';
					}
					los = pto;
					lis = pti;
				}
				else if (lis != chi) // infinite VT loop detection
				{
					if (cnt >= start && cnt < end)
					{
						pto = los;
						*pto++ = '\n';
					}
					pti = chi = lis;
					pti++;
				}
				else if (los != cho)
				{
					if (cnt >= start && cnt < end)
					{
						pto = cho = los;
						pto++;
					}
					else
					{
						cho = los;
					}
					pti = chi = lis;
					pti++;
				}
			}
		}
		else
		{
			ptb = pto;

			if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && is_big5(pti))
			{
				*pto++ = *pti++;
				*pto++ = *pti++;
				ses->cur_col++;
			}
			else if (HAS_BIT(ses->flags, SES_FLAG_UTF8) && is_utf8_head(pti))
			{
				size = get_utf8_width(pti, &width);

				while (size--)
				{
					*pto++ = *pti++;
				}
				ses->cur_col += width;
			}
			else
			{
				*pto++ = *pti++;
				ses->cur_col++;
			}

			if (cnt < start || cnt >= end)
			{
				pto = ptb;
			}
		}
	}
	*pto = 0;

	pop_call();
	return (cnt + 1);
}

