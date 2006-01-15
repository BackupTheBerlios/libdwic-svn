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

#ifndef DIRWAVELET_H
#define DIRWAVELET_H

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

	DirWavelet(int x, int y, int level,
			   DirWavelet * pHigh, void * pAllocated, int Align);

	void Init(int level, void * pAllocated, int Align);

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

	static void LiftBlockOdd(float * pBlock, int Stride, float Coef);
	static void LiftBlockEven(float * pBlock, int Stride, float Coef);
	static void LiftBlockOddT(float * pBlock, int Stride, float Coef);
	static void LiftBlockEvenT(float * pBlock, int Stride, float Coef);
	static void LiftBlockOddB(float * pBlock, int Stride, float Coef);
	static void LiftBlockEvenB(float * pBlock, int Stride, float Coef);
	static void LiftBlockOddL(float * pBlock, int Stride, float Coef);
	static void LiftBlockEvenL(float * pBlock, int Stride, float Coef);
	static void LiftBlockOddR(float * pBlock, int Stride, float Coef);
	static void LiftBlockEvenR(float * pBlock, int Stride, float Coef);
	static void LiftBlockOddTL(float * pBlock, int Stride, float Coef);
	static void LiftBlockEvenTL(float * pBlock, int Stride, float Coef);
	static void LiftBlockOddTR(float * pBlock, int Stride, float Coef);
	static void LiftBlockEvenTR(float * pBlock, int Stride, float Coef);
	static void LiftBlockOddBL(float * pBlock, int Stride, float Coef);
	static void LiftBlockEvenBL(float * pBlock, int Stride, float Coef);
	static void LiftBlockOddBR(float * pBlock, int Stride, float Coef);
	static void LiftBlockEvenBR(float * pBlock, int Stride, float Coef);

	static void LiftBlockDiagOdd(float * pBlock, int Stride, float Coef);
	static void LiftBlockDiagEven(float * pBlock, int Stride, float Coef);
	static void LiftBlockDiagEvenT(float * pBlock, int Stride, float Coef);
	static void LiftBlockDiagOddB(float * pBlock, int Stride, float Coef);
	static void LiftBlockDiagEvenL(float * pBlock, int Stride, float Coef);
	static void LiftBlockDiagOddR(float * pBlock, int Stride, float Coef);
	static void LiftBlockDiagOddBR(float * pBlock, int Stride, float Coef);
	static void LiftBlockDiagEvenTL(float * pBlock, int Stride, float Coef);
};

#endif
