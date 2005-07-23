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


/*
 * face detector 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <opencv/cv.h>
#include <opencv/cvaux.h>

#include "filter_api.h"
#include "fil_face.h"
#include "face.h"
#include "face_tools.h"
#include "facedet.h"
#include "merge_faces.h"
#include "fil_tools.h"
#include "fil_image_tools.h"
#include "image_common.h"


int
f_init_opencv_fdetect(int numarg, char **args, int blob_len, void *blob_data,
                      void **fdatap)
{

	opencv_fdetect_t *fconfig;
	CvHaarClassifierCascade *cascade;


	fconfig = (opencv_fdetect_t *) malloc(sizeof(*fconfig));
	assert(fconfig != NULL);

	fconfig->name = strdup(args[0]);
	assert(fconfig->name != NULL);
	fconfig->scale_mult = atof(args[1]);
	fconfig->xsize = atoi(args[2]);
	fconfig->ysize = atoi(args[3]);
	fconfig->stride = atoi(args[4]);
	/*
	 * XXX skip 5 for now ?? 
	 */
	fconfig->support = atoi(args[6]);

	if (fconfig->scale_mult <= 1) {
		lf_log(LOGL_TRACE,
		       "scale multiplier must be > 1; got %f\n", fconfig->scale_mult);
		exit(1);
	}

	cascade = cvLoadHaarClassifierCascade("<default_face_cascade>",
	                                      cvSize(fconfig->xsize, fconfig->ysize));
	/* XXX check args */
	fconfig->haar_cascade = cvCreateHidHaarClassifierCascade(
	                            cascade, 0, 0, 0, 1);
	cvReleaseHaarClassifierCascade(&cascade);

	*fdatap = fconfig;
	return (0);
}


int
f_fini_opencv_fdetect(void *fdata)
{
	opencv_fdetect_t *fconfig = (opencv_fdetect_t *) fdata;


	cvReleaseHidHaarClassifierCascade(&fconfig->haar_cascade);
	free(fconfig->name);
	free(fconfig);
	return (0);
}


int
f_eval_opencv_fdetect(lf_obj_handle_t ohandle, void *fdata)
{
	int             pass = 0;
	RGBImage *		img;
	search_param_t  param;
	opencv_fdetect_t *fconfig = (opencv_fdetect_t *) fdata;
	int             err;
	bbox_list_t	    blist;
	int				i;
	bbox_t *		cur_box;
	off_t			len;

	lf_log(LOGL_TRACE, "f_eval_opencv_fdetect: enter\n");

	/*
	  * get the img
	  */
	err = lf_ref_attr(ohandle, RGB_IMAGE, &len, (char**)&img);
	assert(err == 0);


	TAILQ_INIT(&blist);
	pass = opencv_face_scan(img, &blist, fconfig);

	i = 0;
	while (!(TAILQ_EMPTY(&blist))) {
		cur_box = TAILQ_FIRST(&blist);
		param.type = PARAM_FACE;
		param.bbox.xmin = cur_box->min_x;
		param.bbox.ymin = cur_box->min_y;
		param.bbox.xsiz = cur_box->max_x - cur_box->min_x;
		param.bbox.ysiz = cur_box->max_y - cur_box->min_y;
		write_param(ohandle, FACE_BBOX_FMT, &param, i);
		TAILQ_REMOVE(&blist, cur_box, link);
		free(cur_box);
		i++;
	}

	/*
	 * save 'pass' in attribs 
	 */
	err = lf_write_attr(ohandle, NUM_FACE, sizeof(int),
	                    (char *) &pass);
	assert(!err);
	lf_log(LOGL_TRACE, "found %d faces\n", pass);


	lf_log(LOGL_TRACE, "f_eval_opencv_fdetect: done\n");
	return pass;
}




