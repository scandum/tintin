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

void init_pos(struct session *ses, int row, int col)
{
	push_call("init_pos(%p)",ses);

	goto_pos(ses, row, col);

	gtd->screen->sav_row[0] = ses->cur_row;
	gtd->screen->sav_col[0] = ses->cur_col;

	gtd->screen->sav_lev = 1;

	pop_call();
	return;
}

void hide_cursor(struct session *ses)
{
	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HIDDENCURSOR))
	{
		print_stdout(0, 0, "\e[?25l");
	}
}

void show_cursor(struct session *ses)
{
	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HIDDENCURSOR))
	{
		print_stdout(0, 0, "\e[?25h");
	}
}

void save_pos(struct session *ses)
{
	gtd->screen->sav_row[gtd->screen->sav_lev] = ses->cur_row;
	gtd->screen->sav_col[gtd->screen->sav_lev] = ses->cur_col;

	if (gtd->screen->sav_lev < STACK_SIZE)
	{
		gtd->screen->sav_lev++;
	}
	else
	{
		syserr_printf(ses, "sav_lev++ above 1000.");
	}

	hide_cursor(ses);
}

void restore_pos(struct session *ses)
{
	if (gtd->screen->sav_lev > 0)
	{
		gtd->screen->sav_lev--;
	}
	else
	{
		gtd->screen->sav_lev = 0;
		syserr_printf(ses, "restore_pos: sav_lev-- below 0.");
	}

	if (gtd->screen->sav_lev == 1 /* gtd->screen->sav_row[gtd->screen->sav_lev] == inputline_cur_row()*/ /*gtd->screen->rows*/)
	{
		goto_pos(ses, inputline_cur_row(), inputline_cur_col());
	}
	else
	{
		goto_pos(ses, gtd->screen->sav_row[gtd->screen->sav_lev], gtd->screen->sav_col[gtd->screen->sav_lev]);
	}
	show_cursor(ses);
}

void goto_pos(struct session *ses, int row, int col)
{
	if (row < 1 || col < 1)
	{
		print_stdout(0, 0, "debug: goto_pos(%d,%d)\n",row,col);
		dump_stack();

		return;
	}
	print_stdout(0, 0, "\e[%d;%dH", row, col);

	ses->cur_row = row;
	ses->cur_col = col;
}

void erase_cols(int cnt)
{
	if (cnt)
	{
		print_stdout(0, 0, "\e[%dX", cnt);
	}
}

/*
	unused
*/

void reset(struct session *ses)
{
	ses->cur_row = 1;
	ses->cur_col = 1;

	print_stdout(0, 0, "\ec");
}


void scroll_region(struct session *ses, int top, int bot)
{
	push_call("scroll_region(%p,%d,%d)",ses,top,bot);

	if (ses == gtd->ses)
	{
		if (top != 1)
		{
			print_stdout(0, 0, "\e[?1047h\e[?7787h\e[%d;%dr", top, bot);
		}
		else
		{
			print_stdout(0, 0, "\e[?1047l\e[?7787l\e[%d;%dr", top, bot);
		}
	}
	ses->split->top_row = top;
	ses->split->bot_row = bot;

	check_all_events(ses, EVENT_FLAG_VT100, 0, 4, "VT100 SCROLL REGION", ntos(top), ntos(bot), ntos(gtd->screen->rows), ntos(gtd->screen->cols), ntos(get_scroll_cols(ses)));

	pop_call();
	return;
}

void reset_scroll_region(struct session *ses)
{
	if (ses == gtd->ses)
	{
		print_stdout(0, 0, "\e[?1047l\e[?7787l\e[r");
	}
	ses->split->top_row = 1;
	ses->split->top_col = 1;
	ses->split->bot_row = gtd->screen->rows;
	ses->split->bot_col = gtd->screen->cols;
}


int skip_vt102_codes(char *str)
{
	int skip;

	switch (str[0])
	{
		case   7:   /* BEL */
		case   8:   /* BS  */
	//	case   9:      HT
	//	case  10:      LF
		case  11:   /* VT  */
		case  12:   /* FF  */
		case  13:   /* CR  */
		case  14:   /* SO  */
		case  15:   /* SI  */
		case  17:   /* DC1 */
		case  19:   /* DC3 */
		case  24:   /* CAN */
		case  26:   /* SUB */
			return 1;

		case  27:   /* ESC */
			break;

		case  28: // HTML_OPEN
			for (skip = 1 ; str[skip] ; skip++)
			{
				if (str[skip] == 30) // HTML_CLOSE
				{
					return skip + 1;
				}
			}
			return 0;

		case 127:   /* DEL */
			return 1;

		default:
			return 0;
	}

	switch (str[1])
	{
		case '\0':
			return 1;

		case '%':
		case '#':
		case '(':
		case ')':
			return str[2] ? 3 : 2;

		case ']':
			switch (str[2])
			{
				case 0:
					return 2;

				case 'P':
					for (skip = 3 ; skip < 10 ; skip++)
					{
						if (str[skip] == 0)
						{
							break;
						}
					}
					return skip;

				case 'R':
					return str[3] ? 3 : 2;

				default:
					for (skip = 2 ; str[skip] ; skip++)
					{
						if (str[skip] == '\a')
						{
							return skip + 1;
						}

						if (str[skip] == '\e' && str[skip+1] == '\\')
						{
							return skip + 2;
						}
					}
					break;
			}
			return 2;

		case '[':
			break;

		default:
			return 2;
	}

	for (skip = 2 ; str[skip] != 0 ; skip++)
	{
		if (is_csichar(str[skip]))
		{
			return skip + 1;
		}
	}
	return skip;
}

int skip_vt102_codes_non_graph(char *str)
{
	int skip = 0;

	switch (str[skip])
	{
		case   7:   // BEL
//		case   8:   // BS  
//		case   9:   // HT  
//		case  10:   // LF  
		case  11:   // VT  
		case  12:   // FF  
		case  13:   // CR  
		case  14:   // SO  
		case  15:   // SI  
		case  17:   // DC1 
		case  19:   // DC3 
		case  24:   // CAN 
		case  26:   // SUB 
		case 127:   // DEL 
			return 1;

		case  27:   // ESC 
			break;

		default:
			return 0;
	}

	switch (str[1])
	{
		case '\0':
			return 0;

		case 'c':
		case 'D':
		case 'E':
		case 'H':
		case 'M':
		case 'Z':
		case '7':
		case '8':
		case '>':
		case '=':
			return 2;

		case '%':
		case '#':
		case '(':
		case ')':
			return str[2] ? 3 : 2;

		case ']':
			switch (str[2])
			{
				case 0:
					return 2;

				case 'P':
					for (skip = 3 ; skip < 10 ; skip++)
					{
						if (str[skip] == 0)
						{
							break;
						}
					}
					return skip;

				case 'R':
					return str[3] ? 3 : 2;

				case '6':
					return 0;

				default:
					for (skip = 2 ; str[skip] ; skip++)
					{
						if (str[skip] == '\a')
						{
							return skip + 1;
						}

						if (str[skip] == '\e' && str[skip+1] == '\\')
						{
							return skip + 2;
						}
					}
					break;
			}
			return 2;

		case '[':
			break;

		default:
			return 2;
	}

	for (skip = 2 ; str[skip] != 0 ; skip++)
	{
		if (str[skip] == 'm')
		{
			return 0;
		}

		if (is_csichar(str[skip]))
		{
			return skip + 1;
		}
	}
	return 0;
}

int get_vt102_width(struct session *ses, char *str, int *str_len)
{
	int raw_len;

	*str_len = 0;

	if (*str)
	{
		raw_len = skip_vt102_codes(str);

		if (raw_len)
		{
			return raw_len;
		}

		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC))
		{
			return get_euc_width(ses, str, str_len);
		}

		if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8))
		{
			return get_utf8_width(str, str_len);
		}

		return get_ascii_width(str, str_len);
	}
	return 0;
}

int strip_vt102_width(struct session *ses, char *str, int *str_width)
{
	int width;
	char *pts;

	pts = str;

	*str_width = 0;

	while (*pts)
	{
		pts += get_vt102_width(ses, pts, &width);

		*str_width += width;
	}
	return pts - str;
}


int find_color_code(char *str)
{
	int skip;

	switch (str[0])
	{
		case  ASCII_ESC:
			break;

		default:
			return 0;
	}

	switch (str[1])
	{
		case '[':
			break;

		default:
			return 0;
	}

	for (skip = 2 ; str[skip] != 0 ; skip++)
	{
		if (str[skip] == 'm')
		{
			return skip + 1;
		}

		if (is_csichar(str[skip]))
		{
			return 0;
		}
	}
	return 0;
}

int find_escaped_color_code(char *str)
{
	int skip;

	switch (str[0])
	{
		case  '\\':
			break;

		default:
			return 0;
	}

	switch (str[1])
	{
		case 'e':
			break;
		default:
			return 0;
	}

	switch (str[2])
	{
		case '[':
			break;

		default:
			return 0;
	}

	for (skip = 3 ; str[skip] != 0 ; skip++)
	{
		if (str[skip] == 'm')
		{
			return skip + 1;
		}

		if (is_csichar(str[skip]))
		{
			return 0;
		}
	}
	return 0;
}

int find_secure_color_code(char *str)
{
	int skip;

	if (*str != ASCII_ESC)
	{
		return 0;
	}

	if (str[1] == '[')
	{
		for (skip = 2 ; str[skip] != 0 ; skip++)
		{
			switch (str[skip])
			{
				case 'm':
					if (is_digit(str[skip - 1]))
					{
						return skip + 1;
					}
					return 0;

				case ';':
				case ':':
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
					break;

				default:
					return 0;
			}
		}
	}
	else if (str[1] == ']' && str[2] == '6' && str[3] == '8' && str[4] == ';')
	{
		for (skip = 5 ; str[skip] != 0 ; skip++)
		{
			if (str[skip] == ASCII_BEL)
			{
				return skip + 1;
			}
		}
		return 0;
	}
	return 0;
}


int strip_vt102_codes(char *str, char *buf)
{
	char *pti, *pto;

	pti = (char *) str;
	pto = (char *) buf;

	while (*pti)
	{
		while (skip_vt102_codes(pti))
		{
			pti += skip_vt102_codes(pti);
		}

		if (*pti)
		{
			*pto++ = *pti++;
		}
	}
	*pto = 0;

	return pto - buf;
}


void strip_vt102_codes_non_graph(char *str, char *buf)
{
	char *pti, *pto;

	pti = str;
	pto = buf;

	while (*pti)
	{
		while (skip_vt102_codes_non_graph(pti))
		{
			pti += skip_vt102_codes_non_graph(pti);
		}

		if (*pti)
		{
			*pto++ = *pti++;
		}
	}
	*pto = 0;
}

void strip_non_vt102_codes(char *str, char *buf)
{
	char *pti, *pto;
	int len;

	pti = str;
	pto = buf;

	while (*pti)
	{
		while ((len = skip_vt102_codes(pti)) != 0)
		{
			memcpy(pto, pti, len);
			pti += len;
			pto += len;
		}

		if (*pti)
		{
			pti++;
		}
	}
	*pto = 0;
}

char *strip_vt102_strstr(char *str, char *buf, int *len)
{ 
	char *pti, *ptm, *pts;

	push_call("strip_vt102_strstr(%p,%p,%p)",str,buf,len);

	pts = str;

	while (*pts)
	{
		pti = pts;
		ptm = buf;

		while (*pti)
		{
			if (*pti++ != *ptm++)
			{
				break;
			}

			if (*ptm == 0)
			{
				if (len)
				{
					*len = pti - pts;
				}
				pop_call();
				return pts;
			}

			while (skip_vt102_codes(pti))
			{
				pti += skip_vt102_codes(pti);
			}
		}
		pts++;
	}
	pop_call();
	return NULL;
}

// mix old and str, then copy compressed color string to buf which can point to old.

void get_color_codes(char *old, char *str, char *buf, int flags)
{
	char *pti, *ptb, *ptc, col[100];
	int hop, vtc, fgc, bgc;
	int rgb[6] = { 0, 0, 0, 0, 0, 0 };

	hop =  0;
	vtc =  0;
	fgc = -1;
	bgc = -1;

	start:

	pti = hop ? str : old;

	while (*pti)
	{
		if (*pti != ASCII_ESC)
		{
			if (!HAS_BIT(flags, GET_ALL))
			{
				break;
			}
			pti++;

			continue;
		}

		pti++;

		if (*pti != '[')
		{
			continue;
		}
		pti++;

		if (*pti == 'm')
		{
			vtc =  0;
			fgc = -1;
			bgc = -1;

			pti++;

			continue;
		}

		ptc = col;

		while (*pti)
		{
			if (*pti >= '0' && *pti <= '9')
			{
				if (ptc - col > 20)
				{
					break;
				}

				*ptc++ = *pti++;

				continue;
			}

			if (*pti != ';' && *pti != 'm')
			{
				pti++;

				break;
			}

			*ptc = 0;

			ptc = col;

			if (HAS_BIT(vtc, COL_XTF_R))
			{
				fgc = URANGE(0, atoi(col), 255);
				DEL_BIT(vtc, COL_XTF_5|COL_XTF_R);
				SET_BIT(vtc, COL_XTF);
			}
			else if (HAS_BIT(vtc, COL_XTB_R))
			{
				bgc = URANGE(0, atoi(col), 255);
				DEL_BIT(vtc, COL_XTB_5|COL_XTB_R);
				SET_BIT(vtc, COL_XTB);
			}
			else if (HAS_BIT(vtc, COL_TCF_R))
			{
				if (rgb[0] == 256)
				{
					rgb[0] = URANGE(0, atoi(col), 255);
				}
				else if (rgb[1] == 256)
				{
					rgb[1] = URANGE(0, atoi(col), 255);
				}
				else
				{
					rgb[2] = URANGE(0, atoi(col), 255);

					fgc = rgb[0] * 256 * 256 + rgb[1] * 256 + rgb[2];
					
					DEL_BIT(vtc, COL_TCF_2|COL_TCF_R);
					SET_BIT(vtc, COL_TCB);
				}
			}
			else if (HAS_BIT(vtc, COL_TCB_R))
			{
				if (rgb[3] == 256)
				{
					rgb[3] = URANGE(0, atoi(col), 255);
				}
				else if (rgb[4] == 256)
				{
					rgb[4] = URANGE(0, atoi(col), 255);
				}
				else
				{
					rgb[5] = URANGE(0, atoi(col), 255);

					fgc = rgb[3] * 256 * 256 + rgb[4] * 256 + rgb[5];
					
					DEL_BIT(vtc, COL_TCB_2|COL_TCF_R);
					SET_BIT(vtc, COL_TCB);
				}
			}
			else
			{
				switch (atoi(col))
				{
					case 0:
						vtc = 0;
						fgc = -1;
						bgc = -1;
						break;
					case 1:
						SET_BIT(vtc, COL_BLD);
						break;
					case 2:
						if (HAS_BIT(vtc, COL_TCF_2))
						{
							rgb[0] = rgb[1] = rgb[2] = 256;
							SET_BIT(vtc, COL_TCF_R);
						}
						else if (HAS_BIT(vtc, COL_TCB_2))
						{
							rgb[3] = rgb[4] = rgb[5] = 256;
							SET_BIT(vtc, COL_TCB_R);
						}
						else
						{
							DEL_BIT(vtc, COL_BLD);
						}
						break;

					case 3:
						SET_BIT(vtc, COL_ITA);
						break;

					case 4:
						SET_BIT(vtc, COL_UND);
						break;
					case 5:
						if (HAS_BIT(vtc, COL_XTF_5))
						{
							SET_BIT(vtc, COL_XTF_R);
						}
						else if (HAS_BIT(vtc, COL_XTB_5))
						{
							SET_BIT(vtc, COL_XTB_R);
						}
						else
						{
							SET_BIT(vtc, COL_BLK);
						}
						break;
					case 7:
						SET_BIT(vtc, COL_REV);
						break;
					case 21:
					case 22:
						DEL_BIT(vtc, COL_BLD);
						break;

					case 23:
						DEL_BIT(vtc, COL_ITA);
						break;

					case 24:
						DEL_BIT(vtc, COL_UND);
						break;
					case 25:
						DEL_BIT(vtc, COL_BLK);
						break;
					case 27:
						DEL_BIT(vtc, COL_REV);
						break;
					case 38:
						SET_BIT(vtc, COL_XTF_5|COL_TCF_2);
						fgc = -1;
						break;
					case 48:
						SET_BIT(vtc, COL_XTB_5|COL_TCB_2);
						bgc = -1;
						break;
					default:
						switch (atoi(col) / 10)
						{
							case 3:
							case 10:
								fgc = atoi(col);
								DEL_BIT(vtc, COL_XTF|COL_TCF);
								break;
							
							case 4:
							case 9:
								bgc = atoi(col);
								DEL_BIT(vtc, COL_XTB|COL_TCB);
								break;
						}
						break;
				}
			}

			if (*pti++ == 'm')
			{
				break;
			}
		}
	}

	if (++hop == 1)
	{
		goto start;
	}

	ptb = buf;

	*ptb++ = '\e';
	*ptb++ = '[';
	*ptb++ = '0';

	if (HAS_BIT(vtc, COL_BLD))
	{
		*ptb++ = ';';
		*ptb++ = '1';
	}
	if (HAS_BIT(vtc, COL_ITA))
	{
		*ptb++ = ';';
		*ptb++ = '3';
	}

	if (HAS_BIT(vtc, COL_UND))
	{
		*ptb++ = ';';
		*ptb++ = '4';
	}
	if (HAS_BIT(vtc, COL_BLK))
	{
		*ptb++ = ';';
		*ptb++ = '5';
	}
	if (HAS_BIT(vtc, COL_REV))
	{
		*ptb++ = ';';
		*ptb++ = '7';
	}

	if (HAS_BIT(vtc, COL_XTF))
	{
		ptb += sprintf(ptb, ";38;5;%d", fgc);
	}
	else if (HAS_BIT(vtc, COL_TCF))
	{
		ptb += sprintf(ptb, ";38;2;%d;%d;%d", fgc / 256 / 256, fgc / 256 % 256, fgc % 256);
	}
	else if (fgc != -1)
	{
		ptb += sprintf(ptb, ";%d", fgc);
	}

	if (HAS_BIT(vtc, COL_XTB))
	{
		ptb += sprintf(ptb, ";48;5;%d", bgc);
	}
	else if (HAS_BIT(vtc, COL_TCB))
	{
		ptb += sprintf(ptb, ";48;2;%d;%d;%d", bgc / 256 / 256, bgc / 256 % 256, bgc % 256);
	}
	else if (bgc != -1)
	{
		ptb += sprintf(ptb, ";%d", bgc);
	}

	*ptb++ = 'm';

	*ptb = 0;

	return;
}

int strip_vt102_strlen(struct session *ses, char *str)
{
	char *pti;
	int size, width, str_len;
	
	str_len = 0;

	pti = str;

	while (*pti)
	{
		size = skip_vt102_codes(pti);

		if (size == 0)
		{
/*			if (*pti == '\t')
			{
				width = ses->tab_width - str_len % ses->tab_width;
				size  = 1;
			}
			else
*/
			if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(pti))
			{
				size = get_utf8_width(pti, &width);
			}
			else
			{
				size = get_ascii_width(pti, &width);
			}
			str_len += width;
		}
		pti += size;
	}
	return str_len;
}

// outdated

int strip_color_strlen(struct session *ses, char *str)
{
	char *buf = str_alloc_stack(0);

	substitute(ses, str, buf, SUB_ESC|SUB_COL);

	return strip_vt102_strlen(ses, buf);
}

int interpret_vt102_codes(struct session *ses, char *str, int real)
{
	char *data = str_alloc_stack(0);
	int skip = 0;

	switch (str[skip])
	{
		case   8:
			ses->cur_col = UMAX(1, ses->cur_col - 1);
			return TRUE;

		case  27:   /* ESC */
			break;

		case 11:    /* VT  */
			ses->cur_row = UMIN(gtd->screen->rows, ses->cur_row + 1);
			return TRUE;

		case 12:    /* FF  */
			ses->cur_row = UMIN(gtd->screen->rows, ses->cur_row + 1);
			return TRUE;

		case  13:   /* CR  */
			ses->cur_col = 1;
			return TRUE;

		default:
			return TRUE;
	}

	switch (str[1])
	{
		case '7':
			ses->sav_row = ses->cur_row;
			ses->sav_col = ses->cur_col;
			return TRUE;

		case '8':
			ses->cur_row = ses->sav_row;
			ses->cur_col = ses->sav_col;
			return TRUE;

		case 'c':
			ses->cur_row = 1;
			ses->cur_col = 1;
			ses->sav_row = ses->cur_row;
			ses->sav_col = ses->cur_col;
			return TRUE;

		case 'D':
			ses->cur_row = URANGE(1, ses->cur_row + 1, gtd->screen->rows);
			return TRUE;

		case 'E':
			ses->cur_row = URANGE(1, ses->cur_row + 1, gtd->screen->rows);
			ses->cur_col = 1;
			return TRUE;

		case 'M':
			ses->cur_row = URANGE(1, ses->cur_row - 1, gtd->screen->rows);
			return TRUE;

		case '[':
			break;

		default:
			return TRUE;
	}

	for (skip = 2 ; str[skip] != 0 ; skip++)
	{
		switch (str[skip])
		{
			case '@':
			case '`':
			case ']':
				return TRUE;

			case 'c':
				return FALSE;

			case 'A':
				ses->cur_row -= UMAX(1, atoi(data));
				break;

			case 'B':
			case 'e':
				ses->cur_row += UMAX(1, atoi(data));
				break;

			case 'C':
			case 'a':
				ses->cur_col += UMAX(1, atoi(data));
				break;

			case 'D':
				ses->cur_col -= UMAX(1, atoi(data));
				break;

			case 'E':
				ses->cur_row -= UMAX(1, atoi(data));
				ses->cur_col = 1;
				break;

			case 'F':
				ses->cur_row -= UMAX(1, atoi(data));
				ses->cur_col = 1;
				break;

			case 'G':
				ses->cur_col = UMAX(1, atoi(data));
				break;

			case 'H':
			case 'f':
				if (sscanf(data, "%d;%d", &ses->cur_row, &ses->cur_col) != 2)
				{
					if (sscanf(data, "%d", &ses->cur_row) == 1)
					{
						ses->cur_col = 1;
					}
					else
					{
						ses->cur_row = 1;
						ses->cur_col = 1;
					}
				}
				break;

			case 'd':
				ses->cur_row = atoi(data);
				break;

			case 'r':
				if (sscanf(data, "%d;%d", &ses->split->top_row, &ses->split->bot_row) != 2)
				{
					if (sscanf(data, "%d", &ses->split->top_row) != 1)
					{
						ses->split->top_row = 1;
						ses->split->bot_row = gtd->screen->rows;
					}
					else
					{
						ses->split->bot_row = gtd->screen->rows;
					}
				}
				ses->cur_row = 1;
				ses->cur_col = 1;
				break;

			case 's':
				ses->sav_row = ses->cur_row;
				ses->sav_col = ses->cur_col;
				break;

			case 'u':
				ses->cur_row = ses->sav_row;
				ses->cur_col = ses->sav_col;
				break;

			case 'x':
				return FALSE;

			default:
				data[skip - 2] = str[skip];
				data[skip - 1] = 0;
				break;
		}

		if (is_alpha(str[skip]))
		{
			ses->cur_row = URANGE(1, ses->cur_row, gtd->screen->rows);

			ses->cur_col = URANGE(1, ses->cur_col, gtd->screen->cols + 1);


			ses->split->top_row = URANGE(1, ses->split->top_row, gtd->screen->rows);

			ses->split->bot_row = ses->split->bot_row ? URANGE(1, ses->split->bot_row, gtd->screen->rows) : gtd->screen->rows;
			
			return TRUE;
		}
	}
	return TRUE;
}

int catch_vt102_codes(struct session *ses, unsigned char *str, int cplen)
{
	int skip, cnt, len, val[5];

	push_call("catch_vt102_codes(%p)",str);

	switch (str[0])
	{
		case ASCII_ENQ:
			if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 1, "CATCH VT100 ENQ", gtd->system->term))
			{
				pop_call();
				return 1;
			}
			pop_call();
			return 0;
			
		case ASCII_ESC:
			break;

		default:
			pop_call();
			return 0;
	}

	val[0] = val[1] = val[2] = val[3] = 0;

	cnt = 0;

	switch (str[1])
	{
		case '\0':
			break;

		case '[':
			for (len = 2 ; str[len] ; len++)
			{
				if (str[len] >= '0' && str[len] <= '9')
				{
					val[cnt] *= 10;
					val[cnt] += str[len] - '0';
				}
				else
				{
					switch (str[len])
					{
						case ';':
							if (cnt < 4)
							{
								cnt++;
							}
							break;

						case 'c':
							check_all_events(ses, EVENT_FLAG_VT100, 0, 1, "VT100 DA", ntos(val[0]));
							pop_call();
							return len + 1;

						case 'n':
							if (val[0] == 5)
							{
								check_all_events(ses, EVENT_FLAG_VT100, 0, 0, "VT100 DSR");
							}
							if (val[0] == 6)
							{
								check_all_events(ses, EVENT_FLAG_VT100, 0, 2, "VT100 CPR", ntos(gtd->screen->cols), ntos(gtd->screen->rows));
							}
							pop_call();
							return len + 1;

						case 'r':
							if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 2, "CATCH VT100 SCROLL REGION", ntos(val[0]), ntos(val[1])))
							{
								pop_call();
								return len + 1;
							}
							break;

						case 'H':
							if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 2, "CATCH VT100 CURSOR H", ntos(val[0]), ntos(val[1])))
							{
								pop_call();
								return len + 1;
							}
							break;

						case 'J':
							if (val[0] == 0)
							{
								if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH VT100 ERASE SCREEN BELOW"))
								{
									pop_call();
									return len + 1;
								}
							}
							if (val[0] == 1)
							{
								if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH VT100 ERASE SCREEN ABOVE"))
								{
									pop_call();
									return len + 1;
								}
							}
							if (val[0] == 2)
							{
								if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH VT100 ERASE SCREEN ALL"))
								{
									pop_call();
									return len + 1;
								}
							}
							if (val[0] == 3)
							{
								if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH VT100 ERASE SCREEN SAVED"))
								{
									pop_call();
									return len + 1;
								}
							}
							break;

						case 'K':
							if (val[0] == 0)
							{
								if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH VT100 ERASE LINE RIGHT"))
								{
									pop_call();
									return len + 1;
								}
							}
							if (val[0] == 1)
							{
								if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH VT100 ERASE LINE LEFT"))
								{
									pop_call();
									return len + 1;
								}
							}
							if (val[0] == 2)
							{
								if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 0, "CATCH VT100 ERASE LINE ALL"))
								{
									pop_call();
									return len + 1;
								}
							}
							break;

						case 'Z':
							check_all_events(ses, EVENT_FLAG_VT100, 0, 0, "VT100 DECID");
							pop_call();
							return len + 1;

						default:
							pop_call();
							return 0;
					}
				}
			}
			break;

		case ']':
			{
				int opt;
				char *osc = str_alloc_stack(0);

				if (str[2] == 'P')
				{
					if (cplen >= 10)
					{
						sprintf(osc, "%.*s", 8, str + 3);
							
						check_all_events(ses, EVENT_FLAG_VT100, 0, 1, "VT100 OSC COLOR PALETTE", osc);
							
						if (check_all_events(ses, EVENT_FLAG_CATCH, 0, 1, "CATCH VT100 OSC", osc))
						{
							pop_call();
							return 11;
						}
					}
					pop_call();
					return 0;
				}
				else
				{
					opt = 0;

					for (skip = 2 ; cplen >= skip && skip < NUMBER_SIZE ; skip++)
					{
						if (!is_digit(str[skip]))
						{
							break;
						}
						opt = opt * 10 + (str[skip] - '0');
					}

					if (opt == 68 && cplen >= skip + 3)
					{
						if (str[skip] != ';')
						{
							pop_call();
							return 0;
						}

						if (str[skip + 1] == '2' && str[skip + 2] == ';')
						{
							pop_call();
							return 1;
						}
					}
					
					while (cplen >= skip && skip < BUFFER_SIZE)
					{
						if (str[skip] == ASCII_BEL)
						{
							break;
						}
						skip++;
					}
	
					sprintf(osc, "%.*s", skip - 2, str + 2);

					check_all_events(ses, SUB_SEC|EVENT_FLAG_VT100, 0, 1, "VT100 OSC", osc);

					if (check_all_events(ses, SUB_SEC|EVENT_FLAG_CATCH, 0, 1, "CATCH VT100 OSC", osc))
					{
						pop_call();
						return skip + 1;
					}
				}
			}
			break;
	}
	pop_call();
	return 0;
}
