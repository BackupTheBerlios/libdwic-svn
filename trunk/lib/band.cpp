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
#include "bitcodec.h"

#include <math.h>
#include <string.h>
#include <iostream>

using namespace std;

namespace libdwic {

const char log[32] = {0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5};

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

void CBand::Init( unsigned int x, unsigned int y, int Align)
{
	DimX = x;
	DimY = y;
	DimXAlign = ( DimX + Align - 1 ) & ( -Align );
	BandSize = DimXAlign * DimY;
	Weight = 1;
	Count = 0;
	if (BandSize != 0){
		pData = new char[BandSize * sizeof(float) + Align];
		pBand = (flint*)(((int)pData + Align - 1) & (-Align));
	}
}

template <cmode mode>
void CBand::pred(CMuxCodec * pCodec)
{
	int * pCur = (int*) pBand;
	int k = 6;
	const int stride = DimXAlign;

	if (mode == encode)
		pCodec->tabooCode(s2u(pCur[0]));
	else
		pCur[0] = u2s(pCodec->tabooDecode());

	for( int i = 1; i < DimX; i++){
		if (mode == encode)
			pCodec->golombCode(s2u(pCur[i] - pCur[i - 1]), k);
		else
			pCur[i] = pCur[i - 1] + u2s(pCodec->golombDecode(k));
	}
	pCur += stride;

	for( int j = 1; j < DimY; j++){
		if (mode == encode)
			pCodec->golombCode(s2u(pCur[0] - pCur[-stride]), k);
		else
			pCur[0] = pCur[-stride] + u2s(pCodec->golombDecode(k));

		for( int i = 1; i < DimX; i++){
			int var = ABS(pCur[i - 1] - pCur[i - 1 - stride]) +
					ABS(pCur[i - stride] - pCur[i - 1 - stride]);
			var -= var >> 2;
			if (var >= 32)
				var = k;
			else
				var = log[var];
			if (mode == encode) {
				int pred = pCur[i] - pCur[i - 1] - pCur[i - stride] +
						pCur[i - 1 - stride];
				pCodec->golombCode(s2u(pred), var);
			} else
				pCur[i] = pCur[i - 1] + pCur[i - stride] -
						pCur[i - 1 - stride] + u2s(pCodec->golombDecode(var));
		}
		pCur += stride;
	}
}

template void CBand::pred<encode>(CMuxCodec *);
template void CBand::pred<decode>(CMuxCodec *);

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
	if (mode == encode)
		if (Count <= 8){
			pCodec->bitsCode(0, 1);
			return;
		} else
			pCodec->bitsCode(1, 1);
	else
		if (pCodec->bitsDecode(1) == 0)
			return;

	int * pCur = (int*) pBand;
	int diff = DimXAlign << 2;
	unsigned int * pTop = new unsigned int [(DimX >> 2) * 3];
	unsigned int * pOld = pTop + (DimX >> 2);
	unsigned int * pLeft = pOld + (DimX >> 2);

	int l = 0;
	if (mode == encode)
		pOld[l] = enuCode4x4<true>(pCodec, pCur, DimXAlign, 0);
	else
		pOld[l] = enuDecode4x4<true>(pCodec, pCur, DimXAlign, 0);

	pLeft[l] = pOld[l] << 1;

	for( int i = 4; i < DimX; i += 4){
		l++;
		if (mode == encode)
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
		if (mode == encode)
			pOld[l] = enuCode4x4<false>(pCodec, pCur, DimXAlign, pTop[l] / 3);
		else
			pOld[l] = enuDecode4x4<false>(pCodec, pCur, DimXAlign, pTop[l] / 3);

		pLeft[l] = pOld[l] << 1;

		for( int i = 4; i < DimX; i += 4){
			l++;
			if (mode == encode)
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

template void CBand::enu<encode>(CMuxCodec * );
template void CBand::enu<decode>(CMuxCodec * );

// k ~ [ln2(nBloc/(16-nBloc))]
// see Resilient Parameterized Tree Codes for Fast Adaptive Coding
const int CBand::golombK[17] =
	{0, -3, -2, -2, -1, -1, -1, 0, 0, 0, 0, 0, 1, 1, 1, 2, 3};

template <bool directK>
		unsigned int CBand::enuCode4x4(CMuxCodec * pCodec, int * pCur,
									   int stride, unsigned int kPred)
{
	int tmp[16];
	unsigned int signif = 0;
	unsigned int k = 0;

	for( int j = 0; j < 4; j++){
		for( int * pEnd = pCur + 4; pCur < pEnd; pCur++){
			signif <<= 1;
			if (pCur[0] != 0) {
				tmp[k] = pCur[0];
				k++;
				signif |= 1;
			}
		}
		pCur += stride - 4;
	}

	// TODO : virer ce hack misérable et faire une vraie R/D
	if (k == 1 && (unsigned int)(tmp[0] + 1) <= 2 && kPred == 0)
		k = 0;

	if (directK)
		pCodec->bitsCode(k ,5);
	else
		pCodec->encode(pcumProba[kPred][k], pcumProba[kPred][k+1]);

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
		unsigned int CBand::enuDecode4x4(CMuxCodec * pCodec, int * pCur,
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
		for( int * pEnd = pCur + 4; pCur < pEnd; pCur++){
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

#define INSIGNIF_BLOCK 0x80000000


template <bool high_band>
		void CBand::buildTree(void)
{
	int * pCur1 = (int *) pBand;
	int * pCur2 = (int *) pBand + DimXAlign;
	int * pChild1 = 0, * pChild2 = 0;
	unsigned int child_stride = 0;

	if (this->pChild) {
		pChild1 = (int*) this->pChild->pBand;
		pChild2 = pChild1 + 2 * this->pChild->DimXAlign;
		child_stride = this->pChild->DimXAlign * 4;
	}

	for( int j = 0; j < DimY; j += 2){
		for( int i = 0; i < DimX; i += 2){
			if (high_band) {
				if (0 == (pCur1[i] | pCur1[i + 1] | pCur2[i] | pCur2[i + 1]))
					pCur1[i] = INSIGNIF_BLOCK;
			} else {
				if (0 == (pCur1[i] | pCur1[i + 1] | pCur2[i] | pCur2[i + 1]) &&
								INSIGNIF_BLOCK == pChild1[2*i] &&
								INSIGNIF_BLOCK == pChild1[2*i + 2] &&
								INSIGNIF_BLOCK == pChild2[2*i] &&
								INSIGNIF_BLOCK == pChild2[2*i + 2])
					pCur1[i] = INSIGNIF_BLOCK;
			}
		}
		pCur1 += DimXAlign * 2;
		pCur2 += DimXAlign * 2;
		pChild1 += child_stride;
		pChild2 += child_stride;
	}

	if (pParent != 0)
		pParent->buildTree<false>();
}

template void CBand::buildTree<true>(void);

#define CODE_COEF(coef) \
	coef = s2u_(coef); \
	len = bitlen(coef >> 1); \
	for(k = 0; k < len; k++) lenCodec.code0(k); \
	if (k < bits) lenCodec.code1(k); \
	if (len != 0) pCodec->bitsCode(coef & ((1 << len) - 1), len); \
	coef = len;

#define DECODE_COEF(coef) \
	len = 0; coef = 0; \
	while( len < bits && lenCodec.decode(len) == 0 ) len++; \
	if (len != 0) coef = u2s_(pCodec->bitsDecode(len) | (1 << len));

template <cmode mode>
		void CBand::tree(CMuxCodec * pCodec)
{
	int bits;
	if (mode == encode) {
		bits = MAX(Max, -Min);
		bits = bitlen((unsigned int) bits);
		pCodec->tabooCode(bits);
	} else
		bits = pCodec->tabooDecode();

	int * pCur1 = (int*) pBand;
	int * pCur2 = (int*) pBand + DimXAlign;
	int * pPar = 0;
	int diff_par = 0;
	int diff = DimXAlign << 1;

	if (pParent != 0) {
		pPar = (int*) pParent->pBand;
		diff_par = pParent->DimXAlign;
	}

	CBitCodec lenCodec(pCodec);
	CBitCodec treeCodec(pCodec);

	for( int j = 0; j < DimY; j += 2){
		for( int i = 0; i < (DimX >> 1); i++){
			int context = 0;
			if (pPar) context = pPar[i];

			if (context == INSIGNIF_BLOCK) {
				pPar[i] = 0;
				pCur1[2*i] = pCur1[2*i + 1] = pCur2[2*i] = pCur2[2*i + 1] = -(pChild != 0) & INSIGNIF_BLOCK;
				continue;
			}

			if (mode == encode) {
				if (pCur1[i*2] == INSIGNIF_BLOCK) {
					treeCodec.code1(context);
					pCur1[2*i] = pCur1[2*i + 1] = pCur2[2*i] = pCur2[2*i + 1] = -(pChild != 0) & INSIGNIF_BLOCK;
				} else {
					treeCodec.code0(context);
					int len, k;
					CODE_COEF(pCur1[i*2]);
					CODE_COEF(pCur1[i*2 + 1]);
					CODE_COEF(pCur2[i*2]);
					CODE_COEF(pCur2[i*2 + 1]);
				}
			} else {
				context = bitlen(s2u_(context) >> 1);
				if (treeCodec.decode(context)) {
					pCur1[2*i] = pCur1[2*i + 1] = pCur2[2*i] = pCur2[2*i + 1] = -(pChild != 0) & INSIGNIF_BLOCK;
				} else {
					int len;
					DECODE_COEF(pCur1[i*2]);
					DECODE_COEF(pCur1[i*2 + 1]);
					DECODE_COEF(pCur2[i*2]);
					DECODE_COEF(pCur2[i*2 + 1]);
				}
			}
		}
		pCur1 += diff;
		pCur2 += diff;
		pPar += diff_par;
	}
}

template void CBand::tree<encode>(CMuxCodec * );
template void CBand::tree<decode>(CMuxCodec * );

void CBand::GetBand(float * pOut)
{
	memcpy(pOut, pBand, BandSize * sizeof(float));
}

void CBand::SimpleQuant( int quant )
{
	int size = DimXAlign * DimY;
	int add = quant >> 1;
	for ( int i = 0; i < size; i++ ) {
		if ( pBand[i].f > quant )
			pBand[i].f = ( pBand[i].f / quant ) * quant + add;
		else if ( pBand[i].f < -quant )
			pBand[i].f = ( pBand[i].f / quant ) * quant - add;
		else
			pBand[i].f = 0;
	}
}


void CBand::Mean( float & Mean, float & Var )
{
	float Sum = 0;
	float SSum = 0;
	for ( unsigned int j = 0; j < DimY; j++ ) {
		unsigned int J = j * DimXAlign;
		for ( unsigned int i = 0; i < DimX; i++ ) {
			Sum += pBand[i + J].f;
			SSum += pBand[i + J].f * pBand[i + J].f;
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
			if ( pBand[n].f > negThres && pBand[n].f < Thres ) {
				pBand[n].f = 0;
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
	float halfQuant = 0;
	int Min = 0, Max = 0;
	Count = 0;
	for ( int j = 0, n = 0; j < DimY ; j ++ ) {
		for ( int nEnd = n + DimX; n < nEnd ; n++ ) {
			if ( pBand[n].f > negThres && pBand[n].f < Thres ) {
				pBand[n].i = 0;
			} else {
				Count++;
				if ( pBand[n].f > 0 ){
					pBand[n].i = lrintf( pBand[n].f * iQuant + halfQuant);
					if (pBand[n].i > Max)
						Max = pBand[n].i;
				} else {
					pBand[n].i = lrintf( pBand[n].f * iQuant - halfQuant);
					if (pBand[n].i < Min)
						Min = pBand[n].i;
				}
			}
		}
		n += Diff;
	}
	this->Min = Min;
	this->Max = Max;
	return Count;
}

void CBand::TSUQi( float Quant, float RecLevel)
{
	int Diff = DimXAlign - DimX;
	Quant /= Weight;
	RecLevel /= Weight;
	for ( int j = 0, n = 0; j < DimY ; j ++ ) {
		for ( int nEnd = n + DimX; n < nEnd ; n++ ) {
			if (pBand[n].i != 0) {
				pBand[n].f = (float) pBand[n].i * Quant;
				if ( pBand[n].f > 0 )
					pBand[n].f += RecLevel;
				else
					pBand[n].f -= RecLevel;
			}
		}
		n += Diff;
	}
}

void CBand::Add( float val )
{
	for ( int i = 0; i < BandSize; i++ )
		pBand[i].f += val;
}

void CBand::Clear(bool recurse)
{
	memset(pBand, 0, BandSize * sizeof(float));
	if (recurse && pParent != 0)
		pParent->Clear(true);
}

} // namespace libdwic
