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

#include "muxcodec.h"

namespace libdwic {

#define BIT_CONTEXT_NB	16

#if FREQ_COUNT != 4096
#error "mod values calculated for FREQ_COUNT = 4096"
#endif

/**
@author Nicolas Botti
 */
class CBitCodec
{
private:
	static const int mod[][2];

public:
	CBitCodec();
	virtual ~CBitCodec();
	void InitModel(void);

	void inline code0(const unsigned int Context = 0){
		pRange->code0(Freq[Context] >> FREQ_POWER);
		Freq[Context] += mod[Freq[Context] >> (2*FREQ_POWER-6)][0];
	}

	void inline code1(const unsigned int Context = 0){
		pRange->code1(Freq[Context] >> FREQ_POWER);
		Freq[Context] += mod[Freq[Context] >> (2*FREQ_POWER-6)][1];
	}

	void inline code(const unsigned short Symbol, const unsigned int Context = 0){
		pRange->codeBin(Freq[Context] >> FREQ_POWER, Symbol);
		Freq[Context] += mod[Freq[Context] >> (2*FREQ_POWER-6)][Symbol];
	}

	unsigned int inline decode(const unsigned int Context = 0){
		const register unsigned int ret = pRange->getBit(Freq[Context] >> FREQ_POWER);
		Freq[Context] += mod[Freq[Context] >> (2*FREQ_POWER-6)][ret];
		return ret;
	}

	void setRange(CMuxCodec * RangeCodec){ pRange = RangeCodec;}
	CMuxCodec * getRange(void){ return pRange;}

private:
	unsigned int Freq[BIT_CONTEXT_NB];
	CMuxCodec *pRange;
};

}
