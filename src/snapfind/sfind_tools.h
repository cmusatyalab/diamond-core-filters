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

#ifndef	_SFIND_TOOLS_H_
#define	_SFIND_TOOLS_H_

#include "rgb.h"
#include "face.h"

#include "searchlet_api.h"
#include "filter_api.h"
#include "image_common.h"
#include "fil_tools.h"

#ifdef __cplusplus
extern "C" {
#endif

	

/* reference counted bunch of handles */
typedef struct image_hooks_t {
	int		refcount;
	RGBImage        *img;
	HistoII         *histo_ii;
	ls_obj_handle_t ohandle;
} image_hooks_t;

image_hooks_t *ih_new_ref(RGBImage *img, HistoII *histo_ii, ls_obj_handle_t ohandle);
void ih_get_ref(image_hooks_t *ptr);
void ih_drop_ref(image_hooks_t *ptr, lf_fhandle_t fhandle);


/* ********************************************************************** */

double compute_scale(RGBImage *img, int xdim, int ydim);

/* ********************************************************************** */



void img_constrain_bbox(bbox_t *bbox, RGBImage *img);


#ifdef __cplusplus
}
#endif
#endif	/* _SFIND_TOOLS_H_ */

