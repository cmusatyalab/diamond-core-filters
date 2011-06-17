/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <math.h>
#include <assert.h>
#include "rgb.h"
#include "gabor.h"


#define	FILTER_OFFSET(angle, freq)  (((angle) * (gab_freq)) + (freq))


gabor::gabor(int angles, int radius, int freq, float max_freq, float min_freq)
{
	int	i,j;
	float 	cur_angle;
	float 	cur_freq;
	float 	freq_step;
	float	sig_sq;
	int		resp;

	gab_angles = angles;
	gab_radius = radius;
	gab_dim = 2*radius + 1;
	gab_freq = freq;
	gab_responses = gab_angles * gab_freq;
	gab_sigma = ((float)radius)/3.0;

	sig_sq = M_PI * M_PI * gab_sigma * gab_sigma;
	gab_max_freq = max_freq;
	gab_min_freq = min_freq;

	gab_filt_real = new float[gab_dim * gab_dim * gab_responses];
	gab_filt_img = new float[gab_dim * gab_dim * gab_responses];

	freq_step = (gab_max_freq - gab_min_freq)/ (float)gab_freq;

	resp = 0;
	for (i=0; i < gab_angles; i++) {
		cur_angle = (float)i * M_PI/(float)gab_angles;
		for (j=0; j < gab_freq; j++) {
			cur_freq = gab_min_freq + (j * freq_step);
			cur_freq = (cur_freq * M_PI)/2.0;
			filter_init(cur_angle, cur_freq, sig_sq, resp);
			resp++;
		}
	}
}


static float
vector_sum(int num, float *vec)
{
	float   sum = 0.0;
	int             i;

	for (i=0; i < num;i++) {
		sum += vec[i];
	}
	return(sum);

}


gabor::~gabor()
{
	delete gab_filt_real;
	delete gab_filt_img;
	return;
}

#define	VAL_OFFSET(x, y, resp)   \
    (((((y) * (gab_dim)) + (x)) * gab_responses) + (resp))


void
gabor::filter_init(float angle, float freq, float sigma_sq, int resp)
{
	int x, y, dist;
	int		voffset;
	float	cos_val, sin_val, exp_val, exp_const;
	float	sum_val;


	cos_val = cos(angle);
	sin_val = sin(angle);
	exp_const = exp((-1.0*M_PI*M_PI)/2.0);

	for (y = -gab_radius; y <= gab_radius; y++) {
		for (x = -gab_radius; x <= gab_radius; x++) {
			dist = x*x + y*y;
			exp_val = exp(-((float)dist)/sigma_sq);
			sum_val = freq*(((float)y*cos_val)-((float)x*sin_val));
			voffset = VAL_OFFSET((x + gab_radius),(y+gab_radius), resp);
			gab_filt_real[voffset] = exp_val * sin(sum_val);
			gab_filt_img[voffset] = exp_val * (cos(sum_val) - exp_const);
		}
	}
}

int
gabor::get_responses(FGImage_t *image, int x, int y, int size, float *rvec,
                     int normalize)
{
	int		i;
	int	poffset;
	int	voffset;
	int	xoff, yoff;
	float	pval;
	float	*real, *img;
	assert(x >=0);
	assert(y >=0);

	/* make sure response vector is large enough */
	if (gab_responses > size) {
		fprintf(stderr, "get_reponses: too little space \n");
		return (EINVAL);
	}

	if ((x + gab_dim) > image->width) {
		return(1);
	}
	if ((y + gab_dim) > image->height) {
		return(1);
	}
	real = (float *)calloc(gab_responses, sizeof(float));
	img = (float *)calloc(gab_responses, sizeof(float));


	for (yoff = 0; yoff < gab_dim; yoff++) {
		poffset = PIXEL_OFFSET(image, x, (y +yoff));
		for (xoff = 0; xoff < gab_dim; xoff++) {
			pval = image->data[poffset + xoff];
			voffset = VAL_OFFSET(xoff, yoff, 0);
			for (i=0; i < gab_responses; i++) {
				real[i] += pval * gab_filt_real[voffset+i];
				img[i] +=  pval * gab_filt_img[voffset+i];
			}
			voffset += gab_responses;
		}
	}

	for (i=0; i < gab_responses; i++) {
		rvec[i] = sqrt(real[i]*real[i] + img[i]*img[i]);
	}

	if (normalize) {
		float	sum;
		sum = vector_sum(gab_responses, rvec);
		for (i=0; i < gab_responses; i++) {
			rvec[i] = rvec[i]/sum;
		}
	}

	free(real);
	free(img);
	return(0);
}
