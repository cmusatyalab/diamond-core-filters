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


/*
 * Synthetic filter to generate load.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "filter_api.h"
#include "fil_file.h"
#include "fil_image_tools.h"
#include "face.h"
#include "fil_data2ii.h"
#include "fil_tools.h"
#include "fil_assert.h"
#include "fil_synth.h"


void
compute_val(int data, int loop)
{
    int             i;
    double          foo = (double) data;

    for (i = 0; i < loop; i++) {
        foo = foo / i;
    }


}

int
f_synth(lf_obj_handle_t ohandle, int numout, lf_obj_handle_t * ohandles,
        int numarg, char **args, int blob_len, void *blob_data)
{
    int             err = 0;
    lf_fhandle_t    fhandle = 0;
    off_t           blen;
    double          thresh;;
    int             loop;
    int             limit;
    char           *data;
    int             ran;
    int             i;

    lf_log(fhandle, LOGL_TRACE, "\nf_synth: enter\n");

    if (numarg != 2) {
        exit(1);
    }
    thresh = atof(args[0]);
    loop = atoi(args[1]);


    err = lf_next_block(fhandle, ohandle, 1, &blen, &data);
    while (err == 0) {
        for (i = 0; i < blen; i++) {
            compute_val(data[i], loop);
        }
        err = lf_free_buffer(fhandle, data);
        assert(err == 0);

        err = lf_next_block(fhandle, ohandle, 1, &blen, &data);
    }


    limit = (int) ((double) RAND_MAX * thresh);
    ran = random();

    if (ran > limit) {
        return (0);
    } else {
        return (1);
    }

}
