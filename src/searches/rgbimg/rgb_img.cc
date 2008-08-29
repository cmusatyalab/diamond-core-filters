/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
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
#include "rgb_img.h"
#include "factory.h"


extern "C" {
        void search_init();
}
                                                                                

void
search_init()
{
        rgb_img_factory *fac;
        // XXX printf("rgb init \n");

        fac = new rgb_img_factory;

        factory_register_rgbimage(fac);

}



rgb_img::rgb_img(const char *name, char *descr)
		: img_search(name, descr)
{
	return;
}

rgb_img::~rgb_img()
{
	return;
}


int
rgb_img::handle_config(int nconf, char **data)
{
	/* should never be called for this class */
	assert(0);
	return(ENOENT);
}


void
rgb_img::edit_search()
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
rgb_img::save_edits()
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
rgb_img::write_fspec(FILE *ostream)
{
	fprintf(ostream, "\n\n");
	fprintf(ostream, "FILTER  RGB  # convert file to rgb \n");
	fprintf(ostream, "THRESHOLD  1  # this will always pass \n");
	fprintf(ostream, "MERIT  500  # run this early hint \n");

	fprintf(ostream, "EVAL_FUNCTION  f_eval_img2rgb  # eval function \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_img2rgb  # init function \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_img2rgb  # fini function \n");
}

void
rgb_img::write_config(FILE *ostream, const char *dirname)
{

	/*
	 * This should never be an editable search, so this function should
	 * never be called.
	 */
	assert(0);
	return;
}

void
rgb_img::region_match(RGBImage *img, bbox_list_t *blist)
{
	/* XXX do something useful -:) */
	return;
}

