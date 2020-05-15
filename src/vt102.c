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

void save_pos(struct session *ses)
{
	push_call("save_pos(%p)",ses);

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

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HIDDENCURSOR))
	{
		print_stdout("\e[?25l");
	}

	pop_call();
	return;
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
		syserr_printf(ses, "sav_lev-- below 0.");
	}

	if (gtd->screen->sav_lev == 1 /* gtd->screen->sav_row[gtd->screen->sav_lev] == inputline_cur_row()*/ /*gtd->screen->rows*/)
	{
		goto_pos(ses, inputline_cur_row(), inputline_cur_col());
	}
	else
	{
		goto_pos(ses, gtd->screen->sav_row[gtd->screen->sav_lev], gtd->screen->sav_col[gtd->screen->sav_lev]);
	}

	if (!HAS_BIT(gtd->flags, TINTIN_FLAG_HIDDENCURSOR))
	{
		print_stdout("\e[?25h");
	}
}

void goto_pos(struct session *ses, int row, int col)
{
	print_stdout("\e[%d;%dH", row, col);

	if (col < 1)
	{
		print_stdout("debug: goto_pos(%d,%d)\n",row,col);
	}
	ses->cur_row = row;
	ses->cur_col = col;
}

void erase_cols(int cnt)
{
	if (cnt)
	{
		print_stdout("\e[%dX", cnt);
	}
}

void erase_row(struct session *ses)
{
	if (ses->wrap == gtd->screen->cols || ses->cur_row == gtd->screen->rows)
	{
		print_stdout("\e[2K");
	}
	else
	{
		save_pos(ses);

		goto_pos(ses, ses->cur_row, ses->split->top_col);

		erase_cols(ses->wrap);

		restore_pos(ses);
	}
}

void erase_lines(struct session *ses, int rows)
{
	print_stdout("\e[%dM", rows);
}

void erase_toeol(void)
{
	print_stdout("\e[K");
}

/*
	unused
*/

void reset(struct session *ses)
{
	ses->cur_row = 1;
	ses->cur_col = 1;

	print_stdout("\ec");
}


void scroll_region(struct session *ses, int top, int bot)
{
	push_call("scroll_region(%p,%d,%d)",ses,top,bot);

	print_stdout("\e[%d;%dr", top, bot);

	ses->split->top_row = top;
	ses->split->bot_row = bot;

	check_all_events(ses, SUB_ARG, 0, 4, "VT100 SCROLL REGION", ntos(top), ntos(bot), ntos(gtd->screen->rows), ntos(gtd->screen->cols), ntos(get_scroll_cols(ses)));

	pop_call();
	return;
}

void reset_scroll_region(struct session *ses)
{
	if (ses == gtd->ses)
	{
		print_stdout("\e[r");
	}
	ses->split->top_row = 1;
	ses->split->top_col = 1;
	ses->split->bot_row = gtd->screen->rows;
	ses->split->bot_col = gtd->screen->cols;
}


int skip_vt102_codes(char *str)
{
	int skip = 0;

	push_call("skip_vt102_codes(%p)",str);

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
			pop_call();
			return 1;

		case  27:   /* ESC */
			break;

		case  28: // HTML_OPEN
			for (skip = 1 ; str[skip] ; skip++)
			{
				if (str[skip] == 30) // HTML_CLOSE
				{
					pop_call();
					return skip + 1;
				}
			}
			pop_call();
			return 0;

		case 127:   /* DEL */
			pop_call();
			return 1;

		default:
			pop_call();
			return 0;
	}

	switch (str[1])
	{
		case '\0':
			pop_call();
			return 1;

		case '%':
		case '#':
		case '(':
		case ')':
			pop_call();
			return str[2] ? 3 : 2;

		case ']':
			switch (str[2])
			{
				case 0:
					pop_call();
					return 2;

				case 'P':
					for (skip = 3 ; skip < 10 ; skip++)
					{
						if (str[skip] == 0)
						{
							break;
						}
					}
					pop_call();
					return skip;

				case 'R':
					pop_call();
					return str[3] ? 3 : 2;

				default:
					for (skip = 2 ; str[skip] ; skip++)
					{
						if (str[skip] == '\a' || (str[skip] == '\e' && str[skip+1] == '\\'))
						{
							pop_call();
							return skip + 1;
						}
					}
			}
			pop_call();
			return 2;

		case '[':
			break;

		default:
			pop_call();
			return 2;
	}

	for (skip = 2 ; str[skip] != 0 ; skip++)
	{
		if (isalpha((int) str[skip]))
		{
			pop_call();
			return skip + 1;
		}

		switch (str[skip])
		{
			case '@':
			case '`':
			case ']':
				pop_call();
				return skip + 1;
		}
	}
	pop_call();
	return skip;
}

int skip_one_char(struct session *ses, char *str, int *width)
{
	int skip;

	*width = 0;

	if (*str)
	{
		skip = skip_vt102_codes(str);

		if (skip)
		{
			return skip;
		}

		if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, str))
		{
			return get_euc_width(ses, str, width);
		}

		if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(str))
		{
			return get_utf8_width(str, width);
		}

		*width = 1;
		return 1;
	}
	return 0;
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
		switch (str[skip])
		{
			case 'm':
				return skip + 1;
			case '@':
			case '`':
			case ']':
				return 0;
		}

		if (isalpha((int) str[skip]))
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
		switch (str[skip])
		{
			case 'm':
				return skip + 1;
			case '@':
			case '`':
			case ']':
				return 0;
		}

		if (isalpha((int) str[skip]))
		{
			return 0;
		}
	}
	return 0;
}

int find_secure_color_code(char *str)
{
	int skip, valid = 1;

	switch (str[0])
	{
		case  ASCII_ESC:   /* ESC */
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
		switch (str[skip])
		{
			case 'm':
				if (valid)
				{
					return skip + 1;
				}
				return 0;

			case ';':
				valid = 0;
				break;

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
				valid = 1;
				break;

			default:
				return 0;
		}
	}
	return 0;
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
					return 3;
			}
			return 0;

		case '[':
			break;

		default:
			return 2;
	}

	for (skip = 2 ; str[skip] != 0 ; skip++)
	{
		switch (str[skip])
		{
			case 'm':
				return 0;
			case '@':
			case '`':
			case ']':
				return skip + 1;
		}

		if (isalpha((int) str[skip]))
		{
			return skip + 1;
		}
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
	char *pti, *pto, col[100], tmp[BUFFER_SIZE];
	int len, vtc, fgc, bgc, cnt;
	int rgb[6] = { 0, 0, 0, 0, 0, 0 };

	push_call("get_color_codes(%p,%p,%p,%d)",old,str,buf,flags);

	pto = tmp;

	pti = old;

	while (*pti)
	{
		while ((len = find_color_code(pti)) != 0)
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

	pti = str;

	while (*pti)
	{
		while ((len = find_color_code(pti)) != 0)
		{
			memcpy(pto, pti, len);
			pti += len;
			pto += len;
		}

		if (!HAS_BIT(flags, GET_ALL))
		{
			break;
		}

		if (*pti == '\n')
		{
			break;
		}

		if (*pti)
		{
			pti++;
		}
	}

	*pto = 0;

	if (strlen(tmp) == 0)
	{
		buf[0] = 0;
		pop_call();
		return;
	}

	vtc =  0;
	fgc = -1;
	bgc = -1;

	pti = tmp;

	while (*pti)
	{
		switch (*pti)
		{
			case ASCII_ESC:
				pti += 2;

				if (pti[-1] == 'm')
				{
					vtc =  0;
					fgc = -1;
					bgc = -1;
					break;
				}

				for (cnt = 0 ; pti[cnt] ; cnt++)
				{
					col[cnt] = pti[cnt];

					if (pti[cnt] == ';' || pti[cnt] == 'm')
					{
						col[cnt] = 0;

						cnt = -1;
						pti += 1 + strlen(col);

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
					}

					if (pti[-1] == 'm')
					{
						break;
					}
				}
				break;

			default:
				pti++;
				break;
		}
	}

	strcpy(buf, "\e[0");

	if (HAS_BIT(vtc, COL_BLD))
	{
		strcat(buf, ";1");
	}
	if (HAS_BIT(vtc, COL_UND))
	{
		strcat(buf, ";4");
	}
	if (HAS_BIT(vtc, COL_BLK))
	{
		strcat(buf, ";5");
	}
	if (HAS_BIT(vtc, COL_REV))
	{
		strcat(buf, ";7");
	}

	if (HAS_BIT(vtc, COL_XTF))
	{
		cat_sprintf(buf, ";38;5;%d", fgc);
	}
	else if (HAS_BIT(vtc, COL_TCF))
	{
		cat_sprintf(buf, ";38;2;%d;%d;%d", fgc / 256 / 256, fgc / 256 % 256, fgc % 256);
	}
	else if (fgc != -1)
	{
		cat_sprintf(buf, ";%d", fgc);
	}

	if (HAS_BIT(vtc, COL_XTB))
	{
		cat_sprintf(buf, ";48;5;%d", bgc);
	}
	else if (HAS_BIT(vtc, COL_TCB))
	{
		cat_sprintf(buf, ";48;2;%d;%d;%d", bgc / 256 / 256, bgc / 256 % 256, bgc % 256);
	}
	else if (bgc != -1)
	{
		cat_sprintf(buf, ";%d", bgc);
	}

	strcat(buf, "m");

	pop_call();
	return;
}

int strip_vt102_strlen(struct session *ses, char *str)
{
	char *pti;
	int w, i = 0;

	pti = str;

	while (*pti)
	{
		if (skip_vt102_codes(pti))
		{
			pti += skip_vt102_codes(pti);

			continue;
		}

		if (*pti == '\t')
		{
			i += ses->tab_width - i % ses->tab_width;
			pti++;
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(pti))
		{
			pti += get_utf8_width(pti, &w);

			i += w;
		}
		else
		{
			pti++;
			i++;
		}
	}
	return i;
}


int strip_vt102_width(struct session *ses, char *str, int *lines)
{
	char *pti;
	int w, i = 0, max = 0;

	*lines = 1;

	pti = str;

	while (*pti)
	{
		if (*pti == '\n')
		{
			*lines += 1;

			if (i > max)
			{
				max = i;
				i = 0;
			}
		}

		if (skip_vt102_codes(pti))
		{
			pti += skip_vt102_codes(pti);

			continue;
		}

		if (*pti == '\t')
		{
			i += ses->tab_width - i % ses->tab_width;
			pti++;
		}
		else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(pti))
		{
			pti += get_utf8_width(pti, &w);

			i += w;
		}
		else
		{
			pti++;
			i++;
		}
	}

	if (i > max)
	{
		return i;
	}
	return max;
}

int strip_color_strlen(struct session *ses, char *str)
{
	char buf[BUFFER_SIZE];

	substitute(ses, str, buf, SUB_ESC|SUB_COL);

	return strip_vt102_strlen(ses, buf);
}

int interpret_vt102_codes(struct session *ses, char *str, int real)
{
	char data[BUFFER_SIZE] = { 0 };
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

		if (isalpha((int) str[skip]))
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

