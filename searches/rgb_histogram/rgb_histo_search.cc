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
#include <sys/queue.h>
#include "snapfind_consts.h"
#include "rgb.h"
#include "lib_results.h"
#include "rgb_histo.h"
#include "rgb_histo_search.h"
#include "img_search.h"
#include "factory.h"

#define	MAX_DISPLAY_NAME	64

/* These are the tokens used in the config files */
#define	METRIC_ID	"METRIC"
#define BINS_ID		"BINS"
#define INTERPOLATION_ID	"INTERPOLATION"
#define DISTANCE_METRIC_ID	"DISTANCEMETRIC"

extern "C" {
	diamond_public
	void search_init();
}

void
search_init()
{
	rgb_histo_factory *fac;

	fac = new rgb_histo_factory;

	factory_register(fac);
}



rgb_histo_search::rgb_histo_search(const char *name, const char *descr)
		: example_search(name, descr)
{
	metric = 0;
	similarity = 0.93;
	bins = 3;
	edit_window = NULL;
	htype = HISTO_INTERPOLATED;
}

rgb_histo_search::~rgb_histo_search()
{
	fprintf(stderr, "rgb_destruct \n");
	return;
}

void
rgb_histo_search::set_bins(int new_bins)
{
	/* XXX any bounds checks */
	bins = new_bins;
	return;
}


void
rgb_histo_search::set_similarity(char *data)
{
	similarity = atof(data);
	if (similarity < 0) {
		similarity = 0.0;
	} else if (similarity > 1.0) {
		similarity = 1.0;
	}
	return;
}

void
rgb_histo_search::set_similarity(double sim)
{
	similarity = sim;
	if (similarity < 0) {
		similarity = 0.0;
	} else if (similarity > 1.0) {
		similarity = 1.0;
	}
	return;
}


int
rgb_histo_search::handle_config(int nconf, char **data)
{
	int	err;

	if (strcmp(METRIC_ID, data[0]) == 0) {
		assert(nconf > 1);
		set_similarity(data[1]);
		err = 0;
	} else if (strcmp(BINS_ID, data[0]) == 0) {
		assert(nconf > 1);
		bins = atoi(data[1]);
		err = 0;
	} else if (strcmp(INTERPOLATION_ID, data[0]) == 0) {
		assert(nconf > 1);
		htype = (histo_type_t) atoi(data[1]);
		err = 0;
	} else if (strcmp(DISTANCE_METRIC_ID, data[0]) == 0) {
		assert(nconf > 1);
		metric = atoi(data[1]);
		err = 0;
	} else {
		err = example_search::handle_config(nconf, data);
	}
	return(err);
}

static void
cb_update_menu_select(GtkWidget* item, GtkUpdateType  policy)
{
	fprintf(stderr, "policy %d \n", policy);
}


static void
cb_close_edit_window(GtkWidget* item, gpointer data)
{
	rgb_histo_search *	search;

	search = (rgb_histo_search *)data;
	search->close_edit_win();
}


void
rgb_histo_search::close_edit_win()
{

	/* save any changes from the edit windows */
	save_edits();

	/* call the parent class to give them change to cleanup */
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
rgb_histo_search::edit_search()
{
	GtkWidget * widget;
	GtkWidget * box;
	GtkWidget * hbox;
	GtkWidget * opt;
	GtkWidget * item;
	GtkWidget * frame;
	GtkWidget * container;
	GtkWidget * menu;
	char		name[MAX_DISPLAY_NAME];

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
	 	 * Create the histo parameters.
	 */

	frame = gtk_frame_new("RGB Histo Params");
	container = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), container);

	widget = create_slider_entry("Min Similarity", 0.0, 1.0, 2,
	                             similarity, 0.05, &sim_adj);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

	widget = create_slider_entry("Bins", 1, 20, 0,
	                             bins, 1, &bin_adj);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);


	interpolated_box = gtk_check_button_new_with_label("Interpolated Histogram");
	if (htype == HISTO_INTERPOLATED) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(interpolated_box), TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(interpolated_box), FALSE);
	}

	gtk_box_pack_start(GTK_BOX(container), interpolated_box, FALSE, TRUE, 0);

	opt = gtk_option_menu_new();
	menu = gtk_menu_new();

	item = make_menu_item("L1 Distance", G_CALLBACK(cb_update_menu_select),
	                      GINT_TO_POINTER(0));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = make_menu_item ("L2 Distance", G_CALLBACK (cb_update_menu_select),
	                       GINT_TO_POINTER(1));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = make_menu_item ("Earth Movers", G_CALLBACK(cb_update_menu_select),
	                       GINT_TO_POINTER(2));
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item);

	gtk_option_menu_set_menu(GTK_OPTION_MENU (opt), menu);
	//	gtk_box_pack_start(GTK_BOX(container), opt, FALSE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);


	/*
	 * Get the controls from the window search class.
		 */
	widget = get_window_cntrl();
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);

	widget = example_display();
	gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);


	gtk_container_add(GTK_CONTAINER(edit_window), box);

	//gtk_window_set_default_size(GTK_WINDOW(edit_window), 400, 500);
	gtk_widget_show_all(edit_window);

}

/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
rgb_histo_search::save_edits()
{
	int		bins;
	int		val;
	double	sim;

	/* no active edit window, so return */
	if (edit_window == NULL) {
		return;
	}

	/* get the number of bins and save */
	bins = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(bin_adj));
	set_bins(bins);

	/* get the similarity and save */
	sim = gtk_adjustment_get_value(GTK_ADJUSTMENT(sim_adj));
	set_similarity(sim);

	val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(interpolated_box));
	if (val) {
		htype = HISTO_INTERPOLATED;
	} else {
		htype = HISTO_SIMPLE;
	}
	/* call the parent class */
	example_search::save_edits();
}

/*
 * This write the relevant section of the filter specification file
 * for this search.
 */

void
rgb_histo_search::write_fspec(FILE *ostream)
{
	Histo 	hgram;
	example_patch_t	*cur_patch;
	int		i = 0;

	save_edits();

	/*
		 * First we write the header section that corrspons
		 * to the filter, the filter name, the assocaited functions.
		 */

	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER %s \n", get_name());
	fprintf(ostream, "THRESHOLD %d \n", (int)(100.0 * similarity));
	fprintf(ostream, "EVAL_FUNCTION  f_eval_histo\n");
	fprintf(ostream, "INIT_FUNCTION  f_init_histo\n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_histo\n");
	fprintf(ostream, "ARG  %s  # name \n", get_name());

	/*
	 * Next we write call the parent to write out the releated args,
	 * note that since the args are passed as a vector of strings
	 * we need keep the order the args are written constant or silly
	 * things will happen.
	 */
	example_search::write_fspec(ostream);

	/*
	 * Now write the state needed that is just dependant on the histogram
	 * search.  This will have the histo releated parameters
	 * as well as the linearized histograms.
	 */

	fprintf(ostream, "ARG  %d  # num bins \n", HBINS);
	fprintf(ostream, "ARG  %f  # similarity \n", 0.0);
	fprintf(ostream, "ARG  %d  # distance metric \n", metric);
	fprintf(ostream, "ARG  %d  # histo type \n", htype);
	fprintf(ostream, "ARG  %d  # num examples \n", num_patches);


	TAILQ_FOREACH(cur_patch, &ex_plist, link) {
		int	bins;
		histo_fill_from_subimage(&hgram, cur_patch->patch_image,
		                         0, 0, cur_patch->xsize, cur_patch->ysize, htype);
		normalize_histo(&hgram);
		for (bins=0; bins < (HBINS * HBINS * HBINS); bins++) {
			fprintf(ostream, "ARG  %f  # sample %d val %d \n",
			        hgram.data[bins], i, bins);
		}
		i++;
	}
	fprintf(ostream, "REQUIRES  HISTO_II # dependancies \n");
	fprintf(ostream, "MERIT 200  # some merit \n");
	fprintf(ostream, "\n");


	/*
	 * This search actually relies on two different filters.
	 * The arguments are fixed, so it is ok to include this
	 * multiple times??
	 */
	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER  HISTO_II  # name \n");
	fprintf(ostream, "THRESHOLD  1  # threshold \n");
	fprintf(ostream, "EVAL_FUNCTION  f_eval_histo\n");
	fprintf(ostream, "INIT_FUNCTION  f_init_histo\n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_histo\n");
	fprintf(ostream, "REQUIRES  RGB  # dependancies \n");
	fprintf(ostream, "MERIT  200  # merit value \n");
	fprintf(ostream, "ARG  4  # dependancies \n");
	fprintf(ostream, "ARG  %d  # dependancies \n", HISTO_INTERPOLATED);
}


void
rgb_histo_search::write_config(FILE *ostream, const char *dirname)
{
	save_edits();

	/* create the search configuration */
	fprintf(ostream, "\n\n");
	fprintf(ostream, "SEARCH rgb_histogram %s\n", get_name());

	/* write out the rgb parameters */
	fprintf(ostream, "%s %f \n", METRIC_ID, similarity);
	fprintf(ostream, "%s %d \n", BINS_ID, bins);
	fprintf(ostream, "%s %d \n", INTERPOLATION_ID, htype);
	fprintf(ostream, "%s %d \n", DISTANCE_METRIC_ID, metric);

	example_search::write_config(ostream, dirname);
	return;
}

void
rgb_histo_search::region_match(RGBImage *img, bbox_list_t *blist)
{
	histo_config_t	hconfig;
	histo_patch_t	*	hpatch;
	example_patch_t* epatch;
	int				i;
	int				pass;
	HistoII *		ii;

	save_edits();

	hconfig.name = strdup(get_name());
	assert(hconfig.name != NULL);

	hconfig.req_matches = 1;	/* XXX */
	hconfig.scale = get_scale();
	hconfig.xsize = get_testx();
	hconfig.ysize = get_testy();
	hconfig.stride = get_stride();
	hconfig.bins = HBINS;	/* XXX */
	hconfig.similarity = similarity;
	hconfig.distance_type = metric;
	hconfig.type = htype;

	TAILQ_INIT(&hconfig.histo_patchlist);

	i = 0;
	TAILQ_FOREACH(epatch, &ex_plist, link) {
		hpatch = (histo_patch_t *) malloc(sizeof(*hpatch));
		histo_clear(&hpatch->histo);
		assert(hpatch != NULL);

		histo_fill_from_subimage(&hpatch->histo, epatch->patch_image,
		                         0, 0,  epatch->xsize, epatch->ysize, htype);
		normalize_histo(&hpatch->histo);
		hpatch->histo.weight = 1.0;

		TAILQ_INSERT_TAIL(&hconfig.histo_patchlist, hpatch, link);
		i++;
	}
	hconfig.num_patches = i;

	ii = histo_get_ii(&hconfig, img);
	pass =  histo_scan_image(hconfig.name, img, ii, &hconfig,
	                         INT_MAX /* XXX */, blist);

	/* XXX cleanup */

	return;
}



bool
rgb_histo_search::is_editable(void)
{
	return true;
}
