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
	DHBand.Init(DimX >> 1, DimY >> 1, Align);
	DLBand.Init(DimX >> 1, DimY >> 1, Align);
	HVBand.Init(DimX >> 1, DimY >> 1, Align);
	HVHBand.Init(DimX >> 1, DimY >> 2, Align);
	HVLBand.Init(DimX >> 1, DimY >> 2, Align);
	HVMap.Init(DimX, DimY);
	DMap.Init(DimX, DimY);
	Level = level;

	if (level > 1){
		pLow = new CWaveletDir(DimX >> 1, DimY >> 1, level - 1, Align, this);
	}else{
		LBand.Init(DimX >> 1, DimY >> 1, Align);
		LMap.Init(DimX >> 1, DimY >> 1);
	}
}

void CWaveletDir::SetCodec(CMuxCodec * Codec)
{
	HVMap.SetCodec(Codec);
	DMap.SetCodec(Codec);

	if (pLow != 0)
		pLow->SetCodec(Codec);
	else
		LMap.SetCodec(Codec);
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

void CWaveletDir::GetDist(unsigned char * pOut, int level, int Direction)
{
	CWaveletDir * pCurWav = this;
	while( pCurWav != 0 && pCurWav->Level != level ){
		pCurWav = pCurWav->pLow;
	}
	if (pCurWav != 0) {
		if (Direction == 0)
			pCurWav->HVMap.GetDist(pOut);
		else
			pCurWav->DMap.GetDist(pOut);
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

void CWaveletDir::CodeMap(int Options)
{
	if (Options == 0) {
		HVMap.CodeNodes();
		DMap.CodeNodes();
	}
}

void CWaveletDir::DecodeMap(int Options)
{
	if (Options == 0) {
		HVMap.DecodeNodes();
		DMap.DecodeNodes();
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
		case 0 :
			while( pCurWav != 0 ){
				pCurWav->DLBand.enu<encode>(pCodec);
				pCurWav->DHBand.enu<encode>(pCodec);
				pCurWav->HVLBand.enu<encode>(pCodec);
				pCurWav->HVHBand.enu<encode>(pCodec);
				if (pCurWav->pLow == 0){
					pCurWav->LBand.pred<encode>(pCodec);
				}
				pCurWav = pCurWav->pLow;
			}
		case 1 :
			pCurWav->DLBand.buildTree<true>();
			pCurWav->DHBand.buildTree<true>();
			pCurWav->HVLBand.buildTree<true>();
			pCurWav->HVHBand.buildTree<true>();
			while( pCurWav->pLow ) pCurWav = pCurWav->pLow;
			pCurWav->LBand.pred<encode>(pCodec);
			while( pCurWav ) {
				pCurWav->DLBand.tree<encode>(pCodec);
				pCurWav->DHBand.tree<encode>(pCodec);
				pCurWav->HVLBand.tree<encode>(pCodec);
				pCurWav->HVHBand.tree<encode>(pCodec);
				pCurWav = pCurWav->pHigh;
			}
	}
}

void CWaveletDir::DecodeBand(CMuxCodec * pCodec, int method)
{
	CWaveletDir * pCurWav = this;

	switch( method ){
		case 0 :
			while( pCurWav != 0 ){
				pCurWav->DLBand.enu<decode>(pCodec);
				pCurWav->DHBand.enu<decode>(pCodec);
				pCurWav->HVLBand.enu<decode>(pCodec);
				pCurWav->HVHBand.enu<decode>(pCodec);
				if (pCurWav->pLow == 0)
					pCurWav->LBand.pred<decode>(pCodec);
				pCurWav = pCurWav->pLow;
			}
		case 1 :
			while( pCurWav->pLow ) pCurWav = pCurWav->pLow;
			pCurWav->LBand.pred<decode>(pCodec);
			while( pCurWav ) {
				pCurWav->DLBand.tree<decode>(pCodec);
				pCurWav->DHBand.tree<decode>(pCodec);
				pCurWav->HVLBand.tree<decode>(pCodec);
				pCurWav->HVHBand.tree<decode>(pCodec);
				pCurWav = pCurWav->pHigh;
			}
	}
}

void CWaveletDir::LazyImage(float * pImage, unsigned int stride)
{
	int Bandpos, Ipos1, Ipos2, Lstride;
	float * pLOut;
	if (pLow == 0){
		pLOut = (float*)LBand.pBand;
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
			DLBand.pBand[Bandpos].f = pImage[Ipos1 + 1];
			DHBand.pBand[Bandpos].f = pImage[Ipos1 + stride];
			HVBand.pBand[Bandpos].f = pImage[Ipos1 + stride + 1];
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
		pLIn = (float*)LBand.pBand;
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
			pImage[Ipos1 + 1] = DLBand.pBand[Bandpos].f;
			pImage[Ipos1 + stride] = DHBand.pBand[Bandpos].f;
			pImage[Ipos1 + stride + 1] = HVBand.pBand[Bandpos].f;
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

#define PXL_LIFT_EVEN(Coef1,Coef2) \
	pCur[0] += (pCur[1] + pCur[-1]) * (Coef1) + \
	(pCur[stride] + pCur[-stride]) * (Coef2); \
	pCur[1+stride] += (pCur[2+stride] + pCur[stride]) * (Coef1) + \
	(pCur[1+2*stride] + pCur[1]) * (Coef2);

#define PXL_LIFT_EVEN_EDGE(Coef1,Coef2,BitField) \
 pCur[0] += ((BitField)&LEFT?pCur[1] * 2:pCur[1] + pCur[-1]) * (Coef1) + \
	((BitField)&TOP?pCur[stride] * 2:pCur[stride] + pCur[-stride]) * (Coef2); \
	pCur[1+stride] += \
 ((BitField)&RIGHT?2*pCur[stride]:pCur[2+stride] + pCur[stride]) * (Coef1) + \
	((BitField)&BOTTOM?2*pCur[1]:pCur[1+2*stride] + pCur[1]) * (Coef2);

#define PXL_LIFT_ODD(Coef1,Coef2) \
	pCur[1] += (pCur[2] + pCur[0]) * (Coef1) + \
	(pCur[1+stride] + pCur[1-stride]) * (Coef2); \
	pCur[stride] += (pCur[1+stride] + pCur[stride-1]) * (Coef1) + \
	(pCur[2*stride] + pCur[0]) * (Coef2);

#define PXL_LIFT_ODD_EDGE(Coef1,Coef2,BitField) \
	pCur[1] += ((BitField)&RIGHT?pCur[0] * 2:pCur[2] + pCur[0]) * (Coef1) + \
	((BitField)&TOP?pCur[1+stride] * 2:pCur[1+stride] + pCur[1-stride]) * (Coef2); \
	pCur[stride] += \
	((BitField)&LEFT?2*pCur[1+stride]:pCur[1+stride] + pCur[stride-1]) * (Coef1) + \
	((BitField)&BOTTOM?2*pCur[0]:pCur[2*stride] + pCur[0]) * (Coef2);

#define PXL_LIFT_EVEN_DIAG(Coef1,Coef2) \
	pCur[0] += (pCur[1+stride] + pCur[-1-stride]) * (Coef1) + \
	(pCur[-1+stride] + pCur[1-stride]) * (Coef2); \
	pCur[1] += (pCur[2+stride] + pCur[-stride]) * (Coef1) + \
	(pCur[stride] + pCur[2-stride]) * (Coef2);

#define PXL_LIFT_EVEN_EDGE_DIAG(Coef1,Coef2,BitField) \
	pCur[0] += ((BitField)&(TOP|LEFT)?2*pCur[1+stride]:pCur[1+stride] + pCur[-1-stride]) * (Coef1) + \
	((BitField)&TOP?(BitField)&LEFT?2*pCur[1+stride]:pCur[-1+stride]*2:(BitField)&LEFT?2*pCur[1-stride]:pCur[-1+stride] + pCur[1-stride]) * (Coef2); \
	pCur[1] += ((BitField)&TOP?(BitField)&RIGHT?2*pCur[stride]:2*pCur[2+stride]:(BitField)&RIGHT?2*pCur[-stride]:pCur[2+stride] + pCur[-stride]) * (Coef1) + \
	((BitField)&(TOP|RIGHT)?2*pCur[stride]:pCur[stride] + pCur[2-stride]) * (Coef2);

#define PXL_LIFT_ODD_DIAG(Coef1,Coef2) \
	pCur[stride] += (pCur[1+2*stride] + pCur[-1]) * (Coef1) + \
	(pCur[-1+2*stride] + pCur[1]) * (Coef2); \
	pCur[1+stride] += (pCur[2+2*stride] + pCur[0]) * (Coef1) + \
	(pCur[2*stride] + pCur[2]) * (Coef2);

#define PXL_LIFT_ODD_EDGE_DIAG(Coef1,Coef2,BitField) \
	pCur[stride] += ((BitField)&BOTTOM?(BitField)&LEFT?2*pCur[1]:2*pCur[-1]:(BitField)&LEFT?2*pCur[1+2*stride]:pCur[1+2*stride] + pCur[-1]) * (Coef1) + \
	((BitField)&(BOTTOM|LEFT)?2*pCur[1]:pCur[-1+2*stride] + pCur[1]) * (Coef2); \
	pCur[1+stride] += ((BitField)&(BOTTOM|RIGHT)?2*pCur[0]:pCur[2+2*stride] + pCur[0]) * (Coef1) + \
	((BitField)&BOTTOM?(BitField)&RIGHT?2*pCur[0]:2*pCur[2]:(BitField)&RIGHT?2*pCur[2*stride]:pCur[2*stride] + pCur[2]) * (Coef2);

#define PXL_LIFT(Coef1,Coef2) \
	switch( lft_opt ){ \
		case even : \
			PXL_LIFT_EVEN(Coef1,Coef2); \
			break; \
		case odd : \
			PXL_LIFT_ODD(Coef1,Coef2); \
			break; \
		case diag_even: \
			PXL_LIFT_EVEN_DIAG(Coef1,Coef2); \
			break; \
		case diag_odd: \
			PXL_LIFT_ODD_DIAG(Coef1,Coef2); \
			break; \
	}

#define PXL_LIFT_EDGE(Coef1,Coef2,BitField) \
	switch( lft_opt ){ \
		case even : \
			PXL_LIFT_EVEN_EDGE(Coef1,Coef2,BitField); \
			break; \
		case odd : \
			PXL_LIFT_ODD_EDGE(Coef1,Coef2,BitField); \
			break; \
		case diag_even : \
			PXL_LIFT_EVEN_EDGE_DIAG(Coef1,Coef2,BitField); \
			break; \
		case diag_odd : \
			PXL_LIFT_ODD_EDGE_DIAG(Coef1,Coef2,BitField); \
			break; \
}

template <lift lft_opt>
void CWaveletDir::LiftBand(float * pCur, int stride, int DimX, int DimY,
						   float Coef1, float Coef2, char * pDir)
{
	float Coef[3][2] = {
		{Coef1, Coef2},
		{Coef2, Coef1},
		{(Coef1 + Coef2) * .5, (Coef1 + Coef2) * .5}
	};
	char * pDirOld = pDir;
	PXL_LIFT_EDGE(Coef[*pDir][0], Coef[*pDir][1], TOP|LEFT);
	float * pNextCur = pCur + (stride << 1);
	pCur += 2;
	for(int i = 2 ; i < DimX - 2;pCur += 2){
		PXL_LIFT_EDGE(Coef[*pDir][0], Coef[*pDir][1], TOP);
		i += 2;
		pDir += (i & 3) == 0;
	}
	PXL_LIFT_EDGE(Coef[*pDir][0], Coef[*pDir][1], TOP|RIGHT);
	pDir = pDirOld;
	for(int j = 2; j < DimY - 2; j += 2){
		pCur = pNextCur;
		pNextCur += stride << 1;
		PXL_LIFT_EDGE(Coef[*pDir][0], Coef[*pDir][1], LEFT);
		pCur += 2;
		for(int i = 2; i < DimX - 2; pCur += 2){
			PXL_LIFT(Coef[*pDir][0], Coef[*pDir][1]);
			i += 2;
			pDir += (i & 3) == 0;
		}
		PXL_LIFT_EDGE(Coef[*pDir][0], Coef[*pDir][1], RIGHT);
		pDir++;
		if ((j & 3) == 0)
			pDir = pDirOld;
		pDirOld = pDir;
	}
	pCur = pNextCur;
	pNextCur += stride << 1;
	PXL_LIFT_EDGE(Coef[*pDir][0], Coef[*pDir][1], BOTTOM|LEFT);
	pCur += 2;
	for(int i = 2 ; i < DimX - 2; pCur += 2){
		PXL_LIFT_EDGE(Coef[*pDir][0], Coef[*pDir][1], BOTTOM);
		i += 2;
		pDir += (i & 3) == 0;
	}
	PXL_LIFT_EDGE(Coef[*pDir][0], Coef[*pDir][1], BOTTOM|RIGHT);
}

void CWaveletDir::Transform97(float * pImage, int stride, const int lambda)
{
	float * pTmpImage = new float [stride * DimY];

	memcpy(pTmpImage, pImage, stride * DimY * sizeof(float));
	Transform97(pTmpImage, stride, true);
	HVMap.BuidNodes(lambda);
	HVMap.ApplyNodes();
	DMap.BuidNodes(lambda);
	DMap.ApplyNodes();
	Transform97(pImage, stride, false);

	delete[] pTmpImage;
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
	if (getDir)
		GetImageDir97(pImage, stride);
	else if (pHigh != 0) {
		float * pBand = (float*)pHigh->HVBand.pBand;
		int DimX = pHigh->HVBand.DimX, DimY = pHigh->HVBand.DimY;
		int stride = pHigh->HVBand.DimXAlign;
		LiftBand<odd>(pBand, stride, DimX, DimY, ALPHA1, ALPHA2, HVMap.pMap);
		LiftBand<even>(pBand, stride, DimX, DimY, BETA1, BETA2, HVMap.pMap);
		LiftBand<odd>(pBand, stride, DimX, DimY, GAMMA1, GAMMA2, HVMap.pMap);
		LiftBand<even>(pBand, stride, DimX, DimY, DELTA1, DELTA2, HVMap.pMap);

		pHigh->LazyBand();
	}

	LiftBand<odd>(pImage, stride, DimX, DimY, ALPHA1, ALPHA2, HVMap.pMap);
	LiftBand<even>(pImage, stride, DimX, DimY, BETA1, BETA2, HVMap.pMap);
	LiftBand<odd>(pImage, stride, DimX, DimY, GAMMA1, GAMMA2, HVMap.pMap);
	LiftBand<even>(pImage, stride, DimX, DimY, DELTA1, DELTA2, HVMap.pMap);

	if (getDir)
		GetImageDirDiag97(pImage, stride);

	LiftBand<diag_odd>(pImage, stride, DimX, DimY, ALPHA1, ALPHA2, DMap.pMap);
	LiftBand<diag_even>(pImage, stride, DimX, DimY, BETA1, BETA2, DMap.pMap);
	LiftBand<diag_odd>(pImage, stride, DimX, DimY, GAMMA1, GAMMA2, DMap.pMap);
	LiftBand<diag_even>(pImage, stride, DimX, DimY, DELTA1, DELTA2, DMap.pMap);

	LazyImage(pImage, stride);

// 	HVWav.Transform97(HVBand.pBand, HVBand.DimXAlign);

	if (pLow != 0){
		pLow->Transform97(pImage, stride, getDir);
	} else {
		if (getDir)
			GetBandDir97();
		else {
			float * pBand = (float*)HVBand.pBand;
			int DimX = HVBand.DimX, DimY = HVBand.DimY;
			int stride = HVBand.DimXAlign;
			LiftBand<odd>(pBand, stride, DimX, DimY, ALPHA1, ALPHA2, LMap.pMap);
			LiftBand<even>(pBand, stride, DimX, DimY, BETA1, BETA2, LMap.pMap);
			LiftBand<odd>(pBand, stride, DimX, DimY, GAMMA1, GAMMA2, LMap.pMap);
			LiftBand<even>(pBand, stride, DimX, DimY, DELTA1, DELTA2, LMap.pMap);

			LazyBand();
		}
	}
}

void CWaveletDir::Transform97I(float * pImage, int stride)
{
	if (pLow != 0)
		pLow->Transform97I(pImage, stride);
	else {
		LazyBandI();

		float * pBand = (float*)HVBand.pBand;
		int DimX = HVBand.DimX, DimY = HVBand.DimY;
		int stride = HVBand.DimXAlign;
		LiftBand<even>(pBand, stride, DimX, DimY, -DELTA1, -DELTA2, LMap.pMap);
		LiftBand<odd>(pBand, stride, DimX, DimY, -GAMMA1, -GAMMA2, LMap.pMap);
		LiftBand<even>(pBand, stride, DimX, DimY, -BETA1, -BETA2, LMap.pMap);
		LiftBand<odd>(pBand, stride, DimX, DimY, -ALPHA1, -ALPHA2, LMap.pMap);
	}

// 	HVWav.Transform97I(HVBand.pBand, HVBand.DimXAlign);

	LazyImageI(pImage, stride);

	LiftBand<diag_even>(pImage, stride, DimX, DimY, -DELTA1, -DELTA2, DMap.pMap);
	LiftBand<diag_odd>(pImage, stride, DimX, DimY, -GAMMA1, -GAMMA2, DMap.pMap);
	LiftBand<diag_even>(pImage, stride, DimX, DimY, -BETA1, -BETA2, DMap.pMap);
	LiftBand<diag_odd>(pImage, stride, DimX, DimY, -ALPHA1, -ALPHA2, DMap.pMap);

	LiftBand<even>(pImage, stride, DimX, DimY, -DELTA1, -DELTA2, HVMap.pMap);
	LiftBand<odd>(pImage, stride, DimX, DimY, -GAMMA1, -GAMMA2, HVMap.pMap);
	LiftBand<even>(pImage, stride, DimX, DimY, -BETA1, -BETA2, HVMap.pMap);
	LiftBand<odd>(pImage, stride, DimX, DimY, -ALPHA1, -ALPHA2, HVMap.pMap);

	if (pHigh != 0) {
		pHigh->LazyBandI();

		float * pBand = (float*)pHigh->HVBand.pBand;
		int DimX = pHigh->HVBand.DimX, DimY = pHigh->HVBand.DimY;
		int stride = pHigh->HVBand.DimXAlign;
		LiftBand<even>(pBand, stride, DimX, DimY, -DELTA1, -DELTA2, HVMap.pMap);
		LiftBand<odd>(pBand, stride, DimX, DimY, -GAMMA1, -GAMMA2, HVMap.pMap);
		LiftBand<even>(pBand, stride, DimX, DimY, -BETA1, -BETA2, HVMap.pMap);
		LiftBand<odd>(pBand, stride, DimX, DimY, -ALPHA1, -ALPHA2, HVMap.pMap);
	}
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

		HVMap.weightH = pHigh->HVMap.weightL;
		HVMap.weightL = HVMap.weightH * XI * XI;
		DMap.weightH = pHigh->DMap.weightL;
		DMap.weightL = DMap.weightH * XI * XI;

	}else{
		DHBand.Weight = 1./(XI * XI);
		DLBand.Weight = 1.;
		HVBand.Weight = 1.;
		HVHBand.Weight = 1./XI;
		HVLBand.Weight = XI;
		LBand.Weight = XI * XI;

		HVMap.weightL = 1./XI;
		DMap.weightL = 1.;
		DMap.weightH = 1./ (XI * XI);
	}
	if (pLow != 0)
		pLow->SetWeight97();
	else {
		LMap.weightH = HVMap.weightL;
		LMap.weightL = LMap.weightH * XI * XI;
	}
}

void CWaveletDir::GetImageDir97(float * pImage, int stride)
{
	float * pImage1 = new float [DimX * DimY];
	float * pImage2 = new float [DimX * DimY];
	float * pBand1 = 0, * pBand2 = 0;
	float * pCur = pImage, * pCur1 = pImage1, * pCur2 = pImage2;

	// copie de l'image x2
	for( int i = 0; i < DimY; i++){
		memcpy(pCur1, pCur, DimX * sizeof(float));
		memcpy(pCur2, pCur, DimX * sizeof(float));
		pCur += stride;
		pCur1 += DimX;
		pCur2 += DimX;
	}

	// copie de la bande HV
	if (pHigh != 0) {
		pBand1 = new float [DimX * DimY];
		pBand2 = new float [DimX * DimY];
		pCur = (float*)pHigh->HVBand.pBand;
		pCur1 = pBand1;
		pCur2 = pBand2;
		for( int i = 0; i < DimY; i++){
			memcpy(pCur1, pCur, DimX * sizeof(float));
			memcpy(pCur2, pCur, DimX * sizeof(float));
			pCur += pHigh->HVBand.DimXAlign;
			pCur1 += DimX;
			pCur2 += DimX;
		}
	}

	// lifting partiel 1
	HVMap.SetDir(0);
	LiftBand<odd>(pImage1, DimX, DimX, DimY, ALPHA1, ALPHA2, HVMap.pMap);
	LiftBand<even>(pImage1, DimX, DimX, DimY, BETA1, BETA2, HVMap.pMap);
	LiftBand<odd>(pImage1, DimX, DimX, DimY, GAMMA1, GAMMA2, HVMap.pMap);
	if (pHigh != 0) {
		LiftBand<odd>(pBand1, DimX, DimX, DimY, ALPHA1, ALPHA2, HVMap.pMap);
		LiftBand<even>(pBand1, DimX, DimX, DimY, BETA1, BETA2, HVMap.pMap);
		LiftBand<odd>(pBand1, DimX, DimX, DimY, GAMMA1, GAMMA2, HVMap.pMap);
	}

	// lifting partiel 2
	HVMap.SetDir(1);
	LiftBand<odd>(pImage2, DimX, DimX, DimY, ALPHA1, ALPHA2, HVMap.pMap);
	LiftBand<even>(pImage2, DimX, DimX, DimY, BETA1, BETA2, HVMap.pMap);
	LiftBand<odd>(pImage2, DimX, DimX, DimY, GAMMA1, GAMMA2, HVMap.pMap);
	if (pHigh != 0) {
		LiftBand<odd>(pBand2, DimX, DimX, DimY, ALPHA1, ALPHA2, HVMap.pMap);
		LiftBand<even>(pBand2, DimX, DimX, DimY, BETA1, BETA2, HVMap.pMap);
		LiftBand<odd>(pBand2, DimX, DimX, DimY, GAMMA1, GAMMA2, HVMap.pMap);
	}

	// calcul des distortions
	if (pHigh == 0)
		HVMap.GetImageDist(pImage1, pImage2, DimX);
	else
		HVMap.GetImageDist(pImage1, pImage2, pBand1, pBand2, DimX);

	HVMap.SelectDir();

	// desallocation de la mémoire
	delete[] pImage1;
	delete[] pImage2;
	delete[] pBand1;
	delete[] pBand2;
}

void CWaveletDir::GetBandDir97(void)
{
	float * pBand1 = new float [HVBand.BandSize];
	float * pBand2 = new float [HVBand.BandSize];
	float * pCur = (float*)HVBand.pBand, * pCur1 = pBand1, * pCur2 = pBand2;
	int stride = HVBand.DimXAlign;
	int DimX = HVBand.DimX;
	int DimY = HVBand.DimY;

	// copie de la bande HV
	for( int i = 0; i < HVBand.DimY; i++){
		memcpy(pCur1, pCur, HVBand.DimX * sizeof(float));
		memcpy(pCur2, pCur, HVBand.DimX * sizeof(float));
		pCur += stride;
		pCur1 += stride;
		pCur2 += stride;
	}

	// lifting partiel 1
	LMap.SetDir(0);
	LiftBand<odd>(pBand1, stride, DimX, DimY, ALPHA1, ALPHA2, LMap.pMap);
	LiftBand<even>(pBand1, stride, DimX, DimY, BETA1, BETA2, LMap.pMap);
	LiftBand<odd>(pBand1, stride, DimX, DimY, GAMMA1, GAMMA2, LMap.pMap);

	// lifting partiel 2
	LMap.SetDir(1);
	LiftBand<odd>(pBand2, stride, DimX, DimY, ALPHA1, ALPHA2, LMap.pMap);
	LiftBand<even>(pBand2, stride, DimX, DimY, BETA1, BETA2, LMap.pMap);
	LiftBand<odd>(pBand2, stride, DimX, DimY, GAMMA1, GAMMA2, LMap.pMap);

	// calcul des distortions
	LMap.GetImageDist(pBand1, pBand2, stride);

	LMap.SelectDir();

	// desallocation de la mémoire
	delete[] pBand1;
	delete[] pBand2;
}

void CWaveletDir::GetImageDirDiag97(float * pImage, int stride)
{
	float * pImage1 = new float [DimX * DimY];
	float * pImage2 = new float [DimX * DimY];
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
	DMap.SetDir(0);
	LiftBand<diag_odd>(pImage1, DimX, DimX, DimY, ALPHA1, ALPHA2, DMap.pMap);
	LiftBand<diag_even>(pImage1, DimX, DimX, DimY, BETA1, BETA2, DMap.pMap);
	LiftBand<diag_odd>(pImage1, DimX, DimX, DimY, GAMMA1, GAMMA2, DMap.pMap);

	// lifting partiel 2
	DMap.SetDir(1);
	LiftBand<diag_odd>(pImage2, DimX, DimX, DimY, ALPHA1, ALPHA2, DMap.pMap);
	LiftBand<diag_even>(pImage2, DimX, DimX, DimY, BETA1, BETA2, DMap.pMap);
	LiftBand<diag_odd>(pImage2, DimX, DimX, DimY, GAMMA1, GAMMA2, DMap.pMap);

	// calcul des distortions
	DMap.GetImageDistDiag(pImage1, pImage2, DimX);
 	DMap.SelectDir();

	// desallocation de la mémoire
	delete[] pImage1;
	delete[] pImage2;
}

unsigned int CWaveletDir::TSUQ(float Quant, float Thres)
{
	unsigned int Count = 0;
	Count += DLBand.TSUQ(Quant, Thres);
	Count += DHBand.TSUQ(Quant, Thres);
	Count += HVHBand.TSUQ(Quant, Thres);
	Count += HVLBand.TSUQ(Quant, Thres);
// 	Count += HVBand.TSUQ(Quant, Thres);

	if (pLow != 0) {
		Count += pLow->TSUQ(Quant, Thres);
	} else {
		Count += LBand.TSUQ(Quant, Quant * .5);
	}
	return Count;
}

void CWaveletDir::TSUQi(float Quant, float RecLevel)
{
	DLBand.TSUQi(Quant, RecLevel);
	DHBand.TSUQi(Quant, RecLevel);
	HVHBand.TSUQi(Quant, RecLevel);
	HVLBand.TSUQi(Quant, RecLevel);
// 	HVBand.TSUQi(Quant, RecLevel);

	if (pLow != 0){
		pLow->TSUQi(Quant, RecLevel);
	} else {
		LBand.TSUQi(Quant, 0);
	}
}

}
