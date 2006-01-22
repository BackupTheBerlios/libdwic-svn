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

#include "libdwic.h"

namespace libdwic {

DirWavelet::DirWavelet(int x, int y, int level, int Align):
DimX(x),
DimY(y),
pHigh(0),
pLow(0),
pData(0)
{
// 	pData = new float [x * y + Align];
	if (level > MAX_WAV_LEVEL)
		level = MAX_WAV_LEVEL;
	Init(level, Align);
}

DirWavelet::DirWavelet(int x, int y, int level, DirWavelet * pHigh, int Align):
DimX(x),
DimY(y),
pHigh(0),
pLow(0),
pData(0)
{
	this->pHigh = pHigh;
// 	pHigh->DBand.pParent = &DBand;
// 	DBand.pChild = &pHigh->DBand;
// 	pHigh->HVBand.pParent = &HVBand;
// 	HVBand.pChild = &pHigh->HVBand;
	Init(level, Align);
}

DirWavelet::~DirWavelet()
{
	delete pLow;
	delete[] pData;
}

void DirWavelet::Init(int level, int Align)
{
	DBand.Init(DimX, DimY >> 1, Align);
	HVBand.Init(DimX >> 1, DimY >> 1, Align);
	HVMap.Init(DimX, DimY);
	DMap.Init(DimX, DimY);
	if (level > 1){
		pLow = new DirWavelet(DimX >> 1, DimY >> 1, level - 1, this, Align);
	}else{
		LBand.Init(DimX >> 1, DimY >> 1, Align);
	}
}

#define PXL_LIFT(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + (y) * Stride] \
		+ pBlock[(x) - 1 + (y) * Stride] \
		+ pBlock[(x) + ((y) + 1) * Stride] \
		+ pBlock[(x) + ((y) - 1) * Stride] ) * Coef;

#define PXL_LIFT_T(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + (y) * Stride] \
		+ pBlock[(x) - 1 + (y) * Stride] \
		+ pBlock[(x) + ((y) + 1) * Stride] * 2 ) * Coef;

#define PXL_LIFT_B(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + (y) * Stride] \
		+ pBlock[(x) - 1 + (y) * Stride] \
		+ pBlock[(x) + ((y) - 1) * Stride] * 2 ) * Coef;

#define PXL_LIFT_L(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + (y) * Stride] * 2 \
		+ pBlock[(x) + ((y) + 1) * Stride] \
		+ pBlock[(x) + ((y) - 1) * Stride] ) * Coef;

#define PXL_LIFT_R(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) - 1 + (y) * Stride] * 2 \
		+ pBlock[(x) + ((y) + 1) * Stride] \
		+ pBlock[(x) + ((y) - 1) * Stride] ) * Coef;

#define PXL_LIFT_TL(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + (y) * Stride] * 2 \
		+ pBlock[(x) + ((y) + 1) * Stride] * 2 ) * Coef;

#define PXL_LIFT_TR(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) - 1 + (y) * Stride] * 2 \
		+ pBlock[(x) + ((y) + 1) * Stride] * 2 ) * Coef;

#define PXL_LIFT_BL(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + (y) * Stride] * 2 \
		+ pBlock[(x) + ((y) - 1) * Stride] * 2 ) * Coef;

#define PXL_LIFT_BR(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) - 1 + (y) * Stride] * 2 \
		+ pBlock[(x) + ((y) - 1) * Stride] * 2 ) * Coef;

void DirWavelet::LiftOdd(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,0);
	PXL_LIFT(3,0);
	PXL_LIFT(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(3,2);
	PXL_LIFT(0,3);
	PXL_LIFT(2,3);
}

void DirWavelet::LiftOdd(float * pBlock, int Stride, float Coef, int BitField)
{
	if (BitField & TOP){
		PXL_LIFT_T(1,0);
	} else {
		PXL_LIFT(1,0);
	}
	if (BitField & TOP){
		if (BitField & RIGHT){
			PXL_LIFT_TR(3,0);
		} else {
			PXL_LIFT_T(3,0);
		}
	}else if (BitField & RIGHT) {
		PXL_LIFT_R(3,0);
	} else {
		PXL_LIFT(3,0);
	}
	if (BitField & LEFT){
		PXL_LIFT_L(0,1);
	} else {
		PXL_LIFT(0,1);
	}
	if (BitField & RIGHT) {
		PXL_LIFT_R(3,2);
	} else {
		PXL_LIFT(3,2);
	}
	if (BitField & BOTTOM) {
		if (BitField & LEFT){
			PXL_LIFT_BL(0,3);
		} else {
			PXL_LIFT_B(0,3);
		}
	} else if (BitField & LEFT) {
		PXL_LIFT_L(0,3);
	} else {
		PXL_LIFT(0,3);
	}
	if (BitField & BOTTOM) {
		PXL_LIFT_B(2,3);
	} else {
		PXL_LIFT(2,3);
	}
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
}

void DirWavelet::LiftEven(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftEven(float * pBlock, int Stride, float Coef, int BitField)
{
	if (BitField & TOP) {
		if (BitField & LEFT) {
			PXL_LIFT_TL(0,0);
		} else {
			PXL_LIFT_T(0,0);
		}
	} else if (BitField & LEFT) {
		PXL_LIFT_L(0,0);
	} else {
		PXL_LIFT(0,0);
	}
	if (BitField & TOP) {
		PXL_LIFT_T(2,0);
	} else {
		PXL_LIFT(2,0);
	}
	if (BitField & RIGHT) {
		PXL_LIFT_R(3,1);
	} else {
		PXL_LIFT(3,1);
	}
	if (BitField & LEFT) {
		PXL_LIFT_L(0,2);
	} else {
		PXL_LIFT(0,2);
	}
	if (BitField & BOTTOM) {
		PXL_LIFT_B(1,3);
	} else {
		PXL_LIFT(1,3);
	}
	if (BitField & BOTTOM) {
		if (BitField & RIGHT) {
			PXL_LIFT_BR(3,3);
		} else {
			PXL_LIFT_B(3,3);
		}
	} else if (BitField & RIGHT) {
		PXL_LIFT_R(3,3);
	} else {
		PXL_LIFT(3,3);
	}
	PXL_LIFT(1,1);
	PXL_LIFT(2,2);
}

#undef PXL_LIFT
#undef PXL_LIFT_T
#undef PXL_LIFT_B
#undef PXL_LIFT_L
#undef PXL_LIFT_R
#undef PXL_LIFT_TL
#undef PXL_LIFT_BL
#undef PXL_LIFT_TR
#undef PXL_LIFT_BR

#define PXL_LIFT(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + ((y) + 1) * Stride] \
		+ pBlock[(x) - 1 + ((y) + 1) * Stride] \
		+ pBlock[(x) + 1 + ((y) - 1) * Stride] \
		+ pBlock[(x) - 1 + ((y) - 1) * Stride] ) * Coef;

#define PXL_LIFT_T(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + ((y) + 1) * Stride] \
		+ pBlock[(x) - 1 + ((y) + 1) * Stride] ) * 2 * Coef;

#define PXL_LIFT_B(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + ((y) - 1) * Stride] \
		+ pBlock[(x) - 1 + ((y) - 1) * Stride] ) * 2 * Coef;

#define PXL_LIFT_L(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + ((y) + 1) * Stride] \
		+ pBlock[(x) + 1 + ((y) - 1) * Stride] ) * 2 * Coef;

#define PXL_LIFT_R(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) - 1 + ((y) + 1) * Stride] \
		+ pBlock[(x) - 1 + ((y) - 1) * Stride] ) * 2 * Coef;

#define PXL_LIFT_TL(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + ((y) + 1) * Stride]) * 4 \
		* Coef;

#define PXL_LIFT_TR(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) - 1 + ((y) + 1) * Stride]) * 4 \
		* Coef;

#define PXL_LIFT_BL(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + ((y) - 1) * Stride]) * 4 \
		* Coef;

#define PXL_LIFT_BR(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) - 1 + ((y) - 1) * Stride]) * 4 \
		* Coef;

void DirWavelet::LiftDiagOdd(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftDiagEven(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
}

void DirWavelet::LiftDiagOdd(float * pBlock, int Stride, float Coef,
							 int BitField)
{
	PXL_LIFT(1,1);
	if (BitField & RIGHT) {
		PXL_LIFT_R(3,1);
	} else {
		PXL_LIFT(3,1);
	}
	if (BitField & BOTTOM) {
		PXL_LIFT_B(1,3);
	} else {
		PXL_LIFT(1,3);
	}
	if (BitField & BOTTOM) {
		if (BitField & RIGHT) {
			PXL_LIFT_BR(3,3);
		} else {
			PXL_LIFT_B(3,3);
		}
	} else if (BitField & RIGHT) {
		PXL_LIFT_R(3,3);
	} else {
		PXL_LIFT(3,3);
	}
}

void DirWavelet::LiftDiagEven(float * pBlock, int Stride, float Coef,
							  int BitField)
{
	if (BitField & TOP) {
		if (BitField & LEFT) {
			PXL_LIFT_TL(0,0);
		} else {
			PXL_LIFT_T(0,0);
		}
	} else if (BitField & LEFT) {
		PXL_LIFT_L(0,0);
	} else {
		PXL_LIFT(0,0);
	}
	if (BitField & TOP) {
		PXL_LIFT_T(2,0);
	} else {
		PXL_LIFT(2,0);
	}
	if (BitField & LEFT) {
		PXL_LIFT_L(0,2);
	} else {
		PXL_LIFT(0,2);
	}
	PXL_LIFT(2,2);
}

#undef PXL_LIFT
#undef PXL_LIFT_T
#undef PXL_LIFT_B
#undef PXL_LIFT_L
#undef PXL_LIFT_R
#undef PXL_LIFT_TL
#undef PXL_LIFT_BL
#undef PXL_LIFT_TR
#undef PXL_LIFT_BR

#define PXL_LIFT(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + (y) * Stride] \
		+ pBlock[(x) - 1 + (y) * Stride]) * Coef;

#define PXL_LIFT_L(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) + 1 + (y) * Stride] * 2 * Coef;

#define PXL_LIFT_R(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) - 1 + (y) * Stride] * 2 * Coef;

void DirWavelet::LiftHOdd(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,0);
	PXL_LIFT(3,0);
	PXL_LIFT(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(3,2);
	PXL_LIFT(0,3);
	PXL_LIFT(2,3);
}

void DirWavelet::LiftHOdd(float * pBlock, int Stride, float Coef, int BitField)
{
	if (BitField & LEFT) {
		PXL_LIFT_L(0,1);
		PXL_LIFT_L(0,3);
	} else {
		PXL_LIFT(0,3);
		PXL_LIFT(0,1);
	}
	if (BitField & RIGHT) {
		PXL_LIFT_R(3,0);
		PXL_LIFT_R(3,2);
	} else {
		PXL_LIFT(3,0);
		PXL_LIFT(3,2);
	}
	PXL_LIFT(1,0);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(2,3);
}

void DirWavelet::LiftHEven(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftHEven(float * pBlock, int Stride, float Coef, int BitField)
{
	if (BitField & LEFT) {
		PXL_LIFT_L(0,0);
		PXL_LIFT_L(0,2);
	} else {
		PXL_LIFT(0,0);
		PXL_LIFT(0,2);
	}
	if (BitField & RIGHT) {
		PXL_LIFT_R(3,1);
		PXL_LIFT_R(3,3);
	} else {
		PXL_LIFT(3,1);
		PXL_LIFT(3,3);
	}
	PXL_LIFT(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT(2,2);
	PXL_LIFT(1,3);
}

#undef PXL_LIFT
#undef PXL_LIFT_L
#undef PXL_LIFT_R

#define PXL_LIFT(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + ((y) + 1) * Stride] \
		+ pBlock[(x) + ((y) - 1) * Stride]) * Coef;

#define PXL_LIFT_T(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) + ((y) + 1) * Stride] * 2 * Coef;

#define PXL_LIFT_B(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) + ((y) - 1) * Stride] * 2 * Coef;

void DirWavelet::LiftVOdd(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,0);
	PXL_LIFT(3,0);
	PXL_LIFT(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(3,2);
	PXL_LIFT(0,3);
	PXL_LIFT(2,3);
}

void DirWavelet::LiftVOdd(float * pBlock, int Stride, float Coef, int BitField)
{
	if (BitField & TOP){
		PXL_LIFT_T(1,0);
		PXL_LIFT_T(3,0);
	} else {
		PXL_LIFT(1,0);
		PXL_LIFT(3,0);
	}
	if (BitField & BOTTOM){
		PXL_LIFT_B(0,3);
		PXL_LIFT_B(2,3);
	} else {
		PXL_LIFT(0,3);
		PXL_LIFT(2,3);
	}
	PXL_LIFT(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(3,2);
}

void DirWavelet::LiftVEven(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftVEven(float * pBlock, int Stride, float Coef, int BitField)
{
	if (BitField & TOP){
		PXL_LIFT_T(0,0);
		PXL_LIFT_T(2,0);
	} else {
		PXL_LIFT(0,0);
		PXL_LIFT(2,0);
	}
	if (BitField & BOTTOM){
		PXL_LIFT_B(1,3);
		PXL_LIFT_B(3,3);
	} else {
		PXL_LIFT(1,3);
		PXL_LIFT(3,3);
	}
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
}

#undef PXL_LIFT
#undef PXL_LIFT_T
#undef PXL_LIFT_B

#define PXL_LIFT(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + ((y) + 1) * Stride] \
		+ pBlock[(x) - 1 + ((y) - 1) * Stride]) * Coef;

#define PXL_LIFT_TL(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) + 1 + ((y) + 1) * Stride] \
		* 2 * Coef;

#define PXL_LIFT_BR(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) - 1 + ((y) - 1) * Stride] \
		* 2 * Coef;

void DirWavelet::LiftDiag1Odd(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftDiag1Odd(float * pBlock, int Stride, float Coef,
							  int BitField)
{
	PXL_LIFT(1,1);
	if (BitField & RIGHT) {
		PXL_LIFT_BR(3,1);
	} else {
		PXL_LIFT(3,1);
	}
	if (BitField & BOTTOM) {
		PXL_LIFT_BR(1,3);
	} else {
		PXL_LIFT(1,3);
	}
	if (BitField & (BOTTOM | RIGHT)) {
		PXL_LIFT_BR(3,3);
	} else {
		PXL_LIFT(3,3);
	}
}

void DirWavelet::LiftDiag1Even(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
}

void DirWavelet::LiftDiag1Even(float * pBlock, int Stride, float Coef,
							   int BitField)
{
	if (BitField & (TOP | LEFT)) {
		PXL_LIFT_TL(0,0);
	} else {
		PXL_LIFT(0,0);
	}
	if (BitField & TOP) {
		PXL_LIFT_TL(2,0);
	} else {
		PXL_LIFT(2,0);
	}
	if (BitField & LEFT) {
		PXL_LIFT_TL(0,2);
	} else {
		PXL_LIFT(0,2);
	}
	PXL_LIFT(2,2);
}

#undef PXL_LIFT
#undef PXL_LIFT_TL
#undef PXL_LIFT_BR

#define PXL_LIFT(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) - 1 + ((y) + 1) * Stride] \
		+ pBlock[(x) + 1 + ((y) - 1) * Stride]) * Coef;

#define PXL_LIFT_TR(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) - 1 + ((y) + 1) * Stride] \
		* 2 * Coef;

#define PXL_LIFT_BL(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) + 1 + ((y) - 1) * Stride] \
		* 2 * Coef;

void DirWavelet::LiftDiag2Odd(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftDiag2Odd(float * pBlock, int Stride, float Coef,
							  int BitField)
{
	PXL_LIFT(1,1);
	if (BitField & RIGHT) {
		PXL_LIFT_TR(3,1);
	} else {
		PXL_LIFT(3,1);
	}
	if (BitField & BOTTOM) {
		PXL_LIFT_BL(1,3);
	} else {
		PXL_LIFT(1,3);
	}
	if (BitField & RIGHT) {
		if (! (BitField & BOTTOM)) {
			PXL_LIFT_TR(3,3);
		}
	} else if (BitField & BOTTOM) {
		PXL_LIFT_BL(3,3);
	} else {
		PXL_LIFT(3,3);
	}
}

void DirWavelet::LiftDiag2Even(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
}

void DirWavelet::LiftDiag2Even(float * pBlock, int Stride, float Coef,
							   int BitField)
{
	if (BitField & TOP) {
		if (! (BitField & LEFT)) {
			PXL_LIFT_TR(0,0);
		}
	} else if (BitField & LEFT) {
		PXL_LIFT_BL(0,0);
	} else {
		PXL_LIFT(0,0);
	}
	if (BitField & TOP) {
		PXL_LIFT_TR(2,0);
	} else {
		PXL_LIFT(2,0);
	}
	if (BitField & LEFT) {
		PXL_LIFT_BL(0,2);
	} else {
		PXL_LIFT(0,2);
	}
	PXL_LIFT(2,2);
}

#undef PXL_LIFT
#undef PXL_LIFT_TR
#undef PXL_LIFT_BL

void DirWavelet::LiftBandDiagOdd(float * pBlock, int Stride, int DimX, int DimY,
							  float Coef)
{
// 	int j = 0;
// 	for( j; j < DimY - 4; j += 4){
// 		int i = 0;
// 		for( ; i < DimX - 4; i += 4){
// 			LiftDiagOdd(pBlock + i, Stride, Coef);
// 		}
// 		LiftDiagOddR(pBlock + i, Stride, Coef);
// 		pBlock += Stride << 2;
// 	}
// 	int i = 0;
// 	for( ; i < DimX - 4; i += 4){
// 		LiftDiagOddB(pBlock + i, Stride, Coef);
// 	}
// 	LiftDiagOddBR(pBlock + i, Stride, Coef);
}

void DirWavelet::LiftBandDiagEven(float * pBlock, int Stride, int DimX,
								int DimY, float Coef)
{
// 	LiftDiagEvenTL(pBlock, Stride, Coef);
// 	int i = 4;
// 	for( ; i < DimX; i += 4){
// 		LiftDiagEvenT(pBlock + i, Stride, Coef);
// 	}
// 	pBlock += Stride << 2;
// 	int j = 4;
// 	for( j; j < DimY; j += 4){
// 		LiftDiagEvenL(pBlock, Stride, Coef);
// 		int i = 4;
// 		for( ; i < DimX; i += 4){
// 			LiftDiagEven(pBlock + i, Stride, Coef);
// 		}
// 		pBlock += Stride << 2;
// 	}
}

// void DirWavelet::LiftBandOdd(float * pBlock, int Stride, int DimX, int DimY,
// 						  float Coef)
// {
// 	LiftOddTL(pBlock, Stride, Coef);
// 	int i = 4;
// 	for( ; i < DimX - 4; i += 4){
// 		LiftOddT(pBlock + i, Stride, Coef);
// 	}
// 	LiftOddTR(pBlock + i, Stride, Coef);
// 	pBlock += Stride << 2;
// 	int j = 4;
// 	for( j; j < DimY - 4; j += 4){
// 		LiftOddL(pBlock, Stride, Coef);
// 		int i = 4;
// 		for( ; i < DimX - 4; i += 4){
// 			LiftOdd(pBlock + i, Stride, Coef);
// 		}
// 		LiftOddR(pBlock + i, Stride, Coef);
// 		pBlock += Stride << 2;
// 	}
// 	LiftOddBL(pBlock, Stride, Coef);
// 	i = 4;
// 	for( ; i < DimX - 4; i += 4){
// 		LiftOddB(pBlock + i, Stride, Coef);
// 	}
// 	LiftOddBR(pBlock + i, Stride, Coef);
// }

void DirWavelet::LiftBandOdd(float * pBlock, int Stride, int DimX, int DimY,
							 float Coef, DirValue * pDir)
{
	void (*LiftOdd1[3])(float*, int, float, int);
	void (*LiftOdd2[3])(float*, int, float);
	LiftOdd1[0] = DirWavelet::LiftHOdd;
	LiftOdd1[1] = DirWavelet::LiftVOdd;
	LiftOdd1[2] = DirWavelet::LiftOdd;

	LiftOdd2[0] = DirWavelet::LiftHOdd;
	LiftOdd2[1] = DirWavelet::LiftVOdd;
	LiftOdd2[2] = DirWavelet::LiftOdd;

	(*LiftOdd1[pDir->Selected])(pBlock, Stride, Coef, TOP | LEFT);
	int i = 4;
	pDir++;
	for( ; i < DimX - 4; i += 4, pDir++){
		(*LiftOdd1[pDir->Selected])(pBlock + i, Stride, Coef, TOP);
	}
	(*LiftOdd1[pDir->Selected])(pBlock + i, Stride, Coef, TOP | RIGHT);
	pBlock += Stride << 2;
	pDir++;
	int j = 4;
	for( j; j < DimY - 4; j += 4){
		(*LiftOdd1[pDir->Selected])(pBlock, Stride, Coef, LEFT);
		int i = 4;
		pDir++;
		for( ; i < DimX - 4; i += 4, pDir++){
			(*LiftOdd2[pDir->Selected])(pBlock + i, Stride, Coef);
		}
		(*LiftOdd1[pDir->Selected])(pBlock + i, Stride, Coef, RIGHT);
		pBlock += Stride << 2;
		pDir++;
	}
	(*LiftOdd1[pDir->Selected])(pBlock, Stride, Coef, BOTTOM | LEFT);
	i = 4;
	pDir++;
	for( ; i < DimX - 4; i += 4, pDir++){
		(*LiftOdd1[pDir->Selected])(pBlock + i, Stride, Coef, BOTTOM);
	}
	(*LiftOdd1[pDir->Selected])(pBlock + i, Stride, Coef, BOTTOM | RIGHT);
}

void DirWavelet::LiftBandEven(float * pBlock, int Stride, int DimX, int DimY,
						  float Coef)
{
// 	LiftEvenTL(pBlock, Stride, Coef);
// 	int i = 4;
// 	for( ; i < DimX - 4; i += 4){
// 		LiftEvenT(pBlock + i, Stride, Coef);
// 	}
// 	LiftEvenTR(pBlock + i, Stride, Coef);
// 	pBlock += Stride << 2;
// 	int j = 4;
// 	for( j; j < DimY - 4; j += 4){
// 		LiftEvenL(pBlock, Stride, Coef);
// 		int i = 4;
// 		for( ; i < DimX - 4; i += 4){
// 			LiftEven(pBlock + i, Stride, Coef);
// 		}
// 		LiftEvenR(pBlock + i, Stride, Coef);
// 		pBlock += Stride << 2;
// 	}
// 	LiftEvenBL(pBlock, Stride, Coef);
// 	i = 4;
// 	for( ; i < DimX - 4; i += 4){
// 		LiftEvenB(pBlock + i, Stride, Coef);
// 	}
// 	LiftEvenBR(pBlock + i, Stride, Coef);
}

void DirWavelet::LazyImage(float * pImage, unsigned int Stride){
	int Dpos, HVpos, Ipos1, Ipos2, LStride;
	float * pLOut;
	if (pLow == 0){
		pLOut = LBand.pBand;
		LStride = LBand.DimX;
	}else{
		pLOut = pImage;
		LStride = Stride;
	}

	for(int j = 0; j < DimY; j += 2){
		Dpos = (j >> 1) * DBand.DimX;
		HVpos = (j >> 1) * HVBand.DimX;
		Ipos1 = j * Stride;
		Ipos2 = (j >> 1) * LStride;
		for(int pEnd = Ipos1 + DimX; Ipos1 < pEnd; Ipos1 += 2){
			pLOut[Ipos2] = pImage[Ipos1];
			DBand.pBand[Dpos + 1] = pImage[Ipos1 + 1];
			DBand.pBand[Dpos] = pImage[Ipos1 + Stride];
			HVBand.pBand[HVpos] = pImage[Ipos1 + Stride + 1];
			Dpos += 2;
			HVpos++;
			Ipos2++;
		}
	}
}

void DirWavelet::LazyImageI(float * pImage, unsigned int Stride){
	int Dpos, HVpos, Ipos1, Ipos2, LStride;
	float * pLIn;
	if (pLow == 0){
		pLIn = LBand.pBand;
		LStride = LBand.DimX;
	}else{
		pLIn = pImage;
		LStride = Stride;
	}

	for(int j = DimY - 2; j >= 0; j -= 2){
		Dpos = ((j >> 1) + 1) * DBand.DimX - 2;
		HVpos = ((j >> 1) + 1) * HVBand.DimX - 1;
		Ipos1 = j * Stride + DimX - 2;
		Ipos2 = (j >> 1) * LStride + (DimX >> 1) - 1;
		for(int pEnd = Ipos1 - DimX; Ipos1 > pEnd; Ipos1 -= 2){
			pImage[Ipos1] = pLIn[Ipos2];
			pImage[Ipos1 + 1] = DBand.pBand[Dpos + 1];
			pImage[Ipos1 + Stride] = DBand.pBand[Dpos];
			pImage[Ipos1 + Stride + 1] = HVBand.pBand[HVpos];
			Dpos -= 2;
			HVpos--;
			Ipos2--;
		}
	}
}

void DirWavelet::LazyTransform(float * pImage, int Stride)
{
	LazyImage(pImage, Stride);
	if (pLow != 0)
		pLow->LazyTransform(pImage, Stride);
}

void DirWavelet::LazyTransformI(float * pImage, int Stride)
{
	if (pLow != 0)
		pLow->LazyTransformI(pImage, Stride);
	LazyImageI(pImage, Stride);
}

void DirWavelet::Transform53(float * pImage, int Stride){
	HVMap.GetImageDir(pImage, Stride);
	LiftBandOdd(pImage, Stride, DimX, DimY, -1./4., HVMap.pMap);
	LiftBandEven(pImage, Stride, DimX, DimY, 1./8.);
	LiftBandDiagOdd(pImage, Stride, DimX, DimY, -1./4.);
	LiftBandDiagEven(pImage, Stride, DimX, DimY, 1./8.);
	LazyImage(pImage, Stride);
	if (pHigh != 0){
		DBand.Weight = pHigh->HVBand.Weight * 1.414213562;
		HVBand.Weight = pHigh->LBand.Weight;
		LBand.Weight = HVBand.Weight * 2;
	}else{
		DBand.Weight = 1./1.414213562;
		HVBand.Weight = 1.;
		LBand.Weight = 2;
	}
 	if (pLow != 0)
		pLow->Transform53(pImage, Stride);
}

void DirWavelet::Transform53I(float * pImage, int Stride){
	if (pLow != 0)
		pLow->Transform53I(pImage, Stride);
	LazyImageI(pImage, Stride);
	LiftBandDiagEven(pImage, Stride, DimX, DimY, -1./8.);
	LiftBandDiagOdd(pImage, Stride, DimX, DimY, 1./4.);
	LiftBandEven(pImage, Stride, DimX, DimY, -1./8.);
	LiftBandOdd(pImage, Stride, DimX, DimY, 1./4., HVMap.pMap);
}

unsigned int DirWavelet::Thres(float Thres){
	unsigned int Count = 0;
	Count += DBand.Thres(Thres);
	Count += HVBand.Thres(Thres);
	if (pLow != 0)
		Count += pLow->Thres(Thres);
	return Count;
}

unsigned int DirWavelet::TSUQ(float Quant, float Thres, float RecLevel){
	unsigned int Count = 0;
	Count += DBand.TSUQ(Quant, Thres, RecLevel);
	Count += HVBand.TSUQ(Quant, Thres, RecLevel);
	if (pLow != 0)
		Count += pLow->TSUQ(Quant, Thres, RecLevel);
	else
		Count += LBand.TSUQ(Quant, Quant * .5, Quant * .5);
	return Count;
}

}
