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
        size_t           bsize;
        char            buf[BUFSIZ];
        int             err;

        lf_log(LOGL_TRACE, "Found: ul=%ld,%ld ",
               param->bbox.xmin, param->bbox.ymin, param->scale);

        sprintf(buf, fmt, i);
        bsize = sizeof(search_param_t);
        err = lf_write_attr(ohandle, buf, bsize, (char *) param);

        return err;
}


int
read_param(lf_obj_handle_t ohandle, char *fmt,
           search_param_t * param, int i)
{
        size_t           bsize;
        char            buf[BUFSIZ];
        int             err;

        sprintf(buf, fmt, i);
        bsize = sizeof(search_param_t);
        err = lf_read_attr(ohandle, buf, &bsize, (char *) param);

        return err;
}


char           *
ft_read_alloc_attr(lf_obj_handle_t ohandle, const char *name)
{
        int             err;
        char           *ptr;
        size_t           bsize;

        /*
         * assume this attr > 0 size
         */

        bsize = 0;
        err = lf_read_attr(ohandle, name, &bsize, (char *) NULL);
        if (err != ENOMEM) {
                // fprintf(stderr, "attribute lookup error: %s\n", name);
                return NULL;
        }

        ptr = (char *)malloc(bsize);
        if (ptr == NULL ) {
                fprintf(stderr, "alloc error\n");
                return (NULL);
        }

        err = lf_read_attr(ohandle, name, &bsize, (char *) ptr);
        if (err) {
                fprintf(stderr, "attribute read error: %s\n", name);
                return NULL;
        }

        return ptr;
}

/* XXX remove ??? */
void
ft_free(char *ptr)
{
        free(ptr);
        // assert(err == 0);
}


