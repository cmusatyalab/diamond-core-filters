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

#include "rtimer.h"
#include "fil_tools.h"
#include "rgb.h"

/*
 * this is the wrong place for these... 
 */

#ifdef __cplusplus
extern          "C" {
#endif

    void            search_param_sprint(char *buf, search_param_t * param);
    void            int_sprint(char *buf, int *v);
    void            float_sprint(char *buf, float *v);
    void            ptr_sprint(char *buf, void *ptr);
    void            cstring_sprint(char *buf, char *str);
    void            time_sprint(char *buf, rtime_t * tm);

    void            ii_image_sprint(char *buf, ii_image_t * img);
    void            rgbimage_sprint(char *buf, RGBImage * img);

#ifdef __cplusplus
}
#endif
void
search_param_sprint(char *buf, search_param_t * param)
{
    sprintf(buf, "[lev=%d-%d; %d,%d, %dx%d; s=%f]", param->lev1, param->lev2,
            param->bbox.xmin, param->bbox.ymin, param->bbox.xsiz,
            param->bbox.ysiz, param->scale);
}

void
int_sprint(char *buf, int *v)
{
    sprintf(buf, "%d", *v);
}

void
float_sprint(char *buf, float *v)
{
    sprintf(buf, "%f", *v);
}


void
ptr_sprint(char *buf, void *ptr)
{
    sprintf(buf, "%p", ptr);
}


void
cstring_sprint(char *buf, char *str)
{
    sprintf(buf, "%s", str);
}


void
time_sprint(char *buf, rtime_t * tm)
{
    sprintf(buf, "%f", rt_time2secs(*tm));
}

void
ii_image_sprint(char *buf, ii_image_t * img)
{
    sprintf(buf, "%dx%d", img->width, img->height);
}

void
rgbimage_sprint(char *buf, RGBImage * img)
{
    sprintf(buf, "%dx%d", img->width, img->height);
}
