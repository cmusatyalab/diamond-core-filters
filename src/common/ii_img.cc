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
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <errno.h>
#include "queue.h"
#include "rgb.h"
//#include "histo.h"
#include "img_search.h"
#include "search_set.h"

#define	MAX_DISPLAY_NAME	64

ii_img::ii_img(const char *name, char *descr)
		: img_search(name, descr)
{
	return;
}

ii_img::~ii_img()
{
	return;
}


int
ii_img::handle_config(config_types_t conf_type, char *data)
{
	/* should never be called for this class */
	assert(0);
	return(ENOENT);
}


void
ii_img::edit_search()
{

	/* should never be called for this class */
	assert(0);
	return;
}



/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
ii_img::save_edits()
{
	/*
	 * This should never be an editable search, so this function should
	 * never be called.
	 */
	assert(0);
	return;
}

/*
 * This write the relevant section of the filter specification file
 * for this search.
 */

void
ii_img::write_fspec(FILE *ostream)
{
	img_search *	rgb;

	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER  INTEGRATE  # dependancies \n");
	fprintf(ostream, "THRESHOLD  1  # dependancies \n");
	fprintf(ostream, "EVAL_FUNCTION  f_eval_integrate  # dependancies \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_integrate  # dependancies \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_integrate  # dependancies \n");
	fprintf(ostream, "REQUIRES  RGB  # dependancies \n");
	fprintf(ostream, "MERIT  30  # merit value \n");

	rgb = new rgb_img("RGB image", "RGB image");
	(this->get_parent())->add_dep(rgb);
}

void
ii_img::write_config(FILE *ostream, const char *dirname)
{

	/*
	 * This should never be an editable search, so this function should
	 * never be called.
	 */
	assert(0);
	return;
}

void
ii_img::region_match(RGBImage *img, bbox_list_t *blist)
{
	/* XXX do something useful -:) */
	return;
}


