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

#ifndef	_LIB_OCVIMAGE_H_
#define	_LIB_OCVIMAGE_H_	1

/*****************/
/* Work around compiler warnings in the OpenCV headers.  Sources should
   include this file rather than <opencv/cv.h>. */

/* In C code, CV_INLINE functions are marked static, which causes a warning
   when they're not used. */
#ifndef __cplusplus
#define CV_INLINE static inline
#endif

/* cvcompat.h has inline functions with unused variables.  Luckily, we can
   avoid including it. */
#define CV_NO_BACKWARD_COMPATIBILITY

/* ...except that cvaux.h uses some deprecated names.  Copy the relevant
   redefinitions from cvcompat.h. */
#define CvPoint3D64d CvPoint3D64f
#define CvPoint2D64d CvPoint2D64f
/*****************/


#include <stdint.h>
#include <errno.h>
#include <opencv/cv.h>
#include "rgb.h"

#ifdef __cplusplus
extern "C"
{
#endif


IplImage *get_gray_ipl_image(RGBImage* rgb_img);
IplImage *get_rgb_ipl_image(RGBImage* rgb_img);


#ifdef __cplusplus
}
#endif
#endif                          /* ! _LIB_OCVIMAGE_H_ */
