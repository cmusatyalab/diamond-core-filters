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


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include <libgen.h>             /* dirname */
#include <assert.h>
#include <stdint.h>

#include <opencv/cv.h>

#include "queue.h"
#include "common_consts.h"
#include "ring.h"
#include "rtimer.h"

// #include "face_search.h"
#include "face_image.h"
#include "fil_face.h"
#include "rgb.h"
#include "face.h"
#include "fil_tools.h"
#include "gui_thread.h"
#include "image_tools.h"
#include "histo.h"
#include "sfind_tools.h"
#include "texture_tools.h"
#include "facedet.h"

/*
 ********************************************************************** */

static pthread_mutex_t ih_mutex = PTHREAD_MUTEX_INITIALIZER;


image_hooks_t  *
ih_new_ref(RGBImage * img, HistoII * histo_ii, ls_obj_handle_t ohandle)
{
	image_hooks_t  *ptr = (image_hooks_t *) calloc(1, sizeof(image_hooks_t));
	assert(ptr);
	ptr->refcount = 1;

	ptr->img = img;
	ptr->histo_ii = histo_ii;
	ptr->ohandle = ohandle;

	return ptr;
}

void
ih_get_ref(image_hooks_t * ptr)
{
	pthread_mutex_lock(&ih_mutex);
	ptr->refcount++;
	pthread_mutex_unlock(&ih_mutex);
}

void
ih_drop_ref(image_hooks_t * ptr, lf_fhandle_t fhandle)
{

	pthread_mutex_lock(&ih_mutex);
	assert(ptr->refcount > 0);
	ptr->refcount--;
	if (ptr->refcount == 0) {
		ft_free(fhandle, (char *) ptr->img);
		if (ptr->histo_ii) {
			ft_free(fhandle, (char *) ptr->histo_ii);
		}
		ls_release_object(fhandle, ptr->ohandle);
		free(ptr);
	}
	pthread_mutex_unlock(&ih_mutex);
}

static inline double
max(double a, double b)
{
	return a > b ? a : b;
}

double
compute_scale(RGBImage * img, int xdim, int ydim)
{
	double          scale = 1.0;

	scale = max(scale, (double) img->width / xdim);
	scale = max(scale, (double) img->height / ydim);

	return scale;
}

