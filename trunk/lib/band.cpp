/***************************************************************************
 *   Copyright (C) 2006 by Nicolas BOTTI <rududu@laposte.net>              *
 *                                                                         *
 * This software is a computer program whose purpose is to compress        *
 * images.                                                                 *
 *                                                                         *
 * This software is governed by the CeCILL  license under French law and   *
 * abiding by the rules of distribution of free software.  You can  use,   *
 * modify and/ or redistribute the software under the terms of the CeCILL  *
 * license as circulated by CEA, CNRS and INRIA at the following URL       *
 * "http://www.cecill.info".                                               *
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
 * knowledge of the CeCILL license and that you accept its terms.          *
 ***************************************************************************/

#include <math.h>
#include <string.h>

#include "global.h"
#include "band.h"

namespace libdwic {

CBand::CBand( void ):
pData(0)
{
	Init();
}

CBand::~CBand()
{
	delete[] pData;
}

void CBand::Init( unsigned int x, unsigned int y, int Align )
{
	DimX = x;
	DimY = y;
	DimXAlign = ( DimX + Align - 1 ) & ( -Align );
	BandSize = DimXAlign * DimY;
	LstLen = 0;
	pTree = 0;
	pList = 0;
	Weight = 1;
	Count = 0;
	if (BandSize != 0){
		pData = new float[BandSize + Align];
		pBand = (float*)(((int)pData + Align - 1) & (-Align));
	}
}

/******************************************************************************
*																			  *
*			Utilitaires														  *
*																			  *
******************************************************************************/

void CBand::ListAllPos( void )
{
	SPos Pos;
	LstLen = 0;
	for ( Pos.y = 0; Pos.y < DimY; Pos.y++ ) {
		for ( Pos.x = 0; Pos.x < DimX; Pos.x++ ) {
			pList[ LstLen++ ] = Pos;
		}
	}
}

void CBand::SimpleQuant( int quant )
{
	int size = DimXAlign * DimY;
	int add = quant >> 1;
	for ( int i = 0; i < size; i++ ) {
		if ( pBand[ i ] > quant )
			pBand[ i ] = ( pBand[ i ] / quant ) * quant + add;
		else if ( pBand[ i ] < -quant )
			pBand[ i ] = ( pBand[ i ] / quant ) * quant - add;
		else
			pBand[ i ] = 0;
	}
}


void CBand::Mean( float & Mean, float & Var )
{
	float Sum = 0;
	float SSum = 0;
	for ( unsigned int j = 0; j < DimY; j++ ) {
		unsigned int J = j * DimXAlign;
		for ( unsigned int i = 0; i < DimX; i++ ) {
			Sum += pBand[ i + J ];
			SSum += pBand[ i + J ] * pBand[ i + J ];
		}
	}
	Mean = Sum / ( DimX * DimY );
	Var = ( SSum - Sum * Sum / ( DimX * DimY ) ) / ( DimX * DimY );
}

unsigned int CBand::Thres( float Thres )
{
	int Diff = DimXAlign - DimX;
	Thres = Thres / Weight;
	float negThres = -Thres;
	Count = 0;
	for ( int j = 0, n = 0; j < DimY ; j ++ ) {
		for ( int nEnd = n + DimX; n < nEnd ; n++ ) {
			if ( pBand[ n ] > negThres && pBand[ n ] < Thres ) {
				pBand[ n ] = 0;
			} else {
				Count++;
			}
		}
		n += Diff;
	}
	return Count;
}

unsigned int CBand::TSUQ( float Quant, float Thres, float RecLevel )
{
	int Diff = DimXAlign - DimX;
	float iQuant = Weight / Quant;
	Quant /= Weight;
	RecLevel /= Weight;
	Thres /= Weight;
	float negThres = -Thres;
	Count = 0;
	for ( int j = 0, n = 0; j < DimY ; j ++ ) {
		for ( int nEnd = n + DimX; n < nEnd ; n++ ) {
			if ( pBand[ n ] > negThres && pBand[ n ] < Thres ) {
				pBand[ n ] = 0;
			} else {
				Count++;
				if ( pBand[ n ] > 0 )
					pBand[ n ] = truncf( pBand[ n ] * iQuant ) * Quant
					             + RecLevel;
				else
					pBand[ n ] = truncf( pBand[ n ] * iQuant ) * Quant
					             - RecLevel;
			}
		}
		n += Diff;
	}
	return Count;
}

void CBand::Correlation( float * pOut, int x, int y )
{
	memset( pOut, 0, x * y * sizeof( *pOut ) );

	for ( int j = 0; j < DimY - y + 1; j++ ) {
		for ( int i = 0; i < DimX - x + 1; i++ ) {
			int pos = j * DimXAlign + i;
			int centerPos = ( y >> 1 ) * DimXAlign + ( x >> 1 ) + pos;
			for ( int l = pos, lend = pos + y * DimXAlign, CorPos = 0;
			        l < lend ; l += DimXAlign ) {
				for ( int k = l, kend = l + x; k < kend; k++, CorPos++ ) {
					pOut[ CorPos ] += pBand[ k ] * pBand[ centerPos ];
				}
			}
		}
	}

	float Center = pOut[ ( x >> 1 ) + x * ( y >> 1 ) ];
	for ( int i = 0; i < x * y ; i++ ) {
		// 		pOut[i] = sqrt(fabsf(pOut[i] / Center));
		// 		pOut[i] = 0.5 + copysignf(0.5, pOut[i]);
		pOut[ i ] = 0.5 + copysignf( sqrt( fabsf( pOut[ i ] / Center ) * 2 ),
		                             pOut[ i ] );
	}
}

#define	DIR_NB	4

void CBand::GetBlockDir4x4( float * pBlock , float * Dir)
{
	float tmp = 0;
	int jEnd = DimXAlign * 4;
	for ( int j = 0; j < jEnd; j += DimXAlign ) {
		int iEnd = j + 4;
		for ( int i = j; i < iEnd; i++ ) {
			float pred = pBlock[ i ] * 2 - pBlock[ i - 1 ] - pBlock[ i + 1 ];
			tmp += pred * pred;
		}
	}
	Dir[ 0 ] = tmp;

	tmp = 0;
	for ( int j = 0; j < jEnd; j += DimXAlign ) {
		int iEnd = j + 4;
		for ( int i = j; i < iEnd; i++ ) {
			float pred = pBlock[ i ] * 2 - pBlock[ i - DimXAlign ]
			             - pBlock[ i + DimXAlign ];
			tmp += pred * pred;
		}
	}
	Dir[ 2 ] = tmp;

	tmp = 0;
	for ( int j = 0; j < jEnd; j += DimXAlign ) {
		int iEnd = j + 4;
		for ( int i = j; i < iEnd; i++ ) {
			float pred = pBlock[ i ] * 2 - pBlock[ i - DimXAlign + 1 ]
			             - pBlock[ i + DimXAlign - 1 ];
			tmp += pred * pred;
		}
	}
	Dir[ 1 ] = tmp;

	tmp = 0;
	for ( int j = 0; j < jEnd; j += DimXAlign ) {
		int iEnd = j + 4;
		for ( int i = j; i < iEnd; i++ ) {
			float pred = pBlock[ i ] * 2 - pBlock[ i - DimXAlign - 1 ]
			             - pBlock[ i + DimXAlign + 1 ];
			tmp += pred * pred;
		}
	}
	Dir[ 3 ] = tmp;
}

void CBand::MarkBlockDir( float * pBlock, int Dir )
{
#define DIFF 0.1
	switch ( Dir ) {
// 	case DIR_NB - 1:
// 		break;
	case 0:
		pBlock[ DimXAlign ] += DIFF;
		pBlock[ DimXAlign + 1 ] += DIFF;
		pBlock[ DimXAlign + 2 ] += DIFF;
		break;
	case 1:
		pBlock[ DimXAlign * 2 ] += DIFF;
		pBlock[ DimXAlign + 1 ] += DIFF;
		pBlock[ 2 ] += DIFF;
		break;
	case -1:
		pBlock[ DimXAlign * 2 ] -= DIFF;
		pBlock[ DimXAlign + 1 ] -= DIFF;
		pBlock[ 2 ] -= DIFF;
		break;
	case 2:
		pBlock[ 1 ] -= DIFF;
		pBlock[ DimXAlign + 1 ] -= DIFF;
		pBlock[ DimXAlign * 2 + 1 ] -= DIFF;
		break;
	case 3:
		pBlock[ DimXAlign * 2 + 2 ] += DIFF;
		pBlock[ DimXAlign + 1 ] += DIFF;
		pBlock[ 0 ] += DIFF;
		break;
	case -3:
		pBlock[ DimXAlign * 2 + 2 ] -= DIFF;
		pBlock[ DimXAlign + 1 ] -= DIFF;
		pBlock[ 0 ] -= DIFF;
		break;
	}
}

int CBand::FilterDir(float * pDirs)
{
	float Dir[DIR_NB];
	int stride = DimX * DIR_NB;
	float min1 = 1000000, min2 = 1000000;
	int r1, r2;
	for (int i = 0; i < DIR_NB; i++){
		Dir[i] = pDirs[i] * 16 + 1 * (pDirs[i-DIR_NB] + pDirs[i+DIR_NB]
			+ pDirs[i+stride] + pDirs[i-stride]);/* + pDirs[i+stride + DIR_NB]
			+ pDirs[i+stride - DIR_NB] + pDirs[i-stride + DIR_NB]
			+ pDirs[i-stride - DIR_NB];*/
		if (min2 > Dir[i]){
			if (min1 > Dir[i]){
				min1 = Dir[i];
				r1 = i;
			}else{
				min2 = pDirs[i];
				r2 = i;
			}
		}
	}
	if ((r1 & 1) == 1 && r2 == 2)
		r1 = -r1;
	return r1;
}

void CBand::MarkBandDir( void )
{
	float * pDirs = new float[DimX * DimY * DIR_NB];
	for ( int j = DimXAlign * 4, l = DimX * DIR_NB; j < BandSize - DimXAlign *
4;
	        j += DimXAlign * 4, l += DimX * DIR_NB ) {
		int iEnd = j + DimX - 4;
		for ( int i = j + 4, k = l + DIR_NB; i < iEnd; i += 4, k += DIR_NB) {
			GetBlockDir4x4( pBand + i , pDirs + k);
		}
	}
	for ( int j = DimXAlign * 8, l = DimX * DIR_NB * 2; j < BandSize - DimXAlign
		* 8; j += DimXAlign * 4, l += DimX * DIR_NB ) {
		int iEnd = j + DimX - 8;
		for ( int i = j + 8, k = l + DIR_NB * 2; i < iEnd; i += 4, k += DIR_NB){
			MarkBlockDir( pBand + i , FilterDir(pDirs + k));
		}
	}
	delete[] pDirs;
}

void CBand::Add( float val )
{
	for ( int i = 0; i < BandSize; i++ )
		pBand[ i ] += val;
}

} // namespace libdwic
