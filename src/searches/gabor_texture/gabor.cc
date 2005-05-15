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
#include <errno.h>
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

	gab_angles = angles;
	gab_radius = radius;
	gab_freq = freq;
	gab_sigma = ((float)radius)/3.0;

	sig_sq = M_PI * M_PI * gab_sigma * gab_sigma;
	gab_max_freq = max_freq;
	gab_min_freq = min_freq;
	gab_filters = new gabor_filter *[gab_angles * gab_freq];

	freq_step = (gab_max_freq - gab_min_freq)/ (float)gab_freq;

	for (i=0; i < gab_angles; i++) {
		cur_angle = (float)i * M_PI/(float)gab_angles;
		for (j=0; j < gab_freq; j++) {
			cur_freq = gab_min_freq + (j * freq_step);
			cur_freq = (cur_freq * M_PI)/2.0;
			gab_filters[FILTER_OFFSET(i,j)] = 
				new gabor_filter(gab_radius, cur_angle,
				cur_freq, sig_sq);
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

int
gabor::get_responses(RGBImage *image, int x, int y, int size, float *rvec,
	int normalize)
{
	int		i;
	int		foffset;
	int		err;
	int		num_responses;
	gabor_filter *	filt;

	/* make sure response vector is large enough */
	num_responses = gab_angles * gab_freq;
	if (num_responses > size) {
		fprintf(stderr, "get_reponses: too little space \n");
		return (EINVAL);
	}


	for (i=0; i < num_responses; i++) {
		filt = gab_filters[foffset];
		err = gab_filters[i]->get_response(image, x, y, &rvec[i]);
		if (err) {
			fprintf(stderr, "get_reponses: get resp failed\n");
			return(err);
		}
	}
	if (normalize) {
		float	sum;
		sum = vector_sum(num_responses, rvec);
		for (i=0; i < num_responses; i++) {
			rvec[i] = rvec[i]/sum;
		}
	}

	return(0);
}

gabor::~gabor()
{
	delete gab_filters;
	return;
}



