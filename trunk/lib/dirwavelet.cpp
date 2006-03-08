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

#include "libdwic.h"
#include <iostream>

using namespace std;

namespace libdwic {

DirWavelet::DirWavelet(int x, int y, int level, int Align):
DimX(x),
DimY(y),
pHigh(0),
pLow(0),
HVMap((CMap*)0),
DMap((CMap*)0)
{
	if (level > MAX_WAV_LEVEL)
		level = MAX_WAV_LEVEL;
	Init(level, Align);
}

DirWavelet::DirWavelet(int x, int y, int level, DirWavelet * pHigh, int Align):
DimX(x),
DimY(y),
pHigh(0),
pLow(0),
HVMap(&pHigh->HVMap),
DMap(&pHigh->DMap)
{
	this->pHigh = pHigh;
// 	pHigh->DBand.pParent = &DBand;
// 	DBand.pChild = &pHigh->DBand;
// 	pHigh->HVBand.pParent = &HVBand;
// 	HVBand.pChild = &pHigh->HVBand;
	Init(level, Align);
}

DirWavelet::~DirWavelet()
{
	delete pLow;
	delete[] pD1D;
	delete[] pHV1D;
}

void DirWavelet::Init(int level, int Align)
{
	DBand.Init(DimX, DimY >> 1, Align);
	HVBand.Init(DimX >> 1, DimY >> 1, Align);
	HVMap.Init(DimX, DimY);
	DMap.Init(DimX, DimY);
	Level = level;
	InitFuncPtr();

	pD1D = new float[DimX * (DimY >> 1)];
	pHV1D = new float[(DimX >> 1) * (DimY >> 1)];

	if (level > 1){
		pLow = new DirWavelet(DimX >> 1, DimY >> 1, level - 1, this, Align);
	}else{
		LBand.Init(DimX >> 1, DimY >> 1, Align);
	}
}

void DirWavelet::SetRange(CRangeCodec * RangeCodec)
{
	HVMap.SetRange(RangeCodec);
	DMap.SetRange(RangeCodec);

	if (pLow != 0)
		pLow->SetRange(RangeCodec);
}

void DirWavelet::Stats(void)
{
	float Mean = 0, Var = 0;
	DBand.Mean(Mean, Var);
	cout << "Niveau (D) : " << Level << endl;
	cout << "Moyenne : " << Mean << endl;
	cout << "Variance : " << Var << endl;
	HVBand.Mean(Mean, Var);
	cout << "Niveau (HV) : " << Level << endl;
	cout << "Moyenne : " << Mean << endl;
	cout << "Variance : " << Var << endl;
	cout << "Hor : " << DimHVDir << endl;
	cout << "Ver : " << HVBand.BandSize - DimHVDir << endl;

	if (pLow != 0)
		pLow->Stats();
	else{
		LBand.Mean(Mean, Var);
		cout << "Niveau (L) : " << Level << endl;
		cout << "Moyenne : " << Mean << endl;
		cout << "Variance : " << Var << endl;
	}
}

void DirWavelet::SetSelected(int Sel)
{
	HVMap.SetSelected(Sel);
	DMap.SetSelected(Sel);
	if (pLow != 0)
		pLow->SetSelected(Sel);
}

void DirWavelet::CodeMap(int Options)
{
	DirWavelet * pCurWav = this;

	if (Options == 2){
		while( pCurWav->pLow != 0 ){
			pCurWav = pCurWav->pLow;
			pCurWav->HVMap.TreeSum();
			pCurWav->DMap.TreeSum();
		}
	}

	while( pCurWav->pLow != 0 ){
		pCurWav = pCurWav->pLow;
	}

	if (Options == 0) {
		pCurWav->DMap.Order0Code();
		pCurWav->HVMap.Order0Code();
		while( pCurWav->pHigh != 0 ){
			pCurWav = pCurWav->pHigh;
			pCurWav->DMap.Order0Code();
			pCurWav->HVMap.Order0Code();
		}
	} else if (Options == 1) {
		pCurWav->DMap.Neighbor4Code();
		pCurWav->HVMap.Neighbor4Code();
		while( pCurWav->pHigh != 0 ){
			pCurWav = pCurWav->pHigh;
			pCurWav->DMap.Neighbor4Code();
			pCurWav->HVMap.Neighbor4Code();
		}
	} else if (Options == 2) {
		pCurWav->DMap.Neighbor4Code(1);
		pCurWav->HVMap.Neighbor4Code(1);
		while( pCurWav->pHigh != 0 ){
			pCurWav = pCurWav->pHigh;
			pCurWav->DMap.TreeCode();
			pCurWav->HVMap.TreeCode();
		}
	}
}

void DirWavelet::DecodeMap(int Options)
{
	DirWavelet * pCurWav = this;
	while( pCurWav->pLow != 0 ){
		pCurWav = pCurWav->pLow;
	}

	if (Options == 0) {
		pCurWav->DMap.Order0Dec();
		pCurWav->HVMap.Order0Dec();
		while( pCurWav->pHigh != 0 ){
			pCurWav = pCurWav->pHigh;
			pCurWav->DMap.Order0Dec();
			pCurWav->HVMap.Order0Dec();
		}
	} else if (Options == 1) {
		pCurWav->DMap.Neighbor4Dec();
		pCurWav->HVMap.Neighbor4Dec();
		while( pCurWav->pHigh != 0 ){
			pCurWav = pCurWav->pHigh;
			pCurWav->DMap.Neighbor4Dec();
			pCurWav->HVMap.Neighbor4Dec();
		}
	} else if (Options == 2) {
		pCurWav->DMap.Neighbor4Dec(1);
		pCurWav->HVMap.Neighbor4Dec(1);
		while( pCurWav->pHigh != 0 ){
			pCurWav = pCurWav->pHigh;
			pCurWav->DMap.TreeDec();
			pCurWav->HVMap.TreeDec();
		}
	}
}

unsigned char * DirWavelet::CodeBand(unsigned char * pBuf)
{
	CRLECodec Codec(pBuf);

	DBand.RLECode(&Codec);
	HVBand.RLECode(&Codec);

	DirWavelet * pCurWav = this;
	while( pCurWav->pLow != 0 ){
		pCurWav = pCurWav->pLow;
		pCurWav->DBand.RLECode(&Codec);
		pCurWav->HVBand.RLECode(&Codec);
	}
	pCurWav->LBand.RLECode(&Codec);
	return Codec.EndCoding();
}

unsigned char * DirWavelet::DecodeBand(unsigned char * pBuf)
{
	CRLECodec Codec(pBuf);
	DBand.RLEDecode(&Codec);
	HVBand.RLEDecode(&Codec);

	DirWavelet * pCurWav = this;
	while( pCurWav->pLow != 0 ){
		pCurWav = pCurWav->pLow;
		pCurWav->DBand.RLEDecode(&Codec);
		pCurWav->HVBand.RLEDecode(&Codec);
	}
	pCurWav->LBand.RLEDecode(&Codec);
	return Codec.EndDecoding();
}

/**
 * Fills pHV1D with horizontal and then vertical oriented coeffs from HVBand
 * @param
 */
void DirWavelet::FillHV1D(void)
{
	float * p1D = pHV1D;
	float * pCurPos = HVBand.pBand;
	DirValue * pMap = pLow->HVMap.pMap;
	int MapDimX = pLow->HVMap.DimX;

	// horizontal direction
	for (int j = 0; j < HVBand.DimY; j++) {
		DirValue * pCurMap = pMap + (j >> 2) * MapDimX;
		for(int i = 0; i < HVBand.DimX ; i++){
			if (pCurMap[i >> 2].Selected == 0){
				p1D[0] = pCurPos[i];
				p1D++;
			}
		}
		j++;
		pCurPos += HVBand.DimXAlign;
		if (j >= HVBand.DimY)
			break;
		pCurMap = pMap + (j >> 2) * MapDimX;
		for(int i = HVBand.DimX - 1; i >= 0; i--){
			if (pCurMap[i >> 2].Selected == 0){
				p1D[0] = pCurPos[i];
				p1D++;
			}
		}
		pCurPos += HVBand.DimXAlign;
	}

	DimHVDir = p1D - pHV1D;
	pCurPos = HVBand.pBand;
	int mod[4] = {0,0,0,MapDimX};

	// vertical direction
	for(int i = 0; i < HVBand.DimX; i++){
		DirValue * pCurMap = pMap + (i >> 2);
		int j, k = 0, l = 0;
		for(j = i; j < HVBand.BandSize ; j += HVBand.DimXAlign, k++){
			if (pCurMap[l].Selected == 1){
				p1D[0] = pCurPos[j];
				p1D++;
			}
			l += mod[k & 3];
		}
		i++;
		if (i >= HVBand.DimX)
			break;
		pCurMap = pMap + (i >> 2);
		j -= HVBand.DimXAlign - 1;
		k--;
		for(; j >= 0 ; j -= HVBand.DimXAlign, k--){
			l -= mod[k & 3];
			if (pCurMap[l].Selected == 1){
				p1D[0] = pCurPos[j];
				p1D++;
			}
		}
	}
}

/**
 * Fills pD1D with D1 and then D2 oriented coeffs from DBand
 * @param
 */
void DirWavelet::FillD1D(void)
{
	float * p1D = pD1D;
	DirValue * pMap = DMap.pMap;
	int MapDimX = DMap.DimX;

	int lutMap[4][2] = {{0, 0}, {0, MapDimX}, {0, 0}, {1, MapDimX + 1}};
	int jPos[2] = {1, DBand.DimXAlign - 1};

	// direction D1 \ (scanning from top right to bottom left)
	float * pLastPos = DBand.pBand + DBand.BandSize + 1;
	float * pCurPos = DBand.pBand + DBand.DimX - 1;
	int j = 0;
	int i = DBand.DimX - 1;
// 	while(  ){
// 		float * pEndPos = ???;
// 		DirValue * pCurMap = ???;
// 		for( ; pCurPos != pEndPos ; i++){
// 			if (pCurMap[0].Completed == 0){
// 				p1D[0] = pCurPos[0];
// 				p1D++;
// 			}
// 			j += i&1;
// 			pCurPos += jPos[i&1];
// 			pCurMap += lutMap[i&3][j&1];
// 		}
// 	}
}

/**
 * Initialize the function pointers arrays
 * @param
 */
void DirWavelet::InitFuncPtr(void)
{
	LiftInOdd[0] = DirWavelet::LiftHOdd;
	LiftInOdd[1] = DirWavelet::LiftVOdd;

	LiftEdgeOdd[0] = DirWavelet::LiftHOdd;
	LiftEdgeOdd[1] = DirWavelet::LiftVOdd;

	LiftInEven[0] = DirWavelet::LiftHEven;
	LiftInEven[1] = DirWavelet::LiftVEven;

	LiftEdgeEven[0] = DirWavelet::LiftHEven;
	LiftEdgeEven[1] = DirWavelet::LiftVEven;

	LiftInDiagOdd[0] = DirWavelet::LiftDiag1Odd;
	LiftInDiagOdd[1] = DirWavelet::LiftDiag2Odd;

	LiftEdgeDiagOdd[0] = DirWavelet::LiftDiag1Odd;
	LiftEdgeDiagOdd[1] = DirWavelet::LiftDiag2Odd;

	LiftInDiagEven[0] = DirWavelet::LiftDiag1Even;
	LiftInDiagEven[1] = DirWavelet::LiftDiag2Even;

	LiftEdgeDiagEven[0] = DirWavelet::LiftDiag1Even;
	LiftEdgeDiagEven[1] = DirWavelet::LiftDiag2Even;
}

#define PXL_LIFT(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + (y) * Stride] \
		+ pBlock[(x) - 1 + (y) * Stride]) * Coef;

#define PXL_LIFT_L(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) + 1 + (y) * Stride] * 2 * Coef;

#define PXL_LIFT_R(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) - 1 + (y) * Stride] * 2 * Coef;

void DirWavelet::LiftHOdd(float * pBlock, int Stride, float Coef)
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

void DirWavelet::LiftHOdd(float * pBlock, int Stride, float Coef, int BitField)
{
	if (BitField & LEFT) {
		PXL_LIFT_L(0,1);
		PXL_LIFT_L(0,3);
	} else {
		PXL_LIFT(0,3);
		PXL_LIFT(0,1);
	}
	if (BitField & RIGHT) {
		PXL_LIFT_R(3,0);
		PXL_LIFT_R(3,2);
	} else {
		PXL_LIFT(3,0);
		PXL_LIFT(3,2);
	}
	PXL_LIFT(1,0);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(2,3);
}

void DirWavelet::LiftHEven(float * pBlock, int Stride, float Coef)
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

void DirWavelet::LiftHEven(float * pBlock, int Stride, float Coef, int BitField)
{
	if (BitField & LEFT) {
		PXL_LIFT_L(0,0);
		PXL_LIFT_L(0,2);
	} else {
		PXL_LIFT(0,0);
		PXL_LIFT(0,2);
	}
	if (BitField & RIGHT) {
		PXL_LIFT_R(3,1);
		PXL_LIFT_R(3,3);
	} else {
		PXL_LIFT(3,1);
		PXL_LIFT(3,3);
	}
	PXL_LIFT(2,0);
	PXL_LIFT(1,1);
	PXL_LIFT(2,2);
	PXL_LIFT(1,3);
}

#undef PXL_LIFT
#undef PXL_LIFT_L
#undef PXL_LIFT_R

#define PXL_LIFT(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + ((y) + 1) * Stride] \
		+ pBlock[(x) + ((y) - 1) * Stride]) * Coef;

#define PXL_LIFT_T(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) + ((y) + 1) * Stride] * 2 * Coef;

#define PXL_LIFT_B(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) + ((y) - 1) * Stride] * 2 * Coef;

void DirWavelet::LiftVOdd(float * pBlock, int Stride, float Coef)
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

void DirWavelet::LiftVOdd(float * pBlock, int Stride, float Coef, int BitField)
{
	if (BitField & TOP){
		PXL_LIFT_T(1,0);
		PXL_LIFT_T(3,0);
	} else {
		PXL_LIFT(1,0);
		PXL_LIFT(3,0);
	}
	if (BitField & BOTTOM){
		PXL_LIFT_B(0,3);
		PXL_LIFT_B(2,3);
	} else {
		PXL_LIFT(0,3);
		PXL_LIFT(2,3);
	}
	PXL_LIFT(0,1);
	PXL_LIFT(2,1);
	PXL_LIFT(1,2);
	PXL_LIFT(3,2);
}

void DirWavelet::LiftVEven(float * pBlock, int Stride, float Coef)
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

void DirWavelet::LiftVEven(float * pBlock, int Stride, float Coef, int BitField)
{
	if (BitField & TOP){
		PXL_LIFT_T(0,0);
		PXL_LIFT_T(2,0);
	} else {
		PXL_LIFT(0,0);
		PXL_LIFT(2,0);
	}
	if (BitField & BOTTOM){
		PXL_LIFT_B(1,3);
		PXL_LIFT_B(3,3);
	} else {
		PXL_LIFT(1,3);
		PXL_LIFT(3,3);
	}
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
}

#undef PXL_LIFT
#undef PXL_LIFT_T
#undef PXL_LIFT_B

#define PXL_LIFT(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) + 1 + ((y) + 1) * Stride] \
		+ pBlock[(x) - 1 + ((y) - 1) * Stride]) * Coef;

#define PXL_LIFT_TL(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) + 1 + ((y) + 1) * Stride] \
		* 2 * Coef;

#define PXL_LIFT_BR(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) - 1 + ((y) - 1) * Stride] \
		* 2 * Coef;

void DirWavelet::LiftDiag1Odd(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftDiag1Odd(float * pBlock, int Stride, float Coef,
							  int BitField)
{
	PXL_LIFT(1,1);
	if (BitField & RIGHT) {
		PXL_LIFT_BR(3,1);
	} else {
		PXL_LIFT(3,1);
	}
	if (BitField & BOTTOM) {
		PXL_LIFT_BR(1,3);
	} else {
		PXL_LIFT(1,3);
	}
	if (BitField & (BOTTOM | RIGHT)) {
		PXL_LIFT_BR(3,3);
	} else {
		PXL_LIFT(3,3);
	}
}

void DirWavelet::LiftDiag1Even(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
}

void DirWavelet::LiftDiag1Even(float * pBlock, int Stride, float Coef,
							   int BitField)
{
	if (BitField & (TOP | LEFT)) {
		PXL_LIFT_TL(0,0);
	} else {
		PXL_LIFT(0,0);
	}
	if (BitField & TOP) {
		PXL_LIFT_TL(2,0);
	} else {
		PXL_LIFT(2,0);
	}
	if (BitField & LEFT) {
		PXL_LIFT_TL(0,2);
	} else {
		PXL_LIFT(0,2);
	}
	PXL_LIFT(2,2);
}

#undef PXL_LIFT
#undef PXL_LIFT_TL
#undef PXL_LIFT_BR

#define PXL_LIFT(x,y) \
	pBlock[(x) + (y) * Stride] += (pBlock[(x) - 1 + ((y) + 1) * Stride] \
		+ pBlock[(x) + 1 + ((y) - 1) * Stride]) * Coef;

#define PXL_LIFT_TR(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) - 1 + ((y) + 1) * Stride] \
		* 2 * Coef;

#define PXL_LIFT_BL(x,y) \
	pBlock[(x) + (y) * Stride] += pBlock[(x) + 1 + ((y) - 1) * Stride] \
		* 2 * Coef;

void DirWavelet::LiftDiag2Odd(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(1,1);
	PXL_LIFT(3,1);
	PXL_LIFT(1,3);
	PXL_LIFT(3,3);
}

void DirWavelet::LiftDiag2Odd(float * pBlock, int Stride, float Coef,
							  int BitField)
{
	PXL_LIFT(1,1);
	if (BitField & RIGHT) {
		PXL_LIFT_TR(3,1);
	} else {
		PXL_LIFT(3,1);
	}
	if (BitField & BOTTOM) {
		PXL_LIFT_BL(1,3);
	} else {
		PXL_LIFT(1,3);
	}
	if (BitField & RIGHT) {
		if (! (BitField & BOTTOM)) {
			PXL_LIFT_TR(3,3);
		}
	} else if (BitField & BOTTOM) {
		PXL_LIFT_BL(3,3);
	} else {
		PXL_LIFT(3,3);
	}
}

void DirWavelet::LiftDiag2Even(float * pBlock, int Stride, float Coef)
{
	PXL_LIFT(0,0);
	PXL_LIFT(2,0);
	PXL_LIFT(0,2);
	PXL_LIFT(2,2);
}

void DirWavelet::LiftDiag2Even(float * pBlock, int Stride, float Coef,
							   int BitField)
{
	if (BitField & TOP) {
		if (! (BitField & LEFT)) {
			PXL_LIFT_TR(0,0);
		}
	} else if (BitField & LEFT) {
		PXL_LIFT_BL(0,0);
	} else {
		PXL_LIFT(0,0);
	}
	if (BitField & TOP) {
		PXL_LIFT_TR(2,0);
	} else {
		PXL_LIFT(2,0);
	}
	if (BitField & LEFT) {
		PXL_LIFT_BL(0,2);
	} else {
		PXL_LIFT(0,2);
	}
	PXL_LIFT(2,2);
}

#undef PXL_LIFT
#undef PXL_LIFT_TR
#undef PXL_LIFT_BL

void DirWavelet::LiftBand(float * pBlock, int Stride, int DimX, int DimY,
						  float Coef, DirValue * pDir,
						  void (**LiftEdge)(float*, int, float, int),
						  void (**Lift)(float*, int, float))
{
	(*LiftEdge[pDir->Selected])(pBlock, Stride, Coef, TOP | LEFT);
	int i = 4;
	pDir++;
	for( ; i < DimX - 4; i += 4, pDir++){
		(*LiftEdge[pDir->Selected])(pBlock + i, Stride, Coef, TOP);
	}
	(*LiftEdge[pDir->Selected])(pBlock + i, Stride, Coef, TOP | RIGHT);
	pBlock += Stride << 2;
	pDir++;
	int j = 4;
	for( j; j < DimY - 4; j += 4){
		(*LiftEdge[pDir->Selected])(pBlock, Stride, Coef, LEFT);
		int i = 4;
		pDir++;
		for( ; i < DimX - 4; i += 4, pDir++){
			(*Lift[pDir->Selected])(pBlock + i, Stride, Coef);
		}
		(*LiftEdge[pDir->Selected])(pBlock + i, Stride, Coef, RIGHT);
		pBlock += Stride << 2;
		pDir++;
	}
	(*LiftEdge[pDir->Selected])(pBlock, Stride, Coef, BOTTOM | LEFT);
	i = 4;
	pDir++;
	for( ; i < DimX - 4; i += 4, pDir++){
		(*LiftEdge[pDir->Selected])(pBlock + i, Stride, Coef, BOTTOM);
	}
	(*LiftEdge[pDir->Selected])(pBlock + i, Stride, Coef, BOTTOM | RIGHT);
}

void DirWavelet::LazyImage(float * pImage, unsigned int Stride)
{
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

void DirWavelet::LazyImageI(float * pImage, unsigned int Stride)
{
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

void DirWavelet::Transform53(float * pImage, int Stride)
{
	HVMap.GetImageDir(pImage, Stride);
	LiftBand(pImage, Stride, DimX, DimY, -1./2., HVMap.pMap, LiftEdgeOdd,
			 LiftInOdd);
	LiftBand(pImage, Stride, DimX, DimY, 1./4., HVMap.pMap, LiftEdgeEven,
			 LiftInEven);
	DMap.GetImageDirDiag(pImage, Stride);
	LiftBand(pImage, Stride, DimX, DimY, -1./2., DMap.pMap, LiftEdgeDiagOdd,
			 LiftInDiagOdd);
	LiftBand(pImage, Stride, DimX, DimY, 1./4., DMap.pMap, LiftEdgeDiagEven,
			 LiftInDiagEven);
	LazyImage(pImage, Stride);

 	if (pLow != 0)
		pLow->Transform53(pImage, Stride);
}

void DirWavelet::Transform53I(float * pImage, int Stride)
{
	if (pLow != 0)
		pLow->Transform53I(pImage, Stride);

	LazyImageI(pImage, Stride);
	LiftBand(pImage, Stride, DimX, DimY, -1./4., DMap.pMap, LiftEdgeDiagEven,
			 LiftInDiagEven);
	LiftBand(pImage, Stride, DimX, DimY, 1./2., DMap.pMap, LiftEdgeDiagOdd,
			 LiftInDiagOdd);
	LiftBand(pImage, Stride, DimX, DimY, -1./4., HVMap.pMap, LiftEdgeEven,
			 LiftInEven);
	LiftBand(pImage, Stride, DimX, DimY, 1./2., HVMap.pMap, LiftEdgeOdd,
			 LiftInOdd);

	if (pHigh == 0)
		Saturate(pImage, Stride);
}

void DirWavelet::SetWeight53(void)
{
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
		pLow->SetWeight53();
}

// #define ALPHA (-1.586134342)
// #define BETA (-0.05298011854)
// #define GAMMA (0.8829110762)
// #define DELTA (0.4435068522)
#define XI 1.149604398
// #define XI 1.25

#define ALPHA (-3./2.)
#define BETA (-1./16.)
#define GAMMA (4./5.)
#define DELTA (15./32.)
// #define XI (4./5.)

void DirWavelet::Transform97(float * pImage, int Stride)
{
	HVMap.GetImageDir(pImage, Stride);
	HVMap.SelectDir();
 	HVMap.OptimiseDir(4 * HVBand.Weight);

	LiftBand(pImage, Stride, DimX, DimY, ALPHA, HVMap.pMap, LiftEdgeOdd,
			 LiftInOdd);
	LiftBand(pImage, Stride, DimX, DimY, BETA, HVMap.pMap, LiftEdgeEven,
			 LiftInEven);
	LiftBand(pImage, Stride, DimX, DimY, GAMMA, HVMap.pMap, LiftEdgeOdd,
			 LiftInOdd);
	LiftBand(pImage, Stride, DimX, DimY, DELTA, HVMap.pMap, LiftEdgeEven,
			 LiftInEven);
	DMap.GetImageDirDiag(pImage, Stride);
	DMap.SelectDir();
 	DMap.OptimiseDir(4 * DBand.Weight);

	LiftBand(pImage, Stride, DimX, DimY, ALPHA, DMap.pMap, LiftEdgeDiagOdd,
			 LiftInDiagOdd);
	LiftBand(pImage, Stride, DimX, DimY, BETA, DMap.pMap, LiftEdgeDiagEven,
			 LiftInDiagEven);
	LiftBand(pImage, Stride, DimX, DimY, GAMMA, DMap.pMap, LiftEdgeDiagOdd,
			 LiftInDiagOdd);
	LiftBand(pImage, Stride, DimX, DimY, DELTA, DMap.pMap, LiftEdgeDiagEven,
			 LiftInDiagEven);
	LazyImage(pImage, Stride);

	if (pLow != 0)
		pLow->Transform97(pImage, Stride);
}

void DirWavelet::Transform97I(float * pImage, int Stride)
{
	if (pLow != 0)
		pLow->Transform97I(pImage, Stride);
	LazyImageI(pImage, Stride);
	LiftBand(pImage, Stride, DimX, DimY, -DELTA, DMap.pMap, LiftEdgeDiagEven,
			 LiftInDiagEven);
	LiftBand(pImage, Stride, DimX, DimY, -GAMMA, DMap.pMap, LiftEdgeDiagOdd,
			 LiftInDiagOdd);
	LiftBand(pImage, Stride, DimX, DimY, -BETA, DMap.pMap, LiftEdgeDiagEven,
			 LiftInDiagEven);
	LiftBand(pImage, Stride, DimX, DimY, -ALPHA, DMap.pMap, LiftEdgeDiagOdd,
			 LiftInDiagOdd);
	LiftBand(pImage, Stride, DimX, DimY, -DELTA, HVMap.pMap, LiftEdgeEven,
			 LiftInEven);
	LiftBand(pImage, Stride, DimX, DimY, -GAMMA, HVMap.pMap, LiftEdgeOdd,
			 LiftInOdd);
	LiftBand(pImage, Stride, DimX, DimY, -BETA, HVMap.pMap, LiftEdgeEven,
			 LiftInEven);
	LiftBand(pImage, Stride, DimX, DimY, -ALPHA, HVMap.pMap, LiftEdgeOdd,
			 LiftInOdd);

	if (pHigh == 0)
		Saturate(pImage, Stride);
}

void DirWavelet::Saturate(float * pImage, int stride)
{
	float * pCurPos = pImage;
	for( int j = 0; j < DimY; j++){
		for (int i = 0; i < DimX; i++){
			if (pCurPos[i] > 1.)
				pCurPos[i] = 1.;
			if (pCurPos[i] < 0.)
				pCurPos[i] = 0.;
		}
		pCurPos += stride;
	}
}

void DirWavelet::SetWeight97(void)
{
	if (pHigh != 0){
		DBand.Weight = pHigh->HVBand.Weight * XI;
		HVBand.Weight = DBand.Weight * XI;
		LBand.Weight = HVBand.Weight * XI * XI;
	}else{
		DBand.Weight = 1./XI;
		HVBand.Weight = 1.;
		LBand.Weight = XI * XI;
	}
	if (pLow != 0)
		pLow->SetWeight97();
}

unsigned int DirWavelet::Thres(float Thres)
{
	unsigned int Count = 0;
	Count += DBand.Thres(Thres);
	Count += HVBand.Thres(Thres);
	if (pLow != 0)
		Count += pLow->Thres(Thres);
	return Count;
}

unsigned int DirWavelet::TSUQ(float Quant, float Thres)
{
	unsigned int Count = 0;
	Count += DBand.TSUQ(Quant, Thres);
	Count += HVBand.TSUQ(Quant, Thres);
	if (pLow != 0)
		Count += pLow->TSUQ(Quant, Thres);
	else
		Count += LBand.TSUQ(Quant, Quant * .5);
	return Count;
}

void DirWavelet::TSUQi(float Quant, float RecLevel)
{
	DBand.TSUQi(Quant, RecLevel);
	HVBand.TSUQi(Quant, RecLevel);
	if (pLow != 0)
		pLow->TSUQi(Quant, RecLevel);
	else
		LBand.TSUQi(Quant, 0);
}

}
