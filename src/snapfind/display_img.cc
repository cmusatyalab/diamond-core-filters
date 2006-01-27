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

/*
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <gtk/gtk.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>		/* dirname */
#include <assert.h>
#include <signal.h>
#include <getopt.h>

#include "lib_searchlet.h"
#include "lib_filter.h"

#include "gui_thread.h"

#include "queue.h"
#include "ring.h"
#include "rtimer.h"

#include "lib_results.h"
#include "lib_sfimage.h"
#include "rgb.h"
#include "img_search.h"
#include "search_support.h"
#include "gtk_image_tools.h"
#include "search_set.h"
#include "display_img.h"


#define COORDS_TO_BBOX(bbox) \
{	\
  bbox.min_x = min(x1, x2); \
  bbox.max_x = max(x1,x2); \
  bbox.min_y = min(y1, y2); \
  bbox.max_y = max(y1, y2); \
}

#define	MIN_DIMENSION	10

#ifdef	XXX
static  int
min(int a, int b)
{
	int min
	return((a < b) ? a : b);
}


static inline int
max(int a, int b)
{
	int max
	max =  (a > b) ? a : b;
	return(max);
}
#endif

static double
compute_scale(RGBImage * img, int xdim, int ydim)
{
	double          scale = 1.0;

	scale = max(scale, (double) img->width / xdim);
	scale = max(scale, (double) img->height / ydim);

	return scale;
}


display_img::display_img(int wd, int ht)
{
	di_width = wd;
	di_height = ht;
	di_cur_img = NULL;
	pthread_mutex_init(&di_mutex, NULL);
}


display_img::~display_img()
{
	/* XXX free images */
}

/*
 * make pixbuf from img
 * XXX move to library
 */
static GdkPixbuf*
pb_from_img(RGBImage *img)
{
	GdkPixbuf *pbuf;

	/* NB pixbuf refers to data */
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
	return pbuf;
}

static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	display_img *dimg;
	dimg = (display_img *)
	       gtk_object_get_user_data(GTK_OBJECT(widget));
	dimg->process_expose(widget, event);
	return(TRUE);
}

void
display_img::process_expose(GtkWidget *widget, GdkEventExpose *event)
{
	int width, height;

	pthread_mutex_lock(&di_mutex);
	width = min(event->area.width, di_cur_img->width - event->area.x);
	height = min(event->area.height, di_cur_img->height - event->area.y);

	if(width <= 0 || height <= 0) {
		pthread_mutex_unlock(&di_mutex);
		goto done;
	}
	assert(widget == di_drawingarea);

	gdk_window_clear_area(widget->window,
	                      event->area.x, event->area.y,
	                      event->area.width, event->area.height);
	gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
	                           &event->area);


	for(int i=0; i<DI_MAX_LAYERS; i++) {
		int pht = gdk_pixbuf_get_height(di_pixbufs[i]);
		assert(event->area.y + height <= pht);
		assert(width >= 0);
		assert(height >= 0);
		gdk_pixbuf_render_to_drawable_alpha(di_pixbufs[i],
		                                    widget->window,
		                                    event->area.x, event->area.y,
		                                    event->area.x, event->area.y,
		                                    width, height,
		                                    GDK_PIXBUF_ALPHA_FULL, 1, /* ign */
		                                    GDK_RGB_DITHER_MAX,
		                                    0, 0);
	}
	pthread_mutex_unlock(&di_mutex);

	gdk_gc_set_clip_rectangle(widget->style->fg_gc[widget->state],
	                          NULL);

done:
	return;
}


/* draw all the bounding boxes */
void
display_img::draw_res(GtkWidget *widget)
{

	GUI_CALLBACK_ENTER();

	/* although we clear the pixbuf data here, we still need to
	 * generate refreshes for either the whole image or the parts
	 * that we cleared. */
	rgbimg_clear(di_layers[DI_RES_LAYER]);


	GUI_CALLBACK_LEAVE();
}


static gboolean
cb_realize_event(GtkWidget *widget, GdkEventAny *event, gpointer data)
{
	display_img *dimg;
	dimg = (display_img *)
	       gtk_object_get_user_data(GTK_OBJECT(widget));
	dimg->event_realize();
	return(TRUE);
}


void
display_img::event_realize()
{
	pthread_mutex_lock(&di_mutex);
	for(int i=0; i<DI_MAX_LAYERS; i++) {
		di_pixbufs[i] = pb_from_img(di_layers[i]);
	}
	pthread_mutex_unlock(&di_mutex);

}

int
display_img::num_selections()
{
	return(di_nselections);
}

void
display_img::get_selection(int i, bbox_t *bbox)
{
	*bbox = di_selections[i];
	return;
}

RGBImage *
display_img::get_image()
{
	return(di_cur_img);
}



void
display_img::clear_selections()
{
	RGBImage *img;

	img = di_layers[DI_SELECT_LAYER];
	rgbimg_clear(img);
	gtk_widget_queue_draw(di_drawingarea);

	di_nselections = 0;
}


void
display_img::clear_highlight()
{
	RGBImage *img;
	pthread_mutex_lock(&di_mutex);
	img = di_layers[DI_HIGHLIGHT_LAYER];
	rgbimg_clear(img);
	pthread_mutex_unlock(&di_mutex);
	gtk_widget_queue_draw_area(di_drawingarea,
	                           0, 0, img->width, img->height);
}


void
display_img::clear_cur_selection( GtkWidget *widget )
{
	bbox_t bbox;

	GUI_THREAD_CHECK();
	COORDS_TO_BBOX(bbox);

	image_fill_bbox_scale(di_layers[DI_SELECT_LAYER], &bbox, 1,
	                      colorMask, clearColor);

	/* refresh */
	gtk_widget_queue_draw_area(di_drawingarea,
	                           bbox.min_x, bbox.min_y,
	                           bbox.max_x - bbox.min_x + 1,
	                           bbox.max_y - bbox.min_y + 1);

}

void
display_img::redraw_selections()
{

	GUI_THREAD_CHECK();

	pthread_mutex_lock(&di_mutex);
	RGBImage *img = di_layers[DI_SELECT_LAYER];

	rgbimg_clear(img);

	for(int i=0; i<di_nselections; i++) {
		image_fill_bbox_scale(img, &di_selections[i],
		                      1, hilitMask, hilitRed);
		image_draw_bbox_scale(img, &di_selections[i],
		                      1, colorMask, red);
	}
	pthread_mutex_unlock(&di_mutex);

	gtk_widget_queue_draw_area(di_drawingarea,
	                           0, 0, img->width, img->height);
}

void
display_img::draw_selection(GtkWidget *widget)
{
	bbox_t bbox;

	GUI_THREAD_CHECK();
	COORDS_TO_BBOX(bbox)

	pthread_mutex_lock(&di_mutex);
	image_fill_bbox_scale(di_layers[DI_SELECT_LAYER], &bbox, 1,
	                      hilitMask, hilitRed);
	image_draw_bbox_scale(di_layers[DI_SELECT_LAYER], &bbox, 1,
	                      colorMask, red);
	pthread_mutex_unlock(&di_mutex);

	/* refresh */
	gtk_widget_queue_draw_area(di_drawingarea,
	                           bbox.min_x, bbox.min_y,
	                           bbox.max_x - bbox.min_x + 1,
	                           bbox.max_y - bbox.min_y + 1);


}


static gboolean
cb_button_press_event(GtkWidget      *widget, GdkEventButton *event )
{
	display_img * dimg;
	dimg = (display_img *)
	       gtk_object_get_user_data(GTK_OBJECT(widget));
	dimg->button_press(widget, event);
	return TRUE;
}

void
display_img::button_press(GtkWidget *widget, GdkEventButton *event )
{

	GUI_CALLBACK_ENTER();

	if (event->button == 1) {
		x1 = (int)event->x;
		y1 = (int)event->y;
		x2 = (int)event->x;
		y2 = (int)event->y;
		di_button_down = 1;
	}

	GUI_CALLBACK_LEAVE();
	return;
}


static gboolean
cb_button_release_event(GtkWidget* widget, GdkEventButton *event)
{
	display_img * dimg;
	dimg = (display_img *)
	       gtk_object_get_user_data(GTK_OBJECT(widget));
	dimg->button_release(widget, event);
	return TRUE;
}

void
display_img::button_release(GtkWidget* widget, GdkEventButton *event)
{
	GUI_CALLBACK_ENTER();

	if (event->button != 1) {
		goto done;
	}

	if (di_button_down != 1) {
		goto done;
	}

	x2 = (int)event->x;
	y2 = (int)event->y;
	di_button_down = 0;

	draw_selection(widget);

	// XXX gtk_widget_grab_focus(import_window.select_button);


	if (di_nselections >= MAX_SELECT) {
		di_nselections--;	/* overwrite last one */
	}
	bbox_t bbox;
	COORDS_TO_BBOX(bbox);
	img_constrain_bbox(&bbox, di_cur_img);

	/* look for cases where box is too small */
	if ((bbox.max_x - bbox.min_x) < MIN_DIMENSION)
		goto done;
	if ((bbox.max_y - bbox.min_y) < MIN_DIMENSION)
		goto done;

	di_selections[di_nselections++] = bbox;

done:
	redraw_selections();
	GUI_CALLBACK_LEAVE();
	return;
}

static gboolean
cb_motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	display_img * dimg;
	dimg = (display_img *)
	       gtk_object_get_user_data(GTK_OBJECT(widget));
	dimg->motion_notify(widget, event);
	return TRUE;
}


void
display_img::set_image(RGBImage *img)
{

	RGBImage *nimg;
	int 	scale;
	int	i;

	scale = (int)ceil(compute_scale(img, di_width, di_height));
	nimg = image_gen_image_scale(img, scale);

	pthread_mutex_lock(&di_mutex);
	di_cur_img = nimg;
	di_button_down = 0;

	release_rgb_image(di_layers[DI_IMG_LAYER]);
	di_layers[DI_IMG_LAYER] = di_cur_img;
	for(i=DI_IMG_LAYER+1; i<DI_MAX_LAYERS; i++) {
		release_rgb_image(di_layers[i]);
		di_layers[i] = rgbimg_new(di_cur_img);
		rgbimg_clear(di_layers[i]);
	}
	pthread_mutex_unlock(&di_mutex);

	/* clean up old realize state */
	event_realize();

	gtk_widget_queue_draw_area(di_drawingarea,
	                           0, 0, di_width, di_height);

}

void
display_img::reset_display()
{
	int	i;

	pthread_mutex_lock(&di_mutex);
	di_cur_img = rgbimg_blank_image(di_width, di_height);
	di_button_down = 0;

	release_rgb_image(di_layers[DI_IMG_LAYER]);
	di_layers[DI_IMG_LAYER] = di_cur_img;
	for(i=DI_IMG_LAYER+1; i<DI_MAX_LAYERS; i++) {
		release_rgb_image(di_layers[i]);
		di_layers[i] = rgbimg_new(di_cur_img);
		rgbimg_clear(di_layers[i]);
	}
	pthread_mutex_unlock(&di_mutex);

	/* clean up old realize state */
	event_realize();

	gtk_widget_queue_draw_area(di_drawingarea,
	                           0, 0, di_width, di_height);

}


void
display_img::motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	GdkModifierType state;

	GUI_CALLBACK_ENTER();

	if (event->is_hint) {
		gdk_window_get_pointer (event->window, &x, &y, &state);
	} else {
		x = (int)event->x;
		y = (int)event->y;
		state = (GdkModifierType)event->state;
	}

	if (state & GDK_BUTTON1_MASK && di_button_down) {
		clear_cur_selection(widget);
		x2 = x;
		y2 = y;

		//draw_brush(widget);
		draw_selection(widget);
	}

	GUI_CALLBACK_LEAVE();
	return;
}




GtkWidget *
display_img::get_display()
{
	GtkWidget *eb;
#ifdef	SCROLL

	GtkWidget *scroll;
#endif

	int		i;

	di_image_area = gtk_viewport_new(NULL, NULL);

	/* open the and create RGB image */
	di_nselections = 0;

	di_cur_img = rgbimg_blank_image(di_width, di_height);
	di_button_down = 0;

	pthread_mutex_lock(&di_mutex);
	di_layers[DI_IMG_LAYER] = di_cur_img;
	for(i=DI_IMG_LAYER+1; i<DI_MAX_LAYERS; i++) {
		di_layers[i] = rgbimg_new(di_cur_img);
		rgbimg_clear(di_layers[i]);
	}
	pthread_mutex_unlock(&di_mutex);

	di_drawingarea = gtk_drawing_area_new();
	GTK_WIDGET_UNSET_FLAGS(di_drawingarea, GTK_CAN_DEFAULT);
	gtk_object_set_user_data(GTK_OBJECT(di_drawingarea),
	                         (void *)this);
	gtk_drawing_area_size(GTK_DRAWING_AREA(di_drawingarea),
	                      di_width, di_height);
	gtk_signal_connect(GTK_OBJECT(di_drawingarea), "expose-event",
	                   GTK_SIGNAL_FUNC(expose_event), (void *)this);
	gtk_signal_connect(GTK_OBJECT(di_drawingarea), "realize",
	                   GTK_SIGNAL_FUNC(cb_realize_event), (void*)this);


	eb = gtk_event_box_new();
	gtk_object_set_user_data(GTK_OBJECT(eb), (void *)this);
	gtk_container_add(GTK_CONTAINER(eb), di_drawingarea);
	gtk_widget_show(eb);

	/* additional events for selection */
	g_signal_connect(G_OBJECT(eb), "motion_notify_event",
	                 G_CALLBACK(cb_motion_notify_event), NULL);
	g_signal_connect(G_OBJECT (eb), "button_press_event",
	                 G_CALLBACK (cb_button_press_event), NULL);
	g_signal_connect(G_OBJECT (eb), "button_release_event",
	                 G_CALLBACK (cb_button_release_event), NULL);
	gtk_widget_set_events (eb, GDK_EXPOSURE_MASK
	                       | GDK_LEAVE_NOTIFY_MASK
	                       | GDK_BUTTON_PRESS_MASK
	                       | GDK_POINTER_MOTION_MASK
	                       | GDK_POINTER_MOTION_HINT_MASK);

#ifdef	SCROLL

	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(
	    GTK_SCROLLED_WINDOW(scroll), eb);
	gtk_container_add(GTK_CONTAINER(di_image_area), scroll);
#else

	gtk_container_add(GTK_CONTAINER(di_image_area), eb);
#endif

	gtk_widget_show_all(di_image_area);
	return(di_image_area);
}
