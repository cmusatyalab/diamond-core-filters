/*
 * 	Diamond (Release 1.0)
 *      A system for interactive brute-force search
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
 * Synthetic filter to generate load.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "filter_api.h"
#include "fil_file.h"
#include "fil_image_tools.h"
#include "face.h"
#include "fil_data2ii.h"
#include "fil_tools.h"
#include "fil_assert.h"
#include "fil_synth.h"


void
compute_val(int data, int loop)
{
	int             i;
	double          foo = (double) data;

	for (i = 0; i < loop; i++) {
		foo = foo / i;
	}


}

int
f_synth(lf_obj_handle_t ohandle, int numout, lf_obj_handle_t * ohandles,
        int numarg, char **args, int blob_len, void *blob_data)
{
	int             err = 0;
	lf_fhandle_t    fhandle = 0;
	off_t           blen;
	double          thresh;
	;
	int             loop;
	int             limit;
	char           *data;
	int             ran;
	int             i;

	lf_log(fhandle, LOGL_TRACE, "\nf_synth: enter\n");

	if (numarg != 2) {
		exit(1);
	}
	thresh = atof(args[0]);
	loop = atoi(args[1]);


	err = lf_next_block(fhandle, ohandle, 1, &blen, &data);
	while (err == 0) {
		for (i = 0; i < blen; i++) {
			compute_val(data[i], loop);
		}
		err = lf_free_buffer(fhandle, data);
		assert(err == 0);

		err = lf_next_block(fhandle, ohandle, 1, &blen, &data);
	}


	limit = (int) ((double) RAND_MAX * thresh);
	ran = random();

	if (ran > limit) {
		return (0);
	} else {
		return (1);
	}

}
