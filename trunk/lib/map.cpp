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

#include "global.h"
#include "map.h"

namespace libdwic {

CMap::CMap(void):
		pMap(0)
{
	Init();
}


CMap::~CMap()
{
	delete[] pMap;
}

void CMap::Init(int DimX, int DimY)
{
	this->ImageX = DimX;
	this->ImageY = DimY;
	this->DimX = (DimX + 3) >> 2;
	this->DimY = (DimY + 3) >> 2;
	MapSize = DimX * DimY;
	if (MapSize != 0){
		pMap = new DirValue[MapSize];
	}
}

void CMap::GetImageDir(float * pBlock, int Stride)
{
	DirValue * pDir = this->pMap;
	GetDirBlock(pBlock, Stride, pDir, TOP | LEFT);
	int i = 4;
	pDir++;
	for( ; i < ImageX - 4; i += 4, pDir++){
		GetDirBlock(pBlock + i, Stride, pDir, TOP);
	}
	GetDirBlock(pBlock + i, Stride, pDir, TOP | RIGHT);
	pBlock += Stride << 2;
	pDir++;
	int j = 4;
	for( j; j < ImageY - 4; j += 4){
		GetDirBlock(pBlock, Stride, pDir, LEFT);
		int i = 4;
		pDir++;
		for( ; i < ImageX - 4; i += 4, pDir++){
			GetDirBlock(pBlock + i, Stride, pDir);
		}
		GetDirBlock(pBlock + i, Stride, pDir, RIGHT);
		pBlock += Stride << 2;
		pDir++;
	}
	GetDirBlock(pBlock, Stride, pDir, BOTTOM | LEFT);
	i = 4;
	pDir++;
	for( ; i < ImageX - 4; i += 4, pDir++){
		GetDirBlock(pBlock + i, Stride, pDir, BOTTOM);
	}
	GetDirBlock(pBlock + i, Stride, pDir, BOTTOM | RIGHT);
}

#define PXL_VAL(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + 1 + (y) * Stride] + \
		pBlock[(x) - 1 + (y) * Stride] + pBlock[(x) + ((y) + 1) * Stride] + \
		pBlock[(x) + ((y) - 1) * Stride]) / 4); \
	Sum += Sqr * Sqr;

#define PXL_VAL_T(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + 1 + (y) * Stride] + \
		pBlock[(x) - 1 + (y) * Stride] + pBlock[(x) + ((y) + 1) * Stride] * 2) \
		/ 4); \
	Sum += Sqr * Sqr;

#define PXL_VAL_B(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + 1 + (y) * Stride] + \
		pBlock[(x) - 1 + (y) * Stride] + pBlock[(x) + ((y) - 1) * Stride] * 2) \
		/ 4); \
	Sum += Sqr * Sqr;

#define PXL_VAL_L(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + 1 + (y) * Stride] * 2 + \
		pBlock[(x) + ((y) + 1) * Stride] + \
		pBlock[(x) + ((y) - 1) * Stride]) / 4); \
	Sum += Sqr * Sqr;

#define PXL_VAL_R(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) - 1 + (y) * Stride] * 2 + \
		pBlock[(x) + ((y) + 1) * Stride] + \
		pBlock[(x) + ((y) - 1) * Stride]) / 4); \
	Sum += Sqr * Sqr;

#define PXL_VAL_TL(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + 1 + (y) * Stride] + \
		pBlock[(x) + ((y) + 1) * Stride]) / 2); \
	Sum += Sqr * Sqr;

#define PXL_VAL_TR(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) - 1 + (y) * Stride] + \
		pBlock[(x) + ((y) + 1) * Stride]) / 2); \
	Sum += Sqr * Sqr;

#define PXL_VAL_BL(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + 1 + (y) * Stride] + \
		pBlock[(x) + ((y) - 1) * Stride]) / 2); \
	Sum += Sqr * Sqr;

#define PXL_VAL_BR(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) - 1 + (y) * Stride] + \
		pBlock[(x) + ((y) - 1) * Stride]) / 2); \
	Sum += Sqr * Sqr;

#define PXL_VAL_H(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + 1 + (y) * Stride] + \
		pBlock[(x) - 1 + (y) * Stride]) / 2); \
	Sum += Sqr * Sqr;

#define PXL_VAL_H_L(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - pBlock[(x) + 1 + (y) * Stride]; \
	Sum += Sqr * Sqr;

#define PXL_VAL_H_R(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - pBlock[(x) - 1 + (y) * Stride]; \
	Sum += Sqr * Sqr;

#define PXL_VAL_V(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + ((y) + 1) * Stride] + \
		pBlock[(x) + ((y) - 1) * Stride]) / 2); \
	Sum += Sqr * Sqr;

#define PXL_VAL_V_T(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - pBlock[(x) + ((y) + 1) * Stride]; \
	Sum += Sqr * Sqr;

#define PXL_VAL_V_B(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - pBlock[(x) + ((y) - 1) * Stride]; \
	Sum += Sqr * Sqr;

void CMap::GetDirBlock(float * pBlock, int Stride, DirValue * Result)
{
	float Sum = 0;
	float Sqr;

	PXL_VAL(1,0);
	PXL_VAL(3,0);
	PXL_VAL(0,1);
	PXL_VAL(2,1);
	PXL_VAL(1,2);
	PXL_VAL(3,2);
	PXL_VAL(0,3);
	PXL_VAL(2,3);

	Result->All = (unsigned short) Sum;
	if (Sum > 65535)
		Result->All = 0xFFFF;

	Sum = 0;
	PXL_VAL_H(1,0);
	PXL_VAL_H(3,0);
	PXL_VAL_H(0,1);
	PXL_VAL_H(2,1);
	PXL_VAL_H(1,2);
	PXL_VAL_H(3,2);
	PXL_VAL_H(0,3);
	PXL_VAL_H(2,3);

	Result->H_D1 = (unsigned short) Sum;
	if (Sum > 65535)
		Result->H_D1 = 0xFFFF;

	Sum = 0;
	PXL_VAL_V(1,0);
	PXL_VAL_V(3,0);
	PXL_VAL_V(0,1);
	PXL_VAL_V(2,1);
	PXL_VAL_V(1,2);
	PXL_VAL_V(3,2);
	PXL_VAL_V(0,3);
	PXL_VAL_V(2,3);

	Result->V_D2 = (unsigned short) Sum;
	if (Sum > 65535)
		Result->V_D2 = 0xFFFF;

	Result->Selected = 0;
	if (Result->Values[1] < Result->Values[0])
		Result->Selected = 1;
	if (Result->Values[2] < Result->Values[Result->Selected])
		Result->Selected = 2;
}

void CMap::GetDirBlock(float * pBlock, int Stride, DirValue * Result
		, int BitField)
{
	float Sum = 0;
	float Sqr;

	if (BitField & TOP){
		PXL_VAL_T(1,0);
	} else {
		PXL_VAL(1,0);
	}
	if (BitField & TOP){
		if (BitField & RIGHT){
			PXL_VAL_TR(3,0);
		} else {
			PXL_VAL_T(3,0);
		}
	}else if (BitField & RIGHT) {
		PXL_VAL_R(3,0);
	} else {
		PXL_VAL(3,0);
	}
	if (BitField & LEFT){
		PXL_VAL_L(0,1);
	} else {
		PXL_VAL(0,1);
	}
	if (BitField & RIGHT) {
		PXL_VAL_R(3,2);
	} else {
		PXL_VAL(3,2);
	}
	if (BitField & BOTTOM) {
		if (BitField & LEFT){
			PXL_VAL_BL(0,3);
		} else {
			PXL_VAL_B(0,3);
		}
	} else if (BitField & LEFT) {
		PXL_VAL_L(0,3);
	} else {
		PXL_VAL(0,3);
	}
	PXL_VAL(2,1);
	PXL_VAL(1,2);
	PXL_VAL(2,3);

	Result->All = (unsigned short) Sum;
	if (Sum > 65535)
		Result->All = 0xFFFF;

	Sum = 0;

	if (BitField & LEFT){
		PXL_VAL_H_L(0,1);
		PXL_VAL_H_L(0,3);
	} else {
		PXL_VAL_H(0,1);
		PXL_VAL_H(0,3);
	}

	if (BitField & RIGHT){
		PXL_VAL_H_R(3,0);
		PXL_VAL_H_R(3,2);
	} else {
		PXL_VAL_H(3,0);
		PXL_VAL_H(3,2);
	}

	PXL_VAL_H(1,0);
	PXL_VAL_H(2,1);
	PXL_VAL_H(1,2);
	PXL_VAL_H(2,3);

	Result->H_D1 = (unsigned short) Sum;
	if (Sum > 65535)
		Result->H_D1 = 0xFFFF;

	Sum = 0;
	if (BitField & TOP){
		PXL_VAL_V_T(1,0);
		PXL_VAL_V_T(3,0);
	} else {
		PXL_VAL_V(1,0);
		PXL_VAL_V(3,0);
	}

	if (BitField & BOTTOM){
		PXL_VAL_V_B(0,3);
		PXL_VAL_V_B(2,3);
	} else {
		PXL_VAL_V(0,3);
		PXL_VAL_V(2,3);
	}

	PXL_VAL_V(0,1);
	PXL_VAL_V(2,1);
	PXL_VAL_V(1,2);
	PXL_VAL_V(3,2);

	Result->V_D2 = (unsigned short) Sum;
	if (Sum > 65535)
		Result->V_D2 = 0xFFFF;

	Result->Selected = 0;
	if (Result->Values[1] < Result->Values[0])
		Result->Selected = 1;
	if (Result->Values[2] < Result->Values[Result->Selected])
		Result->Selected = 2;
}

}
