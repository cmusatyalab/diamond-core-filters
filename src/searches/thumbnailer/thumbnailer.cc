/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2008      Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/queue.h>
#include "lib_results.h"
#include "lib_sfimage.h"
#include "img_search.h"
#include "thumbnailer.h"
#include "snapfind_consts.h"


extern "C" {
void search_init();
}

void
search_init()
{
	thumbnailer *f = new thumbnailer();
	set_thumbnail_filter(f);
}


thumbnailer::thumbnailer() : img_search("THUMBNAIL", "Thumbnailer")
{
	width = THUMBSIZE_X;
	height = THUMBSIZE_Y;
	return;
}

thumbnailer::~thumbnailer()
{
	return;
}


int
thumbnailer::handle_config(int nconf, char **data)
{
	/* should never be called for this class */
	assert(0);
	return(ENOENT);
}


void
thumbnailer::edit_search()
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
thumbnailer::save_edits()
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
thumbnailer::write_fspec(FILE *ostream)
{
	fprintf(ostream, "\n"
		"FILTER  THUMBNAIL  # generate thumbnail image\n"
		"REQUIRES  RGB	# dependencies\n"
		"THRESHOLD  1  # this will always pass\n"
		"EVAL_FUNCTION  f_eval_thumbnailer  # eval function\n"
		"INIT_FUNCTION  f_init_thumbnailer  # init function\n"
		"FINI_FUNCTION  f_fini_thumbnailer  # fini function\n"
		"ARG  %u  # thumbnail image width\n"
		"ARG  %u  # thumbnail image height\n\n",
		width, height);
}

void
thumbnailer::write_config(FILE *ostream, const char *dirname)
{
	/*
	 * This should never be an editable search, so this function should
	 * never be called.
	 */
	assert(0);
	return;
}

void
thumbnailer::region_match(RGBImage *img, bbox_list_t *blist)
{
	/* XXX do something useful -:) */
	return;
}

bool
thumbnailer::is_editable(void)
{
	return false;
}
