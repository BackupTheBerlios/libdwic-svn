/***************************************************************************
 *   Copyright (C) 2006 by Nicolas BOTTI <rududu@laposte.net>              *
 *                                                                         *
 * This software is a computer program whose purpose is to compress        *
 * images.                                                                 *
 *                                                                         *
 * This software is governed by the CeCILL  license under French law and   *
 * abiding by the rules of distribution of free software.  You can  use,   *
 * modify and/ or redistribute the software under the terms of the CeCILL  *
 * license as circulated by CEA, CNRS and INRIA at the following URL       *
 * "http://www.cecill.info".                                               *
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
 * knowledge of the CeCILL license and that you accept its terms.          *
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
	pData = new float [x * y + Align];
	float * pAllocated = (float*)(((int)pData + Align - 1) & (-Align));
	if (level > MAX_WAV_LEVEL)
		level = MAX_WAV_LEVEL;
	Init(level, pAllocated, Align);
}

DirWavelet::DirWavelet(int x, int y, int level,
					   DirWavelet * pHigh, void * pAllocated, int Align):
DimX(x),
DimY(y),
pHigh(0),
pLow(0),
pData(0)
{
	this->pHigh = pHigh;
	pHigh->DBand.pParent = &DBand;
	DBand.pChild = &pHigh->DBand;
	pHigh->HVBand.pParent = &HVBand;
	HVBand.pChild = &pHigh->HVBand;
	Init(level, pAllocated, Align);
}

DirWavelet::~DirWavelet()
{
	delete pLow;
	delete[] pData;
}

void DirWavelet::Init(int level,void * pAllocated, int Align)
{
	pAllocated = DBand.Init(DimX, DimY >> 1, (char *)pAllocated, Align);
	pAllocated = HVBand.Init(DimX >> 1, DimY >> 1, (char *)pAllocated, Align);
	if (level > 1){
		pLow = new DirWavelet(DimX >> 1, DimY >> 1, level - 1,
								   this, pAllocated, Align);
	}else{
		pAllocated = LBand.Init(DimX >> 1, DimY >> 1,
								(char *)pAllocated, Align);
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

void DirWavelet::LiftBlockOdd(float * pBlock, int Stride, float Coef)
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

void DirWavelet::LiftBlockEven(float * pBlock, int Stride, float Coef)
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

void DirWavelet::LiftBlockOddT(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_T(1,0);
	PXL_LIFT_T(3,0);
	PXL_LIFT(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(3,2);
	PXL_LIFT(0,3);
	PXL_LIFT(2,3);
}

void DirWavelet::LiftBlockEvenT(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_T(0,0);
	PXL_LIFT_T(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftBlockOddB(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,0);
	PXL_LIFT(3,0);
	PXL_LIFT(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(3,2);
	PXL_LIFT_B(0,3);
	PXL_LIFT_B(2,3);
}

void DirWavelet::LiftBlockEvenB(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT_B(1,3);
	PXL_LIFT_B(3,3);
}

void DirWavelet::LiftBlockOddL(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,0);
	PXL_LIFT(3,0);
	PXL_LIFT_L(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(3,2);
	PXL_LIFT_L(0,3);
	PXL_LIFT(2,3);
}

void DirWavelet::LiftBlockEvenL(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_L(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT_L(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftBlockOddR(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,0);
	PXL_LIFT_R(3,0);
	PXL_LIFT(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT_R(3,2);
	PXL_LIFT(0,3);
	PXL_LIFT(2,3);
}

void DirWavelet::LiftBlockEvenR(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT_R(3,1);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT(1,3);
	PXL_LIFT_R(3,3);
}

void DirWavelet::LiftBlockOddTL(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_T(1,0);
	PXL_LIFT_T(3,0);
	PXL_LIFT_L(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(3,2);
	PXL_LIFT_L(0,3);
	PXL_LIFT(2,3);
}

void DirWavelet::LiftBlockEvenTL(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_TL(0,0);
	PXL_LIFT_T(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT_L(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftBlockOddTR(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_T(1,0);
	PXL_LIFT_TR(3,0);
	PXL_LIFT(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT_R(3,2);
	PXL_LIFT(0,3);
	PXL_LIFT(2,3);
}

void DirWavelet::LiftBlockEvenTR(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_T(0,0);
	PXL_LIFT_T(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT_R(3,1);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT(1,3);
	PXL_LIFT_R(3,3);
}

void DirWavelet::LiftBlockOddBL(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,0);
	PXL_LIFT(3,0);
	PXL_LIFT_L(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(3,2);
	PXL_LIFT_BL(0,3);
	PXL_LIFT_B(2,3);
}

void DirWavelet::LiftBlockEvenBL(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_L(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT_L(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT_B(1,3);
	PXL_LIFT_B(3,3);
}

void DirWavelet::LiftBlockOddBR(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,0);
	PXL_LIFT_R(3,0);
	PXL_LIFT(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT_R(3,2);
	PXL_LIFT_B(0,3);
	PXL_LIFT_B(2,3);
}

void DirWavelet::LiftBlockEvenBR(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT_R(3,1);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
	PXL_LIFT_B(1,3);
	PXL_LIFT_BR(3,3);
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

void DirWavelet::LiftBlockDiagOdd(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftBlockDiagEven(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
}

void DirWavelet::LiftBlockDiagEvenT(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_T(0,0);
	PXL_LIFT_T(2,0);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
}

void DirWavelet::LiftBlockDiagOddB(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT_B(1,3);
	PXL_LIFT_B(3,3);
}

void DirWavelet::LiftBlockDiagEvenL(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_L(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT_L(0,2);
	PXL_LIFT(2,2);
}

void DirWavelet::LiftBlockDiagOddR(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,1);
	PXL_LIFT_R(3,1);
	PXL_LIFT(1,3);
	PXL_LIFT_R(3,3);
}

void DirWavelet::LiftBlockDiagOddBR(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,1);
	PXL_LIFT_R(3,1);
	PXL_LIFT_B(1,3);
	PXL_LIFT_BR(3,3);
}

void DirWavelet::LiftBlockDiagEvenTL(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT_TL(0,0);
	PXL_LIFT_T(2,0);
	PXL_LIFT_L(0,2);
	PXL_LIFT(2,2);
}

void DirWavelet::LiftBandDiagOdd(float * pBlock, int Stride, int DimX, int DimY,
							  float Coef)
{
	int j = 0;
	for( j; j < DimY - 4; j += 4){
		int i = 0;
		for( ; i < DimX - 4; i += 4){
			LiftBlockDiagOdd(pBlock + i, Stride, Coef);
		}
		LiftBlockDiagOddR(pBlock + i, Stride, Coef);
		pBlock += Stride << 2;
	}
	int i = 0;
	for( ; i < DimX - 4; i += 4){
		LiftBlockDiagOddB(pBlock + i, Stride, Coef);
	}
	LiftBlockDiagOddBR(pBlock + i, Stride, Coef);
}

void DirWavelet::LiftBandDiagEven(float * pBlock, int Stride, int DimX,
								int DimY, float Coef)
{
	LiftBlockDiagEvenTL(pBlock, Stride, Coef);
	int i = 4;
	for( ; i < DimX; i += 4){
		LiftBlockDiagEvenT(pBlock + i, Stride, Coef);
	}
	pBlock += Stride << 2;
	int j = 4;
	for( j; j < DimY; j += 4){
		LiftBlockDiagEvenL(pBlock, Stride, Coef);
		int i = 4;
		for( ; i < DimX; i += 4){
			LiftBlockDiagEven(pBlock + i, Stride, Coef);
		}
		pBlock += Stride << 2;
	}
}

void DirWavelet::LiftBandOdd(float * pBlock, int Stride, int DimX, int DimY,
						  float Coef)
{
	LiftBlockOddTL(pBlock, Stride, Coef);
	int i = 4;
	for( ; i < DimX - 4; i += 4){
		LiftBlockOddT(pBlock + i, Stride, Coef);
	}
	LiftBlockOddTR(pBlock + i, Stride, Coef);
	pBlock += Stride << 2;
	int j = 4;
	for( j; j < DimY - 4; j += 4){
		LiftBlockOddL(pBlock, Stride, Coef);
		int i = 4;
		for( ; i < DimX - 4; i += 4){
			LiftBlockOdd(pBlock + i, Stride, Coef);
		}
		LiftBlockOddR(pBlock + i, Stride, Coef);
		pBlock += Stride << 2;
	}
	LiftBlockOddBL(pBlock, Stride, Coef);
	i = 4;
	for( ; i < DimX - 4; i += 4){
		LiftBlockOddB(pBlock + i, Stride, Coef);
	}
	LiftBlockOddBR(pBlock + i, Stride, Coef);
}

void DirWavelet::LiftBandEven(float * pBlock, int Stride, int DimX, int DimY,
						  float Coef)
{
	LiftBlockEvenTL(pBlock, Stride, Coef);
	int i = 4;
	for( ; i < DimX - 4; i += 4){
		LiftBlockEvenT(pBlock + i, Stride, Coef);
	}
	LiftBlockEvenTR(pBlock + i, Stride, Coef);
	pBlock += Stride << 2;
	int j = 4;
	for( j; j < DimY - 4; j += 4){
		LiftBlockEvenL(pBlock, Stride, Coef);
		int i = 4;
		for( ; i < DimX - 4; i += 4){
			LiftBlockEven(pBlock + i, Stride, Coef);
		}
		LiftBlockEvenR(pBlock + i, Stride, Coef);
		pBlock += Stride << 2;
	}
	LiftBlockEvenBL(pBlock, Stride, Coef);
	i = 4;
	for( ; i < DimX - 4; i += 4){
		LiftBlockEvenB(pBlock + i, Stride, Coef);
	}
	LiftBlockEvenBR(pBlock + i, Stride, Coef);
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
	LiftBandOdd(pImage, Stride, DimX, DimY, -1./4.);
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
	LiftBandOdd(pImage, Stride, DimX, DimY, 1./4.);
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
