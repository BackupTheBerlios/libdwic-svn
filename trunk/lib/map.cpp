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
#include <iostream>

using namespace std;

namespace libdwic {

CMap::CMap(CMap * pHighMap, int treeDepth):
pMap(0),
pDist(0),
pLow(0),
pNodes(0)
{
	pHigh = pHighMap;
	if (pHigh != 0)
		pHigh->pLow = this;
	pTree[0] = 0;
	this->treeDepth = MIN(treeDepth, MAX_TREE_DEPTH);
	Init();
}


CMap::~CMap()
{
	delete[] pMap;
	delete[] pDist;
	delete[] pNodes;
	delete[] pTree[0];
}

void CMap::Init(int DimX, int DimY)
{
	this->ImageX = DimX;
	this->ImageY = DimY;
	this->DimX = (DimX + 1) >> 1;
	this->DimY = (DimY + 1) >> 1;
	MapSize = this->DimX * this->DimY;
	if (MapSize != 0){
		pMap = new char[MapSize];
		pDist = new short[MapSize];
		pTree[0] = new node[(MapSize - (MapSize >> (2 * treeDepth))) / 3];
		if (pHigh != 0)
			pNodes = new node [MapSize];
		for( int i = 1; i < treeDepth; i++)
			pTree[i] = pTree[i-1] + (MapSize >> (2 * i));
	}
}

void CMap::SetDir(int Dir)
{
	for( int i = 0; i < MapSize ; i++){
		pMap[i] = Dir;
	}
}

void CMap::SetDir(int Dir, int x, int y, int depth)
{
	char * pCurMap = pMap;
	int size = 1 << depth;
	pCurMap += (x << depth) + (y << depth) * DimX;

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
		int Out = ((int)pDist[i]) + 128;
		Out = CLIP(Out, 0, 255);
		pOut[i] = (unsigned char) Out;
	}
}

void CMap::SetRange(CMuxCodec * RangeCodec)
{
	DirCodec.SetRange(RangeCodec);
	NodeCodec.SetRange(RangeCodec);
	LeafCodec.SetRange(RangeCodec);
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
		if (pDist[i] >= 0)
			pMap[i] = 1;
	}

	return;
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

	for( int l = 1; l < treeDepth; l++){
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
	int width = DimX >> treeDepth;
	int height = DimY >> treeDepth;
	node * pCurTree = pTree[treeDepth - 1];

	for( int j = 0; j < height; j++){
		for( int i = 0; i < width; i++){
			if (pCurTree[i].rate == 0)
				SetDir(pCurTree[i].refDist < 0 ? 0 : 1, i, j, treeDepth);
			else
				ApplyTree(i << 1, j << 1, treeDepth - 1);
		}
		pCurTree += width;
	}
}

void CMap::ApplyTree(int x, int y, int depth)
{
	if (depth == 0){
		pMap[x + DimX * y] = pDist[x + DimX * y] > 0 ? 1 : 0;
		pMap[x + 1 + DimX * y] = pDist[x + 1 + DimX * y] > 0 ? 1 : 0;
		pMap[x + DimX * (y + 1)] = pDist[x + DimX * (y + 1)] > 0 ? 1 : 0;
		pMap[x + 1 + DimX * (y + 1)] = pDist[x + 1 + DimX * (y + 1)] > 0 ? 1 : 0;
		return;
	}
	node * pCurTree = pTree[depth - 1];
	int width = DimX >> depth;
	pCurTree += y * width;
	if (pCurTree[x].rate == 0)
		SetDir(pCurTree[x].refDist < 0 ? 0 : 1, x, y, depth);
	else
		ApplyTree(x << 1, y << 1, depth - 1);
	x++;
	if (pCurTree[x].rate == 0)
		SetDir(pCurTree[x].refDist < 0 ? 0 : 1, x, y, depth);
	else
		ApplyTree(x << 1, y << 1, depth - 1);
	pCurTree += width;
	y++;
	if (pCurTree[x].rate == 0)
		SetDir(pCurTree[x].refDist < 0 ? 0 : 1, x, y, depth);
	else
		ApplyTree(x << 1, y << 1, depth - 1);
	x--;
	if (pCurTree[x].rate == 0)
		SetDir(pCurTree[x].refDist < 0 ? 0 : 1, x, y, depth);
	else
		ApplyTree(x << 1, y << 1, depth - 1);
}

void CMap::BuidNodes(float const lambda)
{
	int width = pLow->DimX;
	int height = pLow->DimY;
	int stride2 = DimX * 2;
	short * pCurDist1 = pDist;
	short * pCurDist2 = pDist + DimX;
	node * pCurNodes = pLow->pNodes;

	for (int j = 0; j < height; j++) {
		for (int i = 0, k = 0; i < width; i++, k += 2) {
			pCurNodes[i].refDist = pCurDist1[k] + pCurDist1[k + 1]
					+ pCurDist2[k] + pCurDist2[k + 1];
			int Dist = MIN(0, pCurDist1[k]);
			Dist += MIN(0, pCurDist1[k + 1]);
			Dist += MIN(0, pCurDist2[k]);
			Dist += MIN(0, pCurDist2[k + 1]);
			pCurNodes[i].dist = Dist;
			if (pCurNodes[i].refDist < 0)
				Dist -= pCurNodes[i].refDist;
			pCurNodes[i].rate = 0;
			if ((3 + lambda * Dist) < 0)
				pCurNodes[i].rate = 3;
		}
		pCurDist1 += stride2;
		pCurDist2 += stride2;
		pCurNodes += width;
	}

	CMap * pCurMap = pLow;
	while( pCurMap->pLow != 0 ){
		node * pHighNodes1 = pCurMap->pNodes;
		node * pHighNodes2 = pHighNodes1 + width;
		pCurDist1 = pCurMap->pDist;
		pCurDist2 = pCurDist1 + width;
		pCurNodes = pCurMap->pLow->pNodes;
		stride2 = width * 2;
		height = pCurMap->pLow->DimY;
		width = pCurMap->pLow->DimX;

		for( int j = 0; j < height; j++){
			for( int i = 0, k = 0; i < width; i++, k += 2){
				pCurNodes[i].refDist = pHighNodes1[k].refDist
						+ pHighNodes1[k + 1].refDist + pHighNodes2[k].refDist
						+ pHighNodes2[k + 1].refDist
						+ pCurDist1[k] + pCurDist1[k + 1]
						+ pCurDist2[k] + pCurDist2[k + 1];
				int rate = 11 + pHighNodes1[k].rate + pHighNodes1[k + 1].rate
						+ pHighNodes2[k].rate + pHighNodes2[k + 1].rate;
				int Dist = MIN(0, pCurDist1[k]);
				Dist += MIN(0, pCurDist1[k + 1]);
				Dist += MIN(0, pCurDist2[k]);
				Dist += MIN(0, pCurDist2[k + 1]);
				if (pHighNodes1[k].rate > 0)
					Dist += pHighNodes1[k].dist;
				else
					Dist += MIN(0, pHighNodes1[k].refDist);
				if (pHighNodes1[k + 1].rate > 0)
					Dist += pHighNodes1[k + 1].dist;
				else
					Dist += MIN(0, pHighNodes1[k + 1].refDist);
				if (pHighNodes2[k].rate > 0)
					Dist += pHighNodes2[k].dist;
				else
					Dist += MIN(0, pHighNodes2[k].refDist);
				if (pHighNodes2[k + 1].rate > 0)
					Dist += pHighNodes2[k + 1].dist;
				else
					Dist += MIN(0, pHighNodes2[k + 1].refDist);
				pCurNodes[i].dist = Dist;
				if (pCurNodes[i].refDist < 0)
					Dist -= pCurNodes[i].refDist;
				pCurNodes[i].rate = 0;
				if ((rate + lambda * Dist) < 0)
					pCurNodes[i].rate = rate;
			}
			pHighNodes1 += stride2;
			pHighNodes2 += stride2;
			pCurDist1 += stride2;
			pCurDist2 += stride2;
			pCurNodes += width;
		}
		pCurMap = pCurMap->pLow;
	}
}

void CMap::ApplyNodes(void)
{
	CMap * pCurMap = this;
	while( pCurMap->pLow != 0 ){
		pCurMap = pCurMap->pLow;
	}

	int width = pCurMap->DimX;
	int height = pCurMap->DimY;
	node * pCurNodes = pCurMap->pNodes;

	for( int j = 0; j < height; j++){
		for( int i = 0; i < width; i++){
			if (pCurNodes[i].rate == 0)
				pCurMap->SetDir(pCurNodes[i].refDist < 0 ? 0 : 1, i, j);
			else
				pCurMap->pHigh->ApplyNodes(i << 1, j << 1);
		}
		pCurNodes += width;
	}
}

void CMap::SetDir(int Dir, int x, int y)
{
	CMap * pCurMap = this;
	int shift = 1;

	while( pCurMap->pHigh != 0 ){
		pCurMap = pCurMap->pHigh;
		int stride = pCurMap->DimX;
		char * pMap = pCurMap->pMap;
		pMap += (x << shift) + (y << shift) * stride;
		int size = 1 << shift;

		for( int j = 0; j < size; j++){
			for( int i = 0; i < size; i++){
				pMap[i] = Dir;
			}
			pMap += stride;
		}

		shift++;
	}
}

void CMap::ApplyNodes(int x, int y)
{
	pMap[x + DimX * y] = pDist[x + DimX * y] < 0 ? 0 : 1;
	pMap[x + 1 + DimX * y] = pDist[x + 1 + DimX * y] < 0 ? 0 : 1;
	pMap[x + DimX * (y + 1)] = pDist[x + DimX * (y + 1)] < 0 ? 0 : 1;
	pMap[x + 1 + DimX * (y + 1)] = pDist[x + 1 + DimX * (y + 1)] < 0 ? 0 : 1;

	if (pHigh == 0)
		return;

	node * pCurNodes = pNodes + y * DimX;
	if (pCurNodes[x].rate == 0)
		SetDir(pCurNodes[x].refDist < 0 ? 0 : 1, x, y);
	else
		pHigh->ApplyNodes(x << 1, y << 1);
	x++;
	if (pCurNodes[x].rate == 0)
		SetDir(pCurNodes[x].refDist < 0 ? 0 : 1, x, y);
	else
		pHigh->ApplyNodes(x << 1, y << 1);
	pCurNodes += DimX;
	y++;
	if (pCurNodes[x].rate == 0)
		SetDir(pCurNodes[x].refDist < 0 ? 0 : 1, x, y);
	else
		pHigh->ApplyNodes(x << 1, y << 1);
	x--;
	if (pCurNodes[x].rate == 0)
		SetDir(pCurNodes[x].refDist < 0 ? 0 : 1, x, y);
	else
		pHigh->ApplyNodes(x << 1, y << 1);
}



void CMap::CodeNodes(void)
{
	CMap * pCurMap = this;
	while( pCurMap->pLow != 0 ){
		pCurMap->DirCodec.InitModel();
		pCurMap->NodeCodec.InitModel();
		pCurMap->LeafCodec.InitModel();
		pCurMap = pCurMap->pLow;
	}
	pCurMap->DirCodec.InitModel();
	pCurMap->NodeCodec.InitModel();
	pCurMap->LeafCodec.InitModel();

	int width = pCurMap->DimX;
	int height = pCurMap->DimY;
	node * pCurNodes = pCurMap->pNodes;
	char * pMap = pCurMap->pMap;

	for( int j = 0; j < height; j++){
		for( int i = 0; i < width; i++){
			pCurMap->DirCodec.Code(pMap[i], 0);
			if (pCurNodes[i].rate == 0){
				pCurMap->NodeCodec.Code0(0);
				if (pCurNodes[i].refDist < 0)
					pCurMap->LeafCodec.Code0(pMap[i]);
				else
					pCurMap->LeafCodec.Code1(pMap[i]);
			}else{
				pCurMap->NodeCodec.Code1(0);
				pCurMap->pHigh->CodeNodes(i << 1, j << 1, pMap[i]);
			}
		}
		pCurNodes += width;
		pMap += width;
	}
}

void CMap::CodeNodes(int x, int y, int context)
{
	short * pCurDist = pDist + y * DimX + x;
	if (pCurDist[0] < 0)
		DirCodec.Code0(context);
	else
		DirCodec.Code1(context);
	pCurDist++;
	if (pCurDist[0] < 0)
		DirCodec.Code0(context);
	else
		DirCodec.Code1(context);
	pCurDist += DimX;
	if (pCurDist[0] < 0)
		DirCodec.Code0(context);
	else
		DirCodec.Code1(context);
	pCurDist--;
	if (pCurDist[0] < 0)
		DirCodec.Code0(context);
	else
		DirCodec.Code1(context);

	if (pHigh == 0)
		return;

	node * pCurNodes = pNodes + y * DimX;
	pCurDist -= DimX;
	if (pCurNodes[x].rate == 0){
		NodeCodec.Code0(0);
		if (pCurNodes[x].refDist < 0)
			LeafCodec.Code0(pCurDist[0] < 0 ? 0 : 1);
		else
			LeafCodec.Code1(pCurDist[0] < 0 ? 0 : 1);
	}else{
		NodeCodec.Code1(0);
		pHigh->CodeNodes(x << 1, y << 1, pCurDist[0] < 0 ? 0 : 1);
	}
	x++;
	pCurDist++;
	if (pCurNodes[x].rate == 0){
		NodeCodec.Code0(0);
		if (pCurNodes[x].refDist < 0)
			LeafCodec.Code0(pCurDist[0] < 0 ? 0 : 1);
		else
			LeafCodec.Code1(pCurDist[0] < 0 ? 0 : 1);
	}else{
		NodeCodec.Code1(0);
		pHigh->CodeNodes(x << 1, y << 1, pCurDist[0] < 0 ? 0 : 1);
	}
	pCurNodes += DimX;
	pCurDist += DimX;
	y++;
	if (pCurNodes[x].rate == 0){
		NodeCodec.Code0(0);
		if (pCurNodes[x].refDist < 0)
			LeafCodec.Code0(pCurDist[0] < 0 ? 0 : 1);
		else
			LeafCodec.Code1(pCurDist[0] < 0 ? 0 : 1);
	}else{
		NodeCodec.Code1(0);
		pHigh->CodeNodes(x << 1, y << 1, pCurDist[0] < 0 ? 0 : 1);
	}
	x--;
	pCurDist--;
	if (pCurNodes[x].rate == 0){
		NodeCodec.Code0(0);
		if (pCurNodes[x].refDist < 0)
			LeafCodec.Code0(pCurDist[0] < 0 ? 0 : 1);
		else
			LeafCodec.Code1(pCurDist[0] < 0 ? 0 : 1);
	}else{
		NodeCodec.Code1(0);
		pHigh->CodeNodes(x << 1, y << 1, pCurDist[0] < 0 ? 0 : 1);
	}
}

void CMap::DecodeNodes(void)
{
	CMap * pCurMap = this;
	while( pCurMap->pLow != 0 ){
		pCurMap->DirCodec.InitModel();
		pCurMap->NodeCodec.InitModel();
		pCurMap->LeafCodec.InitModel();
		pCurMap = pCurMap->pLow;
	}
	pCurMap->DirCodec.InitModel();
	pCurMap->NodeCodec.InitModel();
	pCurMap->LeafCodec.InitModel();

	int width = pCurMap->DimX;
	int height = pCurMap->DimY;
	char * pMap = pCurMap->pMap;

	for( int j = 0; j < height; j++){
		for( int i = 0; i < width; i++){
			pMap[i] = pCurMap->DirCodec.Decode(0);
			if (pCurMap->NodeCodec.Decode(0) == 0){
				pCurMap->SetDir(pCurMap->LeafCodec.Decode(pMap[i]), i, j);
			}else{
				pCurMap->pHigh->DecodeNodes(i << 1, j << 1, pMap[i]);
			}
		}
		pMap += width;
	}
}

void CMap::DecodeNodes(int x, int y, int context)
{
	char * pCurMap = pMap + y * DimX + x;
	pCurMap[0] = DirCodec.Decode(context);
	pCurMap++;
	pCurMap[0] = DirCodec.Decode(context);
	pCurMap += DimX;
	pCurMap[0] = DirCodec.Decode(context);
	pCurMap--;
	pCurMap[0] = DirCodec.Decode(context);

	if (pHigh == 0)
		return;

	pCurMap -= DimX;

	if (NodeCodec.Decode(0) == 0)
		SetDir(LeafCodec.Decode(pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
	x++;
	pCurMap++;
	if (NodeCodec.Decode(0) == 0)
		SetDir(LeafCodec.Decode(pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
	pCurMap += DimX;
	y++;
	if (NodeCodec.Decode(0) == 0)
		SetDir(LeafCodec.Decode(pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
	x--;
	pCurMap--;
	if (NodeCodec.Decode(0) == 0)
		SetDir(LeafCodec.Decode(pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
}

void CMap::GetImageDist(float * pImage1, float * pImage2, int stride)
{
	short * pDir = this->pDist;
	int diff = 2 * stride - ImageX;
//  	float wl = weightL * weightL * 65536;
	float wl = weightL * 256;
	int end = stride * ImageY;

	for( int pos1 = 1, pos2 = stride; pos1 < end; pos1 += diff, pos2 += diff){
		for( int stop = pos1 + ImageX; pos1 < stop; pos1 += 2, pos2 += 2){
// 			pDir[0] = (short) (sqrtf((pImage1[pos1] * pImage1[pos1] +
// 					pImage1[pos2] * pImage1[pos2]) * wl) -
// 					sqrtf((pImage2[pos1] * pImage2[pos1] +
// 					pImage2[pos2] * pImage2[pos2]) * wl));
			pDir[0] = (short) ((fabsf(pImage1[pos1]) +
					fabsf(pImage1[pos2]) - fabsf(pImage2[pos1]) -
					fabsf(pImage2[pos2])) * wl);
			pDir++;
		}
	}
}

void CMap::GetImageDist(float * pImage1, float * pImage2,
						float * pBand1, float * pBand2, int stride)
{
	short * pDir = this->pDist;
	int diff = 2 * stride - ImageX;
// 	float wl = weightL * weightL * 65536;
// 	float wh = weightH * weightH * 65536;
	float wl = weightL * 256;
	float wh = weightH * 256;
	int end = stride * ImageY;

	for( int pos1 = 1, pos2 = stride; pos1 < end; pos1 += diff, pos2 += diff){
		for( int stop = pos1 + ImageX; pos1 < stop; pos1 += 2, pos2 += 2){
// 			pDir[0] = (short) (sqrtf((pImage1[pos1] * pImage1[pos1] +
// 					pImage1[pos2] * pImage1[pos2]) * wl +
// 					(pBand1[pos1] * pBand1[pos1] +
// 					pBand1[pos2] * pBand1[pos2]) * wh) -
// 					sqrtf((pImage2[pos1] * pImage2[pos1] +
// 					pImage2[pos2] * pImage2[pos2]) * wl +
// 					(pBand2[pos1] * pBand2[pos1] +
// 					pBand2[pos2] * pBand2[pos2]) * wh));
			pDir[0] = (short) ((fabsf(pImage1[pos1]) + fabsf(pImage1[pos2]) -
					fabsf(pImage2[pos1]) - fabsf(pImage2[pos2])) * wl +
					(fabsf(pBand1[pos1]) + fabsf(pBand1[pos2]) -
					fabsf(pBand2[pos1]) - fabsf(pBand2[pos2])) * wh);
			pDir++;
		}
	}
}

void CMap::GetImageDistDiag(float * pImage1, float * pImage2, int stride)
{
	short * pDir = this->pDist;
	int diff = 2 * stride - ImageX;
// 	float wl = weightL * weightL * 65536;
// 	float wh = weightH * weightH * 65536;
	float wl = weightL * 256;
	float wh = weightH * 256;
	int end = stride * ImageY;

	for( int pos1 = stride; pos1 < end; pos1 += diff){
		for( int stop = pos1 + ImageX; pos1 < stop; pos1 += 2){
// 			pDir[0] = (short) (sqrtf(pImage1[pos1] * pImage1[pos1] * wh +
// 					pImage1[pos1 + 1] * pImage1[pos1 + 1] * wl) -
// 					sqrtf(pImage2[pos1] * pImage2[pos1] * wh +
// 					pImage2[pos1 + 1] * pImage2[pos1 + 1] * wl));
			pDir[0] = (short) ((fabsf(pImage1[pos1]) - fabsf(pImage2[pos1])) * wh +
					(fabsf(pImage1[pos1 + 1]) - fabsf(pImage2[pos1 + 1])) * wl);
			pDir++;
		}
	}
}

}
