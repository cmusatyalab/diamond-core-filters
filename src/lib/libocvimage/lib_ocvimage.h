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

#ifndef	_LIB_OCVIMAGE_H_
#define	_LIB_OCVIMAGE_H_	1

#include <stdint.h>
#include <errno.h>
#include <opencv/cv.h>
#include "rgb.h"

#ifdef __cplusplus
extern "C"
{
#endif


IplImage *create_gray_ipl_image(char *filename);
IplImage *create_rgb_ipl_image(char *filename);

IplImage *get_gray_ipl_image(RGBImage* rgb_img);
IplImage *get_rgb_ipl_image(RGBImage* rgb_img);


RGBImage * convert_ipl_to_rgb(IplImage * ipl);



#ifdef __cplusplus
}
#endif
#endif                          /* ! _LIB_OCVIMAGE_H_ */
