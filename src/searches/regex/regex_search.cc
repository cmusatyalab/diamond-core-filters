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
#include "queue.h"
#include "rgb.h"
#include "img_search.h"

#define	MAX_DISPLAY_NAME	64
regex_search::regex_search(const char *name, char *descr)
		: img_search(name, descr)
{
	search_string = NULL;
	edit_window = NULL;
	string_entry = NULL;
	return;
}

regex_search::~regex_search()
{
	if (search_string) {
		free(search_string);
	}
	return;
}


int
regex_search::handle_config(config_types_t conf_type, char *data)
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
	regex_search *    search;
	search = (regex_search *)data;
	search->close_edit_win();
}


void
regex_search::edit_search()
{
	GtkWidget *     widget;
	GtkWidget *     box;
	GtkWidget *     frame;
	GtkWidget *     hbox;
	GtkWidget *     container;
	char        name[MAX_DISPLAY_NAME];

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

	/*
	 * Create the texture parameters.
	 */
	frame = gtk_frame_new("Regex Search");
	container = gtk_hbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), container);

	widget = gtk_label_new("Regex String");
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

	string_entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(container), string_entry, FALSE, TRUE, 0);
	gtk_entry_set_text(GTK_ENTRY(string_entry), search_string);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(edit_window), box);
	//gtk_window_set_default_size(GTK_WINDOW(edit_window), 400, 500);
	gtk_widget_show_all(edit_window);

	return;
}



/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
regex_search::save_edits()
{
	/*
	 * This should never be an editable search, so this function should
	 * never be called.
	 */
	if (edit_window == NULL) {
		return;
	}

	if (search_string != NULL) {
		free(search_string);
	}
	search_string = strdup(gtk_entry_get_text(GTK_ENTRY(string_entry)));
	assert(search_string != NULL);
	return;
}


void
regex_search::close_edit_win()
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
regex_search::write_fspec(FILE *ostream)
{

	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER  %s  # dependancies \n", get_name());
	fprintf(ostream, "THRESHOLD  1  # number of hits ?? \n");
	fprintf(ostream, "MERIT  10000  	# guess at cost \n");
	fprintf(ostream, "EVAL_FUNCTION  f_eval_regex  # eval function \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_regex  # init function \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_regex  # fini function \n");
	fprintf(ostream, "ARG  2  # number of attributes to search \n");
	fprintf(ostream, "ARG  Keywords  # search keywords  \n");
	fprintf(ostream, "ARG  Display-Name  # search Display-Name \n");
	// XXX fprintf(ostream, "ARG  Display-Name  # search Display-Name \n");
	// XXX allow user to select attributes
	/* XXX use more than 1 regex?? */
	fprintf(ostream, "ARG  %s  # Search string \n", search_string);
	fprintf(ostream, "\n");
	fprintf(ostream, "\n");
}

void
regex_search::write_config(FILE *ostream, const char *dirname)
{

	/*
	 * This should never be an editable search, so this function should
	 * never be called.
	 */
	// XXX fix asap assert(0);
	return;
}

void
regex_search::region_match(RGBImage *img, bbox_list_t *blist)
{
	/* XXX do something useful -:) */
	return;
}

