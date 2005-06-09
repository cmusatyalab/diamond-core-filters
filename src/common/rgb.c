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


#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "rgb.h"

RGBImage       *
rgbimg_new(RGBImage * imgsrc)
{
	RGBImage       *img;

	img = (RGBImage *) calloc(imgsrc->nbytes, 1);
	assert(img);
	memcpy(img, imgsrc, sizeof(RGBImage));  /* only copy header */

	return img;
}

RGBImage       *
rgbimg_dup(RGBImage * imgsrc)
{
	RGBImage       *img;

	img = (RGBImage *) malloc(imgsrc->nbytes);
	assert(img);
	memcpy(img, imgsrc, imgsrc->nbytes);

	return img;
}

void
rgbimg_clear(RGBImage * img)
{
	size_t          nb;

	nb = img->width * img->height * sizeof(RGBPixel);
	memset(img->data, 0, nb);
}


RGBImage       *
create_rgb_subimage(RGBImage * old_img, int xoff, int yoff, int xsize,
                    int ysize)
{
	RGBImage       *new_img;
	int             bytes;
	const RGBPixel *oldp;
	RGBPixel       *newp;
	int             i,
	j;


	assert(old_img->width >= (xoff + xsize));
	assert(old_img->height >= (yoff + ysize));

	bytes = sizeof(RGBImage) + xsize * ysize * sizeof(RGBPixel);
	new_img = (RGBImage *) malloc(bytes);

	assert(new_img != NULL);

	new_img->nbytes = bytes;
	new_img->height = ysize;
	new_img->width = xsize;
	new_img->type = old_img->type;


	for (j = 0; j < ysize; j++) {
		oldp = old_img->data + ((yoff + j) * old_img->width) + xoff;
		newp = new_img->data + (j * new_img->width);

		for (i = 0; i < xsize; i++) {
			*newp++ = *oldp++;
		}
	}

	return new_img;
}

RGBImage       *
rgbimg_blank_image(int width, int height)
{
	RGBImage       *img;
	size_t	  	bytes;

	bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);

	img = (RGBImage *) calloc(bytes, 1);
	assert(img);

	img->nbytes = bytes;
	img->height = height;
	img->width = width;
	img->type = IMAGE_PPM;

	return img;
}


void
release_rgb_image(RGBImage * img)
{
	free(img);
}


void
release_fgimage(FGImage_t * img)
{
	free(img);
}

FGImage_t *
create_fgimage(int width, int height)
{
	FGImage_t * 	img;
	size_t		bytes;

	bytes = sizeof(FGImage_t) + (width * height * sizeof(float));

	img = (FGImage_t *) calloc(bytes, 1);

	img->nelements = width * height;
	img->height = height;
	img->width = width;
	return(img);
}

FGImage_t *
rgb_to_fgimage(RGBImage *orig)
{
	FGImage_t *	img;
	int		i;
	

	img = create_fgimage(orig->width, orig->height);
	

	for (i=0; i < (orig->width * orig->height); i++) {
		img->data[i] = ((float)(orig->data[i].r + orig->data[i].b + 
			orig->data[i].g))/3.0;
	}
	return(img);
}
