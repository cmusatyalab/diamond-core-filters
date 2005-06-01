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

#ifndef	_FACE_TOOLS_H_
#define	_FACE_TOOLS_H_

#include "rgb.h"
#include "face.h"

#include "image_common.h"
#include "fil_tools.h"



/* ********************************************************************** */


#ifdef __cplusplus
extern "C"
{
#endif


	int face_scan_image(ii_image_t *ii, ii2_image_t * ii2,
	                    fconfig_fdetect_t *fconfig, bbox_list_t *blist, int height, int width);

	int opencv_face_scan(RGBImage *img, bbox_list_t *blist,
	                     opencv_fdetect_t *fconfig);

#ifdef __cplusplus
}
#endif
#endif	/* _FACE_TOOLS_H_ */

