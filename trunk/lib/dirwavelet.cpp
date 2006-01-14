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

namespace libdwic {

#include "libdwic.h"
#include "dirwavelet.h"

// #include <string.h>

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
	pHigh->NOSEBand.pParent = &NOSEBand;
	NOSEBand.pChild = &pHigh->NOSEBand;
	pHigh->NESOBand.pParent = &NESOBand;
	NESOBand.pChild = &pHigh->NESOBand;
	pHigh->HBand.pParent = &HBand;
	HBand.pChild = &pHigh->HBand;
	pHigh->VBand.pParent = &VBand;
	VBand.pChild = &pHigh->VBand;
	Init(level, pAllocated, Align);
}

DirWavelet::~DirWavelet()
{
	delete pLow;
	delete[] pData;
}

void DirWavelet::Init(int level,void * pAllocated, int Align)
{
	pAllocated = NOSEBand.Init(DimX >> 1, DimY >> 1, (char *)pAllocated, Align);
	pAllocated = NESOBand.Init(DimX >> 1, DimY >> 1, (char *)pAllocated, Align);
	pAllocated = HBand.Init(DimX >> 1, DimY >> 2, (char *)pAllocated, Align);
	pAllocated = VBand.Init(DimX >> 1, DimY >> 2, (char *)pAllocated, Align);
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

void DirWavelet::LiftBandDiag(float * pBlock, int Stride, int DimX, int DimY,
							  float Predict, float Update)
{
	pBlock = pBlock - Stride;
	for( int i = 0; i < DimX - 2; i += 2){
 		LiftBlockDiagT(pBlock + i, Stride, Predict);
 	}
	pBlock += Stride;
	int j = 0;
	for( ; j < DimY - 1; j += 1){
		LiftBlockDiagL(pBlock, Stride, Predict, Update);
		int i = 2;
		for( ; i < DimX - 2; i += 2){
			LiftBlockDiag(pBlock + i, Stride, Predict, Update);
		}
 		LiftBlockDiagR(pBlock + i, Stride, Predict, Update);
		pBlock += Stride;
	}
	for( int i = 2; i < DimX; i += 2){
		LiftBlockDiagB(pBlock + i, Stride, Update);
	}
}

void DirWavelet::LiftBandDiagI(float * pBlock, int Stride, int DimX, int DimY,
							   float Predict, float Update)
{
	LiftBlockDiagTLI(pBlock - 1, Stride, Predict);
	int i = 1;
	for( ; i < DimX - 1; i += 2){
		LiftBlockDiagTI(pBlock + i, Stride, Predict, Update);
	}
	pBlock += Stride;
	int j = 1;
	for( ; j < DimY - 1; j += 1){
		LiftBlockDiagLI(pBlock - 1, Stride, Predict);
		int i = 1;
		for( ; i < DimX - 1; i += 2){
			LiftBlockDiagI(pBlock + i, Stride, Predict, Update);
		}
		LiftBlockDiagRI(pBlock + i, Stride, Update);
		pBlock += Stride;
	}
	i = 1;
	for( ; i < DimX - 1; i += 2){
		LiftBlockDiagBI(pBlock + i, Stride, Predict, Update);
	}
	LiftBlockDiagBRI(pBlock + i, Stride, Update);
}

void DirWavelet::LiftBand(float * pBlock, int Stride, int DimX, int DimY,
						  float Predict, float Update)
{
	LiftBlockTL(pBlock, Stride, Predict, Update);
	int i = 2;
	for( ; i < DimX - 2; i += 2){
		LiftBlockT(pBlock + i, Stride, Predict, Update);
	}
	LiftBlockTR(pBlock + i, Stride, Predict, Update);
	pBlock += Stride << 1;
	int j = 2;
	for( j; j < DimY - 2; j += 2){
		LiftBlockL(pBlock, Stride, Predict, Update);
		int i = 2;
		for( ; i < DimX - 2; i += 2){
			LiftBlock(pBlock + i, Stride, Predict, Update);
		}
		LiftBlockR(pBlock + i, Stride, Predict, Update);
		pBlock += Stride << 1;
	}
	LiftBlockBL(pBlock, Stride, Predict, Update);
	i = 2;
	for( ; i < DimX - 2; i += 2){
		LiftBlockB(pBlock + i, Stride, Predict, Update);
	}
	LiftBlockBR(pBlock + i, Stride, Predict, Update);
}

void DirWavelet::LiftBandI(float * pBlock, int Stride, int DimX, int DimY,
						   float Predict, float Update)
{
	pBlock = pBlock + Stride;
	LiftBlockTLI(pBlock, Stride, Predict, Update);
	int i = 2;
	for( ; i < DimX - 2; i += 2){
		LiftBlockTI(pBlock + i, Stride, Predict, Update);
	}
	LiftBlockTRI(pBlock + i, Stride, Predict, Update);
	pBlock += Stride << 1;
	int j = 3;
	for( j; j < DimY - 1; j += 2){
		LiftBlockL(pBlock, Stride, Predict, Update);
		int i = 2;
		for( ; i < DimX - 2; i += 2){
			LiftBlock(pBlock + i, Stride, Predict, Update);
		}
		LiftBlockR(pBlock + i, Stride, Predict, Update);
		pBlock += Stride << 1;
	}
	LiftBlockBLI(pBlock, Stride, Predict, Update);
	i = 2;
	for( ; i < DimX - 2; i += 2){
		LiftBlockBI(pBlock + i, Stride, Predict, Update);
	}
	LiftBlockBRI(pBlock + i, Stride, Predict, Update);
}

void DirWavelet::LazyBand(QuincunxWavelet * pQWav)
{
	// Diagonal
	float * pInBand = pQWav->DBand.pBand;
	float * pOutBand1 = NOSEBand.pBand;
	float * pOutBand2 = NESOBand.pBand;

	for( int j = 0; j < pQWav->DBand.DimY ; j++ ){
		for( int i = 0, k = 0; i < pQWav->DBand.DimX ; i += 2, k++){
			pOutBand1[k] = pInBand[i];
			pOutBand2[k] = pInBand[i + 1];
		}
		pInBand += pQWav->DBand.DimXAlign;
		pOutBand1 += NOSEBand.DimXAlign;
		pOutBand2 += NESOBand.DimXAlign;
	}

	// Horizontal, Vertical

	int InBandStride = pQWav->HVBand.DimXAlign;
	pInBand = pQWav->HVBand.pBand;
	pOutBand1 = HBand.pBand;
	pOutBand2 = VBand.pBand;

	for( int j = 0; j < pQWav->HVBand.DimY ; j += 2 ){
		for( int i = 0; i < pQWav->HVBand.DimX ; i += 2){
			pOutBand1[i] = pInBand[i];
			pOutBand2[i] = pInBand[i + InBandStride];
			pOutBand1[i + 1] = pInBand[i + InBandStride + 1];
			pOutBand2[i + 1] = pInBand[i + 1];
		}
		pInBand += InBandStride << 1;
		pOutBand1 += NOSEBand.DimXAlign;
		pOutBand2 += NESOBand.DimXAlign;
	}

	// LBand
	if (pLow == 0)
		memcpy (LBand.pBand, pQWav->LBand.pBand, LBand.BandSize *
				sizeof(*LBand.pBand));
}

void DirWavelet::LazyBandI(QuincunxWavelet * pQWav)
{
	// Diagonal
	float * pOutBand = pQWav->DBand.pBand;
	float * pInBand1 = NOSEBand.pBand;
	float * pInBand2 = NESOBand.pBand;

	for( int j = 0; j < pQWav->DBand.DimY ; j++ ){
		for( int i = 0, k = 0; i < pQWav->DBand.DimX ; i += 2, k++){
			pOutBand[i] = pInBand1[k];
			pOutBand[i + 1] = pInBand2[k];
		}
		pOutBand += pQWav->DBand.DimXAlign;
		pInBand1 += NOSEBand.DimXAlign;
		pInBand2 += NESOBand.DimXAlign;
	}

	// Horizontal, Vertical

	int OutBandStride = pQWav->HVBand.DimXAlign;
	pOutBand = pQWav->HVBand.pBand;
	pInBand1 = HBand.pBand;
	pInBand2 = VBand.pBand;

	for( int j = 0; j < pQWav->HVBand.DimY ; j += 2 ){
		for( int i = 0; i < pQWav->HVBand.DimX ; i += 2){
			pOutBand[i] = pInBand1[i];
			pOutBand[i + OutBandStride] = pInBand2[i];
			pOutBand[i + OutBandStride + 1] = pInBand1[i + 1];
			pOutBand[i + 1] = pInBand2[i + 1];
		}
		pOutBand += OutBandStride << 1;
		pInBand1 += NOSEBand.DimXAlign;
		pInBand2 += NESOBand.DimXAlign;
	}

	// LBand
	if (pLow == 0)
 		memcpy (pQWav->LBand.pBand, LBand.pBand, LBand.BandSize *
 				sizeof(*LBand.pBand));
}

void DirWavelet::LazyTransform(QuincunxWavelet * pQWav)
{
	LazyBand(pQWav);
	if (pLow != 0)
		pLow->LazyTransform(pQWav->pLow);
}

void DirWavelet::LazyTransformI(QuincunxWavelet * pQWav)
{
	if (pLow != 0)
		pLow->LazyTransformI(pQWav->pLow);
	LazyBandI(pQWav);
}

void DirWavelet::Transform53(QuincunxWavelet * pQWav)
{
	LiftBandDiag(pQWav->DBand.pBand, pQWav->DBand.DimXAlign,
				 pQWav->DBand.DimX, pQWav->DBand.DimY, -1./4., 1./8.);
	LiftBand(pQWav->HVBand.pBand, pQWav->HVBand.DimXAlign,
			 pQWav->HVBand.DimX, pQWav->HVBand.DimY, -1./4., 1./8.);
	LazyBand(pQWav);
	NOSEBand.Weight = pQWav->DBand.Weight * 1.119000197;
	NESOBand.Weight = pQWav->DBand.Weight / 1.119000197;
	VBand.Weight = pQWav->HVBand.Weight / 1.119000197;
	HBand.Weight = pQWav->HVBand.Weight * 1.119000197;
	LBand.Weight = pQWav->LBand.Weight;
	if (pLow != 0)
		pLow->Transform53(pQWav->pLow);
}

void DirWavelet::Transform53I(QuincunxWavelet * pQWav)
{
	if (pLow != 0)
		pLow->Transform53I(pQWav->pLow);
	LazyBandI(pQWav);
	LiftBandI(pQWav->HVBand.pBand, pQWav->HVBand.DimXAlign,
			  pQWav->HVBand.DimX, pQWav->HVBand.DimY, -1./8., 1./4.);
	LiftBandDiagI(pQWav->DBand.pBand, pQWav->DBand.DimXAlign,
				  pQWav->DBand.DimX, pQWav->DBand.DimY, -1./8., 1./4.);
}

unsigned int DirWavelet::Thres(float Thres){
	unsigned int Count = 0;
	Count += HBand.Thres(Thres);
	Count += VBand.Thres(Thres);
	Count += NOSEBand.Thres(Thres);
	Count += NESOBand.Thres(Thres);
	if (pLow != 0)
		Count += pLow->Thres(Thres);
	return Count;
}

unsigned int DirWavelet::TSUQ(float Quant, float Thres, float RecLevel){
	unsigned int Count = 0;
	Count += HBand.TSUQ(Quant, Thres, RecLevel);
	Count += VBand.TSUQ(Quant, Thres, RecLevel);
	Count += NOSEBand.TSUQ(Quant, Thres, RecLevel);
	Count += NESOBand.TSUQ(Quant, Thres, RecLevel);
	if (pLow != 0)
		Count += pLow->TSUQ(Quant, Thres, RecLevel);
	else
		Count += LBand.TSUQ(Quant, Quant * .5, Quant * .5);
	return Count;
}

}
