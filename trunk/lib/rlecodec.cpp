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
#include "rlecodec.h"

using namespace std;

namespace libdwic {

CRLECodec::CRLECodec(unsigned char * pBuf)
{
	Init(pBuf);
}

CRLECodec::~CRLECodec()
{
}

void CRLECodec::RLECode(float * pBuffer, int stride)
{
	for( int i = 0; i < stride; i++){
		if (pBuffer[i] == 0.) {
			count++;
		} else {
			if (count != 0) {
				if (nbBits >= 32)
					EmptyBuffer();
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
				if (nbBits >= 31)
					EmptyBuffer();
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

static const unsigned int nbFibo[32] =
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
