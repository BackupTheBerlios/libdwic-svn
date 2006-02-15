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

#include <iostream>
#include <cstdlib>

#include <Magick++.h>
#include <string.h>
#include <libdwic.h>

using namespace std;
using namespace Magick;
using namespace libdwic;

void ProcessImage(string & ImageName, float Quant, float Thres, float RecLevel){
	Image img( ImageName );
	img.type( GrayscaleType );

	float * ImgPixels = new float [img.columns() * img.rows()];
	unsigned char * pStream = new unsigned char[img.columns() * img.rows()];

	img.write(0, 0, img.columns(), img.rows(),
			  "R", FloatPixel, ImgPixels);

 	DirWavelet Wavelet(img.columns(), img.rows(),5);
	CRangeCodec RangeCodec(pStream, 0);

	Wavelet.SetRange(&RangeCodec);
 	Wavelet.Transform97(ImgPixels, img.columns());
	cout << Wavelet.TSUQ(Quant, Thres) << endl;
	Wavelet.Code(0);
	cout << "Taille des Cartes : " << (int)(RangeCodec.EndCoding() - pStream);
	cout << endl;
	RangeCodec.InitDecoder(pStream);
	Wavelet.Decode(0);
	Wavelet.TSUQi(Quant, RecLevel);
 	Wavelet.Transform97I(ImgPixels, img.columns());

 	img.read(img.columns(), img.rows(), "R", FloatPixel, ImgPixels);

 	img.write(ImageName + ".97.png");

	delete[] ImgPixels;
	delete[] pStream;
}

int main( int argc, char *argv[] )
{
// 	cout << "Nombre d'arguments : " << argc << endl;
	if (argc > 1) {
// 		cout << "Nom du programme : " << argv[0] << endl;
// 		cout << "Image à lire : " << argv[1] << endl;
// 		cout << "Lecture de l'image" << endl;
		try {
			string ImgName = argv[1];
			float ThresRatio = atof(argv[3]);
 			float Quant = atof(argv[2]);
 			float RecLevelRatio = atof(argv[4]);
			ProcessImage(ImgName, Quant, ThresRatio * Quant, RecLevelRatio * Quant);
		} catch ( Exception & error_ ) {
			cout << "Exception : " << error_.what() << endl;
			return -1;
		}
	}
	return EXIT_SUCCESS;
}
