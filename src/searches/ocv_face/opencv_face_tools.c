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

#include "opencv_face_tools.h"
#include "rgb.h"
#include "opencv_face.h"
#include "lib_sfimage.h"
#include "lib_ocvimage.h"


#define compute_sum(ii,x,y,xsiz,ysiz) (			\
    (ii)->data[((y)+(ysiz)) * (ii)->width + ((x)+(xsiz))]	\
  + (ii)->data[ (y) * (ii)->width         + (x)]		\
  - (ii)->data[((y)+(ysiz)) * (ii)->width + (x)]		\
  - (ii)->data[ (y) * (ii)->width         + ((x)+(xsiz))])



/*
 * Test the image to find each of the faces that are in
 * them.
 */



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
	CvSize minSize = { 20, 20 };
	faces = cvHaarDetectObjects(gray, fconfig->haar_cascade, storage,
	                            fconfig->scale_mult, fconfig->support, CV_HAAR_DO_CANNY_PRUNING, minSize);


	/* XXX who cleans up the faces */
	for (i = 0; i < faces->total; i++) {
		r1 = *(CvAvgComp*)cvGetSeqElem(faces, i);
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



