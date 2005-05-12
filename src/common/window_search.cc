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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <gtk/gtk.h>
#include "queue.h"
#include "rgb.h"
#include "img_search.h"

window_search::window_search(const char *name, char *descr)
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
	/* XXX example search destruct */
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
	/* XXX any bounds checks */
	num_matches = new_matches;
	return;
}

void
window_search::set_matches(char * data)
{
	/* XXX any bounds checks */
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
	/* XXX any bounds checks */
	stride = new_stride;
	return;
}

void
window_search::set_stride(char *data)
{
	/* XXX any bounds checks */
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
	/* XXX any bounds checks */
	scale = new_scale;
	return;
}

void
window_search::set_scale(char * data)
{
	/* XXX any bounds checks */
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
	/* XXX any bounds checks */
	testx = atoi(data);
	return;
}

void
window_search::set_testx(int val)
{
	/* XXX any bounds checks */
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
	/* XXX any bounds checks */
	testy = atoi(data);
	return;
}

void
window_search::set_testy(int val)
{
	/* XXX any bounds checks */
	testy = val;
	return;
}

int
window_search::get_testy()
{
	return(testy);
}

int
window_search::handle_config(config_types_t conf_type, char *data)
{
	/* XXX example search destruct */
	int		err;

	switch (conf_type) {
		case TESTX_TOK:
			set_testx(data);
			err = 0;
			break;

		case TESTY_TOK:
			set_testy(data);
			err = 0;
			break;

		case STRIDE_TOK:
			set_stride(data);
			err = 0;
			break;

		case SCALE_TOK:
			set_scale(data);
			err = 0;
			break;

		case MATCHES_TOK:
			set_matches(data);
			err = 0;
			break;

		default:
			err = window_search::handle_config(conf_type, data);
			assert(err == 0);
			break;
	}
	return(err);
}

static GtkWidget *
create_slider_entry(char *name, float min, float max, int dec, float initial,
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
	gtk_box_pack_start (GTK_BOX(container), button, FALSE, FALSE, 0);

	gtk_widget_show(container);
	gtk_widget_show(label);
	gtk_widget_show(scale);
	gtk_widget_show(button);

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


	gtk_widget_show(container);
	gtk_widget_show(frame);
	//gtk_widget_show(box);

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

	/* XXX look at cleanup issues  */

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

	fprintf(ostream, "TESTX %d \n", testx);
	fprintf(ostream, "TESTY %d \n", testy);
	fprintf(ostream, "STRIDE %d \n", stride);
	fprintf(ostream, "SCALE %f \n", scale);
	fprintf(ostream, "MATCHES %d \n", num_matches);
}

