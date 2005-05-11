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


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include "rgb.h"
#include "assert.h"
#include "gabor.h"


#define	VAL_OFFSET(x, y)  (((y) * (gfilt_dim)) + (x))





gabor_filter::gabor_filter(int radius, float angle, float freq, float sigma)
{
	int x, y, dist;
	int		voffset;
	float	cos_val, sin_val, exp_val, exp_const;
	float	sum_val;

	gfilt_dim = 2 * radius + 1;
	gfilt_real = new float[gfilt_dim * gfilt_dim];
	gfilt_img = new float[gfilt_dim * gfilt_dim];

	cos_val = cos(angle);
	sin_val = sin(angle);
	exp_const = exp((-1.0*M_PI*M_PI)/2.0);

	for (x = -radius; x <= radius; x++) {
		for (y = -radius; y <= radius; y++) {
			dist = x*x + y*y;		
			exp_val = exp(-((float)dist)/sigma);
			sum_val = freq*(((float)y*cos_val)-((float)x*sin_val));
			voffset = VAL_OFFSET((x + radius),(y+radius));
			gfilt_real[voffset] = exp_val * sin(sum_val);
			gfilt_img[voffset] = exp_val * (cos(sum_val) - exp_const);
		}
	}
	x = 0;
	for (y=0; y < 4; y++) {
		fprintf(stderr,"<%f %f> ", gfilt_real[VAL_OFFSET(x,y)],
				gfilt_img[VAL_OFFSET(x,y)]);
		}
	fprintf(stderr, "\n");
			
	
}

gabor_filter::~gabor_filter()
{
	/* XXXX cleanup */
	delete[] gfilt_real;
	delete[] gfilt_img;
	return;
}



int
gabor_filter::get_response(RGBImage *image, int x, int y, float *response)
{
	int	poffset;
	int	voffset;
	int	xoff, yoff;
	int	pval;
	float	real, img, dist;
	assert(x >=0);
	assert(y >=0);


	if ((x + gfilt_dim) > image->width) {
		return(1);
	}
	if ((y + gfilt_dim) > image->height) {
		return(1);
	}

	real = 0.0;
	img = 0.0;

	for (yoff = 0; yoff < gfilt_dim; yoff++) {
		for (xoff = 0; xoff < gfilt_dim; xoff++) {
			poffset = PIXEL_OFFSET(image, (x + xoff), (y +yoff));
			voffset = VAL_OFFSET(xoff, yoff);
			pval = 	(image->data[poffset].r + image->data[poffset].g + 
				image->data[poffset].b)/3;
			real += pval * gfilt_real[voffset];
			img += pval * gfilt_img[voffset];
		}
	}
	dist = sqrt(real*real + img*img);
	*response = dist;
	return(0);
}


