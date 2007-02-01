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
#include "map.h"

#include <math.h>
#include <string.h>
#include <iostream>

using namespace std;

namespace libdwic {

CMap::CMap(CMap * pHighMap):
pMap(0),
pDist(0),
pLow(0),
pNodes(0)
{
	pHigh = pHighMap;
	if (pHigh != 0)
		pHigh->pLow = this;
	Init();
}


CMap::~CMap()
{
	delete[] pMap;
	delete[] pDist;
	delete[] pNodes;
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
		pDist = new dist_t[MapSize];
		if (pHigh != 0)
			pNodes = new node [MapSize];
	}
}

void CMap::SetDir(int Dir)
{
	memset(pMap, Dir, MapSize);
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

void CMap::SetCodec(CMuxCodec * pCodec)
{
	DirCodec.setRange(pCodec);
	NodeCodec.setRange(pCodec);
	LeafCodec.setRange(pCodec);
}

void CMap::SelectDir(void)
{
	for( int i = 0; i < MapSize; i++) {
		char tmp = (pDist[i].d1 | pDist[i].d0) >= 0;
		pMap[i] = tmp * 2 + (pDist[i].d1 < pDist[i].d0) & ~tmp;
	}
}

void CMap::BuidNodes(const int lambda)
{
	int width = pLow->DimX;
	int height = pLow->DimY;
	int stride2 = DimX * 2;
	dist_t * pCurDist1 = pDist;
	dist_t * pCurDist2 = pDist + DimX;
	node * pCurNodes = pLow->pNodes;

	for (int j = 0; j < height; j++) {
		for (int i = 0, k = 0; i < width; i++, k += 2) {
			pCurNodes[i].d0 = pCurDist1[k].d0 + pCurDist1[k + 1].d0
					+ pCurDist2[k].d0 + pCurDist2[k + 1].d0;
			pCurNodes[i].d1 = pCurDist1[k].d1 + pCurDist1[k + 1].d1
					+ pCurDist2[k].d1 + pCurDist2[k + 1].d1;

			int Dist = min<short>(0, pCurDist1[k].d0, pCurDist1[k].d1);
			Dist += min<short>(0, pCurDist1[k + 1].d0, pCurDist1[k + 1].d1);
			Dist += min<short>(0, pCurDist2[k].d0, pCurDist2[k].d1);
			Dist += min<short>(0, pCurDist2[k + 1].d0, pCurDist2[k + 1].d1);
			pCurNodes[i].dist = Dist;
			Dist -= min(0, pCurNodes[i].d0, pCurNodes[i].d1);
			pCurNodes[i].rate = 0;
			if ((5 * lambda + Dist) <= 0) // 5 here means log2(3) * (4 - 1) ~ 4.75
				pCurNodes[i].rate = 5;
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
				pCurNodes[i].d0 = pHighNodes1[k].d0 + pHighNodes1[k + 1].d0
						+ pHighNodes2[k].d0 + pHighNodes2[k + 1].d0
						+ pCurDist1[k].d0 + pCurDist1[k + 1].d0
						+ pCurDist2[k].d0 + pCurDist2[k + 1].d0;
				pCurNodes[i].d1 = pHighNodes1[k].d1 + pHighNodes1[k + 1].d1
						+ pHighNodes2[k].d1 + pHighNodes2[k + 1].d1
						+ pCurDist1[k].d1 + pCurDist1[k + 1].d1
						+ pCurDist2[k].d1 + pCurDist2[k + 1].d1;
				int rate = 15 + pHighNodes1[k].rate + pHighNodes1[k + 1].rate
						+ pHighNodes2[k].rate + pHighNodes2[k + 1].rate; // 15 = log2(3) * ((4-1) + 4) + 4 * log2(2)
				int Dist = min<short>(0, pCurDist1[k].d0, pCurDist1[k].d1);
				Dist += min<short>(0, pCurDist1[k + 1].d0, pCurDist1[k + 1].d1);
				Dist += min<short>(0, pCurDist2[k].d0, pCurDist2[k].d1);
				Dist += min<short>(0, pCurDist2[k + 1].d0, pCurDist2[k + 1].d1);
				if (pHighNodes1[k].rate > 0)
					Dist += pHighNodes1[k].dist;
				else
					Dist += min(0, pHighNodes1[k].d0, pHighNodes1[k].d1);
				if (pHighNodes1[k + 1].rate > 0)
					Dist += pHighNodes1[k + 1].dist;
				else
					Dist += min(0, pHighNodes1[k + 1].d0, pHighNodes1[k + 1].d1);
				if (pHighNodes2[k].rate > 0)
					Dist += pHighNodes2[k].dist;
				else
					Dist += min(0, pHighNodes2[k].d0, pHighNodes2[k].d1);
				if (pHighNodes2[k + 1].rate > 0)
					Dist += pHighNodes2[k + 1].dist;
				else
					Dist += min(0, pHighNodes2[k + 1].d0, pHighNodes2[k + 1].d1);
				pCurNodes[i].dist = Dist;
				Dist -= min(0, pCurNodes[i].d0, pCurNodes[i].d1);
				pCurNodes[i].rate = 0;
				if ((rate * lambda + Dist) < 0)
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
			if (pCurNodes[i].rate == 0) {
				pCurMap->SetDir(GetDir(pCurNodes[i].d0, pCurNodes[i].d1), i, j);
			} else
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
	pMap[x + DimX * y] = GetDir(pDist[x + DimX * y].d0, pDist[x + DimX * y].d1);
	pMap[x + 1 + DimX * y] = GetDir(pDist[x + 1 + DimX * y].d0, pDist[x + 1 + DimX * y].d1);
	pMap[x + DimX * (y + 1)] = GetDir(pDist[x + DimX * (y + 1)].d0, pDist[x + DimX * (y + 1)].d1);
	pMap[x + 1 + DimX * (y + 1)] = GetDir(pDist[x + 1 + DimX * (y + 1)].d0, pDist[x + 1 + DimX * (y + 1)].d1);

	if (pHigh == 0)
		return;

	node * pCurNodes = pNodes + y * DimX;
	if (pCurNodes[x].rate == 0)
		SetDir(GetDir(pCurNodes[x].d0, pCurNodes[x].d1), x, y);
	else
		pHigh->ApplyNodes(x << 1, y << 1);
	x++;
	if (pCurNodes[x].rate == 0)
		SetDir(GetDir(pCurNodes[x].d0, pCurNodes[x].d1), x, y);
	else
		pHigh->ApplyNodes(x << 1, y << 1);
	pCurNodes += DimX;
	y++;
	if (pCurNodes[x].rate == 0)
		SetDir(GetDir(pCurNodes[x].d0, pCurNodes[x].d1), x, y);
	else
		pHigh->ApplyNodes(x << 1, y << 1);
	x--;
	if (pCurNodes[x].rate == 0)
		SetDir(GetDir(pCurNodes[x].d0, pCurNodes[x].d1), x, y);
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
			CodeDir(pMap[i], pCurMap->DirCodec, 0);
			if (pCurNodes[i].rate == 0){
				pCurMap->NodeCodec.code0(0);
				CodeDir(GetDir(pCurNodes[i].d0, pCurNodes[i].d1), pCurMap->LeafCodec, pMap[i]);
			}else{
				pCurMap->NodeCodec.code1(0);
				pCurMap->pHigh->CodeNodes(i << 1, j << 1, pMap[i]);
			}
		}
		pCurNodes += width;
		pMap += width;
	}
}

void CMap::CodeNodes(int x, int y, int context)
{
	dist_t * pCurDist = pDist + y * DimX + x;
	char dirs[4];
	CodeDir(dirs[0] = GetDir(pCurDist[0].d0, pCurDist[0].d1), DirCodec, context);
	pCurDist++;
	CodeDir(dirs[1] = GetDir(pCurDist[0].d0, pCurDist[0].d1), DirCodec, context);
	pCurDist += DimX;
	CodeDir(dirs[2] = GetDir(pCurDist[0].d0, pCurDist[0].d1), DirCodec, context);
	pCurDist--;
	CodeDir(dirs[3] = GetDir(pCurDist[0].d0, pCurDist[0].d1), DirCodec, context);

	if (pHigh == 0)
		return;

	node * pCurNodes = pNodes + y * DimX;
	pCurDist -= DimX;
	if (pCurNodes[x].rate == 0){
		NodeCodec.code0(0);
		CodeDir(GetDir(pCurNodes[x].d0, pCurNodes[x].d1), LeafCodec, dirs[0]);
	}else{
		NodeCodec.code1(0);
		pHigh->CodeNodes(x << 1, y << 1, dirs[0]);
	}
	x++;
	pCurDist++;
	if (pCurNodes[x].rate == 0){
		NodeCodec.code0(0);
		CodeDir(GetDir(pCurNodes[x].d0, pCurNodes[x].d1), LeafCodec, dirs[1]);
	}else{
		NodeCodec.code1(0);
		pHigh->CodeNodes(x << 1, y << 1, dirs[1]);
	}
	pCurNodes += DimX;
	pCurDist += DimX;
	y++;
	if (pCurNodes[x].rate == 0){
		NodeCodec.code0(0);
		CodeDir(GetDir(pCurNodes[x].d0, pCurNodes[x].d1), LeafCodec, dirs[2]);
	}else{
		NodeCodec.code1(0);
		pHigh->CodeNodes(x << 1, y << 1, dirs[2]);
	}
	x--;
	pCurDist--;
	if (pCurNodes[x].rate == 0){
		NodeCodec.code0(0);
		CodeDir(GetDir(pCurNodes[x].d0, pCurNodes[x].d1), LeafCodec, dirs[3]);
	}else{
		NodeCodec.code1(0);
		pHigh->CodeNodes(x << 1, y << 1, dirs[3]);
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
			pMap[i] = DecodeDir(pCurMap->DirCodec, 0);
			if (pCurMap->NodeCodec.decode(0) == 0){
				pCurMap->SetDir(DecodeDir(pCurMap->LeafCodec, pMap[i]), i, j);
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
	pCurMap[0] = DecodeDir(DirCodec, context);
	pCurMap++;
	pCurMap[0] = DecodeDir(DirCodec, context);
	pCurMap += DimX;
	pCurMap[0] = DecodeDir(DirCodec, context);
	pCurMap--;
	pCurMap[0] = DecodeDir(DirCodec, context);

	if (pHigh == 0)
		return;

	pCurMap -= DimX;

	if (NodeCodec.decode(0) == 0)
		SetDir(DecodeDir(LeafCodec, pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
	x++;
	pCurMap++;
	if (NodeCodec.decode(0) == 0)
		SetDir(DecodeDir(LeafCodec, pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
	pCurMap += DimX;
	y++;
	if (NodeCodec.decode(0) == 0)
		SetDir(DecodeDir(LeafCodec, pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
	x--;
	pCurMap--;
	if (NodeCodec.decode(0) == 0)
		SetDir(DecodeDir(LeafCodec, pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
}

void CMap::GetImageDist(float * pImg1, float * pImg2, float * pImg3, int stride)
{
	dist_t * pDir = this->pDist;
	int diff = 2 * stride - ImageX;
	float wl = weightL * 256;
	int end = stride * ImageY;

	for( int pos1 = 1, pos2 = stride; pos1 < end; pos1 += diff, pos2 += diff){
		for( int stop = pos1 + ImageX; pos1 < stop; pos1 += 2, pos2 += 2){
			float tmp = fabsf(pImg3[pos1]) + fabsf(pImg3[pos2]);
			pDir[0].d0 = (short) ((fabsf(pImg1[pos1]) + fabsf(pImg1[pos2]) - tmp) * wl);
			pDir[0].d1 = (short) ((fabsf(pImg2[pos1]) + fabsf(pImg2[pos2]) - tmp) * wl);
			pDir++;
		}
	}
}

void CMap::GetImageDist(float * pImg1, float * pImg2, float * pImg3,
						float * pBand1, float * pBand2, float * pBand3,
						int stride)
{
	dist_t * pDir = this->pDist;
	int diff = 2 * stride - ImageX;
	float wl = weightL * 256;
	float wh = weightH * 256;
	int end = stride * ImageY;

	for( int pos1 = 1, pos2 = stride; pos1 < end; pos1 += diff, pos2 += diff){
		for( int stop = pos1 + ImageX; pos1 < stop; pos1 += 2, pos2 += 2){
			float tmp1 = fabsf(pImg3[pos1]) + fabsf(pImg3[pos2]);
			float tmp2 = fabsf(pBand3[pos1]) + fabsf(pBand3[pos2]);
			pDir[0].d0 = (short) ((fabsf(pImg1[pos1]) + fabsf(pImg1[pos2]) - tmp1) * wl
					+ (fabsf(pBand1[pos1]) + fabsf(pBand1[pos2]) - tmp2) * wh);
			pDir[0].d1 = (short) ((fabsf(pImg2[pos1]) + fabsf(pImg2[pos2]) - tmp1) * wl
					+ (fabsf(pBand2[pos1]) + fabsf(pBand2[pos2]) - tmp2) * wh);
			pDir++;
		}
	}
}

void CMap::GetImageDistDiag(float * pImg1, float * pImg2, float * pImg3,
							int stride)
{
	dist_t * pDir = this->pDist;
	int diff = 2 * stride - ImageX;
	float wl = weightL * 256;
	float wh = weightH * 256;
	int end = stride * ImageY;

	for( int pos1 = stride; pos1 < end; pos1 += diff){
		for( int stop = pos1 + ImageX; pos1 < stop; pos1 += 2){
			float tmp1 = fabsf(pImg3[pos1]);
			float tmp2 = fabsf(pImg3[pos1 + 1]);
			pDir[0].d0 = (short) ((fabsf(pImg1[pos1]) - tmp1) * wh +
					(fabsf(pImg1[pos1 + 1]) - tmp2) * wl);
			pDir[0].d1 = (short) ((fabsf(pImg2[pos1]) - tmp1) * wh +
					(fabsf(pImg2[pos1 + 1]) - tmp2) * wl);
			pDir++;
		}
	}
}

}
