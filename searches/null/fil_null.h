/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_FIL_NULL_H_
#define _FIL_NULL_H_

#include <stdint.h>
#include "lib_filter.h"
#include "snapfind_consts.h"

#ifdef __cplusplus
extern "C"
{
#endif

diamond_public
int f_init_null(int numarg, const char * const *args,
		int blob_len, const void *blob,
		const char *fname, void **data);
diamond_public
int f_fini_null(void *data);

diamond_public
int f_eval_null(lf_obj_handle_t ohandle, void *user_data);

#ifdef __cplusplus
}
#endif

#endif
