/*
 * 	SnapFind (Release 0.9) *      An interactive image search application
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_GABOR_TOOLS_H_
#define	_GABOR_TOOLS_H_	1

#include "gabor.h"

typedef struct gtexture_args
{
	char*               name;
	int                 step;
	int                 xdim;
	int                 ydim;
	int                 min_matches;
	float               max_distance;
	int			num_angles;
	int			num_freq;
	int			radius;
	float		max_freq;
	float		min_freq;
	int                 num_samples;
	float **		response_list;
	gabor *		gobj;
}
gtexture_args_t;


/*
 * An integral image that describes the gabor
 * textures.
 */
typedef struct gabor_ii_img
{
	int			orig_x_size;	/* base image x size */
	int			orig_y_size;	/* base image y size */
	int			x_size;		/* x size of this image */
	int			y_size;		/* y size of the image */
	int			x_offset;	/* convert x to picture coords */
	int			y_offset;	/* convert y to picture coords */
	int			num_resp;	/* # of responses */
	int			resp_size;	/* response size */
	float 		responses[0];
}
gabor_ii_img_t;


/*
 * Some useful macros to compute various sizes and offsets
 * for the variable sized data structures.
 */

#define	NUM_RESPONSES(gargs)	(((gargs)->num_angles) * (gargs->num_freq))

#define	RESPONSE_SIZE(gargs)		\
	(((gargs)->num_angles)*((gargs)->num_freq) * sizeof(float))

#define	II_RESPONSE_OFFSET(gii_img, x, y) \
	(((((y) * (gii_img)->x_size)) + (x)) * (gii_img)->num_resp)


#define GII_PROBE(gii,x,y)                        \
 	((gii)->responses[((y) * ((gii)->x_size) + (x)) * (gii)->num_resp])

#define GII_SIZE(x, y, gargs) \
 	(sizeof(gabor_ii_img_t) + \
	(RESPONSE_SIZE(gargs) * ((x)-2*(gargs)->radius) * \
	((y)-2*(gargs)->radius)))


#ifdef __cplusplus
extern "C"
{
#endif

	int gabor_test_image(gabor_ii_img_t * gii_img, gtexture_args_t * targs,
	                     bbox_list_t * blist);


	void gabor_init_ii_img(int x, int y, gtexture_args_t * gargs,
	                       gabor_ii_img_t * gii_img);

	int gabor_compute_ii_img(RGBImage * img, gtexture_args_t * gargs,
	                         gabor_ii_img_t * gii_img);


	void gabor_response_add(int num_resp, float *res1, float *res2);
	void gabor_response_subtract(int num_resp, float *res1, float *res2);

	int   gabor_patch_response(RGBImage * img, gtexture_args_t * gargs, int
	                           num_resp, float *rvec);


#ifdef __cplusplus
}
#endif

#endif	/* ! _GABOR_TOOLS_H_ */
