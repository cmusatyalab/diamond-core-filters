
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <errno.h>
#include "queue.h"
#include "rgb.h"
#include "img_search.h"

#define	MAX_DISPLAY_NAME	64

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
rgb_img::handle_config(config_types_t conf_type, char *data)
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
	fprintf(ostream, "EVAL_FUNCTION  f_eval_pnm2rgb  # eval function \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_pnm2rgb  # init function \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_pnm2rgb  # fini function \n");
	//fprintf(ostream, "EVAL_FUNCTION  f_eval_attr2rgb  # eval function \n");
	//fprintf(ostream, "INIT_FUNCTION  f_init_attr2rgb  # init function \n");
	//fprintf(ostream, "FINI_FUNCTION  f_fini_attr2rgb  # fini function \n");
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

