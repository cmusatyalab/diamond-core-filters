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



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "lib_filter.h"
#include "lib_log.h"	/* XXX fix this through lib_filter.h */
#include "lib_results.h"


int
write_param(lf_obj_handle_t ohandle, char *fmt, search_param_t * param, int i)
{
        off_t           bsize;
        char            buf[BUFSIZ];
        int             err;

        lf_log(LOGL_TRACE, "FOUND!!! ul=%ld,%ld; scale=%f\n",
               param->bbox.xmin, param->bbox.ymin, param->scale);

        sprintf(buf, fmt, i);
        bsize = sizeof(search_param_t);
        err = lf_write_attr(ohandle, buf, bsize, (char *) param);

        return err;
}



