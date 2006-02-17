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

#pragma once

namespace libdwic {

typedef enum CodingMode {EncodeMode, DecodeMode};

#define FREQ_POWER	12			// total freq count = 2^FREQ_POWER
#define INTERVAL_POWER	4		// FREQ_POWER - 8
#define FREQ_COUNT	4096		// total freq count = FREQ_COUNT = 2^FREQ_POWER
#define HALF_FREQ_COUNT	2048	// FREQ_COUNT / 2

#define MIN_RANGE	0x00800000
#define HALF_MIN_RANGE	0x00400000
#define MAX_RANGE	0x80000000
#define NO_CARRY	0x7FFFFFFF	// MAX_RANGE - 1
#define RANGE_BITS	23
#define END_BITS 9				// 32 - RANGE_BITS

#define HOR_MV_INIT_DEC			HALF_FREQ_COUNT
#define VER_MV_INIT_DEC			HALF_FREQ_COUNT
#define WAV_INIT_DEC			HALF_FREQ_COUNT
#define MEAN_MIN_DEC			64

#define WAV_MIN_DEC				32

#define M_SIGN_MIN_DEC			16
#define H_SIGN_MIN_DEC			16
#define H_SIGNIF_MIN_DEC		16
#define L_SIGNIF_MIN_DEC		16

#define SIGN_MIN_DEC			1
#define SIGNIF_MIN_DEC			1

#define NORMALIZE \
	while (Range <= MIN_RANGE){ \
		LowRange = (LowRange << 8) | (*pStream); \
		pStream++; \
		Range <<= 8; \
	}

/**
@author Nicolas Botti
*/
class CRangeCodec{
private:
	unsigned int Carry;
	unsigned int Range;
	unsigned int LowRange;
	unsigned char *pStream;
	unsigned char *pInitStream;
	unsigned char CarryBuff;
	unsigned char StreamBuff;

	void Normalize(void);

public:
	CRangeCodec(unsigned char *pStream, unsigned char FirstByte);
	CRangeCodec(unsigned char *pStream);
	void InitCoder(unsigned char FirstByte, unsigned char *pStream);
	void CodeDirect(const unsigned int Value, const unsigned int Shift);
	unsigned char * EndCoding(void);
	unsigned int GetCurrentSize(void);

	void InitDecoder(unsigned char *pStream);
	unsigned int DecodeDirect(const unsigned int Shift);

	void inline Code(const unsigned int LowFreq, const unsigned int TopFreq){
		{
			const register unsigned int tmp = Range >> FREQ_POWER;
			LowRange += tmp * LowFreq;
			Range = tmp * (TopFreq - LowFreq);
		}
		if (Range <= MIN_RANGE)
			Normalize();
	}

	void inline Code0(const unsigned int TopFreq){
		Range = (Range >> FREQ_POWER) * TopFreq;
		if (Range <= MIN_RANGE)
			Normalize();
	}

	void inline Code1(const unsigned int LowFreq){
		{
			const register unsigned int tmp = (Range >> FREQ_POWER) * LowFreq;
			LowRange += tmp;
			Range -= tmp;
		}
		if (Range <= MIN_RANGE)
			Normalize();
	}

	void inline CodeBin(const unsigned int Freq, const int bit){
		{
			const register unsigned int tmp = (Range >> FREQ_POWER) * Freq;
			LowRange += tmp & -bit;
			Range = tmp + ((Range-2*tmp) & -bit);
		}
		if (Range <= MIN_RANGE)
			Normalize();
	}

	void inline Code(const unsigned int Freq){
		{
			const register unsigned int tmp = Range >> FREQ_POWER;
			const register unsigned int LowFreq = Freq & 0xFFFF;
			LowRange += tmp * LowFreq;
			Range = tmp * ((Freq >> 16) - LowFreq);
		}
		if (Range <= MIN_RANGE)
			Normalize();
	}

	unsigned int inline GetFreq(void){
		NORMALIZE;
		return (LowRange/(Range >> FREQ_POWER))&(FREQ_COUNT-1);
	}

	void inline Update(const unsigned int LowFreq, const unsigned int TopFreq){
		const register unsigned int tmp = Range >> FREQ_POWER;
		LowRange -= tmp * LowFreq;
		Range = tmp * (TopFreq - LowFreq);
	}

	void inline Update(const unsigned int Freq){
		const register unsigned int tmp = Range >> FREQ_POWER;
		const register unsigned int LowFreq = Freq & 0xFFFF;
		LowRange -= tmp * LowFreq;
		Range = tmp * ((Freq >> 16) - LowFreq);
	}

	unsigned int inline GetBit(const unsigned int Freq){
		NORMALIZE;
		const register unsigned int tmp = (Range >> FREQ_POWER) * Freq;
		const register int tst = (LowRange < tmp)-1;
		LowRange -= tmp & tst;
		Range = tmp + ((Range - 2*tmp) & tst);
		return -tst;
	}

	unsigned char GetFirstByte(void){
		return StreamBuff;
	}
};

}
