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

#ifndef	_FIL_IMG_DIFF_H_
#define _FIL_IMG_DIFF_H_

#include <stdint.h>
#include "lib_filter.h"
#include "snapfind_consts.h"
#include <sys/queue.h>

typedef struct img_diff_config
{
  float   distance;   /* threshold on quality of match */
  RGBImage *img;      /* image to compare against */

} img_diff_config_t;

#ifdef __cplusplus
extern "C"
{
#endif


diamond_public
int f_init_img_diff(int numarg, const char * const *args,
		    int blob_len, const void *blob,
		    const char *fname, void **data);
diamond_public
int f_fini_img_diff(void *data);
diamond_public
int f_eval_img_diff(lf_obj_handle_t ihandle, void *user_data);


#ifdef __cplusplus
}
#endif

#endif	/* !defined _FIL_IMG_DIFF_H_ */
