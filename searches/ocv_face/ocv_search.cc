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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
#include <sys/stat.h>

#include <cv.h>
#include <cvaux.h>

#include <sys/queue.h>
#include "rgb.h"
#include "lib_results.h"
#include "snapfind_consts.h"
#include "img_search.h"
#include "ocv_search.h"
#include "opencv_face_tools.h"
#include "search_set.h"
#include "factory.h"
#include "snapfind_config.h"

#define	MAX_DISPLAY_NAME	64

/* config tokens  */
#define	NUMFACE_ID	"NUMFACE"
#define	SUPPORT_ID	"SUPPORT"

ocv_search::ocv_search(const char *name, const char *descr)
		: window_search(name, descr)
{
	set_count(1);
	set_support(2);

	edit_window = NULL;
	count_widget = NULL;
	support_widget = NULL;
}


ocv_search::~ocv_search()
{
        free((char *) get_auxiliary_data());
	free(cascade_file_name);
	return;
}

int 
ocv_search::get_count() 
{
	return(count);
}

void
ocv_search::set_count(char *data)
{
	int		new_count = atoi(data);

	set_count(new_count);
	return;
}

void
ocv_search::set_count(int new_count)
{
	if (new_count < 0) {
		new_count = 0;
	}

	count = new_count;
	return;
}

int 
ocv_search::get_support() 
{
	return(support_matches);
}


void
ocv_search::set_support(char *data)
{
	int		new_count = atoi(data);

	set_support(new_count);
	return;
}

void
ocv_search::set_support(int new_count)
{
	if (new_count < 0) {
		new_count = 0;
	}

	support_matches = new_count;
	return;
}


void 
ocv_search::set_classifier(const char *name) 
{
        int fd;
	int rc;
        char *cascade_bytes;
	struct stat     stats;
	ssize_t          nbytes;

        cascade_file_name = (char *) malloc(SF_MAX_PATH);
	strcpy(cascade_file_name, sfconf_get_plugin_dir());
        strcat(cascade_file_name, "/");
	strcat(cascade_file_name, name);
	strcat(cascade_file_name, ".xml");

	fd = open(cascade_file_name, O_RDONLY);
        assert(fd >=0);

	rc = fstat(fd, &stats);

	if (rc < 0) {
	  close(fd);
          assert(0);
	}

	cascade_bytes = (char *) malloc(stats.st_size);
	if (cascade_bytes == NULL) {
	  close(fd);
	  assert(0);
	}

	nbytes = read(fd, cascade_bytes, stats.st_size);
	if (nbytes < stats.st_size) {
	  close(fd);
	  assert(0);
	}

	log_message(LOGT_APP, LOGL_TRACE, 
				"auxiliary data: %s file %s buf %x len %d",
				get_name(), cascade_file_name, cascade_bytes, 
				nbytes);

	set_auxiliary_data((void *) cascade_bytes);
	set_auxiliary_data_length(nbytes);

	close(fd);
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
		*adjp = gtk_adjustment_new(min, min, max, step, 0.1, 0);
	} else if (max < 50) {
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
	ocv_search *	search;

	search = (ocv_search *)data;
	search->close_edit_win();
}


void
ocv_search::close_edit_win()
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
ocv_search::edit_search()
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

	frame = gtk_frame_new("OpenCV Search");
	container = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), container);

	widget = create_slider_entry("Number of Objects", 0.0, 20.0, 0,
	                             count, 1.0, &count_widget);
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
ocv_search::save_edits()
{
	int		val;

	/* no active edit window, so return */
	if (edit_window == NULL) {
		return;
	}

	val = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(count_widget));
	set_count(val);

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
ocv_search::write_fspec(FILE *ostream)
{
	/*
	 * Write the remainder of the header.
	 */
	fprintf(ostream, "THRESHOLD %d \n", count);

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
}


void
ocv_search::region_match(RGBImage *img, bbox_list_t *blist)
{
	opencv_fdetect_t fconfig;
	int			pass;

	save_edits();

	fconfig.name = strdup(get_name());
	assert(fconfig.name != NULL);

	fconfig.scale_mult = get_scale();
	fconfig.xsize = get_testx();
	fconfig.ysize = get_testy();
	fconfig.stride = get_stride();
	fconfig.support = support_matches;

	fconfig.haar_cascade = cvLoadHaarClassifierCascade(
			cascade_file_name, cvSize(fconfig.xsize, fconfig.ysize));

	pass = opencv_face_scan(img, blist, &fconfig);

	cvReleaseHaarClassifierCascade(&fconfig.haar_cascade);
	free(fconfig.name);

	return;
}

int
ocv_search::handle_config(int nconf, char **data)
{
	int	err;
	if (strcmp(NUMFACE_ID, data[0]) == 0) {
		assert(nconf > 1);
		set_count(data[1]);
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
