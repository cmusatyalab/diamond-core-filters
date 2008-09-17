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
#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <values.h>
#include <sys/queue.h>
#include "lib_results.h"
#include "rgb.h"
#include "num_attr_search.h"
#include "factory.h"

#define	MAX_DISPLAY_NAME	64

extern "C" {
void search_init();
}

/*
 * configuration constants for setting up the 
 * value spinner bars.
 */ 
#define NATTR_MIN_VALUE	(MINFLOAT)
#define NATTR_MAX_VALUE	(MAXFLOAT)
#define NATTR_STEP	(1.0)
#define NATTR_PAGE	(10.0)
#define NATTR_PAGE_SIZE	(10.0)


/* config file tokens that we write out */
#define	SEARCH_NAME	"num_attr"
#define	DROP_MISSING_ID	"DROP_MISSING"
#define	ATTR_NAME_ID	"ATTR_NAME"
#define	MIN_VALUE_ID	"MIN_VALUE"
#define	MAX_VALUE_ID	"MAX_VALUE"


void 
search_init()
{
	num_attr_factory *fac;
	fac = new num_attr_factory;
	factory_register(fac);
}

void
num_attr_search::add_num_attr_node()
{
	num_attr_node * temp = (num_attr_node *) malloc(sizeof(num_attr_node));
	assert(temp);
	temp->attr_name = NULL;
	temp->min_value = 0.0;
	temp->max_value = 0.0;
	temp->drop_missing = 0;
	temp->next_num_attr = NULL;
	if (num_attr == NULL) {
		num_attr = temp;
	} else {
		num_attr_tail->next_num_attr = temp;
	}
	num_attr_tail = temp;
	
}
num_attr_search::num_attr_search(const char *name, char *descr)
		: img_search(name, descr)
{
	edit_window = NULL;
	num_attr = NULL;
	num_attr_tail = NULL;
	return;
}

num_attr_search::~num_attr_search()
{
	while(num_attr != NULL) {
		num_attr_node * temp = num_attr;
		num_attr = num_attr->next_num_attr;
		if (temp->attr_name) {
			free(temp->attr_name);
		}
		free(temp);
	}
	return;
}


int
num_attr_search::handle_config(int nconf, char **data)
{
	int	err;

	/* we are assuming ATTR_NAME_ID comes first for a node */
	if (strcmp(ATTR_NAME_ID, data[0]) == 0) {
		assert(nconf > 1);
		if (num_attr == NULL || num_attr_tail->attr_name != NULL) {
			add_num_attr_node();
		}
		num_attr_tail->attr_name = strdup(data[1]);
		assert(num_attr_tail->attr_name != NULL);
		err = 0;
	} else if (strcmp(MIN_VALUE_ID, data[0]) == 0) {
		assert(nconf > 1);
		num_attr_tail->min_value = atof(data[1]);
		err = 0;
	} else if (strcmp(MAX_VALUE_ID, data[0]) == 0) {
		assert(nconf > 1);
		num_attr_tail->max_value = atof(data[1]);
		err = 0;
	} else if (strcmp(DROP_MISSING_ID, data[0]) == 0) {
		assert(nconf > 1);
		num_attr_tail->drop_missing = atoi(data[1]);
		err = 0;
	} else {
                err = img_search::handle_config(nconf, data);
        }

        return(err);
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
num_attr_search::add_num_attr_node_to_window(num_attr_node * temp)
{
	GtkWidget *	widget;
	GtkWidget *     table;
   	GtkAdjustment *	spin_adj;

	if (temp == NULL) {
		temp = num_attr_tail;
	}

        table = gtk_table_new(4, 2, FALSE);
        gtk_table_set_row_spacings(GTK_TABLE(table), 2);
        gtk_table_set_col_spacings(GTK_TABLE(table), 4);
        gtk_container_set_border_width(GTK_CONTAINER(table), 10);
        gtk_box_pack_start(GTK_BOX(box), table, FALSE, TRUE, 0);


	/* set the first row label and text entry for the attribute name */
	widget = gtk_label_new("Attribute Name");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 0, 1);
	temp->attr_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(table), temp->attr_entry, 1, 2, 0, 1);
	if (temp->attr_name != NULL) {
		gtk_entry_set_text(GTK_ENTRY(temp->attr_entry), temp->attr_name);
	}

	/* all label and widget for min value */
	widget = gtk_label_new("Min value");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 1, 2);
   	spin_adj = (GtkAdjustment *)gtk_adjustment_new(temp->min_value, 
		NATTR_MIN_VALUE, NATTR_MAX_VALUE, NATTR_STEP, NATTR_PAGE,
		NATTR_PAGE_SIZE);	
   	temp->min_spinner = gtk_spin_button_new(spin_adj, 1.000, 3);
	gtk_table_attach_defaults(GTK_TABLE(table), temp->min_spinner, 1, 2, 1, 2);

	/* all label and widget for max value */
	widget = gtk_label_new("Max value");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 2, 3);
   	spin_adj = (GtkAdjustment *)gtk_adjustment_new(temp->max_value, 
		NATTR_MIN_VALUE, NATTR_MAX_VALUE, NATTR_STEP, NATTR_PAGE,
		NATTR_PAGE_SIZE);	
   	temp->max_spinner = gtk_spin_button_new(spin_adj, 1.000, 3);
	gtk_table_attach_defaults(GTK_TABLE(table), temp->max_spinner, 1, 2, 2, 3);

	/* add label and checkbox to pass/drop objet without named attribute */
	widget = gtk_label_new("Drop missing");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 3, 4);
	temp->dropcb = gtk_check_button_new();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(temp->dropcb), temp->drop_missing);
	gtk_table_attach_defaults(GTK_TABLE(table), temp->dropcb, 1, 2, 3, 4);

	gtk_widget_show_all(edit_window);
}

static void
cb_add_button(GtkButton *item, gpointer data)
{
	num_attr_search *	search;
	search = (num_attr_search *) data;
	search->add_num_attr_node();
	search->add_num_attr_node_to_window(NULL);
}

void
num_attr_search::edit_search()
{
	GtkWidget *     widget;
	GtkWidget *     hbox;
	char        	name[MAX_DISPLAY_NAME];
	num_attr_node * temp;

	
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

	/* Put an "add" button */
	widget = gtk_button_new_with_label("Add");
	g_signal_connect(G_OBJECT(widget), "clicked",
	                 G_CALLBACK(cb_add_button), this);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);

 

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
         * To make the layout look a little cleaner we use a table
         * to place all the fields.  This will make them be nicely
         * aligned.
         */
	temp = num_attr;
	while (temp != NULL) {
		add_num_attr_node_to_window(temp);
		temp = temp->next_num_attr;
	}
	
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
	num_attr_node * temp;

	if (edit_window == NULL) {
		return;
	}

	temp = num_attr;
	while (temp != NULL) {
		if (temp->attr_name!= NULL) {
			free(temp->attr_name);
		}
		temp->attr_name = strdup(gtk_entry_get_text(GTK_ENTRY(temp->attr_entry)));
		assert(temp->attr_name != NULL);

		temp->min_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(temp->min_spinner));
		temp->max_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(temp->max_spinner));
		temp->drop_missing = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(temp->dropcb));
		temp = temp->next_num_attr;
	}
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
	num_attr_node * temp;

	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER  %s  # dependancies \n", get_name());
	fprintf(ostream, "THRESHOLD  1  # boolean \n");
	fprintf(ostream, "MERIT  10000  	# guess at cost \n");
	fprintf(ostream, "EVAL_FUNCTION  f_eval_num_attr  # eval function \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_num_attr  # init function \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_num_attr  # fini function \n");

	
	temp = num_attr;
	while (temp != NULL) {
		if (temp->attr_name != NULL) {
			fprintf(ostream, "ARG  %s  # attribute to search \n", temp->attr_name);
			fprintf(ostream, "ARG  %f  # min value \n", temp->min_value);
			fprintf(ostream, "ARG  %f  # min value \n", temp->max_value);
			fprintf(ostream, "ARG  %d  # drop missing  \n", temp->drop_missing);
		}
		temp = temp->next_num_attr;
	}
	fprintf(ostream, "\n");
	fprintf(ostream, "\n");
}

void
num_attr_search::write_config(FILE *ostream, const char *dirname)
{

	num_attr_node * temp;
	fprintf(ostream, "SEARCH %s %s\n", SEARCH_NAME, get_name());

	temp = num_attr;
	while (temp != NULL) {
		fprintf(ostream, "%s %s\n", ATTR_NAME_ID, temp->attr_name);
		fprintf(ostream, "%s %f \n", MIN_VALUE_ID, temp->min_value);
		fprintf(ostream, "%s %f \n", MAX_VALUE_ID, temp->max_value);
		fprintf(ostream, "%s %d \n", DROP_MISSING_ID, temp->drop_missing);
		temp = temp->next_num_attr;
	}
}

void
num_attr_search::region_match(RGBImage *img, bbox_list_t *blist)
{
	/* XXX do something useful -:) */
	return;
}

bool
num_attr_search::is_editable(void)
{
	return true;
}
