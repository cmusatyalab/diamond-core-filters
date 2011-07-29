/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2011 Carnegie Mellon University
 *  All rights reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <stdio.h>
#include <stdint.h>
#include <png.h>

#include "rgb.h"
#include "readpng.h"

RGBImage* convertPNGtoRGBImage(const void *buf, size_t size)
{
	FILE *fp;
	png_structp png = NULL;
	png_infop info = NULL;
	RGBImage *img = NULL;
	png_bytepp rows = NULL;
	uint32_t nbytes, width, height, y;

	/* Allocate structures */
	fp = fmemopen((void *) buf, size, "rb");
	if (fp == NULL)
		goto bad;
	/* Use default error handling */
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png == NULL)
		goto bad;
	info = png_create_info_struct(png);
	if (info == NULL)
		goto bad;

	/* Handle subsequent fatal errors */
	if (setjmp(png_jmpbuf(png)))
		goto bad;

	/* Configure libpng */
	png_init_io(png, fp);
	png_read_info(png, info);
	/* Expand paletted to RGB, low-bit-depth gray to 8-bit, and
	   transparency to alpha */
	png_set_expand(png);
	/* Reduce 16-bit samples to 8-bit */
	png_set_strip_16(png);
	/* Add alpha channel if missing */
	png_set_filler(png, 0xff, PNG_FILLER_AFTER);
	/* Convert gray to RGB */
	png_set_gray_to_rgb(png);

	/* Allocate image */
	width = png_get_image_width(png, info);
	height = png_get_image_height(png, info);
	nbytes = sizeof(*img) + width * height * sizeof(RGBPixel);
	img = malloc(nbytes);
	img->type = IMAGE_PNG;
	img->nbytes = nbytes;
	img->width = width;
	img->height = height;
	rows = malloc(sizeof(*rows) * height);
	for (y = 0; y < height; y++)
		rows[y] = (png_bytep) &img->data[y * width];

	/* Commit image allocations for error handling */
	if (setjmp(png_jmpbuf(png)))
		goto bad;

	/* Read image */
	png_read_image(png, rows);

	/* Release resources */
	png_read_end(png, NULL);
	png_destroy_read_struct(&png, &info, NULL);
	free(rows);
	fclose(fp);

	return img;
bad:
	free(rows);
	free(img);
	if (png != NULL || info != NULL)
		png_destroy_read_struct(&png, &info, NULL);
	if (fp != NULL)
		fclose(fp);
	return NULL;
}
