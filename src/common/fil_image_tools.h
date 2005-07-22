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

#ifndef	_FIL_IMAGE_TOOLS_H_
#define	_FIL_IMAGE_TOOLS_H_	1

#include "image_tools.h"



#ifdef __cplusplus
extern  "C"
{
#endif

RGBImage       *get_attr_rgb_img(lf_obj_handle_t ohandle, char *name);


RGBImage       *get_rgb_img(lf_obj_handle_t ohandle);

#ifdef __cplusplus
}
#endif
#endif                          /* ! _FIL_IMAGE_TOOLS_H_ */
