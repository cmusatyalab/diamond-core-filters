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

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>
#include "queue.h"
#include "common_consts.h"
#include "rgb.h"
#include "histo.h"
#include "image_tools.h"


/* call to read the cycle counter */
#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))


static unsigned long long
read_cycle()
{
	unsigned long long      foo;
	rdtscll(foo);
	return(foo);
}


void
do_test(RGBImage *img)
{
	histo_config_t  hconfig;
	patch_t         *       hpatch;
	int                             i;
	int                             pass;
	unsigned long long		start, stop;
	HistoII *		ii;
	bbox_list_t            blist;
	TAILQ_INIT(&blist);

	hconfig.name = strdup("foo");
	hconfig.req_matches = 1;        /* XXX */
	hconfig.scale = 1.4;
	hconfig.xsize = 32;
	hconfig.ysize = 32;
	hconfig.stride = 4;
	hconfig.bins = 3;   /* XXX */
	hconfig.simularity = 0.80;
	hconfig.distance_type = 0;
	hconfig.type = HISTO_INTERPOLATED;

	TAILQ_INIT(&hconfig.patchlist);
	hconfig.num_patches = 1;

	hpatch = (patch_t *) malloc(sizeof(*hpatch));
	histo_clear(&hpatch->histo);

	hpatch->histo.data[0] = 1.0;
	normalize_histo(&hpatch->histo);
	hpatch->histo.weight = 1.0;

	TAILQ_INSERT_TAIL(&hconfig.patchlist, hpatch, link);

	start = read_cycle();
	ii = histo_get_ii(&hconfig, img);
	stop = read_cycle();
	printf("ii time %lld \n", (stop - start));

	start = read_cycle();
	pass =  histo_scan_image(hconfig.name, img, ii, &hconfig,
	                         INT_MAX /* XXX */, &blist);
	stop = read_cycle();
	printf("scan time %lld \n", (stop - start));
}


usage()
{
	fprintf(stderr, "histo_bench <ppm image> ... \n");
}
int
main(int argc, char **argv)
{
	RGBImage *	img;
	int		i;

	if (argc < 2) {
		usage();
		exit(1);
	}

	for (i=1; i < argc; i++) {
		img = create_rgb_image(argv[i]);
		assert(img);

		do_test(img);
		release_rgb_image(img);
	}

}
