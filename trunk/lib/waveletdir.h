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

#include "band.h"
#include "map.h"
#include "wavelet.h"
#include "wavelet2d.h"

namespace libdwic {

typedef enum lift {even, odd, diag_even, diag_odd};

#define MOD		.125
#define ALPHA1	(ALPHA * MOD)
#define ALPHA2	(ALPHA * (1-MOD))
#define BETA1	(BETA * MOD)
#define BETA2	(BETA * (1-MOD))
#define GAMMA1	(GAMMA * MOD)
#define GAMMA2	(GAMMA * (1-MOD))
#define DELTA1	(DELTA * MOD)
#define DELTA2	(DELTA * (1-MOD))

/**
@author Nicolas Botti
*/
class CWaveletDir{
public:

	CWaveletDir(int x, int y, int level, int Align = ALIGN);

	~CWaveletDir();

	void LazyTransform(float * pImage, int Stride);
	void LazyTransformI(float * pImage, int Stride);
	void Transform53(float * pImage, int Stride);
	void Transform53I(float * pImage, int Stride);
	void SetWeight53(void);
	void Transform97(float * pImage, int stride, float lambda);
	void Transform97I(float * pImage, int Stride);
	void SetWeight97(void);

	void SetRange(CRangeCodec * RangeCodec);
	void CodeMap(int Options = 0);
	void DecodeMap(int Options = 0);
	unsigned char * CodeBand(unsigned char * pBuf);
	unsigned char * DecodeBand(unsigned char * pBuf);

	unsigned int Thres(float Thres);
	unsigned int TSUQ(float Quant, float Thres);
	void TSUQi(float Quant, float RecLevel);
	void Saturate(float * pImage, int stride);

	void Stats(void);
	void SetDir(int Sel);
	void GetMap(unsigned char * pOut, int level, int Direction);
	void GetBand(float * pOut, int level, int Direction);
	void GetDist(unsigned char * pOut, int level, int Direction);

	int GetDimX(void){ return DimX;}
	int GetDimY(void){ return DimY;}

private:

	int DimX;
	int DimY;
	int Level;

	CWaveletDir * pLow;
	CWaveletDir * pHigh;

	CBand DLBand;
	CBand DHBand;
	CBand HVBand;
	CBand HVLBand;
	CBand HVHBand;
	CBand LBand;

	CWavelet2D HVWav;

	CMap HVMap;
	CMap DMap;

	CWaveletDir(int x, int y, int level, CWaveletDir * pHigh, int Align);

	void Init(int level, int Align);

	void Transform97(float * pImage, int Stride, bool getDir);

	void LazyImage(float * pImage, unsigned int Stride);
	void LazyImageI(float * pImage, unsigned int Stride);
	void LazyBand(void);
	void LazyBandI(void);

	template <lift lft_opt>
	static void LiftBand(float * pCur, int stride, int DimX, int DimY,
						 float Coef1, float Coef2, char * pDir1);

	void GetImageDir97(float * pImage, int stride);
	void GetImageDirDiag97(float * pImage, int stride);
};

}
