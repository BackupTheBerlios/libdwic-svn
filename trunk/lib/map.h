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

#include "bitcodec.h"

namespace libdwic {

typedef union DirValue{
	struct {
		unsigned short H_D1;
		unsigned short V_D2;
		unsigned int Selected;
	};
	unsigned short Values[4];
	short sValues[4];
} DirValue;

/**
@author Nicolas Botti
*/
class CMap{
public:
	CMap(CMap * pHighMap);

    ~CMap();

	void Init(int DimX = 0, int DimY = 0);
	void GetImageDir(float * pBlock, int Stride);
	void GetImageDirDiag(float * pBlock, int Stride);
	void SetSelected(int Sel);

	void SetRange(CRangeCodec * RangeCodec);
	void Order0Code(void);
	void Order0Dec(void);

	unsigned int DimX;		// Width of the map (blocks)
	unsigned int DimY;		// Height of the map (blocks)
	unsigned int ImageX;	// Width of the original image
	unsigned int ImageY;	// Height of the original image
	unsigned int MapSize;	// (DimX * DimY), the band size in blocks
	DirValue * pMap;		// Directional map information

private:

	CMap * pLow, * pHigh;	// Pointers to low and high direction map
	CBitCodec DirCodec;

	static void GetDirBlock(float * pBlock, int Stride, DirValue * Result);
	static void GetDirBlock(float * pBlock, int Stride, DirValue * Result
			, int BitField);
	static void GetDirBlockDiag(float * pBlock, int Stride, DirValue * Result);
	static void GetDirBlockDiag(float * pBlock, int Stride, DirValue * Result
			, int BitField);
};

}

