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
	Carry = 0;
	if (pOutStream != 0){
		pStream = pOutStream;
		pInitStream = pOutStream + 1;
		CarryBuff = FirstByte << 1;
		StreamBuff = FirstByte & 0x80;
	}
}

void CRangeCodec::InitDecoder(unsigned char *pInStream){
	if (pInStream){
		StreamBuff = *pInStream;
		pInitStream = pInStream + 1;
		pStream = pInStream + 1;
		Range = 0x80;							// 1<<7
		LowRange = *pStream;
		pStream++;
	}
}

void CRangeCodec::Normalize(void){
	do{
		if (LowRange < 0x7F800000){
			*pStream = StreamBuff | (CarryBuff >> 1);
			StreamBuff = CarryBuff << 7;
			pStream++;
			while(Carry){
				Carry--;
				*pStream = StreamBuff | 0x7F;
				StreamBuff = 0x80;
				pStream++;
			}
			CarryBuff = (unsigned char)(LowRange >> RANGE_BITS);
		}
		else if (LowRange & MAX_RANGE){
			*pStream = StreamBuff | ((CarryBuff + 1) >> 1);
			StreamBuff = (CarryBuff + 1) << 7;
			pStream++;
			while (Carry){
				Carry--;
				*pStream = StreamBuff;
				StreamBuff = 0x00;
				pStream++;
			}
			CarryBuff = (unsigned char)(LowRange >> RANGE_BITS);
		}
		else
			Carry++;

		Range <<= 8;
		LowRange = (LowRange << 8) & NO_CARRY;
	} while (Range <= MIN_RANGE);
}

unsigned char * CRangeCodec::EndCoding(void){
	if (Range <= MIN_RANGE)
		Normalize();

	unsigned int tmp = (LowRange >> RANGE_BITS) + 1;

	if (tmp > 0xff){
		*pStream = StreamBuff | ((CarryBuff + 1) >> 1);
		StreamBuff = (CarryBuff + 1) << 7;
		pStream++;
		while (Carry){
			Carry--;
			*pStream = StreamBuff;
			StreamBuff = 0x00;
			pStream++;
		}
	}
	else{
		*pStream = StreamBuff | (CarryBuff >> 1);
		StreamBuff = CarryBuff << 7;
		pStream++;
		while(Carry){
			Carry--;
			*pStream = StreamBuff | 0x7F;
			StreamBuff = 0x80;
			pStream++;
		}
	}
	pStream[0] = StreamBuff | (((unsigned char)tmp) >> 1);
	pStream[1] = (unsigned char)(tmp << 7);
	pStream += 2;
	pStream[0] = 0;
	pStream[1] = 0;
	return pStream;
}

unsigned int CRangeCodec::GetCurrentSize(void){
	return (unsigned int)(pStream - pInitStream);
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
