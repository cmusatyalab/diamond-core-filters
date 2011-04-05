/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

/*
 * texture filter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "lib_filter.h"
#include "rgb.h"
#include "lib_results.h"
#include "fil_gab_texture.h"
#include "gabor_priv.h"
#include "gabor_tools.h"
#include "gabor.h"

#define VERBOSE 1



static int
read_texture_args(const char *fname, gtexture_args_t *data, int argc, const char * const *args)
{
	int	i,j;
	float *	respv;
	int num_resp;

	data->name = strdup(fname);
	assert(data->name != NULL);

	data->xdim = atoi(*args++);
	data->ydim = atoi(*args++);

	data->step = atoi(*args++);
	data->min_matches = atoi(*args++);
	data->max_distance = (1.0 - atof(*args++));


	data->num_angles = atoi(*args++);
	data->num_freq = atoi(*args++);
	data->radius = atoi(*args++);
	data->max_freq = atof(*args++);
	data->min_freq = atof(*args++);

	data->num_samples = atoi(*args++);
	num_resp = data->num_angles * data->num_freq;

	data->response_list = (float **) malloc(sizeof(float *) *
	                                        data->num_samples);

	for (i=0; i < data->num_samples; i++) {
		respv = (float *) malloc(sizeof(float) * num_resp);
		assert(respv != NULL);
		data->response_list[i] = respv;

		for (j=0; j < num_resp; j++) {
			respv[j] = atof(*args++);
		}
	}

	data->gobj = new gabor(data->num_angles, data->radius, data->num_freq,
	                       data->max_freq, data->min_freq);
	return (0);
}

                                                                                


static int
f_init_gab_texture(int numarg, const char * const *args, int blob_len,
                   const void *blob, const char *fname, void **f_datap)
{
	gtexture_args_t*	data;
	int			err;

	data = (gtexture_args_t *) malloc(sizeof(*data));
	assert(data != NULL);

	err = read_texture_args(fname, data, numarg, args);
	assert(err == 0);

	*f_datap = data;
	return(0);
}


/* XXX */
#define RGB_IMAGE  "_rgb_image.rgbimage"

static int
f_eval_gab_texture(lf_obj_handle_t ohandle, void *f_datap)
{
	int		pass = 0;
	int		err;
	RGBImage      * rgb_img = NULL;
	const void    * dptr;
	size_t 		bsize;
	size_t 		len;
	float			min_simularity;
	gtexture_args_t  *targs = (gtexture_args_t *)f_datap;
	bbox_list_t		blist;
	bbox_t	*		cur_box;
	gabor_ii_img_t *	gii_img;
	search_param_t param;

	lf_log(LOGL_TRACE, "f_texture_detect: enter");

	err = lf_ref_attr(ohandle, RGB_IMAGE, &len, &dptr);
	assert(err == 0);
	rgb_img = (RGBImage *)dptr;
	assert(rgb_img);

	/* Get the gabor II */
	/* XXX move this to a different stage */
	bsize = GII_SIZE(rgb_img->width, rgb_img->height, targs);
	gii_img = (gabor_ii_img_t *)malloc(bsize);
	assert(gii_img != NULL);
	gabor_init_ii_img(rgb_img->width, rgb_img->height, targs, gii_img);

	gabor_compute_ii_img(rgb_img, targs, gii_img);

	/* XXX */

	TAILQ_INIT(&blist);

	pass = gabor_test_image(gii_img, targs, &blist);

	if (pass >= targs->min_matches) {

		save_patches(ohandle, targs->name, &blist);
		min_simularity = 2.0;
		while (!(TAILQ_EMPTY(&blist))) {
			cur_box = TAILQ_FIRST(&blist);
			if ((1.0 - cur_box->distance) < min_simularity) {
				min_simularity = 1.0 - param.distance;
			}
			free(cur_box);
		}

		if (min_simularity == 2.0) {
			pass = 0;
		} else {
			pass = (int)(100.0 * min_simularity);
		}
	} else {
		/* XXX clean list ?? */
		pass = 0;
	}

	free(gii_img);

	char buf[BUFSIZ];
	sprintf(buf, "_texture_detect.int");
	lf_write_attr(ohandle, buf, sizeof(int), (unsigned char *)&pass);

	return pass;
}

LF_MAIN(f_init_gab_texture, f_eval_gab_texture)
