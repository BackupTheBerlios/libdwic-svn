/***************************************************************************
 *   Copyright (C) 2006-2007 Nicolas BOTTI <rududu@laposte.net>            *
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

typedef struct {
	short rate;		// rate of this tree branch, 0 if the node is a leaf
	int dist;		// distortion associated with this rate
	int refDist;	// reference distortion
} node;


/**
@author Nicolas Botti
*/
class CMap{
public:
	CMap(CMap * pHighMap);

    ~CMap();

	void Init(int DimX = 0, int DimY = 0);
	void GetImageDist(float * pImage1, float * pImage2, int stride);
	void GetImageDist(float * pImage1, float * pImage2,
					  float * pBand1, float * pBand2, int stride);
	void GetImageDistDiag(float * pImage1, float * pImage2, int stride);
	void SetDir(int Sel);
	void GetMap(unsigned char * pOut);
	void GetDist(unsigned char * pOut);

	void SetCodec(CMuxCodec * pCodec);

	void SelectDir(void);

	void BuidNodes(float const lambda);
	void ApplyNodes(void);
	void CodeNodes(void);
	void DecodeNodes(void);

	unsigned int DimX;		// Width of the map (blocks)
	unsigned int DimY;		// Height of the map (blocks)
	unsigned int ImageX;	// Width of the original image
	unsigned int ImageY;	// Height of the original image
	unsigned int MapSize;	// (DimX * DimY), the band size in blocks
	char * pMap;			// Directional map information
	short * pDist;			// Distortion difference

	float weightL;			// Low band weight
	float weightH;			// High band weight

private:

	CMap * pLow, * pHigh;	// Pointers to low and high direction map
	CBitCodec DirCodec;		// Context coder for directions
	CBitCodec NodeCodec;	// Context coder for nodes
	CBitCodec LeafCodec;	// Context coder for nodes direction

	node * pNodes;

	void SetDir(int Dir, int x, int y, int depth);

	void SetDir(int Dir, int x, int y);
	void ApplyNodes(int x, int y);
	void CodeNodes(int x, int y, int context);
	void DecodeNodes(int x, int y, int context);
};

}

