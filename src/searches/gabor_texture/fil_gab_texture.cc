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
	/* XXX size of the radius?? */
	data->box_width = atoi(*args++);
	data->box_height = atoi(*args++);

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

	/* XXX hack for now */
	data->box_width = 2 * data->radius + 1;
	data->box_height = 2 * data->radius + 1;

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
	texture_args_t  *targs = (texture_args_t *)f_datap;
	lf_fhandle_t 	fhandle = 0; /* XXX */
	int		i;

	for (i=0; i<targs->num_samples; i++) {
		free(targs->sample_values[i]);
	}
	free(targs->sample_values);
	lf_free_buffer(fhandle, (char*)targs);
	return(0);
}


static float
vsum(int num, float *vec)
{
	float	sum = 0.0;
	int		i;

	for (i=0; i < num;i++) {
		sum += vec[i];	
	}
	return(sum);

}

/* XXX make more efficient */

static float
comp_distance(int num_resp, float * new_vec, float *orig_vec)
{
	float	osum, nsum;
	float	running = 0.0;
	int		i;

	osum = vsum(num_resp, orig_vec);
	nsum = vsum(num_resp, new_vec);

	for (i=0; i < num_resp; i++) {
		running += fabsf(orig_vec[i]/osum - new_vec[i]/nsum);
	}
	running = running/(float)num_resp;
	return(running);
}

static int
gabor_test_image(RGBImage * img, gtexture_args_t * targs, bbox_list_t * blist)
{

	/*
	 * first process entire image 
	 */
	int		num_resp;
	double          min_distance;   // min distance for one window from all
	// samples
	int             passed = 0;
	int             i, x, y;
	int             test_x, test_y;
	float *			respv;
	int		err;
	float		dist;
	bbox_t         *bbox;
	int             quit_on_pass = 1;   // quits as soon as its known that


	num_resp = targs->num_angles * targs->num_freq;
	respv = (float *)malloc(sizeof(float)*num_resp);
	assert(respv != NULL);

	/*
	 * test each subwindow 
	 */
	for (x = 0; (x + targs->box_width) < img->width; x += targs->step) {
		for (y = 0; (y + targs->box_height) < img->height; y += targs->step) {
			test_x = (int) targs->box_width;
			test_y = (int) targs->box_height;

			/* XXX scale ?? */

			err = targs->gobj->get_responses(img, x, y, num_resp, respv);
			assert(err == 0);

			min_distance = 2000.0;
			for (i=0; i < targs->num_samples; i++) {
				dist = comp_distance(num_resp, respv, targs->response_list[i]);
				if (dist < min_distance) {
					min_distance = dist;
				}
			}

			if (min_distance <= targs->max_distance) {
					passed++;
					bbox = (bbox_t *) malloc(sizeof(*bbox));
					assert(bbox != NULL);
					bbox->min_x = x;
					bbox->min_y = y;
					bbox->max_x = x + test_x;   /* XXX scale */
					bbox->max_y = y + test_y;
					bbox->distance = min_distance;
					TAILQ_INSERT_TAIL(blist, bbox, link);

					if (quit_on_pass && (passed >= targs->min_matches)) {
						goto done;
					}
			}
		}
	}

done:
	free(respv);
	return (passed);
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
			printf("min unchange \n");
		} else {
			pass = (int)(100.0 * min_simularity);
			//printf("min change \n");
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

