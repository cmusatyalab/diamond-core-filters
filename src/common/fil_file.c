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
#include <assert.h>

#include "fil_file.h"
#include "fil_assert.h"


void
ff_open(lf_fhandle_t fhandle, lf_obj_handle_t obj_handle, ffile_t * fh)
{
    fh->fhandle = fhandle;
    fh->obj_handle = obj_handle;
    fh->len = 0;
    fh->buf = NULL;
    fh->pos = 0;
}


void
ff_close(ffile_t * fh)
{
    if (fh->buf) {
        lf_free_buffer(fh->fhandle, fh->buf);
    }
}


#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#define BLOCKSIZE (4<<10)

static int
ff_get_block(ffile_t * fh, size_t size)
{
    int             err;
    fh->len = max(size, BLOCKSIZE);
    err = lf_next_block(fh->fhandle, fh->obj_handle, 1, &fh->len, &fh->buf);
    /*
     * assert(!err && "lf_next_block"); 
     */
    ASSERTX((fh->len = 0), !err);

done:
    return err;
}


size_t
ff_read(ffile_t * fh, char **data, size_t size)
{
    size_t          nbytes = 0;
    size_t          remaining;
    int             err = 0;

    ASSERTX(nbytes = 0, size);

    if (fh->pos >= fh->len) {
        if (fh->buf) {
            lf_free_buffer(fh->fhandle, fh->buf);
            fh->buf = NULL;
        }
        err = ff_get_block(fh, size);   /* if err, rest works */
        if (err) {
            log_message(LOGT_FILT, LOGL_ERR,
                        "error %d getting block: ff_get_block (%d bytes)",
                        err, size);
            fprintf(stderr,
                    "ERROR %d getting block: ff_get_block (%d bytes)\n", err,
                    size);
        }
        fh->pos = 0;
    }

    *data = &fh->buf[fh->pos];
    remaining = fh->len - fh->pos;
    nbytes = min(remaining, size);
    fh->pos += nbytes;

    assert(!err || !nbytes);    /* err => (nbytes == 0) */

done:
    return nbytes;
}




/*
 * setup a pointer into the current buffer, len 
 */
void
ff_getbuf(ffile_t * fh, char **buf, size_t * len)
{
    if (!fh->buf) {
        ff_get_block(fh, BLOCKSIZE);
    }

    *buf = fh->buf;             /* buffer */
    *len = fh->len - fh->pos;   /* bytes remaining */
}

/*
 * mark some number of bytes as read 
 */
void
ff_consume(ffile_t * fh, size_t size)
{

    ASSERTX(assert(0), (off_t) (fh->pos + size) < fh->len);
    fh->pos += size;

  done:
    return;
}
