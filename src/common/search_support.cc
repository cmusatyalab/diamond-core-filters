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
		
		default:
			cur_search = NULL;

	}
	return(cur_search);
}


int
search_exists(const char *name, img_search **search_list, int slist_size)
{
	int		i;
	for (i=0; i < slist_size; i++) {
		if (strcmp(search_list[i]->get_name(), name) == 0) {
			return (1);
		}
	}
	return(0);
}


#define	NUM_FNS	32
typedef	void (*update_fn_t)(img_search *searches, int num_searches);


update_fn_t	cb_fns[NUM_FNS] = {NULL, };




void
register_update_function(void (*update_fn)(img_search *searches, 
	int num_searches))
{
	int		i;
	for (i=0; i < NUM_FNS; i++) {
		if (cb_fns[i] == NULL) {
			cb_fns[i] = update_fn;
			return;
		}
	}
	/* XXX if we get here we failed */
	fprintf(stderr, "Too many update fns \n");
	assert(0);
}

void
unregister_update_function(void (*update_fn)(img_search *searches, 
	int num_searches))
{
	int		i;
	for (i=0; i < NUM_FNS; i++) {
		if (cb_fns[i] == update_fn) {
			cb_fns[i] = NULL;
			return;
		}
	}
	/* XXX if we get here we failed to find it */
	fprintf(stderr, "Too many update fns \n");
	assert(0);
}

void
search_add_list(img_search *new_search, img_search **search_list, int
	*search_list_size)
{
	int	i;

	/* XXX do some error checks */
	search_list[*search_list_size] = new_search;
	*search_list_size++;

	for (i=0; i < NUM_FNS; i++) {
		if (cb_fns[i] != NULL) {
			(*cb_fns[i])(new_search, *search_list_size);
		}
	}
}
