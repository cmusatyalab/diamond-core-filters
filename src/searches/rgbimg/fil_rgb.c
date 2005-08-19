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
#include "fil_image_tools.h"
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


	/* XXX rahul   put some decoder here to figure out the file type */

	img = get_rgb_img(ohandle);
	if (img == NULL) {
		return(0);
	}

	/*
	 * save some attribs 
	 */
#ifdef	XXX
	lf_write_attr(ohandle, IMG_HEADERLEN, sizeof(int),
	              (char *) &headerlen);
#endif

	lf_write_attr(ohandle, ROWS, sizeof(int), (char *) &img->height);
	lf_write_attr(ohandle, COLS, sizeof(int), (char *) &img->width);

	/*
	 * save img as an attribute 
	 */
	err = lf_write_attr(ohandle, RGB_IMAGE, img->nbytes, (char *) img);
	ASSERT(!err);
done:
	if (img)
		free(img);
	lf_log(LOGL_TRACE, "f_pnm2rgb: done");
	return pass;
}

