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


#include "filter_api.h"


typedef struct
{
	int			num_attrs;
	int			num_regexs;
	char **		attr_names;
	char **		regex_names;
	regex_t	*	regs;

}
fdata_regex_t;

#ifdef __cplusplus
extern "C"
{
#endif

int f_init_regex(int numarg, char **args, int blob_len, void *blob,
	                 void **fdatap);
int f_fini_regex(void *fdata);
int f_eval_regex(lf_obj_handle_t ohandle, void *fdata);

#ifdef __cplusplus
}
#endif
