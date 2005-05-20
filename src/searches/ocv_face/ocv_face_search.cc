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
#include <pthread.h>
#include <gtk/gtk.h>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include "queue.h"
#include "rgb.h"
#include "common_consts.h"
#include "image_tools.h"
#include "img_search.h"
#include "facedet.h"
#include "face_tools.h"
#include "fil_data2ii.h"
#include "search_set.h"
#include "read_config.h"

#define	MAX_DISPLAY_NAME	64

/* config tokens  */
#define	NUMFACE_ID	"NUMFACE"
#define	SUPPORT_ID	"SUPPORT"

#define	SEARCH_NAME	"ocv_face_search"

void
ocv_face_init()
{
        ocv_face_factory *fac;

        fac = new ocv_face_factory;
        read_config_register(SEARCH_NAME, fac);
}


ocv_face_search::ocv_face_search(const char *name, char *descr)
		: window_search(name, descr)
{
	face_count = 1;
	support_matches = 2;

	edit_window = NULL;
	count_widget = NULL;
	support_widget = NULL;

	set_scale(1.20);
	set_stride(1);
	set_testx(24);
	set_testy(24);
}

ocv_face_search::~ocv_face_search()
{
	return;
}


void
ocv_face_search::set_face_count(char *data)
{
	int		new_count = atoi(data);

	set_face_count(new_count);
	return;
}

void
ocv_face_search::set_face_count(int new_count)
{
	if (new_count < 0) {
		new_count = 0;
	}

	face_count = new_count;
	return;
}

void
ocv_face_search::set_support(char *data)
{
	int		new_count = atoi(data);

	set_support(new_count);
	return;
}

void
ocv_face_search::set_support(int new_count)
{
	if (new_count < 0) {
		new_count = 0;
	}

	support_matches = new_count;
	return;
}



#define	NUMFACE_ID	"NUMFACE"
#define	SUPPORT_ID	"SUPPORT"

int
ocv_face_search::handle_config(int nconf, char **data)
{
	int	err;
	if (strcmp(NUMFACE_ID, data[0]) == 0) {
		assert(nconf > 1);
		set_face_count(data[1]);
		err = 0;
	} else if (strcmp(SUPPORT_ID, data[0]) == 0) {
		assert(nconf > 1);
		set_support(data[1]);
		err = 0;
	} else {
		err = window_search::handle_config(nconf, data);
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

static void
cb_close_edit_window(GtkWidget* item, gpointer data)
{
	texture_search *	search;

	search = (texture_search *)data;
	search->close_edit_win();
}


void
ocv_face_search::close_edit_win()
{

	/* save any changes from the edit windows */
	save_edits();

	/* call the parent class to give them change to cleanup */
	window_search::close_edit_win();

	edit_window = NULL;

}

static void
edit_search_done_cb(GtkButton *item, gpointer data)
{
	GtkWidget * widget = (GtkWidget *)data;
	gtk_widget_destroy(widget);
}


void
ocv_face_search::edit_search()
{
	GtkWidget * 	widget;
	GtkWidget * 	box;
	GtkWidget * 	frame;
	GtkWidget * 	hbox;
	GtkWidget * 	container;
	char		name[MAX_DISPLAY_NAME];

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
	//gtk_window_set_default_size(GTK_WINDOW(edit_window), 750, 350);
	g_signal_connect(G_OBJECT(edit_window), "destroy",
	                 G_CALLBACK(cb_close_edit_window), this);
	box = gtk_vbox_new(FALSE, 10);


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
	 	 * Create the texture parameters.
	 */

	frame = gtk_frame_new("OpenCV Face Search");
	container = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), container);

	widget = create_slider_entry("Number of Faces", 0.0, 20.0, 0,
	                             face_count, 1.0, &count_widget);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

	widget = create_slider_entry("Num Supporting", 0.0, 20.0, 0,
	                             support_matches, 1.0, &support_widget);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);


	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);

	gtk_widget_show(container);

	/*
	 * Get the controls from the window search class.
		 */
	widget = get_window_cntrl();
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);


	gtk_container_add(GTK_CONTAINER(edit_window), box);

	//gtk_window_set_default_size(GTK_WINDOW(edit_window), 400, 500);
	gtk_widget_show_all(edit_window);

}

/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
ocv_face_search::save_edits()
{
	int		val;

	/* no active edit window, so return */
	if (edit_window == NULL) {
		return;
	}

	val = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(count_widget));
	set_face_count(val);

	val = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(support_widget));
	/* XXX use accessor method ?? */
	support_matches = val;

	/* call the parent class */
	window_search::save_edits();
}

/*
 * This write the relevant section of the filter specification file
 * for this search.
 */

void
ocv_face_search::write_fspec(FILE *ostream)
{
	img_search *	ss;
	save_edits();
	/*
		 * First we write the header section that corrspons
		 * to the filter, the filter name, the assocaited functions.
		 */

	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER %s \n", get_name());
	fprintf(ostream, "THRESHOLD %d \n", face_count);

	fprintf(ostream, "EVAL_FUNCTION  f_eval_opencv_fdetect \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_opencv_fdetect \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_opencv_fdetect \n");

	fprintf(ostream, "ARG  %s  # name \n", get_name());

	/*
	 * Next we write call the parent to write out the releated args,
	 * not that since the args are passed as a vector of strings
	 * we need keep the order the args are written constant or silly
	 * things will happen.
	 */
	window_search::write_fspec(ostream);


	fprintf(ostream, "ARG  %d  # support  \n", support_matches);

	/* XXX can we use the integrate somehow in opencv ?? */
	fprintf(ostream, "REQUIRES  RGB  # dependancies \n");
	fprintf(ostream, "MERIT  10  # some relative cost \n");
	fprintf(ostream, "\n");

	ss = new rgb_img("RGB image", "RGB image");
	(this->get_parent())->add_dep(ss);
}


void
ocv_face_search::write_config(FILE *ostream, const char *dirname)
{
	save_edits();

	/* create the search configuration */
	fprintf(ostream, "\n\n");
	fprintf(ostream, "SEARCH %s %s\n", SEARCH_NAME, get_name());
	fprintf(ostream, "%s %d \n", NUMFACE_ID, face_count);
	fprintf(ostream, "%s %d \n", SUPPORT_ID, support_matches);
	window_search::write_config(ostream, dirname);
	return;
}


void
ocv_face_search::region_match(RGBImage *img, bbox_list_t *blist)
{
	opencv_fdetect_t fconfig;
	CvHaarClassifierCascade *cascade;
	int			pass;

	save_edits();


	fconfig.name = strdup(get_name());
	assert(fconfig.name != NULL);

	fconfig.scale_mult = get_scale();
	fconfig.xsize = get_testx();
	fconfig.ysize = get_testy();
	fconfig.stride = get_stride();
	fconfig.support = support_matches;

	cascade = cvLoadHaarClassifierCascade("<default_face_cascade>",
				      cvSize(fconfig.xsize, fconfig.ysize));
	/* XXX check args */
	fconfig.haar_cascade = cvCreateHidHaarClassifierCascade(
	                           cascade, 0, 0, 0, 1);
	cvReleaseHaarClassifierCascade(&cascade);

	pass = opencv_face_scan(img, blist, &fconfig);

	/* cleanup */
	cvReleaseHidHaarClassifierCascade(&fconfig.haar_cascade);
	free(fconfig.name);

	return;
}


