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

#ifndef	_FIL_TEXTURE_H_
#define	_FIL_TEXTURE_H_	1

#include <stdint.h>
#include "lib_filter.h"

typedef struct write_notify_context_t {
	lf_obj_handle_t ohandle;
} write_notify_context_t;



#ifdef __cplusplus
extern "C"
{
#endif
diamond_public
int f_init_texture_detect(int numarg, const char * const *args,
			  int blob_len, const void *blob,
			  const char *name, void **data);
diamond_public
int f_fini_texture_detect(void *data);
diamond_public
int f_eval_texture_detect(lf_obj_handle_t ihandle, void *user_data);

#ifdef __cplusplus
}
#endif

#endif	/* ! _FIL_TEXTURE_H_ */
