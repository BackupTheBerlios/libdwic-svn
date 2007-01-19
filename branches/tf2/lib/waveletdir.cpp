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
#include "waveletdir.h"
#include <iostream>
#include <string.h>

using namespace std;

namespace libdwic {

CWaveletDir::CWaveletDir(int x, int y, int level, int Align,
						 CWaveletDir * pHigh):
		DimX(x),
		DimY(y),
		pHigh(0),
		pLow(0),
		HVMap(pHigh == 0 ? (CMap*)0 : &pHigh->HVMap),
		DMap(pHigh == 0 ? (CMap*)0 : &pHigh->DMap),
		LMap(level > 1 ? (CMap*)0 : &HVMap)
{
	if (level > MAX_WAV_LEVEL)
		level = MAX_WAV_LEVEL;
	this->pHigh = pHigh;
	if (pHigh != 0){
		pHigh->DHBand.pParent = &DHBand;
		DHBand.pChild = &pHigh->DHBand;
		pHigh->DLBand.pParent = &DLBand;
		DLBand.pChild = &pHigh->DLBand;
		pHigh->HVHBand.pParent = &HVHBand;
		HVHBand.pChild = &pHigh->HVHBand;
		pHigh->HVLBand.pParent = &HVLBand;
		HVLBand.pChild = &pHigh->HVLBand;
	}
	Init(level, Align);
}

CWaveletDir::~CWaveletDir()
{
	delete pLow;
}

void CWaveletDir::Init(int level, int Align)
{
	DHBand.Init(DimX >> 1, DimY >> 1, Align, true);
	DLBand.Init(DimX >> 1, DimY >> 1, Align, true);
	HVBand.Init(DimX >> 1, DimY >> 1, Align);
	HVHBand.Init(DimX >> 1, DimY >> 2, Align, true);
	HVLBand.Init(DimX >> 1, DimY >> 2, Align, true);
	HVMap.Init(DimX >> 1, DimY >> 1);
	DMap.Init(DimX >> 1, DimY >> 2);
	Level = level;

	if (level > 1){
		pLow = new CWaveletDir(DimX >> 1, DimY >> 1, level - 1, Align, this);
	}else{
		LBand.Init(DimX >> 1, DimY >> 1, Align);
	}
}

void CWaveletDir::SetCodec(CMuxCodec * Codec)
{
// 	HVMap.SetCodec(Codec);
// 	DMap.SetCodec(Codec);

	if (pLow != 0)
		pLow->SetCodec(Codec);
// 	else
// 		LMap.SetCodec(Codec);
}

void CWaveletDir::GetMap(unsigned char * pOut, int level, int Direction)
{
	CWaveletDir * pCurWav = this;
	while( pCurWav != 0 && pCurWav->Level != level ){
		pCurWav = pCurWav->pLow;
	}
	if (pCurWav != 0) {
		if (Direction == 0)
			pCurWav->HVMap.GetMap(pOut);
		else
			pCurWav->DMap.GetMap(pOut);
	}
}

/**
 *
 * @param pOut output buffer
 * @param level band level in the wavelet transform
 * @param Direction band direction (0 = HV, 1 = HVL, 2 = HVH, 3 = DL, 4 = DH)
 */
void CWaveletDir::GetBand(float * pOut, int level, int Direction)
{
	CWaveletDir * pCurWav = this;
	while( pCurWav != 0 && pCurWav->Level != level ){
		pCurWav = pCurWav->pLow;
	}
	if (pCurWav != 0) {
		switch (Direction) {
			case 0 :
				pCurWav->HVBand.GetBand(pOut);
				break;
			case 1 :
				pCurWav->HVLBand.GetBand(pOut);
				break;
			case 2 :
				pCurWav->HVHBand.GetBand(pOut);
				break;
			case 3 :
				pCurWav->DLBand.GetBand(pOut);
				break;
			case 4 :
				pCurWav->DHBand.GetBand(pOut);
				break;
		}
	}
}

#define PRINT_STAT(band) \
	cout << band << " " << Level << " :\t"; \
	/*cout << "Moyenne : " << Mean << endl;*/ \
	cout << Var << endl;

void CWaveletDir::Stats(void)
{
	float Mean = 0, Var = 0;
	DHBand.Mean(Mean, Var);
	PRINT_STAT("DH");
	DLBand.Mean(Mean, Var);
	PRINT_STAT("DL");
	HVBand.Mean(Mean, Var);
	PRINT_STAT("HV");
	HVHBand.Mean(Mean, Var);
	PRINT_STAT("HVH");
	HVLBand.Mean(Mean, Var);
	PRINT_STAT("HVL");
// 	HVWav.Stats();

	if (pLow != 0)
		pLow->Stats();
	else{
		LBand.Mean(Mean, Var);
		PRINT_STAT("L");
	}
}

void CWaveletDir::SetDir(int Sel)
{
	HVMap.SetDir(Sel);
	DMap.SetDir(Sel);
	if (pLow != 0)
		pLow->SetDir(Sel);
}

void CWaveletDir::CodeBand(CMuxCodec * pCodec, int method)
{
	CWaveletDir * pCurWav = this;

	switch( method ){
		case 1 :
		case 2 :
			HVLBand.BuildTree<false>();
			HVHBand.BuildTree<false>();
			DLBand.BuildTree<false>();
			DHBand.BuildTree<false>();
			while( pCurWav->pLow != 0 )
				pCurWav = pCurWav->pLow;
			pCurWav->LBand.bit<code>(pCodec);
			pCurWav->HVLBand.Tree<code>(pCodec);
			pCurWav->HVHBand.Tree<code>(pCodec);
			pCurWav->DLBand.Tree<code>(pCodec);
			pCurWav->DHBand.Tree<code>(pCodec);
			break;
		case 3 :
			while( pCurWav != 0 ){
				pCurWav->DLBand.enu<code>(pCodec);
				pCurWav->DHBand.enu<code>(pCodec);
				pCurWav->HVLBand.enu<code>(pCodec);
				pCurWav->HVHBand.enu<code>(pCodec);
				if (pCurWav->pLow == 0){
					pCurWav->LBand.pred<code>(pCodec);
				}
				pCurWav = pCurWav->pLow;
			}
	}
}

void CWaveletDir::DecodeBand(CMuxCodec * pCodec, int method)
{
	CWaveletDir * pCurWav = this;

	switch( method ){
		case 1 :
		case 2 :
			HVLBand.Clear(true);
			HVHBand.Clear(true);
			DLBand.Clear(true);
			DHBand.Clear(true);
			while( pCurWav->pLow != 0 )
				pCurWav = pCurWav->pLow;
			pCurWav->LBand.bit<decode>(pCodec);
			pCurWav->HVLBand.Tree<decode>(pCodec);
			pCurWav->HVHBand.Tree<decode>(pCodec);
			pCurWav->DLBand.Tree<decode>(pCodec);
			pCurWav->DHBand.Tree<decode>(pCodec);
			break;
		case 3 :
			while( pCurWav != 0 ){
				pCurWav->DLBand.enu<decode>(pCodec);
				pCurWav->DHBand.enu<decode>(pCodec);
				pCurWav->HVLBand.enu<decode>(pCodec);
				pCurWav->HVHBand.enu<decode>(pCodec);
				if (pCurWav->pLow == 0)
					pCurWav->LBand.pred<decode>(pCodec);
				pCurWav = pCurWav->pLow;
			}
	}
}

void CWaveletDir::LazyImage(float * pImage, unsigned int stride)
{
	int Bandpos, Ipos1, Ipos2, Lstride;
	float * pLOut;
	if (pLow == 0){
		pLOut = LBand.pBand;
		Lstride = LBand.DimX;
	}else{
		pLOut = pImage;
		Lstride = stride;
	}

	for(int j = 0; j < DimY; j += 2){
		Bandpos = (j >> 1) * DLBand.DimXAlign;
		Ipos1 = j * stride;
		Ipos2 = (j >> 1) * Lstride;
		for(int pEnd = Ipos1 + DimX; Ipos1 < pEnd; Ipos1 += 2){
			pLOut[Ipos2] = pImage[Ipos1];
			DLBand.pBand[Bandpos] = pImage[Ipos1 + 1];
			DHBand.pBand[Bandpos] = pImage[Ipos1 + stride];
			HVBand.pBand[Bandpos] = pImage[Ipos1 + stride + 1];
			Bandpos++;
			Ipos2++;
		}
	}
}

void CWaveletDir::LazyImageI(float * pImage, unsigned int stride)
{
	int Bandpos, Ipos1, Ipos2, Lstride;
	float * pLIn;
	if (pLow == 0){
		pLIn = LBand.pBand;
		Lstride = LBand.DimX;
	}else{
		pLIn = pImage;
		Lstride = stride;
	}

	for(int j = DimY - 2; j >= 0; j -= 2){
		Bandpos = (j >> 1) * DLBand.DimXAlign + DLBand.DimX - 1;
		Ipos1 = j * stride + DimX - 2;
		Ipos2 = (j >> 1) * Lstride + (DimX >> 1) - 1;
		for(int pEnd = Ipos1 - DimX; Ipos1 > pEnd; Ipos1 -= 2){
			pImage[Ipos1] = pLIn[Ipos2];
			pImage[Ipos1 + 1] = DLBand.pBand[Bandpos];
			pImage[Ipos1 + stride] = DHBand.pBand[Bandpos];
			pImage[Ipos1 + stride + 1] = HVBand.pBand[Bandpos];
			Bandpos--;
			Ipos2--;
		}
	}
}

void CWaveletDir::LazyBand(void)
{
	int inPos, outPos;

	for(int j = 0; j < HVBand.DimY; j += 2){
		inPos = j * HVBand.DimXAlign;
		outPos = (j >> 1) * HVLBand.DimXAlign;
		for(int pEnd = inPos + HVBand.DimX; inPos < pEnd; inPos += 2){
			HVLBand.pBand[outPos] = HVBand.pBand[inPos];
			HVLBand.pBand[outPos + 1] = HVBand.pBand[inPos+1+HVBand.DimXAlign];
			HVHBand.pBand[outPos] = HVBand.pBand[inPos + HVBand.DimXAlign];
			HVHBand.pBand[outPos + 1] = HVBand.pBand[inPos + 1];
			outPos += 2;
		}
	}
}

void CWaveletDir::LazyBandI(void)
{
	int inPos, outPos;

	for(int j = 0; j < HVBand.DimY; j += 2){
		outPos = j * HVBand.DimXAlign;
		inPos = (j >> 1) * HVLBand.DimXAlign;
		for(int pEnd = outPos + HVBand.DimX; outPos < pEnd; outPos += 2){
			HVBand.pBand[outPos] = HVLBand.pBand[inPos];
			HVBand.pBand[outPos+1+HVBand.DimXAlign] = HVLBand.pBand[inPos + 1];
			HVBand.pBand[outPos + HVBand.DimXAlign] = HVHBand.pBand[inPos];
			HVBand.pBand[outPos + 1] = HVHBand.pBand[inPos + 1];
			inPos += 2;
		}
	}
}

#define PXL_LIFT_EVEN \
	pCur[0] += (pCur[1] + pCur[-1]) * (Coef[pDir[0]][0]) + \
	(pCur[stride] + pCur[-stride]) * (Coef[pDir[0]][1]); \
	pCur[1+stride] += (pCur[2+stride] + pCur[stride]) * (Coef[pDir[1+DimX]][0]) + \
	(pCur[1+2*stride] + pCur[1]) * (Coef[pDir[1+DimX]][1]);

#define PXL_LIFT_EVEN_EDGE(BitField) \
 pCur[0] += ((BitField)&LEFT?pCur[1] * 2:pCur[1] + pCur[-1]) * (Coef[pDir[0]][0]) + \
	((BitField)&TOP?pCur[stride] * 2:pCur[stride] + pCur[-stride]) * (Coef[pDir[0]][1]); \
	pCur[1+stride] += \
 ((BitField)&RIGHT?2*pCur[stride]:pCur[2+stride] + pCur[stride]) * (Coef[pDir[1+DimX]][0]) + \
	((BitField)&BOTTOM?2*pCur[1]:pCur[1+2*stride] + pCur[1]) * (Coef[pDir[1+DimX]][1]);

#define PXL_LIFT_ODD \
	pCur[1] += (pCur[2] + pCur[0]) * (Coef[pDir[1]][0]) + \
	(pCur[1+stride] + pCur[1-stride]) * (Coef[pDir[1]][1]); \
	pCur[stride] += (pCur[1+stride] + pCur[stride-1]) * (Coef[pDir[DimX]][0]) + \
	(pCur[2*stride] + pCur[0]) * (Coef[pDir[DimX]][1]);

#define PXL_LIFT_ODD_EDGE(BitField) \
	pCur[1] += ((BitField)&RIGHT?pCur[0] * 2:pCur[2] + pCur[0]) * (Coef[pDir[1]][0]) + \
	((BitField)&TOP?pCur[1+stride] * 2:pCur[1+stride] + pCur[1-stride]) * (Coef[pDir[1]][1]); \
	pCur[stride] += \
	((BitField)&LEFT?2*pCur[1+stride]:pCur[1+stride] + pCur[stride-1]) * (Coef[pDir[DimX]][0]) + \
	((BitField)&BOTTOM?2*pCur[0]:pCur[2*stride] + pCur[0]) * (Coef[pDir[DimX]][1]);

#define PXL_LIFT_EVEN_DIAG \
	pCur[0] += (pCur[1+stride] + pCur[-1-stride]) * (Coef[pDir[0]][0]) + \
	(pCur[-1+stride] + pCur[1-stride]) * (Coef[pDir[0]][1]);/* \
	pCur[1] += (pCur[2+stride] + pCur[-stride]) * (Coef[pDir[1]][0]) + \
	(pCur[stride] + pCur[2-stride]) * (Coef[pDir[1]][1]);*/

#define PXL_LIFT_EVEN_EDGE_DIAG(BitField) \
	pCur[0] += ((BitField)&(TOP|LEFT)?2*pCur[1+stride]:pCur[1+stride] + pCur[-1-stride]) * (Coef[pDir[0]][0]) + \
	((BitField)&TOP?(BitField)&LEFT?2*pCur[1+stride]:pCur[-1+stride]*2:(BitField)&LEFT?2*pCur[1-stride]:pCur[-1+stride] + pCur[1-stride]) * (Coef[pDir[0]][1]);/* \
	pCur[1] += ((BitField)&TOP?(BitField)&RIGHT?2*pCur[stride]:2*pCur[2+stride]:(BitField)&RIGHT?2*pCur[-stride]:pCur[2+stride] + pCur[-stride]) * (Coef[pDir[1]][0]) + \
	((BitField)&(TOP|RIGHT)?2*pCur[stride]:pCur[stride] + pCur[2-stride]) * (Coef[pDir[1]][1]);*/

#define PXL_LIFT_ODD_DIAG \
	/*pCur[stride] += (pCur[1+2*stride] + pCur[-1]) * (Coef[pDir[DimX]][0]) + \
	(pCur[-1+2*stride] + pCur[1]) * (Coef[pDir[DimX]][1]);*/ \
	pCur[1+stride] += (pCur[2+2*stride] + pCur[0]) * (Coef[pDir[1+DimX]][0]) + \
	(pCur[2*stride] + pCur[2]) * (Coef[pDir[1+DimX]][1]);

#define PXL_LIFT_ODD_EDGE_DIAG(BitField) \
	/*pCur[stride] += ((BitField)&BOTTOM?(BitField)&LEFT?2*pCur[1]:2*pCur[-1]:(BitField)&LEFT?2*pCur[1+2*stride]:pCur[1+2*stride] + pCur[-1]) * (Coef[pDir[DimX]][0]) + \
	((BitField)&(BOTTOM|LEFT)?2*pCur[1]:pCur[-1+2*stride] + pCur[1]) * (Coef[pDir[DimX]][1]);*/ \
	pCur[1+stride] += ((BitField)&(BOTTOM|RIGHT)?2*pCur[0]:pCur[2+2*stride] + pCur[0]) * (Coef[pDir[1+DimX]][0]) + \
	((BitField)&BOTTOM?(BitField)&RIGHT?2*pCur[0]:2*pCur[2]:(BitField)&RIGHT?2*pCur[2*stride]:pCur[2*stride] + pCur[2]) * (Coef[pDir[1+DimX]][1]);

#define PXL_LIFT \
	switch( lft_opt ){ \
		case even : \
			PXL_LIFT_EVEN; \
			break; \
		case odd : \
			PXL_LIFT_ODD; \
			break; \
		case diag_even: \
			PXL_LIFT_EVEN_DIAG; \
			break; \
		case diag_odd: \
			PXL_LIFT_ODD_DIAG; \
	}

#define PXL_LIFT_EDGE(BitField) \
	switch( lft_opt ){ \
		case even : \
			PXL_LIFT_EVEN_EDGE(BitField); \
			break; \
		case odd : \
			PXL_LIFT_ODD_EDGE(BitField); \
			break; \
		case diag_even : \
			PXL_LIFT_EVEN_EDGE_DIAG(BitField); \
			break; \
		case diag_odd : \
			PXL_LIFT_ODD_EDGE_DIAG(BitField); \
	}

template <lift lft_opt>
void CWaveletDir::LiftBand(float * pCur, int stride, int DimX, int DimY,
						   float Coef1, float Coef2, char * pDir)
{
	float Coef[3][2] = {
		{Coef1, Coef2},
		{(Coef1 + Coef2) * .5, (Coef1 + Coef2) * .5},
		{Coef2, Coef1}
	};
	PXL_LIFT_EDGE(TOP|LEFT);
	float * pNextCur = pCur + (stride << 1);
	char * pNextDir = pDir + (DimX << 1);
	pDir += 2; pCur += 2;
	for(int i = 2 ; i < DimX - 2; i += 2, pDir += 2, pCur += 2){
		PXL_LIFT_EDGE(TOP);
	}
	PXL_LIFT_EDGE(TOP|RIGHT);
	for(int j = 2; j < DimY - 2; j += 2){
		pCur = pNextCur; pDir = pNextDir;
		pNextCur += stride << 1; pNextDir += DimX << 1;
		PXL_LIFT_EDGE(LEFT);
		pDir += 2; pCur += 2;
		for(int i = 2; i < DimX - 2; i += 2, pDir += 2, pCur += 2){
			PXL_LIFT;
		}
		PXL_LIFT_EDGE(RIGHT);
	}
	pCur = pNextCur; pDir = pNextDir;
	pNextCur += stride << 1; pNextDir += DimX << 1;
	PXL_LIFT_EDGE(BOTTOM|LEFT);
	pDir += 2; pCur += 2;
	for(int i = 2 ; i < DimX - 2; i += 2, pDir += 2, pCur += 2){
		PXL_LIFT_EDGE(BOTTOM);
	}
	PXL_LIFT_EDGE(BOTTOM|RIGHT);
}

void CWaveletDir::Transform97(float * pImage, int stride, float lambda)
{
// 	float * pTmpImage = new float [stride * DimY];
//
// 	memcpy(pTmpImage, pImage, stride * DimY * sizeof(float));
// 	Transform97(pTmpImage, stride, true);
// 	HVMap.BuidNodes(lambda);
// 	HVMap.ApplyNodes();
// 	DMap.BuidNodes(lambda);
// 	DMap.ApplyNodes();
	Transform97(pImage, stride, true);

// 	delete[] pTmpImage;
}

/**
 * Tranform an image using the 9/7 wavelet transform
 *
 * output for 4 pixels : (e = even, o = odd)
 * -------------
 * | e e | o e |
 * | (L) | (DL)|
 * -------------
 * | o o | e o |
 * | (DH)| (HV)|
 * -------------
 *
 * @param pImage pointer on the image
 * @param stride width of the image buffer
 */
void CWaveletDir::Transform97(float * pImage, int stride, bool getDir)
{
	char * pDir = new char [DimX * DimY];

	if (getDir)
		GetImageDir97(pImage, stride);

	HVMap.GetDirs(pDir, DimX);
	LiftBand<odd>(pImage, stride, DimX, DimY, ALPHA1, ALPHA2, pDir);
	LiftBand<even>(pImage, stride, DimX, DimY, BETA1, BETA2, pDir);
	LiftBand<odd>(pImage, stride, DimX, DimY, GAMMA1, GAMMA2, pDir);
	LiftBand<even>(pImage, stride, DimX, DimY, DELTA1, DELTA2, pDir);

	if (getDir)
		GetImageDirDiag97(pImage, stride);

	DMap.GetDirsDiag(pDir, DimX);
	LiftBand<diag_odd>(pImage, stride, DimX, DimY, ALPHA1, ALPHA2, pDir);
	LiftBand<diag_even>(pImage, stride, DimX, DimY, BETA1, BETA2, pDir);
	LiftBand<diag_odd>(pImage, stride, DimX, DimY, GAMMA1, GAMMA2, pDir);
	LiftBand<diag_even>(pImage, stride, DimX, DimY, DELTA1, DELTA2, pDir);

	LazyImage(pImage, stride);

	delete[] pDir;
	if (pLow != 0)
		pLow->Transform97(pImage, stride, getDir);
}

void CWaveletDir::Transform97I(float * pImage, int stride)
{
	if (pLow != 0)
		pLow->Transform97I(pImage, stride);
	char * pDir = new char [DimX * DimY];

	LazyImageI(pImage, stride);

	DMap.GetDirsDiag(pDir, DimX);
	LiftBand<diag_even>(pImage, stride, DimX, DimY, -DELTA1, -DELTA2, pDir);
	LiftBand<diag_odd>(pImage, stride, DimX, DimY, -GAMMA1, -GAMMA2, pDir);
	LiftBand<diag_even>(pImage, stride, DimX, DimY, -BETA1, -BETA2, pDir);
	LiftBand<diag_odd>(pImage, stride, DimX, DimY, -ALPHA1, -ALPHA2, pDir);

	HVMap.GetDirs(pDir, DimX);
	LiftBand<even>(pImage, stride, DimX, DimY, -DELTA1, -DELTA2, pDir);
	LiftBand<odd>(pImage, stride, DimX, DimY, -GAMMA1, -GAMMA2, pDir);
	LiftBand<even>(pImage, stride, DimX, DimY, -BETA1, -BETA2, pDir);
	LiftBand<odd>(pImage, stride, DimX, DimY, -ALPHA1, -ALPHA2, pDir);

	delete[] pDir;
}

void CWaveletDir::SetWeight97(void)
{
	if (pHigh != 0){
		DLBand.Weight = pHigh->LBand.Weight;
		DHBand.Weight = pHigh->DLBand.Weight;
		HVBand.Weight = DLBand.Weight;
		HVHBand.Weight = pHigh->HVLBand.Weight;
		HVLBand.Weight = DLBand.Weight * XI;
		LBand.Weight = HVLBand.Weight * XI;
	}else{
		DHBand.Weight = 1./(XI * XI);
		DLBand.Weight = 1.;
		HVBand.Weight = 1.;
		HVHBand.Weight = 1./XI;
		HVLBand.Weight = XI;
		LBand.Weight = XI * XI;
	}
	if (pLow != 0)
		pLow->SetWeight97();
}

void CWaveletDir::GetImageDir97(float * pImage, int stride)
{
	float * pImage1 = new float [DimX * DimY];
	float * pImage2 = new float [DimX * DimY];
	char * pDir = new char [DimX * DimY];
	float * pCur = pImage, * pCur1 = pImage1, * pCur2 = pImage2;

	// copie de l'image x2
	for( int i = 0; i < DimY; i++){
		memcpy(pCur1, pCur, DimX * sizeof(float));
		memcpy(pCur2, pCur, DimX * sizeof(float));
		pCur += stride;
		pCur1 += DimX;
		pCur2 += DimX;
	}

	// lifting partiel 1
	memset(pDir, 0, DimX * DimY);
	LiftBand<odd>(pImage1, DimX, DimX, DimY, ALPHA1, ALPHA2, pDir);
	LiftBand<even>(pImage1, DimX, DimX, DimY, BETA1, BETA2, pDir);
	LiftBand<odd>(pImage1, DimX, DimX, DimY, GAMMA1, GAMMA2, pDir);

	// lifting partiel 2
	memset(pDir, 2, DimX * DimY);
	LiftBand<odd>(pImage2, DimX, DimX, DimY, ALPHA1, ALPHA2, pDir);
	LiftBand<even>(pImage2, DimX, DimX, DimY, BETA1, BETA2, pDir);
	LiftBand<odd>(pImage2, DimX, DimX, DimY, GAMMA1, GAMMA2, pDir);

	// calcul des distortions
	HVMap.SelectDir(pImage1, pImage2, DimX);

	// desallocation de la mémoire
	delete[] pImage1;
	delete[] pImage2;
	delete[] pDir;
}

void CWaveletDir::GetImageDirDiag97(float * pImage, int stride)
{
	float * pImage1 = new float [DimX * DimY];
	float * pImage2 = new float [DimX * DimY];
	char * pDir = new char [DimX * DimY];
	float * pCur = pImage, * pCur1 = pImage1, * pCur2 = pImage2;

	// copie de l'image x2
	for( int i = 0; i < DimY;i++ ){
		memcpy(pCur1, pCur, DimX * sizeof(float));
		memcpy(pCur2, pCur, DimX * sizeof(float));
		pCur += stride;
		pCur1 += DimX;
		pCur2 += DimX;
	}

	// lifting partiel 1
	memset(pDir, 0, DimX * DimY);
	LiftBand<diag_odd>(pImage1, DimX, DimX, DimY, ALPHA1, ALPHA2, pDir);
	LiftBand<diag_even>(pImage1, DimX, DimX, DimY, BETA1, BETA2, pDir);
	LiftBand<diag_odd>(pImage1, DimX, DimX, DimY, GAMMA1, GAMMA2, pDir);

	// lifting partiel 2
	memset(pDir, 2, DimX * DimY);
	LiftBand<diag_odd>(pImage2, DimX, DimX, DimY, ALPHA1, ALPHA2, pDir);
	LiftBand<diag_even>(pImage2, DimX, DimX, DimY, BETA1, BETA2, pDir);
	LiftBand<diag_odd>(pImage2, DimX, DimX, DimY, GAMMA1, GAMMA2, pDir);

	// calcul des distortions
	DMap.SelectDirDiag(pImage1, pImage2, DimX);

	// desallocation de la mémoire
	delete[] pImage1;
	delete[] pImage2;
	delete[] pDir;
}

unsigned int CWaveletDir::TSUQ(float Quant, float Thres)
{
	unsigned int Count = 0;
	Count += DLBand.TSUQ(Quant, Thres);
	Count += DHBand.TSUQ(Quant, Thres);

	if (pLow != 0) {
		Count += pLow->TSUQ(Quant, Thres);
		Count += HVHBand.TSUQ(Quant, Thres);
		Count += HVLBand.TSUQ(Quant, Thres);
// 		HVWav.TSUQ(Quant, Thres, 1);
	} else {
		Count += LBand.TSUQ(Quant, Quant * .5);
		Count += HVHBand.TSUQ(Quant, Thres);
		Count += HVLBand.TSUQ(Quant, Thres);
// 		Count += HVBand.TSUQ(Quant, Thres);
// 		HVWav.TSUQ(Quant, Thres, 1);
	}
	return Count;
}

void CWaveletDir::TSUQi(float Quant, float RecLevel)
{
	DLBand.TSUQi(Quant, RecLevel);
	DHBand.TSUQi(Quant, RecLevel);

	if (pLow != 0){
		pLow->TSUQi(Quant, RecLevel);
		HVHBand.TSUQi(Quant, RecLevel);
		HVLBand.TSUQi(Quant, RecLevel);
// 		HVWav.TSUQi(Quant, RecLevel, 1);
	} else {
		LBand.TSUQi(Quant, 0);
		HVHBand.TSUQi(Quant, RecLevel);
		HVLBand.TSUQi(Quant, RecLevel);
// 		HVBand.TSUQi(Quant, RecLevel);
// 		HVWav.TSUQi(Quant, RecLevel, 1);
	}
}

}
