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
 * texture filter
 */

#include <opencv/cv.h>
#include <stdio.h>
#include "face.h"
#include "filter_api.h"
#include "fil_file.h"
#include "fil_texture.h"
#include "fil_tools.h"
#include "texture_tools.h"
#include "image_tools.h"

#define VERBOSE 1

#define ASSERT(exp)							\
if(!(exp)) {								\
  lf_log(fhandle, LOGL_ERR, "Assertion %s failed at ", #exp);		\
  lf_log(fhandle, LOGL_ERR, "%s, line %d.", __FILE__, __LINE__);	\
  pass = 0;								\
  goto done;								\
}


static int
read_texture_args(lf_fhandle_t fhandle, texture_args_t *texture_args, 
				int argc, char **args) 
{

	int pass = 0;
	int s_index, f_index;

	texture_args->name = strdup(args[0]);
	assert(texture_args->name != NULL);

	texture_args->scale = atof(args[1]);
	texture_args->box_width = atoi(args[2]);
	texture_args->box_height = atoi(args[3]);
  	texture_args->step = atoi(args[4]);
  	texture_args->min_matches = atoi(args[5]);
	// <-- for normalized
 	//texture_args->max_distance = (1.0 - atof(args[6])) * NUM_LAP_PYR_LEVELS; 
 	texture_args->max_distance = (1.0 - atof(args[6]));

  	texture_args->num_channels = atoi(args[7]);
  	
	texture_args->texture_distance = (texture_dist_t) atoi(args[8]);

	texture_args->num_samples = atoi(args[9]);
	/* read in the arguments for each sample */


	args += 10;

  	texture_args->sample_values = new (double*)[texture_args->num_samples];
  	for (s_index = 0; s_index < texture_args->num_samples; s_index++) {
    	texture_args->sample_values[s_index] =
			new double[NUM_LAP_PYR_LEVELS*TEXTURE_MAX_CHANNELS];
    	for (f_index = 0; f_index < NUM_LAP_PYR_LEVELS*texture_args->num_channels; 
			f_index++) {
      			texture_args->sample_values[s_index][f_index] = atof(*args);
      			args++;
    	}
  	}
  	pass = 1;
  	return pass;
}


static void
write_notify_f(void *cont, search_param_t *param) {
  write_notify_context_t *context = (write_notify_context_t *)cont;
  write_param(context->fhandle, context->ohandle, HISTO_BBOX_FMT, param, param->id);
}


int
f_init_texture_detect(int numarg, char **args, int blob_len,
		void *blob, void **f_datap) 
{
	texture_args_t*	targs;
  	lf_fhandle_t 	fhandle = 0; /* XXX */
	int				err;

	err = lf_alloc_buffer(fhandle, sizeof(*targs), (char **)&targs);
  	assert(!err);
  
  	err = read_texture_args(fhandle, targs, numarg, args);
  	assert(err);

	*f_datap = targs;
	return(0);
} 


int
f_fini_texture_detect(void *f_datap)
{
	texture_args_t  *targs = (texture_args_t *)f_datap;
  	lf_fhandle_t 	fhandle = 0; /* XXX */
	int		i;

	for (i=0; i<targs->num_samples; i++) {
		delete[] targs->sample_values[i];
	}
	delete[] targs->sample_values;
	lf_free_buffer(fhandle, (char*)targs);
	return(0);
}


int
f_eval_texture_detect(lf_obj_handle_t ohandle, int numout, 
		lf_obj_handle_t *ohandles, void *f_datap)
{

  int		pass = 0;
  int		err;
  IplImage 	*img = NULL;
  IplImage 	*dst_img = NULL;
  RGBImage      * rgb_img = NULL;
  off_t 		bsize;
  float			min_simularity;
  lf_fhandle_t 	fhandle = 0; /* XXX */
  texture_args_t  *targs = (texture_args_t *)f_datap;
  bbox_list_t		blist;
  bbox_t	*		cur_box;
	int				i;
  search_param_t param;
  int ntexture;

  lf_log(fhandle, LOGL_TRACE, "f_texture_detect: enter");


  rgb_img = (RGBImage*)ft_read_alloc_attr(fhandle, ohandle, RGB_IMAGE);
  if (rgb_img == NULL) {
    rgb_img = get_rgb_img(ohandle);
  }
  ASSERT(rgb_img);


  if (targs->num_channels == 1) {
    img = get_gray_ipl_image(rgb_img);
  } else if (targs->num_channels == 3) {
    img = get_rgb_ipl_image(rgb_img);
  }
  ASSERT(img);

  dst_img = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 1);

  TAILQ_INIT(&blist); 

  if (targs->texture_distance == TEXTURE_DIST_MAHOLONOBIS) {
  	pass = texture_test_entire_image_maholonobis(img, targs, &blist);
  } else if (targs->texture_distance == TEXTURE_DIST_VARIANCE) {
  	pass = texture_test_entire_image_variance(img, targs, &blist);
  } else if (targs->texture_distance == TEXTURE_DIST_PAIRWISE) {
  	pass = texture_test_entire_image_pairwise(img, targs, &blist);
  } else {
	assert(0);
  }
  if (pass >= targs->min_matches) {

  	/* increase num_histo counter (for boxes in app)*/
    int num_histo = 0;
    bsize = sizeof(int);
    err = lf_read_attr(fhandle, ohandle, NUM_HISTO, &bsize, (char *)&num_histo);
    if (err) num_histo=0;

    /* increase the ntexture counter */
    bsize = sizeof(int);
    err = lf_read_attr(fhandle, ohandle, NUM_TEXTURE, &bsize, (char *)&ntexture);
    if(err) ntexture = 0;     
    ntexture= ntexture+pass;
    err = lf_write_attr(fhandle, ohandle, NUM_TEXTURE, sizeof(int), (char *)&ntexture);
    ASSERT(!err);


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
    ASSERT(!err);

  	if (min_simularity == 2.0) {
		pass = 0;
		printf("min unchange \n");
  	} else {
		pass = (int)(100.0 * min_simularity);
		printf("min change \n");
  	}
  } else {
	pass = 0;
  }
done:
  
  if (dst_img) cvReleaseImage(&dst_img);

  if (img) {
    cvReleaseImage(&img);
  }
  if (rgb_img) {
    ft_free(fhandle, (char*)rgb_img);
  }
  
  char buf[BUFSIZ];
  sprintf(buf, "_texture_detect.int");
  lf_write_attr(fhandle, ohandle, buf, sizeof(int), (char *)&pass);

  return pass;
}

/* XXX move elsewhere */
typedef struct {
    int     num_req;
} tpass_data_t;


int 
f_init_tpass(int numarg, char **args, int blob_len, void *blob,
			void **f_datap)	
{
	tpass_data_t *  fstate;
                                                                                
    fstate = (tpass_data_t *)malloc(sizeof(*fstate));
    if (fstate == NULL) {
        /* XXX log */
        return(ENOMEM);
    }
                                                                                
    assert(numarg == 1);
    fstate->num_req = atoi(args[0]);
                                                                                
    *f_datap = fstate;
    return(0);
} 

int 
f_fini_tpass(void *f_datap)	
{

	tpass_data_t *  fstate = (tpass_data_t *)f_datap;
	free(fstate);
    return(0);
}



int 
f_eval_tpass(lf_obj_handle_t ohandle, int numout, lf_obj_handle_t *ohandles,
			void *f_data)
{
	int 				ntexture;
	tpass_data_t*		tdata = (tpass_data_t *)f_data;
	lf_fhandle_t		fhandle = 0; /* XXX */
	int 				err, pass;
	off_t           	bsize;

	/* get ntexture */
	bsize = sizeof(int);
	err = lf_read_attr(fhandle, ohandle, NUM_TEXTURE, &bsize, (char *)&ntexture);
	ASSERT(!err);

	pass = (ntexture >= tdata->num_req);

done:  

	char buf[BUFSIZ];
	sprintf(buf, "_texture_pass.int");
	lf_write_attr(fhandle, ohandle, buf, sizeof(int), (char *)&pass);

	return pass;
}
