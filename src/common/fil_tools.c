/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <stdint.h>

#include "fil_assert.h"
#include "filter_api.h"
#include "fil_tools.h"

char           *
ft_read_alloc_attr(lf_fhandle_t fhandle, lf_obj_handle_t ohandle,
                   const char *name)
{
    int             err;
    char           *ptr;
    off_t           bsize;

    /*
     * assume this attr > 0 size 
     */

    bsize = 0;
    err = lf_read_attr(fhandle, ohandle, name, &bsize, (char *) NULL);
    if (err != ENOMEM) {
        // fprintf(stderr, "attribute lookup error: %s\n", name);
        return NULL;
    }

    err = lf_alloc_buffer(fhandle, bsize, (char **) &ptr);
    if (err) {
        fprintf(stderr, "alloc error\n");
        return NULL;
    }

    err = lf_read_attr(fhandle, ohandle, name, &bsize, (char *) ptr);
    if (err) {
        fprintf(stderr, "attribute read error: %s\n", name);
        return NULL;
    }

    return ptr;
}

void
ft_free(lf_fhandle_t fhandle, char *ptr)
{
    int             err;
    err = lf_free_buffer(fhandle, ptr);
    // assert(err == 0);
}



u_int32_t
ii_probe(ii_image_t * ii, dim_t x, dim_t y)
{
    u_int32_t       value;

    ASSERTX(value = 0, x >= 0);
    ASSERTX(value = 0, y >= 0);
    ASSERTX(value = 0, x < ii->width);
    ASSERTX(value = 0, y < ii->height);
    value = ii->data[y * ii->width + x];
  done:
    return value;
}




int
write_param(lf_fhandle_t fhandle, lf_obj_handle_t ohandle, char *fmt,
            search_param_t * param, int i)
{
    off_t           bsize;
    char            buf[BUFSIZ];
    int             err;

#ifdef VERBOSE
    lf_log(fhandle, LOGL_TRACE, "FOUND!!! ul=%ld,%ld; scale=%f\n",
           param->bbox.xmin, param->bbox.ymin, param->scale);
#endif
    sprintf(buf, fmt, i);
    bsize = sizeof(search_param_t);
    err = lf_write_attr(fhandle, ohandle, buf, bsize, (char *) param);

    return err;
}

int
read_param(lf_fhandle_t fhandle, lf_obj_handle_t ohandle, char *fmt,
           search_param_t * param, int i)
{
    off_t           bsize;
    char            buf[BUFSIZ];
    int             err;

    sprintf(buf, fmt, i);
    bsize = sizeof(search_param_t);
    err = lf_read_attr(fhandle, ohandle, buf, &bsize, (char *) param);

    return err;
}


/*
 ********************************************************************** */

int
log2_int(int x)
{
    int             logval = 0;

    /*
     * garbage in, garbage out 
     */

    switch (x) {
    case 4:
        logval = 2;
        break;
    case 8:
        logval = 3;
        break;
    case 16:
        logval = 4;
        break;
    case 32:
        logval = 5;
        break;
    default:
        while (x > 1) {
            // assert((x & 1) == 0);
            x >>= 1;
            logval++;
        }
    }

    return logval;
}


/*
 ********************************************************************** */
