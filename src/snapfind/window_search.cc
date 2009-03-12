/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <gtk/gtk.h>
#include <sys/queue.h>
#include "rgb.h"
#include "lib_results.h"
#include "img_search.h"

/* tokens for the config file */
#define	BOX_X_ID	"TESTX"
#define	BOX_Y_ID	"TESTY"
#define	STRIDE_ID	"STRIDE"
#define	SCALE_ID	"SCALE"
#define	MATCH_ID	"MATCHES"

window_search::window_search(const char *name, const char *descr)
		: img_search(name, descr)
{
	scale = 1.0;
	testx = 32;
	testy = 32;
	stride = 16;
	num_matches = 1;
	enable_size = 1;
	enable_stride = 1;
	enable_scale = 1;

}

window_search::~window_search()
{
	return;
}

void
window_search::set_stride_control(int val)
{
	enable_stride = val;
}

void
window_search::set_size_control(int val)
{
	enable_size = val;
}

void
window_search::set_scale_control(int val)
{
	enable_scale = val;
}


void
window_search::set_matches(int new_matches)
{
	num_matches = new_matches;
	return;
}

void
window_search::set_matches(char * data)
{
	num_matches = atoi(data);
	return;
}

int
window_search::get_matches()
{
	return(num_matches);
}



void
window_search::set_stride(int new_stride)
{
	stride = new_stride;
	return;
}

void
window_search::set_stride(char *data)
{
	stride = atoi(data);
	return;
}

int
window_search::get_stride()
{
	return(stride);
}


void
window_search::set_scale(double new_scale)
{
	scale = new_scale;
	return;
}

void
window_search::set_scale(char * data)
{
	scale = atof(data);
	return;
}

double
window_search::get_scale()
{
	float	local_scale;
	if (scale == 1.0) {
		local_scale = 100000.00;	/* some large number */
	} else {
		local_scale = scale;
	}
	return(local_scale);
}


void
window_search::set_testx(char *data)
{
	testx = atoi(data);
	return;
}

void
window_search::set_testx(int val)
{
	testx = val;
	return;
}

int
window_search::get_testx()
{
	return(testx);
}



void
window_search::set_testy(char *data)
{
	testy = atoi(data);
	return;
}

void
window_search::set_testy(int val)
{
	testy = val;
	return;
}

int
window_search::get_testy()
{
	return(testy);
}


int
window_search::handle_config(int nconf, char **data)
{
	int		err;

	if (strcmp(data[0], BOX_X_ID) == 0) {
		assert(nconf > 1);
		set_testx(data[1]);
		err = 0;
	} else if (strcmp(data[0], BOX_Y_ID) == 0) {
		assert(nconf > 1);
		set_testy(data[1]);
		err = 0;
	} else if (strcmp(data[0], STRIDE_ID) == 0) {
		assert(nconf > 1);
		set_stride(data[1]);
		err = 0;
	} else if (strcmp(data[0], SCALE_ID) == 0) {
		assert(nconf > 1);
		set_scale(data[1]);
		err = 0;
	} else if (strcmp(data[0], MATCH_ID) == 0) {
		assert(nconf >1);
		set_matches(data[1]);
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

	if (max < 50) {
		max++;
		*adjp = gtk_adjustment_new(min, min, max, step, 1.0, 0);
	} else {
		max+= 10;
		*adjp = gtk_adjustment_new(min, min, max, step, 10.0, 0);
	}
	gtk_adjustment_set_value(GTK_ADJUSTMENT(*adjp), initial);

	scale = gtk_hscale_new(GTK_ADJUSTMENT(*adjp));
	gtk_widget_set_size_request (GTK_WIDGET(scale), 200, -1);
	gtk_range_set_update_policy (GTK_RANGE(scale), GTK_UPDATE_CONTINUOUS);
	gtk_scale_set_draw_value (GTK_SCALE(scale), FALSE);
	gtk_box_pack_start (GTK_BOX(container), scale, TRUE, TRUE, 0);
	gtk_widget_set_size_request(scale, 120, 0);

	button = gtk_spin_button_new(GTK_ADJUSTMENT(*adjp), step, dec);
	gtk_box_pack_start (GTK_BOX(container), button, FALSE, FALSE, 0);

	gtk_widget_show_all(container);
	return(container);
}

void
window_search::close_edit_win()
{

	/* call parent */
	img_search::close_edit_win();

}

GtkWidget *
window_search::get_window_cntrl()
{
	GtkWidget *	widget;
	GtkWidget *	frame;
	GtkWidget *	container;

	frame = gtk_frame_new("Window Search");

	container = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), container);

	if (enable_scale) {
		widget = create_slider_entry("scale", 1.0, 200.0, 2,
		                             scale, 0.25, &scale_adj);
		gtk_box_pack_start(GTK_BOX(container), widget, FALSE, FALSE, 0);
	}

	if (enable_size) {
		widget = create_slider_entry("testx", 1.0, 100.0, 0,
		                             testx, 1.0, &testx_adj);
		gtk_box_pack_start(GTK_BOX(container), widget, FALSE, FALSE, 0);

		widget = create_slider_entry("testy", 1.0, 100.0, 0,
		                             testy, 1.0, &testy_adj);
		gtk_box_pack_start(GTK_BOX(container), widget, FALSE, FALSE, 0);
	}

	if (enable_stride) {
		widget = create_slider_entry("stride", 1.0, 100.0, 0,
		                             stride, 1.0, &stride_adj);
		gtk_box_pack_start(GTK_BOX(container), widget, FALSE, FALSE, 0);
	}

	widget = create_slider_entry("Matches", 1.0, 100.0, 0,
	                             num_matches, 1.0, &match_adj);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, FALSE, 0);


	gtk_widget_show_all(frame);

	return (frame);
}

void
window_search::save_edits()
{
	int     ival;
	double  dval;

	if (enable_scale) {
		/* get the scale value */
		dval = gtk_adjustment_get_value(GTK_ADJUSTMENT(scale_adj));
		set_scale(dval);
	}

	if (enable_size) {
		ival = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(testx_adj));
		set_testx(ival);

		ival = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(testy_adj));
		set_testy(ival);
	}

	if (enable_stride) {
		ival = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(stride_adj));
		set_stride(ival);
	}

	ival = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(match_adj));
	set_matches(ival);

	/* call the parent class */
	img_search::save_edits();
}

/*
 * This method writes out the filter spec arguments for this
 * file.
 */
void
window_search::write_fspec(FILE *ostream)
{

	/* write the related filter spec arguments*/
	if (enable_scale) {
		fprintf(ostream, "ARG  %9.7f  # Scale \n", get_scale());
	}
	if (enable_size) {
		fprintf(ostream, "ARG  %d  # Test X \n", testx);
		fprintf(ostream, "ARG  %d  # Test Y \n", testy);
	}
	if (enable_stride) {
		fprintf(ostream, "ARG  %d  # Stride \n", stride);
	}
	fprintf(ostream, "ARG  %d  # matches \n", num_matches);

}

void
window_search::write_config(FILE *ostream, const char *dirname)
{
	/* write the related filter spec arguments*/

	fprintf(ostream, "%s %d \n", BOX_X_ID, testx);
	fprintf(ostream, "%s %d \n", BOX_Y_ID, testy);
	fprintf(ostream, "%s %d \n", STRIDE_ID, stride);
	fprintf(ostream, "%s %f \n", SCALE_ID, scale);
	fprintf(ostream, "%s %d \n", MATCH_ID, num_matches);
}

