/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_LIB_SFIMAGE_H_
#define	_LIB_SFIMAGE_H_	1

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include "rgb.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * some interfaces to read images from a path/file/buffer
 */

RGBImage	   *read_rgb_image(unsigned char *buf, size_t buflen);

RGBImage 	*create_rgb_image(const char *filename);

int rgb_write_image(RGBImage *img, const char *filename, const char *path);
int rgb_write_image_file(RGBImage *img, FILE *fp);

/*
 * some interfaces to read images from a bytestream
 */
typedef struct pnm_state_t {
	RGBImage 	*img;
	size_t 	bytes_remaining;
	int 		parity;
	uint8_t 	*img_cur;	/* XXX assumes structure of RGBPixel */
} pnm_state_t;

pnm_state_t *pnm_state_new(RGBImage *img);
void pnm_state_delete(pnm_state_t *);
int pnm_parse_header(char *fdata, size_t nb,
		     int *width, int *height,
		     image_type_t *magic, int *headerlen);
int ppm_add_data(pnm_state_t *, char *fdata, size_t nb);

void img_constrain_bbox(bbox_t * bbox, RGBImage * img);

                                                                                
                                                                                
RGBImage* image_gen_image_scale(RGBImage *, int scale);


void image_draw_bbox_scale(RGBImage *, bbox_t *bbox, int scale,
    RGBPixel mask, RGBPixel color);

void image_draw_patch_scale(RGBImage *, patch_t *patch, int scale,
    RGBPixel mask, RGBPixel color);
                                                                                
void image_fill_bbox_scale(RGBImage *, bbox_t *bbox, int scale,
                           RGBPixel mask, RGBPixel color);
                                                                                

/* In place image normalization */
int rgb_normalize(RGBImage* img);

#ifdef __cplusplus
}
#endif
#endif                          /* ! _LIB_SFIMAGE_H_ */
