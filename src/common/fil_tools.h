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


#ifndef _FIL_TOOLS_H_
#define _FIL_TOOLS_H_

#include "filter_api.h"
#include "face.h"

#ifdef __cplusplus
extern "C" {
#endif

char *ft_read_alloc_attr(lf_fhandle_t fhandle, lf_obj_handle_t ohandle, 
			 const char *name);
void ft_free(lf_fhandle_t fhandle, char *ptr);


/* 
 * integral image
 */
typedef struct ii_image {
	size_t    nbytes;	/* size of variable struct */
	dim_t     width, height;
	u_int32_t data[0];	/* variable size struct. width*height
				 * elements of data follows. */
} ii_image_t;


//extern u_int32_t ii_probe(ii_image_t *ii, dim_t x, dim_t y);

#if 0
#define II_PROBE(ii,x,y) ii_probe(ii, x, y)
#endif

#define II_PROBE(ii,x,y) 			\
(assert((x) >= 0), assert((y) >= 0),           \
 assert((x) < (ii)->width),			\
 assert((y) < (ii)->height),			\
 (ii)->data[(y) * ((ii)->width) + (x)])

typedef struct ii2_image {
	size_t    nbytes;	/* size of variable struct */
	dim_t     width, height;
	float    data[0];	/* variable size struct. width*height
				 * elements of data follows. */
} ii2_image_t;



int write_param(lf_fhandle_t fhandle, lf_obj_handle_t ohandle, char *fmt,
		search_param_t *param, int i);
int read_param(lf_fhandle_t fhandle, lf_obj_handle_t ohandle, char *fmt,
	       search_param_t *param, int i);


/* ********************************************************************** */

int log2_int(int x);

#ifdef __cplusplus
}
#endif


#endif /*  _FIL_TOOLS_H_ */
