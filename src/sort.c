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

#include <assert.h>

//typedef int CMPFUNC (const void *a, const void *b);

void quad_sort32(int *array, int *swap, size_t nmemb, size_t block, CMPFUNC *cmp);

void quad_swap32(int *array, int *swap, size_t nmemb, CMPFUNC *cmp)
{
	size_t offset;
	register unsigned char loop;
	register int *pta, *pts, *ptt, tmp;

	pta = array;

	for (offset = 0 ; offset + 4 <= nmemb ; offset += 4)
	{
		if (cmp(&pta[0], &pta[1]) > 0)
		{
			tmp = pta[0];
			pta[0] = pta[1];
			pta[1] = tmp;
		}

		if (cmp(&pta[2], &pta[3]) > 0)
		{
			tmp = pta[2];
			pta[2] = pta[3];
			pta[3] = tmp;
		}

		if (cmp(&pta[1], &pta[2]) > 0)
		{
			if (cmp(&pta[0], &pta[3]) > 0)
			{
				tmp = pta[0];
				pta[0] = pta[2];
				pta[2] = tmp;

				tmp = pta[1];
				pta[1] = pta[3];
				pta[3] = tmp;
			}
			else if (cmp(&pta[0], &pta[2]) <= 0)
			{
				if (cmp(&pta[1], &pta[3]) <= 0)
				{
					tmp = pta[1];
					pta[1] = pta[2];
					pta[2] = tmp;
				}
				else
				{
					tmp = pta[1];
					pta[1] = pta[2];
					pta[2] = pta[3];
					pta[3] = tmp;
				}
			}
			else
			{
				if (cmp(&pta[1], &pta[3]) <= 0)
				{
					tmp = pta[0];
					pta[0] = pta[2];
					pta[2] = pta[1];
					pta[1] = tmp;
				}
				else
				{
					tmp = pta[0];
					pta[0] = pta[2];
					pta[2] = pta[3];
					pta[3] = pta[1];
					pta[1] = tmp;
				}
			}
		}
		pta += 4;
	}

	switch (nmemb - offset)
	{
		case 0:
		case 1:
			break;
		case 2:
			if (cmp(&pta[0], &pta[1]) > 0)
			{
				tmp = pta[0];
				pta[0] = pta[1];
				pta[1] = tmp;
			}
			break;
		case 3:
			if (cmp(&pta[0], &pta[1]) > 0)
			{
				tmp = pta[0];
				pta[0] = pta[1];
				pta[1] = tmp;
			}
			if (cmp(&pta[1], &pta[2]) > 0)
			{
				tmp = pta[1];
				pta[1] = pta[2];
				pta[2] = tmp;
			}
			if (cmp(&pta[0], &pta[1]) > 0)
			{
				tmp = pta[0];
				pta[0] = pta[1];
				pta[1] = tmp;
			}
			break;
		default:
			assert(nmemb - offset > 3);
	}

	pta = array;

	for (offset = 0 ; offset + 16 <= nmemb ; offset += 16)
	{
		if (cmp(&pta[3], &pta[4]) <= 0)
		{
			if (cmp(&pta[11], &pta[12]) <= 0)
			{
				if (cmp(&pta[7], &pta[8]) <= 0)
				{
					pta += 16;
					continue;
				}
				pts = swap;

				for (loop = 0 ; loop < 16 ; loop++)
					*pts++ = *pta++;

				goto step3;
			}
			pts = swap;

			for (loop = 0 ; loop < 8 ; loop++)
				*pts++ = *pta++;

			goto step2;
		}

		// step1:

		pts = swap;

		if (cmp(&pta[3], &pta[7]) <= 0)
		{
			ptt = pta + 4;

			for (loop = 0 ; loop < 5 ; loop++)
				if (cmp(pta, ptt) > 0)
					*pts++ = *ptt++;
				else
					*pts++ = *pta++;

			while (pta < array + offset + 4)
			{
				*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			}
			while (ptt < array + offset + 8)
			{
				*pts++ = *ptt++;
			}
			pta = ptt;
		}
		else if (cmp(&pta[0], &pta[7]) > 0)
		{
			if (cmp(&pta[8], &pta[15]) > 0)
			{
				if (cmp(&pta[4], &pta[11]) > 0)
				{
					tmp = pta[0]; pta[0] = pta[12]; pta[12] = tmp;
					tmp = pta[1]; pta[1] = pta[13];	pta[13] = tmp;
					tmp = pta[2]; pta[2] = pta[14]; pta[14] = tmp;
					tmp = pta[3]; pta[3] = pta[15]; pta[15] = tmp;
					tmp = pta[4]; pta[4] = pta[8]; pta[8] = tmp;
					tmp = pta[5]; pta[5] = pta[9]; pta[9] = tmp;
					tmp = pta[6]; pta[6] = pta[10]; pta[10] = tmp;
					tmp = pta[7]; pta[7] = pta[11]; pta[11] = tmp;

					pta += 16;
					continue;
				}
				pta += 4;

				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

				pta -= 8;

				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; 

				pta += 8;

				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

				pta -= 8;

				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; 

				pta += 4;

				goto step3;
			}
			pta += 4;

			*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

			pta -= 8;

			*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; 

			pta += 4;
		}
		else
		{
			ptt = pta + 4;

			for (loop = 0 ; loop < 5 ; loop++)
				if (cmp(pta, ptt) > 0)
					*pts++ = *ptt++;
				else
					*pts++ = *pta++;

			while (ptt < array + offset + 8)
			{
				*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			}
			while (pta < array + offset + 4)
			{
				*pts++ = *pta++;
			}
			pta = ptt;
		}

		step2:

		if (cmp(&pta[3], &pta[7]) <= 0)
		{
			ptt = pta + 4;

			for (loop = 0 ; loop < 4 ; loop++)
				*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;

			while (pta < array + offset + 12)
			{
				*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			}
			while (ptt < array + offset + 16)
			{
				*pts++ = *ptt++;
			}
		}
		else if (cmp(&pta[0], &pta[7]) > 0)
		{
			pta += 4;

			*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

			pta -= 8;

			*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
		}
		else
		{
			ptt = pta + 4;

			for (loop = 0 ; loop < 5 ; loop++)
				*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;

			while (ptt < array + offset + 16)
			{
				*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			}
			while (pta < array + offset + 12)
			{
				*pts++ = *pta++;
			}
		}

		step3:

		pta = array + offset;
		pts = swap;

		if (cmp(&pts[7], &pts[15]) <= 0)
		{
			ptt = pts + 8;

			for (loop = 0 ; loop < 8 ; loop++)
				*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

			while (pts < swap + 8)
			{
				*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			}
			while (ptt < swap + 16)
			{
				*pta++ = *ptt++;
			}
		}
		else if (cmp(&pts[0], &pts[15]) > 0)
		{
			pts += 8;

			for (loop = 0 ; loop < 8 ; loop++)
				*pta++ = *pts++;

			pts -= 16;

			for (loop = 0 ; loop < 8 ; loop++)
				*pta++ = *pts++;
		}
		else
		{
			ptt = pts + 8;

			for (loop = 0 ; loop < 9 ; loop++)
				*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

			while (ptt < swap + 16)
			{
				*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			}
			while (pts < swap + 8)
			{
				*pta++ = *pts++;
			}
		}
	}

	switch (nmemb - offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			return;
		default:
			quad_sort32(pta, swap, nmemb - offset, 4, cmp);
	}
}

void quad_sort64(long long *array, long long *swap, size_t nmemb, size_t block, CMPFUNC *cmp);

void quad_swap64(long long *array, long long *swap, size_t nmemb, CMPFUNC *cmp)
{
	size_t offset;
	register long long *pta, *pts, *ptt, tmp;

	pta = array;

	for (offset = 0 ; offset + 4 <= nmemb ; offset += 4)
	{
		if (cmp(&pta[0], &pta[1]) > 0)
		{
			tmp = pta[0];
			pta[0] = pta[1];
			pta[1] = tmp;
		}

		if (cmp(&pta[2], &pta[3]) > 0)
		{
			tmp = pta[2];
			pta[2] = pta[3];
			pta[3] = tmp;
		}

		if (cmp(&pta[1], &pta[2]) > 0)
		{
			if (cmp(&pta[0], &pta[3]) > 0)
			{
				tmp = pta[0];
				pta[0] = pta[2];
				pta[2] = tmp;

				tmp = pta[1];
				pta[1] = pta[3];
				pta[3] = tmp;
			}
			else if (cmp(&pta[0], &pta[2]) <= 0)
			{
				if (cmp(&pta[1], &pta[3]) <= 0)
				{
					tmp = pta[1];
					pta[1] = pta[2];
					pta[2] = tmp;
				}
				else
				{
					tmp = pta[1];
					pta[1] = pta[2];
					pta[2] = pta[3];
					pta[3] = tmp;
				}
			}
			else
			{
				if (cmp(&pta[1], &pta[3]) <= 0)
				{
					tmp = pta[0];
					pta[0] = pta[2];
					pta[2] = pta[1];
					pta[1] = tmp;
				}
				else
				{
					tmp = pta[0];
					pta[0] = pta[2];
					pta[2] = pta[3];
					pta[3] = pta[1];
					pta[1] = tmp;
				}
			}
		}
		pta += 4;
	}

	switch (nmemb - offset)
	{
		case 0:
		case 1:
			break;
		case 2:
			if (cmp(&pta[0], &pta[1]) > 0)
			{
				tmp = pta[0];
				pta[0] = pta[1];
				pta[1] = tmp;
			}
			break;
		case 3:
			if (cmp(&pta[0], &pta[1]) > 0)
			{
				tmp = pta[0];
				pta[0] = pta[1];
				pta[1] = tmp;
			}
			if (cmp(&pta[1], &pta[2]) > 0)
			{
				tmp = pta[1];
				pta[1] = pta[2];
				pta[2] = tmp;
			}
			if (cmp(&pta[0], &pta[1]) > 0)
			{
				tmp = pta[0];
				pta[0] = pta[1];
				pta[1] = tmp;
			}
			break;
		default:
			assert(nmemb - offset > 3);
	}

	pta = array;

	for (offset = 0 ; offset + 16 <= nmemb ; offset += 16)
	{
		if (cmp(&pta[3], &pta[4]) <= 0)
		{
			if (cmp(&pta[11], &pta[12]) <= 0)
			{
				if (cmp(&pta[7], &pta[8]) <= 0)
				{
					pta += 16;
					continue;
				}
				pts = swap;

				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

				goto step3;
			}
			pts = swap;

			*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
			*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

			goto step2;
		}

		// step1:

		pts = swap;

		if (cmp(&pta[3], &pta[7]) <= 0)
		{
			ptt = pta + 4;

			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;

			while (pta < array + offset + 4)
			{
				*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			}
			while (ptt < array + offset + 8)
			{
				*pts++ = *ptt++;
			}
			pta = ptt;
		}
		else if (cmp(&pta[0], &pta[7]) > 0)
		{
			if (cmp(&pta[8], &pta[15]) > 0)
			{
				if (cmp(&pta[4], &pta[11]) > 0)
				{
					tmp = pta[0]; pta[0] = pta[12]; pta[12] = tmp;
					tmp = pta[1]; pta[1] = pta[13];	pta[13] = tmp;
					tmp = pta[2]; pta[2] = pta[14]; pta[14] = tmp;
					tmp = pta[3]; pta[3] = pta[15]; pta[15] = tmp;
					tmp = pta[4]; pta[4] = pta[8]; pta[8] = tmp;
					tmp = pta[5]; pta[5] = pta[9]; pta[9] = tmp;
					tmp = pta[6]; pta[6] = pta[10]; pta[10] = tmp;
					tmp = pta[7]; pta[7] = pta[11]; pta[11] = tmp;

					pta += 16;
					continue;
				}
				pta += 4;

				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

				pta -= 8;

				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; 

				pta += 8;

				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

				pta -= 8;

				*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; 

				pta += 4;

				goto step3;
			}
			pta += 4;

			*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

			pta -= 8;

			*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; 

			pta += 4;
		}
		else
		{
			ptt = pta + 4;

			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;

			while (ptt < array + offset + 8)
			{
				*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			}
			while (pta < array + offset + 4)
			{
				*pts++ = *pta++;
			}
			pta = ptt;
		}

		step2:

		if (cmp(&pta[3], &pta[7]) <= 0)
		{
			ptt = pta + 4;

			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
//			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;

			while (pta < array + offset + 12)
			{
				*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			}
			while (ptt < array + offset + 16)
			{
				*pts++ = *ptt++;
			}
		}
		else if (cmp(&pta[0], &pta[7]) > 0)
		{
			pta += 4;

			*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;

			pta -= 8;

			*pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++; *pts++ = *pta++;
		}
		else
		{
			ptt = pta + 4;

			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;

			while (ptt < array + offset + 16)
			{
				*pts++ = cmp(pta, ptt) > 0 ? *ptt++ : *pta++;
			}
			while (pta < array + offset + 12)
			{
				*pts++ = *pta++;
			}
		}

		step3:

		pta = array + offset;
		pts = swap;

		if (cmp(&pts[7], &pts[15]) <= 0)
		{
			ptt = pts + 8;

			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
//			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

			while (pts < swap + 8)
			{
				*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			}
			while (ptt < swap + 16)
			{
				*pta++ = *ptt++;
			}
		}
		else if (cmp(&pts[0], &pts[15]) > 0)
		{
			pts += 8;

			*pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++;
			*pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++;

			pts -= 16;

			*pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++;
			*pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++; *pta++ = *pts++;
		}
		else
		{
			ptt = pts + 8;

			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;

			while (ptt < swap + 16)
			{
				*pta++ = cmp(pts, ptt) > 0 ? *ptt++ : *pts++;
			}
			while (pts < swap + 8)
			{
				*pta++ = *pts++;
			}
		}
	}

	switch (nmemb - offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			return;
		default:
			quad_sort64(pta, swap, nmemb - offset, 4, cmp);
	}
}

void quad_sort32(int *array, int *swap, size_t nmemb, size_t block, CMPFUNC *cmp)
{
	size_t offset;
	register int *pta, *pts, *c, *c_max, *d, *d_max;

	while (block < nmemb)
	{
		offset = 0;

		while (offset + block < nmemb)
		{
			pta = array;
			pta += offset;

			c_max = pta + block;

			if (cmp(c_max - 1, c_max) <= 0)
			{
				if (offset + block * 3 < nmemb)
				{
					c_max = pta + block * 3;

					if (cmp(c_max - 1, c_max) <= 0)
					{
						c_max = pta + block * 2;

						if (cmp(c_max - 1, c_max) <= 0)
						{
							offset += block * 4;
							continue;
						}
						pts = swap;

						c = pta;

						while (c < c_max - 8)
						{
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
						}
						while (c < c_max)
							*pts++ = *c++;

						c = c_max;
						c_max = offset + block * 4 <= nmemb ? c + block * 2 : array + nmemb;

						while (c < c_max - 8)
						{
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
						}
						while (c < c_max)
							*pts++ = *c++;

						goto step3;
					}
					pts = swap;

					c = pta;
					c_max = pta + block * 2;

					while (c < c_max)
						*pts++ = *c++;

					goto step2;
				}
				else if (offset + block * 2 < nmemb)
				{
					c_max = pta + block * 2;

					if (cmp(c_max - 1, c_max) <= 0)
					{
						offset += block * 4;
						continue;
					}
					pts = swap;

					c = pta;

					while (c < c_max - 8)
					{
						*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
						*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
					}
					while (c < c_max)
						*pts++ = *c++;

					goto step2;
				}
				else
				{
					offset += block * 4;
					continue;
				}
			}

			// step1:

			pts = swap;

			c = pta;

			d = c_max;
			d_max = offset + block * 2 <= nmemb ? d + block : array + nmemb;

			if (cmp(c_max - 1, d_max - 1) <= 0)
			{
				while (c < c_max)
				{
					while (cmp(c, d) > 0)
					{
						*pts++ = *d++;
					}
					*pts++ = *c++;
				}
				while (d < d_max)
					*pts++ = *d++;
			}
			else if (cmp(c, d_max - 1) > 0)
			{
				if (offset + block * 4 <= nmemb)
				{
					int *e, *e_max, *f, *f_max, tmp;

					e = pta + block * 2;
					e_max = e + block;
					f = e_max;
					f_max = f + block;

					if (cmp(e, f_max - 1) > 0)
					{
						if (cmp(d, f_max - 1) > 0)
						{
							while (c < c_max)
							{
								tmp = *c;
								*c++ = *f;
								*f++ = tmp;
							}
							while (d < d_max)
							{
								tmp = *d;
								*d++ = *e;
								*e++ = tmp;
							}
							offset += block * 4;
							continue;
						}
					}
				}

				while (d < d_max - 8)
				{
					*pts++ = *d++; *pts++ = *d++; *pts++ = *d++; *pts++ = *d++;
					*pts++ = *d++; *pts++ = *d++; *pts++ = *d++; *pts++ = *d++;
				}
				while (d < d_max)
					*pts++ = *d++;

				while (c < c_max - 8)
				{
					*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
					*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
				}
				while (c < c_max)
					*pts++ = *c++;
			}
			else
			{
				while (d < d_max)
				{
					while (cmp(c, d) <= 0)
					{
						*pts++ = *c++;
					}
					*pts++ = *d++;
				}

				while (c < c_max)
					*pts++ = *c++;
			}

			step2:

			if (offset + block * 2 < nmemb)
			{
				c = pta + block * 2;

				if (offset + block * 3 < nmemb)
				{
					c_max = c + block;
					d = c_max;
					d_max = offset + block * 4 <= nmemb ? d + block : array + nmemb;

					if (cmp(c_max - 1, d_max - 1) <= 0)
					{
						while (c < c_max)
						{
							while (cmp(c, d) > 0)
							{
								*pts++ = *d++;
							}
							*pts++ = *c++;
						}

						while (d < d_max)
							*pts++ = *d++;
					}
					else if (cmp(c, d_max - 1) > 0)
					{
						while (d < d_max - 8)
						{
							*pts++ = *d++; *pts++ = *d++; *pts++ = *d++; *pts++ = *d++;
							*pts++ = *d++; *pts++ = *d++; *pts++ = *d++; *pts++ = *d++;
						}
						while (d < d_max)
							*pts++ = *d++;

						while (c < c_max - 8)
						{
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
						}
						while (c < c_max)
							*pts++ = *c++;
					}
					else
					{
						while (d < d_max)
						{
							while (cmp(c, d) <= 0)
							{
								*pts++ = *c++;
							}
							*pts++ = *d++;
						}

						while (c < c_max)
							*pts++ = *c++;
					}
				}
				else
				{
					d = c;
					d_max = array + nmemb;

					pts = swap;
					c = pts;
					c_max = c + block * 2;

					goto quickstep;
				}
			}

			step3:

			pts = swap;

			c = pts;

			if (offset + block * 2 < nmemb)
			{
				c_max = c + block * 2;

				d = c_max;
				d_max = offset + block * 4 <= nmemb ? d + block * 2 : pts + nmemb - offset;

				quickstep:

				if (cmp(c_max - 1, d_max - 1) <= 0)
				{
					while (c < c_max)
					{
						while (cmp(c, d) > 0)
						{
							*pta++ = *d++;
						}
						*pta++ = *c++;
					}

					while (d < d_max)
						*pta++ = *d++;
				}
				else if (cmp(c, d_max - 1) > 0)
				{
					while (d < d_max - 16)
					{
						*pta++ = *d++; *pta++ = *d++; *pta++ = *d++; *pta++ = *d++;
						*pta++ = *d++; *pta++ = *d++; *pta++ = *d++; *pta++ = *d++;
						*pta++ = *d++; *pta++ = *d++; *pta++ = *d++; *pta++ = *d++;
						*pta++ = *d++; *pta++ = *d++; *pta++ = *d++; *pta++ = *d++;
					}
					while (d < d_max)
						*pta++ = *d++;

					while (c < c_max - 16)
					{
						*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
						*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
						*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
						*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
					}
					while (c < c_max)
						*pta++ = *c++;
				}
				else
				{
					while (d < d_max)
					{
						while (cmp(d, c) > 0)
						{
							*pta++ = *c++;
						}
						*pta++ = *d++;
					}

					while (c < c_max)
						*pta++ = *c++;
				}
			}
			else
			{
				d_max = pts + nmemb - offset;

				while (c < d_max - 8)
				{
					*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
					*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
				}
				while (c < d_max)
					*pta++ = *c++;
			}
			offset += block * 4;
		}
		block *= 4;
	}
}

void quad_sort64(long long *array, long long *swap, size_t nmemb, size_t block, CMPFUNC *cmp)
{
	size_t offset;
	register long long *pta, *pts, *c, *c_max, *d, *d_max;

	while (block < nmemb)
	{
		offset = 0;

		while (offset + block < nmemb)
		{
			pta = array;
			pta += offset;

			c_max = pta + block;

			if (cmp(c_max - 1, c_max) <= 0)
			{
				if (offset + block * 3 < nmemb)
				{
					c_max = pta + block * 3;

					if (cmp(c_max - 1, c_max) <= 0)
					{
						c_max = pta + block * 2;

						if (cmp(c_max - 1, c_max) <= 0)
						{
							offset += block * 4;
							continue;
						}
						pts = swap;

						c = pta;

						while (c < c_max - 8)
						{
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
						}
						while (c < c_max)
							*pts++ = *c++;

						c = c_max;
						c_max = offset + block * 4 <= nmemb ? c + block * 2 : array + nmemb;

						while (c < c_max - 8)
						{
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
						}
						while (c < c_max)
							*pts++ = *c++;

						goto step3;
					}
					pts = swap;

					c = pta;
					c_max = pta + block * 2;

					while (c < c_max)
						*pts++ = *c++;

					goto step2;
				}
				else if (offset + block * 2 < nmemb)
				{
					c_max = pta + block * 2;

					if (cmp(c_max - 1, c_max) <= 0)
					{
						offset += block * 4;
						continue;
					}
					pts = swap;

					c = pta;

					while (c < c_max - 8)
					{
						*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
						*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
					}
					while (c < c_max)
						*pts++ = *c++;

					goto step2;
				}
				else
				{
					offset += block * 4;
					continue;
				}
			}

			// step1:

			pts = swap;

			c = pta;

			d = c_max;
			d_max = offset + block * 2 <= nmemb ? d + block : array + nmemb;

			if (cmp(c_max - 1, d_max - 1) <= 0)
			{
				while (c < c_max)
				{
					while (cmp(c, d) > 0)
					{
						*pts++ = *d++;
					}
					*pts++ = *c++;
				}
				while (d < d_max)
					*pts++ = *d++;
			}
			else if (cmp(c, d_max - 1) > 0)
			{
				if (offset + block * 4 <= nmemb)
				{
					long long *e, *e_max, *f, *f_max, tmp;

					e = pta + block * 2;
					e_max = e + block;
					f = e_max;
					f_max = f + block;

					if (cmp(e, f_max - 1) > 0)
					{
						if (cmp(d, f_max - 1) > 0)
						{
							while (c < c_max)
							{
								tmp = *c;
								*c++ = *f;
								*f++ = tmp;
							}
							while (d < d_max)
							{
								tmp = *d;
								*d++ = *e;
								*e++ = tmp;
							}
							offset += block * 4;
							continue;
						}
					}
				}

				while (d < d_max - 8)
				{
					*pts++ = *d++; *pts++ = *d++; *pts++ = *d++; *pts++ = *d++;
					*pts++ = *d++; *pts++ = *d++; *pts++ = *d++; *pts++ = *d++;
				}
				while (d < d_max)
					*pts++ = *d++;

				while (c < c_max - 8)
				{
					*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
					*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
				}
				while (c < c_max)
					*pts++ = *c++;
			}
			else
			{
				while (d < d_max)
				{
					while (cmp(c, d) <= 0)
					{
						*pts++ = *c++;
					}
					*pts++ = *d++;
				}

				while (c < c_max)
					*pts++ = *c++;
			}

			step2:

			if (offset + block * 2 < nmemb)
			{
				c = pta + block * 2;

				if (offset + block * 3 < nmemb)
				{
					c_max = c + block;
					d = c_max;
					d_max = offset + block * 4 <= nmemb ? d + block : array + nmemb;

					if (cmp(c_max - 1, d_max - 1) <= 0)
					{
						while (c < c_max)
						{
							while (cmp(c, d) > 0)
							{
								*pts++ = *d++;
							}
							*pts++ = *c++;
						}

						while (d < d_max)
							*pts++ = *d++;
					}
					else if (cmp(c, d_max - 1) > 0)
					{
						while (d < d_max - 8)
						{
							*pts++ = *d++; *pts++ = *d++; *pts++ = *d++; *pts++ = *d++;
							*pts++ = *d++; *pts++ = *d++; *pts++ = *d++; *pts++ = *d++;
						}
						while (d < d_max)
							*pts++ = *d++;

						while (c < c_max - 8)
						{
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
							*pts++ = *c++; *pts++ = *c++; *pts++ = *c++; *pts++ = *c++;
						}
						while (c < c_max)
							*pts++ = *c++;
					}
					else
					{
						while (d < d_max)
						{
							while (cmp(c, d) <= 0)
							{
								*pts++ = *c++;
							}
							*pts++ = *d++;
						}

						while (c < c_max)
							*pts++ = *c++;
					}
				}
				else
				{
					d = c;
					d_max = array + nmemb;

					pts = swap;
					c = pts;
					c_max = c + block * 2;

					goto quickstep;
				}
			}

			step3:

			pts = swap;

			c = pts;

			if (offset + block * 2 < nmemb)
			{
				c_max = c + block * 2;

				d = c_max;
				d_max = offset + block * 4 <= nmemb ? d + block * 2 : pts + nmemb - offset;

				quickstep:

				if (cmp(c_max - 1, d_max - 1) <= 0)
				{
					while (c < c_max)
					{
						while (cmp(c, d) > 0)
						{
							*pta++ = *d++;
						}
						*pta++ = *c++;
					}

					while (d < d_max)
						*pta++ = *d++;
				}
				else if (cmp(c, d_max - 1) > 0)
				{
					while (d < d_max - 16)
					{
						*pta++ = *d++; *pta++ = *d++; *pta++ = *d++; *pta++ = *d++;
						*pta++ = *d++; *pta++ = *d++; *pta++ = *d++; *pta++ = *d++;
						*pta++ = *d++; *pta++ = *d++; *pta++ = *d++; *pta++ = *d++;
						*pta++ = *d++; *pta++ = *d++; *pta++ = *d++; *pta++ = *d++;
					}
					while (d < d_max)
						*pta++ = *d++;

					while (c < c_max - 16)
					{
						*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
						*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
						*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
						*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
					}
					while (c < c_max)
						*pta++ = *c++;
				}
				else
				{
					while (d < d_max)
					{
						while (cmp(d, c) > 0)
						{
							*pta++ = *c++;
						}
						*pta++ = *d++;
					}

					while (c < c_max)
						*pta++ = *c++;
				}
			}
			else
			{
				d_max = pts + nmemb - offset;

				while (c < d_max - 8)
				{
					*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
					*pta++ = *c++; *pta++ = *c++; *pta++ = *c++; *pta++ = *c++;
				}
				while (c < d_max)
					*pta++ = *c++;
			}
			offset += block * 4;
		}
		block *= 4;
	}
}

void quadsort(void *array, size_t nmemb, size_t size, CMPFUNC *cmp)
{
	void *swap;

	swap = malloc(nmemb * size);

	if (size == sizeof(int))
	{
		quad_swap32(array, swap, nmemb, cmp);
		quad_sort32(array, swap, nmemb, 16, cmp);
	}
	else if (size == sizeof(long long))
	{
		quad_swap64(array, swap, nmemb, cmp);
		quad_sort64(array, swap, nmemb, 16, cmp);
	}
	else
	{
		assert(size == 4 || size == 8);
	}

	free(swap);
}

int cmp_int(const void * a, const void * b)
{
	return *(int *) a - *(int *) b;
}

int cmp_str(const void * a, const void * b)
{
	return strcmp(*(const char **) a, *(const char **) b);
}

int cmp_float(const void * a, const void * b)
{
	return *(float *) a - *(float *) b;
}

int cmp_num(const void * a, const void * b)
{
	unsigned char isnum_a, isnum_b;

	isnum_a = is_number(*(char **) a);
	isnum_b = is_number(*(char **) b);

	if (isnum_a && isnum_b)
	{
		long double num_a = tintoi(*(char **) a);
		long double num_b = tintoi(*(char **) b);

		return num_a < num_b ? -1 : num_a > num_b ? 1 : 0;
	}
	else if (isnum_a)
	{
		return -1;
	}
	else if (isnum_b)
	{
		return 1;
	}
	else
	{
		return strcmp(*(const char **) a, *(const char **) b);
	}
}
