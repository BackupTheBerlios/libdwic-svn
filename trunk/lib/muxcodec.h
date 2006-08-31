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

#define FREQ_POWER	12
#define FREQ_COUNT	(1 << FREQ_POWER)
#define HALF_FREQ_COUNT	(1 << (FREQ_POWER - 1))	// FREQ_COUNT / 2

#define MIN_RANGE		0x00800000
#define HALF_MIN_RANGE	0x00400000
#define MAX_RANGE		0x80000000
#define NO_CARRY		(MAX_RANGE - 1)
#define RANGE_BITS		23
#define END_BITS 		(32 - RANGE_BITS)

#define NORMALIZE \
	while (Range <= MIN_RANGE){ \
		LowRange = (LowRange << 8) | (*pStream); \
		pStream++; \
		Range <<= 8; \
}

/**
@author Nicolas Botti
*/
class CMuxCodec{
	private:
		unsigned int Range;
		unsigned int LowRange;
		unsigned char *pStream;
		unsigned char *pInitStream;
		unsigned int CarryBuff;

		void Normalize(void);

	public:
		CMuxCodec(unsigned char *pStream, unsigned char FirstByte);
		CMuxCodec(unsigned char *pStream);
		void InitCoder(unsigned char FirstByte, unsigned char *pStream);
		unsigned char * EndCoding(void);

		void InitDecoder(unsigned char *pStream);

		void inline Code(const unsigned int LowFreq, const unsigned int TopFreq){
			if (Range <= MIN_RANGE)
				Normalize();
			{
				const register unsigned int tmp = Range >> FREQ_POWER;
				LowRange += tmp * LowFreq;
				Range = tmp * (TopFreq - LowFreq);
			}
		}

		void inline Code0(const unsigned int TopFreq){
			if (Range <= MIN_RANGE)
				Normalize();
			Range = (Range >> FREQ_POWER) * TopFreq;
		}

		void inline Code1(const unsigned int LowFreq){
			if (Range <= MIN_RANGE)
				Normalize();
			{
				const register unsigned int tmp = (Range >> FREQ_POWER) * LowFreq;
				LowRange += tmp;
				Range -= tmp;
			}
		}

		void inline CodeBin(const unsigned int Freq, const int bit){
			if (Range <= MIN_RANGE)
				Normalize();
			{
				const register unsigned int tmp = (Range >> FREQ_POWER) * Freq;
				LowRange += tmp & -bit;
				Range = tmp + ((Range-2*tmp) & -bit);
			}
		}

		unsigned int inline Decode(const unsigned short * pFreqs)
		{
			NORMALIZE;
			unsigned short freq = (LowRange/(Range >> FREQ_POWER))&(FREQ_COUNT-1);
			unsigned int i = 1;
			for( ; freq >= pFreqs[i]; i++){}
			i -= 1;
			const register unsigned int tmp = Range >> FREQ_POWER;
			LowRange -= tmp * pFreqs[i];
			Range = tmp * (pFreqs[i+1] - pFreqs[i]);
			return i;
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

		unsigned int inline GetBit(const unsigned int Freq){
			NORMALIZE;
			const register unsigned int tmp = (Range >> FREQ_POWER) * Freq;
			const register int tst = (LowRange < tmp)-1;
			LowRange -= tmp & tst;
			Range = tmp + ((Range - 2*tmp) & tst);
			return -tst;
		}

};

}

