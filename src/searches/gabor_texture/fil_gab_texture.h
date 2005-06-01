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

#ifndef	_FIL_GABOR_TEXTURE_H_
#define	_FIL_GABOR_TEXTURE_H_	1

#include "filter_api.h"
#include "gabor.h"

typedef struct write_notify_context_t
{
	lf_fhandle_t 	fhandle;
	lf_obj_handle_t ohandle;
}
write_notify_context_t;



#ifdef __cplusplus
extern "C"
{
#endif
	int f_init_gab_texture(int numarg, char **args, int blob_len, void *blob,
	                       void **data);
	int f_fini_gab_texture(void *data);
	int f_eval_gab_texture(lf_obj_handle_t ihandle, int numout,
	                       lf_obj_handle_t *ohandles, void *user_data);



#ifdef __cplusplus
}
#endif

#endif	/* ! _FIL_GABOR_TEXTURE_H_ */
