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
 * rgb reader
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "lib_results.h"
#include "lib_filter.h"
#include "lib_filimage.h"
#include "rgb.h"



static int
f_init_img2rgb(int numarg, const char * const *args,
	       int blob_len, const void *blob,
	       const char *fname, void **data)
{

	assert(numarg == 0);
	*data = NULL;
	return (0);
}

/*
 * filter eval function to create an RGB_IMAGE attribute
 */

static int
f_eval_img2rgb(lf_obj_handle_t ohandle, void *user_data)
{
	RGBImage       *img;
	int             err = 0;
	const void    * obj_data;
	size_t          data_len;

	lf_log(LOGL_TRACE, "f_pnm2rgb: enter");


	err = lf_ref_attr(ohandle, "", &data_len, &obj_data);
	assert(!err);
	img = read_rgb_image(obj_data, data_len);
	if (img == NULL) {
		lf_log(LOGL_ERR, "f_pnm2rgb: failed to get rgb image");
		return(0);
	}

	/* save some attribs */
	lf_write_attr(ohandle, ROWS, sizeof(int), 
	    (unsigned char *) &img->height);
	lf_write_attr(ohandle, COLS, sizeof(int), 
	    (unsigned char *) &img->width);

	/* save img as an attribute */
	err = lf_write_attr(ohandle, RGB_IMAGE, img->nbytes,
	    (unsigned char *) img);
	assert(!err);

	/* but don't send it to the client */
	lf_omit_attr(ohandle, RGB_IMAGE);
	if (img)
		free(img);
	lf_log(LOGL_TRACE, "f_pnm2rgb: done");
	return 1;
}

LF_MAIN(f_init_img2rgb, f_eval_img2rgb)
