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
//#include "sf_consts.h"

//#include "face_search.h"
#include "face_image.h"
#include "rgb.h"
#include "face.h"
#include "fil_tools.h"
#include "image_tools.h"
//#include "face_widgets.h"
#include "texture_tools.h"
#include "img_search.h"
//#include "sfind_search.h"
#include "search_support.h"
//#include "sfind_tools.h"
//#include "snap_popup.h"
//#include "snapfind.h"
#define MAX_SELECT	64
#include "import_sample.h"
#include "search_set.h"
#include "gabor_texture_search.h"

/* XXXX fix this */
#define MAX_SEARCHES    64


img_search *
create_search(search_types_t type, const char *name)
{
	img_search *cur_search;

	switch(type) {
		case TEXTURE_SEARCH:
			cur_search = new texture_search(name, "Texture Search");
			break;

		case RGB_HISTO_SEARCH:
			cur_search = new rgb_histo_search(name, "RGB Histogram");
			break;

		case VJ_FACE_SEARCH:
			cur_search = new vj_face_search(name, "VJ Face Detect");
			break;

		case OCV_FACE_SEARCH:
			cur_search = new ocv_face_search(name, "OCV Face Detect");
			break;

		case REGEX_SEARCH:
			cur_search = new regex_search(name, "Regex Search");
			break;

		case GABOR_TEXTURE_SEARCH:
			cur_search = new gabor_texture_search(name, "Gabor Texture");
			break;

		default:
			cur_search = NULL;

	}
	return(cur_search);
}


int
search_exists(const char *name, search_set *set
             )
{
	img_search *cur;
	search_iter_t	iter;

	set
		->reset_search_iter(&iter);
	while ((cur = set
		              ->get_next_search(&iter)) != NULL) {
			if (strcmp(cur->get_name(), name) == 0) {
				return(1);
			}
		}
	return(0);
}

