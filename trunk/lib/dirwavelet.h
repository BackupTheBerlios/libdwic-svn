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
#include "rangecodec.h"

namespace libdwic {

#define MAX_WAV_LEVEL 5

/**
@author Nicolas Botti
*/

class DirWavelet{
public:
	DirWavelet(int x, int y, int level, int Align = ALIGN);

	~DirWavelet();

	void LazyTransform(float * pImage, int Stride);
	void LazyTransformI(float * pImage, int Stride);
	void Transform53(float * pImage, int Stride);
	void Transform53I(float * pImage, int Stride);
	void Transform97(float * pImage, int Stride);
	void Transform97I(float * pImage, int Stride);

	void SetRange(CRangeCodec * RangeCodec);
	void Code(int Options = 0);
	void Decode(int Options = 0);

	unsigned int Thres(float Thres);
	unsigned int TSUQ(float Quant, float Thres);
	void DirWavelet::TSUQi(float Quant, float RecLevel);

	void Stats(void);
	void SetSelected(int Sel);

private:

	int DimX;
	int DimY;
	int Level;
	CMap HVMap;
	CMap DMap;

	DirWavelet * pLow;
	DirWavelet * pHigh;

	CBand DBand;
	CBand HVBand;
	CBand LBand;

	int DimDDir;
	int DimHVDir;
	float * pD1D;
	float * pHV1D;

	void (*LiftEdgeOdd[3])(float*, int, float, int);
	void (*LiftInOdd[3])(float*, int, float);
	void (*LiftEdgeEven[3])(float*, int, float, int);
	void (*LiftInEven[3])(float*, int, float);
	void (*LiftEdgeDiagOdd[3])(float*, int, float, int);
	void (*LiftInDiagOdd[3])(float*, int, float);
	void (*LiftEdgeDiagEven[3])(float*, int, float, int);
	void (*LiftInDiagEven[3])(float*, int, float);

	DirWavelet(int x, int y, int level, DirWavelet * pHigh, int Align);

	void Init(int level, int Align);
	void InitFuncPtr(void);

	void LazyImage(float * pImage, unsigned int Stride);
	void LazyImageI(float * pImage, unsigned int Stride);

	void Fill1D(void);
	void FillHV1D(void);
	void FillD1D(void);

	static void LiftBand(float * pBlock, int Stride, int DimX, int DimY,
							  float Coef, DirValue * pDir,
							  void (**LiftEdge)(float*, int, float, int),
							  void (**Lift)(float*, int, float));

	static void LiftHOdd(float * pBlock, int Stride, float Coef);
	static void LiftHEven(float * pBlock, int Stride, float Coef);
	static void LiftHOdd(float * pBlock, int Stride, float Coef, int BitField);
	static void LiftHEven(float * pBlock, int Stride, float Coef, int BitField);

	static void LiftVOdd(float * pBlock, int Stride, float Coef);
	static void LiftVEven(float * pBlock, int Stride, float Coef);
	static void LiftVOdd(float * pBlock, int Stride, float Coef, int BitField);
	static void LiftVEven(float * pBlock, int Stride, float Coef, int BitField);

	static void LiftDiag1Odd(float * pBlock, int Stride, float Coef);
	static void LiftDiag1Even(float * pBlock, int Stride, float Coef);
	static void LiftDiag1Odd(float * pBlock, int Stride, float Coef,
							 int BitField);
	static void LiftDiag1Even(float * pBlock, int Stride, float Coef,
							  int BitField);

	static void LiftDiag2Odd(float * pBlock, int Stride, float Coef);
	static void LiftDiag2Even(float * pBlock, int Stride, float Coef);
	static void LiftDiag2Odd(float * pBlock, int Stride, float Coef,
							 int BitField);
	static void LiftDiag2Even(float * pBlock, int Stride, float Coef,
							  int BitField);
};

}
