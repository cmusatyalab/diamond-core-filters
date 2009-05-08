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
#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/queue.h>
#include "snapfind_consts.h"
#include "lib_results.h"
#include "lib_sfimage.h"
#include "img_diff.h"
#include "img_search.h"
#include "fil_img_diff.h"
#include "search_set.h"
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
		: img_search(name, descr)
{
	distance = DEFAULT_DISTANCE;
	edit_window = NULL;
}

img_diff::~img_diff()
{
	free((char *) get_auxiliary_data());
	printf("img_diff_destruct \n");
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
		err = img_search::handle_config(nconf, data);
	}
	return(err);
}

static GtkWidget *
create_slider_entry(const char *name, float min, float max, int dec, float initial,
                    float step, GtkObject **adjp)
{
	GtkWidget *container;
	GtkWidget *scale;
	GtkWidget *button;
	GtkWidget *label;


	container = gtk_hbox_new(FALSE, 10);

	label = gtk_label_new(name);
	gtk_box_pack_start(GTK_BOX(container), label, FALSE, FALSE, 0);

	if (max <= 1.0) {
		max += 0.1;
		*adjp = gtk_adjustment_new(min, min, max, step, 0.1, 0.1);
	} else if (max < 50) {
		max++;
		*adjp = gtk_adjustment_new(min, min, max, step, 1.0, 1.0);
	} else {
		max+= 10;
		*adjp = gtk_adjustment_new(min, min, max, step, 10.0, 10.0);
	}
	gtk_adjustment_set_value(GTK_ADJUSTMENT(*adjp), initial);

	scale = gtk_hscale_new(GTK_ADJUSTMENT(*adjp));
	gtk_widget_set_size_request (GTK_WIDGET(scale), 200, -1);
	gtk_range_set_update_policy (GTK_RANGE(scale), GTK_UPDATE_CONTINUOUS);
	gtk_scale_set_draw_value (GTK_SCALE(scale), FALSE);
	gtk_box_pack_start (GTK_BOX(container), scale, TRUE, TRUE, 0);
	gtk_widget_set_size_request(scale, 120, 0);

	button = gtk_spin_button_new(GTK_ADJUSTMENT(*adjp), step, dec);
	gtk_box_pack_start(GTK_BOX(container), button, FALSE, FALSE, 0);

	gtk_widget_show(container);
	gtk_widget_show(label);
	gtk_widget_show(scale);
	gtk_widget_show(button);

	return(container);
}

GtkWidget *
make_menu_item (gchar* name, GCallback callback, gpointer  data)
{
	GtkWidget *item;

	item = gtk_menu_item_new_with_label(name);
	g_signal_connect(G_OBJECT(item), "activate", callback, (gpointer) data);
	gtk_widget_show(item);

	return item;
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
	int fd;
	int err;
	ssize_t nbytes;
	char *buf;
	struct stat     stats;
		
	/* save any changes from the edit windows */
	save_edits();

	/* grab the example image */
	fd = open(get_example_name(), O_RDONLY);
	assert(fd >=0);

	err = fstat(fd, &stats);

	if (err < 0) {
	  	close(fd);
        assert(0);
	}

	buf = (char *) malloc(stats.st_size);
	if (buf == NULL) {
	  close(fd);
	  assert(0);
	}

	nbytes = read(fd, buf, stats.st_size);
	if (nbytes < stats.st_size) {
	  close(fd);
	  assert(0);
	}

	log_message(LOGT_APP, LOGL_TRACE, 
				"auxiliary data: %s file %s buf %x len %d",
				get_name(), get_example_name(), buf, nbytes);

	set_auxiliary_data((void *) buf);
	set_auxiliary_data_length(nbytes);

	close(fd);

	/* call the parent class to give them chance to cleanup */
	img_search::close_edit_win();

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

	/* make everything visible */
	gtk_widget_show_all(edit_window);

}

/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
img_diff::save_edits()
{
	double	sim;

	/* no active edit window, so return */
	if (edit_window == NULL) {
		return;
	}

	/* get the distance and save */
	sim = gtk_adjustment_get_value(GTK_ADJUSTMENT(sim_adj));
	set_distance((int) sim);

	img_search::save_edits();
}

/*
 * This write the relevant section of the filter specification file
 * for this search.
 */

void
img_diff::write_fspec(FILE *ostream)
{
	save_edits();

	/*
	 * First we write the header section that corresponds
	 * to the filter, the filter name, the associated functions.
	 */
	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER %s \n", get_name());
	fprintf(ostream, "THRESHOLD 1\n");
	fprintf(ostream, "EVAL_FUNCTION  f_eval_img_diff \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_img_diff \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_img_diff \n");
	fprintf(ostream, "ARG  %f  # distance \n", distance);
	fprintf(ostream, "REQUIRES  RGB  # dependancies \n");
	fprintf(ostream, "MERIT  10  # some relative cost \n\n");

	return;
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

	return;
}

void img_diff::region_match(RGBImage *img, bbox_list_t *blist) 
{
}

int img_diff::matches_filter(char *name) 
{
	void *handle;
	void *initfn = NULL;
                                                                                
	handle = dlopen(name, RTLD_NOW);
	if (handle) {
		initfn = dlsym(handle, "f_init_img_diff");
		dlclose(handle);
	}
	
	return ((int) initfn);
}

bool
img_diff::is_editable(void)
{
	return true;
}
