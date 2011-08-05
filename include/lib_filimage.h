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

#ifndef	_LIB_FILIMAGE_H_
#define	_LIB_FILIMAGE_H_	1

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include "rgb.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct example_patch {
	RGBImage			*image;
	TAILQ_ENTRY(example_patch)	link;
} example_patch_t;

typedef TAILQ_HEAD(example_list_t, example_patch) example_list_t;

/*
 * some interfaces to read images from a file/buffer
 */

RGBImage	   *read_rgb_image(const void *buf, size_t buflen);

int pnm_parse_header(char *fdata, size_t nb,
		     int *width, int *height,
		     image_type_t *magic, int *headerlen);

                                                                                
                                                                                
RGBImage* image_gen_image_scale(RGBImage *, int scale);

void load_examples(const void *data, size_t len, example_list_t *examples);

void free_examples(example_list_t *examples);
                                                                                
#ifdef __cplusplus
}
#endif
#endif                          /* ! _LIB_FILIMAGE_H_ */
