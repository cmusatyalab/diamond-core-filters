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
#include "gabor.h"


#define	FILTER_OFFSET(angle, freq)  (((angle) * (gab_freq)) + (freq))


gabor::gabor(int angles, int radius, int freq, float max_freq, float min_freq,
	float sigma)
{
	int	i,j;
	float 	cur_angle;
	float 	cur_freq;
	float 	freq_step;

	gab_angles = angles;
	gab_radius = radius;
	gab_freq = freq;
	gab_sigma = M_PI * M_PI * sigma;
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
				new gabor_filter(gab_radius, cur_angle, cur_freq, gab_sigma);
		}
	}
}

gabor::~gabor()
{
	delete gab_filters;
	return;
}



