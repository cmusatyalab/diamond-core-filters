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
#include <sys/queue.h>
#include "lib_results.h"
#include "rgb.h"
#include "shingling_search.h"
#include "factory.h"

#define	MAX_DISPLAY_NAME	64

/* config file tokens that we write out */
#define SEARCH_NAME     "shingling"
#define METRIC_ID  	"METRIC"
#define STRING_ID    	"STRING"
#define SHINGLE_SIZE_ID	"SHINGLE_SIZE"


extern "C" {
	diamond_public
	void search_init();
}

/*
 * Initialization function that creates the factory and registers
 * it with the rest of the UI.
 */
void 
search_init()
{
	shingling_factory *fac;
	fac = new shingling_factory;
	factory_register(fac);
}

static unsigned char encode_nibble(unsigned char c)
{
    c &= 0xf;
    if (c > 9)	c += 'a' - 10;
    else	c += '0';
    return c;
}

char *encode_hex(const char *buf)
{
    size_t i, outlen = strlen(buf) * 2;
    char *out = (char *)malloc(outlen+1);

    for (i = 0; i < outlen; i += 2) {
	out[i]   = encode_nibble(buf[i/2] >> 4);
	out[i+1] = encode_nibble(buf[i/2]);
    }
    out[outlen] = '\0';
    return out;
}

static unsigned char decode_nibble(unsigned char c)
{
    if	    (c >= 'a')	c -= 'a' - 10;
    else if (c >= 'A')	c -= 'A' - 10;
    else		c -= '0';
    return c;
}

char *decode_hex(const char *buf)
{
    size_t i, buflen = strlen(buf);
    char *out = (char *)malloc(buflen/2 + 1);

    for (i = 0; i+1 < buflen; i += 2)
	out[i/2] = decode_nibble(buf[i]) << 4 | decode_nibble(buf[i+1]);

    out[buflen/2] = '\0';
    return out;
}

shingling_search::shingling_search(const char *name, const char *descr)
		: img_search(name, descr)
{
	similarity = 0.90;
	search_string = strdup("");
	shingle_size = 4;
	edit_window = NULL;
}

shingling_search::~shingling_search()
{
	free(search_string);
}


int
shingling_search::handle_config(int nconf, char **data)
{
	int err;

	if (strcmp(METRIC_ID, data[0]) == 0) {
		assert(nconf > 1);
		similarity = atof(data[1]);
		if (similarity < 0.0) similarity = 0.0;
		if (similarity > 1.0) similarity = 1.0;
		err = 0;
	} else if (strcmp(SHINGLE_SIZE_ID, data[0]) == 0) {
		assert(nconf > 1);
		shingle_size = atoi(data[1]);
		err = 0;
	} else if (strcmp(STRING_ID, data[0]) == 0) {
		assert(nconf >= 1);
		free(search_string);
		search_string = decode_hex(data[1]);
		assert(search_string != NULL);
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
	shingling_search *    search;
	search = (shingling_search *)data;
	search->close_edit_win();
}


void
shingling_search::edit_search()
{
	GtkWidget *widget;
	GtkWidget *box;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *container;
	GtkTextBuffer *buffer;
	char name[MAX_DISPLAY_NAME];

	/* see if it already exists */
	if (edit_window) {
	    gdk_window_raise(GTK_WIDGET(edit_window)->window);
	    return;
	}

	edit_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(name, MAX_DISPLAY_NAME-1, "Edit %s", get_name());
	name[MAX_DISPLAY_NAME-1] = '\0';
	gtk_window_set_title(GTK_WINDOW(edit_window), name);
	g_signal_connect(G_OBJECT(edit_window), "destroy",
	                 G_CALLBACK(cb_close_edit_window), this);

	box = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(box), 5);
	gtk_container_add(GTK_CONTAINER(edit_window), box);

	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, TRUE, 0);

	widget = gtk_button_new_with_label("Close");
	g_signal_connect(G_OBJECT(widget), "clicked",
	                 G_CALLBACK(cb_edit_done), edit_window);
	GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);

	/* Get the controls from the img_search. */
	widget = img_search_display();
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);

	/* w-shingling parameters */
	frame = gtk_frame_new("Shingling Parameters");
	container = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), container);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);

	/* similarity setting */
	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(container), hbox, FALSE, TRUE, 0);

	widget = gtk_label_new("Min Similarity");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);

	sim_adj =  gtk_adjustment_new(0.0, 0.0, 1.0, 0.05, 0.1, 0);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(sim_adj), similarity);

	widget = gtk_hscale_new(GTK_ADJUSTMENT(sim_adj));
	gtk_widget_set_size_request(GTK_WIDGET(widget), 200, -1);
	gtk_range_set_update_policy(GTK_RANGE(widget), GTK_UPDATE_CONTINUOUS);
	gtk_scale_set_draw_value(GTK_SCALE(widget), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_widget_set_size_request(GTK_WIDGET(widget), 120, 0);

	widget = gtk_spin_button_new(GTK_ADJUSTMENT(sim_adj), 0.05, 2);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);

	/* shingle size */
	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(container), hbox, FALSE, TRUE, 0);

	widget = gtk_label_new("Shingle size");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);

        shingle_size_cb = gtk_spin_button_new_with_range(1.0, INT_MAX, 1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(shingle_size_cb), shingle_size);
	gtk_box_pack_start(GTK_BOX(hbox), shingle_size_cb, FALSE, TRUE, 0);

	/* text entry for the search string */
	frame = gtk_frame_new("Search Fragment");
	container = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), container);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);

	string_entry = gtk_text_view_new();

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(string_entry));
	gtk_text_buffer_set_text(buffer, search_string, -1);

	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), string_entry);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

	/* make everything visible */
	gtk_widget_show_all(edit_window);
}



/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
shingling_search::save_edits()
{
	GtkTextIter start, end;
	GtkTextBuffer *buffer;

	if (!edit_window)
		return;

	if (search_string)
		free(search_string);

	similarity = gtk_adjustment_get_value(GTK_ADJUSTMENT(sim_adj));

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(string_entry));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	search_string = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

	search_string = strdup(search_string);
	assert(search_string != NULL);
	
	shingle_size =
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(shingle_size_cb));
}


void
shingling_search::close_edit_win()
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
shingling_search::write_fspec(FILE *ostream)
{
	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER %s  # dependancies \n", get_name());
	fprintf(ostream, "THRESHOLD %d  # similarity \n",
		(int)(100.0 * similarity));
	fprintf(ostream, "MERIT 1000  	# guess at cost \n");
	fprintf(ostream, "EVAL_FUNCTION  f_eval_shingling  # eval function \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_shingling  # init function \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_shingling  # fini function \n");
	fprintf(ostream, "ARG  %d  # shingle width  \n", shingle_size );
	fprintf(ostream, "\n");
	fprintf(ostream, "\n");

	set_auxiliary_data(search_string);
	set_auxiliary_data_length(strlen(search_string));
}

void
shingling_search::write_config(FILE *ostream, const char *dirname)
{
	char *fragment = encode_hex(search_string);

 	fprintf(ostream, "SEARCH %s %s\n", SEARCH_NAME, get_name());
 	fprintf(ostream, "%s %f \n", METRIC_ID, similarity);
 	fprintf(ostream, "%s %d \n", SHINGLE_SIZE_ID, shingle_size);
 	fprintf(ostream, "%s %s \n", STRING_ID, fragment);

	free(fragment);
}

/* Region match isn't meaninful for this search */
void
shingling_search::region_match(RGBImage *img, bbox_list_t *blist)
{
	return;
}

bool
shingling_search::is_editable(void)
{
	return true;
}
