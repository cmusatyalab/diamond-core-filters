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


typedef struct {
	int             	scale;
	histo_type_t	type;
} hintegrate_data_t;

struct histo_data {
	histo_config_t *hconfig;
	hintegrate_data_t *hintegrate;
};


/*
 ********************************************************************** */

/*
 * read in volatile state from args. also see patch_spec_write_args 
 */
static int
patch_spec_read_args(histo_config_t * hconfig, int argc,
		     const char * const *args)
{
	int             i, j;
	histo_patch_t   *histo_patch;
	double          sum;
	int             nbins = HBINS * HBINS * HBINS;  /* XXX */

	/*
	 * XXX for now until we allow variable amount of bins 
	 */
	assert(HBINS == hconfig->bins);

	TAILQ_INIT(&hconfig->histo_patchlist);
	for (i = 0; i < hconfig->num_patches; i++) {
		histo_patch = (histo_patch_t *)malloc(sizeof(*histo_patch));
		assert(histo_patch);
		histo_clear(&histo_patch->histo);

		argc -= nbins;
		assert(argc >= 0);

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

	return 1;
}





/*
 ********************************************************************** */
/*
 * Initialize filter to detect histograms.
 */
static int
f_init_histo_detect(int numarg, const char * const *args,
		    int blob_len, const void *blob,
		    const char *fname, histo_config_t **data)
{
	histo_config_t *hconfig;
	int             err;

	/*
	 * filter initialization
	 */
	hconfig = (histo_config_t *)malloc(sizeof(*hconfig));
	assert(hconfig);

	assert(numarg > 6);
	hconfig->name = strdup(fname);
	assert(hconfig->name != NULL);

	hconfig->scale = atof(args[0]);
	hconfig->xsize = atoi(args[1]);
	hconfig->ysize = atoi(args[2]);
	hconfig->stride = atoi(args[3]);
	hconfig->req_matches = atoi(args[4]);
	hconfig->bins = atoi(args[5]);
	hconfig->similarity = atof(args[6]);
	hconfig->distance_type = atoi(args[7]);
	hconfig->type = atoi(args[8]);
	hconfig->num_patches = atoi(args[9]);

	/*
	 * read the histogram patches in 
	 */
	err = patch_spec_read_args(hconfig, numarg - 10, args + 10);
	assert(err);

	/*
	 * save the data pointer 
	 */
	*data = hconfig;
	return (0);
}

static double
f_eval_histo_detect(lf_obj_handle_t ohandle, histo_config_t *hconfig)
{
	int             err;
	RGBImage       *img = NULL;
	size_t   	bsize;
	bbox_list_t	blist;
	int             nhisto;
	float           min_similarity;
	HistoII        *ii = NULL;
	bbox_t *	cur_box;
	int		ii_alloc = 0;
	size_t		len;
	const void    *	dptr;
	double          rv = 0;     /* return value */


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
	histo_scan_image(hconfig->name, img, ii, hconfig,
	                         hconfig->req_matches, &blist);

	save_patches(ohandle, hconfig->name, &blist);


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
		rv = 0;
	} else {
		rv = 100.0 * min_similarity;
	}

	if (ii_alloc) {
		free(ii);
	}

	return rv;
}



static int
f_init_hintegrate(int numarg, const char * const *args,
		  int blob_len, const void *blob,
		  const char *fname, hintegrate_data_t **data)
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

static int
f_eval_hintegrate(lf_obj_handle_t ohandle, hintegrate_data_t *fstate)
{
	RGBImage       *img = NULL;
	HistoII        *ii = NULL;
	int             err;
	size_t          nbytes;
	const void    *	dptr;
	int             width,
	height;
	int             scalebits;
	size_t			len;

	assert(fstate != NULL);
	lf_log(LOGL_TRACE, "f_hintegrate: start");


	/*
	 * get the img 
	 */
	err = lf_ref_attr(ohandle, RGB_IMAGE, &len, &dptr);
	img = (RGBImage *)dptr;

	assert(img);

	/*
	 * initialize a new ii 
	 */
	scalebits = log2_int(fstate->scale);
	width = (img->width >> scalebits) + 1;
	height = (img->height >> scalebits) + 1;
	nbytes = width * height * sizeof(Histo) + sizeof(HistoII);


	ii = (HistoII *) malloc(nbytes); 
	assert(ii);
	ii->nbytes = nbytes;
	ii->width = width;
	ii->height = height;
	ii->scalebits = scalebits;

	histo_compute_ii(img, ii, fstate->scale, fstate->scale, fstate->type);


	err = lf_write_attr(ohandle, HISTO_II, ii->nbytes,
	    (unsigned char *) ii);
	assert(!err);
	lf_omit_attr(ohandle, HISTO_II);
	if (ii) {
		free(ii);
	}
	lf_log(LOGL_TRACE, "f_hintegrate: done");
	return 1;
}


static int f_init_histo(int numarg, const char * const *args,
			int blob_len, const void *blob,
			const char *fname, void **data)
{
	struct histo_data *hdata;
	int ret;

	hdata = malloc(sizeof(*hdata));
	memset(hdata, 0, sizeof(*hdata));
	/* Decide which filter to run based on the argument count */
	if (numarg == 2)
		ret = f_init_hintegrate(numarg, args, blob_len, blob, fname,
					&hdata->hintegrate);
	else
		ret = f_init_histo_detect(numarg, args, blob_len, blob, fname,
					&hdata->hconfig);
	if (ret)
		free(hdata);
	else
		*data = hdata;
	return ret;
}

static double f_eval_histo(lf_obj_handle_t ihandle, void *user_data)
{
	struct histo_data *hdata = user_data;

	if (hdata->hintegrate)
		return f_eval_hintegrate(ihandle, hdata->hintegrate);
	else
		return f_eval_histo_detect(ihandle, hdata->hconfig);
}

LF_MAIN(f_init_histo, f_eval_histo)
