
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <assert.h>
#include "queue.h"
#include "rgb.h"
#include "histo.h"
#include "img_search.h"
#include "gui_thread.h"
#include "snapfind.h"

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

    rgb = new rgb_img("RGB image", "RGB image");
    ss_add_dep(rgb);
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


