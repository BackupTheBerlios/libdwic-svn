/***************************************************************************
*   Copyright (C) 2005 by Nicolas Botti                                   *
*   nico@nico-linux                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#pragma once

#include <stdio.h>

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

#ifndef NULL
#define NULL	0
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

#define CLIP(NbToClip,ValueMin, ValueMax)	\
	((NbToClip) > (ValueMax) ? (ValueMax) : ((NbToClip) < (ValueMin) ? (ValueMin) : (NbToClip)))

#define ABS(Number)					\
	((Number) < 0 ? -(Number) : (Number))

#define MAX(a,b)	\
	(((a) > (b)) ? (a) : (b))

#define ALIGN	8
