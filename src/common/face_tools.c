/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#include <opencv/cv.h>
#include <opencv/cvaux.h>

#include "queue.h"
#include "ring.h"
#include "rtimer.h"

// #include "face_search.h"
#include "face_tools.h"
//#include "face_image.h"
//#include "fil_face.h"
#include "rgb.h"
#include "face.h"
#include "fil_tools.h"
#include "image_tools.h"
#include "texture_tools.h"
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

int
opencv_face_scan(RGBImage *rgb, bbox_list_t *blist, opencv_fdetect_t *fconfig)
{
	int 		i;
	CvAvgComp	r1;	
	bbox_t	*	bb;
	CvMemStorage *	storage = cvCreateMemStorage(0);
	IplImage *	gray;	
	CvSeq* 		faces;
	int			total;


	/* XXX we should find some way to reuse IPL */
	gray = get_gray_ipl_image(rgb);


	/* XXX fix args */	
	faces = cvHaarDetectObjects(gray, fconfig->haar_cascade, storage, 
		fconfig->scale_mult, fconfig->support, CV_HAAR_DO_CANNY_PRUNING);


	/* XXX who cleans up the faces */
	for (i = 0; i < faces->total; i++) {
		r1 = *(CvAvgComp*)cvGetSeqElem(faces, i, NULL);
		bb = (bbox_t *)malloc(sizeof(*bb));
		assert(bb != NULL);
		bb->min_x = r1.rect.x;
		bb->min_y = r1.rect.y;
		bb->max_x = r1.rect.x + r1.rect.width;
		bb->max_y = r1.rect.y + r1.rect.height;
		bb->distance = 0.0;
		TAILQ_INSERT_TAIL(blist, bb, link);
	}

	/* faces will be relased by the cvReleaseMemStorage */

	total = faces->total;
	cvReleaseMemStorage(&storage);
	cvReleaseImage(&gray);		
	return(total);
}



