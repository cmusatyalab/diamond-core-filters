/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005 Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */


#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <opencv/cv.h>

#include "lib_results.h"
#include "rgb.h"
#include "lib_sfimage.h"
#include "snapfind_consts.h"



/*
 ********************************************************************** */
/*
 * create/read a Ipl Image (OpenCV) from the given file and convert to gray
 * scale.
 */

IplImage       *
create_gray_ipl_image(char *filename)
{
	IplImage       *rgba_img;
	IplImage       *gray_img;
	RGBImage       *img;
	int             err;
	FILE           *fp;
	char           *buf;
	int             buflen;
	int             width,
	height;
	image_type_t    magic;
	int             headerlen;
	int             bytes;
	const int       read_buffer_size = 128 << 10;

	fp = fopen(filename, "r");

	if (!fp) {
		perror(filename);
		return NULL;
	}

	buf = (char *) malloc(read_buffer_size);
	assert(buf);

	buflen = fread(buf, 1, read_buffer_size, fp);
	if (!buflen) {
		perror(filename);
		return NULL;
	}

	/*
	 * read the header and figure out the dimensions 
	 */
	err = pnm_parse_header(buf, buflen, &width, &height, &magic, &headerlen);
	if (err) {
		fprintf(stderr, "%s: parse error\n", filename);
		return NULL;
	}
	// fprintf(stderr, "create_rgb_image: width=%d, height=%d\n", width,
	// height); /* XXX */

	if (magic != IMAGE_PPM) {
		fprintf(stderr, "%s: only ppm format supported\n", filename);
		return NULL;
	}

	/*
	 * create image to hold the data 
	 */
	bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);
	img = (RGBImage *) malloc(bytes);
	if (!img) {
		fprintf(stderr, "out of memory!\n");
		return NULL;
	}
	img->nbytes = bytes;
	img->height = height;
	img->width = width;
	img->type = magic;

	/*
	 * read the data into img 
	 */
	{
		char *bufp;
		pnm_state_t    *state;
		state = pnm_state_new(img);
		bufp = &buf[headerlen];
		buflen -= headerlen;
		do
		{
			err = ppm_add_data(state, bufp, buflen);
			if (err) {
				free(img);
				img = NULL;
				fprintf(stderr, "error\n");
			}
			// fprintf(stderr, "added %d bytes\n", buflen);
			buflen = fread(bufp, 1, read_buffer_size, fp);
			// fprintf(stderr, "read %d bytes\n", buflen);
		} while (!err && buflen);
		// fprintf(stderr, "complete\n");
		pnm_state_delete(state);
	}

	free(buf);
	fclose(fp);

	/*
	 * create rgba ipl image 
	 */
	rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
	memcpy(rgba_img->imageData, img->data, rgba_img->imageSize);
	// rgba_img->imageData = (char*)&img->data;

	/*
	 * create grayscale image from rgb image 
	 */
	gray_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	cvCvtColor(rgba_img, gray_img, CV_RGBA2GRAY);

	cvReleaseImage(&rgba_img);
	release_rgb_image(img);

	return gray_img;
}


IplImage       *
create_rgb_ipl_image(char *filename)
{
	IplImage       *rgba_img;
	IplImage       *ipl_rgb_img;
	RGBImage       *img;
	// int err;
	// FILE *fp;
	// char *buf;
	// int buflen;
	int             width,
	height;
	// image_type_t magic;
	// int headerlen;
	// int bytes;
	// const int read_buffer_size = 128<<10;

	img = create_rgb_image(filename);

	height = img->height;
	width = img->width;

	/*
	 * create rgba ipl image 
	 */
	rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
	memcpy(rgba_img->imageData, img->data, rgba_img->imageSize);
	// rgba_img->imageData = (char*)&img->data;

	/*
	 * create grayscale image from rgb image (HUH?? -RW) 
	 */
	ipl_rgb_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	cvCvtColor(rgba_img, ipl_rgb_img, CV_RGBA2RGB);

	cvReleaseImage(&rgba_img);

	release_rgb_image(img);

	return ipl_rgb_img;
}


IplImage       *
get_gray_ipl_image(RGBImage * rgb_img)
{
	IplImage       *ipl_rgba_img;
	IplImage       *ipl_gray_img;
	int             width = rgb_img->width;
	int             height = rgb_img->height;

	ipl_rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
	ipl_gray_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

	/*
	 * read the data into img 
	 */

	memcpy(ipl_rgba_img->imageData, rgb_img->data, ipl_rgba_img->imageSize);

	/*
	 * create grayscale image from rgb image 
	 */
	cvCvtColor(ipl_rgba_img, ipl_gray_img, CV_RGBA2GRAY);
	cvReleaseImage(&ipl_rgba_img);
	return ipl_gray_img;
}

IplImage       *
get_rgb_ipl_image(RGBImage * rgb_img)
{
	IplImage       *ipl_rgba_img;
	IplImage       *ipl_rgb_img;
	int             width = rgb_img->width;
	int             height = rgb_img->height;

	ipl_rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
	ipl_rgb_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

	/*
	 * read the data into img 
	 */

	memcpy(ipl_rgba_img->imageData, rgb_img->data, ipl_rgba_img->imageSize);

	/*
	 * create grayscale image from rgb image 
	 */
	cvCvtColor(ipl_rgba_img, ipl_rgb_img, CV_RGBA2RGB);
	cvReleaseImage(&ipl_rgba_img);

	return ipl_rgb_img;
}

RGBImage *
convert_ipl_to_rgb(IplImage * ipl)
{
	IplImage       *ipl_rgba_img;
	int             width = ipl->width;
	int             height = ipl->height;
	RGBImage *		rgb;
	int				bytes;
	int				i;


	ipl_rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
	cvCvtColor(ipl, ipl_rgba_img, CV_RGB2RGBA);

	bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);
	rgb = (RGBImage *)malloc(bytes);

	rgb->nbytes = bytes;
	rgb->height = height;
	rgb->width = width;
	rgb->type = IMAGE_PPM;

	memcpy(rgb->data, ipl_rgba_img->imageData, ipl_rgba_img->imageSize);

	/*
	 * create grayscale image from rgb image 
	 */
	cvReleaseImage(&ipl_rgba_img);

	/* set all the alpha to 255 */
	for (i=0; i < (width * height); i++) {
		char 	tmp;
		tmp = rgb->data[i].r;
		rgb->data[i].r = rgb->data[i].b;
		rgb->data[i].b = tmp;
		rgb->data[i].a = 255;
	}

	return(rgb);
}


