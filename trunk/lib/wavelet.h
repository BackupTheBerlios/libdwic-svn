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

#include "rlecodec.h"

namespace libdwic {

#define MAX_WAV_LEVEL 5

// #define ALPHA (-1.586134342)
// #define BETA (-0.05298011854)
// #define GAMMA (0.8829110762)
// #define DELTA (0.4435068522)
#define XI 1.149604398


// http://www.ece.vt.edu/fac_support/dspcl/docs/TCASII05.pdf
#define ALPHA (-3./2.)
#define BETA (-1./16.)
#define GAMMA (4./5.)
#define DELTA (15./32.)
// #define XI 1.13137085

/**
@author Nicolas Botti
*/
class CWavelet{
public:
    CWavelet(int stride, int Level);

    ~CWavelet();

	void Trans1D97(float * pIn);
	void Trans1D97I(float * pOut);

	void TSUQ(float Quant, float Thres);
	void TSUQi(float Quant, float Thres);

	void SetWeight97(float baseWeight);
	void SetDirLength(int length);

	void RLECode(CRLECodec * pCodec);
	void RLEDecode(CRLECodec * pCodec);

private:

	float * pData;
	float * pBand[MAX_WAV_LEVEL + 1][2];
	int strides[MAX_WAV_LEVEL + 1][2];
	float weights[MAX_WAV_LEVEL + 1];
	int Levels;
	int totalStride;


	static void Lift1D(float * pBuf, int stride, float Predict, float Update);
	static void Lift1DI(float * pBuf, int stride, float Predict, float Update);
	static void Lazy1D(float * pIn, int stride, float * pOut);
	static void Lazy1DI(float * pIn, int stride, float * pOut);

	static void TSUQ(float * pIn, int stride, float Quant, float Thres);
	static void TSUQi(float * pIn, int stride, float Quant, float RecLevel);

};

}

