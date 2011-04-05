/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <stdlib.h>
#include <string.h>
#include "lib_results.h"
#include "lib_sfimage.h"
#include "rgb.h"

#include "image.c"

static int
f_init_null(int numarg, const char * const *args,
	    int blob_len, const void *blob,
	    const char *fname, void **data)
{
    RGBImage *img;
    if (gimp_image.bytes_per_pixel != 4) {
	lf_log(LOGL_TRACE, "f_init_null: source image missing alpha channel");
	return 0;
    }

    img = rgbimg_blank_image(gimp_image.width, gimp_image.height);
    if (!img) {
	lf_log(LOGL_TRACE, "f_init_null: image allocation failed");
	return 0;
    }

    GIMP_IMAGE_RUN_LENGTH_DECODE((unsigned char *)img->data,
				 gimp_image.rle_pixel_data,
				 gimp_image.width * gimp_image.height,
				 gimp_image.bytes_per_pixel);
    *data = img;
    return 0;
}

static int
f_eval_null(lf_obj_handle_t ohandle, void *data)
{
    RGBImage *img = (RGBImage *)data;
    //char *display = "icon";
    char *display = "label";
    //char *display = "icon-and-label";

    if (!img) {
	lf_log(LOGL_TRACE, "f_eval_null: no rgb image data");
	return 0;
    }

    lf_write_attr(ohandle, "hyperfind.thumbnail-display",
		  strlen(display)+1, (unsigned char *)display);

    lf_write_attr(ohandle, ROWS, sizeof(int), 
		  (unsigned char *) &img->height);
    lf_write_attr(ohandle, COLS, sizeof(int), 
		  (unsigned char *) &img->width);
    lf_write_attr(ohandle, RGB_IMAGE, img->nbytes,
		  (unsigned char *) img);

    /* don't send it to the client */
    lf_omit_attr(ohandle, RGB_IMAGE);

    lf_log(LOGL_TRACE, "f_null: done");
    return 1;
}

LF_MAIN(f_init_null, f_eval_null)
