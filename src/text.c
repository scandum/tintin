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


void print_line(struct session *ses, char **str, int prompt)
{
	int height, width;
	char *out;

	push_call("print_line(%p,%p,%d)",ses,*str,prompt);

	if (ses->scroll->line != -1 && HAS_BIT(ses->config_flags, CONFIG_FLAG_SCROLLLOCK))
	{
		pop_call();
		return;
	}

	if (gtd->level->scan && gtd->level->verbose == 0)
	{
		pop_call();
		return;
	}

	if (HAS_BIT(ses->flags, SES_FLAG_SPLIT) && ses->wrap != gtd->screen->cols)
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_SESSIONUPDATE);
		SET_BIT(ses->flags, SES_FLAG_PRINTLINE);

		pop_call();
		return;
	}

	out = str_alloc_stack(strlen(*str) * 2);

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_CONVERTMETA) || gtd->level->convert)
	{
		convert_meta(*str, out, TRUE);

		str_cpy(str, out);
	}

	if (HAS_BIT(ses->flags, SES_FLAG_SPLIT) || HAS_BIT(ses->config_flags, CONFIG_FLAG_WORDWRAP))
	{
		word_wrap(ses, *str, out, TRUE, &height, &width);
	}
	else
	{
		str_cpy(&out, *str);
	}

	if (prompt)
	{
		print_stdout(0, 0, "%s", out);
	}
	else
	{
		print_stdout(0, 0, "%s\n", out);
	}

//	add_line_screen(out);

	pop_call();
	return;
}

void print_stdout(int row, int col, char *format, ...)
{
	char *buffer;
	va_list args;
	int len;

	va_start(args, format);
	len = vasprintf(&buffer, format, args);
	va_end(args);

	if (gtd->detach_port)
	{
		if (gtd->detach_sock)
		{
			if (write(gtd->detach_sock, buffer, len) == -1)
			{
				printf("error: print_stdout: write:\n");
			}
		}
	}
	else
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_DISPLAYUPDATE);

		fputs(buffer, stdout);
//		printf("%s", buffer);

		if (row && col)
		{
			set_line_screen(gtd->ses, buffer, row, col);
		}
	}
	free(buffer);
}

/*
	Word wrapper, only wraps scrolling region
*/

int word_wrap(struct session *ses, char *textin, char *textout, int flags, int *height, int *width)
{
	char color[COLOR_SIZE];
	char *pti, *pto, *lis, *los, *chi, *cho;
	int cur_height, cur_width, size, i, skip, lines, cur_col, tab, wrap, cur_space;

	push_call("word_wrap(%s,%p,%p)",ses->name,textin,textout);

	pti = chi = lis = textin;
	pto = cho = los = textout;

	*color       = 0;
	cur_height   = 0;
	lines        = 0;
	*height      = 0;

	cur_col      = ses->cur_col;
	ses->cur_col = 1;
	cur_space    = 1;

	cur_width    = 0;
	*width       = 0;

	skip         = 0;

	wrap = get_scroll_cols(ses);

	while (*pti && pto - textout < BUFFER_SIZE)
	{
		skip = skip_vt102_codes(pti);

		if (skip)
		{
			if (ses->color)
			{
				get_color_codes(color, pti, color, GET_ONE);

				if (HAS_BIT(flags, WRAP_FLAG_DISPLAY))
				{
					interpret_vt102_codes(ses, pti, TRUE);
				}

				for (i = 0 ; i < skip ; i++)
				{
					*pto++ = *pti++;
				}
			}
			else
			{
				pti += skip;
			}
			continue;
		}

		if (*pti == '\n')
		{
			lines++;
			cur_height++;

			*pto++ = *pti++;

			lis = pti;
			los = pto;

			if (*pti)
			{
				pto += sprintf(pto, "%s", color);
			}

			if (cur_width > *width)
			{
				*width = cur_width;
			}

			cur_width    = 0;
			ses->cur_col = 1;
			cur_space    = 1;

			continue;
		}

		if (*pti == ' ' || *pti == '\t')
		{
			cur_space = ses->cur_col;
			los = pto;
			lis = pti;
		}

		size = get_vt102_width(ses, pti, &tab);

		if (ses->cur_col > 1 && ses->cur_col + tab > wrap + 1)
		{
			cur_height++;

			if (HAS_BIT(ses->config_flags, CONFIG_FLAG_WORDWRAP))
			{
				if (ses->cur_col - cur_space >= 15 || wrap <= 20 || !SCROLL(ses))
				{
					*pto++ = '\n';
					pto += sprintf(pto, "%s", color);

					los = pto;
					lis = pti;
				}
				else if (lis != chi) // infinite VT loop detection
				{
					pto = los;
					*pto++ = '\n';
					pto += sprintf(pto, "%s", color);
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
			else if (ses->wrap)
			{
				*pto++ = '\n';
			}
			ses->cur_col = 1;
			cur_space = 1;
		}
		else
		{
			if (*pti == '\t')
			{
				tab = ses->tab_width - (ses->cur_col - 1) % ses->tab_width;

				if (ses->cur_col + tab >= wrap) // xterm tabs
				{
					tab = (wrap - ses->cur_col);
				}
				pto += sprintf(pto, "%.*s", tab, "                ");
				pti++;

				cur_width += tab;
				ses->cur_col += tab;
				cur_space = ses->cur_col;
			}
			else
			{
				while (size--)
				{
					*pto++ = *pti++;
				}
				cur_width += tab;
				ses->cur_col += tab;
			}
		}
	}
	*pto = 0;

	*height = cur_height + 1;

	if (cur_width > *width)
	{
		*width = cur_width;
	}

	ses->cur_col = cur_col;

	pop_call();
	return lines + 1;
}

// store whatever falls inbetween skip and keep. Used by #buffer not checking SCROLL().

int word_wrap_split(struct session *ses, char *textin, char *textout, int wrap, int start, int end, int flags, int *height, int *width)
{
	char color[COLOR_SIZE] = { 0 };
	char *pti, *pto, *lis, *los;
	int cur_height, size, i, lines, cur_col, cur_width, tab, skip, cur_space;

	push_call("word_wrap_split(%s,%p,%p,%d,%d,%d,%d)",ses->name,textin,textout,wrap,start,end,flags);

	pti = lis = textin;
	pto = los = textout;

	if (wrap <= 0)
	{
		wrap = ses->wrap;

		if (ses->wrap == 0)
		{
			print_stdout(0, 0, "debug: word_wrap_split: wrap is 0\n");
			pop_call();
			return 1;
		}
	}

	lines      = 0;
	*height    = 0;
	cur_height = 0;
	cur_width  = 0;
	*width     = 0;
	cur_col    = 1;
	cur_space  = cur_col;

	if (HAS_BIT(flags, WRAP_FLAG_SPLIT) && end == 0)
	{
		print_stdout(0, 0, "debug: word_wrap_split: end point is 0.");
	}

	while (*pti && pto - textout < BUFFER_SIZE - 20)
	{
		if (cur_height > 10000 || cur_width > 100000)
		{
			print_stdout(0, 0, "debug: word_wrap_split: wrap %d height %d width %d los %d start %d end %d\n", wrap, cur_height, cur_width, pto - los, start, end);
			pop_call();
			return 1;
		}

		skip = skip_vt102_codes(pti);

		if (skip)
		{
			if (ses->color)
			{
				get_color_codes(color, pti, color, GET_ONE);

				for (i = 0 ; i < skip ; i++)
				{
					*pto++ = *pti++;
				}
			}
			else
			{
				pti += skip;
			}
			continue;
		}

		if (*pti == '\n')
		{
			if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height >= start && cur_height < end))
			{
				if (cur_width > *width)
				{
					*width = cur_width;
				}
			}

			lines++;
			cur_height++;

			lis = pti;
			los = pto;

			if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height > start && cur_height < end))
			{
				*pto++ = *pti++;
			}
			else
			{
				pti++;
			}

			if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height >= start && cur_height < end))
			{
				if (*pti)
				{
					pto += sprintf(pto, "%s", color);
				}
			}

			cur_col = 1;
			cur_space = 1;
			cur_width = 0;
			continue;
		}

		if (*pti == ' ' || *pti == '\t')
		{
			cur_space = cur_col;
			lis = pti;
			los = pto;
		}

		size = get_vt102_width(ses, pti, &tab);

		if (cur_col > 1 && cur_col + tab > wrap + 1)
		{
			if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height >= start && cur_height < end))
			{
				if (wrap > *width)
				{
					*width = wrap;
				}
			}
			else
			{
				cur_width = 0;
			}

			cur_height++;

			if (HAS_BIT(ses->config_flags, CONFIG_FLAG_WORDWRAP))
			{
				if (cur_col - cur_space > 15 || wrap <= 20)
				{
					if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height > start && cur_height < end))
					{
						*pto++ = '\n';
					}

					if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height >= start && cur_height < end))
					{
						pto += sprintf(pto, "%s", color);
//						pto += sprintf(pto, "%s(%d,%d,%d)", color, start, end, cur_height);

					}
				}
				else
				{
					pti = lis;
					pto = los;

					if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height > start && cur_height < end))
					{
						*pto++ = '\n';
					}

					if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height >= start && cur_height < end))
					{
						pto += sprintf(pto, "%s", color);

					}
					pti++;
				}
			}
			else
			{
				if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height > start && cur_height < end))
				{
					*pto++ = '\n';
				}

				if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height >= start && cur_height < end))
				{
					pto += sprintf(pto, "%s", color);
				}
			}
			cur_col = 1;
			cur_space = 1;
			continue;
		}

		if (*pti == '\t')
		{
			tab = ses->tab_width - (cur_col - 1) % ses->tab_width;

			if (cur_col + tab >= wrap)
			{
				tab = (wrap - cur_col);
			}

			if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height >= start && cur_height < end))
			{
				pto += sprintf(pto, "%.*s", tab, "                ");
			}
			pti++;

			cur_width += tab;
			cur_col += tab;
			cur_space = cur_col;
		}
		else
		{
			if (!HAS_BIT(flags, WRAP_FLAG_SPLIT) || (cur_height >= start && cur_height < end))
			{
				while (size--)
				{
					*pto++ = *pti++;
				}
			}
			else
			{
				pti += size;
			}
			cur_width += tab;
			cur_col += tab;
		}
	}
	*pto = 0;

	if (cur_width > *width)
	{
		*width = cur_width;
	}
	*height = cur_height + 1;

	pop_call();
	return lines + 1;
}

