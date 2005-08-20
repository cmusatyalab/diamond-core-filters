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


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include <libgen.h>             /* dirname */
#include <assert.h>
#include <stdint.h>

#include "lib_filter.h"
#include "lib_log.h"
#include "queue.h"
#include "ring.h"
#include "rtimer.h"
#include "lib_results.h"
#include "lib_sfimage.h"

#include "fil_tools.h"
#include "vj_face_tools.h"
#include "rgb.h"
#include "facedet.h"


#define compute_sum(ii,x,y,xsiz,ysiz) (			\
    (ii)->data[((y)+(ysiz)) * (ii)->width + ((x)+(xsiz))]	\
  + (ii)->data[ (y) * (ii)->width         + (x)]		\
  - (ii)->data[((y)+(ysiz)) * (ii)->width + (x)]		\
  - (ii)->data[ (y) * (ii)->width         + ((x)+(xsiz))])


double
image_variance(ii_image_t * iimage, ii2_image_t * iimage_sq,
               dim_t x, dim_t y, dim_t xsiz, dim_t ysiz, double scale)
{
	double          m;
	dim_t           n = (xsiz) * (ysiz);
	double          mean,
	var2,
	variance;

	assert(xsiz > 0 && ysiz > 0);
	assert(n > 0);
	mean = (double) compute_sum(iimage, x, y, xsiz, ysiz) / n;
	var2 = (double) compute_sum(iimage_sq, x, y, xsiz, ysiz) / n;
	variance = var2 - mean * mean;
	m = sqrt(4000.0 / (variance + 10.0));
	m /= (scale * scale);
	return m;
}

/*
 * Test the image to find each of the faces that are in
 * them.
 */


int
face_scan_image(ii_image_t *ii, ii2_image_t * ii2, fconfig_fdetect_t *fconfig,
                bbox_list_t *blist, int height, int width)
{

	/* generate all the possible windows and test them */
	double          x, y;
	double          scale;
	int             got_work = 1;
	double			img_var;
	double			xsiz, ysiz;
	int				pass = 0;
	bbox_t	 *		bbox;

	/*
		 * for all scales 
		 */
	xsiz = fconfig->xsize;
	ysiz = fconfig->ysize;

	for (scale = 1; got_work; scale *= fconfig->scale_mult) {
		/*
			 * each time we change the scale, readjust the feature table 
			 */
		scale_feature_table(scale);
		for (x = 1; x + xsiz <= width; x += fconfig->stride) {
			for (y = 1; y + ysiz <= height; y += fconfig->stride) {
				img_var = image_variance(ii, ii2, (dim_t) x, (dim_t) y,
				                         (dim_t) xsiz, (dim_t) ysiz, scale);

				if (test_region(ii, fconfig->lev1, fconfig->lev2,
				                (int) x, (int)y, scale, img_var)) {

					bbox = (bbox_t *)malloc(sizeof(*bbox));
					assert(bbox != NULL);
					bbox->min_x = x;
					bbox->min_y = y;
					bbox->max_x = x + xsiz;
					bbox->max_y = y + ysiz;
					bbox->distance = 0.0;
					TAILQ_INSERT_TAIL(blist, bbox, link);
					pass++;
				}
			}
		}
		xsiz *= fconfig->scale_mult;
		ysiz *= fconfig->scale_mult;
		got_work = (xsiz <= width) && (ysiz <= height);
	}

	return(pass);
}



