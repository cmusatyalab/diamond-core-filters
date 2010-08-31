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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "lib_filter.h"
#include "lib_results.h"


void
save_patches(lf_obj_handle_t ohandle, char *fname, bbox_list_t *blist)
{
        char            buf[BUFSIZ];
	int		err;
	int		count = 0;
	bbox_t *	cur_box;
	img_patches_t *	ipatch;
	double 		min_dist = 2.0;

	TAILQ_FOREACH(cur_box, blist, link) {
		count++;
	}

	ipatch = (img_patches_t *)malloc(IMG_PATCH_SZ(count));
	assert(ipatch != NULL);
	ipatch->num_patches = count;

	count = 0;
	TAILQ_FOREACH(cur_box, blist, link) {
		ipatch->patches[count].min_x = cur_box->min_x;
		ipatch->patches[count].min_y = cur_box->min_y;
		ipatch->patches[count].max_x = cur_box->max_x;
		ipatch->patches[count].max_y = cur_box->max_y;
		if (cur_box->distance < min_dist) {
			min_dist = cur_box->distance;
		}
		count++;
	}
	ipatch->distance = min_dist;

        sprintf(buf, FILTER_MATCHES, fname);
        err = lf_write_attr(ohandle, buf, IMG_PATCH_SZ(count), 
	   (unsigned char *)ipatch);
	assert(err == 0);
	free(ipatch);
}
