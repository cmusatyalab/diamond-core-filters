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

#include <opencv/cv.h> 
#include <stdio.h>
#include <math.h>
#include "face.h"
#include "rgb.h"
#include "filter_api.h"
#include "fil_file.h"
#include "image_tools.h"
#include "fil_tools.h"
#include "gabor_tools.h"
#include "texture_tools.h"
//#include "fil_gab_texture.h"
#include "gabor.h"


static void
dump_gtexture_args(gtexture_args_t *gargs)
{
	int	i,j;
	float * farray;
	fprintf(stderr, "xdim - %d\n", gargs->xdim);
	fprintf(stderr, "ydim - %d\n", gargs->ydim);
	fprintf(stderr, "step - %d\n", gargs->step);
	fprintf(stderr, "min_match - %d\n", gargs->min_matches);
	fprintf(stderr, "max_dist - %f\n", gargs->max_distance);
	fprintf(stderr, "num_angles - %d\n", gargs->num_angles);
	fprintf(stderr, "num_freq - %d\n", gargs->num_freq);
	fprintf(stderr, "radius - %d\n", gargs->radius);
	fprintf(stderr, "max_freq - %f\n", gargs->max_freq);
	fprintf(stderr, "min_freq - %f\n", gargs->min_freq);
	fprintf(stderr, "samples - %d\n", gargs->num_samples);

	for (i=0; i < gargs->num_samples; i++) {
		farray = gargs->response_list[i];	
		fprintf(stderr, "sample %d: ", i);
		for (j=0; j<(gargs->num_freq * gargs->num_angles); j++) {
			fprintf(stderr, "%f ",	farray[j]);
		}
		fprintf(stderr, "\n");
	}
}

static float
gabor_vsum(int num, float *vec)
{
	float	sum = 0.0;
	int		i;

	for (i=0; i < num;i++) {
		sum += vec[i];	
	}
	return(sum);
}

static void
dump_single_respv(int num_resp, float *new_vec)
{
	int	i;
	fprintf(stderr, "new: ");
	for (i=0; i < num_resp; i++) {
		fprintf(stderr, "%f ", new_vec[i]);
	}
	fprintf(stderr, "\n");
}

static void
dump_respv(int num_resp, float *new_vec, float *orig_vec)
{
	int	i;
	fprintf(stderr, "new: ");
	for (i=0; i < num_resp; i++) {
		fprintf(stderr, "%f ", new_vec[i]);
	}
	fprintf(stderr, "\n");

	fprintf(stderr, "old: ");
	for (i=0; i < num_resp; i++) {
		fprintf(stderr, "%f ", orig_vec[i]);
	}
	fprintf(stderr, "\n");
}



/* XXX make more efficient */

static float
gabor_comp_distance(int num_resp, float * new_vec, float *orig_vec)
{
	float	osum, nsum;
	float	running = 0.0;
	int		i;

	osum = gabor_vsum(num_resp, orig_vec);
	nsum = gabor_vsum(num_resp, new_vec);

	for (i=0; i < num_resp; i++) {
		running += fabsf(orig_vec[i]/osum - new_vec[i]/nsum);
	}
	return(running);
}


static void
gabor_get_ii_response(gabor_ii_img_t * gii_img, int x, int y, 
	int xsize, int ysize, int vecsz, float *rvec)
{

	assert(vecsz == gii_img->num_resp);

	memcpy(&GII_PROBE(gii_img, x, y), rvec, gii_img->resp_size);
	gabor_response_add(gii_img->num_resp, rvec, 
				&GII_PROBE(gii_img, (x+xsize), (y+ysize)));
	gabor_response_subtract(gii_img->num_resp, rvec, 
				&GII_PROBE(gii_img, (x+xsize), y));
	gabor_response_subtract(gii_img->num_resp, rvec, 
				&GII_PROBE(gii_img, x, (y+ysize)));
}



int
gabor_test_image(gabor_ii_img_t * gii_img, gtexture_args_t * gargs, 
	bbox_list_t * blist)
{
	int				num_resp;
	float          min_distance; 
	int             passed = 0;
	int             i, x, y;
	float *			respv;
	float		dist;
	bbox_t         *bbox;
	bbox_t          best_box;

	best_box.distance = 500000.0;

	num_resp = NUM_RESPONSES(gargs);
	respv = (float *)malloc(RESPONSE_SIZE(gargs));
	assert(respv != NULL);

	/*
	 * test each subwindow 
	 */
	/* XXX scale ?? */
	for (y = 0; (y + gargs->ydim) < gii_img->y_size; y += gargs->step) {
    		for (x = 0; (x + gargs->xdim) < gii_img->x_size; x += gargs->step) {
			gabor_get_ii_response(gii_img, x, y, gargs->xdim,
				gargs->ydim, num_resp, respv);

			min_distance = 2000.0;
			for (i=0; i < gargs->num_samples; i++) {
				dist = gabor_comp_distance(num_resp, respv, 
					gargs->response_list[i]);
				if (dist < min_distance) {
					min_distance = dist;
				}
			}

			if ((gargs->min_matches == 1) &&
				(min_distance <= gargs->max_distance) &&
				(min_distance < best_box.distance)) {
				best_box.min_x = x + gii_img->x_offset;
				best_box.min_y = y + gii_img->y_offset;
				best_box.max_x = best_box.min_x + gargs->xdim;
				best_box.max_y = best_box.min_y + gargs->ydim;
				best_box.distance = min_distance;
			} else if ((gargs->min_matches > 1) &&
					   (min_distance <= gargs->max_distance)) {
				passed++;
				bbox = (bbox_t *) malloc(sizeof(*bbox));
				assert(bbox != NULL);
				bbox->min_x = x + gii_img->x_offset;
				bbox->min_y = y + gii_img->y_offset;
				bbox->max_x = bbox->min_x + gargs->xdim;
				bbox->max_y = bbox->min_y + gargs->ydim;
				bbox->distance = min_distance;
				TAILQ_INSERT_TAIL(blist, bbox, link);
																			
				if (passed >= gargs->min_matches) {
					goto done;
				}
			}
		}
	}

    if ((gargs->min_matches == 1)
        && (best_box.distance <= gargs->max_distance)) {
        passed++;
        bbox = (bbox_t *) malloc(sizeof(*bbox));
        assert(bbox != NULL);
        bbox->min_x = best_box.min_x;
        bbox->min_y = best_box.min_y;
        bbox->max_x = best_box.max_x;
        bbox->max_y = best_box.max_y;
        bbox->distance = best_box.distance;
        TAILQ_INSERT_TAIL(blist, bbox, link);
    }

done:
	free(respv);
	return (passed);
}

/* 
 * returns the average responses over patch.
 */
int
gabor_patch_response(RGBImage * img, gtexture_args_t * gargs, int
	num_resp, float *rvec)
{
	int             passed = 0;
	int              x, y;
	float *			respv;
	int		err;
	int		width;
	int				patches;

	/* make sure we have the correct amount of space */
	assert(NUM_RESPONSES(gargs) == num_resp);


	/* make sure the patch is big enough */
	width = gargs->radius *2 + 1;

	if ((img->width < width) || (img->height < width)) {
		return(1);
	}

	respv = (float *)malloc(RESPONSE_SIZE(gargs));
	assert(respv != NULL);

	memset(rvec, 0, RESPONSE_SIZE(gargs));


	/* test each subwindow and sum them */
	patches = 0;
	for (x = 0; (x + width) < img->width; x++) {
		for (y = 0; (y + width) < img->height; y++) {

			/* XXX scale ?? */

			err = gargs->gobj->get_responses(img, x, y, num_resp, respv, 1);
			assert(err == 0);
			gabor_response_add(num_resp, rvec, respv);
			patches++;

		}
	}

	fprintf(stderr, "patch_response: loop %d \n", patches);
	dump_single_respv(num_resp, rvec);

	/* normalize the values */
	for (x=0; x < num_resp; x++) {
		rvec[x] = rvec[x]/((float)patches);
	}
	dump_single_respv(num_resp, rvec);

	free(respv);
	return (passed);
}



void
gabor_init_ii_img(int x, int y, gtexture_args_t * gargs, 
	gabor_ii_img_t * gii_img)
{

	gii_img->orig_x_size = x;
	gii_img->orig_y_size = y;

	gii_img->x_size = x - 2*gargs->radius;
	gii_img->y_size = y - 2*gargs->radius; 
	gii_img->x_offset = gargs->radius - 1;
	gii_img->y_offset = gargs->radius - 1;
	gii_img->num_resp = gargs->num_freq * gargs->num_angles;
	gii_img->resp_size = gii_img->num_resp * sizeof(float);

}

/* add response res2 to res1 and store the result in res1 */

void
gabor_response_add(int num_resp, float *res1, float *res2)
{
	int	i;
	for (i=0; i < num_resp; i++) {
		res1[i] += res2[i];
	}
}


/* subtract response res2 from res1 and store the result in res1 */
void
gabor_response_subtract(int num_resp, float *res1, float *res2)
{
	int	i;
	for (i=0; i < num_resp; i++) {
		res1[i] -= res2[i];
	}
}


int
gabor_compute_ii_img(RGBImage * img, gtexture_args_t * gargs, 
	gabor_ii_img_t * gii_img)
{
	int             x, y;
	int				xoff, yoff;
	float *			respv;
	int				err;
	int				width;

	respv = (float *)malloc(gii_img->resp_size);
	assert(respv != NULL);

	width = gargs->radius *2 + 1;

	/* clear the first row and column */
	memset(respv, 0, gii_img->resp_size);
	for (x=0; (x + width) < img->width; x++) {
		memcpy(&GII_PROBE(gii_img, x, 0), respv, gii_img->resp_size);
	}
	for (y=0; (y + width) < img->height; y++) {
		memcpy(&GII_PROBE(gii_img, 0, y), respv, gii_img->resp_size);
	}


	/*
	 * Compute the II for each of the pixels.  (We may
	 * want to do this at a lower resolution later).
	 */
	for (yoff = 0; (yoff + width) < img->height; yoff++) {
		for (xoff = 0; (xoff + width) < img->width; xoff++) {
			x = xoff + 1;
			y = yoff + 1;
			err = gargs->gobj->get_responses(img, xoff, yoff,
				gii_img->num_resp, respv, 1);

			/* get the II by do the adds/subtracts */
			gabor_response_add(gii_img->num_resp, respv, 
				&GII_PROBE(gii_img, x-1, y));
			gabor_response_add(gii_img->num_resp, respv, 
				&GII_PROBE(gii_img, x, y-1));
			gabor_response_subtract(gii_img->num_resp, respv, 
				&GII_PROBE(gii_img, x-1, y-1));

			/* store the new value */
			memcpy(&GII_PROBE(gii_img, x, y), respv, gii_img->resp_size);
		}
	}

	free(respv);
	return (0);
}
