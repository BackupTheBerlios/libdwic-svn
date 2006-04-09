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

#include <math.h>
#include <string.h>

namespace libdwic {

CMap::CMap(CMap * pHighMap):
pMap(0),
pDist(0),
pLow(0)
{
	pHigh = pHighMap;
	if (pHigh != 0)
		pHigh->pLow = this;
	pTree[0] = 0;
	Init();
}


CMap::~CMap()
{
	delete[] pMap;
	delete[] pDist;
	delete[] pTree[0];
}

void CMap::Init(int DimX, int DimY)
{
	this->ImageX = DimX;
	this->ImageY = DimY;
	this->DimX = (DimX + 3) >> 2;
	this->DimY = (DimY + 3) >> 2;
	MapSize = this->DimX * this->DimY;
	if (MapSize != 0){
		pMap = new char[MapSize];
		pDist = new short[MapSize];
		if (MapSize > 8192) {
			pTree[0] = new node[(MapSize - (MapSize >> (2 * TREE_DEPTH))) / 3];
			for( int i = 1; i < TREE_DEPTH; i++)
				pTree[i] = pTree[i-1] + (MapSize >> (2 * i));
		}
	}
}

void CMap::SetDir(int Dir)
{
	for( int i = 0; i < MapSize ; i++){
		pMap[i] = Dir;
	}
}

void CMap::SetDir(int Dir, int x, int y, int treeDepth)
{
	char * pCurMap = pMap;
	int size = 1 << treeDepth;
	pCurMap += (x << treeDepth) + (y << treeDepth) * DimX;

	for( int j = 0; j < size; j++){
		for( int i = 0; i < size; i++){
			pCurMap[i] = Dir;
		}
		pCurMap += DimX;
	}
}

void CMap::GetMap(unsigned char * pOut)
{
	memcpy(pOut, pMap, MapSize);
}

void CMap::GetDist(unsigned char * pOut)
{
	for( int i = 0; i < MapSize; i++){
		int Out = ((int)pDist[i]>>8) + 128;
		if (Out > 255)
			Out = 255;
		if (Out < 0)
			Out = 0;
		pOut[i] = (unsigned char) Out;
	}
}

void CMap::SetRange(CRangeCodec * RangeCodec)
{
	DirCodec.SetRange(RangeCodec);
}

void CMap::Order0Code(void)
{
	DirCodec.InitModel();
	for( int j = 0; j < DimY; j++){
		char * pCur = pMap + j * DimX;
		for( int i = 0; i < DimX; i++ ){
			DirCodec.Code(pCur[i], 0);
		}
	}
}

void CMap::Order0Dec(void)
{
	DirCodec.InitModel();
	for( int j = 0; j < DimY; j++){
		char * pCur = pMap + j * DimX;
		for( int i = 0; i < DimX; i++ ){
			pCur[i] = DirCodec.Decode(0);
		}
	}
}

void CMap::Neighbor4Code(void)
{
	char * pCur = pMap;
	unsigned int context;
	DirCodec.InitModel();

	DirCodec.Code(pCur[0], 2);
	for( int i = 1; i < DimX; i++ ){
		context = pCur[i - 1] * 2 + 1;
		DirCodec.Code(pCur[i], context);
	}

	for( int j = 1; j < DimY; j++){
		pCur += DimX;
		context = pCur[-DimX] + pCur[1 - DimX] + 1;
		DirCodec.Code(pCur[0], context);
		int i = 1;
		for( ; i < DimX - 1; i++ ){
			context = pCur[i - 1] + pCur[i - 1 - DimX] + pCur[i - DimX]
					+ pCur[i + 1 - DimX];
			DirCodec.Code(pCur[i], context);
		}
		context = pCur[i - 1] + pCur[i - DimX] + 1;
		DirCodec.Code(pCur[i], context);
	}
}

void CMap::Neighbor4Dec(void)
{
	char * pCur = pMap;
	unsigned int context;
	DirCodec.InitModel();

	pCur[0] = DirCodec.Decode(2);
	for( int i = 1; i < DimX; i++ ){
		context = pCur[i - 1] * 2 + 1;
		pCur[i] = DirCodec.Decode(context);
	}

	for( int j = 1; j < DimY; j++){
		pCur += DimX;
		context = pCur[-DimX] + pCur[1 - DimX] + 1;
		pCur[0] = DirCodec.Decode(context);
		int i = 1;
		for( ; i < DimX - 1; i++ ){
			context = pCur[i - 1] + pCur[i - 1 - DimX]
					+ pCur[i - DimX] + pCur[i + 1 - DimX];
			pCur[i] = DirCodec.Decode(context);
		}
		context = pCur[i - 1] + pCur[i - DimX] + 1;
		pCur[i] = DirCodec.Decode(context);
	}
}

void CMap::TreeCode(void)
{
	char * pCur = pMap;
	char * pCurLow = pLow->pMap;
	unsigned int context;
	DirCodec.InitModel();
	unsigned int const conv[2] = {pLow->DimX, 0};

	int k = 0;
	int i = 0;
	DirCodec.Code(pCur[i], 4 | pCurLow[k]);
	for( i = 1; i < DimX; i++ ){
		context = pCur[i - 1] * 2 + 1;
		context = (context << 1) | pCurLow[k];
		DirCodec.Code(pCur[i], context);
		k += i & 1;
	}

	for( int j = 1; j < DimY; j++){
		pCur += DimX;
		pCurLow += conv[j & 1];
		k = 0;
		i = 0;
		context = pCur[-DimX] + pCur[1 - DimX] + 1;
		context = (context << 1) | pCurLow[k];
		DirCodec.Code(pCur[i], context);
		for( i = 1; i < DimX - 1; i++ ){
			context = pCur[i - 1] + pCur[i - 1 - DimX]
					+ pCur[i - DimX] + pCur[i + 1 - DimX];
			context = (context << 1) | pCurLow[k];
			DirCodec.Code(pCur[i], context);
			k += i & 1;
		}
		context = pCur[i - 1] + pCur[i - DimX] + 1;
		context = (context << 1) | pCurLow[k];
		DirCodec.Code(pCur[i], context);
	}
}

void CMap::TreeDec(void)
{
	char * pCur = pMap;
	char * pCurLow = pLow->pMap;
	unsigned int context;
	DirCodec.InitModel();
	unsigned int const conv[2] = {pLow->DimX, 0};

	int k = 0;
	int i = 0;
	pCur[i] = DirCodec.Decode(4 | pCurLow[k]);
	for( i = 1; i < DimX; i++ ){
		context = pCur[i - 1] * 2 + 1;
		context = (context << 1) | pCurLow[k];
		pCur[i] = DirCodec.Decode(context);
		k += i & 1;
	}

	for( int j = 1; j < DimY; j++){
		pCur += DimX;
		pCurLow += conv[j & 1];
		k = 0;
		i = 0;
		context = pCur[-DimX] + pCur[1 - DimX] + 1;
		context = (context << 1) | pCurLow[k];
		pCur[i] = DirCodec.Decode(context);
		for( i = 1; i < DimX - 1; i++ ){
			context = pCur[i - 1] + pCur[i - 1 - DimX]
					+ pCur[i - DimX] + pCur[i + 1 - DimX];
			context = (context << 1) | pCurLow[k];
			pCur[i] = DirCodec.Decode(context);
			k += i & 1;
		}
		context = pCur[i - 1] + pCur[i - DimX] + 1;
		context = (context << 1) | pCurLow[k];
		pCur[i] = DirCodec.Decode(context);
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

void CMap::OptimiseDir(float const lambda)
{
	char * pCur = pMap;
	short * pCurDist = pDist;
	unsigned int context;
	int const rate[] = { -3246, -1252, 0, 1252, 3246 };

	for( int i = 1; i < DimX; i++ ){
		context = pCur[i - 1] * 2 + 1;
		pCur[i] = 0;
		if ((rate[context] + lambda * pCurDist[i]) > 0)
			pCur[i] = 1;
	}

	for( int j = 1; j < DimY; j++){
		pCur += DimX;
		pCurDist += DimX;
		context = pCur[-DimX] + pCur[1 - DimX] + 1;
		pCur[0] = 0;
		if ((rate[context] + lambda * pCurDist[0]) > 0)
			pCur[0] = 1;
		int i = 1;
		for( ; i < DimX - 1; i++ ){
			context = pCur[i - 1] + pCur[i + 1 - DimX]
					+ pCur[i - 1 - DimX] + pCur[i - DimX];
			pCur[i] = 0;
			if ((rate[context] + lambda * pCurDist[i]) > 0)
				pCur[i] = 1;
		}
		context = pCur[i - 1] + pCur[i - DimX] + 1;
		pCur[i] = 0;
		if ((rate[context] + lambda * pCurDist[i]) > 0)
			pCur[i] = 1;
	}
}

void CMap::SelectDir(void)
{

	for( int i = 0; i < MapSize; i++){
		pMap[i] = 0;
		if (pDist[i] > 0)
			pMap[i] = 1;
	}

	return;

// 	DirValue * pCurMap = pMap;
// 	unsigned int * pA = new unsigned int [DimX];
// 	unsigned int * pB = new unsigned int [DimX];
//
// 	// Top -> Bottom
// 	for( int i = 0; i < DimX; i++) {
// 		pA[i] = pCurMap[i].H_D1 << 1;
// 		pB[i] = pCurMap[i].V_D2 << 1;
// 	}
// 	for( int j = 0; j < DimY; j++ ){
// 		for( int i = 0; i < DimX ; i++){
// 			pA[i] = pCurMap[i].H_D1 + (pA[i] >> 1);
// 			pB[i] = pCurMap[i].V_D2 + (pB[i] >> 1);
// 			pCurMap[i].Selected = pA[i];
// 			pCurMap[i].Old = pB[i];
// 		}
// 		pCurMap += DimX;
// 	}
// 	pCurMap -= DimX;
// 	// Bottom -> Top
// 	for( int i = 0; i < DimX; i++) {
// 		pA[i] = pCurMap[i].H_D1 << 1;
// 		pB[i] = pCurMap[i].V_D2 << 1;
// 	}
// 	for( int j = 0; j < DimY; j++ ){
// 		for( int i = 0; i < DimX ; i++){
// 			pA[i] = pCurMap[i].H_D1 + (pA[i] >> 1);
// 			pB[i] = pCurMap[i].V_D2 + (pB[i] >> 1);
// 			pCurMap[i].Selected += pA[i];
// 			pCurMap[i].Old += pB[i];
// 		}
// 		pCurMap -= DimX;
// 	}
//
// 	pCurMap = pMap;
//
// 	for( int j = 0; j < DimY; j++ ){
// 		unsigned int a = pCurMap[0].Selected << 1;
// 		unsigned int b = pCurMap[0].Old << 1;
// 		for( int i = 0; i < DimX; i++){
// 			a = pCurMap[i].Selected + (a >> 1);
// 			b = pCurMap[i].Old + (b >> 1);
// 			pA[i] = a;
// 			pB[i] = b;
// 		}
// 		a = pCurMap[DimX - 1].Selected << 1;
// 		b = pCurMap[DimX - 1].Old << 1;
// 		for( int i = DimX - 1; i >= 0 ; i--){
// 			a = pCurMap[i].Selected + (a >> 1);
// 			b = pCurMap[i].Old + (b >> 1);
// 			pCurMap[i].Selected = 0;
// 			if (b < a)
// 				pCurMap[i].Selected = 1;
// 		}
// 		pCurMap += DimX;
// 	}
//
// 	delete[] pA;
// 	delete[] pB;
}

void CMap::BuidTree(float const lambda)
{
	int width = DimX >> 1;
	int height = DimY >> 1;
	short * pCurDist1 = pDist;
	short * pCurDist2 = pDist + DimX;
	node * pCurTree = pTree[0];

	for (int j = 0; j < height; j++) {
		for (int i = 0, k = 0; i < width; i++, k += 2) {
			pCurTree[i].refDist = pCurDist1[k] + pCurDist1[k + 1]
					+ pCurDist2[k] + pCurDist2[k + 1];
			int Dist = 0;
			if (pCurDist1[k] < 0)
				Dist += pCurDist1[k];
			if (pCurDist1[k + 1] < 0)
				Dist += pCurDist1[k + 1];
			if (pCurDist2[k] < 0)
				Dist += pCurDist2[k];
			if (pCurDist2[k + 1] < 0)
				Dist += pCurDist2[k + 1];
			pCurTree[i].dist = Dist;
			if (pCurTree[i].refDist < 0)
				Dist -= pCurTree[i].refDist;
			pCurTree[i].rate = 0;
			if ((3 + lambda * Dist) < 0)
				pCurTree[i].rate = 3;
		}
		pCurDist1 += DimX * 2;
		pCurDist2 += DimX * 2;
		pCurTree += width;
	}

	for( int l = 1; l < TREE_DEPTH; l++){
		node * pLowTree1 = pTree[l-1];
		node * pLowTree2 = pLowTree1 + width;
		pCurTree = pTree[l];
		int stride2 = width * 2;
		height >>= 1;
		width >>= 1;

		for( int j = 0; j < height; j++){
			for( int i = 0, k = 0; i < width; i++, k += 2){
				pCurTree[i].refDist = pLowTree1[k].refDist
						+ pLowTree1[k + 1].refDist + pLowTree2[k].refDist
						+ pLowTree2[k + 1].refDist;
				int rate = 7 + pLowTree1[k].rate + pLowTree1[k + 1].rate
						+ pLowTree2[k].rate + pLowTree2[k + 1].rate;
				int Dist = 0;
				if (pLowTree1[k].rate > 0)
					Dist += pLowTree1[k].dist;
				else
					Dist += MIN(0, pLowTree1[k].refDist);
				if (pLowTree1[k + 1].rate > 0)
					Dist += pLowTree1[k + 1].dist;
				else
					Dist += MIN(0, pLowTree1[k + 1].refDist);
				if (pLowTree2[k].rate > 0)
					Dist += pLowTree2[k].dist;
				else
					Dist += MIN(0, pLowTree2[k].refDist);
				if (pLowTree2[k + 1].rate > 0)
					Dist += pLowTree2[k + 1].dist;
				else
					Dist += MIN(0, pLowTree2[k + 1].refDist);
				pCurTree[i].dist = Dist;
				if (pCurTree[i].refDist < 0)
					Dist -= pCurTree[i].refDist;
				pCurTree[i].rate = 0;
				if ((rate + lambda * Dist) < 0)
					pCurTree[i].rate = rate;
			}
			pLowTree1 += stride2;
			pLowTree2 += stride2;
			pCurTree += width;
		}
	}
}

void CMap::ApplyTree(void)
{
	int width = DimX >> TREE_DEPTH;
	int height = DimY >> TREE_DEPTH;
	node * pCurTree = pTree[TREE_DEPTH - 1];

	for( int j = 0; j < height; j++){
		for( int i = 0; i < width; i++){
			if (pCurTree[i].rate == 0)
				SetDir(pCurTree[i].refDist < 0 ? 0 : 1, i, j, TREE_DEPTH);
			else
				ApplyTree(i << 1, j << 1, TREE_DEPTH - 1);
		}
		pCurTree += width;
	}
}

void CMap::ApplyTree(int x, int y, int treeDepth)
{
	if (treeDepth == 0){
		pMap[x + DimX * y] = 0;
		if (pDist[x + DimX * y] > 0)
			pMap[x + DimX * y] = 1;
		return;
	}
	node * pCurTree = pTree[treeDepth - 1];
	int width = DimX >> treeDepth;
	pCurTree += y * width;
	if (pCurTree[x].rate == 0)
		SetDir(pCurTree[x].refDist < 0 ? 0 : 1, x, y, treeDepth);
	else
		ApplyTree(x << 1, y << 1, treeDepth - 1);
	x++;
	if (pCurTree[x].rate == 0)
		SetDir(pCurTree[x].refDist < 0 ? 0 : 1, x, y, treeDepth);
	else
		ApplyTree(x << 1, y << 1, treeDepth - 1);
	pCurTree += width;
	y++;
	if (pCurTree[x].rate == 0)
		SetDir(pCurTree[x].refDist < 0 ? 0 : 1, x, y, treeDepth);
	else
		ApplyTree(x << 1, y << 1, treeDepth - 1);
	x--;
	if (pCurTree[x].rate == 0)
		SetDir(pCurTree[x].refDist < 0 ? 0 : 1, x, y, treeDepth);
	else
		ApplyTree(x << 1, y << 1, treeDepth - 1);
}

void CMap::GetImageDir(float * pBlock, int Stride)
{
	short * pDir = this->pDist;
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
	short * pDir = this->pDist;
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

void CMap::GetDirBlock(float * pBlock, int Stride, short * Result)
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

	Sum = -Sum;
	PXL_VAL_V(1,0);
	PXL_VAL_V(3,0);
	PXL_VAL_V(0,1);
	PXL_VAL_V(2,1);
	PXL_VAL_V(1,2);
	PXL_VAL_V(3,2);
	PXL_VAL_V(0,3);
	PXL_VAL_V(2,3);

	*Result = (short) (-Sum * 65536);
}

void CMap::GetDirBlock(float * pBlock, int Stride, short * Result
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

	Sum = -Sum;
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

	*Result = (short) (-Sum * 65536);
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

void CMap::GetDirBlockDiag(float * pBlock, int Stride, short * Result)
{
	float Sum = 0;
	float Sqr;

	PXL_D1(1,1);
	PXL_D1(3,1);
	PXL_D1(1,3);
	PXL_D1(3,3);

	Sum = -Sum;
	PXL_D2(1,1);
	PXL_D2(3,1);
	PXL_D2(1,3);
	PXL_D2(3,3);

	*Result = (short) (-Sum * 65536);
}

void CMap::GetDirBlockDiag(float * pBlock, int Stride, short * Result,
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

	Sum = -Sum;
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

	*Result = (short) (-Sum * 65536);
}

}
