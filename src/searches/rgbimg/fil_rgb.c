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
 * color histogram filter * rgb reader
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "snapfind_consts.h"
#include "lib_results.h"
#include "lib_filter.h"
#include "lib_sfimage.h"
#include "rgb.h"



#define ASSERT(exp)							\
if(!(exp)) {								\
  lf_log(LOGL_ERR, "Assertion %s failed at ", #exp);		\
  lf_log(LOGL_ERR, "%s, line %d.", __FILE__, __LINE__);	\
  pass = -1;								\
  goto done;								\
}


int
f_init_img2rgb(int numarg, char **args, int blob_len, void *blob, 
		const char *fname, void **data)
{

	assert(numarg == 0);
	*data = NULL;
	return (0);
}

int
f_fini_img2rgb(void *data)
{
	return (0);
}

/*
 * filter eval function to create an RGB_IMAGE attribute
 */

int
f_eval_img2rgb(lf_obj_handle_t ohandle, void *user_data)
{
	RGBImage       *img;
	int             err = 0, pass = 1;

	lf_log(LOGL_TRACE, "f_pnm2rgb: enter");


	img = get_rgb_img(ohandle);
	if (img == NULL) {
		lf_log(LOGL_ERR, "f_pnm2rgb: failed to get rgb image");
		return(0);
	}

	// Rahul TODO
	// We can normalize the image here, but we should not do
	// so automatically -- should only do it if needed
	/* rgb_normalize(img); */

	/*
	 * save some attribs 
	 */
	lf_write_attr(ohandle, ROWS, sizeof(int), 
	    (unsigned char *) &img->height);
	lf_write_attr(ohandle, COLS, sizeof(int), 
	    (unsigned char *) &img->width);

	/*
	 * save img as an attribute 
	 */
	err = lf_write_attr(ohandle, RGB_IMAGE, img->nbytes,
	    (unsigned char *) img);
	ASSERT(!err);
done:
	if (img)
		free(img);
	lf_log(LOGL_TRACE, "f_pnm2rgb: done");
	return pass;
}

