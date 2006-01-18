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

#pragma once

#include "band.h"
#include "map.h"

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
	unsigned int Thres(float Thres);
	unsigned int TSUQ(float Quant, float Thres, float RecLevel);

	DirWavelet * pLow;
	DirWavelet * pHigh;
	CBand DBand;
	CBand HVBand;
	CBand LBand;

private:

	int DimX;
	int DimY;
	float * pData;
	CMap HVMap;
	CMap DMap;

	DirWavelet(int x, int y, int level, DirWavelet * pHigh, int Align);

	void Init(int level, int Align);

	void LazyImage(float * pImage, unsigned int Stride);
	void LazyImageI(float * pImage, unsigned int Stride);

	static void LiftBandOdd(float * pBlock, int Stride, int DimX, int DimY,
				  float Coef);
	static void LiftBandEven(float * pBlock, int Stride, int DimX, int DimY,
				   float Coef);
	static void LiftBandDiagOdd(float * pBlock, int Stride, int DimX,
					int DimY, float Coef);
	static void LiftBandDiagEven(float * pBlock, int Stride, int DimX,
					int DimY, float Coef);

	void GetDirValues(float * pBlock, int Stride, DirValue * Result);
	void GetDirValues(float * pBlock, int Stride, DirValue * Result
			, int BitField);

	static void LiftOdd(float * pBlock, int Stride, float Coef);
	static void LiftEven(float * pBlock, int Stride, float Coef);
	static void LiftOddT(float * pBlock, int Stride, float Coef);
	static void LiftEvenT(float * pBlock, int Stride, float Coef);
	static void LiftOddB(float * pBlock, int Stride, float Coef);
	static void LiftEvenB(float * pBlock, int Stride, float Coef);
	static void LiftOddL(float * pBlock, int Stride, float Coef);
	static void LiftEvenL(float * pBlock, int Stride, float Coef);
	static void LiftOddR(float * pBlock, int Stride, float Coef);
	static void LiftEvenR(float * pBlock, int Stride, float Coef);
	static void LiftOddTL(float * pBlock, int Stride, float Coef);
	static void LiftEvenTL(float * pBlock, int Stride, float Coef);
	static void LiftOddTR(float * pBlock, int Stride, float Coef);
	static void LiftEvenTR(float * pBlock, int Stride, float Coef);
	static void LiftOddBL(float * pBlock, int Stride, float Coef);
	static void LiftEvenBL(float * pBlock, int Stride, float Coef);
	static void LiftOddBR(float * pBlock, int Stride, float Coef);
	static void LiftEvenBR(float * pBlock, int Stride, float Coef);

	static void LiftDiagOdd(float * pBlock, int Stride, float Coef);
	static void LiftDiagEven(float * pBlock, int Stride, float Coef);
	static void LiftDiagEvenT(float * pBlock, int Stride, float Coef);
	static void LiftDiagOddB(float * pBlock, int Stride, float Coef);
	static void LiftDiagEvenL(float * pBlock, int Stride, float Coef);
	static void LiftDiagOddR(float * pBlock, int Stride, float Coef);
	static void LiftDiagOddBR(float * pBlock, int Stride, float Coef);
	static void LiftDiagEvenTL(float * pBlock, int Stride, float Coef);

	static void LiftHOdd(float * pBlock, int Stride, float Coef);
	static void LiftHEven(float * pBlock, int Stride, float Coef);
	static void LiftHOddL(float * pBlock, int Stride, float Coef);
	static void LiftHEvenL(float * pBlock, int Stride, float Coef);
	static void LiftHOddR(float * pBlock, int Stride, float Coef);
	static void LiftHEvenR(float * pBlock, int Stride, float Coef);

	static void LiftVOdd(float * pBlock, int Stride, float Coef);
	static void LiftVEven(float * pBlock, int Stride, float Coef);
	static void LiftVOddT(float * pBlock, int Stride, float Coef);
	static void LiftVEvenT(float * pBlock, int Stride, float Coef);
	static void LiftVOddB(float * pBlock, int Stride, float Coef);
	static void LiftVEvenB(float * pBlock, int Stride, float Coef);

	static void LiftDiag1Odd(float * pBlock, int Stride, float Coef);
	static void LiftDiag1Even(float * pBlock, int Stride, float Coef);
	static void LiftDiag1EvenT(float * pBlock, int Stride, float Coef);
	static void LiftDiag1OddB(float * pBlock, int Stride, float Coef);
	static void LiftDiag1EvenL(float * pBlock, int Stride, float Coef);
	static void LiftDiag1OddR(float * pBlock, int Stride, float Coef);
	static void LiftDiag1OddBR(float * pBlock, int Stride, float Coef);
	static void LiftDiag1EvenTL(float * pBlock, int Stride, float Coef);

	static void LiftDiag2Odd(float * pBlock, int Stride, float Coef);
	static void LiftDiag2Even(float * pBlock, int Stride, float Coef);
	static void LiftDiag2EvenT(float * pBlock, int Stride, float Coef);
	static void LiftDiag2OddB(float * pBlock, int Stride, float Coef);
	static void LiftDiag2EvenL(float * pBlock, int Stride, float Coef);
	static void LiftDiag2OddR(float * pBlock, int Stride, float Coef);
	static void LiftDiag2OddBR(float * pBlock, int Stride, float Coef);
	static void LiftDiag2EvenTL(float * pBlock, int Stride, float Coef);
};

}
