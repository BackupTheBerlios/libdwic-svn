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

#include "global.h"
#include "map.h"

namespace libdwic {

CMap::CMap(CMap * pHighMap):
pMap(0),
pLow(0)
{
	pHigh = pHighMap;
	if (pHigh != 0){
		pHigh->pLow = this;
		AllOnes = 4 * pHigh->AllOnes + 1;
	} else {
		AllOnes = 1;
	}
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
	MapSize = this->DimX * this->DimY;
	if (MapSize != 0){
		pMap = new DirValue[MapSize];
	}
}

void CMap::SetSelected(int Sel)
{
	for( int i = 0; i < MapSize ; i++){
		pMap[i].Selected = Sel;
	}
}

void CMap::SetRange(CRangeCodec * RangeCodec)
{
	DirCodec.SetRange(RangeCodec);
	TreeCodec.SetRange(RangeCodec);
}

void CMap::Order0Code(void)
{
	DirCodec.InitModel();
	for( int j = 0; j < DimY; j++){
		DirValue * pCur = pMap + j * DimX;
		for( int i = 0; i < DimX; i++ ){
			DirCodec.Code(pCur[i].Selected, 0);
		}
	}
}

void CMap::Order0Dec(void)
{
	DirCodec.InitModel();
	for( int j = 0; j < DimY; j++){
		DirValue * pCur = pMap + j * DimX;
		for( int i = 0; i < DimX; i++ ){
			pCur[i].Selected = DirCodec.Decode(0);
		}
	}
}

void CMap::Neighbor4Code(int CodeTree)
{
	DirValue * pCur = pMap;
	unsigned int context;
	DirCodec.InitModel();
	if (CodeTree)
		TreeCodec.InitModel();

	DirCodec.Code(pCur[0].Selected, 2);
	if (CodeTree)
		TreeCodec.Code(pCur[0].Old == 0 || pCur[0].Old == AllOnes, 2);
	for( int i = 1; i < DimX; i++ ){
		context = pCur[i - 1].Selected * 2 + 1;
		DirCodec.Code(pCur[i].Selected, context);
		if (CodeTree)
			TreeCodec.Code(pCur[i].Old == 0 || pCur[i].Old == AllOnes, context);
	}

	for( int j = 1; j < DimY; j++){
		pCur += DimX;
		context = pCur[-DimX].Selected + pCur[1 - DimX].Selected + 1;
		DirCodec.Code(pCur[0].Selected, context);
		if (CodeTree)
			TreeCodec.Code(pCur[0].Old == 0 || pCur[0].Old == AllOnes,
						   context);
		int i = 1;
		for( ; i < DimX - 1; i++ ){
			context = pCur[i - 1].Selected + pCur[i - 1 - DimX].Selected
					+ pCur[i - DimX].Selected + pCur[i + 1 - DimX].Selected;
			DirCodec.Code(pCur[i].Selected, context);
			if (CodeTree)
				TreeCodec.Code(pCur[i].Old == 0 || pCur[i].Old == AllOnes,
						   context);
		}
		context = pCur[i - 1].Selected + pCur[i - DimX].Selected + 1;
		DirCodec.Code(pCur[i].Selected, context);
		if (CodeTree)
			TreeCodec.Code(pCur[i].Old == 0 || pCur[i].Old == AllOnes,
						   context);
	}
}

void CMap::Neighbor4Dec(int DecodeTree)
{
	DirValue * pCur = pMap;
	unsigned int context;
	DirCodec.InitModel();
	if (DecodeTree)
		TreeCodec.InitModel();

	pCur[0].Selected = DirCodec.Decode(2);
	if (DecodeTree)
		pCur[0].Old = TreeCodec.Decode(2);
	for( int i = 1; i < DimX; i++ ){
		context = pCur[i - 1].Selected * 2 + 1;
		pCur[i].Selected = DirCodec.Decode(context);
		if (DecodeTree)
			pCur[i].Old = TreeCodec.Decode(context);
	}

	for( int j = 1; j < DimY; j++){
		pCur += DimX;
		context = pCur[-DimX].Selected + pCur[1 - DimX].Selected + 1;
		pCur[0].Selected = DirCodec.Decode(context);
		if (DecodeTree)
			pCur[0].Old = TreeCodec.Decode(context);
		int i = 1;
		for( ; i < DimX - 1; i++ ){
			context = pCur[i - 1].Selected + pCur[i - 1 - DimX].Selected
					+ pCur[i - DimX].Selected + pCur[i + 1 - DimX].Selected;
			pCur[i].Selected = DirCodec.Decode(context);
			if (DecodeTree)
				pCur[i].Old = TreeCodec.Decode(context);
		}
		context = pCur[i - 1].Selected + pCur[i - DimX].Selected + 1;
		pCur[i].Selected = DirCodec.Decode(context);
		if (DecodeTree)
			pCur[i].Old = TreeCodec.Decode(context);
	}
}

void CMap::TreeCode(void)
{
	DirValue * pCur = pMap;
	DirValue * pCurLow = pLow->pMap;
	unsigned int context;
	unsigned int LowAllOnes = pLow->AllOnes;
	DirCodec.InitModel();
	TreeCodec.InitModel();
	unsigned int const conv[2] = {pLow->DimX, 0};

	int k = 0;
	int i = 0;
	if (pCurLow[k].Old != LowAllOnes && pCurLow[k].Old != 0) {
		DirCodec.Code(pCur[i].Selected, 2);
		if (pHigh != 0)
			TreeCodec.Code(pCur[i].Old == 0 || pCur[i].Old == AllOnes, 2);
	}
	for( i = 1; i < DimX; i++ ){
		if (pCurLow[k].Old != LowAllOnes && pCurLow[k].Old != 0) {
			context = pCur[i - 1].Selected * 2 + 1;
			DirCodec.Code(pCur[i].Selected, context);
			if (pHigh != 0)
				TreeCodec.Code(pCur[i].Old == 0 || pCur[i].Old == AllOnes,
							   context);
		}
		k += i & 1;
	}

	for( int j = 1; j < DimY; j++){
		pCur += DimX;
		pCurLow += conv[j & 1];
		k = 0;
		i = 0;
		if (pCurLow[k].Old != LowAllOnes && pCurLow[k].Old != 0) {
			context = pCur[-DimX].Selected + pCur[1 - DimX].Selected + 1;
			DirCodec.Code(pCur[i].Selected, context);
			if (pHigh != 0)
				TreeCodec.Code(pCur[i].Old == 0 || pCur[i].Old == AllOnes,
							   context);
		}
		for( i = 1; i < DimX - 1; i++ ){
			if (pCurLow[k].Old != LowAllOnes && pCurLow[k].Old != 0) {
				context = pCur[i - 1].Selected + pCur[i - 1 - DimX].Selected
						+ pCur[i - DimX].Selected + pCur[i + 1 - DimX].Selected;
				DirCodec.Code(pCur[i].Selected, context);
				if (pHigh != 0)
					TreeCodec.Code(pCur[i].Old == 0 || pCur[i].Old == AllOnes,
								   context);
			}
			k += i & 1;
		}
		if (pCurLow[k].Old != LowAllOnes && pCurLow[k].Old != 0) {
			context = pCur[i - 1].Selected + pCur[i - DimX].Selected + 1;
			DirCodec.Code(pCur[i].Selected, context);
			if (pHigh != 0)
				TreeCodec.Code(pCur[i].Old == 0 || pCur[i].Old == AllOnes,
							   context);
		}
	}
}

void CMap::TreeDec(void)
{
	DirValue * pCur = pMap;
	DirValue * pCurLow = pLow->pMap;
	unsigned int context;
	unsigned int LowAllOnes = pLow->AllOnes;
	DirCodec.InitModel();
	TreeCodec.InitModel();
	unsigned int const conv[2] = {pLow->DimX, 0};

	int k = 0;
	int i = 0;

	pCur[i].Selected = pCurLow[k].Selected;
	pCur[i].Old = 1;
	if (pCurLow[k].Old == 0) {
		pCur[i].Selected = DirCodec.Decode(2);
		if (pHigh != 0)
			pCur[i].Old = TreeCodec.Decode(2);
	}
	for( i = 1; i < DimX; i++ ){
		pCur[i].Selected = pCurLow[k].Selected;
		pCur[i].Old = 1;
		if (pCurLow[k].Old == 0) {
			context = pCur[i - 1].Selected * 2 + 1;
			pCur[i].Selected = DirCodec.Decode(context);
			if (pHigh != 0)
				pCur[i].Old = TreeCodec.Decode(context);
		}
		k += i & 1;
	}

	for( int j = 1; j < DimY; j++){
		pCur += DimX;
		pCurLow += conv[j & 1];
		k = 0;
		i = 0;
		pCur[i].Selected = pCurLow[k].Selected;
		pCur[i].Old = 1;
		if (pCurLow[k].Old == 0) {
			context = pCur[-DimX].Selected + pCur[1 - DimX].Selected + 1;
			pCur[i].Selected = DirCodec.Decode(context);
			if (pHigh != 0)
				pCur[i].Old = TreeCodec.Decode(context);
		}
		for( i = 1; i < DimX - 1; i++ ){
			pCur[i].Selected = pCurLow[k].Selected;
			pCur[i].Old = 1;
			if (pCurLow[k].Old == 0) {
				context = pCur[i - 1].Selected + pCur[i - 1 - DimX].Selected
						+ pCur[i - DimX].Selected + pCur[i + 1 - DimX].Selected;
				pCur[i].Selected = DirCodec.Decode(context);
				if (pHigh != 0)
					pCur[i].Old = TreeCodec.Decode(context);
			}
			k += i & 1;
		}
		pCur[i].Selected = pCurLow[k].Selected;
		pCur[i].Old = 1;
		if (pCurLow[k].Old == 0) {
			context = pCur[i - 1].Selected + pCur[i - DimX].Selected + 1;
			pCur[i].Selected = DirCodec.Decode(context);
			if (pHigh != 0)
				pCur[i].Old = TreeCodec.Decode(context);
		}
	}
}

void CMap::TreeSum(void)
{
	DirValue * pCur = pMap;
	unsigned int HDimX2 = pHigh->DimX * 2;
	DirValue * pCurH1 = pHigh->pMap;
	DirValue * pCurH2 = pHigh->pMap + pHigh->DimX;
	unsigned int SumOld = 0;

	if (pHigh->pHigh != 0)
		SumOld = 1;

	for( int j = 0; j < DimY; j++){
		for ( int i = 0; i < DimX; i++){
			pCur[i].Old = pCur[i].Selected;
			if (SumOld == 1)
				pCur[i].Old += pCurH1[2 * i].Old + pCurH1[2 * i + 1].Old
						+ pCurH2[2 * i].Old + pCurH2[2 * i + 1].Old;
			else
				pCur[i].Old +=
						pCurH1[2 * i].Selected + pCurH1[2 * i + 1].Selected
						+ pCurH2[2 * i].Selected + pCurH2[2 * i + 1].Selected;
		}

		pCur += DimX;
		pCurH1 += HDimX2;
		pCurH2 += HDimX2;
	}
}

// rate^2 * cst
// static const unsigned int rate[5][2] =
// {
// 	{24, 11300},
// 	{271, 3089},
// 	{1024, 1024},
// 	{3089, 271},
// 	{11300, 24}
// };

static const unsigned int rate[5][2] =
{
	{156, 3402},
	{527, 1779},
	{1024, 1024},
	{1779, 527},
	{3402, 156}
};

void CMap::OptimiseDir(float const lambda)
{
	DirValue * pCur = pMap;
	unsigned int context;

	for( int i = 1; i < DimX; i++ ){
		context = pCur[i - 1].Selected * 2 + 1;
		float min = rate[context][0] + lambda * pCur[i].H_D1;
		pCur[i].Selected = 0;
		if (min > (rate[context][1] + lambda * pCur[i].V_D2))
			pCur[i].Selected = 1;
	}

	for( int j = 1; j < DimY; j++){
		pCur += DimX;
		context = pCur[-DimX].Selected + pCur[1 - DimX].Selected + 1;
		float min = rate[context][0] + lambda * pCur[0].H_D1;
		pCur[0].Selected = 0;
		if (min > (rate[context][1] + lambda * pCur[0].V_D2))
			pCur[0].Selected = 1;
		int i = 1;
		for( ; i < DimX - 1; i++ ){
			context = pCur[i - 1].Selected + pCur[i + 1 - DimX].Selected
					+ pCur[i - 1 - DimX].Selected + pCur[i - DimX].Selected;
			min = rate[context][0] + lambda * pCur[i].H_D1;
			pCur[i].Selected = 0;
			if (min > (rate[context][1] + lambda * pCur[i].V_D2))
				pCur[i].Selected = 1;
		}
		context = pCur[i - 1].Selected + pCur[i - DimX].Selected + 1;
		min = rate[context][0] + lambda * pCur[i].H_D1;
		pCur[i].Selected = 0;
		if (min > (rate[context][1] + lambda * pCur[i].V_D2))
			pCur[i].Selected = 1;
	}
}

void CMap::SelectDir(void)
{
	DirValue * pCurMap = pMap;
	unsigned int * pA = new unsigned int [DimX];
	unsigned int * pB = new unsigned int [DimX];

	// Top -> Bottom
	for( int i = 0; i < DimX; i++) {
		pA[i] = pCurMap[i].H_D1 << 1;
		pB[i] = pCurMap[i].V_D2 << 1;
	}
	for( int j = 0; j < DimY; j++ ){
		for( int i = 0; i < DimX ; i++){
			pA[i] = pCurMap[i].H_D1 + (pA[i] >> 1);
			pB[i] = pCurMap[i].V_D2 + (pB[i] >> 1);
			pCurMap[i].Selected = pA[i];
			pCurMap[i].Old = pB[i];
		}
		pCurMap += DimX;
	}
	pCurMap -= DimX;
	// Bottom -> Top
	for( int i = 0; i < DimX; i++) {
		pA[i] = pCurMap[i].H_D1 << 1;
		pB[i] = pCurMap[i].V_D2 << 1;
	}
	for( int j = 0; j < DimY; j++ ){
		for( int i = 0; i < DimX ; i++){
			pA[i] = pCurMap[i].H_D1 + (pA[i] >> 1);
			pB[i] = pCurMap[i].V_D2 + (pB[i] >> 1);
			pCurMap[i].Selected += pA[i];
			pCurMap[i].Old += pB[i];
		}
		pCurMap -= DimX;
	}

	pCurMap = pMap;

	for( int j = 0; j < DimY; j++ ){
		unsigned int a = pCurMap[0].Selected << 1;
		unsigned int b = pCurMap[0].Old << 1;
		for( int i = 0; i < DimX; i++){
			a = pCurMap[i].Selected + (a >> 1);
			b = pCurMap[i].Old + (b >> 1);
			pA[i] = a;
			pB[i] = b;
		}
		a = pCurMap[DimX - 1].Selected << 1;
		b = pCurMap[DimX - 1].Old << 1;
		for( int i = DimX - 1; i >= 0 ; i--){
			a = pCurMap[i].Selected + (a >> 1);
			b = pCurMap[i].Old + (b >> 1);
			pCurMap[i].Selected = 0;
			if (b < a)
				pCurMap[i].Selected = 1;
		}
		pCurMap += DimX;
	}

	delete[] pA;
	delete[] pB;
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

void CMap::GetImageDirDiag(float * pBlock, int Stride)
{
	DirValue * pDir = this->pMap;
	GetDirBlockDiag(pBlock, Stride, pDir, TOP | LEFT);
	int i = 4;
	pDir++;
	for( ; i < ImageX - 4; i += 4, pDir++){
		GetDirBlockDiag(pBlock + i, Stride, pDir, TOP);
	}
	GetDirBlockDiag(pBlock + i, Stride, pDir, TOP | RIGHT);
	pBlock += Stride << 2;
	pDir++;
	int j = 4;
	for( j; j < ImageY - 4; j += 4){
		GetDirBlockDiag(pBlock, Stride, pDir, LEFT);
		int i = 4;
		pDir++;
		for( ; i < ImageX - 4; i += 4, pDir++){
			GetDirBlockDiag(pBlock + i, Stride, pDir);
		}
		GetDirBlockDiag(pBlock + i, Stride, pDir, RIGHT);
		pBlock += Stride << 2;
		pDir++;
	}
	GetDirBlockDiag(pBlock, Stride, pDir, BOTTOM | LEFT);
	i = 4;
	pDir++;
	for( ; i < ImageX - 4; i += 4, pDir++){
		GetDirBlockDiag(pBlock + i, Stride, pDir, BOTTOM);
	}
	GetDirBlockDiag(pBlock + i, Stride, pDir, BOTTOM | RIGHT);
}

#define PXL_VAL_H(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + 1 + (y) * Stride] + \
		pBlock[(x) - 1 + (y) * Stride]) * .5); \
	Sum += Sqr * Sqr;

#define PXL_VAL_H_L(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - pBlock[(x) + 1 + (y) * Stride]; \
	Sum += Sqr * Sqr;

#define PXL_VAL_H_R(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - pBlock[(x) - 1 + (y) * Stride]; \
	Sum += Sqr * Sqr;

#define PXL_VAL_V(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + ((y) + 1) * Stride] + \
		pBlock[(x) + ((y) - 1) * Stride]) * .5); \
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

	PXL_VAL_H(1,0);
	PXL_VAL_H(3,0);
	PXL_VAL_H(0,1);
	PXL_VAL_H(2,1);
	PXL_VAL_H(1,2);
	PXL_VAL_H(3,2);
	PXL_VAL_H(0,3);
	PXL_VAL_H(2,3);

	Result->H_D1 = (unsigned short) (Sum * 65536);
	if (Sum > 1)
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

	Result->V_D2 = (unsigned short) (Sum * 65536);
	if (Sum > 1)
		Result->V_D2 = 0xFFFF;
}

void CMap::GetDirBlock(float * pBlock, int Stride, DirValue * Result
		, int BitField)
{
	float Sum = 0;
	float Sqr;

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

	Result->H_D1 = (unsigned short) (Sum * 65536);
	if (Sum > 1)
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

	Result->V_D2 = (unsigned short) (Sum * 65536);
	if (Sum > 1)
		Result->V_D2 = 0xFFFF;
}

#define PXL_D1(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) + 1 + ((y) + 1) * Stride] \
 	+ pBlock[(x) - 1 + ((y) - 1) * Stride]) * .5); \
	Sum += Sqr * Sqr;

#define PXL_D1_TL(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - pBlock[(x) + 1 + ((y) + 1) * Stride]; \
	Sum += Sqr * Sqr;

#define PXL_D1_BR(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - pBlock[(x) - 1 + ((y) - 1) * Stride]; \
	Sum += Sqr * Sqr;

#define PXL_D2(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - ((pBlock[(x) - 1 + ((y) + 1) * Stride] \
 	+ pBlock[(x) + 1 + ((y) - 1) * Stride]) * .5); \
	Sum += Sqr * Sqr;

#define PXL_D2_BL(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - pBlock[(x) - 1 + ((y) + 1) * Stride]; \
	Sum += Sqr * Sqr;

#define PXL_D2_TR(x,y) \
	Sqr = pBlock[(x) + (y) * Stride] - pBlock[(x) + 1 + ((y) - 1) * Stride]; \
	Sum += Sqr * Sqr;

void CMap::GetDirBlockDiag(float * pBlock, int Stride, DirValue * Result)
{
	float Sum = 0;
	float Sqr;

	PXL_D1(1,1);
	PXL_D1(3,1);
	PXL_D1(1,3);
	PXL_D1(3,3);

	Result->H_D1 = (unsigned short) (Sum * 65536);
	if (Sum > 1)
		Result->H_D1 = 0xFFFF;

	Sum = 0;
	PXL_D2(1,1);
	PXL_D2(3,1);
	PXL_D2(1,3);
	PXL_D2(3,3);

	Result->V_D2 = (unsigned short) (Sum * 65536);
	if (Sum > 1)
		Result->V_D2 = 0xFFFF;
}

void CMap::GetDirBlockDiag(float * pBlock, int Stride, DirValue * Result,
						   int BitField)
{
	float Sum = 0;
	float Sqr;

	PXL_D1(1,1);
	if (BitField & RIGHT) {
		PXL_D1_BR(3,1);
	} else {
		PXL_D1(3,1);
	}
	if (BitField & BOTTOM) {
		PXL_D1_BR(1,3);
	} else {
		PXL_D1(1,3);
	}
	if (BitField & (BOTTOM | RIGHT)) {
		PXL_D1_BR(3,3);
	} else {
		PXL_D1(3,3);
	}

	Result->H_D1 = (unsigned short) (Sum * 65536);
	if (Sum > 1)
		Result->H_D1 = 0xFFFF;

	Sum = 0;
	PXL_D2(1,1);
	if (BitField & RIGHT) {
		PXL_D2_TR(3,1);
	} else {
		PXL_D2(3,1);
	}
	if (BitField & BOTTOM) {
		PXL_D2_BL(1,3);
	} else {
		PXL_D2(1,3);
	}
	if (BitField & RIGHT) {
		if (! (BitField & BOTTOM)) {
			PXL_D2_TR(3,3);
		}
	} else if (BitField & BOTTOM) {
		PXL_D2_BL(3,3);
	} else {
		PXL_D2(3,3);
	}

	Result->V_D2 = (unsigned short) (Sum * 65536);
	if (Sum > 1)
		Result->V_D2 = 0xFFFF;
}

}
