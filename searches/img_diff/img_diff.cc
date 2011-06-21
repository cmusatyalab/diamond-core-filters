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
#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>

#include <sys/queue.h>
#include "snapfind_consts.h"
#include "lib_results.h"
#include "lib_sfimage.h"
#include "img_diff.h"
#include "img_search.h"
#include "fil_img_diff.h"
#include "factory.h"

#define	MAX_DISPLAY_NAME	64

/* These are the tokens used in the config files */
#define	METRIC_ID	"METRIC"

extern "C" {
	diamond_public
	void search_init();
}

/*
 * Here we register the factories 
 */
void
search_init()
{
	img_diff_factory *fac;

	fac = new img_diff_factory;

	factory_register(fac);
}



img_diff::img_diff(const char *name, const char *descr)
		: example_search(name, descr)
{
	distance = DEFAULT_DISTANCE;
	edit_window = NULL;
	set_scale_control(0);
	set_size_control(0);
	set_stride_control(0);
}

img_diff::~img_diff()
{
	free((char *) get_auxiliary_data());
	fprintf(stderr, "img_diff_destruct \n");
	return;
}

void
img_diff::set_distance(char *data)
{
	distance = atof(data);
	if (distance < 0) {
		distance = 0;
	} else if (distance > MAX_DISTANCE) {
		distance = MAX_DISTANCE;
	}
	return;
}

void
img_diff::set_distance(int sim)
{
	distance = sim;
	if (distance < 0) {
		distance = 0;
	} else if (distance > MAX_DISTANCE) {
		distance = MAX_DISTANCE;
	}
	return;
}


int
img_diff::handle_config(int nconf, char **data)
{
	int	err;

	if (strcmp(METRIC_ID, data[0]) == 0) {
		assert(nconf > 1);
		set_distance(data[1]);
		err = 0;
	} else {
		err = example_search::handle_config(nconf, data);
	}
	return(err);
}

void
img_diff::truncate_patch_list()
{
	/* Only keep the most recent patch */
	while (num_patches > 1) {
		remove_patch(TAILQ_FIRST(&ex_plist));
	}
}

static void
cb_close_edit_window(GtkWidget* item, gpointer data)
{
	img_diff *	search;

	search = (img_diff *)data;
	search->close_edit_win();
}


void
img_diff::close_edit_win()
{
	/* save any changes from the edit windows */
	save_edits();

	/* call the parent class to give them chance to cleanup */
	example_search::close_edit_win();

	edit_window = NULL;
}


static void
edit_search_done_cb(GtkButton *item, gpointer data)
{
	GtkWidget * widget = (GtkWidget *)data;
	gtk_widget_destroy(widget);
}



void
img_diff::edit_search()
{
	GtkWidget * widget;
	GtkWidget * window_cntrl;
	GtkWidget * box;
	GtkWidget * hbox;
	char	    name[MAX_DISPLAY_NAME];

	/*
	 * see if it already exists, if so raise to top and return.
	 */
	if (edit_window != NULL) {
		gdk_window_raise(GTK_WIDGET(edit_window)->window);
		return;
	}

	truncate_patch_list();

	edit_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(name, MAX_DISPLAY_NAME - 1, "Edit %s", get_name());
	name[MAX_DISPLAY_NAME -1] = '\0';
	gtk_window_set_title(GTK_WINDOW(edit_window), name);
	g_signal_connect(G_OBJECT(edit_window), "destroy",
	                 G_CALLBACK(cb_close_edit_window), this);
	box = gtk_vbox_new(FALSE, 10);

	/* create button to close the edit window */
	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, TRUE, 0);

	widget = gtk_button_new_with_label("Close");
	g_signal_connect(G_OBJECT(widget), "clicked",
	                 G_CALLBACK(edit_search_done_cb), edit_window);
	GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);

	/*
	 * Get the controls from the img_search.
	 */
	widget = img_search_display();
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);

	/*
	 * Create the search parameters.
	 */

	widget = create_slider_entry("Max Distance", 0.0, MAX_DISTANCE, 
				     			2, distance, 0.05, &sim_adj);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(edit_window), box);

	/*
	 * Get the controls from the window search class, even though the
	 * filter doesn't actually use them.
	 */
	window_cntrl = get_window_cntrl();
	gtk_box_pack_start(GTK_BOX(window_cntrl), widget, TRUE, TRUE, 0);

	/*
	 * Get the controls from the example search class.
	 */
	widget = example_display();
	gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);

	/* make everything visible except for window options */
	gtk_widget_show_all(edit_window);
	gtk_widget_hide(window_cntrl);
}

/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
img_diff::save_edits()
{
	double	sim;

	// Patches may have been added even without an edit window, so do
	// this unconditionally
	truncate_patch_list();

	/* no active edit window, so return */
	if (edit_window == NULL) {
		return;
	}

	/* get the distance and save */
	sim = gtk_adjustment_get_value(GTK_ADJUSTMENT(sim_adj));
	set_distance((int) sim);

	example_search::save_edits();
}

/*
 * This write the relevant section of the filter specification file
 * for this search.
 */

void
img_diff::write_fspec(FILE *ostream)
{
	example_patch_t *patch;

	save_edits();

	/*
	 * First we write the header section that corresponds
	 * to the filter, the filter name, the associated functions.
	 */
	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER %s \n", get_name());
	fprintf(ostream, "THRESHOLD %f\n", 100.0 - distance);
	fprintf(ostream, "SIGNATURE @\n");
	fprintf(ostream, "REQUIRES  RGB  # dependancies \n");

	// window_search writes at least one unnecessary argument.
	// Put it here where it won't matter.
	example_search::write_fspec(ostream);

	// Load the example patch into the blob argument.
	patch = TAILQ_FIRST(&ex_plist);
	if (patch != NULL) {
		set_auxiliary_data(patch->patch_image);
		set_auxiliary_data_length(patch->patch_image->nbytes);
	} else {
		set_auxiliary_data(NULL);
		set_auxiliary_data_length(0);
	}

	/* call the parent class to give them chance to cleanup */
	example_search::close_edit_win();

	edit_window = NULL;
}


void
img_diff::write_config(FILE *ostream, const char *dirname)
{
	save_edits();

	/* create the search configuration */
	fprintf(ostream, "\n\n");
	fprintf(ostream, "SEARCH img_diff %s\n", get_name());

	/* write out the parameters */
	fprintf(ostream, "%s %f \n", METRIC_ID, distance);

	example_search::write_config(ostream, dirname);
}

void img_diff::region_match(RGBImage *img, bbox_list_t *blist) 
{
}

bool
img_diff::is_editable(void)
{
	return true;
}
