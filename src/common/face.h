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


#ifndef _FACE_H_
#define _FACE_H_

#include <sys/types.h>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include "queue.h"

/* image dimension */
//typedef u_int32_t dim_t;
typedef int32_t dim_t;

/* 
 * bounding box of region to consider
 */
typedef struct region {
	dim_t xmin, ymin;
	dim_t xsiz, ysiz;
} region_t;

typedef enum param_type_t {
	PARAM_UNKNOWN = 0,
	PARAM_FACE, 
	PARAM_HISTO,
	PARAM_TEXTURE
} param_type_t;

//void param_type_string(char *buf, param_type_t type);

/*
 * attributes passed to the test function
 */
#define PARAM_NAME_MAX 15
typedef struct search_param {
	param_type_t type;
	int lev1, lev2;		/* range of tests to apply (face) */
	region_t bbox;		/* bounding box */
	double scale;		/* (face) */
	union {
		double img_var;	/* (face) */
		double distance; /* (histo) */
	};
	char name[PARAM_NAME_MAX+1];
	int id;
} search_param_t;


/*
 * some more attributes
 */

//#define NUM_BBOXES "_num_bboxes.int"
//#define BBOX_FMT   "_bbox%d.search_param"

#define NUM_HISTO        "_nhisto_passed.int"
#define NUM_FACE         "_nface_passed.int"
#define NUM_TEXTURE      "_ntexture_passed.int"
#define HISTO_BBOX_FMT   "_h_box%d.search_param"
#define FACE_BBOX_FMT    "_f_box%d.search_param"

#define POISON_FN  "poison_%s"

#define II_DATA    "_integral_img.ii_image"
#define II_SQ_DATA "_integral_img_sq.ii_image"
#define RGB_IMAGE  "_rgb_image.rgbimage"
#define HISTO_II   "integral_histo"

#define ROWS       "_rows.int"
#define COLS       "_cols.int"
#define IMG_HEADERLEN "_headerlen.int"

#define RGBA_IPL_IMAGE "_rgba_image.ipl_image"
#define GRAY_IPL_IMAGE "_gray_image.ipl_image"


#define DISPLAY_NAME "Display-Name"
#define KEYWORDS "Keywords"
#define CONTENT_TYPE "Content-Type"
#define PARENT_OIT "Parent-OID"


/*
 * Structure we use to keep track of the state
 * used for the filter config.
 */

typedef struct fconfig_fdetect {
	char *		name;
	int			lev1;
	int			lev2;
	double		xsize;
	double		ysize;
	double		stride;
	double		scale_mult;
} fconfig_fdetect_t;


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
	CvHidHaarClassifierCascade	*haar_cascade;	
} opencv_fdetect_t;



#define FILTER_TYPE_COLOR 0
#define FILTER_TYPE_TEXTURE 1
#define FILTER_TYPE_ARENA 2

#endif /*  _FACE_H_ */
