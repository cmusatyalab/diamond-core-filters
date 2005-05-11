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
 * texture filter
 */

#include <opencv/cv.h>
#include <stdio.h>
#include "face.h"
#include "filter_api.h"
#include "fil_file.h"
#include "fil_gab_texture.h"
#include "fil_tools.h"
#include "texture_tools.h"
#include "image_tools.h"
#include "gabor_tools.h"
#include "gabor.h"

#define VERBOSE 1



static int
read_texture_args(lf_fhandle_t fhandle, gtexture_args_t *data,
                  int argc, char **args)
{
	int	i,j;
	float *	respv;
	int num_resp;

	data->name = strdup(*args++);
	assert(data->name != NULL);

	data->scale = atof(*args++);
	/* XXX fix this later , eat box sizes ...*/
	args++;
	args++;

	data->step = atoi(*args++);
	data->min_matches = atoi(*args++);
	data->max_distance = (1.0 - atof(*args++));


	data->num_angles = atoi(*args++);
	data->num_freq = atoi(*args++);
	data->radius = atoi(*args++);
	data->max_freq = atof(*args++);
	data->min_freq = atof(*args++);
	data->sigma = atof(*args++);
		

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
		data->max_freq, data->min_freq, data->sigma);
	return (0);
}


static void
write_notify_f(void *cont, search_param_t *param)
{
	write_notify_context_t *context = (write_notify_context_t *)cont;
	write_param(context->fhandle, context->ohandle, HISTO_BBOX_FMT, param, param->id);
}


int
f_init_gab_texture(int numarg, char **args, int blob_len,
                      void *blob, void **f_datap)
{
	gtexture_args_t*	data;
	lf_fhandle_t 	fhandle = 0; /* XXX */
	int				err;


	err = lf_alloc_buffer(fhandle, sizeof(*data), (char **)&data);
	assert(!err);

	err = read_texture_args(fhandle, data, numarg, args);
	assert(err == 0);

	*f_datap = data;
	return(0);
}


int
f_fini_gab_texture(void *f_datap)
{
	gtexture_args_t  *data = (gtexture_args_t *)f_datap;
	int		i;

	delete data->gobj;

	for (i=0; i < data->num_samples; i++) {
		free(data->response_list[i]);
	}
	free(data->response_list);
	free(data);
	return(0);
}





int
f_eval_gab_texture(lf_obj_handle_t ohandle, int numout,
                      lf_obj_handle_t *ohandles, void *f_datap)
{
	int		pass = 0;
	int		err;
	RGBImage      * rgb_img = NULL;
	off_t 		bsize;
	off_t 		len;
	float			min_simularity;
	lf_fhandle_t 	fhandle = 0; /* XXX */
	gtexture_args_t  *targs = (gtexture_args_t *)f_datap;
	bbox_list_t		blist;
	bbox_t	*		cur_box;
	int				i;
	int			rgb_alloc = 0;
	search_param_t param;
	int ntexture;

	lf_log(fhandle, LOGL_TRACE, "f_texture_detect: enter");

	err = lf_ref_attr(fhandle, ohandle, RGB_IMAGE, &len, (char**)&rgb_img);
	assert(err == 0);
	if (rgb_img == NULL) {
		rgb_alloc = 1;
		rgb_img = get_rgb_img(ohandle);
	}
	assert(rgb_img);



	TAILQ_INIT(&blist);

	pass = gabor_test_image(rgb_img, targs, &blist);

	if (pass >= targs->min_matches) {

		/* increase num_histo counter (for boxes in app)*/
		int num_histo = 0;
		bsize = sizeof(int);
		err = lf_read_attr(fhandle, ohandle, NUM_HISTO, &bsize, (char *)&num_histo);
		if (err)
			num_histo=0;

		/* increase the ntexture counter */
		bsize = sizeof(int);
		err = lf_read_attr(fhandle, ohandle, NUM_TEXTURE, &bsize, (char *)&ntexture);
		if(err)
			ntexture = 0;
		ntexture= ntexture+pass;
		err = lf_write_attr(fhandle, ohandle, NUM_TEXTURE, sizeof(int), (char *)&ntexture);
		assert(!err);


		min_simularity = 2.0;
		i = num_histo;
		while (!(TAILQ_EMPTY(&blist))) {
			cur_box = TAILQ_FIRST(&blist);
			param.type = PARAM_HISTO;  //temporary hack
			param.bbox.xmin = cur_box->min_x;
			param.bbox.ymin = cur_box->min_y;
			param.bbox.xsiz = cur_box->max_x - cur_box->min_x;
			param.bbox.ysiz = cur_box->max_y - cur_box->min_y;
			param.distance = cur_box->distance;

			if ((1.0 - param.distance) < min_simularity) {
				min_simularity = 1.0 - param.distance;
			}

			strncpy(param.name, targs->name, PARAM_NAME_MAX);
			param.name[PARAM_NAME_MAX] = '\0';
			param.id = i;
			write_notify_context_t context;
			context.fhandle = fhandle;
			context.ohandle = ohandle;
			write_notify_f(&context, &param);
			TAILQ_REMOVE(&blist, cur_box, link);
			free(cur_box);
			i++;
		}

		/* write out the update number of matches, XXX clean this up soon */
		num_histo = num_histo+pass;
		err = lf_write_attr(fhandle, ohandle, NUM_HISTO, sizeof(int), (char *)&num_histo);
		assert(!err);

		if (min_simularity == 2.0) {
			pass = 0;
		} else {
			pass = (int)(100.0 * min_simularity);
		}
	} else {
		pass = 0;
	}

	if (rgb_alloc) {
		ft_free(fhandle, (char*)rgb_img);
	}

	char buf[BUFSIZ];
	sprintf(buf, "_texture_detect.int");
	lf_write_attr(fhandle, ohandle, buf, sizeof(int), (char *)&pass);

	return pass;
}
