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


#ifndef	_FIL_OCV_FACE_H_
#define	_FIL_OCV_FACE_H_ 1


#ifdef __cplusplus
extern "C" {
#endif
int f_eval_opencv_fdetect(lf_obj_handle_t ohandle, void *fdata);

int f_fini_opencv_fdetect(void *fdata);

int f_init_opencv_fdetect(int numarg, char **args, int blob_len, 
	void *blob_data, void **fdatap);

#ifdef __cplusplus
}
#endif

#endif	/* !_FIL_OCV_FACE_H_ */
