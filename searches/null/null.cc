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
#include "null.h"
#include "factory.h"


extern "C" {
	diamond_public
        void search_init();
}


void
search_init()
{
        null_codec_factory *fac;

        fac = new null_codec_factory;

        factory_register_codec(fac);
}



null_codec::null_codec(const char *name, const char *descr)
		: img_search(name, descr)
{
	return;
}

null_codec::~null_codec()
{
	return;
}


int
null_codec::handle_config(int nconf, char **data)
{
	/* should never be called for this class */
	assert(0);
	return(ENOENT);
}


void
null_codec::edit_search()
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
null_codec::save_edits()
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
null_codec::write_fspec(FILE *ostream)
{
	fprintf(ostream, "\n\n");
	fprintf(ostream, "FILTER  RGB  # generates dummy rgb image \n");
	fprintf(ostream, "THRESHOLD  1  # this will always pass \n");
	fprintf(ostream, "MERIT  500  # run this early hint \n");
	fprintf(ostream, "SIGNATURE @\n");
}

void
null_codec::write_config(FILE *ostream, const char *dirname)
{
	/*
	 * This should never be an editable search, so this function should
	 * never be called.
	 */
	assert(0);
	return;
}

bool
null_codec::is_editable(void)
{
	return false;
}
