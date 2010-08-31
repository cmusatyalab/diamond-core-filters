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

#ifndef	_FIL_OCV_COMMON_H_
#define	_FIL_OCV_COMMON_H_ 1


#ifdef __cplusplus
extern "C" {
#endif
diamond_public
int f_eval_opencv_detect(lf_obj_handle_t ohandle, void *fdata);

diamond_public
int f_fini_opencv_detect(void *fdata);

diamond_public
int f_init_opencv_detect(int numarg, const char * const *args,
			 int blob_len, const void *blob_data,
			 const char *fname, void **fdatap);

#ifdef __cplusplus
}
#endif

#endif	/* !_FIL_OCV_COMMON_H_ */
