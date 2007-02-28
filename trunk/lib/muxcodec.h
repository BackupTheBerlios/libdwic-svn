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
#define FREQ_MASK	((1 << FREQ_POWER) - 1)
#define HALF_FREQ_COUNT	(1 << (FREQ_POWER - 1))	// FREQ_COUNT / 2

#define RANGE_BITS		12
#define MIN_RANGE		(1 << RANGE_BITS)

#define ROT_BUF_SIZE 	4
#define ROT_BUF_MASK	(ROT_BUF_SIZE - 1)

#define REG_SIZE (sizeof(unsigned int) * 8)

/**
@author Nicolas Botti
 * This class is a carryless range coder whose output is multiplexed with the
 * bits sent to bitsCode(). The decoder do the demultiplexing when bits are
 * requested. This allow to use a range coder and to do fast bit input / output.
*/
class CMuxCodec{
	private:
		unsigned char *pStream;
		unsigned char *pInitStream;

		// variables for the range coder
		unsigned int range;
		unsigned int low;
		unsigned int code; // only for the decoder
		unsigned char *pLast[ROT_BUF_SIZE]; // only for the encoder
		unsigned int outCount; // only for the encoder

		void normalize_enc(void);
		void normalize_dec(void);

		// variables for bit output
		unsigned int nbBits;
		unsigned int buffer;
		unsigned char * pReserved;

		// Taboo coding
		unsigned int nbTaboo[REG_SIZE];
		unsigned int sumTaboo[REG_SIZE];
		unsigned int nTaboo;

		static const unsigned int nbFibo[32];
		static const unsigned int Cnk[16][16];
		static const unsigned int enumLenth[];
		static const unsigned int enumLost[];

		void emptyBuffer(void);
		template <bool end> void flushBuffer(void);
		void fillBuffer(const unsigned int length);

	public:
		CMuxCodec(unsigned char *pStream, unsigned short firstWord);
		CMuxCodec(unsigned char *pStream);
		void initCoder(unsigned short firstWord, unsigned char *pStream);
		void initDecoder(unsigned char *pStream);
		unsigned char * endCoding(void);

		void golombCode(unsigned int nb, const int k);
		unsigned int golombDecode(const int k);

		void fiboCode(unsigned int nb);
		unsigned int fiboDecode(void);

		void initTaboo(unsigned int k);
		void tabooCode(unsigned int nb);
		unsigned int tabooDecode(void);

		void enum16Code(unsigned int bits, const unsigned int k);
		unsigned int enum16Decode(unsigned int k);

		void inline encode(const unsigned int lowFreq, const unsigned int topFreq)
		{
			if (range <= MIN_RANGE)
				normalize_enc();
			unsigned int tmp = range * lowFreq;
			low += tmp >> FREQ_POWER;
			range = (range * (topFreq - lowFreq) + (tmp & FREQ_MASK)) >> FREQ_POWER;
		}

		void inline code0(const unsigned int topFreq)
		{
			if (range <= MIN_RANGE)
				normalize_enc();
			range = (range * topFreq) >> FREQ_POWER;
		}

		void inline code1(const unsigned int lowFreq)
		{
			if (range <= MIN_RANGE)
				normalize_enc();
			const unsigned int tmp = (range * lowFreq) >> FREQ_POWER;
			low += tmp;
			range -= tmp;
		}

		void inline codeBin(const unsigned int freq, const int bit)
		{
			if (range <= MIN_RANGE)
				normalize_enc();
			const unsigned int tmp = (range * freq) >> FREQ_POWER;
 			low += tmp & -bit;
 			range = tmp + ((range - 2 * tmp) & -bit);
		}

		void inline codeSkew(const unsigned int shift, const unsigned int bit)
		{
			if (range <= MIN_RANGE)
				normalize_enc();
			const unsigned int tmp = range - (range >> shift);
			low += tmp & -bit;
			range = tmp + ((range - 2 * tmp) & -bit);
		}

		void inline codeSkew0(const unsigned int shift)
		{
			if (range <= MIN_RANGE)
				normalize_enc();
			range -= range >> shift;
		}

		void inline codeSkew1(const unsigned int shift)
		{
			if (range <= MIN_RANGE)
				normalize_enc();
			const unsigned int tmp = range - (range >> shift);
			low += tmp;
			range -= tmp;
		}

		unsigned int inline decode(const unsigned short * pFreqs)
		{
			if (range <= MIN_RANGE)
				normalize_dec();
			unsigned short freq = ((low << FREQ_POWER) + FREQ_MASK) / range;
			unsigned int i = 1;
			while(freq >= pFreqs[i]) i++;
			i--;

			unsigned int tmp = range * pFreqs[i];
			low -= tmp >> FREQ_POWER;
			range = (range * (pFreqs[i+1] - pFreqs[i]) + (tmp & FREQ_MASK)) >> FREQ_POWER;
			return i;
		}

		unsigned int inline getBit(const unsigned int freq)
		{
			if (range <= MIN_RANGE) normalize_dec();
			const unsigned int tmp = (range * freq) >> FREQ_POWER;
			const int tst = (low < tmp) - 1;
			low -= tmp & tst;
			range = tmp + ((range - 2*tmp) & tst);
			return -tst;
		}

		unsigned int inline decSkew(const unsigned int shift)
		{
			if (range <= MIN_RANGE) normalize_dec();
			const unsigned int tmp = range - (range >> shift);
			const int tst = (low < tmp) - 1;
			low -= tmp & tst;
			range = tmp + ((range - 2*tmp) & tst);
			return -tst;
		}

		void inline bitsCode(unsigned int bits, unsigned int length)
		{
			if (nbBits + length > REG_SIZE)
				emptyBuffer();
			buffer = (buffer << length) | bits;
			nbBits += length;
		}

		unsigned int inline bitsDecode(unsigned int length)
		{
			if (nbBits < length)
				fillBuffer(length);
			nbBits -= length;
			return (buffer >> nbBits) & ((1 << length) - 1);
		}

};

}

