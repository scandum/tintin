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

// quadsort 1.2.1.3

#define QUAD_CACHE 262144

// utilize branchless ternary operations in clang

#if !defined __clang__
#define head_branchless_merge(ptd, x, ptl, ptr, cmp)  \
	x = cmp(ptl, ptr) <= 0;  \
	*ptd = *ptl;  \
	ptl += x;  \
	ptd[x] = *ptr;  \
	ptr += !x;  \
	ptd++;
#else
#define head_branchless_merge(ptd, x, ptl, ptr, cmp)  \
	*ptd++ = cmp(ptl, ptr) <= 0 ? *ptl++ : *ptr++;
#endif

#if !defined __clang__
#define tail_branchless_merge(tpd, y, tpl, tpr, cmp)  \
	y = cmp(tpl, tpr) <= 0;  \
	*tpd = *tpl;  \
	tpl -= !y;  \
	tpd--;  \
	tpd[y] = *tpr;  \
	tpr -= y;
#else
#define tail_branchless_merge(tpd, x, tpl, tpr, cmp)  \
	*tpd-- = cmp(tpl, tpr) > 0 ? *tpl-- : *tpr--;
#endif

// guarantee small parity merges are inlined with minimal overhead

#define parity_merge_two(array, swap, x, ptl, ptr, pts, cmp)  \
	ptl = array; ptr = array + 2; pts = swap;  \
	head_branchless_merge(pts, x, ptl, ptr, cmp);  \
	*pts = cmp(ptl, ptr) <= 0 ? *ptl : *ptr;  \
  \
	ptl = array + 1; ptr = array + 3; pts = swap + 3;  \
	tail_branchless_merge(pts, x, ptl, ptr, cmp);  \
	*pts = cmp(ptl, ptr)  > 0 ? *ptl : *ptr;

#define parity_merge_four(array, swap, x, ptl, ptr, pts, cmp)  \
	ptl = array + 0; ptr = array + 4; pts = swap;  \
	head_branchless_merge(pts, x, ptl, ptr, cmp);  \
	head_branchless_merge(pts, x, ptl, ptr, cmp);  \
	head_branchless_merge(pts, x, ptl, ptr, cmp);  \
	*pts = cmp(ptl, ptr) <= 0 ? *ptl : *ptr;  \
  \
	ptl = array + 3; ptr = array + 7; pts = swap + 7;  \
	tail_branchless_merge(pts, x, ptl, ptr, cmp);  \
	tail_branchless_merge(pts, x, ptl, ptr, cmp);  \
	tail_branchless_merge(pts, x, ptl, ptr, cmp);  \
	*pts = cmp(ptl, ptr)  > 0 ? *ptl : *ptr;


#if !defined __clang__
#define branchless_swap(pta, swap, x, cmp)  \
	x = cmp(pta, pta + 1) > 0;  \
	swap = pta[!x];  \
	pta[0] = pta[x];  \
	pta[1] = swap;
#else
#define branchless_swap(pta, swap, x, cmp)  \
	x = 0;  \
	swap = cmp(pta, pta + 1) > 0 ? pta[x++] : pta[1];  \
	pta[0] = pta[x];  \
	pta[1] = swap;
#endif

#define swap_branchless(pta, swap, x, y, cmp)  \
	x = cmp(pta, pta + 1) > 0;  \
	y = !x;  \
	swap = pta[y];  \
	pta[0] = pta[x];  \
	pta[1] = swap;

// 32 bit

#define VAR int
#define FUNC(NAME) NAME##32

#include "quadsort.h"

#undef VAR
#undef FUNC

// 64 bit

#define VAR long long
#define FUNC(NAME) NAME##64

#include "quadsort.h"

#undef VAR
#undef FUNC

void quadsort(void *array, size_t nmemb, size_t size, CMPFUNC *cmp)
{
	if (nmemb < 2)
	{
		return;
	}

	switch (size)
	{
		case sizeof(int):
			quadsort32(array, nmemb, cmp);
			return;

		case sizeof(long long):
			quadsort64(array, nmemb, cmp);
			return;

		default:
			qsort(array, nmemb, size, cmp);
			return;
	}
}

#undef QUAD_CACHE

int cmp_int(const void * a, const void * b)
{
	return *(int *) a > *(int *) b;
}

int cmp_str(const void * a, const void * b)
{
	return strcmp(*(const char **) a, *(const char **) b);
}

int cmp_list_str(const void * a, const void * b)
{
	struct listnode *node_a = *(struct listnode **) a;
	struct listnode *node_b = *(struct listnode **) b;

	return strcmp(node_a->arg2, node_b->arg2);
}

int cmp_list_num(const void * a, const void * b)
{
	struct listnode *node_a = *(struct listnode **) a;
	struct listnode *node_b = *(struct listnode **) b;

	return cmp_num(&node_a->arg2, &node_b->arg2);
}

int cmp_num(const void * a, const void * b)
{
	unsigned char isnum_a, isnum_b;
	char *str_a = *(char **) a;
	char *str_b = *(char **) b;

	isnum_a = is_number(str_a);
	isnum_b = is_number(str_b);

	if (isnum_a && isnum_b)
	{
		long double num_a = tintoi(str_a);
		long double num_b = tintoi(str_b);

		return num_a > num_b;
	}
	if (isnum_a)
	{
		return -1;
	}
	if (isnum_b)
	{
		return 1;
	}
	return strcmp(str_a, str_b);
}
