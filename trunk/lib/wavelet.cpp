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

#include "wavelet.h"

#include <string.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace libdwic {

CWavelet::CWavelet(int stride, int Level)
{
	pData = new float[stride];
	Levels = Level;
	pBand[0][0] = pData;
	totalStride = stride;
}

CWavelet::~CWavelet()
{
	delete[] pData;
}

void CWavelet::SetDirLength(int length)
{
	pBand[0][1] = pBand[0][0] + length;
	int l1 = length;
	int l2 = totalStride - length;
	for( int i = Levels; i > 0 ; i--){
		strides[i][0] = l1;
		l1 = (l1 + 1) >> 1;
		strides[i][1] = l2;
		l2 = (l2 + 1) >> 1;
		pBand[i][0] = pBand[0][0] + l1;
		pBand[i][1] = pBand[0][1] + l2;
	}
	strides[0][0] = l1;
	strides[0][1] = l2;
}

/**
 * Set the quantization weights
 * @param baseWeight weight of the coef before this wavelet transform
 */
void CWavelet::SetWeight97(float baseWeight)
{
	weights[Levels] = baseWeight/XI;
	for( int i = Levels - 1; i > 0 ; i--){
		weights[i] = weights[i+1] * XI;
	}
	weights[0] = weights[1] * XI * XI;
}

/**
 * 1D lifting steps (predict and update) of a buffer
 * @param pBuf input buffer
 * @param stride buffer length
 * @param Predict lifting predict coef
 * @param Update lifting update coef
 */
void CWavelet::Lift1D(float * pBuf, int stride, float Predict, float Update)
{
	pBuf[1] += (pBuf[0] + pBuf[2]) * Predict;
	pBuf[0] += pBuf[1] * 2 * Update;
	int i = 2;
	for( ; i < stride - 2; i+=2 ){
		pBuf[i+1] += (pBuf[i] + pBuf[i+2]) * Predict;
		pBuf[i] += (pBuf[i-1] + pBuf[i+1]) * Update;
	}
	if (stride & 1){
		pBuf[i] += pBuf[i-1] * 2 * Update;
	} else {
		pBuf[i+1] += pBuf[i] * 2 * Predict;
		pBuf[i] += (pBuf[i-1] + pBuf[i+1]) * Update;
	}
}

/**
 * 1D inverse lifting steps (update and predict), inverse the Lift1D function
 * @param pBuf input buffer
 * @param stride buffer length
 * @param Predict lifting predict coef (-Predict to inverse Lift1D)
 * @param Update lifting update coef (-Update to inverse Lift1D)
 */
void CWavelet::Lift1DI(float * pBuf, int stride, float Predict, float Update)
{
	pBuf[0] += pBuf[1] * 2 * Update;
	int i = 1;
	for( ; i < stride - 2; i+=2 ){
		pBuf[i+1] += (pBuf[i] + pBuf[i+2]) * Update;
		pBuf[i] += (pBuf[i-1] + pBuf[i+1]) * Predict;
	}
	if (stride & 1){
		pBuf[i+1] += pBuf[i] * 2 * Update;
		pBuf[i] += (pBuf[i-1] + pBuf[i+1]) * Predict;
	} else {
		pBuf[i] += pBuf[i-1] * 2 * Predict;
	}
}

void CWavelet::Lazy1D(float * pIn, int stride, float * pOut)
{
	int i = 0;
	int iEnd = stride >> 1;
	for( ; i < iEnd ; i++){
		pOut[i] = pIn[2 * i + 1];
		pIn[i] = pIn[2 * i];
	}
	if (stride & 1){
		pIn[i] = pIn[2 * i];
		i++;
	}
}

void CWavelet::Lazy1DI(float * pIn, int stride, float * pOut)
{
	int i = (stride + 1) >> 1;
	i--;

	if (stride & 1){
		pIn[2 * i] = pIn[i];
		i--;
	}
	for( ; i >= 0 ; i--){
		pIn[2 * i] = pIn[i];
		pIn[2 * i + 1] = pOut[i];
	}
}

void CWavelet::Trans1D97(float * pIn)
{
	for( int j = 0; j <= 1; j++){
		int i = Levels;
		for( ; i > 0; i--){
			if (strides[i][j] < 5)
				break;
			Lift1D(pIn, strides[i][j], ALPHA, BETA);
			Lift1D(pIn, strides[i][j], GAMMA, DELTA);
			Lazy1D(pIn, strides[i][j], pBand[i][j]);
		}
		memcpy(pBand[0][j], pIn, strides[i][j] * sizeof(float));
		pIn += strides[Levels][j];
	}
}

void CWavelet::Trans1D97I(float * pIn)
{
	for( int j = 0; j <= 1; j++){
		int i = 1;
		while( i <= Levels && strides[i][j] < 5 )
			i++;
		memcpy(pIn, pBand[0][j], strides[i-1][j] * sizeof(float));
		for( ; i <= Levels; i++){
			Lazy1DI(pIn, strides[i][j], pBand[i][j]);
			Lift1DI(pIn, strides[i][j], -GAMMA, -DELTA);
			Lift1DI(pIn, strides[i][j], -ALPHA, -BETA);
		}
		pIn += strides[Levels][j];
	}
}

void CWavelet::TSUQ(float * pIn, int stride, float Quant, float Thres)
{
	float iQuant = 1 / Quant;
	float negThres = -Thres;
	float halfQuant = Quant * .5;
	for ( int n = 0; n < stride ; n++ ) {
		if ( pIn[ n ] > negThres && pIn[ n ] < Thres ) {
			pIn[ n ] = 0;
		} else {
			if ( pIn[n] > 0 ){
				pIn[n] = truncf( (pIn[n] + halfQuant) * iQuant );
			} else {
				pIn[n] = truncf( (pIn[n] - halfQuant) * iQuant );
			}
		}
	}
}

void CWavelet::TSUQi(float * pIn, int stride, float Quant, float RecLevel)
{
	for ( int n = 0; n < stride ; n++ ) {
		if (pIn[n] != 0) {
			pIn[n] *= Quant;
			if ( pIn[n] > 0 )
				pIn[n] += RecLevel;
			else
				pIn[n] -= RecLevel;
		}
	}
}

void CWavelet::TSUQ(float Quant, float Thres)
{
	for( int j = 0; j <= 1; j++){
		for( int i = Levels; i > 0; i--){
			TSUQ(pBand[i][j], strides[i][j] >> 1, Quant / weights[i],
				 Thres / weights[i]);
		}
		TSUQ(pBand[0][j], strides[0][j], Quant / weights[0],
			 Thres / weights[0]);
	}
}

void CWavelet::TSUQi(float Quant, float RecLevel)
{
	for( int j = 0; j <= 1; j++){
		for( int i = Levels; i > 0; i--){
			TSUQi(pBand[i][j], strides[i][j] >> 1, Quant / weights[i],
				  RecLevel / weights[i]);
		}
		TSUQi(pBand[0][j], strides[0][j], Quant / weights[0],
			  RecLevel / weights[0]);
	}
}

void CWavelet::Mean(void)
{
	for( int j = 0; j <= 1; j++){
		for( int i = Levels; i > 0; i--){
			Mean(pBand[i][j], strides[i][j] >> 1, weights[i]);
		}
		Mean(pBand[0][j], strides[0][j], weights[0]);
		cout << endl;
	}
}

void CWavelet::Mean(float * pIn, int stride, float Weight)
{
	float Sum = 0;
	float SSum = 0;
	for( int i = 0; i < stride; i++){
		Sum += pIn[ i ];
		SSum += pIn[ i ] * pIn[ i ];
	}
	float Mean = Sum * Weight / ( stride );
	float Var = ( ( SSum - Sum * Sum / ( stride ) ) / ( stride ) )
			* Weight * Weight;
	cout << Mean << "\t" << Var << " (" << stride << ")" << endl;
}

void CWavelet::RLECode(CRLECodec * pCodec)
{
	for( int j = Levels; j > 0; j--){
		pCodec->RLECode(pBand[j][0], strides[j][0] >> 1);
		pCodec->RLECode(pBand[j][1], strides[j][1] >> 1);
	}
	pCodec->RLECode(pBand[0][0], strides[0][0]);
	pCodec->RLECode(pBand[0][1], strides[0][1]);
}

void CWavelet::RLEDecode(CRLECodec * pCodec)
{
	for( int j = Levels; j > 0; j--){
		pCodec->RLEDecode(pBand[j][0], strides[j][0] >> 1);
		pCodec->RLEDecode(pBand[j][1], strides[j][1] >> 1);
	}
	pCodec->RLEDecode(pBand[0][0], strides[0][0]);
	pCodec->RLEDecode(pBand[0][1], strides[0][1]);
}

}
