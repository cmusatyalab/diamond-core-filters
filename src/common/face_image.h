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

#ifndef	_FACE_IMAGE_H_
#define	_FACE_IMAGE_H_


#include "face_tools.h"
#include "rgb.h"

#ifdef __cplusplus
extern "C"
{
#endif



	RGBImage* image_gen_image_scale(RGBImage *, int scale);
	void image_draw_bbox_scale(RGBImage *, bbox_t *bbox, int scale,
	                           RGBPixel mask, RGBPixel color);

	void image_fill_bbox_scale(RGBImage *, bbox_t *bbox, int scale,
	                           RGBPixel mask, RGBPixel color);


#ifdef __cplusplus
}
#endif

#endif	/* _FACE_IMAGE_H_ */

