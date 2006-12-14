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

#include "global.h"
#include "waveletdir.h"

namespace libdwic{

static const float Quants[32] =
{	0.0156, 0.0179, 0.0206, 0.0237, 0.0272, 0.0313, 0.0359, 0.0412, 0.0474,
	0.0544, 0.0625, 0.0718, 0.0825, 0.0947, 0.1088, 0.1250, 0.1436, 0.1649,
	0.1895, 0.2176, 0.2500, 0.2872, 0.3299, 0.3789, 0.4353, 0.5000, 0.5743,
	0.6598, 0.7579, 0.8706, 1.0000, 1.1487
};

static const float LambdaDir[32] =
// {	0.25, 0.2176, 0.1895, 0.1649, 0.1436, 0.125, 0.1088, 0.0947, 0.0825, 0.0718,
//  	0.0625, 0.0544, 0.0474, 0.0412, 0.0359, 0.0313, 0.0272, 0.0237, 0.0206,
// 	0.0179, 0.0156, 0.0136, 0.0118, 0.0103, 0.009, 0.0078, 0.0068, 0.0059,
// 	0.0052, 0.0045, 0.0039, 0.0034
// };

// {	0.125, 0.1088, 0.0947, 0.0825, 0.0718, 0.0625, 0.0544, 0.0474, 0.0412,
//  	0.0359, 0.0313, 0.0272, 0.0237, 0.0206, 0.0179, 0.0156, 0.0136, 0.0118,
// 	0.0103, 0.009, 0.0078, 0.0068, 0.0059, 0.0052, 0.0045, 0.0039, 0.0034,
// 	0.003, 0.0026, 0.0022, 0.002, 0.0017
// };

{	0.125, 0.1166, 0.1088, 0.1015, 0.0947, 0.0884, 0.0825, 0.0769, 0.0718,
	0.067, 0.0625, 0.0583, 0.0544, 0.0508, 0.0474, 0.0442, 0.0412, 0.0385,
	0.0359, 0.0335, 0.0313, 0.0292, 0.0272, 0.0254, 0.0237, 0.0221, 0.0206,
	0.0192, 0.0179, 0.0167, 0.0156, 0.0146
};

// {
// 	0.0625, 0.0583, 0.0544, 0.0508, 0.0474, 0.0442, 0.0412, 0.0385, 0.0359,
// 	0.0335, 0.0313, 0.0292, 0.0272, 0.0254, 0.0237, 0.0221, 0.0206, 0.0192,
// 	0.0179, 0.0167, 0.0156, 0.0146, 0.0136, 0.0127, 0.0118, 0.011, 0.0103,
// 	0.0096, 0.009, 0.0084, 0.0078, 0.0073
// };

// diagonal1 = \
// diagonal2 = /
typedef enum direction {horizontal, vertical, diagonal1, diagonal2};

}
