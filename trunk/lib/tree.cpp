/***************************************************************************
 *   Copyright (C) 2007 by Nicolas BOTTI <rududu@laposte.net>              *
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
#include "tree.h"

namespace libdwic {

CTree::CTree( void ):
		pData(0)
{
	pParent = 0;
	pChild = 0;
	Init();
}


CTree::~CTree()
{
	delete[] pData;
}

void CTree::Init( unsigned int x, unsigned int y)
{
	DimX = x;
	DimY = y;
	if (x * y != 0){
		pData = new char[x * y * sizeof(treeBlock)];
		pBitField = (treeBlock*) pData;
	}
}

void CTree::SetCodec(CMuxCodec * pCodec)
{
	TreeCodec.setRange(pCodec);
	BlockCodec.setRange(pCodec);
}

template <class T>
int inline CTree::BlockSignif(T * pBlock, int stride, int width, int height) {
	T result = 0;
	for( int j = 0; j < height; j++){
		for( int i = 0; i < width; i++){
			result |= pBlock[i];
		}
		pBlock += stride;
	}
	return result != 0;
}

template <class T>
void CTree::Fill(T * HVHBand, T * HVLBand, T * DHBand, T * DLBand, int stride)
{
	treeBlock * pBits = pBitField, * pChBits = 0;
	int hv_stride = stride * 2;
	int d_stride = stride * 4;
	int ch_stride;

	if (pChild != 0) {
		pChBits = pChild->pBitField;
		ch_stride = pChild->DimX;
	}

	for( int j = 0; j < DimY; j +=2){
		for( int i = 0, k = 0; i < DimX; i += 2, k += 8){
			pBits[i].bitField = 0;
			pBits[i].b0.hv_l = BlockSignif(HVLBand + k, stride, 4, 2);
			pBits[i].b0.hv_h = BlockSignif(HVHBand + k, stride, 4, 2);
			pBits[i].b0.d_l = BlockSignif(DLBand + k, stride, 4, 4);
			pBits[i].b0.d_h = BlockSignif(DHBand + k, stride, 4, 4);

			pBits[i].b1.hv_l = BlockSignif(HVLBand + k + 4, stride, 4, 2);
			pBits[i].b1.hv_h = BlockSignif(HVHBand + k + 4, stride, 4, 2);
			pBits[i].b1.d_l = BlockSignif(DLBand + k + 4, stride, 4, 4);
			pBits[i].b1.d_h = BlockSignif(DHBand + k + 4, stride, 4, 4);

			pBits[i].b2.hv_l = BlockSignif(HVLBand + k + hv_stride + 4, stride, 4, 2);
			pBits[i].b2.hv_h = BlockSignif(HVHBand + k + hv_stride + 4, stride, 4, 2);
			pBits[i].b2.d_l = BlockSignif(DLBand + k + d_stride + 4, stride, 4, 4);
			pBits[i].b2.d_h = BlockSignif(DHBand + k + d_stride + 4, stride, 4, 4);

			pBits[i].b3.hv_l = BlockSignif(HVLBand + k + hv_stride, stride, 4, 2);
			pBits[i].b3.hv_h = BlockSignif(HVHBand + k + hv_stride, stride, 4, 2);
			pBits[i].b3.d_l = BlockSignif(DLBand + k + d_stride, stride, 4, 4);
			pBits[i].b3.d_h = BlockSignif(DHBand + k + d_stride, stride, 4, 4);

			if (pChBits) {
				treeBlock tmp;
				tmp.bitField = pChBits[2 * i + 1 + ch_stride].bitField |
						pChBits[2 * i].bitField | pChBits[2 * i + 1].bitField |
						pChBits[2 * i + ch_stride].bitField;
				pBits[i].b0.offspring = tmp.b0.all != 0;
				pBits[i].b1.offspring = tmp.b1.all != 0;
				pBits[i].b2.offspring = tmp.b2.all != 0;
				pBits[i].b3.offspring = tmp.b3.all != 0;
			}
		}
		HVLBand += hv_stride;
		HVHBand += hv_stride;
		DLBand += d_stride;
		DHBand += d_stride;
		pBits += DimX;
		pChBits += ch_stride * 2;
	}
}

void CTree::Fill(CBand * HVHBand, CBand * HVLBand, CBand * DHBand, CBand * DLBand)
{
	CBand * HVH = HVHBand, * HVL = HVLBand, * DH = DHBand, * DL = DLBand;
	do {
		Fill((int*)HVH->pBand, (int*)HVL->pBand, (int*)DH->pBand, (int*)DL->pBand,
				HVL->DimXAlign);
		HVH = HVH->pParent;
		HVL = HVL->pParent;
		DH = DH->pParent;
		DL = DL->pParent;
	} while (HVL != 0);
}

template <cmode mode>
void CTree::Tree(CMuxCodec * pCodec)
{
	treeBlock * pBits = pBitField;

	for( int j = 0; j < DimY; j +=2){
		for( int i = 0, k = 0; i < DimX; i += 2, k += 8){
			if (pBits[i].bitField) {
				TreeCodec.code1(0);
				if (pBits[i].bitField & BLOCK_MASK) {
					BlockCodec.code1(0);
					unsigned int k;
					unsigned int code = (pBits[i].bitField & 0xF) |
							((pBits[i].bitField & 0xF00) >> 4) |
							((pBits[i].bitField & 0xF0000) >> 8) |
							((pBits[i].bitField & 0xF000000) >> 12);
					k = bitcnt(code);
					pCodec->bitsCode(k, 4);
					if (k != 16) pCodec->enum16Code(code, k);
					// coder les bloc à 1
				} else
					BlockCodec.code0(0);
			} else
				TreeCodec.code0(0);

		}
		pBits += DimX;
	}
}

}
