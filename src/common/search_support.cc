/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>		/* dirname */
#include <assert.h>
#include <stdint.h>
#include <signal.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef linux
#include <getopt.h>
#else
#ifndef HAVE_DECL_GETOPT
#define HAVE_DECL_GETOPT 1
#endif
#include <gnugetopt/getopt.h>
#endif

#include "lib_searchlet.h"
#include "gui_thread.h"

#include "lib_filter.h"
#include "lib_log.h"

#include "queue.h"
#include "ring.h"
#include "rtimer.h"

#include "lib_results.h"
#include "rgb.h"
#include "fil_tools.h"
#include "lib_sfimage.h"
#include "img_search.h"
#include "search_support.h"
#include "import_sample.h"
#include "search_set.h"


int
search_exists(const char *name, search_set *set)
{
	img_search *cur;
	search_iter_t	iter;

	set->reset_search_iter(&iter);
	while ((cur = set->get_next_search(&iter)) != NULL) {
		if (strcmp(cur->get_name(), name) == 0) {
			return(1);
		}
	}
	return(0);
}

