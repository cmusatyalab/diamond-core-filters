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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef linux
#include <getopt.h>
#else
#ifndef HAVE_DECL_GETOPT
#define HAVE_DECL_GETOPT 1
#endif
#include <gnugetopt/getopt.h>
#endif

#include "filter_api.h"
#include "common_consts.h"
#include "searchlet_api.h"
#include "gui_thread.h"

#include "queue.h"
#include "ring.h"
#include "rtimer.h"
#include "sf_consts.h"

#include "face_search.h"
#include "face_image.h"
#include "rgb.h"
#include "face.h"
#include "fil_tools.h"
#include "image_tools.h"
#include "face_widgets.h"
#include "texture_tools.h"
#include "img_search.h"
#include "sfind_search.h"
#include "histo.h"
#include "sfind_tools.h"
#include "snap_popup.h"
#include "search_support.h"
#include "snapfind.h"
#include "import_sample.h"
#include "gtk_image_tools.h"
#include "fil_image_tools.h"
#include "search_set.h"
#include "read_config.h"

/* number of thumbnails to show */
static const int TABLE_COLS = 3;
static const int TABLE_ROWS = 2;

static int default_min_faces = 0;
static int default_face_levels = 37;


thumblist_t thumbnails = TAILQ_HEAD_INITIALIZER(thumbnails);
thumbnail_t *cur_thumbnail = NULL;

/* XXX move these later to common header */
#define	TO_SEARCH_RING_SIZE		512
#define	FROM_SEARCH_RING_SIZE		512


int expert_mode = 0;		/* global (also used in face_widgets.c) */
int dump_attributes = 0;
char *dump_spec_file = NULL;		/* dump spec file and exit */
int dump_objects = 0;		/* just dump all the objects and exit (no gui) */
GtkTooltips *tooltips = NULL;
char *read_spec_filename = NULL;

/* XXX */
int do_display = 1;


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


TAILQ_HEAD(export_list_t, export_threshold_t) export_list = TAILQ_HEAD_INITIALIZER(export_list);


static struct
{
	GtkWidget *main_window;
	GtkWidget *min_faces;
	GtkWidget *face_levels;
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

static lf_fhandle_t fhandle = 0;	/* XXX */

#define		MAX_SEARCHES	64	/* XXX */

/**********************************************************************/

/*
 * state required to support popup window to show fullsize img
 */

pop_win_t	 popup_window = {NULL, NULL, NULL};



/* ********************************************************************** */

/* some stats for user study */

struct
{
	int total_seen, total_marked;
}
user_measurement = { 0, 0 };


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
	GtkWidget *	parent_box;
	GtkWidget *	info_box1;
	GtkWidget *	info_box2;
	GtkWidget *	name_tag;
	GtkWidget *	name_label;
	GtkWidget *	count_tag;
	GtkWidget *	count_label;

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
ring_data_t *	to_search_thread;
ring_data_t *	from_search_thread;
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
pthread_mutex_t	ring_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t	thumb_mutex = PTHREAD_MUTEX_INITIALIZER;



/*
 * some prototypes 
 */

extern region_t draw_bounding_box(RGBImage *img, int scale,
	                                  lf_fhandle_t fhandle, ls_obj_handle_t ohandle,
	                                  RGBPixel color, RGBPixel mask, char *fmt, int i);
static GtkWidget *make_gimage(RGBImage *img, int w, int h);


/* from face_search.c */
extern void drain_ring(ring_data_t *ring);


/* ********************************************************************** */

struct collection_t
{
	char *name;
	//int id;
	int active;
};

struct collection_t collections[MAX_ALBUMS+1] =
    {
	    {
		    NULL
	    }
    };




/* ********************************************************************** */
/* utility functions */
/* ********************************************************************** */


/*
 * make a gtk image from an img
 */
static GtkWidget *
make_gimage(RGBImage *img, int dest_width, int dest_height)
{
	GdkPixbuf *pbuf; // *scaled_pbuf;

	//fprintf(stderr, "gimage called\n");

	GUI_THREAD_CHECK();

	pbuf = gdk_pixbuf_new_from_data((const guchar *)&img->data[0],
	                                GDK_COLORSPACE_RGB, 1, 8,
	                                img->columns, img->rows,
	                                (img->columns*sizeof(RGBPixel)),
	                                NULL,
	                                NULL);
	if (pbuf == NULL) {
		printf("failed to allocate pbuf\n");
		exit(1);
	}


	GtkWidget *image;
	image = gtk_image_new_from_pixbuf(pbuf);
	assert(image);

	return image;
}




static void
clear_image_info(image_info_t *img_info)
{
	char	data[BUFSIZ];

	GUI_THREAD_CHECK();

	sprintf(data, "%-60s", " ");
	gtk_label_set_text(GTK_LABEL(img_info->name_label), data);

	sprintf(data, "%-3s", " ");
	gtk_label_set_text(GTK_LABEL(img_info->count_label), data);

	//gtk_label_set_text(GTK_LABEL(img_info->qsize_label), data);

}


static void
write_image_info(image_info_t *img_info, char *name, int count)
{
	char	txt[BUFSIZ];

	GUI_THREAD_CHECK();

	sprintf(txt, "%-60s", name);
	gtk_label_set_text(GTK_LABEL(img_info->name_label), txt);

	sprintf(txt, "%-3d", count);
	gtk_label_set_text(GTK_LABEL(img_info->count_label), txt);
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
get_gid_list(gid_list_t *main_region)
{
	int	i;
	int j;
	/*
	 * figure out the args and build message
	 */
	for(i=0; i<MAX_ALBUMS && collections[i].name; i++) {
		/* if collection active, figure out the gids and add to out list
		 * allows duplicates XXX */
		if(collections[i].active) {
			int err;
			int num_gids = MAX_ALBUMS;
			groupid_t gids[MAX_ALBUMS];
			err = nlkup_lookup_collection(collections[i].name, &num_gids, gids);
			assert(!err);
			for (j=0; j < num_gids; j++) {
				main_region->gids[main_region->ngids++] = gids[j];
			}
		}
	}
}


static void
do_img_mark(GtkWidget *widget)
{
	thumbnail_t *thumb;

	thumb= (thumbnail_t *)gtk_object_get_user_data(GTK_OBJECT(widget));

	thumb->marked ^= 1;

	/* adjust count */
	user_measurement.total_marked += (thumb->marked ? 1 : -1);

	printf("marked count = %d/%d\n", user_measurement.total_marked,
	       user_measurement.total_seen);

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




/* ********************************************************************** */

static void
display_thumbnail(ls_obj_handle_t ohandle)
{
	RGBImage        *rgbimg, *scaledimg;
	char            name[MAX_NAME];
	off_t		bsize;
	lf_fhandle_t	fhandle = 0; /* XXX */
	int		err;
	int		num_face, num_histo;

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

	/* get path XXX */
	bsize = MAX_NAME;
	err = lf_read_attr(fhandle, ohandle, DISPLAY_NAME, &bsize, name);
	if(err) {
		err = lf_read_attr(fhandle, ohandle, OBJ_PATH, &bsize, name);
	}
	if (err) {
		sprintf(name, "%s", "uknown");
		bsize = strlen(name);
	}
	name[bsize] = '\0';	/* terminate string */


	/* get the img */
	rgbimg = (RGBImage*)ft_read_alloc_attr(fhandle, ohandle, RGB_IMAGE);

	if (rgbimg == NULL) {
		//rgbimg = get_attr_rgb_img(ohandle, "DATA0");
		rgbimg = get_rgb_img(ohandle);
	}
	assert(rgbimg);
	assert(rgbimg->width);

	/* figure out bboxes to highlight */
	bsize = sizeof(num_histo);
	err = lf_read_attr(fhandle, ohandle, NUM_HISTO, &bsize, (char *)&num_histo);
	if (err) {
		num_histo = 0;
	}
	bsize = sizeof(num_face);
	err = lf_read_attr(fhandle, ohandle, NUM_FACE, &bsize, (char *)&num_face);
	if (err) {
		num_face = 0;
	}


	int scale = (int)ceil(compute_scale(rgbimg, THUMBSIZE_X, THUMBSIZE_Y));
	scaledimg = image_gen_image_scale(rgbimg, scale);
	assert(scaledimg);

	for(int i=0; i<num_histo; i++) {
		draw_bounding_box(scaledimg, scale, fhandle, ohandle,
		                  green, colorMask, HISTO_BBOX_FMT, i);
	}
	for(int i=0; i<num_face; i++) {
		draw_bounding_box(scaledimg, scale, fhandle, ohandle,
		                  red, colorMask, FACE_BBOX_FMT, i);
	}

	user_measurement.total_seen++;

	GUI_THREAD_ENTER();

	/*
	 * Build image the new data
	 */
	GtkWidget *image = make_gimage(scaledimg, THUMBSIZE_X, THUMBSIZE_Y);
	assert(image);

	/*
	 * update the display
	 */

	pthread_mutex_lock(&thumb_mutex);

	if(!cur_thumbnail) {
		cur_thumbnail = TAILQ_FIRST(&thumbnails);
	}
	if (cur_thumbnail->img) { /* cleanup */
		gtk_container_remove(GTK_CONTAINER(cur_thumbnail->viewport),
		                     cur_thumbnail->gimage);
		lf_free_buffer(fhandle, (char *)cur_thumbnail->img); /* XXX */
		//lf_free_buffer(fhandle, (char *)cur_thumbnail->fullimage); /* XXX */
		ih_drop_ref(cur_thumbnail->hooks, fhandle);
		//ls_release_object(fhandle, cur_thumbnail->ohandle);
	}
	gtk_frame_set_label(GTK_FRAME(cur_thumbnail->frame), "");
	cur_thumbnail->marked = 0;
	cur_thumbnail->img = scaledimg;
	cur_thumbnail->gimage = image;
	strcpy(cur_thumbnail->name, name);
	cur_thumbnail->nboxes = num_histo;
	cur_thumbnail->nfaces = num_face;
	cur_thumbnail->hooks = ih_new_ref(rgbimg, (HistoII*)NULL, ohandle);

	gtk_container_add(GTK_CONTAINER(cur_thumbnail->viewport), image);
	gtk_widget_show_now(image);


	cur_thumbnail = TAILQ_NEXT(cur_thumbnail, link);

	/* check if panel is full */
	if(cur_thumbnail == NULL) {
		pthread_mutex_unlock(&thumb_mutex);

		enable_image_control(&image_controls);
		//fprintf(stderr, "WINDOW FULL. waiting...\n");
		GUI_THREAD_LEAVE();

		/* block until user hits next */
		pthread_mutex_lock(&display_mutex);
		image_controls.cur_op = CNTRL_WAIT;
		pthread_cond_wait(&display_cond, &display_mutex);
		pthread_mutex_unlock(&display_mutex);

		GUI_THREAD_ENTER();
		disable_image_control(&image_controls);
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
	clear_image_info(&image_information);

	pthread_mutex_lock(&thumb_mutex);
	TAILQ_FOREACH(cur_thumbnail, &thumbnails, link) {
		if (cur_thumbnail->img) { /* cleanup */
			gtk_container_remove(GTK_CONTAINER(cur_thumbnail->viewport),
			                     cur_thumbnail->gimage);
			free(cur_thumbnail->img); /* XXX */
			ih_drop_ref(cur_thumbnail->hooks, fhandle);
			cur_thumbnail->img = NULL;
		}

	}
	cur_thumbnail = NULL;
	pthread_mutex_unlock(&thumb_mutex);
}


static void *
display_thread(void *data)
{
	message_t *		message;
	struct timespec timeout;


	while (1) {

		if (do_display) {
			pthread_mutex_lock(&ring_mutex);
			message = (message_t *)ring_deq(from_search_thread);
			pthread_mutex_unlock(&ring_mutex);

			if (message != NULL) {
				switch (message->type) {
					case NEXT_OBJECT:
						display_thumbnail((ls_obj_handle_t)message->data);
						break;

					case DONE_OBJECTS:
						/*
							* We are done recieving objects.
							* We need to disable the thread
							* image controls and enable start
							* search button.
							*/

						free(message);
						gtk_widget_set_sensitive(gui.start_button, TRUE);
						gtk_widget_set_sensitive(gui.stop_button, FALSE);
						display_thread_running = 0;
						pthread_exit(0);
						break;

					default:
						break;

				}
				free(message);
			}
		}

		timeout.tv_sec = 0;
		timeout.tv_nsec = 10000000; /* XXX 10 ms?? */
		nanosleep(&timeout, NULL);

	}
	return 0;
}

static void
stop_search()
{
	message_t *		message;
	int			err;

	/*
	 * Toggle the start and stop buttons.
	 */
	gtk_widget_set_sensitive(gui.start_button, TRUE);

	/* we should not be messing with the default. this is here so
	 * that we can trigger a search from the text entry without a
	 * return-pressed handler.  XXX */
	gtk_widget_grab_default (gui.start_button);
	gtk_widget_set_sensitive(gui.stop_button, FALSE);

	message = (message_t *)malloc(sizeof(*message));
	if (message == NULL) {
		printf("failed to allocate message \n");
		exit(1);
	}

	message->type = TERM_SEARCH;
	message->data = NULL;

	err = ring_enq(to_search_thread, message);
	if (err) {
		printf("XXX failed to enq message \n");
		exit(1);
	}
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
	gtk_widget_set_sensitive(gui.stop_button, TRUE);
	clear_thumbnails();

	/* another global, ack!! this should be on the heap XXX */
	static gid_list_t gid_list;
	gid_list.ngids = 0;
	get_gid_list(&gid_list);

	pthread_mutex_lock(&display_mutex);
	image_controls.cur_op = CNTRL_NEXT;
	pthread_mutex_unlock(&display_mutex);


	/* problem: the signal (below) gets handled before the search
	   thread has a chance to drain the ring. so do it here. */
	pthread_mutex_lock(&ring_mutex);
	drain_ring(from_search_thread);
	pthread_mutex_unlock(&ring_mutex);


	/* send the message */
	message = (message_t *)malloc(sizeof(*message));
	if (message == NULL) {
		printf("failed to allocate message \n");
		exit(1);
	}
	message->type = START_SEARCH;
	message->data = (void *)&gid_list;
	err = ring_enq(to_search_thread, message);
	if (err) {
		printf("XXX failed to enq message \n");
		exit(1);
	}

	GUI_CALLBACK_LEAVE();	/* need to do this before signal (below) */

	if(!display_thread_running) {
		display_thread_running = 1;
		err = pthread_create(&display_thread_info, PATTR_DEFAULT, display_thread, NULL);
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
	snap_searchset->build_filter_spec(buf);

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

static void
update_search_entry(search_set *cur_set)
{
	gtk_container_remove(GTK_CONTAINER(search_frame), config_table);
	config_table = cur_set->build_edit_table();
	gtk_container_add(GTK_CONTAINER(search_frame), config_table);
	gtk_widget_show_all(search_frame);

}


static GtkWidget *
create_search_window()
{
	GtkWidget *box2, *box1;
	GtkWidget *separator;

	GUI_THREAD_CHECK();

	box1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (box1);

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
	gtk_box_pack_start (GTK_BOX (box2), gui.stop_button, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS (gui.stop_button, GTK_CAN_DEFAULT);
	gtk_widget_set_sensitive(gui.stop_button, FALSE);
	gtk_widget_show (gui.stop_button);

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

	g_signal_connect(GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
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
	toggle_stats_win(shandle, expert_mode);
	GUI_CALLBACK_LEAVE();
}

/* For the check button */
static void
cb_toggle_progress(gpointer   callback_data,
                   guint      callback_action,
                   GtkWidget *menu_item )
{
	GUI_CALLBACK_ENTER();
	toggle_progress_win(shandle, expert_mode);
	GUI_CALLBACK_LEAVE();
}


static void
cb_toggle_ccontrol(gpointer   callback_data,
                   guint      callback_action,
                   GtkWidget *menu_item )
{
	GUI_CALLBACK_ENTER();
	toggle_ccontrol_win(shandle, expert_mode);
	GUI_CALLBACK_LEAVE();
}



/* For the check button */
static void
cb_toggle_dump_attributes( gpointer   callback_data,
                           guint      callback_action,
                           GtkWidget *menu_item )
{
	GUI_CALLBACK_ENTER();
	dump_attributes = GTK_CHECK_MENU_ITEM (menu_item)->active;
	GUI_CALLBACK_LEAVE();
}



/* ********************************************************************** */
/* widget setup functions */
/* ********************************************************************** */

static void
create_image_info(GtkWidget *container_box, image_info_t *img_info)
{

	char		data[BUFSIZ];

	GUI_THREAD_CHECK();

	/*
	 * Now create another region that has the controls
	 * that manipulate the current image being displayed.
	 */

	img_info->parent_box = container_box;
	img_info->info_box1 = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(img_info->info_box1), 10);

	GtkWidget *frame;
	frame = gtk_frame_new("Image Info");
	gtk_container_add(GTK_CONTAINER(frame), img_info->info_box1);
	gtk_widget_show(frame);

	gtk_box_pack_start(GTK_BOX(img_info->parent_box),
	                   frame, TRUE, TRUE, 0);
	gtk_widget_show(img_info->info_box1);


	/*
	 * image name
	 */
	img_info->name_tag = gtk_label_new("Name:");
	gtk_box_pack_start (GTK_BOX(img_info->info_box1),
	                    img_info->name_tag, FALSE, FALSE, 0);
	gtk_widget_show(img_info->name_tag);


	sprintf(data, "%-60s:", " ");
	img_info->name_label = gtk_label_new(data);
	gtk_box_pack_start (GTK_BOX(img_info->info_box1),
	                    img_info->name_label, TRUE, TRUE, 0);
	gtk_widget_show(img_info->name_label);

	/*
	 * Place holder and blank spot for the number of bounding
	 * boxes found.
	 */
	sprintf(data, "%-3s", " ");
	img_info->count_label = gtk_label_new(data);
	gtk_box_pack_end (GTK_BOX(img_info->info_box1),
	                  img_info->count_label, FALSE, FALSE, 0);
	gtk_widget_show(img_info->count_label);

	img_info->count_tag = gtk_label_new("Num Scenes:");
	gtk_box_pack_end(GTK_BOX(img_info->info_box1),
	                 img_info->count_tag, FALSE, FALSE, 0);
	gtk_widget_show(img_info->count_tag);
}


static void
cb_next_image(GtkButton* item, gpointer data)
{
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
cb_img_info(GtkWidget *widget, gpointer data)
{
	thumbnail_t *thumb;

	GUI_CALLBACK_ENTER();
	thumb = (thumbnail_t *)gtk_object_get_user_data(GTK_OBJECT(widget));

	/* the data gpointer passed in seems to not be the data
	 * pointer we gave gtk. instead, we save a pointer in the
	 * widget. -RW */

	//fprintf(stderr, "thumb=%p\n", thumb);

	if(thumb->img) {
		write_image_info(&image_information, thumb->name, thumb->nboxes);
	}
	GUI_CALLBACK_LEAVE();
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
	gtk_timeout_add(500 /* ms */, timeout_write_qsize, img_info->qsize_label);

	img_info->tobjs_label = gtk_label_new ("");
	gtk_box_pack_start(GTK_BOX(img_cntrl->control_box),
	                   img_info->tobjs_label, FALSE, FALSE, 0);
	gtk_widget_show(img_info->tobjs_label);
	gtk_timeout_add(500 /* ms */, timeout_write_tobjs, img_info->tobjs_label);

	img_info->sobjs_label = gtk_label_new ("");
	gtk_box_pack_start(GTK_BOX(img_cntrl->control_box),
	                   img_info->sobjs_label, FALSE, FALSE, 0);
	gtk_widget_show(img_info->sobjs_label);
	gtk_timeout_add(500 /* ms */, timeout_write_sobjs, img_info->sobjs_label);

	img_info->dobjs_label = gtk_label_new ("");
	gtk_box_pack_start(GTK_BOX(img_cntrl->control_box),
	                   img_info->dobjs_label, FALSE, FALSE, 0);
	gtk_widget_show(img_info->dobjs_label);
	gtk_timeout_add(500 /* ms */, timeout_write_dobjs, img_info->dobjs_label);

	adj = gtk_adjustment_new(2.0, 1.0, 10.0, 1.0, 1.0, 1.0);
	img_cntrl->zlevel = 2;
	img_cntrl->zbutton = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_box_pack_start (GTK_BOX(img_cntrl->control_box),img_cntrl->zbutton,
	                    FALSE, FALSE, 0);
	//gtk_widget_show(img_cntrl->zbutton);


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
	GtkWidget *	separator;
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

	/* create the image information area */
	create_image_info(result_box, &image_information);

	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX(result_box), separator, FALSE, FALSE, 0);
	gtk_widget_show (separator);

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
			gtk_widget_set_size_request(widget, THUMBSIZE_X, THUMBSIZE_Y);
			gtk_container_add(GTK_CONTAINER(frame), widget);
			gtk_table_attach_defaults(GTK_TABLE(thumbnail_view), eb,
			                          j, j+1, i, i+1);
			gtk_widget_show(widget);
			gtk_widget_show(eb);

			thumbnail_t *thumb = (thumbnail_t *)malloc(sizeof(thumbnail_t));
			thumb->marked = 0;
			thumb->img = NULL;
			thumb->viewport = widget;
			thumb->frame = frame;
			thumb->name[0] = '\0';
			thumb->nboxes = 0;
			thumb->nfaces = 0;
			thumb->hooks = NULL;
			TAILQ_INSERT_TAIL(&thumbnails, thumb, link);

			//fprintf(stderr, "thumb=%p\n", thumb);

			/* the data pointer mechanism seems to be
			 * broken, so use the object user pointer
			 * instead */
			gtk_object_set_user_data(GTK_OBJECT(eb), thumb);
			gtk_signal_connect(GTK_OBJECT(eb),
			                   "enter-notify-event",
			                   GTK_SIGNAL_FUNC(cb_img_info),
			                   (gpointer)thumb);
			gtk_signal_connect(GTK_OBJECT(eb),
			                   "button-press-event",
			                   GTK_SIGNAL_FUNC(cb_img_popup),
			                   (gpointer)thumb);
		}
	}
	gtk_widget_show(thumbnail_view);


	/* create the image control area */
	create_image_control(result_box, &image_information, &image_controls);


	clear_image_info(&image_information);
	disable_image_control(&image_controls);
}


static void
cb_quit()
{
	printf("MARKED: %d of %d seen\n", user_measurement.total_marked,
	       user_measurement.total_seen);
	gtk_main_quit();
}


static void
cb_import(GtkWidget *widget, gpointer user_data)
{
	open_import_window(snap_searchset);
}


static void
cb_create(GtkWidget *widget, gpointer user_data)
{
	GtkWidget 	*hbox;
	GtkWidget 	*vbox;
	GtkWidget	*dialog;
	GtkWidget	*label;
	GtkWidget	*helplabel;
	GtkWidget	*entry;
	int			x, result;
	unsigned int	i;
	const char *		new_name;
	search_types_t	stype;
	img_search *	ssearch;
	x = (int) user_data;

	stype = (search_types_t)x;

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

		ssearch = create_search(stype, new_name);
		assert(ssearch != NULL);

		/* add to the list of searches */
		snap_searchset->add_search(ssearch);

		/* popup the new search edit box */
		ssearch->edit_search();
	}
	gtk_widget_destroy(dialog);

	/* XXX get the name from the user */
}


/* Our menu, an array of GtkItemFactoryEntry structures that defines each menu item */
static GtkItemFactoryEntry menu_items[] = { /* XXX */

            { "/_File", NULL,  NULL,           0, "<Branch>" },
            { "/File/Load Search", NULL, G_CALLBACK(cb_load_search), 0, "<Item>" },
            { "/File/Import Search", NULL, G_CALLBACK(cb_import_search), 0, "<Item>" },
            { "/File/Save Search as", NULL,  G_CALLBACK(cb_save_search_as),  0, "<Item>" },
            { "/File/Save filterspec", NULL,  G_CALLBACK(cb_save_spec_to_filename),
              0, "<Item>" },
            { "/File/sep1",     NULL,         NULL,           0, "<Separator>" },
            { "/File/_Quit", "<CTRL>Q", (GtkItemFactoryCallback)cb_quit, 0, "<StockItem>", GTK_STOCK_QUIT },

            { "/_Searches", NULL, NULL, 0, "<Branch>"},
            {"/Searches/New", NULL, NULL, 0, "<Branch>"},
            {"/Searches/New/RGB Histogram", NULL, G_CALLBACK(cb_create), RGB_HISTO_SEARCH, "<Item>" },
            {"/Searches/New/VJ Face Detect", NULL, G_CALLBACK(cb_create), VJ_FACE_SEARCH, "<Item>" },
            {"/Searches/New/OCV Face Detect", NULL, G_CALLBACK(cb_create), OCV_FACE_SEARCH, "<Item>" },
            {"/Searches/New/Texture", NULL, G_CALLBACK(cb_create), TEXTURE_SEARCH, "<Item>" },
            {"/Searches/New/Gabor Texture", NULL, G_CALLBACK(cb_create), GABOR_TEXTURE_SEARCH, "<Item>" },
            {"/Searches/New/Regex", NULL, G_CALLBACK(cb_create), REGEX_SEARCH, "<Item>" },
            {"/Searches/Import Example", NULL, G_CALLBACK(cb_import), 0, "<Item>" },

            { "/_View", NULL,  NULL, 0, "<Branch>" },
            { "/_View/Stats Window", "<CTRL>I",  G_CALLBACK(cb_toggle_stats), 0,"<Item>" },
            { "/_View/Progress Window", "<CTRL>P",  G_CALLBACK(cb_toggle_progress), 0,"<Item>" },
            { "/_View/Cache Control", "<CTRL>C",  G_CALLBACK(cb_toggle_ccontrol), 0,"<Item>" },

            { "/Options",             NULL, NULL, 0, "<Branch>" },
            { "/Options/sep1",            NULL, NULL, 0, "<Separator>" },
            { "/Options/Dump Attributes", NULL, G_CALLBACK(cb_toggle_dump_attributes), 0, "<CheckItem>" },
            { "/Albums",                  NULL, NULL, 0, "<Branch>" },
            { "/Albums/tear",             NULL, NULL, 0, "<Tearoff>" },
            { NULL, NULL, NULL }
        };

static void
cb_collection(gpointer callback_data, guint callback_action,
              GtkWidget  *menu_item )
{

	/* printf("cb_collection: #%d\n", callback_action); */

	if(GTK_CHECK_MENU_ITEM(menu_item)->active) {
		collections[callback_action].active = 1;
	} else {
		collections[callback_action].active = 0;
	}
}


/* Returns a menubar widget made from the above menu */
static GtkWidget *
get_menubar_menu( GtkWidget  *window )
{
	GtkItemFactory *item_factory;
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
	gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);

	/* create more menu items */
	struct collection_t *tmp_coll;
	for(tmp_coll = collections; tmp_coll->name; tmp_coll++) {
		GtkItemFactoryEntry entry;
		char buf[BUFSIZ];

		sprintf(buf, "/Albums/%s", tmp_coll->name);
		for(char *s=buf; *s; s++) {
			if(*s == '_') {
				*s = ' ';
			}
		}
		entry.path = strdup(buf);
		entry.accelerator = NULL;
		entry.callback = G_CALLBACK(cb_collection);
		entry.callback_action = tmp_coll - collections;
		entry.item_type = "<CheckItem>";
		gtk_item_factory_create_item(item_factory,
		                             &entry,
		                             NULL,
		                             1); /* XXX guess, no doc */

		GtkWidget *widget = gtk_item_factory_get_widget(item_factory, buf);
		//gtk_widget_set_sensitive(widget, FALSE);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), tmp_coll->active);

		//tmp_menu++;
		//nmenu_items++;
	}

	/* Attach the new accelerator group to the window. */
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	/* Finally, return the actual *menu* bar created by the item factory. */
	return gtk_item_factory_get_widget (item_factory, "<main>");
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

	gtk_widget_show(gui.main_window);
}





static void
usage(char **argv)
{
	fprintf(stderr, "usage: %s [options]\n", basename(argv[0]));
	fprintf(stderr, "  -h        - help\n");
	fprintf(stderr, "  -e        - expert mode\n");
	fprintf(stderr, "  -s<file>  - histo index file\n");
	fprintf(stderr, "  --similarity <filter>=<val>  - set default threshold (repeatable)\n");
	fprintf(stderr, "  --min-faces=<num>       - set min number of faces \n");
	fprintf(stderr, "  --face-levels=<num>     - set face detector levels \n");
	fprintf(stderr, "  --dump-spec=<file>      - dump spec file and exit \n");
	fprintf(stderr, "  --dump-objects          - start search and dump objects\n");
	fprintf(stderr, "  --read-spec=<file>      - use spec file. requires dump-objects\n");
}



static int
set_export_threshold(char *arg)
{
	char *s = arg;

	while(*s && *s != '=') {
		s++;
	}
	if(!*s)
		return -1;		/* error */
	*s++ = '\0';

	export_threshold_t *et = (export_threshold_t *)malloc(sizeof(export_threshold_t));
	assert(et);
	et->name = arg;
	double d = atof(s);
	if(d > 1)
		d /= 100.0;
	if(d > 1 || d < 0) {
		fprintf(stderr, "bad distance: %s\n", s);
		return -1;
	}
	et->distance = 1.0 - d;	/* similarity */
	et->index = -1;

	TAILQ_INSERT_TAIL(&export_list, et, link);

	return 0;
}

/* XXX fix */
void rgb_histo_init();
void vj_face_init();
void ocv_face_init();
void texture_init();
void gabor_texture_init();

int
main(int argc, char *argv[])
{

	pthread_t 	search_thread;
	int		err;
	char *scapeconf = "histo/search_config";
	int c;
	static const char *optstring = "hes:f:";
	struct option long_opt[] = {
		                           {"dump-spec", required_argument, NULL, 0
		                           },
		                           {"dump-objects", no_argument, NULL, 0},
		                           {"read-spec", required_argument, NULL, 0},
		                           {"similarity", required_argument, NULL, 'f'},
		                           {"min-faces", required_argument, NULL, 0},
		                           {"face-levels", required_argument, NULL, 0},
		                           {0, 0, 0, 0}
	                           };
	int option_index = 0;

	/*
	 * Start the GUI
	 */

	GUI_THREAD_INIT();
	gtk_init(&argc, &argv);
	gtk_rc_parse("gtkrc");
	gdk_rgb_init();
	printf("Starting main\n");

	while((c = getopt_long(argc, argv, optstring, long_opt, &option_index)) != -1) {
		switch(c) {
			case 0:
				if(strcmp(long_opt[option_index].name, "dump-spec") == 0) {
					dump_spec_file = optarg;
				} else if(strcmp(long_opt[option_index].name, "dump-objects") == 0) {
					dump_objects = 1;
				} else if(strcmp(long_opt[option_index].name, "read-spec") == 0) {
					read_spec_filename = optarg;
				} else if(strcmp(long_opt[option_index].name, "min-faces") == 0) {
					default_min_faces = atoi(optarg);
				} else if(strcmp(long_opt[option_index].name, "face-levels") == 0) {
					default_face_levels = atoi(optarg);
				}
				break;
			case 'e':
				fprintf(stderr, "expert mode must now be turned on with the menu option\n");
				//expert_mode = 1;
				break;
			case 's':
				scapeconf = optarg;
				break;
			case 'f':
				if(set_export_threshold(optarg) < 0) {
					usage(argv);
					exit(1);
				}
				break;
			case ':':
				fprintf(stderr, "missing parameter\n");
				exit(1);
			case 'h':
			case '?':
			default:
				usage(argv);
				exit(1);
				break;
		}
	}

	printf("Initializing communciation rings...\n");

	/*
	 * Initialize communications rings with the thread
	 * that interacts with the search thread.
	 */
	err = ring_init(&to_search_thread, TO_SEARCH_RING_SIZE);
	if (err) {
		exit(1);
	}

	err = ring_init(&from_search_thread, FROM_SEARCH_RING_SIZE);
	if (err) {
		exit(1);
	}


	snap_searchset = new search_set();

	/*
	 * read the list of collections
	 */
	{
		void *cookie;
		char *name;
		int err;
		int pos = 0;
		err = nlkup_first_entry(&name, &cookie);
		while(!err && pos < MAX_ALBUMS)
		{
			collections[pos].name = name;
			collections[pos].active = pos ? 0 : 1; /* first one active */
			pos++;
			err = nlkup_next_entry(&name, &cookie);
		}
		collections[pos].name = NULL;
	}


	/*
	 * Create the main window.
	 */
	tooltips = gtk_tooltips_new();
	gtk_tooltips_enable(tooltips);


	GUI_THREAD_ENTER();
	create_main_window();
	GUI_THREAD_LEAVE();

	/*
	 * initialize and start the background thread 
	 */
	init_search();
	err = pthread_create(&search_thread, PATTR_DEFAULT, sfind_search_main,
	                     snap_searchset);
	if (err) {
		perror("failed to create search thread");
		exit(1);
	}
	//gtk_timeout_add(100, func, NULL);

	/* XXX for now */
	rgb_histo_init();
	vj_face_init();
	ocv_face_init();
	texture_init();
	gabor_texture_init();

	/*
	 * Start the main loop processing for the GUI.
	 */

	MAIN_THREADS_ENTER();
	gtk_main();
	MAIN_THREADS_LEAVE();

	return(0);
}
