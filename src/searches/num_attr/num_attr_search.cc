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
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <values.h>
#include "queue.h"
#include "lib_results.h"
#include "rgb.h"
#include "num_attr_search.h"
#include "factory.h"

#define	MAX_DISPLAY_NAME	64

extern "C" {
void search_init();
}
		
	
#define NATTR_MIN_VALUE	(MINFLOAT)
#define NATTR_MAX_VALUE	(MAXFLOAT)
#define NATTR_STEP	(1.0)
#define NATTR_PAGE	(10.0)
#define NATTR_PAGE_SIZE	(10.0)

void 
search_init()
{
	num_attr_factory *fac;
	fac = new num_attr_factory;
	factory_register(fac);
}

num_attr_search::num_attr_search(const char *name, char *descr)
		: img_search(name, descr)
{
	edit_window = NULL;
	attr_str = NULL;
	min_value = 0.0;
	max_value = 0.0;
	drop_missing = 0;
	return;
}

num_attr_search::~num_attr_search()
{
	if (attr_str) {
		free(attr_str);
	}
	return;
}


int
num_attr_search::handle_config(int nconf, char **data)
{
	/* should never be called for this class */
	assert(0);
	return(ENOENT);
}



static void
cb_edit_done(GtkButton *item, gpointer data)
{
	GtkWidget * widget = (GtkWidget *)data;
	gtk_widget_destroy(widget);
}


static void
cb_close_edit_window(GtkWidget* item, gpointer data)
{
	num_attr_search *    search;
	search = (num_attr_search *)data;
	search->close_edit_win();
}


void
num_attr_search::edit_search()
{
	GtkWidget *     widget;
	GtkWidget *     box;
	GtkWidget *     hbox;
   	GtkAdjustment *	spin_adj;
	char        	name[MAX_DISPLAY_NAME];

	/* see if it already exists */
	if (edit_window != NULL) {
		/* raise to top ??? */
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
	gtk_container_add(GTK_CONTAINER(edit_window), box);

	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, TRUE, 0);

	widget = gtk_button_new_with_label("Close");
	g_signal_connect(G_OBJECT(widget), "clicked",
	                 G_CALLBACK(cb_edit_done), edit_window);
	GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);

	/*
	 * Get the controls from the img_search.
	 */
	widget = img_search_display();
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);


	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, TRUE, 0);
	widget = gtk_label_new("Attribute Name");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	attr_name = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), attr_name, FALSE, TRUE, 0);
	if (attr_str != NULL) {
		gtk_entry_set_text(GTK_ENTRY(attr_name), attr_str);
	}

 	hbox = gtk_hbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(box), hbox);
	widget = gtk_label_new("Min Value");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);

   	spin_adj = (GtkAdjustment *)gtk_adjustment_new(min_value, 
		NATTR_MIN_VALUE, NATTR_MAX_VALUE, NATTR_STEP, NATTR_PAGE,
		NATTR_PAGE_SIZE);	
   	min_spinner = gtk_spin_button_new(spin_adj, 1.000, 3);
	gtk_box_pack_start(GTK_BOX(hbox), min_spinner, FALSE, TRUE, 0);


	hbox = gtk_hbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(box), hbox);
	widget = gtk_label_new("Max Value");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
   	spin_adj = (GtkAdjustment *)gtk_adjustment_new(max_value, 
		NATTR_MIN_VALUE, NATTR_MAX_VALUE, NATTR_STEP, NATTR_PAGE,
		NATTR_PAGE_SIZE);	
   	max_spinner = gtk_spin_button_new(spin_adj, 1.000, 3);
	gtk_box_pack_start(GTK_BOX(hbox), max_spinner, FALSE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(box), hbox);
	dropcb = gtk_check_button_new_with_label("Drop without attribute");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dropcb), drop_missing);
	gtk_box_pack_start(GTK_BOX(hbox), dropcb, FALSE, TRUE, 0);


	gtk_widget_show_all(edit_window);

	return;
}



/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
num_attr_search::save_edits()
{
	/*
	 * This should never be an editable search, so this function should
	 * never be called.
	 */
	if (edit_window == NULL) {
		return;
	}

	if (attr_str != NULL) {
		free(attr_str);
	}


	attr_str = strdup(gtk_entry_get_text(GTK_ENTRY(attr_name)));
	assert(attr_str != NULL);
	min_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(min_spinner));
	max_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(max_spinner));
	drop_missing = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dropcb));
	return;
}


void
num_attr_search::close_edit_win()
{
	save_edits();

	/* call parent to give them a chance to cleanup */
	img_search::close_edit_win();

	edit_window = NULL;
}

/*
 * This write the relevant section of the filter specification file
 * for this search.
 */

void
num_attr_search::write_fspec(FILE *ostream)
{

	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER  %s  # dependancies \n", get_name());
	fprintf(ostream, "THRESHOLD  1  # boolean \n");
	fprintf(ostream, "MERIT  10000  	# guess at cost \n");
	fprintf(ostream, "EVAL_FUNCTION  f_eval_num_attr  # eval function \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_num_attr  # init function \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_num_attr  # fini function \n");
	fprintf(ostream, "ARG  %s  # attribute to search \n", attr_str);
	fprintf(ostream, "ARG  %f  # min value \n", min_value);
	fprintf(ostream, "ARG  %f  # min value \n", max_value);
	fprintf(ostream, "ARG  %d  # drop missing  \n", drop_missing);
	fprintf(ostream, "\n");
	fprintf(ostream, "\n");
}

void
num_attr_search::write_config(FILE *ostream, const char *dirname)
{
	/*
	 * This should never be an editable search, so this function should
	 * never be called.
	 */
	// XXX fix asap assert(0);
	return;
}

void
num_attr_search::region_match(RGBImage *img, bbox_list_t *blist)
{
	/* XXX do something useful -:) */
	return;
}

