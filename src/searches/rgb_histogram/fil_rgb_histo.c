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
 * color histogram filter
 * rgb reader
 */

#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#include "lib_results.h"
#include "lib_sfimage.h"
#include "lib_filter.h"
#include "snapfind_consts.h"
#include "rgb.h"
#include "rgb_histo.h"
#include "fil_rgb_histo.h"


typedef struct {
	int             	scale;
	histo_type_t	type;
} hintegrate_data_t;

typedef struct {
	int             num_hist;
} hpass_data_t;



#define ASSERT(exp)							\
if(!(exp)) {								\
  lf_log(LOGL_ERR, "Assertion %s failed at ", #exp);		\
  lf_log(LOGL_ERR, "%s, line %d.", __FILE__, __LINE__);	\
  pass = -1;								\
  goto done;								\
}



/*
 ********************************************************************** */

/*
 * read in volatile state from args. also see patch_spec_write_args 
 */
static int
patch_spec_read_args(histo_config_t * hconfig, int argc, char **args)
{
	int             i, j;
	histo_patch_t   *histo_patch;
	double          sum;
	int             pass = 1;
	int             nbins = HBINS * HBINS * HBINS;  /* XXX */

	/*
	 * XXX for now until we allow variable amount of bins 
	 */
	assert(HBINS == hconfig->bins);

	TAILQ_INIT(&hconfig->histo_patchlist);
	for (i = 0; i < hconfig->num_patches; i++) {
		histo_patch = (histo_patch_t *)malloc(sizeof(*histo_patch));
		ASSERT(histo_patch);
		histo_clear(&histo_patch->histo);

		argc -= nbins;
		ASSERT(argc >= 0);

		sum = 0.0;
		for (j = 0; j < nbins; j++) {
			histo_patch->histo.data[j] = atof(*args++);
			sum += histo_patch->histo.data[j];
		}
		histo_patch->histo.weight = 1.0;
		/*
		 * any errors should be close to 1.0 
		 */
		assert(fabs(sum - 1.0) < 0.001);
		TAILQ_INSERT_TAIL(&hconfig->histo_patchlist, histo_patch, link);
	}

done:
	return pass;
}





/*
 ********************************************************************** */
/*
 * Initialize filter to detect histograms.
 */
int
f_init_histo_detect(int numarg, char **args, int blob_len,
                    void *blob, const char *fname, void **data)
{
	histo_config_t *hconfig;
	int             err;

	/*
	 * filter initialization
	 */
	hconfig = (histo_config_t *)malloc(sizeof(*hconfig));
	assert(hconfig);

	assert(numarg > 6);
	hconfig->name = strdup(args[0]);
	assert(hconfig->name != NULL);

	hconfig->scale = atof(args[1]);
	hconfig->xsize = atoi(args[2]);
	hconfig->ysize = atoi(args[3]);
	hconfig->stride = atoi(args[4]);
	hconfig->req_matches = atoi(args[5]);
	hconfig->bins = atoi(args[6]);
	hconfig->simularity = atof(args[7]);
	hconfig->distance_type = atoi(args[8]);
	hconfig->type = atoi(args[9]);
	hconfig->num_patches = atoi(args[10]);

	/*
	 * read the histogram patches in 
	 */
	err = patch_spec_read_args(hconfig, numarg - 11, args + 11);
	assert(err);

	/*
	 * save the data pointer 
	 */
	*data = (void *) hconfig;
	return (0);
}

int
f_fini_histo_detect(void *data)
{
	histo_patch_t        *histo_patch;
	histo_config_t *hconfig = (histo_config_t *) data;

	while ((histo_patch = TAILQ_FIRST(&hconfig->histo_patchlist))) {
		TAILQ_REMOVE(&hconfig->histo_patchlist, histo_patch, link);
		free(histo_patch);
	}
	free(hconfig);

	return (0);
}


int
f_eval_histo_detect(lf_obj_handle_t ohandle, void *f_data)
{
	int             pass = 0;
	int             err;
	RGBImage       *img = NULL;
	size_t   	bsize;
	bbox_list_t	blist;
	histo_config_t *hconfig = (histo_config_t *) f_data;
	int             nhisto;
	float           min_simularity;
	HistoII        *ii = NULL;
	bbox_t *	cur_box;
	int		ii_alloc = 0;
	size_t		len;
	unsigned char *	dptr;
	int             rv = 0;     /* return value */


	lf_log(LOGL_TRACE, "f_histo_detect: enter");

	/*
	 * get the img 
	 */
	err = lf_ref_attr(ohandle, RGB_IMAGE, &len, &dptr);
	img = (RGBImage *)dptr;

	err = lf_ref_attr(ohandle, HISTO_II, &len, &dptr);
	ii = (HistoII *)dptr;
	if (err != 0) {
		ii_alloc = 1;
		ii = histo_get_ii(hconfig, img);
	}

	/*
	 * get nhisto 
	 */
	bsize = sizeof(int);
	err = lf_read_attr(ohandle, NUM_HISTO, &bsize, 
	    (unsigned char *)&nhisto);
	if (err) {
		nhisto = 0;             /* XXX */
	}


	/*
	 * scan the image
	 */

	TAILQ_INIT(&blist);
	pass = 	histo_scan_image(hconfig->name, img, ii, hconfig,
	                         hconfig->req_matches, &blist);

	save_patches(ohandle, hconfig->name, &blist);


	min_simularity = 2.0;
	while (!(TAILQ_EMPTY(&blist))) {
		cur_box = TAILQ_FIRST(&blist);
		if ((1.0 - cur_box->distance) < min_simularity) {
			min_simularity = 1.0 - cur_box->distance;
		}
		TAILQ_REMOVE(&blist, cur_box, link);
		free(cur_box);
	}

	if (min_simularity == 2.0) {
		rv = 0;
	} else {
		rv = (int)(100.0 * min_simularity);
	}

	if (ii_alloc) {
		ft_free((char *) ii);
	}

	return rv;
}


int
f_init_hpass(int numarg, char **args, int blob_len, void *blob, 
		const char *fname, void **data)
{
	hpass_data_t   *fstate;

	fstate = (hpass_data_t *) malloc(sizeof(*fstate));
	if (fstate == NULL) {
		/*
		 * XXX log 
		 */
		return (ENOMEM);
	}

	assert(numarg == 1);
	fstate->num_hist = atoi(args[0]);

	*data = fstate;
	return (0);
}


int
f_fini_hpass(void *data)
{
	hpass_data_t   *fstate = (hpass_data_t *) data;
	free(fstate);

	return (0);
}


int
f_eval_hpass(lf_obj_handle_t ohandle, void *f_data)
{
	int             nhisto;
	int             err,
	pass;
	size_t           bsize;
	hpass_data_t   *fstate = (hpass_data_t *) f_data;
	;


	/*
	 * get nhisto 
	 */
	bsize = sizeof(int);
	err = lf_read_attr(ohandle, NUM_HISTO, &bsize, 
			(unsigned char *) &nhisto);
	ASSERT(!err);

	pass = (nhisto >= fstate->num_hist);

done:
	return pass;
}


int
f_init_hintegrate(int numarg, char **args, int blob_len, void *blob,
		const char *fname, void **data)
{
	hintegrate_data_t *fstate;

	fstate = (hintegrate_data_t *) malloc(sizeof(*fstate));
	if (fstate == NULL) {
		/*
		 * XXX log 
		 */
		assert(0);
		return (ENOMEM);
	}

	/*
	 * read args 
	 */
	assert(numarg == 2);
	fstate->scale = atoi(args[0]);
	fstate->type = atoi(args[1]);
	// printf("fstate !!! %p \n", *data);
	*data = fstate;
	return (0);
}

int
f_fini_hintegrate(void *data)
{
	hintegrate_data_t *fstate = (hintegrate_data_t *) data;
	free(fstate);
	return (0);
}



int
f_eval_hintegrate(lf_obj_handle_t ohandle, void *f_data)
{
	int             pass = 1;
	RGBImage       *img = NULL;
	HistoII        *ii = NULL;
	int             err;
	size_t          nbytes;
	unsigned char *	dptr;
	hintegrate_data_t *fstate = (hintegrate_data_t *) f_data;
	int             width,
	height;
	int             scalebits;
	size_t			len;

	assert(f_data != NULL);
	// printf("f_data: %p \n", f_data);
	lf_log(LOGL_TRACE, "f_hintegrate: start");


	/*
	 * get the img 
	 */
	err = lf_ref_attr(ohandle, RGB_IMAGE, &len, &dptr);
	img = (RGBImage *)dptr;

	ASSERT(img);

	/*
	 * initialize a new ii 
	 */
	scalebits = log2_int(fstate->scale);
	width = (img->width >> scalebits) + 1;
	height = (img->height >> scalebits) + 1;
	nbytes = width * height * sizeof(Histo) + sizeof(HistoII);


	ii = (HistoII *) malloc(nbytes); 
	ASSERT(ii);
	ii->nbytes = nbytes;
	ii->width = width;
	ii->height = height;
	ii->scalebits = scalebits;

	histo_compute_ii(img, ii, fstate->scale, fstate->scale, fstate->type);


	err = lf_write_attr(ohandle, HISTO_II, ii->nbytes,
	    (unsigned char *) ii);
	ASSERT(!err);
done:
	if (ii) {
		free(ii);
	}
	lf_log(LOGL_TRACE, "f_hintegrate: done");
	return pass;
}


