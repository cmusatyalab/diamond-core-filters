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

#ifndef	_OPENCV_FACE_TOOLS_H_
#define	_OPENCV_FACE_TOOLS_H_ 1

#include "rgb.h"
#include "opencv_face.h"

#ifdef __cplusplus
extern "C"
{
#endif

int opencv_face_scan(RGBImage *img, bbox_list_t *blist,
		     opencv_fdetect_t *fconfig);

#ifdef __cplusplus
}
#endif
#endif	/* _OPENCV_FACE_TOOLS_H_ */

