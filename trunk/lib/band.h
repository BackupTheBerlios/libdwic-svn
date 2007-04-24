/***************************************************************************
 *   Copyright (C) 2006-2007 by Nicolas BOTTI <rududu@laposte.net>         *
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

#define LL_BAND		0
#define V_BAND		1
#define H_BAND		2
#define D1_BAND		3	// diag direction = "\"
#define D2_BAND		4	// diag direction = "/"

class CBand
{
public:
	CBand(void);
	virtual ~CBand();

	unsigned int DimX;		// Width of the band (Datas)
	unsigned int DimY;		// Height of the band
	unsigned int DimXAlign;	// Alignement Width (Buffer)
	unsigned int BandSize;	// (DimXAlign * DimY), the band size in SAMPLES
	int Max;				// Max of the band
	int Min;				// Min of the band
	unsigned int Dist;		// Distortion (Square Error)
	unsigned int Count;		// Count of non-zero coeffs
	float Weight;			// Weighting of the band distortion
							// (used for Quant calc and PSNR calc)

	CBand *pParent;			// Parent Band
	CBand *pChild;			// Child Band
	CBand *pNeighbor[3];	// Band neighbors (other component bands
							// that can be used for context modeling)
	float *pBand;			// Band datas

	void Init(unsigned int x = 0, unsigned int y = 0, int Align = ALIGN);

	// Quantification
	unsigned int Thres(float Thres);
	unsigned int TSUQ(float Quant, float Thres);
	void TSUQi( float Quant, float RecLevel);
	void SimpleQuant(int quant);

	// Codage
	template <cmode mode>
			void enu(CMuxCodec * pCodec);
	template <cmode mode>
			void pred(CMuxCodec * pCodec);

	// Statistiques
	void Mean(float & Mean, float & Var);

	// Utilitaires
	void Add(float val);
	void GetBand(float * pOut);
	void Clear(bool recurse = false);

private:
	char * pData;

	// Codage
	void CoefCode(int i, int j, CMuxCodec * pCodec);
	void CoefDecode(int i, int j, CMuxCodec * pCodec);

	template <bool directK>
	static unsigned int enuCode4x4(CMuxCodec * pCodec, float * pCur,
									   int stride, unsigned int kPred);
	template <bool directK>
	static unsigned int enuDecode4x4(CMuxCodec * pCodec, float * pCur,
										 int stride, unsigned int kPred);

	static const unsigned short cumProba[33][18];
	static const unsigned short * pcumProba[33];
	static const int golombK[17];
};

}
