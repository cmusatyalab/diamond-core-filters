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

#include "filter_api.h"
#include "searchlet_api.h"
#include "gui_thread.h"

#include "queue.h"
#include "ring.h"
#include "rtimer.h"
#include "sf_consts.h"

#include "face_search.h"
#include "face_image.h"
#include "rgb.h"
#include "face.h"
#include "fil_tools.h"
#include "image_tools.h"
#include "face_widgets.h"
#include "texture_tools.h"
#include "img_search.h"
#include "sfind_search.h"
#include "search_support.h"
#include "sfind_tools.h"
#include "snap_popup.h"
#include "snapfind.h"
#include "import_sample.h"

/* XXXX fix this */
#define MAX_SEARCHES    64
extern img_search * snap_searches[MAX_SEARCHES];
extern int num_searches;


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
		
		default:
			cur_search = NULL;

	}
	return(cur_search);
}


int
search_exists(const char *name)
{
	int		i;
	for (i=0; i < num_searches; i++) {
		if (strcmp(snap_searches[i]->get_name(), name) == 0) {
			return (1);
		}
	}
	return(0);
}


void
search_add_list(img_search *new_search)
{

	/* XXX do some error checks */
	snap_searches[num_searches] = new_search;
	num_searches++;

	/* update the entry in the main panel */
	update_search_entry(new_search, num_searches);

	/* add the entry to the search popup list */
	search_popup_add(new_search, num_searches);

	import_update_searches();

}
