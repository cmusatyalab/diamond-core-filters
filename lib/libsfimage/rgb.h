/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _RGB_H_
#define _RGB_H_ 	1

#include <stdint.h>
#include <sys/types.h>
#include "lib_filter.h"


typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;	// Alpha value -- mainly just padding
} RGBPixel;



typedef enum image_type_t {
    IMAGE_UNKNOWN = 0,
    IMAGE_PBM,
    IMAGE_PGM,
    IMAGE_PPM,
    IMAGE_TIFF,
    IMAGE_JPEG,
    IMAGE_PNG
} image_type_t;


typedef struct {
	uint32_t type;			/* image_type_t */
	uint32_t nbytes;		/* size of this var size struct */
	union {
		int32_t height;
		int32_t rows;
	};
	union {
		int32_t width;
		int32_t columns;
	};
	RGBPixel data[0];		/* var size struct */
} RGBImage;


typedef struct FGImage {
	image_type_t type;
	size_t nelements;		/* size of this var size struct */
	int width;
	int height;
	float 	data[0];		/* var size struct */
} FGImage_t;

#define	PIXEL_OFFSET(img, x, y)	(((y) * ((img)->width)) + (x))

/* some colors/masks */

static const RGBPixel red = { 255, 0, 0, 255 };
static const RGBPixel green = { 32, 255, 32, 255 };
static const RGBPixel blue = { 32, 32, 255, 255 };
static const RGBPixel clearColor = { 0, 0, 0, 0 };
static const RGBPixel colorMask = { 1, 1, 1, 1 };
static const RGBPixel clearMask = { 0, 0, 0, 0 };

static const RGBPixel hilit = { 255, 255, 255, 128 };
static const RGBPixel hilitRed = { 255, 0, 0, 32 };
static const RGBPixel hilitMask = { 1, 1, 1, 1 };


#ifdef __cplusplus
extern "C"
{
#endif



/* make a new image the same size as src */
RGBImage *rgbimg_new(RGBImage *srcimg);

/* make a new image duplicating src */
RGBImage *rgbimg_dup(RGBImage *srcimg);

/* wipe image clean */
void rgbimg_clear(RGBImage *img);


RGBImage *create_rgb_subimage(RGBImage *old, int xoff, int yoff, int xsize,
			      int ysize);
void release_rgb_image(RGBImage *);

FGImage_t * rgb_to_fgimage(RGBImage *);

RGBImage * rgbimg_blank_image(int width, int height);


#ifdef __cplusplus
}
#endif

#endif /* !_RGB_H_ */
