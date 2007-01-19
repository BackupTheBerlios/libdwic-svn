/***************************************************************************
 *   Copyright (C) 2006-2007 Nicolas BOTTI <rududu@laposte.net>            *
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

#include "global.h"
#include "map.h"

#include <math.h>
#include <string.h>
#include <iostream>

using namespace std;

namespace libdwic {

CMap::CMap(CMap * pHighMap):
pMap(0),
pLow(0)
{
	pHigh = pHighMap;
	if (pHigh != 0)
		pHigh->pLow = this;
	Init();
}


CMap::~CMap()
{
	delete[] pMap;
}

void CMap::Init(int DimX, int DimY)
{
	this->DimX = DimX;
	this->DimY = DimY;
	MapSize = this->DimX * this->DimY;
	if (MapSize != 0)
		pMap = new char[MapSize];
}

void CMap::SetDir(char Dir)
{
	memset(pMap, Dir, MapSize);
}

void CMap::GetMap(unsigned char * pOut)
{
	memcpy(pOut, pMap, MapSize);
}

/**
 * fills pMap with the best local direction, estimated from 2 prediction images
 * along the H and V directions
 * @param pImage1
 * @param pImage2
 * @param stride
 */
void CMap::SelectDir(float * pImage1, float * pImage2, int stride)
{
	char * pDir = this->pMap;
	int diff = 2 * stride - (DimX << 1);
	int end = stride * (DimY << 1);
	int pos1 = 1, pos2 = stride, pos3 = 2 * stride + 1;

	for( ; pos1 < end; pos1 += diff, pos2 += diff){
		for( int stop = pos1 + (DimX << 1); pos1 < stop; pos1 += 2, pos2 += 2){
			pImage1[pos1] = fabsf(pImage1[pos1]) - fabsf(pImage2[pos1]);
			pImage1[pos2] = fabsf(pImage1[pos2]) - fabsf(pImage2[pos2]);
		}
	}

	for( pos1 = 1, pos2 = stride; pos3 < end; pos1 += diff, pos2 += diff, pos3 += diff){
		for( char * pStop = pDir + DimX - 1; pDir < pStop; pDir++, pos1 += 2, pos2 += 2, pos3 += 2){
			pDir[0] = ((pImage1[pos1] + pImage1[pos2] + pImage1[pos2 + 2] + pImage1[pos3]) > 0) << 1;
		}
		pDir[0] = ((pImage1[pos1] + pImage1[pos2] + pImage1[pos3]) > 0) << 1;
		pDir++, pos1 += 2, pos2 += 2, pos3 += 2;
	}
	for( char * pStop = pDir + DimX - 1; pDir < pStop; pDir++, pos1 += 2, pos2 += 2){
		pDir[0] = ((pImage1[pos1] + pImage1[pos2] + pImage1[pos2 + 2]) > 0) << 1;
	}
	pDir[0] = ((pImage1[pos1] + pImage1[pos2]) > 0) << 1;
}

/**
 * fills pMap with the best local direction, estimated from 2 prediction images
 * along the 2 diagonal directions
 * @param pImage1
 * @param pImage2
 * @param stride
 */
void CMap::SelectDirDiag(float * pImage1, float * pImage2, int stride)
{
	char * pDir = this->pMap;
	int diff = 2 * stride - (DimX << 1);
	int end = stride * (DimY << 2);

	for(int pos = 1 + stride; pos < end; pos += diff){
		for( int stop = pos + (DimX << 1); pos < stop; pos += 2){
			pImage1[pos] = fabsf(pImage1[pos]) - fabsf(pImage2[pos]);
		}
	}

	int pos1 = -stride + 1, pos2 = stride - 1, pos3 = 3 * stride - 1;
	diff = 4 * stride - (DimX << 1);

	pDir[0] = ((pImage1[pos2 + 2] + pImage1[pos3 + 2]) > 0) << 1;
	pDir[1] = ((pImage1[pos2 + 2] + pImage1[pos2 + 4]) > 0) << 1;
	pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4;
	for( char * pStop = pDir + DimX - 2; pDir < pStop; pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4){
		pDir[0] = ((pImage1[pos2] + pImage1[pos2 + 2] + pImage1[pos3] + pImage1[pos3 + 2]) > 0) << 1;
		pDir[1] = ((pImage1[pos2 + 2] + pImage1[pos2 + 4]) > 0) << 1;
	}
	pos1 += diff, pos2 += diff, pos3 += diff;
	for( ; pos2 < end; pos1 += diff, pos2 += diff, pos3 += diff){
		pDir[0] = ((pImage1[pos2 + 2] + pImage1[pos3 + 2]) > 0) << 1;
		pDir[1] = ((pImage1[pos1] + pImage1[pos1 + 2] + pImage1[pos2 + 2] + pImage1[pos2 + 4]) > 0) << 1;
		pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4;
		for( char * pStop = pDir + DimX - 2; pDir < pStop; pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4){
			pDir[0] = ((pImage1[pos2] + pImage1[pos2 + 2] + pImage1[pos3] + pImage1[pos3 + 2]) > 0) << 1;
			pDir[1] = ((pImage1[pos1] + pImage1[pos1 + 2] + pImage1[pos2 + 2] + pImage1[pos2 + 4]) > 0) << 1;
		}
	}
}

const char CMap::LUT1[5] = {0, 0, 1, 2, 2};
const char CMap::LUT2[9] = {0, 0, 0, 0, 1, 2, 2, 2, 2};

/**
 * Put the interpolated directions from pMap to pOut
 * use for H/V transform
 * @param pOut
 * @param stride
 */
void CMap::GetDirs(char * pOut, int stride)
{
	char * pDir = this->pMap - DimX - 1;
	int diff = 2 * stride - (DimX << 1);
	int end = stride * (DimY << 1);
	int pos1 = 0, pos2 = stride;

	pOut[pos1 + 1] = pOut[pos1] = pOut[pos2] = pOut[pos2 + 1] = pDir[DimX + 1];
	pDir++, pos1 += 2, pos2 += 2;
	for( char * pStop = pDir + DimX - 1; pDir < pStop; pDir++, pos1 += 2, pos2 += 2){
		pOut[pos2] = pOut[pos1] = LUT1[pDir[DimX] + pDir[DimX + 1]];
		pOut[pos1 + 1] = pOut[pos2 + 1] = pDir[DimX + 1];
	}
	pos1 += diff, pos2 += diff;
	for( ; pos1 < end; pos1 += diff, pos2 += diff){
		pOut[pos1 + 1] = pOut[pos1] = LUT1[pDir[1] + pDir[DimX + 1]];
		pOut[pos2] = pOut[pos2 + 1] = pDir[DimX + 1];
		pDir++, pos1 += 2, pos2 += 2;
		for( char * pStop = pDir + DimX - 1; pDir < pStop; pDir++, pos1 += 2, pos2 += 2){
			pOut[pos1] = LUT2[pDir[0] + pDir[1] + pDir[DimX] + pDir[DimX + 1]];
			pOut[pos1 + 1] = LUT1[pDir[1] + pDir[DimX + 1]];
			pOut[pos2] = LUT1[pDir[DimX] + pDir[DimX + 1]];
			pOut[pos2 + 1] = pDir[DimX + 1];
		}
	}
}

/**
 * Put the interpolated directions from pMap to pOut
 * use for diagonal transform
 * @param pOut
 * @param stride
 */
void CMap::GetDirsDiag(char * pOut, int stride)
{
	char * pDir = this->pMap;
	int diff = 4 * stride - (DimX << 1);
	int end = stride * ((DimY - 1) << 2);
	int pos1 = 0, pos2 = stride + 1, pos3 = 2 * stride, pos4 = 3 * stride + 1;

	pOut[pos1] = LUT1[pDir[0] + pDir[1]];
	pOut[pos3 + 2] = LUT2[pDir[2] + pDir[DimX + 1] + pDir[0] + pDir[1]];
	pOut[pos2] = LUT1[pDir[0] + pDir[1]];
	pOut[pos2 + 2] = LUT1[pDir[1] + pDir[2]];
	pOut[pos4] = LUT1[pDir[0] + pDir[DimX + 1]];
	pOut[pos4 + 2] = LUT1[pDir[2] + pDir[DimX + 1]];
	pOut[pos1 + 2] = pDir[1];
	pOut[pos3] = pDir[0];
	pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4, pos4 += 4;
	for( char * pStop = pDir + DimX - 4; pDir < pStop; pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4, pos4 += 4){
		pOut[pos1] = LUT2[pDir[-1] + 2 * pDir[0] + pDir[1]];
		pOut[pos3 + 2] = LUT2[pDir[2] + pDir[DimX + 1] + pDir[0] + pDir[1]];
		pOut[pos2] = LUT1[pDir[0] + pDir[1]];
		pOut[pos2 + 2] = LUT1[pDir[1] + pDir[2]];
		pOut[pos4] = LUT1[pDir[0] + pDir[DimX + 1]];
		pOut[pos4 + 2] = LUT1[pDir[2] + pDir[DimX + 1]];
		pOut[pos1 + 2] = pDir[1];
		pOut[pos3] = pDir[0];
	}
	pOut[pos1] = LUT2[pDir[-1] + 2 * pDir[0] + pDir[1]];
	pOut[pos3 + 2] = LUT2[pDir[DimX + 1] + 2 * pDir[0] + pDir[1]];
	pOut[pos2] = LUT1[pDir[0] + pDir[1]];
	pOut[pos2 + 2] = pDir[1];
	pOut[pos4] = LUT1[pDir[0] + pDir[DimX + 1]];
	pOut[pos4 + 2] = pDir[DimX + 1];
	pOut[pos1 + 2] = pDir[1];
	pOut[pos3] = pDir[0];
	pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4, pos4 += 4;
	pos1 += diff, pos2 += diff, pos3 += diff, pos4 += diff;
	for( ; pos1 < end; pos1 += diff, pos2 += diff, pos3 += diff, pos4 += diff){
		pOut[pos1] = LUT2[pDir[-DimX] + pDir[0] + 2 * pDir[1]];
		pOut[pos3 + 2] = LUT2[pDir[2] + pDir[DimX + 1] + pDir[0] + pDir[1]];
		pOut[pos2] = LUT1[pDir[0] + pDir[1]];
		pOut[pos2 + 2] = LUT1[pDir[1] + pDir[2]];
		pOut[pos4] = LUT1[pDir[0] + pDir[DimX + 1]];
		pOut[pos4 + 2] = LUT1[pDir[2] + pDir[DimX + 1]];
		pOut[pos1 + 2] = pDir[1];
		pOut[pos3] = pDir[0];
		pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4, pos4 += 4;
		for( char * pStop = pDir + DimX - 4; pDir < pStop; pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4, pos4 += 4){
			pOut[pos1] = LUT2[pDir[-DimX] + pDir[-1] + pDir[0] + pDir[1]];
			pOut[pos3 + 2] = LUT2[pDir[2] + pDir[DimX + 1] + pDir[0] + pDir[1]];
			pOut[pos2] = LUT1[pDir[0] + pDir[1]];
			pOut[pos2 + 2] = LUT1[pDir[1] + pDir[2]];
			pOut[pos4] = LUT1[pDir[0] + pDir[DimX + 1]];
			pOut[pos4 + 2] = LUT1[pDir[2] + pDir[DimX + 1]];
			pOut[pos1 + 2] = pDir[1];
			pOut[pos3] = pDir[0];
		}
		pOut[pos1] = LUT2[pDir[-DimX] + pDir[-1] + pDir[0] + pDir[1]];
		pOut[pos3 + 2] = LUT2[pDir[DimX + 1] + 2 * pDir[0] + pDir[1]];
		pOut[pos2] = LUT1[pDir[0] + pDir[1]];
		pOut[pos2 + 2] = pDir[1];
		pOut[pos4] = LUT1[pDir[0] + pDir[DimX + 1]];
		pOut[pos4 + 2] = pDir[DimX + 1];
		pOut[pos1 + 2] = pDir[1];
		pOut[pos3] = pDir[0];
		pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4, pos4 += 4;
	}
	pOut[pos1] = LUT2[pDir[-DimX] + pDir[0] + 2 * pDir[1]];
	pOut[pos3 + 2] = LUT2[pDir[2] + pDir[0] + 2 * pDir[1]];
	pOut[pos2] = LUT1[pDir[0] + pDir[1]];
	pOut[pos2 + 2] = LUT1[pDir[1] + pDir[2]];
	pOut[pos4] = pDir[0];
	pOut[pos4 + 2] = pDir[2];
	pOut[pos1 + 2] = pDir[1];
	pOut[pos3] = pDir[0];
	pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4, pos4 += 4;
	for( char * pStop = pDir + DimX - 4; pDir < pStop; pDir += 2, pos1 += 4, pos2 += 4, pos3 += 4, pos4 += 4){
		pOut[pos1] = LUT2[pDir[-DimX] + pDir[-1] + pDir[0] + pDir[1]];
		pOut[pos3 + 2] = LUT2[pDir[2] + pDir[0] + 2 * pDir[1]];
		pOut[pos2] = LUT1[pDir[0] + pDir[1]];
		pOut[pos2 + 2] = LUT1[pDir[1] + pDir[2]];
		pOut[pos4] = pDir[0];
		pOut[pos4 + 2] = pDir[2];
		pOut[pos1 + 2] = pDir[1];
		pOut[pos3] = pDir[0];
	}
	pOut[pos1] = LUT2[pDir[-DimX] + pDir[-1] + pDir[0] + pDir[1]];
	pOut[pos3 + 2] = LUT1[pDir[0] + pDir[1]];
	pOut[pos2] = LUT1[pDir[0] + pDir[1]];
	pOut[pos2 + 2] = pDir[1];
	pOut[pos4] = pDir[0];
	pOut[pos4 + 2] = 1;
	pOut[pos1 + 2] = pDir[1];
	pOut[pos3] = pDir[0];
}

}
