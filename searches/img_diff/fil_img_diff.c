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

/*
 * image difference filter
 */

#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <sys/param.h>

#include "rgb.h"        
#include "lib_filter.h"
#include "lib_results.h"
#include "snapfind_consts.h"
#include "fil_img_diff.h"
#include "lib_sfimage.h"

double image_diff(const RGBImage* img1, const RGBImage* img2);

static int
f_init_img_diff(int numarg, const char * const *args,
		int blob_len, const void *blob,
		const char *fname, void **data)
{
	img_diff_config_t *config;
	
	config = (img_diff_config_t *)malloc(sizeof(img_diff_config_t));
	assert(config);
	
	/* example RGB image stored in blob */
	assert (blob != NULL);
	config->img = blob;
	config->distance = atof(args[0]);
	*data = (void *)config;
	return (0);
}

static int
f_eval_img_diff(lf_obj_handle_t ohandle, void *f_data)
{
	int             err;
	int             rv = 0;     /* return value */
	size_t 		len;
	const void *	dptr;	
	double distance = 0.0;

	lf_log(LOGL_TRACE, "f_eval_img_diff: enter");

	img_diff_config_t *config = (img_diff_config_t *)f_data;
	err = lf_ref_attr(ohandle, RGB_IMAGE, &len, &dptr);
	assert(err == 0);
	distance = image_diff((RGBImage *)dptr, config->img);
	/* return 1 if image exceeds distance threshold */
	if (distance * 100.0 > config->distance)
		rv = 1;

	return rv;
}

LF_MAIN(f_init_img_diff, f_eval_img_diff)

// Given 2 images of the same size, returns a number (0 to 1)
// quantifying the pixel-wise image difference.  Zero means
// images are identical.
//
double
image_diff(const RGBImage* img1, const RGBImage* img2) {
  double diff = 0.0;
  int i, pixels;

  // Might want to die more gracefully
  assert(img1->height == img2->height);
  assert(img1->rows == img2->rows);
  assert(img1->nbytes == img2->nbytes);

  // Assumes identically-sized images
  pixels = img1->height * img1->width;
  for (i=0; i < pixels; i++) {
  	diff += fabs(img1->data[i].r - img2->data[i].r) + 
  			fabs(img1->data[i].g - img2->data[i].g) +
  			fabs(img1->data[i].b - img2->data[i].b);
  }

  // normalize to [0,1] 
  diff /= (3 * pixels * 255);

  assert(diff >= 0.0);
  assert(diff <= 1.0);
  return diff;
}




