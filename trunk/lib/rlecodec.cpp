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

#include <iostream>
#include <cstdlib>
#include "global.h"
#include "rlecodec.h"

using namespace std;

namespace libdwic {

#define TAU	7
#define RES	8
#define CN_MIN	(11585 << (TAU + RES - 15))
#define CN_MAX	(11585 << (TAU + RES - 14))

CRLECodec::CRLECodec(unsigned char * pBuf)
{
	Init(pBuf);
}

CRLECodec::~CRLECodec()
{
}

void CRLECodec::Init(unsigned char * pBuf){
	pStream = pBuf;
	nbBits = 0;
	count = 0;
	Kn = 0;
	Cn = 1 << (TAU + RES - 1 - Kn);
}

void CRLECodec::RLECode(float * pBuffer, int stride)
{
	for( int i = 0; i < stride; i++){
		if (pBuffer[i] == 0.) {
			count++;
		} else {
			if (nbBits >= 31)
				EmptyBuffer();
			if (count != 0) {
				buffer <<= 1;
				nbBits++;
				fiboCode(count);
				if (nbBits >= 32)
					EmptyBuffer();
				int Value = (int) pBuffer[i];
				buffer <<= 1;
				if (Value < 0){
					Value = -Value;
					buffer |= 1;
				}
				nbBits++;
				fiboCode(Value);
				count = 0;
			} else {
				int Value = (int) pBuffer[i];
				buffer = (buffer << 2) | 2;
				if (Value < 0){
					Value = -Value;
					buffer |= 1;
				}
				nbBits += 2;
				fiboCode(Value);
			}
		}
	}
}

void CRLECodec::RLEDecode(float * pBuffer, int stride)
{
	for( int i = 0; i < stride; i++){
		if (count == (unsigned int) -1){
			count = 0;
			if (nbBits < 1)
				FillBuffer();
			nbBits--;
			if (buffer & (1 << nbBits)) {
				pBuffer[i] = - (float) fiboDecode();
			} else {
				pBuffer[i] = (float) fiboDecode();
			}
			continue;
		}

		if (count > 0) {
			do {
				pBuffer[i] = 0.;
				i++;
				count--;
			} while( i < stride && count > 0 ) ;
			if (count == 0) {
				i--;
				count = (unsigned int) -1;
				continue;
			} else {
				break;
			}
		}

		if (nbBits < 2)
			FillBuffer();
		nbBits--;
		if (buffer & (1 << nbBits)) {
			nbBits--;
			if (buffer & (1 << nbBits)) {
				pBuffer[i] = - (float) fiboDecode();
			} else {
				pBuffer[i] = (float) fiboDecode();
			}
		} else {
			count = fiboDecode();
			i--;
		}
	}
}

const unsigned int CRLECodec::nbFibo[32] =
{
	1,
	2,
	3,
	5,
	8,
	13,
	21,
	34,
	55,
	89,
	144,
	233,
	377,
	610,
	987,
	1597,
	2584,
	4181,
	6765,
	10946,
	17711,
	28657,
	46368,
	75025,
	121393,
	196418,
	317811,
	514229,
	832040,
	1346269,
	2178309,
	3524578
};

void CRLECodec::fiboCode(unsigned int nb)
{
	if ( nbBits >= 8 )
		EmptyBuffer();

	int i = 1, t;
	for( ; nbFibo[i] <= nb; i++){
	}
	int l = i + 1;
	i--;
	nb -= nbFibo[i];

	register unsigned int r = 0xC0000000;
	t = i;
	i--;
	while( nb > 0 ){
		i--;
		if (nbFibo[i] <= nb){
			nb -= nbFibo[i];
			r >>= t-i;
			r |= 0x80000000;
			t = i;
			i--;
		}
	}
	buffer = (buffer << l) | (r >> (33 - l + i));
	nbBits += l;
}

unsigned int CRLECodec::fiboDecode(void)
{
	if ( nbBits < 24 )
		FillBuffer();

	unsigned int t = 3 << (nbBits - 2);
	int l = 2;
	while( (buffer & t) != t ){
		t >>= 1;
		l++;
	}
	nbBits -= l;
	l -= 2;
	unsigned int nb = nbFibo[l];
	t = 1 << (nbBits + 2);
	l--;
	while( l > 0 ){
		l--;
		t <<= 1;
		if (buffer & t){
			nb += nbFibo[l];
			t <<= 1;
			l--;
		}
	}
	return nb;
}

const unsigned int CRLECodec::Cnk[16][16] =
{
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{0, 0, 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105},
	{0, 0, 0, 1, 4, 10, 20, 35, 56, 84, 120, 165, 220, 286, 364, 455},
	{0, 0, 0, 0, 1, 5, 15, 35, 70, 126, 210, 330, 495, 715, 1001, 1365},
	{0, 0, 0, 0, 0, 1, 6, 21, 56, 126, 252, 462, 792, 1287, 2002, 3003},
	{0, 0, 0, 0, 0, 0, 1, 7, 28, 84, 210, 462, 924, 1716, 3003, 5005},
	{0, 0, 0, 0, 0, 0, 0, 1, 8, 36, 120, 330, 792, 1716, 3432, 6435},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 9, 45, 165, 495, 1287, 3003, 6435},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 10, 55, 220, 715, 2002, 5005},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 11, 66, 286, 1001, 3003},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 12, 78, 364, 1365},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 13, 91, 455},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 14, 105},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 15},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const unsigned int CRLECodec::enumLenth[] =
{0, 4, 7, 10, 11, 13, 13, 14, 14, 14, 13, 13, 11, 10, 7, 4, 0};

const unsigned int CRLECodec::enumLost[] =
{0, 0, 8, 464, 228, 3824, 184, 4944, 3514, 4944, 184, 3824, 228, 464, 8, 0, 0};

/**
 * Attention : il n'est pas possible de coder 0
 * @param bits
 */
void CRLECodec::enum16Code(unsigned int bits, const unsigned int k)
{
	unsigned int code = 0;
	const unsigned int * C = Cnk[0];
	unsigned int n = 0;
	do {
		if (bits & 1) {
			code += C[n];
			C += 16;
		}
		n++;
		bits >>= 1;
	} while(bits != 0);

	if (code < enumLost[k])
		bitsCode(code, enumLenth[k]-1);
	else
		bitsCode(code + enumLost[k], enumLenth[k]);
}

/**
 * Attention : il n'est pas possible de décoder k = 0 ou k = 16
 * @param k
 * @return
 */
unsigned int CRLECodec::enum16Decode(unsigned int k)
{
	unsigned int n = 15;
	unsigned int code = bitsDecode(enumLenth[k] - 1);
	const unsigned int * C = Cnk[k-1];
	unsigned int bits = 0;

	if (code >= enumLost[k])
		code = ((code << 1) | bitsDecode(1)) - enumLost[k];

	do {
		if (code >= C[n]) {
			bits |= 1 << n;
			code -= C[n];
			C -= 16;
			k--;
		}
		n--;
	} while(k > 0);

	return bits;
}

void CRLECodec::golombCode(unsigned int nb, const unsigned int k)
{
	unsigned int l = (nb >> k) + 1;
	nb &= (1 << k) - 1;

	while ((int)l > (31 - (int)nbBits)){
		if (31 - (int)nbBits >= 0) {
			buffer <<= 31 - nbBits;
			l -= 31 - nbBits;
			nbBits = 31;
		}
		EmptyBuffer();
	}

	buffer <<= l;
	buffer |= 1;
	nbBits += l;

	bitsCode(nb, k);
}

unsigned int CRLECodec::golombDecode(const unsigned int k)
{
	unsigned int l = 0;

	while(0 == (buffer & ((1 << nbBits) - 1))) {
		if (nbBits == 32 && buffer != 0)
			break;
		l += nbBits;
		nbBits = 0;
		FillBuffer();
	}

	while( (buffer & (1 << --nbBits)) == 0 )
		l++;

	unsigned int nb = (l << k) | bitsDecode(k);
	return nb;
}

void CRLECodec::golombCode(unsigned int nb)
{
	golombCode(nb, Kn);
	Cn -= (Cn >> TAU);
	if (nb == 0)
		Cn += 1 << RES;
	if (Cn < (CN_MIN >> Kn))
		Kn++;
	else if (Kn > 0 && Cn > (CN_MAX >> Kn))
		Kn--;
}

unsigned int CRLECodec::golombDecode(void)
{
	unsigned int nb = golombDecode(Kn);
	Cn -= (Cn >> TAU);
	if (nb == 0)
		Cn += 1 << RES;
	if (Cn < (CN_MIN >> Kn))
		Kn++;
	else if (Kn > 0 && Cn > (CN_MAX >> Kn))
		Kn--;
}

void CRLECodec::EmptyBuffer(void)
{
	do {
		nbBits -= 8;
		pStream[0] = (unsigned char) (buffer >> nbBits);
		pStream++;
	} while( nbBits >= 8 );
}

void CRLECodec::FillBuffer(void)
{
	do {
		nbBits += 8;
		buffer = (buffer << 8) | pStream[0];
		pStream++;
	} while( nbBits <= 24 );
}

unsigned char * CRLECodec::EndCoding(void){
	if (count != 0) {
		if (nbBits >= 32)
			EmptyBuffer();
		buffer <<= 1;
		nbBits++;
		fiboCode(count);
		count = 0;
	}
	if ( nbBits >= 8 )
		EmptyBuffer();
	if ( nbBits > 0 ) {
		pStream[0] = (unsigned char) (buffer << (8-nbBits));
		pStream++;
	}
	return pStream;
}

unsigned char * CRLECodec::EndDecoding(void){
	return pStream - (nbBits / 8);
}

}
