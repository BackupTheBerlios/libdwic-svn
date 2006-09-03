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
#include "band.h"

#include <math.h>
#include <string.h>
#include <iostream>

using namespace std;

namespace libdwic {

CBand::CBand( void ):
pData(0)
{
	pParent = 0;
	pChild = 0;
	Init();
}

CBand::~CBand()
{
	delete[] pData;
}

void CBand::Init( unsigned int x, unsigned int y, int Align, bool useTree )
{
	DimX = x;
	DimY = y;
	DimXAlign = ( DimX + Align - 1 ) & ( -Align );
	BandSize = DimXAlign * DimY;
	LstLen = 0;
	pTree = 0;
	pList = 0;
	Weight = 1;
	Count = 0;
	if (BandSize != 0){
		if (useTree){
			pData = new char[BandSize * sizeof(float) +
					(BandSize >> 2) * sizeof(char) + Align];
			pBand = (float*)(((int)pData + Align - 1) & (-Align));
			pTree = (unsigned char *)(pBand + BandSize);
		} else {
			pData = new char[BandSize * sizeof(float) + Align];
			pBand = (float*)(((int)pData + Align - 1) & (-Align));
		}
	}
}

template <cmode mode>
void CBand::bit(CMuxCodec * pCodec)
{
	int max = Max;
	unsigned int bits = 0;

	if (mode == code) {
		while( max > 0 ){
			bits++;
			max >>= 1;
		}
		pCodec->fiboCode(bits);
	} else
		bits = pCodec->fiboDecode();

	float * pCur = pBand;

	for( int j = 0; j < DimY; j++){
		for( int i = 0; i < DimX; i++){
			if (mode == code)
				pCodec->bitsCode((unsigned int)pCur[i], bits);
			else
				pCur[i] = pCodec->bitsDecode(bits);
		}
		pCur += DimXAlign;
	}
}

template void CBand::bit<code>(CMuxCodec *);
template void CBand::bit<decode>(CMuxCodec *);

const unsigned short CBand::cumProba[33][18] =
{
	{ 0, 4055, 4081, 4082, 4083, 4084, 4085, 4086, 4087, 4088, 4089, 4090, 4091, 4092, 4093, 4094, 4095, 4096},
	{ 0, 2456, 3724, 4031, 4078, 4084, 4085, 4086, 4087, 4088, 4089, 4090, 4091, 4092, 4093, 4094, 4095, 4096},
	{ 0, 1454, 3004, 3780, 4022, 4075, 4084, 4086, 4087, 4088, 4089, 4090, 4091, 4092, 4093, 4094, 4095, 4096},
	{ 0, 845, 2244, 3329, 3854, 4031, 4076, 4085, 4087, 4088, 4089, 4090, 4091, 4092, 4093, 4094, 4095, 4096},
	{ 0, 483, 1585, 2766, 3553, 3919, 4045, 4079, 4086, 4088, 4089, 4090, 4091, 4092, 4093, 4094, 4095, 4096},
	{ 0, 270, 1068, 2177, 3135, 3712, 3969, 4057, 4081, 4087, 4089, 4090, 4091, 4092, 4093, 4094, 4095, 4096},
	{ 0, 148, 692, 1633, 2647, 3408, 3830, 4009, 4069, 4085, 4089, 4090, 4091, 4092, 4093, 4094, 4095, 4096},
	{ 0, 80, 433, 1173, 2140, 3020, 3612, 3916, 4038, 4077, 4087, 4090, 4091, 4092, 4093, 4094, 4095, 4096},
	{ 0, 42, 261, 807, 1656, 2576, 3312, 3762, 3977, 4058, 4083, 4089, 4091, 4092, 4093, 4094, 4095, 4096},
	{ 0, 22, 153, 534, 1229, 2113, 2943, 3539, 3873, 4020, 4072, 4087, 4091, 4092, 4093, 4094, 4095, 4096},
	{ 0, 11, 86, 339, 874, 1665, 2527, 3246, 3713, 3952, 4049, 4081, 4090, 4092, 4093, 4094, 4095, 4096},
	{ 0, 6, 47, 207, 596, 1258, 2091, 2891, 3489, 3842, 4007, 4068, 4086, 4091, 4093, 4094, 4095, 4096},
	{ 0, 3, 25, 122, 390, 913, 1665, 2492, 3201, 3680, 3936, 4044, 4080, 4090, 4093, 4094, 4095, 4096},
	{ 0, 2, 14, 70, 245, 634, 1272, 2073, 2856, 3459, 3826, 4002, 4068, 4088, 4093, 4094, 4095, 4096},
	{ 0, 1, 7, 38, 147, 421, 931, 1659, 2467, 3174, 3663, 3930, 4044, 4082, 4092, 4094, 4095, 4096},
	{ 0, 1, 4, 20, 84, 266, 651, 1273, 2056, 2834, 3444, 3821, 4003, 4071, 4090, 4094, 4095, 4096},
	{ 0, 1, 3, 11, 47, 161, 434, 934, 1647, 2449, 3162, 3662, 3935, 4049, 4085, 4093, 4095, 4096},
	{ 0, 1, 2, 6, 25, 93, 275, 652, 1262, 2040, 2823, 3445, 3830, 4012, 4076, 4092, 4095, 4096},
	{ 0, 1, 2, 4, 14, 52, 166, 433, 922, 1629, 2437, 3165, 3675, 3949, 4058, 4089, 4095, 4096},
	{ 0, 1, 2, 3, 8, 28, 94, 270, 637, 1240, 2023, 2824, 3462, 3851, 4026, 4082, 4094, 4096},
	{ 0, 1, 2, 3, 6, 16, 52, 160, 416, 895, 1604, 2431, 3183, 3706, 3974, 4071, 4093, 4096},
	{ 0, 1, 2, 3, 5, 10, 28, 89, 254, 607, 1205, 2005, 2838, 3500, 3889, 4049, 4090, 4096},
	{ 0, 1, 2, 3, 4, 6, 15, 47, 144, 383, 850, 1569, 2431, 3222, 3757, 4010, 4085, 4096},
	{ 0, 1, 2, 3, 4, 5, 9, 24, 76, 223, 557, 1153, 1983, 2867, 3562, 3943, 4074, 4096},
	{ 0, 1, 2, 3, 4, 5, 7, 13, 38, 119, 334, 784, 1520, 2440, 3289, 3835, 4054, 4096},
	{ 0, 1, 2, 3, 4, 5, 6, 9, 19, 58, 180, 484, 1076, 1956, 2923, 3663, 4016, 4096},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 11, 27, 87, 266, 688, 1449, 2463, 3404, 3948, 4096},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 9, 15, 39, 127, 384, 961, 1919, 3028, 3826, 4096},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 17, 51, 177, 543, 1330, 2511, 3613, 4096},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 20, 65, 242, 767, 1852, 3251, 4096},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 21, 74, 316, 1092, 2642, 4096},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 18, 65, 372, 1640, 4096},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 41, 4096}
};

const unsigned short * CBand::pcumProba[33] =
{
	cumProba[0], cumProba[1], cumProba[2], cumProba[3], cumProba[4],
	cumProba[5], cumProba[6], cumProba[7], cumProba[8], cumProba[9],
	cumProba[10], cumProba[11], cumProba[12], cumProba[13], cumProba[14],
	cumProba[15], cumProba[16], cumProba[17], cumProba[18], cumProba[19],
	cumProba[20], cumProba[21], cumProba[22], cumProba[23], cumProba[24],
	cumProba[25], cumProba[26], cumProba[27], cumProba[28], cumProba[29],
	cumProba[30], cumProba[31], cumProba[32]
};

template <cmode mode>
		void CBand::enu(CMuxCodec * pCodec)
{
	if (mode == code)
		if (Count <= 2){
			pCodec->bitsCode(0, 1);
			return;
		} else
			pCodec->bitsCode(1, 1);
	else
		if (pCodec->bitsDecode(1) == 0)
			return;

	float * pCur = pBand;
	int diff = DimXAlign << 2;
	unsigned int * pTop = new unsigned int [(DimX >> 2) * 3];
	unsigned int * pOld = pTop + (DimX >> 2);
	unsigned int * pLeft = pOld + (DimX >> 2);

	int l = 0;
	if (mode == code)
		pOld[l] = enuCode4x4<true>(pCodec, pCur, DimXAlign, 0);
	else
		pOld[l] = enuDecode4x4<true>(pCodec, pCur, DimXAlign, 0);

	pLeft[l] = pOld[l] << 1;

	for( int i = 4; i < DimX; i += 4){
		l++;
		if (mode == code)
			pOld[l] = enuCode4x4<false>(pCodec, pCur + i, DimXAlign,
										pLeft[l-1]);
		else
			pOld[l] = enuDecode4x4<false>(pCodec, pCur + i, DimXAlign,
										  pLeft[l-1]);
		pLeft[l] = pOld[l] + (pLeft[l-1] >> 1);
	}

	unsigned int tmp = pOld[l] << 1;
	pTop[l] = pLeft[l] * 3;
	l--;
	for( ; l >= 0; l--){
		tmp >>= 1;
		pTop[l] = (pLeft[l] + tmp) << 1;
		tmp += pOld[l];
	}

	pCur += diff;

	for( int j = 4; j < DimY; j += 4){
		l = 0;
		if (mode == code)
			pOld[l] = enuCode4x4<false>(pCodec, pCur, DimXAlign, pTop[l] / 3);
		else
			pOld[l] = enuDecode4x4<false>(pCodec, pCur, DimXAlign, pTop[l] / 3);

		pLeft[l] = pOld[l] << 1;

		for( int i = 4; i < DimX; i += 4){
			l++;
			if (mode == code)
				pOld[l] = enuCode4x4<false>(pCodec, pCur + i, DimXAlign,
											(pLeft[l-1] + pTop[l] + 2) >> 2);
			else
				pOld[l] = enuDecode4x4<false>(pCodec, pCur + i, DimXAlign,
											  (pLeft[l-1] + pTop[l] + 2) >> 2);
			pLeft[l] = pOld[l] + (pLeft[l-1] >> 1);
		}

		tmp = pOld[l] << 1;
		pTop[l] = ((pLeft[l] + pTop[l]) >> 1) + pLeft[l];
		l--;
		for( ; l >= 0; l--){
			tmp >>= 1;
			pTop[l] = pLeft[l] + tmp + (pTop[l] >> 1);
			tmp += pOld[l];
		}

		pCur += diff;
	}

	delete[] pTop;
}

template void CBand::enu<code>(CMuxCodec * );
template void CBand::enu<decode>(CMuxCodec * );


const int CBand::golombK[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 3};

template <bool directK>
		unsigned int CBand::enuCode4x4(CMuxCodec * pCodec, float * pCur,
									   int stride, unsigned int kPred)
{
	float tmp[16];
	unsigned int signif = 0;
	unsigned int k = 0;

	for( int j = 0; j < 4; j++){
		for( float * pEnd = pCur + 4; pCur < pEnd; pCur++){
			signif <<= 1;
			if (pCur[0] != 0) {
				tmp[k] = pCur[0];
				k++;
				signif |= 1;
			}
		}
		pCur += stride - 4;
	}

	if (directK)
		pCodec->bitsCode(k ,5);
	else
		pCodec->code(pcumProba[kPred][k], pcumProba[kPred][k+1]);

	if (k != 0) {
		if (k != 16)
			pCodec->enum16Code(signif, k);
		for( int i = 0; i < k; i++){
			if (tmp[i] < 0) {
				pCodec->golombCode((unsigned int)(-tmp[i]-1), golombK[k]);
				pCodec->bitsCode(1,1);
			} else {
				pCodec->golombCode((unsigned int)(tmp[i]-1), golombK[k]);
				pCodec->bitsCode(0,1);
			}
		}
	}

	return k;
}

template <bool directK>
		unsigned int CBand::enuDecode4x4(CMuxCodec * pCodec, float * pCur,
										 int stride, unsigned int kPred)
{
	unsigned int k;
	if (directK)
		k = pCodec->bitsDecode(5);
	else
		k = pCodec->decode(pcumProba[kPred]);

	if (k == 0)
		return k;

	unsigned int signif = 0xFFFF;

	if (k != 16)
		signif = pCodec->enum16Decode(k);

	for( int j = 0; j < 4; j++){
		for( float * pEnd = pCur + 4; pCur < pEnd; pCur++){
			if (signif & (1 << 15)) {
				pCur[0] = pCodec->golombDecode(golombK[k]) + 1;
				if (pCodec->bitsDecode(1))
					pCur[0] = -pCur[0];
			}
			signif <<= 1;
		}
		pCur += stride - 4;
	}

	return k;
}

template <cmode mode>
void CBand::Tree(CMuxCodec * pCodec)
{
	unsigned char * pCurTree = pTree;
	for( int j = 0; j < DimY; j += 2){
		for( int i = 0; i < DimX; i += 2){
			if (mode == code) {
				pCodec->bitsCode(pCurTree[0] & 1, 1);
				if (pCurTree[0]) {
					pCodec->bitsCode(pCurTree[0] >> 1, 1);
					CoefCode(i, j, pCodec);
					if (pCurTree[0] & 2)
						pChild->TreeCode(i, j, pCodec);
				}
			} else {
				if (pCodec->bitsDecode(1)) {
					unsigned int tmp = pCodec->bitsDecode(1);
					CoefDecode(i, j, pCodec);
					if (tmp)
						pChild->TreeDecode(i, j, pCodec);
				}
			}
			pCurTree++;
		}
	}
}

template void CBand::Tree<code>(CMuxCodec * );
template void CBand::Tree<decode>(CMuxCodec * );

void CBand::TreeCode(int i, int j, CMuxCodec * pCodec)
{
	int stride = DimX >> 1;
	unsigned char * pCurTree = pTree + i + j * stride;

	unsigned int tmp = ((pCurTree[0] & 1) << 3) | ((pCurTree[1] & 1) << 2) |
			((pCurTree[stride] & 1) << 1) | (pCurTree[stride+1] & 1);

	pCodec->bitsCode(tmp, 4);

	i <<= 1;
	j <<= 1;

	if (pCurTree[0]) {
		if (pChild) pCodec->bitsCode(pCurTree[0] >> 1, 1);
		CoefCode(i, j, pCodec);
		if (pCurTree[0] & 2)
			pChild->TreeCode(i, j, pCodec);
	}
	pCurTree++;
	i += 2;
	if (pCurTree[0]) {
		if (pChild) pCodec->bitsCode(pCurTree[0] >> 1, 1);
		CoefCode(i, j, pCodec);
		if (pCurTree[0] & 2)
			pChild->TreeCode(i, j, pCodec);
	}
	pCurTree += stride;
	j += 2;
	if (pCurTree[0]) {
		if (pChild) pCodec->bitsCode(pCurTree[0] >> 1, 1);
		CoefCode(i, j, pCodec);
		if (pCurTree[0] & 2)
			pChild->TreeCode(i, j, pCodec);
	}
	pCurTree--;
	i -= 2;
	if (pCurTree[0]) {
		if (pChild) pCodec->bitsCode(pCurTree[0] >> 1, 1);
		CoefCode(i, j, pCodec);
		if (pCurTree[0] & 2)
			pChild->TreeCode(i, j, pCodec);
	}
}

void CBand::TreeDecode(int i, int j, CMuxCodec * pCodec)
{
	unsigned int tmp = pCodec->bitsDecode(4);
	i <<= 1;
	j <<= 1;

	if (tmp & 8) {
		unsigned int tmp = pChild ? pCodec->bitsDecode(1) : 0;
		CoefDecode(i, j, pCodec);
		if (tmp)
			pChild->TreeDecode(i, j, pCodec);
	}
	i += 2;
	if (tmp & 4) {
		unsigned int tmp = pChild ? pCodec->bitsDecode(1) : 0;
		CoefDecode(i, j, pCodec);
		if (tmp)
			pChild->TreeDecode(i, j, pCodec);
	}
	j += 2;
	if (tmp & 1) {
		unsigned int tmp = pChild ? pCodec->bitsDecode(1) : 0;
		CoefDecode(i, j, pCodec);
		if (tmp)
			pChild->TreeDecode(i, j, pCodec);
	}
	i -= 2;
	if (tmp & 2) {
		unsigned int tmp = pChild ? pCodec->bitsDecode(1) : 0;
		CoefDecode(i, j, pCodec);
		if (tmp)
			pChild->TreeDecode(i, j, pCodec);
	}
}

void CBand::CoefCode(int i, int j, CMuxCodec * pCodec)
{
	float * pCur = pBand + i + j * DimXAlign;
	unsigned int signif = 0, sign = 0, count = 0;
	unsigned int tmp[4];

	if (pCur[0] != 0) {
		signif = 1;
		if (pCur[0] < 0) {
			pCur[0] = -pCur[0];
			sign = 1;
		}
		tmp[count] = (unsigned int) pCur[0];
		count++;
	}
	pCur++;
	signif <<= 1;
	if (pCur[0] != 0) {
		signif |= 1;
		sign <<= 1;
		if (pCur[0] < 0) {
			pCur[0] = -pCur[0];
			sign |= 1;
		}
		tmp[count] = (unsigned int) pCur[0];
		count++;
	}
	pCur += DimXAlign;
	signif <<= 1;
	if (pCur[0] != 0) {
		signif |= 1;
		sign <<= 1;
		if (pCur[0] < 0) {
			pCur[0] = -pCur[0];
			sign |= 1;
		}
		tmp[count] = (unsigned int) pCur[0];
		count++;
	}
	pCur--;
	signif <<= 1;
	if (pCur[0] != 0) {
		signif |= 1;
		sign <<= 1;
		if (pCur[0] < 0) {
			pCur[0] = -pCur[0];
			sign |= 1;
		}
		tmp[count] = (unsigned int) pCur[0];
		count++;
	}

	pCodec->bitsCode(signif, 4);
	pCodec->bitsCode(sign, count);
	for( int k = 0; k < count; k++){
		pCodec->fiboCode(tmp[k]);
	}
}

void CBand::CoefDecode(int i, int j, CMuxCodec * pCodec)
{
	float * pCur = pBand + i + (j+1) * DimXAlign;
	unsigned int tmp[4];
	unsigned int signif = pCodec->bitsDecode(4);
	unsigned int count = (signif & 1) + ((signif >> 1) & 1) +
			((signif >> 2) & 1) + (signif >> 3);
	unsigned int sign = pCodec->bitsDecode(count);

	for( unsigned int k = 0; k < count; k++){
		tmp[k] = pCodec->fiboDecode();
	}

	if (signif & 1) {
		count--;
		pCur[0] = tmp[count];
		if (sign & 1)
			pCur[0] = -pCur[0];
		sign >>= 1;
	}
	pCur++;
	signif >>= 1;
	if (signif & 1) {
		count--;
		pCur[0] = tmp[count];
		if (sign & 1)
			pCur[0] = -pCur[0];
		sign >>= 1;
	}
	pCur -= DimXAlign;
	signif >>= 1;
	if (signif & 1) {
		count--;
		pCur[0] = tmp[count];
		if (sign & 1)
			pCur[0] = -pCur[0];
		sign >>= 1;
	}
	pCur--;
	signif >>= 1;
	if (signif & 1) {
		count--;
		pCur[0] = tmp[count];
		if (sign & 1)
			pCur[0] = -pCur[0];
	}
}

void CBand::ListAllPos( void )
{
	SPos Pos;
	LstLen = 0;
	for ( Pos.y = 0; Pos.y < DimY; Pos.y++ ) {
		for ( Pos.x = 0; Pos.x < DimX; Pos.x++ ) {
			pList[ LstLen++ ] = Pos;
		}
	}
}

void CBand::GetBand(float * pOut)
{
	memcpy(pOut, pBand, BandSize * sizeof(float));
}

void CBand::SimpleQuant( int quant )
{
	int size = DimXAlign * DimY;
	int add = quant >> 1;
	for ( int i = 0; i < size; i++ ) {
		if ( pBand[ i ] > quant )
			pBand[ i ] = ( pBand[ i ] / quant ) * quant + add;
		else if ( pBand[ i ] < -quant )
			pBand[ i ] = ( pBand[ i ] / quant ) * quant - add;
		else
			pBand[ i ] = 0;
	}
}


void CBand::Mean( float & Mean, float & Var )
{
	float Sum = 0;
	float SSum = 0;
	for ( unsigned int j = 0; j < DimY; j++ ) {
		unsigned int J = j * DimXAlign;
		for ( unsigned int i = 0; i < DimX; i++ ) {
			Sum += pBand[ i + J ];
			SSum += pBand[ i + J ] * pBand[ i + J ];
		}
	}
	Mean = Sum * Weight / ( DimX * DimY );
	Var = ( ( SSum - Sum * Sum / ( DimX * DimY ) ) / ( DimX * DimY ) )
			* Weight * Weight;
}

unsigned int CBand::Thres( float Thres )
{
	int Diff = DimXAlign - DimX;
	Thres = Thres / Weight;
	float negThres = -Thres;
	Count = 0;
	for ( int j = 0, n = 0; j < DimY ; j ++ ) {
		for ( int nEnd = n + DimX; n < nEnd ; n++ ) {
			if ( pBand[ n ] > negThres && pBand[ n ] < Thres ) {
				pBand[ n ] = 0;
			} else {
				Count++;
			}
		}
		n += Diff;
	}
	return Count;
}

unsigned int CBand::TSUQ( float Quant, float Thres)
{
	int Diff = DimXAlign - DimX;
	float iQuant = Weight / Quant;
	Quant /= Weight;
	Thres /= Weight;
	float negThres = -Thres;
	float halfQuant = Quant * .5;
	float Min = 0, Max = 0;
	Count = 0;
	for ( int j = 0, n = 0; j < DimY ; j ++ ) {
		for ( int nEnd = n + DimX; n < nEnd ; n++ ) {
			if ( pBand[ n ] > negThres && pBand[ n ] < Thres ) {
				pBand[ n ] = 0;
			} else {
				Count++;
				if ( pBand[n] > 0 ){
					pBand[n] = truncf( (pBand[n] + halfQuant) * iQuant );
					if (pBand[n] > Max)
						Max = pBand[n];
				} else {
					pBand[n] = truncf( (pBand[n] - halfQuant) * iQuant );
					if (pBand[n] < Min)
						Min = pBand[n];
				}
			}
		}
		n += Diff;
	}
	this->Min = (int) Min;
	this->Max = (int) Max;
	return Count;
}

void CBand::TSUQi( float Quant, float RecLevel)
{
	int Diff = DimXAlign - DimX;
	Quant /= Weight;
	RecLevel /= Weight;
	for ( int j = 0, n = 0; j < DimY ; j ++ ) {
		for ( int nEnd = n + DimX; n < nEnd ; n++ ) {
			if (pBand[n] != 0) {
				pBand[n] *= Quant;
				if ( pBand[n] > 0 )
					pBand[n] += RecLevel;
				else
					pBand[n] -= RecLevel;
			}
		}
		n += Diff;
	}
}

/**
 * 1 dans l'arbre signifie 'coef signif dans cet arbre'
 * 2 dans l'arbre signifie 'coef signif en dessous des 4 premiers'
 * @param useHighTree
 */
template <bool useHighTree>
void CBand::BuildTree(void)
{
	float * pCur1 = pBand;
	float * pCur2 = pBand + DimXAlign;
	unsigned char * pCurTree = pTree;
	int diff = DimXAlign << 1;
	unsigned char * pCurTree1 = useHighTree ? pChild->pTree : 0;
	unsigned char * pCurTree2 = useHighTree ? pChild->pTree +
			(pChild->DimX >> 1) : 0;
	int diffChild = useHighTree ? pChild->DimX : 0;

	for( int j = 0; j < DimY; j += 2){
		for( int i = 0; i < DimX; i += 2){
			pCurTree[0] = 0;
			if (pCur1[i] != 0 || pCur1[i+1] != 0 || pCur2[i] != 0 ||
						 pCur2[i+1] != 0)
				pCurTree[0] = 1;
			if (useHighTree) {
				if (pCurTree1[i] | pCurTree1[i+1] | pCurTree2[i] |
								pCurTree2[i+1])
					pCurTree[0] = 3;
			}
			pCurTree++;
		}
		pCur1 += diff;
		pCur2 += diff;
		if (useHighTree){
			pCurTree1 += diffChild;
			pCurTree2 += diffChild;
		}
	}

	if (pParent != 0)
		pParent->BuildTree<true>();
}

// force l'instanciation de la fonction (sinon pb à l'édition de lien)
template void CBand::BuildTree<false>(void);

void CBand::Correlation( float * pOut, int x, int y )
{
	memset( pOut, 0, x * y * sizeof( *pOut ) );

	for ( int j = 0; j < DimY - y + 1; j++ ) {
		for ( int i = 0; i < DimX - x + 1; i++ ) {
			int pos = j * DimXAlign + i;
			int centerPos = ( y >> 1 ) * DimXAlign + ( x >> 1 ) + pos;
			for ( int l = pos, lend = pos + y * DimXAlign, CorPos = 0;
			        l < lend ; l += DimXAlign ) {
				for ( int k = l, kend = l + x; k < kend; k++, CorPos++ ) {
					pOut[ CorPos ] += pBand[ k ] * pBand[ centerPos ];
				}
			}
		}
	}

	float Center = pOut[ ( x >> 1 ) + x * ( y >> 1 ) ];
	for ( int i = 0; i < x * y ; i++ ) {
		// 		pOut[i] = sqrt(fabsf(pOut[i] / Center));
		// 		pOut[i] = 0.5 + copysignf(0.5, pOut[i]);
		pOut[ i ] = 0.5 + copysignf( sqrt( fabsf( pOut[ i ] / Center ) * 2 ),
		                             pOut[ i ] );
	}
}

void CBand::Add( float val )
{
	for ( int i = 0; i < BandSize; i++ )
		pBand[ i ] += val;
}

void CBand::Clear(bool recurse)
{
	memset(pBand, 0, BandSize * sizeof(float));
	if (recurse && pParent != 0)
		pParent->Clear(true);
}

} // namespace libdwic
