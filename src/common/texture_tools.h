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

#ifndef TEXTURE_TOOLS_H
#define TEXTURE_TOOLS_H

#include <opencv/cv.h>
#include "queue.h"

// Code to support texture filters
// Derek Hoiem 2003.05.21


#define NUM_LAP_PYR_LEVELS 		4
#define TEXTURE_MAX_CHANNELS 	3


typedef	enum {
    TEXTURE_DIST_MAHOLONOBIS,
    TEXTURE_DIST_VARIANCE,
    TEXTURE_DIST_PAIRWISE
} texture_dist_t;



typedef struct texture_args_t {
	char* 		name;
	int 		num_samples;
	double **	sample_values;
	double 		max_distance;
	int 		box_width;
	int 		box_height;
	int 		step;
	double 		scale;
	int 		min_matches;
	int 		num_channels;
	texture_dist_t	texture_distance;
} texture_args_t;

/**
 * Returns the value of the minimum distance from some box in the image to
 * one of the samples.
 * img - the image to be tested
 * samples - the feature values of the samples
 * threshold - the minimum distance to fail
 * quit_on_pass - true if testing should stop after first passing window
 * dx, dy - the number of x and y pixels to skip between tests
 * width, height - the width and height of the testing window
 * min_x, min_y - to store the location of the region with lowest distance
 * dst_img - each pixel: 0 = failed or not tested, 1 = passed
 **/

#ifdef __cplusplus
extern "C"
{
#endif


int texture_test_entire_image_maholonobis(IplImage *img, texture_args_t *targs,
	        bbox_list_t *blist);
int texture_test_entire_image_variance(IplImage *img, texture_args_t *targs,
								   bbox_list_t *blist);
int texture_test_entire_image_pairwise(IplImage *img, texture_args_t *targs,
								   bbox_list_t *blist);


/* gets features from a single subwindow
*/
void texture_get_lap_pyr_features_from_subimage(IplImage* img,
	int num_channels,
	int min_x,
	int min_y,
	int box_width,
	int box_height,
	double *feature_values);

#ifdef __cplusplus
}
#endif

#endif //TEXTURE_TOOLS
