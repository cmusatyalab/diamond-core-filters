/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

