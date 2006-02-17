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

#include "rlecodec.h"

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
	unsigned int cnt = count;

	for( int i = 0; i < stride; i++){
		if (pBuffer[i] == 0.) {
			cnt++;
		} else {
			if (cnt != 0) {
				if (nbBits >= 32)
					EmptyBuffer();
				buffer <<= 1;
				nbBits++;
				fiboCode(cnt);
				cnt = 0;
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
	count = cnt;
}

void CRLECodec::RLEDecode(float * pBuffer, int stride)
{
	unsigned int cnt = count;

	for( int i = 0; i < stride; i++){
		if (cnt == (unsigned int) -1){
			cnt = 0;
			if (nbBits < 1)
				FillBuffer();
			nbBits--;
			if (buffer & (1 << nbBits)) {
				pBuffer[i] = (float) -fiboDecode();
			} else {
				pBuffer[i] = (float) fiboDecode();
			}
			continue;
		}
		if (cnt > 0) {
			do {
				pBuffer[i] = 0.;
				i++;
				cnt--;
			} while( i < stride && cnt > 0 ) ;
			if (cnt == 0) {
				if (i < stride){
					if (nbBits < 1)
						FillBuffer();
					nbBits--;
					if (buffer & (1 << nbBits)) {
						pBuffer[i] = (float) -fiboDecode();
					} else {
						pBuffer[i] = (float) fiboDecode();
					}
				} else {
					cnt = (unsigned int) -1;
					break;
				}
			} else {
				break;
			}
		}

		if (nbBits < 1)
			FillBuffer();
		nbBits--;
		if (buffer & (1 << nbBits)) {
			if (nbBits < 1)
				FillBuffer();
			nbBits--;
			if (buffer & (1 << nbBits)) {
				pBuffer[i] = (float) -fiboDecode();
			} else {
				pBuffer[i] = (float) fiboDecode();
			}
		} else {
			cnt = fiboDecode();
			pBuffer[i] = 0.;
		}
	}
	count = cnt;
}

void CRLECodec::fiboCode(unsigned int nb)
{

}

unsigned int CRLECodec::fiboDecode(void)
{

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
	} while( nbBits < 24 );
}

}
