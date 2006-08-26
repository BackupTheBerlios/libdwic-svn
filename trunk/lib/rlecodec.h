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

/**
@author Nicolas Botti
*/
class CRLECodec{
public:
	CRLECodec(unsigned char * pBuf);

    ~CRLECodec();

	void Init(unsigned char * pBuf);

	void RLECode(float * pBuffer, int stride);
	void RLEDecode(float * pBuffer, int stride);
	unsigned char * EndCoding(void);
	unsigned char * EndDecoding(void);

	void golombCode(unsigned int nb, const unsigned int k);
	void golombCode(unsigned int nb);
	unsigned int golombDecode(const unsigned int k);
	unsigned int golombDecode(void);

	void fiboCode(unsigned int nb);
	unsigned int fiboDecode(void);

	void enum16Code(unsigned int bits, const unsigned int k);
	unsigned int enum16Decode(unsigned int k);



	void inline bitsCode(unsigned int bits, unsigned int length)
	{
		if (nbBits + length > 32)
			EmptyBuffer();
		buffer = (buffer << length) | bits;
		nbBits += length;
	}

	unsigned int inline bitsDecode(unsigned int length)
	{
		if (nbBits < length)
			FillBuffer();
		nbBits -= length;
		return (buffer >> nbBits) & ((1 << length) - 1);
	}

private:
	unsigned char * pStream;
	unsigned int nbBits;
	unsigned int buffer;
	unsigned int count;
	unsigned int Cn;
	unsigned int Kn;

	void EmptyBuffer(void);
	void FillBuffer(void);

	static const unsigned int nbFibo[32];
	static const unsigned int Cnk[16][16];
	static const unsigned int enumLenth[];
	static const unsigned int enumLost[];
};

}
