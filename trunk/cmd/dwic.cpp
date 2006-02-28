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

#include <fstream>
#include <iostream>
#include <cstdlib>

#include <unistd.h>
#include <Magick++.h>

#include <libdwic.h>

using namespace std;
using namespace Magick;
using namespace libdwic;

void CompressImage(string & infile, string & outfile, float Quant, float Thres){
	ofstream oFile( outfile.c_str() , ios::out );
	oFile << "DWIC";

	Image img( infile );
	img.type( GrayscaleType );
	float * ImgPixels = new float [img.columns() * img.rows()];
	unsigned char * pStream = new unsigned char[img.columns() * img.rows()];

	img.write(0, 0, img.columns(), img.rows(), "R", FloatPixel, ImgPixels);

	unsigned short tmp = img.columns();
	oFile.write((char *)&tmp, sizeof(unsigned short));
	tmp = img.rows();
	oFile.write((char *)&tmp, sizeof(unsigned short));

	DirWavelet Wavelet(img.columns(), img.rows(), 5);
	Wavelet.SetWeight97();
 	CRangeCodec RangeCodec(0, 0);
 	Wavelet.SetRange(&RangeCodec);

 	Wavelet.Transform97(ImgPixels, img.columns());
	Wavelet.TSUQ(Quant, Thres);

	unsigned char * pEnd = Wavelet.CodeBand(pStream);
	RangeCodec.InitCoder(0, pEnd);
 	Wavelet.CodeMap(0);
	pEnd = RangeCodec.EndCoding();
	oFile.write((char *) pStream, (pEnd - pStream));

	oFile.close();

// 	Wavelet.TSUQi(Quant, 0);
// 	Wavelet.Transform97I(ImgPixels, img.columns());
//
// 	img.read(img.columns(), img.rows(), "R", FloatPixel, ImgPixels);
// 	img.write("./test_encode.png");

	delete[] ImgPixels;
	delete[] pStream;
}

void DecompressImage(string & infile, string & outfile, float Quant,
					 float RecLevel){
	ifstream iFile( infile.c_str() , ios::in );
	char magic[4] = {0,0,0,0};

	iFile.read(magic, 4);

	if (magic[0] != 'D' || magic[1] != 'W' || magic[2]!= 'I' || magic[3] != 'C')
		throw 2;

	unsigned short width, heigth;
	iFile.read((char *) &width, sizeof(unsigned short));
	iFile.read((char *) &heigth, sizeof(unsigned short));

	float * ImgPixels = new float [width * heigth * 3];
	unsigned char * pStream = new unsigned char[width * heigth];

	iFile.read((char *) pStream, width * heigth);

	DirWavelet Wavelet(width, heigth, 5);
	Wavelet.SetWeight97();
 	CRangeCodec RangeCodec(0);
 	Wavelet.SetRange(&RangeCodec);

 	unsigned char * pEnd = Wavelet.DecodeBand(pStream);
 	RangeCodec.InitDecoder(pEnd);
 	Wavelet.DecodeMap(0);

	Wavelet.TSUQi(Quant, RecLevel);
 	Wavelet.Transform97I(ImgPixels, width);

	for( int i = width * heigth - 1, j = width * heigth * 3 - 3; i > 0 ; i--){
		ImgPixels[j] = ImgPixels[j+1] = ImgPixels[j+2] = ImgPixels[i];
		j-=3;
	}
	Image img(width, heigth, "RGB", FloatPixel, ImgPixels);
	img.type( GrayscaleType );
	img.display();

	img.write(outfile);

	delete[] ImgPixels;
	delete[] pStream;
}

int main( int argc, char *argv[] )
{
	int c;
	extern char * optarg;
	extern int optind, opterr;
	char * progname = argv[0];
	string infile;
	string outfile;
	float ThresRatio = 0.75;
	float Quant = 0.05;
	float RecLevelRatio = 0;

	while ((c = getopt(argc , argv, "i:o:q:r:t:")) != -1) {
		switch (c) {
			case 'i':
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'q':
				Quant = atof(optarg);
				break;
			case 'r':
				RecLevelRatio = atof(optarg);
				break;
			case 't':
				ThresRatio = atof(optarg);
				break;
		}
	}
	if (infile.length() == 0) {
		cout << "An input file name must be specified (option -i)" << endl;
		exit(1);
	}

	int mode = 0; // 0 = code , 1 = decode
	int loc;
	if ((loc = infile.rfind(".dwi", string::npos, 4)) != string::npos){
		// mode décodage
		mode = 1;
		if (outfile.length() == 0) {
			outfile = infile;
			outfile.resize(loc);
			outfile.append(".png");
		}
	} else {
		// mode codage
		mode = 0;
		if (outfile.length() == 0) {
			outfile = infile;
			loc = infile.find_last_of(".", string::npos, 5);
			int loc2 = infile.find_last_of("/");
			if (loc != string::npos && loc2 < loc) {
				outfile.resize(loc);
			}
			outfile.append(".dwi");
		}
	}


	if (mode == 0) {
		CompressImage(infile, outfile, Quant, ThresRatio * Quant);
	} else {
		DecompressImage(infile, outfile, Quant, RecLevelRatio * Quant);
	}

	return EXIT_SUCCESS;
}
