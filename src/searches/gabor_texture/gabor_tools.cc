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
#include "face.h"
#include "filter_api.h"
#include "fil_file.h"
#include "image_tools.h"
#include "fil_tools.h"
#include "gabor_tools.h"
#include "texture_tools.h"
#include "fil_gab_texture.h"
#include "gabor.h"


static void
dump_gtexture_args(gtexture_args_t *gargs)
{
	int	i,j;
	float * farray;
	fprintf(stderr, "scale - %f\n", gargs->scale);
	fprintf(stderr, "step - %d\n", gargs->step);
	fprintf(stderr, "min_match - %d\n", gargs->min_matches);
	fprintf(stderr, "max_dist - %f\n", gargs->max_distance);
	fprintf(stderr, "num_angles - %d\n", gargs->num_angles);
	fprintf(stderr, "num_freq - %d\n", gargs->num_freq);
	fprintf(stderr, "radius - %d\n", gargs->radius);
	fprintf(stderr, "max_freq - %f\n", gargs->max_freq);
	fprintf(stderr, "min_freq - %f\n", gargs->min_freq);
	fprintf(stderr, "sigma - %f\n", gargs->sigma);
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

int
gabor_test_image(RGBImage * img, gtexture_args_t * targs, bbox_list_t * blist)
{
	int				num_resp;
	float          min_distance; 
	int             passed = 0;
	int             i, x, y;
	float *			respv;
	int		err;
	int		width;
	float		dist;
	bbox_t         *bbox;
	bbox_t          best_box;

	best_box.distance = 500000.0;

	dump_gtexture_args(targs);

	num_resp = targs->num_angles * targs->num_freq;
	respv = (float *)malloc(sizeof(float)*num_resp);
	assert(respv != NULL);

	/*
	 * test each subwindow 
	 */
	width = targs->radius *2 + 1;
	for (x = 0; (x + width) < img->width; x += targs->step) {
		for (y = 0; (y + width) < img->height; y += targs->step) {

			/* XXX scale ?? */

			err = targs->gobj->get_responses(img, x, y, num_resp, respv);
			assert(err == 0);

			min_distance = 2000.0;
			for (i=0; i < targs->num_samples; i++) {
				dist = gabor_comp_distance(num_resp, respv, targs->response_list[i]);
				if (dist < min_distance) {
					min_distance = dist;
				}
			}

			fprintf(stderr, "<%d %d > = %f \n", x, y, min_distance);
			if ((targs->min_matches == 1) &&
				(min_distance <= targs->max_distance) &&
				(min_distance < best_box.distance)) {
				best_box.min_x = x;
				best_box.min_y = y;
				best_box.max_x = x + width;    /* XXX scale */
				best_box.max_y = y + width;
				printf("update bbox: %d %d old %f new %f \n", 
					x,y, best_box.distance,
					min_distance);
				best_box.distance = min_distance;
			} else if ((targs->min_matches > 1) &&
					   (min_distance <= targs->max_distance)) {
				printf("%d %d -- %f \n", x, y, min_distance);
				passed++;
				bbox = (bbox_t *) malloc(sizeof(*bbox));
				assert(bbox != NULL);
				bbox->min_x = x;
				bbox->min_y = y;
				bbox->max_x = x + width;   /* XXX scale */
				bbox->max_y = y + width;
				bbox->distance = min_distance;
				printf("add bbox:  new %f \n", min_distance);
				TAILQ_INSERT_TAIL(blist, bbox, link);
																			
				if (passed >= targs->min_matches) {
					goto done;
				}
			}
		}
	}

    if ((targs->min_matches == 1)
        && (best_box.distance <= targs->max_distance)) {
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



