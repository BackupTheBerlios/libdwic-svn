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

#include "bitcodec.h"

namespace libdwic {

CBitCodec::CBitCodec(CMuxCodec * RangeCodec)
{
	InitModel();
	setRange(RangeCodec);
}

CBitCodec::~CBitCodec()
{

}

const int CBitCodec::mod[][2] = {
	{101356, 0},
	{106680, -2560},
	{110641, -4498},
	{114166, -6605},
	{117591, -8893},
	{121006, -11377},
	{124494, -14073},
	{128079, -17002},
	{131886, -20199},
	{135892, -23688},
	{140179, -27512},
	{144782, -31714},
	{149803, -36360},
	{155298, -41515},
	{161378, -47272},
	{168113, -53727},
	{175651, -61016},
	{184155, -69306},
	{193910, -78843},
	{205141, -89893},
	{218293, -102874},
	{233893, -118322},
	{252718, -137016},
	{275918, -160101},
	{305231, -189320},
	{343531, -227533},
	{395640, -279586},
	{470821, -354728},
	{588789, -472690},
	{800532, -684513},
	{1032508, -940044},
	{1001686, -970865},
	{970865, -1001686},
	{940044, -1032508},
	{684513, -800532},
	{472690, -588789},
	{354728, -470821},
	{279586, -395640},
	{227533, -343531},
	{189320, -305231},
	{160101, -275918},
	{137016, -252718},
	{118322, -233893},
	{102874, -218293},
	{89893, -205141},
	{78843, -193910},
	{69306, -184155},
	{61016, -175651},
	{53727, -168113},
	{47272, -161378},
	{41515, -155298},
	{36360, -149803},
	{31714, -144782},
	{27512, -140179},
	{23688, -135892},
	{20199, -131886},
	{17002, -128079},
	{14073, -124494},
	{11377, -121006},
	{8893, -117591},
	{6605, -114166},
	{4498, -110641},
	{2560, -106680},
	{0, -101356}
};

void CBitCodec::InitModel(void){
	for (int i = 0; i < BIT_CONTEXT_NB; i++){
		Freq[i] = HALF_FREQ_COUNT << FREQ_POWER;
	}
}

}
