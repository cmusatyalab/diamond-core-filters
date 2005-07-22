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

#ifndef	_FIL_HISTO_H_
#define _FIL_HISTO_H_

#include "filter_api.h"
#include "common_consts.h"
#include "histo.h"
#include "queue.h"
#include "face.h"
#include "image_common.h"



#ifdef __cplusplus
extern "C"
{
#endif


int f_init_pnm2rgb(int numarg, char **args, int blob_len, void *blob,
	                   void **data);
int f_fini_pnm2rgb(void *data);
int f_eval_pnm2rgb(lf_obj_handle_t ihandle, void *user_data);



int f_init_histo_detect(int numarg, char **args, int blob_len, void *blob,
	                        void **data);
int f_fini_histo_detect(void *data);
int f_eval_histo_detect(lf_obj_handle_t ihandle, void *user_data);


int f_init_hpass(int numarg, char **args, int blob_len, void *blob,
	                 void **data);
int f_fini_hpass(void *data);
int f_eval_hpass(lf_obj_handle_t ihandle, void *user_data);


int f_init_hintegrate(int numarg, char **args, int blob_len, void *blob,
	                      void **data);
int f_fini_hintegrate(void *data);
int f_eval_hintegrate(lf_obj_handle_t ihandle, void *user_data);



#ifdef __cplusplus
}
#endif

#endif	/* !defined _FIL_HISTO_H_ */
