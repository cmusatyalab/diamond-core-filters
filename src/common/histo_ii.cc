
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

histo_ii::histo_ii(const char *name, char *descr)
	: img_search(name, descr)
{
	return;
}

histo_ii::~histo_ii()
{
	return;
}


int
histo_ii::handle_config(config_types_t conf_type, char *data)
{
	/* should never be called for this class */
	assert(0);
	return(ENOENT);	
}


void
histo_ii::edit_search()
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
histo_ii::save_edits()
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
histo_ii::write_fspec(FILE *ostream)
{
	img_search *	rgb;

	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER  HISTO_II  # name \n");
	fprintf(ostream, "THRESHOLD  1  # threshold \n");
	fprintf(ostream, "EVAL_FUNCTION  f_eval_hintegrate  # evan fn \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_hintegrate  # init fn \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_hintegrate  # fini fn \n");
	fprintf(ostream, "REQUIRES  RGB  # dependancies \n");
	fprintf(ostream, "MERIT  200  # merit value \n");
	fprintf(ostream, "ARG  4  # dependancies \n");
	fprintf(ostream, "ARG  %d  # dependancies \n", HISTO_INTERPOLATED);

    	rgb = new rgb_img("RGB image", "RGB image");
	(this->get_parent())->add_dep(rgb);
}

void
histo_ii::write_config(FILE *ostream, const char *dirname)
{

	/*
	 * This should never be an editable search, so this function should
	 * never be called.
	 */
	assert(0);
	return;
}

void
histo_ii::region_match(RGBImage *img, bbox_list_t *blist)
{
    /* XXX do something useful -:) */
    return;
}


