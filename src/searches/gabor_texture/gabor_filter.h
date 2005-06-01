/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_GABOR_FILTER_H_
#define	_GABOR_FILTER_H_	1

#include <math.h>
#include "rgb.h"

class gabor_filter
{
public:
	gabor_filter(int radius, float cur_angle, float cur_freq, float sigma);
	~gabor_filter(void);

	int get_response(RGBImage *im, int x, int y, float *response);

private:
	int		gfilt_dim;
	float *		gfilt_real;
	float *		gfilt_img;
};


#endif	/* !_GABOR_FILTER_H_ */
