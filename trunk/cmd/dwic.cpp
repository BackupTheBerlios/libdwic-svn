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

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <math.h>

#include <unistd.h>
#include <Magick++.h>

#include <libdwic.h>

using namespace std;
using namespace Magick;
using namespace libdwic;

#define BAD_MAGIC		2
#define UNKNOW_TYPE		3

template <class Pxl>
void BW2RGB(Pxl * pIn, int stride, Pxl offset = 0, Pxl scale = 1)
{
	for( int i = stride - 1, j = (stride - 1) * 3; i > 0 ; i--){
		pIn[j] = pIn[j+1] = pIn[j+2] = pIn[i] * scale + offset;
		j-=3;
	}
}

void FLOAT2CHAR(float * pIn, int size)
{
	unsigned char * pOut = (unsigned char *) pIn;
	for( int i = 0; i < size ; i++){
		pIn[i] = rintf((pIn[i] + 0.5) * 255);
		pOut[i] = (unsigned char) CLIP(pIn[i], 0, 255);
	}
}

void FLOAT2CHAR(float * pIn, int width, int heigth)
{
	unsigned char * pOut = (unsigned char *) pIn;
	for( int j = 0; j < heigth - 1; j++){
		pIn[0] = rintf((pIn[0] + 0.5) * 255);
		pOut[0] = (unsigned char) CLIP(pIn[0], 0, 255);
		for( int i = 1; i < width - 1; i++){
			float tmp = pIn[i] + 0.5;
			pIn[i] = rintf(tmp * 255);
			tmp -= pIn[i] * (1./255);
			pIn[i+1] += tmp * (7./16);
			pIn[i+width - 1] += tmp * (3./16);
			pIn[i+width] += tmp * (5./16);
			pIn[i+width + 1] += tmp * (1./16);
			pOut[i] = (unsigned char) CLIP(pIn[i], 0, 255);
		}
		pIn += width;
		pOut += width;
		pIn[-1] = rintf((pIn[-1] + 0.5) * 255);
		pOut[-1] = (unsigned char) CLIP(pIn[-1], 0, 255);
	}
	for( int i = 0; i < width; i++){
		pIn[i] = rintf((pIn[i] + 0.5) * 255);
		pOut[i] = (unsigned char) CLIP(pIn[i], 0, 255);
	}
}

void Map2PNG(CWaveletDir & Wavelet, string & outfile)
{

	int MapDimX = Wavelet.GetDimX() >> 1;
	int MapDimY = Wavelet.GetDimY() >> 1;
	unsigned char * pMap = new unsigned char [MapDimX * MapDimY * 3];
	for( int i = 5; i > 0; i--){
		Wavelet.GetMap(pMap, i, 0);
		BW2RGB(pMap, MapDimX * MapDimY);
		Image map1(MapDimX, MapDimY, "RGB", CharPixel, pMap);
		map1.type( GrayscaleType );
		map1.normalize();
		char tmp[8];
		sprintf(tmp, "MapHV%i.png", i);
		map1.write(outfile + tmp);
		Wavelet.GetMap(pMap, i, 1);
		BW2RGB(pMap, MapDimX * MapDimY);
		Image map2(MapDimX, MapDimY, "RGB", CharPixel, pMap);
		map2.type( GrayscaleType );
		map2.normalize();
		sprintf(tmp, "MapD%i.png", i);
		map2.write(outfile + tmp);
		MapDimX >>= 1;
		MapDimY >>= 1;
	}
	delete[] pMap;
}

void Dist2PNG(CWaveletDir & Wavelet, string & outfile)
{

	int MapDimX = Wavelet.GetDimX() >> 1;
	int MapDimY = Wavelet.GetDimY() >> 1;
	unsigned char * pMap = new unsigned char [MapDimX * MapDimY * 3];
	for( int i = 5; i > 0; i--){
		Wavelet.GetDist(pMap, i, 0);
		BW2RGB(pMap, MapDimX * MapDimY);
		Image map1(MapDimX, MapDimY, "RGB", CharPixel, pMap);
		map1.type( GrayscaleType );
		char tmp[8];
		sprintf(tmp, "HV%i.png", i);
		map1.write(outfile + tmp);
		Wavelet.GetDist(pMap, i, 1);
		BW2RGB(pMap, MapDimX * MapDimY);
		Image map2(MapDimX, MapDimY, "RGB", CharPixel, pMap);
		map2.type( GrayscaleType );
		sprintf(tmp, "D%i.png", i);
		map2.write(outfile + tmp);
		MapDimX >>= 1;
		MapDimY >>= 1;
	}
	delete[] pMap;
}

void Band2PNG(CWaveletDir & Wavelet, string & outfile)
{

	int BandDimX = Wavelet.GetDimX() >> 1;
	int BandDimY = Wavelet.GetDimY() >> 1;
	float * pBand = new float [BandDimX * BandDimY * 3];
	for( int i = 5; i > 0; i--){
		char tmp[8];
		{
			Wavelet.GetBand(pBand, i, 0);
			BW2RGB(pBand, BandDimX * BandDimY, 0.5f, 1.f/255);
			Image Band(BandDimX, BandDimY, "RGB", FloatPixel, pBand);
			Band.type( GrayscaleType );
			sprintf(tmp, "HV%i.png", i);
			Band.write(outfile + tmp);
		}
		{
			Wavelet.GetBand(pBand, i, 1);
			BW2RGB(pBand, BandDimX * (BandDimY >> 1), 0.5f, 1.f/255);
			Image Band(BandDimX, BandDimY >> 1, "RGB", FloatPixel, pBand);
			Band.type( GrayscaleType );
			sprintf(tmp, "HVL%i.png", i);
			Band.write(outfile + tmp);
		}
		{
			Wavelet.GetBand(pBand, i, 2);
			BW2RGB(pBand, BandDimX * (BandDimY >> 1), 0.5f, 1.f/255);
			Image Band(BandDimX, BandDimY >> 1, "RGB", FloatPixel, pBand);
			Band.type( GrayscaleType );
			sprintf(tmp, "HVH%i.png", i);
			Band.write(outfile + tmp);
		}
		{
			Wavelet.GetBand(pBand, i, 3);
			BW2RGB(pBand, BandDimX * BandDimY, 0.5f, 1.f/255);
			Image Band(BandDimX, BandDimY, "RGB", FloatPixel, pBand);
			Band.type( GrayscaleType );
			sprintf(tmp, "DL%i.png", i);
			Band.write(outfile + tmp);
		}
		{
			Wavelet.GetBand(pBand, i, 4);
			BW2RGB(pBand, BandDimX * BandDimY, 0.5f, 1.f/255);
			Image Band(BandDimX, BandDimY, "RGB", FloatPixel, pBand);
			Band.type( GrayscaleType );
			sprintf(tmp, "DH%i.png", i);
			Band.write(outfile + tmp);
		}

		BandDimX >>= 1;
		BandDimY >>= 1;
	}
	delete[] pBand;
}

typedef union {
	struct  {
		unsigned char Quant	:5;
		unsigned char Type	:3;
	};
	char last;
} Header;

#define WAV_LEVELS 5
#define TABOO_LEN 2

void CompressImage(string & infile, string & outfile, int Quant, float Thres,
				   int Type)
{

	Header Head;
	Head.Quant = Quant;
	Head.Type = Type;

	ofstream oFile( outfile.c_str() , ios::out );
	oFile << "DWIC";

	Image img( infile );
	img.type( GrayscaleType );
	unsigned int imSize = img.columns() * img.rows();
	float * ImgPixels = new float [imSize];
	unsigned char * pStream = new unsigned char[imSize];

	img.write(0, 0, img.columns(), img.rows(), "R", FloatPixel, ImgPixels);
	for( int i = 0; i < imSize; i++)
		ImgPixels[i] -= 0.5;

	unsigned short tmp = img.columns();
	oFile.write((char *)&tmp, sizeof(unsigned short));
	tmp = img.rows();
	oFile.write((char *)&tmp, sizeof(unsigned short));
	oFile.write((char *)&Head, sizeof(Header));
	unsigned char * pEnd = pStream;

	if (Type == 0) {
		CMuxCodec Codec(pEnd, 0);
		Codec.initTaboo(TABOO_LEN);

		CWaveletDir Wavelet(img.columns(), img.rows(), WAV_LEVELS);
		Wavelet.SetWeight97();
		Wavelet.SetCodec(&Codec);
		Wavelet.Transform97(ImgPixels, img.columns(), (int)(sqrtf(Quants[Quant]) * 256.f / 3.f + .5));

// 		Wavelet.Stats();
//  	Map2PNG(Wavelet, outfile);
// 		Dist2PNG(Wavelet, outfile);

		Wavelet.TSUQ(Quants[Quant], Quants[Quant] * Thres);

// 		Band2PNG(Wavelet, outfile);

		Wavelet.CodeMap(0);

		Wavelet.CodeBand(&Codec, 1);

		pEnd = Codec.endCoding();
	} else {
		throw UNKNOW_TYPE;
	}

	oFile.write((char *) pStream + 2, (pEnd - pStream - 2));
	oFile.close();

	delete[] ImgPixels;
	delete[] pStream;
}

void DecompressImage(string & infile, string & outfile, float RecLevel, int Dither)
{
	ifstream iFile( infile.c_str() , ios::in );
	char magic[4] = {0,0,0,0};

	iFile.read(magic, 4);

	if (magic[0] != 'D' || magic[1] != 'W' || magic[2]!= 'I' || magic[3] != 'C')
		throw BAD_MAGIC;

	unsigned short width, heigth;
	iFile.read((char *) &width, sizeof(unsigned short));
	iFile.read((char *) &heigth, sizeof(unsigned short));

	float * ImgPixels = new float [width * heigth * 3];
	unsigned char * pStream = new unsigned char[width * heigth];

	iFile.read((char *) pStream, width * heigth);

	Header Head;
	Head.last = *pStream;

	unsigned char * pEnd = pStream;

	if (Head.Type == 0) {
		CMuxCodec Codec(pEnd - 1);
		Codec.initTaboo(TABOO_LEN);
		CWaveletDir Wavelet(width, heigth, WAV_LEVELS);
		Wavelet.SetWeight97();
		Wavelet.SetCodec(&Codec);
		Wavelet.DecodeMap(0);
		Wavelet.DecodeBand(&Codec, 1);
		Wavelet.TSUQi(Quants[Head.Quant], Quants[Head.Quant] * RecLevel);
		Wavelet.Transform97I(ImgPixels, width);
	} else {
		throw UNKNOW_TYPE;
	}

	if (Dither == 1)
		FLOAT2CHAR(ImgPixels, width, heigth);
	else
		FLOAT2CHAR(ImgPixels, width * heigth);
	BW2RGB((unsigned char *)ImgPixels, width * heigth);
	Image img(width, heigth, "RGB", CharPixel, ImgPixels);
	img.type( GrayscaleType );
	img.depth(8);
	img.compressType(UndefinedCompression);
	// img.display();

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
	float ThresRatio = 0.7;
	int Quant = 9;
	float RecLevelRatio = 0;
	int Type = 0;
	int Dither = 0;

	while ((c = getopt(argc , argv, "i:o:q:r:t:v:d")) != -1) {
		switch (c) {
			case 'i':
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'q':
				Quant = atoi(optarg);
				break;
			case 'r':
				RecLevelRatio = atof(optarg);
				break;
			case 't':
				ThresRatio = atof(optarg);
				break;
			case 'v':
				Type = atoi(optarg);
				break;
			case 'd':
				Dither = 1;
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
		CompressImage(infile, outfile, Quant, ThresRatio, Type);
	} else {
		DecompressImage(infile, outfile, RecLevelRatio, Dither);
	}

	return EXIT_SUCCESS;
}
