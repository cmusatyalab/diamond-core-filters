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
 * filter to make an integral image from raw data in pgm format
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <assert.h>

#include "lib_filter.h"
#include "lib_results.h"
#include "rgb.h"
#include "lib_log.h"
#include "fil_data2ii.h"
#include "facedet.h"

#define MAXCOLS 1281


#define FILTER_ASSERT(exp,msg) \
if(!(exp)) {\
  log_message(LOGT_FILT, LOGL_ERR, "Assertion %s failed at ", #exp);    \
  log_message(LOGT_FILT, LOGL_ERR, "%s, line %d.", __FILE__, __LINE__); \
  log_message(LOGT_FILT, LOGL_ERR, "(%s)", msg); \
  fprintf(stderr, "ERROR %s (%s, line %d)\n", (msg), __FILE__, __LINE__);\
  pass = 0;\
  goto done;    \
}


/* read: evaluate <error_exp> if(exp) is true. */

#define ASSERTX(error_exp,exp)                  \
if(!(exp)) {                                            \
  log_message(LOGT_FILT, LOGL_ERR, "Assertion %s failed at ", #exp); \
  log_message(LOGT_FILT, LOGL_ERR, "%s, line %d.", __FILE__, __LINE__);\
  fprintf(stderr, "ASSERT %s failed (%s, line %d)\n", #exp, __FILE__, __LINE__);\  
  (error_exp);  \
  goto done;    \
}



int
rgb_integrate(RGBImage * img,
              u_int32_t * sumarr,
              float *sumsqarr, size_t iiwidth, size_t iiheight)
{
	int             err = 0;
	int             count;
	int             i;
	size_t          nb;
	size_t          row,
	col;
	u_int32_t       colsum[MAXCOLS];    /* keep a column total */
	float          colsumsq[MAXCOLS];
	u_int32_t      *sumarr_end = sumarr + (iiwidth * iiheight);
	unsigned char  *fdata = NULL;

	ASSERTX(err = 1, iiwidth < MAXCOLS);

	for (col = 0; col < iiwidth; col++) {
		colsum[col] = 0;
		colsumsq[col] = 0;
	}

	for (col = 0; col < iiwidth; col++) {
		sumarr[col] = 0;
		sumsqarr[col] = 0;
	}
	for (row = 0; row < iiheight; row++) {
		sumarr[row * iiwidth] = 0;
		sumsqarr[row * iiwidth] = 0;
	}

	row = 1;                    /* excl */
	col = 1;                    /* excl */
	sumarr += 1 + iiwidth;      /* excl */
	sumsqarr += 1 + iiwidth;    /* excl */
	count = (iiwidth - 1) * (iiheight - 1); /* excl */

	fdata = (unsigned char *) img->data;    /* XXX */
	for (i = 0; i < count; i++) {
		u_int32_t       value;

		value = fdata[0];
		value += fdata[1];
		value += fdata[2];
		value /= 3;
		fdata += 4;             /* skip alpha */
		nb -= 4;

		colsum[col] += value;
		colsumsq[col] += value * value;
		*sumarr = colsum[col] + (col ? *(sumarr - 1) : 0);
		*sumsqarr = colsumsq[col] + (col ? *(sumsqarr - 1) : 0);
		// fprintf(stderr, " %f", *sumsqarr); /* XXX */

		sumarr++;
		ASSERTX(err = 1, sumarr <= sumarr_end);
		sumsqarr++;

		col++;
		ASSERTX(err = 1, col <= iiwidth);
		if (col == iiwidth) {
			sumarr++;           /* excl */
			sumsqarr++;         /* excl */
			col = 1;            /* excl */
			row++;
			ASSERTX(err = 1, row <= iiheight);
			// fprintf(stderr, "\n");//XXX
		}
	}
	ASSERTX(err = 1, sumarr == sumarr_end + 1);
done:
	return err;
}





#ifndef NDEBUG
void
print_ii(ii_image_t * ii)
{
	int             row,
	col;
	for (row = 0; row < ii->height; row++) {
		for (col = 0; col < ii->width; col++) {
			printf(" %03d", ii->data[row * ii->width + col]);
		}
		printf("\n");
	}
}
#endif


int
f_init_integrate(int numarg, char **args, int blob_len, void *blob_data,
                 const char *fname, void **fdatap)
{

	*fdatap = NULL;
	return (0);
}

int
f_fini_integrate(void *fdata)
{
	return (0);
}

int
f_eval_integrate(lf_obj_handle_t ohandle, void *fdata)
{
	ii_image_t     *img = NULL;
	ii2_image_t    *img2 = NULL;
	int             err = 0,
	                      pass = 1;
	off_t           bytes;
	int             width,
	height,
	headerlen;
	// image_type_t magic;

	lf_log(LOGL_TRACE, "\nf_integrate: enter\n");

	/*
	 * get image 
	 */
	RGBImage       *rgbimg =
	    (RGBImage *) ft_read_alloc_attr(ohandle, RGB_IMAGE);
	FILTER_ASSERT(rgbimg, "error getting RGB_IMAGE");
	ASSERTX(pass = 0, rgbimg);  /* XXX */
	width = rgbimg->width;
	height = rgbimg->height;

	/*
	 * read the header and figure out the dimensions 
	 */

	lf_log(LOGL_TRACE, "got image: width=%d, height=%d\n", width,
	       height);

	/*
	 * save some attribs 
	 */
	lf_write_attr(ohandle, IMG_HEADERLEN, sizeof(int),
	              (char *) &headerlen);
	lf_write_attr(ohandle, ROWS, sizeof(int), (char *) &height);
	lf_write_attr(ohandle, COLS, sizeof(int), (char *) &width);

	/*
	 * create image to hold the integral image 
	 */
	bytes = sizeof(ii_image_t) + sizeof(u_int32_t) * (width + 1) * (height + 1);
	img = (ii_image_t *)malloc(bytes);
	assert(img != NULL);
	img->nbytes = bytes;
	img->width = width + 1;
	img->height = height + 1;

	/*
	 * create image to hold the squared-integral image 
	 */
	bytes = sizeof(ii2_image_t) + sizeof(float) * (width + 1) * (height + 1);
	img2 = (ii2_image_t *)malloc(bytes);
	assert(img2 != NULL);
	img2->nbytes = bytes;
	img2->width = width + 1;
	img2->height = height + 1;

	/*
	 * make ii 
	 */
	rgb_integrate(rgbimg, img->data, img2->data, width + 1, height + 1);
	FILTER_ASSERT(!err, "read data");
	// ff_close(&file);

	/*
	 * save img as an attribute? 
	 */
	err = lf_write_attr(ohandle, II_DATA, img->nbytes, (char *) img);
	FILTER_ASSERT(!err, "write_attr");
	err = lf_write_attr(ohandle, II_SQ_DATA, img2->nbytes,
	                    (char *) img2);
	FILTER_ASSERT(!err, "write_attr");


done:
	if (rgbimg) {
		ft_free((char *) rgbimg);
	}
	if (img) {
		ft_free((char *) img);
	}
	if (img2) {
		ft_free((char *) img2);
	}
	lf_log(LOGL_TRACE, "f_integrate: done\n");
	return pass;
}
