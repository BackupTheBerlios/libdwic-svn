/***************************************************************************
 *   Copyright (C) 2006 by Nicolas BOTTI <rududu@laposte.net>              *
 *                                                                         *
 * This software is a computer program whose purpose is to compress        *
 * images.                                                                 *
 *                                                                         *
 * This software is governed by the CeCILL v2 license under French law and *
 * abiding by the rules of distribution of free software.  You can  use,   *
 * modify and/ or redistribute the software under the terms of the         *
 * CeCILL v2 license as circulated by CEA, CNRS and INRIA at the following *
 * URL "http://www.cecill.info".                                           *
 *                                                                         *
 * As a counterpart to the access to the source code and  rights to copy,  *
 * modify and redistribute granted by the license, users are provided only *
 * with a limited warranty  and the software's author,  the holder of the  *
 * economic rights,  and the successive licensors  have only  limited      *
 * liability.                                                              *
 *                                                                         *
 * In this respect, the user's attention is drawn to the risks associated  *
 * with loading,  using,  modifying and/or developing or reproducing the   *
 * software by the user in light of its specific status of free software,  *
 * that may mean  that it is complicated to manipulate,  and  that  also   *
 * therefore means  that it is reserved for developers  and  experienced   *
 * professionals having in-depth computer knowledge. Users are therefore   *
 * encouraged to load and test the software's suitability as regards their *
 * requirements in conditions enabling the security of their systems       *
 * and/or data to be ensured and,  more generally, to use and operate it   *
 * in the same conditions as regards security.                             *
 *                                                                         *
 * The fact that you are presently reading this means that you have had    *
 * knowledge of the CeCILL v2 license and that you accept its terms.       *
 ***************************************************************************/

#pragma once

#include <stdio.h>

namespace libdwic {

typedef enum cmode {encode, decode};

#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(condition)	\
	if (!(condition)){	\
		char tmp[128];	\
		sprintf(tmp, "Assert failed in file : %s line %i\n",	\
 				__FILE__, __LINE__);	\
		DbgOut(tmp);	\
		__asm int 3		\
	}
#else
#define ASSERT(condition)
#endif
#endif

#define NO_RESET	0
#define RESET		1

#define UP		1
#define TOP		1
#define RIGHT	2
#define DOWN	4
#define BOTTOM	4
#define LEFT	8

#define NORIGHT		16
#define NOTOP		32

#define UP_PWR		0
#define RIGHT_PWR	1
#define DOWN_PWR	2
#define LEFT_PWR	3

#define UNSIGN_CLIP(NbToClip,Value)	\
	((NbToClip) > (Value) ? (Value) : (NbToClip))

#define CLIP(NbToClip,ValueMin,ValueMax)	\
	((NbToClip) > (ValueMax) ? (ValueMax) : ((NbToClip) < (ValueMin) ? (ValueMin) : (NbToClip)))

#define ABS(Number)					\
	((Number) < 0 ? -(Number) : (Number))

#define MAX(a,b)	\
	(((a) > (b)) ? (a) : (b))

#define MIN(a,b)	\
	(((a) > (b)) ? (b) : (a))

#define ALIGN	8

int inline s2u(int s)
{
	int u = -(2 * s + 1);
	u ^= u >> 31;
	return u;
}

int inline u2s(int u)
{
	return (u >> 1) ^ -(u & 1);
}

int inline s2u_(int s)
{
	int m = s >> 31;
	return (2 * s + m) ^ (m * 2);
}

int inline u2s_(int u)
{
	int m = (u << 31) >> 31;
	return ((u >> 1) + m) ^ m;
}

// from http://graphics.stanford.edu/~seander/bithacks.html
int inline bitcnt(int v)
{
	v -= (v >> 1) & 0x55555555;
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
	return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

extern const char log[];

int inline bitlen(unsigned int v)
{
	int r = 0;
	while (v >= 32) {
		r += 5;
		v >>= 5;
	}
	return r + log[v];
}

}

