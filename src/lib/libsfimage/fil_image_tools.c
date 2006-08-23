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
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
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
	const u_char jpeg[4]		= { 0xff, 0xd8, 0xff, 0xe0 };
	const u_char jpeg_wexif[4]	= { 0xff, 0xd8, 0xff, 0xe1 };
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
	} else if (0 == memcmp(buf, jpeg, 4)) {
	  	type = IMAGE_JPEG;
	} else if (0 == memcmp(buf, jpeg_wexif, 4)) {
	  	type = IMAGE_JPEG;
	} else if (0 == memcmp(buf, png, 4)) {
	  	type = IMAGE_PNG;
        }

	return type;
}

int
pbm_read_data(off_t dlen, u_char *buf,  RGBImage * img)
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
pgm_read_data(off_t dlen, u_char *buf,  RGBImage * img)
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
ppm_read_data(off_t dlen, u_char *buf,  RGBImage * img)
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
get_rgb_from_pnm(u_char* buf, off_t size, image_type_t type)
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
				lf_log(LOGL_ERR, "get_rgb_from_pnm: invalid pbm image");
			}
			break;
		case IMAGE_PGM:
			err = pgm_read_data( (size - headerlen), &buf[headerlen], rgb);
			if (err) {
				lf_log(LOGL_ERR, "get_rgb_from_pnm: invalid pgm image");
			}
			break;
		case IMAGE_PPM:
			err = ppm_read_data( (size - headerlen), &buf[headerlen], rgb);
			if (err) {
				lf_log(LOGL_ERR, "get_rgb_from_pnm: invalid ppm image");
			}
			break;
		default:
			err = EINVAL;
			lf_log(LOGL_ERR, "get_rgb_from_pnm: uknown type");
			break;
	}
	if (err) {
		release_rgb_image(rgb);
		return(NULL);
	}

	return rgb;
}

RGBImage*
get_rgb_from_tiff(u_char* buf, off_t size)
{
	assert(buf);

	MyTIFF mytiff;
	mytiff.offset	= 0;
	mytiff.buf	= buf;
	mytiff.bytes	= size;

	return convertTIFFtoRGBImage(&mytiff);
}

RGBImage*
get_rgb_from_jpeg(u_char* buf, off_t size)
{
  	assert(buf);

	MyJPEG myjpeg;
	myjpeg.buf	= buf;
	myjpeg.bytes	= size;

	return convertJPEGtoRGBImage(&myjpeg);
}

RGBImage*
get_rgb_from_png(u_char* buf, off_t size)
{
  	// TODO
  return NULL;
}

RGBImage*
get_rgb_img(lf_obj_handle_t ohandle)
{
	int		err = 0;
	unsigned char *	obj_data;
	size_t		data_len;

	err = lf_next_block(ohandle, INT_MAX, &data_len, &obj_data);
	assert(!err);
	
	return read_rgb_image(obj_data, data_len);
}

RGBImage *
read_rgb_image(unsigned char *buf, size_t buflen)
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
	    lf_log(LOGL_ERR, "Unknown image format!!", magic);
	    break;
	}
	return img;
	
}

// Does an in-place linear normalization of the given image
// (each channel scaled independently)
// Returns a 0 if all is well.
//
int
rgb_normalize(RGBImage* img)
{
	assert(img);

	uint8_t rmin=255, gmin=255, bmin=255;
	uint8_t rmax=0,   gmax=0,   bmax=0;
	double	rscale,	  gscale,   bscale;
	int i, pixels;

	pixels = img->width * img->height;
	for (i=0; i < pixels; i++) {
		uint8_t r = img->data[i].r;
		uint8_t g = img->data[i].g;
		uint8_t b = img->data[i].b;
		if (r < rmin) { rmin = r; }
		if (r > rmax) { rmax = r; }
		if (g < gmin) { gmin = g; }
		if (g > gmax) { gmax = g; }
		if (b < bmin) { bmin = b; }
		if (b > bmax) { bmax = b; }
	}
	lf_log(LOGL_TRACE, "rgb_normalize: r(%d,%d), g(%d,%d), b(%d,%d)\n",
	    rmin,rmax, gmin,gmax, bmin,bmax);

	if (rmin < rmax) {	// only normalize if channels differ
	  rscale = (double)255.0/(rmax-rmin);
	  img->data[i].r = (uint8_t)(rmin + rscale * (img->data[i].r - rmin));
	}
	if (gmin < gmax) {	// only normalize if channels differ
	  gscale = (double)255.0/(gmax-gmin);
	  img->data[i].g = (uint8_t)(gmin + gscale * (img->data[i].g - gmin));
	}
	if (bmin < bmax) {	// only normalize if channels differ
	  bscale = (double)255.0/(bmax-bmin);
	  img->data[i].b = (uint8_t)(bmin + bscale * (img->data[i].b - bmin));
	}
	  
	return 0;
}
