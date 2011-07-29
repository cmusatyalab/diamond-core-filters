/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <stdio.h>
#include <limits.h>
#include <string.h>		// for memcmp

#include "rgb.h"		// for image_type_h
#include "lib_results.h"
#include "lib_sfimage.h"
#include "readjpeg.h"
#include "readtiff.h"
#include "readpng.h"
#include "assert.h"

/*
 * Given a ffile_t, examines the first 8 bytes to try to guess
 * whether it is a TIFF, PNM, etc.  Doesn't "read" the file per se.
 */
image_type_t
determine_image_type(const u_char* buf)
{
	const u_char pbm_ascii[2]	= "P1";
	const u_char pbm_raw[2]		= "P4";
	const u_char pgm_ascii[2]	= "P2";
	const u_char pgm_raw[2] 	= "P5";
	const u_char ppm_ascii[2]	= "P3";
	const u_char ppm_raw[2]		= "P6";
	const u_char tiff_big_endian[4] = { 0x4d, 0x4d, 0x00, 0x2a };
	const u_char tiff_lit_endian[4] = { 0x49, 0x49, 0x2a, 0x00 };
	const u_char jpeg[2]		= { 0xff, 0xd8 };
	const u_char png[4]		= { 0x89, 0x50, 0x4e, 0x47 };

	image_type_t type = IMAGE_UNKNOWN;
	if	  (0 == memcmp(buf, pbm_ascii, 2))	 {
		type = IMAGE_PBM;
	} else if (0 == memcmp(buf, pbm_raw, 2))	 {
		type = IMAGE_PBM;
	} else if (0 == memcmp(buf, pgm_ascii, 2))	 {
		type = IMAGE_PGM;
	} else if (0 == memcmp(buf, pgm_raw, 2))	 {
		type = IMAGE_PGM;
	} else if (0 == memcmp(buf, ppm_ascii, 2))	 {
		type = IMAGE_PPM;
	} else if (0 == memcmp(buf, ppm_raw, 2))	 {
		type = IMAGE_PPM;
	} else if (0 == memcmp(buf, tiff_big_endian, 4)) {
		type = IMAGE_TIFF;
	} else if (0 == memcmp(buf, tiff_lit_endian, 4)) {
		type = IMAGE_TIFF;
	} else if (0 == memcmp(buf, jpeg, 2)) {
	  	type = IMAGE_JPEG;
	} else if (0 == memcmp(buf, png, 4)) {
	  	type = IMAGE_PNG;
        }

	return type;
}

int
pbm_read_data(off_t dlen, const u_char *buf,  RGBImage * img)
{
	int             pixels;
	int		i;

	assert(sizeof(RGBPixel) >= 4);

	pixels =  img->width * img->height;

	/* verify we have enough data */
	assert((pixels / 8) <= dlen);

	for (i=0; i < pixels; i+=8) {
		int j;
		for (j=0; j<8; j++) {
			int bit = 255 * ((*buf >> (7-j)) & 0x1);
			img->data[i+j].r = bit;
			img->data[i+j].g = bit;
			img->data[i+j].b = bit;
			img->data[i+j].a = 255;
		}
		buf++;
	}
	return (0);
}

int
pgm_read_data(off_t dlen, const u_char *buf,  RGBImage * img)
{
	int 	pixels;
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

int
ppm_read_data(off_t dlen, const u_char *buf,  RGBImage * img)
{
	int     pixels;
	int		i;

	assert(sizeof(RGBPixel) >= 4);

	pixels =  img->width * img->height;

	if (dlen < (pixels * 3)) {
		return (EINVAL);
	}

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

RGBImage*
get_rgb_from_pnm(const u_char* buf, off_t size, image_type_t type)
{
	assert(buf);
	assert((type==IMAGE_PBM) || (type==IMAGE_PGM) || (type==IMAGE_PPM));

	int err, width, height, headerlen;
	image_type_t magic;
	err = pnm_parse_header((char *)buf, size, &width, &height, 
	    &magic, &headerlen);
	if (err) {
		return NULL;
	}
	assert(type == magic);	// paranoia :-)

	RGBImage* rgb = rgbimg_blank_image(width, height);
	rgb->type = magic;
	switch (rgb->type) {
		case IMAGE_PBM:
			err = pbm_read_data( (size - headerlen), &buf[headerlen], rgb);
			if (err) {
				fprintf(stderr, "get_rgb_from_pnm: invalid pbm image\n");
			}
			break;
		case IMAGE_PGM:
			err = pgm_read_data( (size - headerlen), &buf[headerlen], rgb);
			if (err) {
				fprintf(stderr, "get_rgb_from_pnm: invalid pgm image\n");
			}
			break;
		case IMAGE_PPM:
			err = ppm_read_data( (size - headerlen), &buf[headerlen], rgb);
			if (err) {
				fprintf(stderr, "get_rgb_from_pnm: invalid ppm image\n");
			}
			break;
		default:
			err = EINVAL;
			fprintf(stderr, "get_rgb_from_pnm: uknown type\n");
			break;
	}
	if (err) {
		release_rgb_image(rgb);
		return(NULL);
	}

	return rgb;
}

RGBImage*
get_rgb_from_tiff(const u_char* buf, off_t size)
{
	assert(buf);

	MyTIFF mytiff;
	mytiff.offset	= 0;
	mytiff.buf	= buf;
	mytiff.bytes	= size;

	return convertTIFFtoRGBImage(&mytiff);
}

RGBImage*
get_rgb_from_jpeg(const u_char* buf, off_t size)
{
  	assert(buf);

	MyJPEG myjpeg;
	myjpeg.buf	= buf;
	myjpeg.bytes	= size;

	return convertJPEGtoRGBImage(&myjpeg);
}

RGBImage*
get_rgb_from_png(const u_char* buf, off_t size)
{
	assert(buf);
	return convertPNGtoRGBImage(buf, size);
}

RGBImage *
read_rgb_image(const void *buf, size_t buflen)
{
	RGBImage*	img = NULL;
		
	image_type_t magic = determine_image_type(buf);
	switch(magic) {
	  case IMAGE_PBM:
	  case IMAGE_PGM:
	  case IMAGE_PPM:
	    img = get_rgb_from_pnm(buf, buflen, magic);
	    break;
	  case IMAGE_TIFF:
	    img = get_rgb_from_tiff(buf, buflen);
	    break;
	  case IMAGE_JPEG:
	    img = get_rgb_from_jpeg(buf, buflen);
	    break;
	  case IMAGE_PNG:
	    img = get_rgb_from_png(buf, buflen);
	    break;
	  default:
	    fprintf(stderr, "Unknown image format!!\n");
	    break;
	}
	return img;
	
}
