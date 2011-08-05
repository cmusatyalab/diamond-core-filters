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

#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "lib_filter.h"
#include "rgb.h"
#include "lib_results.h"
#include "lib_filimage.h"
#include "gabor_tools.h"
#include "gabor.h"

static void
read_texture_args(const char *fname, gtexture_args_t *data, int argc,
			const char * const *args, const void *blob,
			int blob_len)
{
	example_list_t examples;
	example_patch_t *patch;
	int i, err, patch_size, num_resp;

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


	/* Process examples */
	TAILQ_INIT(&examples);
	load_examples(blob, blob_len, &examples);

	patch_size = 2 * data->radius + 1;
	data->num_samples = 0;
	TAILQ_FOREACH(patch, &examples, link) {
		if ((patch->image->width <= patch_size) ||
		    (patch->image->height <= patch_size)) {
			continue;
		}
		data->num_samples++;
	}
	if (data->num_samples == 0) {
		fprintf(stderr, "No patches of large enough size \n");
		abort();
	}

	data->response_list = (float **) malloc(sizeof(float *) *
				data->num_samples);
	data->gobj = new gabor(data->num_angles, data->radius,
				data->num_freq, data->max_freq,
				data->min_freq);

	num_resp = data->num_angles * data->num_freq;
	i = 0;
	TAILQ_FOREACH(patch, &examples, link) {
		if ((patch->image->width <= patch_size) ||
		    (patch->image->height <= patch_size)) {
			continue;
		}
		data->response_list[i] = (float *) malloc(sizeof(float) *
					num_resp);
		err = gabor_patch_response(patch->image, data, num_resp,
					data->response_list[i]);
		if (err) {
			fprintf(stderr, "get_response failed\n");
			abort();
		}
		i++;
	}

	free_examples(&examples);
}



static int
f_init_gab_texture(int numarg, const char * const *args, int blob_len,
                   const void *blob, const char *fname, void **f_datap)
{
	gtexture_args_t*	data;

	data = (gtexture_args_t *) malloc(sizeof(*data));
	assert(data != NULL);

	read_texture_args(fname, data, numarg, args, blob, blob_len);

	*f_datap = data;
	return(0);
}


static double
f_eval_gab_texture(lf_obj_handle_t ohandle, void *f_datap)
{
	int		pass = 0;
	double		score;
	int		err;
	RGBImage      * rgb_img = NULL;
	const void    * dptr;
	size_t 		bsize;
	size_t 		len;
	float			min_similarity;
	gtexture_args_t  *targs = (gtexture_args_t *)f_datap;
	bbox_list_t		blist;
	bbox_t	*		cur_box;
	gabor_ii_img_t *	gii_img;

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
		min_similarity = 2.0;
		while (!(TAILQ_EMPTY(&blist))) {
			cur_box = TAILQ_FIRST(&blist);
			if ((1.0 - cur_box->distance) < min_similarity) {
				min_similarity = 1.0 - cur_box->distance;
			}
			TAILQ_REMOVE(&blist, cur_box, link);
			free(cur_box);
		}

		if (min_similarity == 2.0) {
			score = 0;
		} else {
			score = 100.0 * min_similarity;
		}
	} else {
		while (!(TAILQ_EMPTY(&blist))) {
			cur_box = TAILQ_FIRST(&blist);
			TAILQ_REMOVE(&blist, cur_box, link);
			free(cur_box);
		}
		score = 0;
	}

	free(gii_img);

	return score;
}

int main(void)
{
	lf_main_double(f_init_gab_texture, f_eval_gab_texture);
	return 0;
}
