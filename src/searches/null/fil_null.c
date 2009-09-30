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

#include "lib_results.h"
#include "fil_null.h"

int
f_init_null(int numarg, char **args, int blob_len, void *blob,
	    const char *fname, void **data)
{
    *data = NULL;
    return (0);
}

int
f_fini_null(void *data)
{
    return (0);
}

int
f_eval_null(lf_obj_handle_t ohandle, void *user_data)
{
    lf_log(LOGL_TRACE, "f_null: done");
    return 1;
}

