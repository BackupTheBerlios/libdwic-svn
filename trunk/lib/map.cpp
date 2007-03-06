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
	this->DimX = (DimX + 3) >> 2;
	this->DimY = (DimY + 3) >> 2;
	MapSize = this->DimX * this->DimY;
	if (MapSize != 0){
		pMap = new char[MapSize];
		pDist = new int[MapSize];
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

void CMap::GetDist(unsigned char * pOut)
{
	for( int i = 0; i < MapSize; i++){
		int Out = ((int)pDist[i]) + 128;
		Out = CLIP(Out, 0, 255);
		pOut[i] = (unsigned char) Out;
	}
}

void CMap::SetCodec(CMuxCodec * pCodec)
{
	DirCodec.setRange(pCodec);
	NodeCodec.setRange(pCodec);
	LeafCodec.setRange(pCodec);
}

void CMap::SelectDir(void)
{
	for( int i = 0; i < MapSize; i++)
		pMap[i] = pDist[i] >= 0;
}

void CMap::BuidNodes(float const lambda)
{
	int width = pLow->DimX;
	int height = pLow->DimY;
	int stride2 = DimX * 2;
	int * pCurDist1 = pDist;
	int * pCurDist2 = pDist + DimX;
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
				if ((rate + lambda * Dist) <= 0)
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
			pCurMap->DirCodec.code(pMap[i], 0);
			if (pCurNodes[i].rate == 0){
				pCurMap->NodeCodec.code0(0);
				if (pCurNodes[i].refDist < 0)
					pCurMap->LeafCodec.code0(pMap[i]);
				else
					pCurMap->LeafCodec.code1(pMap[i]);
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
	int * pCurDist = pDist + y * DimX + x;
	if (pCurDist[0] < 0)
		DirCodec.code0(context);
	else
		DirCodec.code1(context);
	pCurDist++;
	if (pCurDist[0] < 0)
		DirCodec.code0(context);
	else
		DirCodec.code1(context);
	pCurDist += DimX;
	if (pCurDist[0] < 0)
		DirCodec.code0(context);
	else
		DirCodec.code1(context);
	pCurDist--;
	if (pCurDist[0] < 0)
		DirCodec.code0(context);
	else
		DirCodec.code1(context);

	if (pHigh == 0)
		return;

	node * pCurNodes = pNodes + y * DimX;
	pCurDist -= DimX;
	if (pCurNodes[x].rate == 0){
		NodeCodec.code0(0);
		if (pCurNodes[x].refDist < 0)
			LeafCodec.code0(pCurDist[0] < 0 ? 0 : 1);
		else
			LeafCodec.code1(pCurDist[0] < 0 ? 0 : 1);
	}else{
		NodeCodec.code1(0);
		pHigh->CodeNodes(x << 1, y << 1, pCurDist[0] < 0 ? 0 : 1);
	}
	x++;
	pCurDist++;
	if (pCurNodes[x].rate == 0){
		NodeCodec.code0(0);
		if (pCurNodes[x].refDist < 0)
			LeafCodec.code0(pCurDist[0] < 0 ? 0 : 1);
		else
			LeafCodec.code1(pCurDist[0] < 0 ? 0 : 1);
	}else{
		NodeCodec.code1(0);
		pHigh->CodeNodes(x << 1, y << 1, pCurDist[0] < 0 ? 0 : 1);
	}
	pCurNodes += DimX;
	pCurDist += DimX;
	y++;
	if (pCurNodes[x].rate == 0){
		NodeCodec.code0(0);
		if (pCurNodes[x].refDist < 0)
			LeafCodec.code0(pCurDist[0] < 0 ? 0 : 1);
		else
			LeafCodec.code1(pCurDist[0] < 0 ? 0 : 1);
	}else{
		NodeCodec.code1(0);
		pHigh->CodeNodes(x << 1, y << 1, pCurDist[0] < 0 ? 0 : 1);
	}
	x--;
	pCurDist--;
	if (pCurNodes[x].rate == 0){
		NodeCodec.code0(0);
		if (pCurNodes[x].refDist < 0)
			LeafCodec.code0(pCurDist[0] < 0 ? 0 : 1);
		else
			LeafCodec.code1(pCurDist[0] < 0 ? 0 : 1);
	}else{
		NodeCodec.code1(0);
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
			pMap[i] = pCurMap->DirCodec.decode(0);
			if (pCurMap->NodeCodec.decode(0) == 0){
				pCurMap->SetDir(pCurMap->LeafCodec.decode(pMap[i]), i, j);
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
	pCurMap[0] = DirCodec.decode(context);
	pCurMap++;
	pCurMap[0] = DirCodec.decode(context);
	pCurMap += DimX;
	pCurMap[0] = DirCodec.decode(context);
	pCurMap--;
	pCurMap[0] = DirCodec.decode(context);

	if (pHigh == 0)
		return;

	pCurMap -= DimX;

	if (NodeCodec.decode(0) == 0)
		SetDir(LeafCodec.decode(pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
	x++;
	pCurMap++;
	if (NodeCodec.decode(0) == 0)
		SetDir(LeafCodec.decode(pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
	pCurMap += DimX;
	y++;
	if (NodeCodec.decode(0) == 0)
		SetDir(LeafCodec.decode(pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
	x--;
	pCurMap--;
	if (NodeCodec.decode(0) == 0)
		SetDir(LeafCodec.decode(pCurMap[0]), x, y);
	else
		pHigh->DecodeNodes(x << 1, y << 1, pCurMap[0]);
}

void CMap::GetImageDist(float * pImage1, float * pImage2, int stride)
{
	int * pDir = this->pDist;
	int diff = 4 * stride - ImageX;
	float wl = weightL * 256;
	int end = stride * ImageY;

	for( int pos1 = 1, pos2 = stride, pos3 = 2 * stride + 1, pos4 = 3 * stride;
			pos1 < end; pos1 += diff, pos2 += diff, pos3 += diff, pos4 += diff){
		for( int stop = pos1 + ImageX; pos1 < stop; pos1 += 4, pos2 += 4, pos3 += 4, pos4 += 4){
			pDir[0] = (int) ((
					fabsf(pImage1[pos1]) + fabsf(pImage1[pos1 + 2]) +
					fabsf(pImage1[pos2]) + fabsf(pImage1[pos2 + 2]) +
					fabsf(pImage1[pos3]) + fabsf(pImage1[pos3 + 2]) +
					fabsf(pImage1[pos4]) + fabsf(pImage1[pos4 + 2]) - (
					fabsf(pImage2[pos1]) + fabsf(pImage2[pos1 + 2]) +
					fabsf(pImage2[pos2]) + fabsf(pImage2[pos2 + 2]) +
					fabsf(pImage2[pos3]) + fabsf(pImage2[pos3 + 2]) +
					fabsf(pImage2[pos4]) + fabsf(pImage2[pos4 + 2]))) * wl);
			pDir++;
		}
	}
}

void CMap::GetImageDist(float * pImage1, float * pImage2,
						float * pBand1, float * pBand2, int stride)
{
	int * pDir = this->pDist;
	int diff = 4 * stride - ImageX;
	float wl = weightL * 256;
	float wh = weightH * 256;
	int end = stride * ImageY;

	for( int pos1 = 1, pos2 = stride, pos3 = 2 * stride + 1, pos4 = 3 * stride;
			pos1 < end; pos1 += diff, pos2 += diff, pos3 += diff, pos4 += diff){
		for( int stop = pos1 + ImageX; pos1 < stop; pos1 += 4, pos2 += 4, pos3 += 4, pos4 += 4){
			pDir[0] = (int) ((
					fabsf(pImage1[pos1]) + fabsf(pImage1[pos1 + 2]) +
					fabsf(pImage1[pos2]) + fabsf(pImage1[pos2 + 2]) +
					fabsf(pImage1[pos3]) + fabsf(pImage1[pos3 + 2]) +
					fabsf(pImage1[pos4]) + fabsf(pImage1[pos4 + 2]) - (
					fabsf(pImage2[pos1]) + fabsf(pImage2[pos1 + 2]) +
					fabsf(pImage2[pos2]) + fabsf(pImage2[pos2 + 2]) +
					fabsf(pImage2[pos3]) + fabsf(pImage2[pos3 + 2]) +
					fabsf(pImage2[pos4]) + fabsf(pImage2[pos4 + 2]))) * wl + (
					fabsf(pBand1[pos1]) + fabsf(pBand1[pos1 + 2]) +
					fabsf(pBand1[pos2]) + fabsf(pBand1[pos2 + 2]) +
					fabsf(pBand1[pos3]) + fabsf(pBand1[pos3 + 2]) +
					fabsf(pBand1[pos4]) + fabsf(pBand1[pos4 + 2]) - (
					fabsf(pBand2[pos1]) + fabsf(pBand2[pos1 + 2]) +
					fabsf(pBand2[pos2]) + fabsf(pBand2[pos2 + 2]) +
					fabsf(pBand2[pos3]) + fabsf(pBand2[pos3 + 2]) +
					fabsf(pBand2[pos4]) + fabsf(pBand2[pos4 + 2]))) * wh);
			pDir++;
		}
	}
}

void CMap::GetImageDistDiag(float * pImage1, float * pImage2, int stride)
{
	int * pDir = this->pDist;
	int diff = 4 * stride - ImageX;
	float wl = weightL * 256;
	float wh = weightH * 256;
	int end = stride * ImageY;

	for( int pos1 = stride, pos2 = 3 * stride; pos1 < end; pos1 += diff, pos2 += diff){
		for( int stop = pos1 + ImageX; pos1 < stop; pos1 += 4, pos2 += 4){
			pDir[0] = (int) ((
					fabsf(pImage1[pos1]) + fabsf(pImage1[pos1 + 2]) +
					fabsf(pImage1[pos2]) + fabsf(pImage1[pos2 + 2]) - (
					fabsf(pImage2[pos1]) + fabsf(pImage2[pos1 + 2]) +
					fabsf(pImage2[pos2]) + fabsf(pImage2[pos2 + 2]))) * wh + (
					fabsf(pImage1[pos1 + 1]) + fabsf(pImage1[pos1 + 3]) +
					fabsf(pImage1[pos2 + 1]) + fabsf(pImage1[pos2 + 3]) - (
					fabsf(pImage2[pos1 + 1]) + fabsf(pImage2[pos1 + 3]) +
					fabsf(pImage2[pos2 + 1]) + fabsf(pImage2[pos2 + 3]))) * wl);
			pDir++;
		}
	}
}

}
