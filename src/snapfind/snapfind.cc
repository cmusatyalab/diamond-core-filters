/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  Copyright (c) 2007, 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>		/* dirname */
#include <assert.h>
#include <stdint.h>
#include <signal.h>
#include <dlfcn.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <glib.h>

#include "lib_filter.h"
#include "lib_log.h"
#include "sys_attr.h"
#include "snapfind_consts.h"
#include "searchlet_api.h"
#include "gui_thread.h"

#include "lib_results.h"
#include "lib_sfimage.h"
#include <sys/queue.h>
#include "sf_consts.h"
#include "attr_info.h"

#include "face_search.h"
#include "lib_results.h"
#include "rgb.h"
#include "face_widgets.h"
#include "img_search.h"
#include "sfind_search.h"
#include "sfind_tools.h"
#include "snap_popup.h"
#include "search_support.h"
#include "snapfind.h"
#include "import_sample.h"
#include "gtk_image_tools.h"
#include "search_set.h"
#include "read_config.h"
#include "plugin.h"
#include "attr_decode.h"
#include "decoder.h"
#include "plugin-runner.h"
#include "readme.h"

/* number of thumbnails to show */
static const int TABLE_COLS = 3;
static const int TABLE_ROWS = 2;

static bool show_user_measurement = true;

thumblist_t thumbnails = TAILQ_HEAD_INITIALIZER(thumbnails);
thumbnail_t *cur_thumbnail = NULL;

GtkTooltips *tooltips = NULL;

user_state_t user_state = USER_WAITING;

/* XXXX fix this */
GtkWidget *config_table;

typedef struct export_threshold_t
{
	char *name;
	double distance;
	int index;			/* index into scapes[] */
	TAILQ_ENTRY(export_threshold_t) link;
}
export_threshold_t;


static struct
{
	GtkWidget *main_window;
	GtkWidget *min_faces;
	GtkWidget *face_levels;
	GtkWidget *scope_button;
	GtkWidget *start_button;
	GtkWidget *stop_button;
	GtkWidget *search_box;
	GtkWidget *search_widget;
	GtkWidget *attribute_cb, *attribute_entry;
	GtkWidget *scapes_tables[2];
}
gui;



/* the search entries for this search */
search_set *	snap_searchset;

/*
 * state required to support popup window to show fullsize img
 */

pop_win_t	 popup_window = {NULL, NULL, NULL};

/* some stats for user study */
user_stats_t user_measurement = { 0, 0 };


typedef enum {
    CNTRL_ELEV,
    CNTRL_WAIT,
    CNTRL_NEXT,
} control_ops_t;

typedef	 struct
{
	GtkWidget *	parent_box;
	GtkWidget *	control_box;
	GtkWidget *	next_button;
	GtkWidget *	zbutton;
	control_ops_t 	cur_op;
	int	 	zlevel;
}
image_control_t;

typedef struct
{
	GtkWidget *     qsize_label; /* no real need to save this */
	GtkWidget *     tobjs_label; /* Total objs being searche */
	GtkWidget *     sobjs_label; /* Total objects searched */
	GtkWidget *     dobjs_label; /* Total objects dropped */
}
image_info_t;


/* XXX */
static image_control_t		image_controls;
static image_info_t		image_information;


/*
 * some globals that we need to find a place for
 */
GAsyncQueue *	to_search_thread;
GAsyncQueue *	from_search_thread;
int		pend_obj_cnt = 0;
int		tobj_cnt = 0;
int		sobj_cnt = 0;
int		dobj_cnt = 0;

static pthread_t	display_thread_info;
static int		display_thread_running = 0;

/*
 * Display the cond variables.
 */
static pthread_cond_t	display_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t	display_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t	thumb_mutex = PTHREAD_MUTEX_INITIALIZER;



/*
 * some prototypes 
 */

void draw_patches(RGBImage *img, int scale, RGBPixel color, RGBPixel mask,
    img_patches_t *ipatches);

static GtkWidget *make_gimage(RGBImage *img);


/* ********************************************************************** */
/* utility functions */
/* ********************************************************************** */

static void free_rgbimage(guchar *pixels, gpointer data) { free(data); }

/*
 * make a gtk image from an img
 */
static GtkWidget *
make_gimage(RGBImage *img)
{
	GdkPixbuf *pbuf; // *scaled_pbuf;

	GUI_THREAD_CHECK();

	pbuf = gdk_pixbuf_new_from_data((const guchar *)&img->data[0],
					GDK_COLORSPACE_RGB, 1, 8,
					img->columns, img->rows,
					(img->columns*sizeof(RGBPixel)),
					free_rgbimage, img);
	if (pbuf == NULL) {
		printf("failed to allocate pbuf\n");
		exit(1);
	}


	GtkWidget *image;
	image = gtk_image_new_from_pixbuf(pbuf);
	assert(image);

	return image;
}




/*
 * This disables all the buttons in the image control section
 * of the display.  This will be called when there is no active image
 * to manipulate.
 */
void
disable_image_control(image_control_t *img_cntrl)
{

	GUI_THREAD_CHECK();
	gtk_widget_set_sensitive(img_cntrl->next_button, FALSE);
}


/*
 * This enables all the buttons in the image control section
 * of the display.  This will be called when there is a new image to 
 * manipulated.
 */
static void
enable_image_control(image_control_t *img_cntrl)
{

	GUI_THREAD_CHECK();
	gtk_widget_set_sensitive(img_cntrl->next_button, TRUE);
	gtk_widget_grab_default(img_cntrl->next_button);

}

static void
do_img_mark(GtkWidget *widget)
{
	thumbnail_t *thumb;

	thumb= (thumbnail_t *)gtk_object_get_user_data(GTK_OBJECT(widget));

	thumb->marked ^= 1;

	/* adjust count */
	user_measurement.total_marked += (thumb->marked ? 1 : -1);

	if (show_user_measurement) {
		printf("marked count = %d/%d\n", user_measurement.total_marked,
		       user_measurement.total_seen);
	}

	gtk_frame_set_label(GTK_FRAME(thumb->frame),
	                    (thumb->marked) ? "marked" : "");
}

static void
cb_img_popup(GtkWidget *widget, GdkEventButton *event, gpointer data)
{

	GUI_CALLBACK_ENTER();

	/* dispatch based on the button pressed */
	switch(event->button) {
		case 1:
			do_img_popup(widget, snap_searchset);
			break;
		case 3:
			do_img_mark(widget);
		default:
			break;
	}

	GUI_CALLBACK_LEAVE();
}



static int
timeout_write_qsize(gpointer label)
{
	char	txt[BUFSIZ];
	int     qsize;

	qsize = pend_obj_cnt;
	sprintf(txt, "Pending images = %-6d", qsize);

	GUI_THREAD_ENTER();
	gtk_label_set_text(GTK_LABEL(label), txt);
	GUI_THREAD_LEAVE();

	return TRUE;
}

static int
timeout_write_tobjs(gpointer label)
{
	char	txt[BUFSIZ];
	int     tobjs;

	tobjs = tobj_cnt;
	sprintf(txt, "Total Objs = %-6d ", tobjs);

	GUI_THREAD_ENTER();
	gtk_label_set_text(GTK_LABEL(label), txt);
	GUI_THREAD_LEAVE();
	return TRUE;
}

static int
timeout_write_sobjs(gpointer label)
{
	char	txt[BUFSIZ];
	int     sobjs;

	sobjs = sobj_cnt;
	sprintf(txt, "Searched Objs = %-6d ", sobjs);

	GUI_THREAD_ENTER();
	gtk_label_set_text(GTK_LABEL(label), txt);
	GUI_THREAD_LEAVE();
	return TRUE;
}

static int
timeout_write_dobjs(gpointer label)
{
	char	txt[BUFSIZ];
	int     dobjs;

	dobjs = dobj_cnt;
	sprintf(txt, "Dropped Objs = %-6d ", dobjs);

	GUI_THREAD_ENTER();
	gtk_label_set_text(GTK_LABEL(label), txt);
	GUI_THREAD_LEAVE();
	return TRUE;
}

static void
user_busy()
{
	message_t *		message;

	message = (message_t *)malloc(sizeof(*message));
	if (message == NULL) {
		printf("failed to allocate message \n");
		exit(1);
	}

	message->type = SET_USER_BUSY;

	g_async_queue_push(to_search_thread, message);
}


static void
user_waiting()
{
	message_t *		message;

	message = (message_t *)malloc(sizeof(*message));
	if (message == NULL) {
		printf("failed to allocate message \n");
		exit(1);
	}

	message->type = SET_USER_WAITING;

	g_async_queue_push(to_search_thread, message);
}


/* ********************************************************************** */

static void
display_thumbnail(ls_obj_handle_t ohandle)
{
	RGBImage	*scaledimg;
	size_t		len;
	int		err;
	search_name_t *	cur;
	int32_t		width, height;
	double		scale;
	unsigned char  *jpeg_data;
	size_t		jpeg_len;

	while(image_controls.cur_op == CNTRL_WAIT) {
		fprintf(stderr, "GOT WAIT. waiting...\n");
		pthread_mutex_lock(&display_mutex);
		if(image_controls.cur_op == CNTRL_WAIT) {
			pthread_cond_wait(&display_cond, &display_mutex);
		}
		pthread_mutex_unlock(&display_mutex);
	}

	assert(image_controls.cur_op == CNTRL_NEXT ||
	       image_controls.cur_op == CNTRL_ELEV);

	/* read thumbnail image data */
	assert(!lf_ref_attr(ohandle, THUMBNAIL_ATTR, &jpeg_len, &jpeg_data));
	scaledimg = read_rgb_image(jpeg_data, jpeg_len);
	assert(scaledimg);

	/* find out the set of results to highlight */

	/* calculate the scale factor */
	len = sizeof(int32_t);
	err = lf_read_attr(ohandle, COLS, &len, (unsigned char *)&width);
	assert(!err);

	len = sizeof(int32_t);
	err = lf_read_attr(ohandle, ROWS, &len, (unsigned char *)&height);
	assert(!err);

	scale = 1.0;
	scale = max(scale, (double)width / (double)scaledimg->width);
	scale = max(scale, (double)height / (double)scaledimg->height);
	scale = round(scale);

	/* 
	 * for each of the active searches look for a set of
	 * result boxes with a well known name.
	 */
	for (cur = active_searches; cur != NULL; cur = cur->sn_next) {
	    img_patches_t *ipatch = get_patches(ohandle, cur->sn_name);
	    if (ipatch)
		draw_patches(scaledimg, (int)scale, green, colorMask, ipatch);
	}
	user_measurement.total_seen++;

	GUI_THREAD_ENTER();

	/*
	 * Build image from the thumbnail image data
	 */
	GtkWidget *image = make_gimage(scaledimg);
	assert(image);

	/*
	 * update the display
	 */

	pthread_mutex_lock(&thumb_mutex);

	if(!cur_thumbnail) {
		cur_thumbnail = TAILQ_FIRST(&thumbnails);
	}
	if (cur_thumbnail->gimage) { /* cleanup */
		gtk_container_remove(GTK_CONTAINER(cur_thumbnail->viewport),
		                     cur_thumbnail->gimage);
		free((void *)cur_thumbnail->img_id);
	}
	gtk_frame_set_label(GTK_FRAME(cur_thumbnail->frame), "");
	cur_thumbnail->marked = 0;
	cur_thumbnail->gimage = image;

	ls_get_objectid(shandle, ohandle, &cur_thumbnail->img_id);
	ls_release_object(shandle, ohandle);

	gtk_container_add(GTK_CONTAINER(cur_thumbnail->viewport), image);
	gtk_widget_show_now(image);


	cur_thumbnail = TAILQ_NEXT(cur_thumbnail, link);

	/* check if panel is full */
	if(cur_thumbnail == NULL) {
		pthread_mutex_unlock(&thumb_mutex);

		enable_image_control(&image_controls);
		log_message(LOGT_APP, LOGL_TRACE, "snapfind: window full");
		if (user_state == USER_WAITING) {
			user_state = USER_BUSY;
			user_busy();
		}
		//fprintf(stderr, "WINDOW FULL. waiting...\n");
		GUI_THREAD_LEAVE();

		/* block until user hits next */
		pthread_mutex_lock(&display_mutex);
		image_controls.cur_op = CNTRL_WAIT;
		pthread_cond_wait(&display_cond, &display_mutex);
		pthread_mutex_unlock(&display_mutex);

		GUI_THREAD_ENTER();
		disable_image_control(&image_controls);
		/*
		 * signal Diamond that the user is waiting
		 * only if there are not enough images buffered 
		 * to fill the next screen.
		 */
		if ((user_state == USER_BUSY) && 
			(g_async_queue_length(from_search_thread) < THUMBNAIL_DISPLAY_SIZE)) {
			user_state = USER_WAITING;
			user_waiting();
		}
		//fprintf(stderr, "WAIT COMPLETE...\n");
		GUI_THREAD_LEAVE();
	} else {
		pthread_mutex_unlock(&thumb_mutex);
		GUI_THREAD_LEAVE();
	}
}


static void
clear_thumbnails()
{
	pthread_mutex_lock(&thumb_mutex);
	TAILQ_FOREACH(cur_thumbnail, &thumbnails, link) {
		if (cur_thumbnail->gimage) { /* cleanup */
			gtk_container_remove(GTK_CONTAINER(cur_thumbnail->viewport),
			                     cur_thumbnail->gimage);
			free((void *)cur_thumbnail->img_id);
			cur_thumbnail->img_id = NULL;
			cur_thumbnail->gimage = NULL;
		}

	}
	cur_thumbnail = NULL;
	pthread_mutex_unlock(&thumb_mutex);
}


static void *
display_thread(void *data)
{
	message_t *message;
	int done = 0;

	while (!done) {
		message = (message_t *)g_async_queue_pop(from_search_thread);
		if (!message) break;

		switch (message->type) {
		case NEXT_OBJECT:
			display_thumbnail((ls_obj_handle_t) message->data);
			break;

		case DONE_OBJECTS:
			done = 1;
			break;

		default:
			break;
		}
		free(message);
	}

	/*
	 * We are done receiving objects. We need to disable the thread image
	 * controls and enable start search button.
	 */
	gtk_widget_set_sensitive(gui.start_button, TRUE);
	gtk_widget_set_sensitive(gui.scope_button, TRUE);
	gtk_widget_set_sensitive(gui.stop_button, FALSE);

	/* we should not be messing with the default. this is here so
	 * that we can trigger a search from the text entry without a
	 * return-pressed handler.  XXX */
	gtk_widget_grab_default (gui.start_button);

	display_thread_running = 0;
	pthread_exit(0);
}

static void
define_scope()
{
	message_t *		message;

	message = (message_t *)malloc(sizeof(*message));
	if (message == NULL) {
		printf("failed to allocate message \n");
		exit(1);
	}

	message->type = DEFINE_SCOPE;
	message->data = NULL;

	g_async_queue_push(to_search_thread, message);
}

static void
stop_search()
{
	message_t *message;

	/* Toggle the start and stop buttons. */
	gtk_widget_set_sensitive(gui.start_button, TRUE);
	gtk_widget_set_sensitive(gui.scope_button, TRUE);
	gtk_widget_set_sensitive(gui.stop_button, FALSE);

	/* we should not be messing with the default. this is here so
	 * that we can trigger a search from the text entry without a
	 * return-pressed handler.  XXX */
	gtk_widget_grab_default (gui.start_button);

	message = (message_t *)malloc(sizeof(*message));
	if (message == NULL) {
		printf("failed to allocate message \n");
		exit(1);
	}

	message->type = TERM_SEARCH;
	message->data = NULL;

	g_async_queue_push(to_search_thread, message);
}

static void
cb_define_scope(GtkButton* item, gpointer data)
{
	GUI_CALLBACK_ENTER();
	define_scope();
	GUI_CALLBACK_LEAVE();
}

static void
cb_stop_search(GtkButton* item, gpointer data)
{
	GUI_CALLBACK_ENTER();
	stop_search();
	GUI_CALLBACK_LEAVE();
}


static void
cb_start_search(GtkButton* item, gpointer data)
{
	message_t *		message;
	int			err;

	GUI_CALLBACK_ENTER();

	/*
	 * Disable the start search button and enable the stop search
	 * button.
	 */
	gtk_widget_set_sensitive(gui.start_button, FALSE);
	gtk_widget_set_sensitive(gui.scope_button, FALSE);
	gtk_widget_set_sensitive(gui.stop_button, TRUE);
	clear_thumbnails();

	/* reset user state */
	user_state = USER_WAITING;

	pthread_mutex_lock(&display_mutex);
	image_controls.cur_op = CNTRL_NEXT;
	pthread_mutex_unlock(&display_mutex);


	/* problem: the signal (below) gets handled before the search
	   thread has a chance to drain the ring. so do it here. */
	drain_ring(from_search_thread);


	/* send the message */
	message = (message_t *)malloc(sizeof(*message));
	if (message == NULL) {
		printf("failed to allocate message \n");
		exit(1);
	}
	message->type = START_SEARCH;
	g_async_queue_push(to_search_thread, message);

	GUI_CALLBACK_LEAVE();	/* need to do this before signal (below) */

	if(!display_thread_running) {
		display_thread_running = 1;
		err = pthread_create(&display_thread_info, NULL, display_thread, NULL);
		if (err) {
			printf("failed to create  display thread \n");
			exit(1);
		}
	} else {
		pthread_mutex_lock(&display_mutex);
		pthread_cond_signal(&display_cond);
		pthread_mutex_unlock(&display_mutex);
	}
}



static void
cb_write_fspec_to_file(GtkWidget *widget, gpointer user_data)
{
	GtkWidget *file_selector = (GtkWidget *)user_data;
	const gchar *selected_filename;
	char buf[BUFSIZ];
	int	len;

	GUI_CALLBACK_ENTER();

	selected_filename =
	    gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector));

	len = snprintf(buf, BUFSIZ, "%s", selected_filename);
	assert(len < BUFSIZ);
	snap_searchset->build_filter_spec(shandle, buf);

	GUI_CALLBACK_LEAVE();
}


static void
cb_save_spec_to_filename()
{
	GtkWidget *file_selector;

	GUI_CALLBACK_ENTER();

	/* Create the selector */
	file_selector = gtk_file_selection_new("Filter spec name");
	gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(file_selector));


	g_signal_connect(GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
	                 "clicked", G_CALLBACK(cb_write_fspec_to_file),
	                 (gpointer)file_selector);

	/*
	  * Ensure that the dialog box is destroyed when the user clicks a button. 
	* Use swapper here to get the right argument to destroy (YUCK).
	  */
	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
	                         "clicked",
	                         G_CALLBACK(gtk_widget_destroy),
	                         (gpointer)file_selector);

	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
	                         "clicked",
	                         G_CALLBACK (gtk_widget_destroy),
	                         (gpointer) file_selector);

	/* Display that dialog */
	gtk_widget_show(file_selector);
	GUI_CALLBACK_LEAVE();
}

static void
write_search_config(const char *dirname, search_set *set
                   )
{
	struct stat	buf;
	int			err;
	FILE *		conf_file;
	char		buffer[256];	/* XXX check */
	img_search *snapobj;
	search_iter_t	iter;


	/* Do some test on the dir */
	/* XXX popup errors */
	err = stat(dirname, &buf);
	if (err != 0) {
		if (errno == ENOENT) {
			err = mkdir(dirname, 0777);
			if (err != 0) {
				perror("Failed to make save directory ");
				assert(0);
			}
		} else {
			perror("open data file: ");
			assert(0);
		}
	} else {
		/* make sure it is a dir */
		if (!S_ISDIR(buf.st_mode)) {
			fprintf(stderr, "%s is not a directory \n", dirname);
			assert(0);
		}
	}

	sprintf(buffer, "%s/%s", dirname, SEARCH_CONFIG_FILE);
	conf_file = fopen(buffer, "w");


	snap_searchset->reset_search_iter(&iter);
	while ((snapobj = snap_searchset->get_next_search(&iter)) != NULL) {
		snapobj->write_config(conf_file, dirname);
	}

	fclose(conf_file);
}


static GtkWidget *search_frame;
static GtkListStore *codec_model;
static GtkWidget *codec_selector;
static GtkWidget *codec_table;

static void
update_search_entry(search_set *cur_set)
{
	gtk_container_remove(GTK_CONTAINER(search_frame), config_table);
	config_table = cur_set->build_edit_table();
	gtk_container_add(GTK_CONTAINER(search_frame), config_table);
	gtk_widget_show_all(search_frame);
}

static img_search *current_codec = NULL;
static GtkWidget *current_codec_edit_button = NULL;

img_search *get_current_codec(void)
{
  return current_codec;
}

static
void codec_changed_cb (GtkComboBox *widget, gpointer user_data)
{
  GtkTreeIter iter;
  img_factory *ifac;
  gchar *name;

  /* get the active item */
  gint active = gtk_combo_box_get_active(widget);
  gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(codec_model),
				&iter, NULL, active);
  gtk_tree_model_get(GTK_TREE_MODEL(codec_model),
		     &iter,
		     1, &ifac,
		     0, &name,
		     -1);

  /* create a new one */
  if (current_codec) {
    gtk_widget_destroy(current_codec_edit_button);
    delete current_codec;
  }
  current_codec = ifac->create("RGB");

  /* get the edit button and plug it in */
  current_codec_edit_button = current_codec->get_edit_widget();
  gtk_table_attach_defaults(GTK_TABLE(codec_table), current_codec_edit_button, 0, 1, 1, 2);
}


static GtkWidget *
create_search_window()
{
	GtkWidget *box2, *box1;
	GtkWidget *separator;

	GtkWidget *codec_frame;
	GtkCellRenderer *renderer;

	GUI_THREAD_CHECK();

	box1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (box1);

	/* Codec selector */
	codec_frame = gtk_frame_new("Codec");
	codec_table = gtk_table_new(2, 1, FALSE);
	gtk_box_pack_start(GTK_BOX(box1), codec_frame, FALSE, FALSE, 10);
	codec_model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	codec_selector = gtk_combo_box_new_with_model(GTK_TREE_MODEL(codec_model));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (codec_selector), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (codec_selector), renderer,
					"text", 0,
					NULL);
	gtk_table_attach_defaults(GTK_TABLE(codec_table), codec_selector, 0, 1, 0, 1);
	gtk_container_add(GTK_CONTAINER(codec_frame), codec_table);

	g_signal_connect(G_OBJECT(codec_selector), "changed",
			 G_CALLBACK(codec_changed_cb), NULL);

	gtk_widget_show_all(codec_frame);



	search_frame = gtk_frame_new("Searches");
	config_table = snap_searchset->build_edit_table();

	gtk_container_add(GTK_CONTAINER(search_frame), config_table);
	gtk_box_pack_start(GTK_BOX(box1), search_frame, FALSE, FALSE, 10);
	gtk_widget_show_all(search_frame);

	/* Add the start and stop buttons */

	box2 = gtk_hbox_new (FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
	gtk_box_pack_end (GTK_BOX (box1), box2, FALSE, FALSE, 0);
	gtk_widget_show (box2);

	gui.start_button = gtk_button_new_with_label ("Start");
	g_signal_connect_swapped (G_OBJECT (gui.start_button), "clicked",
	                          G_CALLBACK(cb_start_search), NULL);
	gtk_box_pack_start (GTK_BOX (box2), gui.start_button, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS (gui.start_button, GTK_CAN_DEFAULT);
	gtk_widget_show (gui.start_button);

	gui.stop_button = gtk_button_new_with_label ("Stop");
	g_signal_connect_swapped (G_OBJECT (gui.stop_button), "clicked",
	                          G_CALLBACK(cb_stop_search), NULL);
	gtk_box_pack_start(GTK_BOX (box2), gui.stop_button, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS (gui.stop_button, GTK_CAN_DEFAULT);
	gtk_widget_set_sensitive(gui.stop_button, FALSE);
	gtk_widget_show (gui.stop_button);

	gui.scope_button = gtk_button_new_with_label ("Define Scope");
	g_signal_connect_swapped (G_OBJECT (gui.scope_button), "clicked",
	                          G_CALLBACK(cb_define_scope), NULL);
	gtk_box_pack_end (GTK_BOX (box1), gui.scope_button, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS (gui.scope_button, GTK_CAN_DEFAULT);
	gtk_widget_show (gui.scope_button);

	separator = gtk_hseparator_new ();
	gtk_box_pack_end (GTK_BOX (box1), separator, FALSE, FALSE, 0);
	gtk_widget_show (separator);

	/* register callback function to get notified when search set changes */
	snap_searchset->register_update_fn(update_search_entry);

	return(box1);
}


static void
cb_import_search_from_dir(GtkWidget *widget, gpointer user_data)
{
	GtkWidget *file_selector = (GtkWidget *)user_data;
	const gchar *dirname;
	char *	olddir;
	int			err;
	char buf[BUFSIZ];

	GUI_CALLBACK_ENTER();

	dirname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector));
	sprintf(buf, "%s/%s", dirname, SEARCH_CONFIG_FILE);

	olddir = getcwd(NULL, 0);

	err = chdir(dirname);
	if (err) {
		show_popup_error("Load search", "Invalid search directory", gui.main_window);
		goto done;
	}

	/* XXXX cleanup all the old searches first */

	read_search_config(buf, snap_searchset);

	err = chdir(olddir);
	assert(err == 0);
	free(olddir);

	stop_search();
	gtk_widget_destroy(gui.search_widget);
	gui.search_widget = create_search_window();
	gtk_box_pack_start (GTK_BOX(gui.search_box), gui.search_widget,
	                    FALSE, FALSE, 10);

done:
	GUI_CALLBACK_LEAVE();
}

static void
cb_load_search_from_dir(GtkWidget *widget, gpointer user_data)
{
	GtkWidget *file_selector = (GtkWidget *)user_data;
	const gchar *dirname;
	char *	olddir;
	int			err;
	char buf[BUFSIZ];

	GUI_CALLBACK_ENTER();

	dirname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector));
	sprintf(buf, "%s/%s", dirname, SEARCH_CONFIG_FILE);

	olddir = getcwd(NULL, 0);

	err = chdir(dirname);
	if (err) {
		show_popup_error("Load search", "Invalid search directory",
		                 gui.main_window);
		goto done;
	}

	/* XXXX cleanup all the old searches first */
	read_search_config(buf, snap_searchset);

	err = chdir(olddir);
	assert(err == 0);
	free(olddir);

	stop_search();
	gtk_widget_destroy(gui.search_widget);
	gui.search_widget = create_search_window();
	gtk_box_pack_start (GTK_BOX(gui.search_box), gui.search_widget,
	                    FALSE, FALSE, 10);

done:
	GUI_CALLBACK_LEAVE();

}

static void
cb_save_search_dir(GtkWidget *widget, gpointer user_data)
{
	GtkWidget *file_selector = (GtkWidget *)user_data;
	const gchar *dirname;
	char buf[BUFSIZ];

	GUI_CALLBACK_ENTER();

	dirname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector));

	sprintf(buf, "%s/%s", dirname, "search_config");

	/* write out the config */
	write_search_config(dirname, snap_searchset);

	gtk_widget_destroy(gui.search_widget);
	gui.search_widget = create_search_window();
	gtk_box_pack_start (GTK_BOX(gui.search_box), gui.search_widget,
	                    FALSE, FALSE, 10);

	GUI_CALLBACK_LEAVE();
}


static void
cb_load_search()
{
	GtkWidget *file_selector;

	GUI_CALLBACK_ENTER();

	/* Create the selector */
	file_selector = gtk_file_selection_new("Dir name for search");
	gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(file_selector));

	g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
	                 "clicked", G_CALLBACK(cb_load_search_from_dir),
	                 (gpointer) file_selector);

	/*
	  * Ensure that the dialog box is destroyed when the user clicks a button. 
	* Use swapper here to get the right argument to destroy (YUCK).
	  */
	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
	                         "clicked",
	                         G_CALLBACK(gtk_widget_destroy),
	                         (gpointer) file_selector);

	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
	                         "clicked",
	                         G_CALLBACK (gtk_widget_destroy),
	                         (gpointer) file_selector);

	/* Display that dialog */
	gtk_widget_show (file_selector);

	GUI_CALLBACK_LEAVE();
}

static void
cb_import_search()
{
	GtkWidget *file_selector;

	GUI_CALLBACK_ENTER();

	/* Create the selector */
	file_selector = gtk_file_selection_new("Dir name for search");
	gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(file_selector));

	g_signal_connect(GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
	                 "clicked", G_CALLBACK(cb_import_search_from_dir),
	                 (gpointer)file_selector);

	/*
	  * Ensure that the dialog box is destroyed when the user clicks a button. 
	* Use swapper here to get the right argument to destroy (YUCK).
	  */
	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
	                         "clicked",
	                         G_CALLBACK(gtk_widget_destroy),
	                         (gpointer)file_selector);

	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
	                         "clicked",
	                         G_CALLBACK (gtk_widget_destroy),
	                         (gpointer) file_selector);

	/* Display that dialog */
	gtk_widget_show(file_selector);
	GUI_CALLBACK_LEAVE();
}

static void
cb_save_search_as()
{
	GtkWidget *file_selector;

	GUI_CALLBACK_ENTER();
	printf("Save search \n");

	/* Create the selector */
	file_selector = gtk_file_selection_new("Save Directory:");
	gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(file_selector));

	g_signal_connect(GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
	                 "clicked", G_CALLBACK(cb_save_search_dir),
	                 (gpointer) file_selector);

	/*
	  * Ensure that the dialog box is destroyed when the user clicks a button. 
	* Use swapper here to get the right argument to destroy (YUCK).
	  */
	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
	                         "clicked",
	                         G_CALLBACK(gtk_widget_destroy),
	                         (gpointer) file_selector);

	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
	                         "clicked",
	                         G_CALLBACK (gtk_widget_destroy),
	                         (gpointer) file_selector);

	/* Display that dialog */
	gtk_widget_show (file_selector);
	GUI_CALLBACK_LEAVE();
}



/* For the check button */
static void
cb_toggle_stats( gpointer   callback_data,
                 guint      callback_action,
                 GtkWidget *menu_item )
{
	GUI_CALLBACK_ENTER();
	toggle_stats_win(shandle, 1);
	GUI_CALLBACK_LEAVE();
}

/* For the check button */
static void
cb_toggle_progress(gpointer callback_data, guint callback_action,
    GtkWidget *menu_item )
{
	GUI_CALLBACK_ENTER();
	toggle_progress_win(shandle, 1);
	GUI_CALLBACK_LEAVE();
}

static void
cb_toggle_log(gpointer callback_data, guint callback_action,
    GtkWidget *menu_item )
{
	GUI_CALLBACK_ENTER();
	//	toggle_log_win(shandle);
	GUI_CALLBACK_LEAVE();
}


static void
cb_toggle_ccontrol(gpointer callback_data, guint callback_action,
    GtkWidget *menu_item )
{
	GUI_CALLBACK_ENTER();
	toggle_ccontrol_win(shandle, 1);
	GUI_CALLBACK_LEAVE();
}



/* ********************************************************************** */
/* widget setup functions */
/* ********************************************************************** */
static void
cb_next_image(GtkButton* item, gpointer data)
{
	log_message(LOGT_APP, LOGL_TRACE, "snapfind: next button pressed");
	
	GUI_CALLBACK_ENTER();
	image_controls.zlevel = gtk_spin_button_get_value_as_int(
	                            GTK_SPIN_BUTTON(image_controls.zbutton));
	clear_thumbnails();
	GUI_CALLBACK_LEAVE(); /* need to put this here instead of at
												 end because signal wakes up another
												 thread immediately... */

	pthread_mutex_lock(&display_mutex);
	image_controls.cur_op = CNTRL_NEXT;
	pthread_cond_signal(&display_cond);
	pthread_mutex_unlock(&display_mutex);

}


static void
create_image_control(GtkWidget *container_box,
                     image_info_t *img_info,
                     image_control_t *img_cntrl)
{
	GtkObject *adj;

	GUI_THREAD_CHECK();

	/*
	 * Now create another region that has the controls
	 * that manipulate the current image being displayed.
	 */

	img_cntrl->parent_box = container_box;
	img_cntrl->control_box = gtk_hbox_new (FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(img_cntrl->control_box), 0);
	gtk_box_pack_start(GTK_BOX(img_cntrl->parent_box),
	                   img_cntrl->control_box, FALSE, FALSE, 0);
	gtk_widget_show(img_cntrl->control_box);

	img_cntrl->next_button = gtk_button_new_with_label ("Next");
	g_signal_connect_swapped(G_OBJECT(img_cntrl->next_button),
	                         "clicked", G_CALLBACK(cb_next_image), NULL);
	gtk_box_pack_end(GTK_BOX(img_cntrl->control_box),
	                 img_cntrl->next_button, FALSE, FALSE, 0);
	GTK_WIDGET_SET_FLAGS (img_cntrl->next_button, GTK_CAN_DEFAULT);
	gtk_widget_show(img_cntrl->next_button);

	img_info->qsize_label = gtk_label_new ("");
	gtk_box_pack_end(GTK_BOX(img_cntrl->control_box),
	                 img_info->qsize_label, FALSE, FALSE, 0);
	gtk_widget_show(img_info->qsize_label);
	gtk_timeout_add(500 /*ms*/, timeout_write_qsize, img_info->qsize_label);

	img_info->tobjs_label = gtk_label_new ("");
	gtk_box_pack_start(GTK_BOX(img_cntrl->control_box),
	                   img_info->tobjs_label, FALSE, FALSE, 0);
	gtk_widget_show(img_info->tobjs_label);
	gtk_timeout_add(500 /*ms*/, timeout_write_tobjs, img_info->tobjs_label);

	img_info->sobjs_label = gtk_label_new ("");
	gtk_box_pack_start(GTK_BOX(img_cntrl->control_box),
	                   img_info->sobjs_label, FALSE, FALSE, 0);
	gtk_widget_show(img_info->sobjs_label);
	gtk_timeout_add(500 /*ms*/, timeout_write_sobjs, img_info->sobjs_label);

	img_info->dobjs_label = gtk_label_new ("");
	gtk_box_pack_start(GTK_BOX(img_cntrl->control_box),
	                   img_info->dobjs_label, FALSE, FALSE, 0);
	gtk_widget_show(img_info->dobjs_label);
	gtk_timeout_add(500 /*ms*/, timeout_write_dobjs, img_info->dobjs_label);

	adj = gtk_adjustment_new(2.0, 1.0, 10.0, 1.0, 1.0, 0);
	img_cntrl->zlevel = 2;
	img_cntrl->zbutton = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_box_pack_start (GTK_BOX(img_cntrl->control_box),img_cntrl->zbutton,
	                    FALSE, FALSE, 0);

	img_cntrl->cur_op = CNTRL_ELEV;
}

/*
 * Create the region that will display the results of the 
 * search.
 */
static void
create_display_region(GtkWidget *main_box)
{

	GtkWidget *	box2;
	GtkWidget *	x;

	GUI_THREAD_CHECK();

	/*
	 * Create a box that holds the following sub-regions.
	 */
	x = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(x), 10);
	gtk_box_pack_start(GTK_BOX(main_box), x, TRUE, TRUE, 0);
	gtk_widget_show(x);

	GtkWidget *result_box;
	result_box = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(result_box), 0);
	gtk_box_pack_start(GTK_BOX(x), result_box, TRUE, TRUE, 0);
	gtk_widget_show(result_box);

	/*
	 * Create the region that will hold the current results
	 * being displayed.
	 */
	box2 = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
	gtk_box_pack_start (GTK_BOX (result_box), box2, TRUE, TRUE, 0);
	gtk_widget_show(box2);


	GtkWidget *thumbnail_view;
	thumbnail_view = gtk_table_new(TABLE_ROWS, TABLE_COLS, TRUE);
	gtk_box_pack_start(GTK_BOX (box2), thumbnail_view, FALSE, TRUE, 0);
	for(int i=0; i< TABLE_ROWS; i++) {
		for(int j=0; j< TABLE_COLS; j++) {
			GtkWidget *widget, *eb;
			GtkWidget *frame;

			eb = gtk_event_box_new();

			frame = gtk_frame_new(NULL);
			gtk_frame_set_label(GTK_FRAME(frame), "");
			gtk_container_add(GTK_CONTAINER(eb), frame);
			gtk_widget_show(frame);

			widget = gtk_viewport_new(NULL, NULL);
			gtk_widget_set_size_request(widget, THUMBSIZE_X, 
			    THUMBSIZE_Y);
			gtk_container_add(GTK_CONTAINER(frame), widget);
			gtk_table_attach_defaults(GTK_TABLE(thumbnail_view), 
			    eb, j, j+1, i, i+1);
			gtk_widget_show(widget);
			gtk_widget_show(eb);

			thumbnail_t *thumb = (thumbnail_t *)malloc(
			    sizeof(thumbnail_t));
			thumb->marked = 0;
			thumb->img_id = NULL;
			thumb->gimage = NULL;
			thumb->viewport = widget;
			thumb->frame = frame;
			TAILQ_INSERT_TAIL(&thumbnails, thumb, link);

			/* the data pointer mechanism seems to be
			 * broken, so use the object user pointer
			 * instead */
			gtk_object_set_user_data(GTK_OBJECT(eb), thumb);
			gtk_signal_connect(GTK_OBJECT(eb),
			                   "button-press-event",
			                   GTK_SIGNAL_FUNC(cb_img_popup),
			                   (gpointer)thumb);
		}
	}
	gtk_widget_show(thumbnail_view);


	/* create the image control area */
	create_image_control(result_box, &image_information, &image_controls);


	disable_image_control(&image_controls);
}


static void
cb_quit()
{
	if (show_user_measurement) {
		printf("MARKED: %d of %d seen\n", user_measurement.total_marked,
		       user_measurement.total_seen);
	}
	gtk_main_quit();
}


static void
cb_import(GtkWidget *widget, gpointer user_data)
{
	open_import_window(snap_searchset);
}


static void
cb_create(gpointer *callback_data, guint callback_action, GtkWidget *widget)
{
	GtkWidget 	*hbox;
	GtkWidget 	*vbox;
	GtkWidget	*dialog;
	GtkWidget	*label;
	GtkWidget	*helplabel;
	GtkWidget	*entry;
	int		result;
	unsigned int	i;
	const char *		new_name;
	img_search *	ssearch;
	img_factory *	factory;

	factory = (img_factory *)callback_data;

	GUI_CALLBACK_ENTER();

	/* get the search name from the user */
	dialog = gtk_dialog_new_with_buttons("Search Name",
	                                     GTK_WINDOW(popup_window.window),
	                                     GTK_DIALOG_DESTROY_WITH_PARENT,
	                                     GTK_STOCK_OK, GTK_RESPONSE_OK,
	                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);

	vbox = gtk_vbox_new(FALSE, 10);
	helplabel = gtk_label_new("Please enter name of search");
	gtk_box_pack_start(GTK_BOX(vbox), helplabel, TRUE, FALSE, 0);


	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 0);

	label = gtk_label_new("Search Name");
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 0);

	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);

redo:
	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if (result == GTK_RESPONSE_CANCEL) {
		gtk_widget_destroy(dialog);
		GUI_CALLBACK_LEAVE();
		return;
	} else {
		/* get the name from the box and do some error checking on it */
		new_name = gtk_entry_get_text(GTK_ENTRY(entry));

		/* if name is null, then redo */
		if (strlen(new_name) == 0) {
			gtk_label_set_text(GTK_LABEL(helplabel),
		   	    "No name: please enter search name");
			goto redo;
		}
		/* Make sure there are not spaces */
		for (i=0; i < strlen(new_name); i++) {
			if (new_name[i] == ' ') {
				gtk_label_set_text(GTK_LABEL(helplabel),
		      		    "Name has spaces: please changes");
				goto redo;
			}
		}

		/* check for name conflicts */
		if (search_exists(new_name, snap_searchset)) {
			gtk_label_set_text(GTK_LABEL(helplabel),
		   	    "Name exists: Please change");
			goto redo;
		}
		ssearch = factory->create(new_name);
		assert(ssearch != NULL);

		/* add to the list of searches */
		snap_searchset->add_search(ssearch);

		/* popup the new search edit box */
		if (ssearch->is_editable()) {
			ssearch->edit_search();
		}
	}
	gtk_widget_destroy(dialog);

	GUI_CALLBACK_LEAVE();

	/* XXX get the name from the user */
}


/* The menu at the top of the main window. */
static GtkItemFactoryEntry menu_items[] = {
	{ "/_File", NULL,  NULL, 0, "<Branch>" },
	{ "/File/Load Search", NULL, G_CALLBACK(cb_load_search), 0, "<Item>" },
	{ "/File/Import Search", NULL, G_CALLBACK(cb_import_search), 
	    0, "<Item>" },
	{ "/File/Save Search as", NULL, G_CALLBACK(cb_save_search_as),
	    0, "<Item>" },
	{ "/File/Save filterspec", NULL, G_CALLBACK(cb_save_spec_to_filename), 
	    0, "<Item>" },
	{ "/File/sep1", NULL, NULL, 0, "<Separator>" },
	{ "/File/_Quit", "<CTRL>Q", (GtkItemFactoryCallback)cb_quit,
	    0, "<StockItem>", GTK_STOCK_QUIT },

	{ "/_Searches", NULL, NULL, 0, "<Branch>"},
	{"/Searches/_New", "<CTRL>N", NULL, 0, "<Branch>"},

	{"/Searches/Import Example", NULL, G_CALLBACK(cb_import),
	    0, "<Item>" },

	{ "/_View", NULL,  NULL, 0, "<Branch>" },
	{ "/_View/Stats Window", "<CTRL>I",  G_CALLBACK(cb_toggle_stats),
	    0,"<Item>" },
	{ "/_View/Progress Window", "<CTRL>P",
	    G_CALLBACK(cb_toggle_progress), 0,"<Item>" },
	{ "/_View/Log Window", "<CTRL>L",
	    G_CALLBACK(cb_toggle_log), 0,"<Item>" },
	{ "/_View/Cache Control", "<CTRL>C", G_CALLBACK(cb_toggle_ccontrol),
	    0,"<Item>" },
	{ "/Options", NULL, NULL, 0, "<Branch>" },
	{ "/Options/sep1", NULL, NULL, 0, "<Separator>" },
	{ NULL, NULL, NULL }
};


static GtkItemFactory *item_factory;

/* Returns a menubar widget made from the above menu */
static GtkWidget *
get_menubar_menu( GtkWidget  *window )
{
	GtkAccelGroup *accel_group;
	gint nmenu_items;

	/* Make an accelerator group (shortcut keys) */
	accel_group = gtk_accel_group_new ();

	/* Make an ItemFactory (that makes a menubar) */
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
	                                     accel_group);

	GtkItemFactoryEntry *tmp_menu;

	for(tmp_menu = menu_items, nmenu_items = 0;
	    (tmp_menu->path);
	    tmp_menu++) {
		nmenu_items++;
	}

	/* This function generates the menu items. Pass the item factory,
	   the number of items in the array, the array itself, and any
	   callback data for the the menu items. */
	gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, NULL);

	/* Attach the new accelerator group to the window. */
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	/* Finally, return the actual *menu* bar created by the item factory. */
	return gtk_item_factory_get_widget (item_factory, "<main>");
}

void
add_new_search_type(img_factory *factory)
{
	GtkItemFactoryEntry entry;
	char buf[BUFSIZ];

	sprintf(buf, "/Searches/New/%s", factory->get_name());

	entry.path = strdup(buf);
	entry.accelerator = NULL;
	entry.callback = G_CALLBACK(cb_create);
	entry.callback_action = 1;
	entry.item_type = "<Item>";

	gtk_item_factory_create_item(item_factory, &entry, factory, 1);
}

void
add_new_codec(img_factory *factory)
{
	GtkTreeIter iter;
	const char *name = factory->get_name();
	gtk_list_store_insert_with_values (codec_model, &iter, 0,
					   0, name,
					   1, factory,
					   -1);
	if (strcmp(name, "Built-in") == 0) {
	  gtk_combo_box_set_active(GTK_COMBO_BOX(codec_selector), 0);
	}
}



/*
 * makes the main window 
 */

static void
create_main_window(void)
{
	GtkWidget * separator;
	GtkWidget *main_vbox;
	GtkWidget *menubar;
	GtkWidget *main_box;

	GUI_THREAD_CHECK();

	/*
	 * Create the the main window.
	 */
	gui.main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (gui.main_window), "destroy",
	                  G_CALLBACK (cb_quit), NULL);

	gtk_window_set_title(GTK_WINDOW (gui.main_window), "Diamond SnapFind");

	main_vbox = gtk_vbox_new (FALSE, 1);
	gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 1);
	gtk_container_add (GTK_CONTAINER (gui.main_window), main_vbox);
	gtk_widget_show(main_vbox);

	menubar = get_menubar_menu (gui.main_window);
	gtk_box_pack_start (GTK_BOX (main_vbox), menubar, FALSE, TRUE, 0);
	gtk_widget_show(menubar);

	main_box = gtk_hbox_new (FALSE, 0);
	//gtk_container_add (GTK_CONTAINER (main_vbox), main_box);
	gtk_box_pack_end (GTK_BOX (main_vbox), main_box, FALSE, TRUE, 0);
	gtk_widget_show (main_box);

	gui.search_box = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(main_box), gui.search_box, FALSE, FALSE, 10);
	gtk_widget_show (gui.search_box);

	gui.search_widget = create_search_window();
	gtk_box_pack_start (GTK_BOX(gui.search_box), gui.search_widget,
	                    FALSE, FALSE, 10);

	separator = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(main_box), separator, FALSE, FALSE, 0);
	gtk_widget_show (separator);

	create_display_region(main_box);
}


static void
run_gui(void)
{
	pthread_t 	search_thread;
	int		err;

	/*
	 * initialize and start the background thread 
	 */
	init_search();
	err = pthread_create(&search_thread, NULL, sfind_search_main,
	                     snap_searchset);
	if (err) {
		perror("failed to create search thread");
		exit(1);
	}

	/*
	 * Display the main window
	 */
	gtk_widget_show(gui.main_window);

	/*
	 * Start the main loop processing for the GUI.
	 */

	MAIN_THREADS_ENTER();
	gtk_main();
	MAIN_THREADS_LEAVE();
}

static void
initialize_snapfind(void)
{
	/*
	 * Initialize communications rings with the thread
	 * that interacts with the search thread.
	 */
	to_search_thread = g_async_queue_new();
	if (!to_search_thread) {
		exit(1);
	}

	from_search_thread = g_async_queue_new();
	if (!from_search_thread) {
		exit(1);
	}


	snap_searchset = new search_set();

	/*
	 * Create the main window.
	 */
	tooltips = gtk_tooltips_new();
	gtk_tooltips_enable(tooltips);

	GUI_THREAD_ENTER();
	create_main_window();
	GUI_THREAD_LEAVE();

	/*
	 * start logging
	 */
	log_init("snapfind", NULL);

	/*
	 * Load all the plugins now.
	 */
	decoders_init();
	load_attr_map();
	load_plugins();
	//	init_logging();
}


static bool
sc(const char *a, const char *b) {
	return strcmp(a, b) == 0;
}

int
main(int argc, char *argv[])
{
	/*
	 * Init GTK
	 */

	GUI_THREAD_INIT();
	gtk_init(&argc, &argv);
	gdk_rgb_init();
	gtk_rc_parse("gtkrc");

	initialize_snapfind();

	/*
	 * Decide what we are doing
	 */
	const char *cmd;
	if (argc == 1) {
		cmd = "run-gui";
		printf("Running in GUI mode, run \"%s help\" for "
		       "other options\n",
		       argv[0]);
	} else {
		cmd = argv[1];
	}

	if (sc(cmd, "run-gui")) {
		run_gui();
		return 0;
	} else if (sc(cmd, "help")) {
		print_usage();
		return 0;
	} else if (sc(cmd, "list-plugins")) {
		list_plugins();
		return 0;
	} else if (sc(cmd, "get-plugin-initial-config")) {
		// check parameters
		if (argc < 4) {
			printf("Missing arguments\n");
			return 1;
		}
		return get_plugin_initial_config(argv[2], argv[3]);
	} else if (sc(cmd, "edit-plugin-config")) {
		// check parameters
		if (argc < 4) {
			printf("Missing arguments\n");
			return 1;
		}
		return edit_plugin_config(argv[2], argv[3]);
	} else if (sc(cmd, "run-plugin")) {
		// check parameters
		if (argc < 4) {
			printf("Missing arguments\n");
			return 1;
		}
		return run_plugin(argv[2], argv[3]);
	} else {
		printf("Unknown command: \"%s\"\n", cmd);
		return 1;
	}
}
