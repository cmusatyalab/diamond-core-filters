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
#include "fil_img_diff.h"
#include "lib_filimage.h"

double image_diff(const RGBImage* img1, const RGBImage* img2,
	bbox_list_t *bbox);

static int
f_init_img_diff(int numarg, const char * const *args,
		int blob_len, const void *blob,
		const char *fname, void **data)
{
	img_diff_config_t *config;
	
	config = (img_diff_config_t *)malloc(sizeof(img_diff_config_t));
	assert(config);
	config->fname = strdup(fname);

	/* load example images from blob */
	TAILQ_INIT(&config->examples);
	load_examples(blob, blob_len, &config->examples);

	*data = (void *)config;
	return (0);
}

static double
f_eval_img_diff(lf_obj_handle_t ohandle, void *f_data)
{
	int             err;
	size_t 		len;
	const void *	dptr;	
	double		best_distance = 1.0;
	double		cur_distance;
	bbox_list_t	blist;
	bbox_list_t	accepted;
	bbox_t *	cur_box;
	example_patch_t	*patch;

	lf_log(LOGL_TRACE, "f_eval_img_diff: enter");

	img_diff_config_t *config = (img_diff_config_t *)f_data;
	err = lf_ref_attr(ohandle, RGB_IMAGE, &len, &dptr);
	assert(err == 0);

	TAILQ_INIT(&blist);
	TAILQ_FOREACH(patch, &config->examples, link) {
		cur_distance = image_diff((RGBImage *)dptr, patch->image,
					&blist);
		if (cur_distance < best_distance)
			best_distance = cur_distance;
	}

	/* Return only the patch(es) representing the best match */
	TAILQ_INIT(&accepted);
	while (!TAILQ_EMPTY(&blist)) {
		cur_box = TAILQ_FIRST(&blist);
		TAILQ_REMOVE(&blist, cur_box, link);
		if (cur_box->distance == best_distance) {
			TAILQ_INSERT_TAIL(&accepted, cur_box, link);
		} else {
			free(cur_box);
		}
	}
	save_patches(ohandle, config->fname, &accepted);

	while (!(TAILQ_EMPTY(&accepted))) {
		cur_box = TAILQ_FIRST(&accepted);
		TAILQ_REMOVE(&accepted, cur_box, link);
		free(cur_box);
	}

	/* return % similarity */
	return 100 - best_distance * 100.0;
}

LF_MAIN(f_init_img_diff, f_eval_img_diff)

// Given 2 images of the same size, returns a number (0 to 1)
// quantifying the pixel-wise image difference.  Zero means
// images are identical.
//
double
image_diff(const RGBImage* img1, const RGBImage* img2, bbox_list_t *blist) {
  double diff = 0.0;
  int x, y, width, height;
  const RGBPixel *pixel1, *pixel2;
  bbox_t *bb;

  // Compare the top-left corners of the images
  width = MIN(img1->width, img2->width);
  height = MIN(img1->height, img2->height);
  for (x=0; x < width; x++) {
    for (y=0; y < height; y++) {
      pixel1 = &img1->data[PIXEL_OFFSET(img1, x, y)];
      pixel2 = &img2->data[PIXEL_OFFSET(img2, x, y)];
      diff += fabs(pixel1->r - pixel2->r) + fabs(pixel1->g - pixel2->g) +
			fabs(pixel1->b - pixel2->b);
    }
  }

  // normalize to [0,1] 
  diff /= (3 * width * height * 255);

  assert(diff >= 0.0);
  assert(diff <= 1.0);

  bb = malloc(sizeof(*bb));
  bb->min_x = 0;
  bb->min_y = 0;
  bb->max_x = width - 1;
  bb->max_y = height - 1;
  bb->distance = diff;
  TAILQ_INSERT_TAIL(blist, bb, link);

  return diff;
}




