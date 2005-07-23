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


#include <stdio.h>
#include <opencv/cv.h>
#include <limits.h>

#include "fil_image_tools.h"
#include "fil_assert.h"


int
ppm_read_data(off_t dlen, u_char *buf,  RGBImage * img)
{
	int             pixels;
	int		i;

	assert(sizeof(RGBPixel) >= 4);

	pixels =  img->width * img->height;

	/* verify we have enough data */
	assert((pixels * 3) <= dlen);

	for (i=0; i < pixels; i++) {
	    	img->data[i].r = *buf++;
	    	img->data[i].g = *buf++;
	    	img->data[i].b = *buf++;
	    	img->data[i].a = 255;
	}
	return (0);
}

int
pgm_read_data(off_t dlen, u_char *buf,  RGBImage * img)
{
	int             pixels;
	int		i;

	assert(sizeof(RGBPixel) >= 4);

	pixels =  img->width * img->height;

	/* verify we have enough data */
	assert((pixels) <= dlen);

	for (i=0; i < pixels; i++) {
	    	img->data[i].r = *buf;
	    	img->data[i].g = *buf;
	    	img->data[i].b = *buf++;
	    	img->data[i].a = 255;
	}
	return (0);
}



RGBImage       *
get_rgb_img(lf_obj_handle_t ohandle)
{
	RGBImage       *img = NULL;
	int             err = 0;
	int             width, height, headerlen;
	off_t           bytes;
	image_type_t    magic;
	char *		obj_data;
	off_t		data_len;

	/*
	 * read the header and figure out the dimensions 
	 */
	err = lf_next_block(ohandle, INT_MAX, &data_len, &obj_data);
	assert(!err);

	err = pnm_parse_header(obj_data, data_len, &width, &height, &magic, 
		&headerlen);
	if (err) {
		return(NULL);
	}
			
	/*
	 * create image to hold the data 
	 */
	bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);
	img = (RGBImage *)malloc(bytes);
	assert(img);
	img->nbytes = bytes;
	img->height = height;
	img->width = width;
	img->type = magic;

	/*
	 * read the data into img 
	 */
	/*
	 * this should be elsewhere... 
	 */
	switch (img->type) {
		case IMAGE_PPM:
			err = ppm_read_data((data_len - headerlen), 
				&obj_data[headerlen], img);
			assert(!err);
			break;
		case IMAGE_PGM:
			err = pgm_read_data((data_len - headerlen),
				&obj_data[headerlen], img);
			assert(!err);
			break;
		default:
			assert(0 && "unsupported image format");
			/*
			 * should close file as well XXX 
			 */
	}
	return (img);

}

RGBImage       *
get_attr_rgb_img(lf_obj_handle_t ohandle, char *attr_name)
{
	int             err = 0;
	char           *image_buf;
	off_t           bsize;
	IplImage       *srcimage;
	IplImage       *image;
	RGBImage       *rgb;

	/*
	 * assume this attr > 0 size
	 */

	bsize = 0;
	err = lf_read_attr(ohandle, attr_name, &bsize, (char *) NULL);
	if (err != ENOMEM) {
		return NULL;
	}

	image_buf = (char *)malloc(bsize);
	if (image_buf == NULL) {
		return NULL;
	}

	err = lf_read_attr( ohandle, attr_name, &bsize,
	                   (char *) image_buf);
	if (err) {
		return NULL;
	}


	srcimage = (IplImage *) image_buf;
	image = cvCreateImage(cvSize(srcimage->width, srcimage->height),
	                      srcimage->depth, srcimage->nChannels);

	memcpy(image->imageDataOrigin, image_buf + sizeof(IplImage),
	       image->imageSize);


	rgb = convert_ipl_to_rgb(image);
	cvReleaseImage(&image);
	free(image_buf);

	return (rgb);
}
