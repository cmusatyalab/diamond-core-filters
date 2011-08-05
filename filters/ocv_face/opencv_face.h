/*
 *  Diamond Core Filters - collected filters for the Diamond platform
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _OPENCV_FACE_H_
#define _OPENCV_FACE_H_

#include <sys/types.h>
#include "lib_ocvimage.h"
#include <opencv/cvaux.h>
#include "lib_results.h"
#include <sys/queue.h>





/*
 * Structure we use to keep track of the state
 * used for the filter config.
 */
typedef struct opencv_fdetect {
	char *		name;
	int		xsize;
	int		ysize;
	int		stride;
	float		scale_mult;
	int		support;
	CvHaarClassifierCascade	*haar_cascade;
} opencv_fdetect_t;

#endif /*  ! _OPENCV_FACE_H_ */
