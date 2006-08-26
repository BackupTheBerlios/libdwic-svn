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

#include "rangecodec.h"

namespace libdwic {

CRangeCodec::CRangeCodec(unsigned char *pStream, unsigned char FirstByte){
	InitCoder(FirstByte, pStream);
}

CRangeCodec::CRangeCodec(unsigned char *pStream){
	InitDecoder(pStream);
}

void CRangeCodec::InitCoder(unsigned char FirstByte = 0,
							unsigned char *pOutStream = 0){
	LowRange = 0;
	Range = MAX_RANGE;
	if (pOutStream != 0){
		pStream = pOutStream;
		pInitStream = pOutStream + 1;
		CarryBuff = FirstByte << 1;
	}
}

void CRangeCodec::InitDecoder(unsigned char *pInStream){
	if (pInStream){
		pInitStream = pInStream + 1;
		pStream = pInStream + 1;
		Range = 0x80;							// 1<<7
		LowRange = *pStream;
		pStream++;
	}
}

void CRangeCodec::Normalize(void){
	do{
		CarryBuff = (CarryBuff << 8) + (LowRange >> RANGE_BITS);
		*pStream = (unsigned char) (CarryBuff >> 9);
		if (CarryBuff & 0x20000){
			int i = -1;
			unsigned int tmp = 1;
			do {
				tmp += pStream[i];
				pStream[i] = (unsigned char) tmp;
				tmp >>= 8;
				i--;
			} while (tmp != 0);
		}
		CarryBuff &= 0x1FF;
		pStream++;
		Range <<= 8;
		LowRange = (LowRange << 8) & NO_CARRY;
	} while (Range <= MIN_RANGE);
}

unsigned char * CRangeCodec::EndCoding(void){
	if (Range <= MIN_RANGE)
		Normalize();

	CarryBuff = (CarryBuff << 8) + (LowRange >> RANGE_BITS) + 1;
	*pStream = (unsigned char) (CarryBuff >> 9);
	if (CarryBuff & 0x20000){
		int i = -1;
		unsigned int tmp = 1;
		do {
			tmp += pStream[i];
			pStream[i] = (unsigned char) tmp;
			tmp >>= 8;
			i--;
		} while (tmp != 0);
	}
	pStream++;

	// 	FIXME enlever les 0, à ajouter au décodage
	pStream[0] = (unsigned char) (CarryBuff >> 1);
	pStream[1] = (unsigned char) (CarryBuff << 7);
	pStream[2] = 0;
	pStream[3] = 0;
	pStream += 3;
	return pStream;
}

unsigned int CRangeCodec::GetCurrentSize(void){
	unsigned int tmp = (unsigned int)(pStream - pInitStream);
	if ((Range >> RANGE_BITS) == 0)
		tmp++;
	return tmp;
}

void CRangeCodec::CodeDirect(const unsigned int Value,
							 const unsigned int Shift){
	int i = 1 << (Shift-1);
	do{
		do{
			Range >>= 1;
			register unsigned int tmp = LowRange + Range;
			if (Value & i) LowRange = tmp;

		} while(Range > MIN_RANGE && (i >>= 1));
		if (Range <= MIN_RANGE) Normalize();
	} while(i);
}

unsigned int CRangeCodec::DecodeDirect(const unsigned int Shift){
	unsigned int Value = 0;
	int i = 1 << (Shift-1);
	do{
		NORMALIZE;
		Range >>= 1;
		if (LowRange >= Range){
			LowRange -= Range;
			Value |= i;
		}
	} while(i >>= 1);
	return Value;
}

}
