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
#include <getopt.h>

#include "lib_filter.h"
#include "lib_searchlet.h"

#include "gui_thread.h"

#include "queue.h"
#include "ring.h"

#include "lib_results.h"
#include "lib_sfimage.h"
#include "rgb.h"
#include "img_search.h"
#include "search_support.h"
#include "gtk_image_tools.h"
#include "search_set.h"
#include "graph_win.h"


#define COORDS_TO_BBOX(bbox) \
{	\
  bbox.min_x = min(x1, x2); \
  bbox.max_x = max(x1,x2); \
  bbox.min_y = min(y1, y2); \
  bbox.max_y = max(y1, y2); \
}

#define	MIN_DIMENSION	10



graph_win::graph_win(const double xmin, const double xmax, const double ymin, const double ymax)
{

	gw_xmin = xmin;
	gw_xmax = xmax;
	gw_xspan = xmax - xmin;
	gw_orig_xmax = xmax;

	gw_ymin = ymin;
	gw_ymax = ymax;
	gw_yspan = ymax - ymin;
	gw_orig_ymax = ymax;

	gw_active_win = 0;

}


graph_win::~graph_win()
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
	graph_win *gwin;
	gwin = (graph_win *)
	       gtk_object_get_user_data(GTK_OBJECT(widget));
	gwin->process_expose(widget, event);
	return(TRUE);
}

void
graph_win::process_expose(GtkWidget *widget, GdkEventExpose *event)
{
	int width, height;

	width = min(event->area.width, gw_cur_img->width - event->area.x);
	height = min(event->area.height, gw_cur_img->height - event->area.y);

	if(width <= 0 || height <= 0) {
		goto done;
	}
	assert(widget == gw_drawingarea);

	gdk_window_clear_area(widget->window,
	                      event->area.x, event->area.y,
	                      event->area.width, event->area.height);
	gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
	                           &event->area);

	gdk_draw_drawable(widget->window,
	                  gw_drawingarea->style->fg_gc[GTK_WIDGET_STATE(gw_drawingarea)],
	                  gw_pixmap, event->area.x, event->area.y,
	                  event->area.x, event->area.y, width, height);

	gdk_gc_set_clip_rectangle(widget->style->fg_gc[widget->state],
	                          NULL);

done:
	return;
}


/* draw all the bounding boxes */
void
graph_win::draw_res(GtkWidget *widget)
{


	/* although we clear the pixbuf data here, we still need to
	 * generate refreshes for either the whole image or the parts
	 * that we cleared. */
	rgbimg_clear(gw_layers[GW_RES_LAYER]);
}


static gboolean
cb_realize_event(GtkWidget *widget, GdkEventAny *event, gpointer data)
{
	graph_win *dimg;

	dimg = (graph_win *)
	       gtk_object_get_user_data(GTK_OBJECT(widget));
	dimg->event_realize();
	return(TRUE);
}



void
graph_win::get_series_color(int i, GdkColor *color)
{
	switch(i) {
		case 0:
			/* first is red */
			color->pixel = 0;
			color->red = 65535;
			color->green = 0;
			color->blue = 0;
			break;

		case 1:
			/* second is green */
			color->pixel = 0;
			color->red = 0;
			color->green = 65535;
			color->blue = 0;
			break;

		case 2:
			/* third is blue */
			color->pixel = 0;
			color->red = 0;
			color->green = 0;
			color->blue = 65535;
			break;

		case 3:
			/* fourth is red + green */
			color->pixel = 0;
			color->red = 65535;
			color->green = 65535;
			color->blue = 0;
			break;

		case 4:
			/* fifth is red + blue */
			color->pixel = 0;
			color->red = 65535;
			color->green = 0;
			color->blue = 65535;
			break;

		case 5:
			/* sixth is green + blue */
			color->pixel = 0;
			color->red = 0;
			color->green = 65535;
			color->blue = 65535;
			break;

		case 6:
			/* seventh is gray */
			color->pixel = 0;
			color->red = 32000;
			color->green = 32000;
			color->blue = 32000;
			break;

		case 7:
			/* Eighth is black */
			color->pixel = 0;
			color->red = 0;
			color->green = 0;
			color->blue = 0;
			break;

		default:
			assert(0);
			break;
	}
}

void
graph_win::init_series()
{
	GdkColormap	*cmap;
	int			i;

	for (i = 0; i < GW_MAX_SERIES; i++) {
		gw_series[i].gc = gdk_gc_new(gw_pixmap);
		cmap = gdk_gc_get_colormap(gw_series[i].gc);

		/* set the color and allocate it from the color map */
		get_series_color(i, &gw_series[i].color);
		gdk_colormap_alloc_color(cmap, &gw_series[i].color, TRUE, TRUE);


		/* set the gc attributes for the forground, background, and lines */
		gdk_gc_set_foreground(gw_series[i].gc, &gw_series[i].color);
		gdk_gc_set_line_attributes(gw_series[i].gc, 2, GDK_LINE_SOLID,
		                           GDK_CAP_BUTT, GDK_JOIN_MITER);

		gw_series[i].lastx = 0 + X_ZERO_OFFSET;
		gw_series[i].lasty = gw_height - Y_ZERO_OFFSET;

		/* XXX deal with datastructure to keep the points */
	}
}

void
graph_win::redraw_series()
{
	int	i;
	point_iter_t	iter;

	init_window();

	for (i=0; i < GW_MAX_SERIES; i++) {
		gw_series[i].lastx = 0 + X_ZERO_OFFSET;
		gw_series[i].lasty = gw_height - Y_ZERO_OFFSET;
		for (iter = gw_series[i].points.begin();
		     iter != gw_series[i].points.end(); iter++) {
			draw_point(iter->x, iter->y, i);
		}
	}
	gtk_widget_queue_draw_area(gw_drawingarea, 0, 0, gw_width, gw_height);
}


void
graph_win::clear_series(int series)
{

	/* erase the points in this series */
	gw_series[series].points.erase(gw_series[series].points.begin(),
	                               gw_series[series].points.end());

	redraw_series();
}

void
graph_win::scale_x_up(const double x)
{
	double newx;
	double mult;

	assert(x > gw_xmax);
	newx = x/0.75;

	mult = newx/gw_orig_xmax;
	newx = ceil(mult) * gw_orig_xmax;

	gw_xmax = newx;
	gw_xspan = gw_xmax - gw_xmin;

	redraw_series();
}

void
graph_win::scale_y_up(const double y)
{
	double newy;
	double mult;

	assert(y > gw_ymax);

	newy = y/0.75;

	mult = newy/gw_orig_ymax;
	newy = ceil(mult) * gw_orig_ymax;

	gw_ymax = newy;
	gw_yspan = gw_ymax - gw_ymin;

	redraw_series();
}


void
graph_win::add_point(const double x, const double y, int series)
{
	point_t		point;

	assert(series < GW_MAX_SERIES);

	if (x >  gw_xmax) {
		scale_x_up(x);
	}

	if (y > gw_ymax) {
		scale_y_up(y);
	}

	point.x = x;
	point.y = y;
	gw_series[series].points.push_back(point);

	draw_point(x, y, series);

	gtk_widget_queue_draw_area(gw_drawingarea, 0, 0, gw_width, gw_height);
}



void
graph_win::draw_point(const double x, const double y, int series)
{
	int pixelx, pixely;

	assert(series < GW_MAX_SERIES);

	pixelx = (int)(((x - gw_xmin) /gw_xspan) * (double)gw_xdisp) +
	         X_ZERO_OFFSET ;

	pixely = (int)(((gw_ymax - y)/gw_yspan) * (double)gw_ydisp) +
	         Y_END_OFFSET;

	if (pixelx > (gw_width - X_END_OFFSET)) {
		pixelx = gw_width - X_END_OFFSET;
	} else if (pixelx < X_ZERO_OFFSET) {
		pixelx = X_ZERO_OFFSET;
	}


	if (pixely > (gw_height - Y_ZERO_OFFSET)) {
		pixely = gw_height - Y_ZERO_OFFSET;
	} else if (pixely < Y_END_OFFSET) {
		pixely = Y_END_OFFSET;
	}

	gdk_draw_line(GDK_DRAWABLE(gw_pixmap), gw_series[series].gc,
	              gw_series[series].lastx, gw_series[series].lasty, pixelx, pixely);

	gw_series[series].lastx = pixelx;
	gw_series[series].lasty = pixely;

}

void
graph_win::init_window()
{
	GdkColor	color;
	GdkGC *		gc;
	GdkColormap	*cmap;
	PangoLayout *	playout;
	char		buf[20];

	/* Get the graphics context and color maps for later */
	gc = gdk_gc_new(gw_pixmap);
	cmap = gdk_gc_get_colormap(gc);

	/* create a white color */
	color.pixel = 0;
	color.red = 65535;
	color.green = 65535;
	color.blue = 65535;
	gdk_colormap_alloc_color(cmap, &color, TRUE, TRUE);


	/* set the colors in the graphics context */
	gdk_gc_set_foreground(gc, &color);
	gdk_gc_set_background(gc, &color);

	/* draw a rectangle to cover the image (to clear it) */
	gdk_draw_rectangle(gw_pixmap, gc, 1, 0, 0, gw_width, gw_height);


	/* create a black color */
	color.pixel = 0;
	color.red = 0;
	color.green = 0;
	color.blue = 0;
	gdk_colormap_alloc_color(cmap, &color, TRUE, TRUE);
	/* set the colors in the graphics context */
	gdk_gc_set_foreground(gc, &color);

	/* x zero label */
	sprintf(buf, "%5.2f", gw_xmin);
	playout = gtk_widget_create_pango_layout(gw_drawingarea, buf);
	gdk_draw_layout(gw_pixmap, gc, (X_ZERO_OFFSET - X_FIRST_TEXT_OFFSET),
	                gw_height - (Y_ZERO_OFFSET - Y_TEXT_GAP), playout);

	/* x max label */
	sprintf(buf, "%5.2f", gw_xmax);
	playout = gtk_widget_create_pango_layout(gw_drawingarea, buf);
	gdk_draw_layout(gw_pixmap, gc,
	                gw_width - (X_END_OFFSET + X_END_TEXT_OFFSET),
	                gw_height - (Y_ZERO_OFFSET - Y_TEXT_GAP), playout);

	/* y min label */
	sprintf(buf, "%5.2f", gw_ymin);
	playout = gtk_widget_create_pango_layout(gw_drawingarea, buf);
	gdk_draw_layout(gw_pixmap, gc, Y_LABEL_GAP,
	                gw_height - (Y_ZERO_OFFSET + Y_ZERO_TEXT_OFFSET),playout);

	/* y max label */
	sprintf(buf, "%5.2f", gw_ymax);
	playout = gtk_widget_create_pango_layout(gw_drawingarea, buf);
	gdk_draw_layout(gw_pixmap, gc, Y_LABEL_GAP,
	                Y_END_OFFSET, playout);

	/* x axis label */
	sprintf(buf, "%s", "Time (secs)");
	playout = gtk_widget_create_pango_layout(gw_drawingarea, buf);
	gdk_draw_layout(gw_pixmap, gc, (gw_width/2 - 20),
	                gw_height - (Y_ZERO_OFFSET - Y_TEXT_GAP), playout);

	/* y axis label */
	sprintf(buf, "%s", "  Objs \nSearched");
	playout = gtk_widget_create_pango_layout(gw_drawingarea, buf);
	gdk_draw_layout(gw_pixmap, gc, Y_LABEL_GAP, ((gw_height/2) - 10),
	                playout);



	/* draw x - axis line */
	gdk_draw_line(GDK_DRAWABLE(gw_pixmap), gc, X_ZERO_OFFSET,
	              gw_height - Y_ZERO_OFFSET, gw_width - X_END_OFFSET,
	              gw_height - Y_ZERO_OFFSET);

	/* draw y - axis line */
	gdk_draw_line(GDK_DRAWABLE(gw_pixmap), gc, X_ZERO_OFFSET,
	              gw_height - Y_ZERO_OFFSET, X_ZERO_OFFSET, Y_END_OFFSET);
}



void
graph_win::event_realize()
{


	printf("event realize \n");
	for(int i=0; i<GW_MAX_LAYERS; i++) {
		gw_pixbufs[i] = pb_from_img(gw_layers[i]);
	}

	gw_pixmap = gdk_pixmap_new(gw_drawingarea->window, gw_width, gw_height, -1);

	init_window();
	init_series();

}



void
graph_win::clear_graph()
{
	RGBImage *img;
	img = gw_layers[GW_HIGHLIGHT_LAYER];
	rgbimg_clear(img);
	gtk_widget_queue_draw_area(gw_drawingarea,
	                           0, 0, img->width, img->height);
}

GtkWidget *
graph_win::get_graph_display(int width, int height)
{
	GtkWidget *eb;
	int		i;


	gw_height = height;
	gw_width = width;
	gw_xdisp = width - (X_ZERO_OFFSET + X_END_OFFSET);
	gw_ydisp = height - (Y_ZERO_OFFSET + Y_END_OFFSET);

	gw_image_area = gtk_viewport_new(NULL, NULL);
	gw_cur_img = rgbimg_blank_image(gw_width, gw_height);


	gw_layers[GW_IMG_LAYER] = gw_cur_img;
	for(i=GW_IMG_LAYER+1; i<GW_MAX_LAYERS; i++) {
		gw_layers[i] = rgbimg_new(gw_cur_img);
		rgbimg_clear(gw_layers[i]);
	}

	gw_drawingarea = gtk_drawing_area_new();

	GTK_WIDGET_UNSET_FLAGS(gw_drawingarea, GTK_CAN_DEFAULT);
	gtk_object_set_user_data(GTK_OBJECT(gw_drawingarea),
	                         (void *)this);
	gtk_drawing_area_size(GTK_DRAWING_AREA(gw_drawingarea), gw_width, gw_height);
	gtk_signal_connect(GTK_OBJECT(gw_drawingarea), "expose-event",
	                   GTK_SIGNAL_FUNC(expose_event), (void *)this);
	gtk_signal_connect(GTK_OBJECT(gw_drawingarea), "realize",
	                   GTK_SIGNAL_FUNC(cb_realize_event), (void*)this);

	eb = gtk_event_box_new();
	gtk_object_set_user_data(GTK_OBJECT(eb), (void *)this);
	gtk_container_add(GTK_CONTAINER(eb), gw_drawingarea);
	gtk_widget_show(eb);

	gtk_widget_set_events (eb, GDK_EXPOSURE_MASK);
	gtk_container_add(GTK_CONTAINER(gw_image_area), eb);
	gtk_widget_show_all(gw_image_area);
	return(gw_image_area);
}
