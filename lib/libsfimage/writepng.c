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
#include "writepng.h"

void *convertRGBImagetoPNG(const RGBImage *img, size_t *len)
{
	char *data = NULL;
	FILE *fp;
	png_structp png = NULL;
	png_infop info = NULL;
	png_bytepp rows = NULL;
	uint32_t y;

	/* Allocate structures */
	fp = open_memstream(&data, len);
	if (fp == NULL)
		goto bad;
	/* Use default error handling */
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png == NULL)
		goto bad;
	info = png_create_info_struct(png);
	if (info == NULL)
		goto bad;
	rows = malloc(sizeof(*rows) * img->height);
	for (y = 0; y < img->height; y++)
		rows[y] = (png_bytep) &img->data[y * img->width];

	/* Handle subsequent fatal errors */
	if (setjmp(png_jmpbuf(png)))
		goto bad;

	/* Configure libpng */
	png_init_io(png, fp);
	png_set_IHDR(png, info, img->width, img->height, 8,
				PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
	png_set_rows(png, info, rows);

	/* Write image */
	png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);

	/* Release resources */
	png_destroy_write_struct(&png, &info);
	free(rows);
	fclose(fp);

	return data;
bad:
	free(rows);
	if (png != NULL || info != NULL)
		png_destroy_write_struct(&png, &info);
	if (fp != NULL)
		fclose(fp);
	free(data);
	return NULL;
}
