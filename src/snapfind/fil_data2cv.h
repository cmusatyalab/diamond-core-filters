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


//#ifndef _FIL_DATA2CV_H_
//#define _FIL_DATA2CV_H_

#include <opencv/cv.h>

#ifdef __cplusplus
extern "C" {
#endif

int f_init_get_rgba_ipl_image(int numarg, char **args, int blob_len, 
				void *blob_data, void **fdatap);

int f_fini_get_rgba_ipl_image(void *fdata);

int f_eval_get_rgba_ipl_image(lf_obj_handle_t ohandle, int numout, 
				lf_obj_handle_t * ohandles, void *fdata);




int f_init_get_gray_ipl_image(int numarg, char **args, int blob_len,
    void *blob_data, void **fdatap);

int f_fini_get_gray_ipl_image(void *fdata);

int f_eval_get_gray_ipl_image(lf_obj_handle_t ohandle, int numout, 
				lf_obj_handle_t * ohandles, void *fdata);


#ifdef __cplusplus
}
#endif

//#endif /* _FIL_DATA2CV_H_ */
