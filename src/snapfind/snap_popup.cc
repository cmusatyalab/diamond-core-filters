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
#include <getopt.h>

#include "filter_api.h"
#include "searchlet_api.h"
#include "gui_thread.h"

#include "queue.h"
#include "ring.h"
#include "rtimer.h"
#include "sf_consts.h"

#include "lib_results.h"
#include "face_search.h"
#include "face_image.h"
#include "rgb.h"
#include "fil_tools.h"
#include "image_tools.h"
#include "face_widgets.h"
#include "texture_tools.h"
#include "img_search.h"
#include "sfind_search.h"
#include "search_support.h"
#include "histo.h"
#include "sfind_tools.h"
#include "snap_popup.h"
#include "snapfind.h"
#include "search_set.h"

/* XXX fix this */
static search_set *sset;

/*
 * global state used for highlighting (running filters locally)
 */
static struct
{
	pthread_mutex_t mutex;
	int 		thread_running;
	pthread_t 	thread;
}
highlight_info = { PTHREAD_MUTEX_INITIALIZER, 0 };

/* forward function declarations */

static void kill_highlight_thread(int run);
static void draw_face_func(GtkWidget *widget, void *ptr);
static void draw_hbbox_func(GtkWidget *widget, void *ptr);


static inline int
min(int a, int b)
{
	return ( (a < b) ? a : b );
}
static inline int
max(int a, int b)
{
	return ( (a > b) ? a : b );
}


/*
 * make pixbuf from img
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



/*
 * draw a bounding box into image at scale. bbox is read from object(!)
 */
region_t
draw_bounding_box(RGBImage *img, int scale,
                  ls_obj_handle_t ohandle,
                  RGBPixel color, RGBPixel mask, char *fmt, int i)
{
	search_param_t 	param;
	int 		err;
	bbox_t		bbox;

	err = read_param(ohandle, fmt, &param, i);

	bbox.min_x = param.bbox.xmin;
	bbox.min_y = param.bbox.ymin;
	bbox.max_x = param.bbox.xmin + param.bbox.xsiz - 1;
	bbox.max_y = param.bbox.ymin + param.bbox.ysiz - 1;

	if (err) {
		//printf("XXXX failed to get bbox %d\n", i);
	} else {
		image_draw_bbox_scale(img, &bbox, scale, mask, color);
		//image_fill_bbox_scale(img, &bbox, scale, mask, color);
	}

	return param.bbox;
}

static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	RGBImage *img = (RGBImage *)data;
	int width, height;

	width = min(event->area.width, img->width - event->area.x);
	height = min(event->area.height, img->height - event->area.y);

	if(width <= 0 || height <= 0) {
		goto done;
	}
	assert(widget == popup_window.drawing_area);

	gdk_window_clear_area(widget->window,
	                      event->area.x, event->area.y,
	                      event->area.width, event->area.height);
	gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
	                           &event->area);

	for(int i=0; i<MAX_LAYERS; i++) {
		int pht = gdk_pixbuf_get_height(popup_window.pixbufs[i]);
		assert(event->area.y + height <= pht);
		assert(width >= 0);
		assert(height >= 0);
		gdk_pixbuf_render_to_drawable_alpha(popup_window.pixbufs[i],
		                                    widget->window,
		                                    event->area.x, event->area.y,
		                                    event->area.x, event->area.y,
		                                    width, height,
		                                    GDK_PIXBUF_ALPHA_FULL, 1, /* ignored */
		                                    GDK_RGB_DITHER_MAX,
		                                    0, 0);
	}

	gdk_gc_set_clip_rectangle(widget->style->fg_gc[widget->state],
	                          NULL);

done:
	return TRUE;
}



/* draw all the bounding boxes */
static void
cb_draw_res_layer(GtkWidget *widget, gpointer ptr)
{

	GUI_CALLBACK_ENTER();

	/* although we clear the pixbuf data here, we still need to
	 * generate refreshes for either the whole image or the parts
	 * that we cleared. */
	rgbimg_clear(popup_window.layers[RES_LAYER]);

	/* draw faces (presumably there's only one checkbox, but that's ok) */
	gtk_container_foreach(GTK_CONTAINER(popup_window.face_cb_area), draw_face_func,
	                      popup_window.face_cb_area);

	/* draw histo bboxes, if on */
	gtk_container_foreach(GTK_CONTAINER(popup_window.histo_cb_area),
	                      draw_hbbox_func, popup_window.histo_cb_area);

	GUI_CALLBACK_LEAVE();
}


static GtkWidget *
describe_hbbox(ls_obj_handle_t ohandle, int i,
               GtkWidget **button)
{
	search_param_t 	param;
	int 		err;

	GUI_THREAD_CHECK();

	err = read_param(ohandle, HISTO_BBOX_FMT, &param, i);
	if (err) {
		printf("XXX failed to read parameter <%s> \n", HISTO_BBOX_FMT);
		/* 		label = gtk_label_new("ERR"); */
		/* 		gtk_box_pack_start(GTK_BOX(container), label, TRUE, TRUE, 0); */
		/* 		gtk_widget_show(label); */
	} else {
		char buf[BUFSIZ];

		if(param.type == PARAM_HISTO) {
			sprintf(buf, "%s (similarity %.0f%%)", param.name,
			        100 - 100.0*param.distance);
			*button = gtk_check_button_new_with_label(buf);
			g_signal_connect (G_OBJECT(*button), "toggled",
			                  G_CALLBACK(cb_draw_res_layer), GINT_TO_POINTER(i));
			gtk_object_set_user_data(GTK_OBJECT(*button), GINT_TO_POINTER(i));


			gtk_widget_show(*button);
		} else {
			printf("param type not histo !!! %d \n", param.type);
		}
	}
	return *button;
}




static gboolean
realize_event(GtkWidget *widget, GdkEventAny *event, gpointer data)
{

	assert(widget == popup_window.drawing_area);

	for(int i=0; i<MAX_LAYERS; i++) {
		popup_window.pixbufs[i] = pb_from_img(popup_window.layers[i]);
	}

	return TRUE;
}



static void
draw_hbbox_func(GtkWidget *widget, void *ptr)
{
	int i = GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(widget)));
	region_t region;
	RGBPixel mask = colorMask;
	RGBPixel color = green;

	GUI_THREAD_CHECK();

	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		/* don't draw, but still need to refresh */
		mask = clearMask;

		/* can't draw clear, lest we wipe out overlapping box */
		//color = clearColor;
	}

	region = draw_bounding_box(popup_window.layers[RES_LAYER], 1,
	                           popup_window.hooks->ohandle,
	                           color, mask, HISTO_BBOX_FMT, i);

	/* refresh */
	gtk_widget_queue_draw_area(popup_window.drawing_area,
	                           region.xmin, region.ymin,
	                           region.xsiz, region.ysiz);
}

static void
draw_face_func(GtkWidget *widget, void *ptr)
{
	region_t region;
	RGBPixel mask = colorMask;
	RGBPixel color = red;
	int num_faces = popup_window.nfaces;

	/* draw faces, if on */
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		/* don't draw, but still need to refresh */
		mask = clearMask;
	}

	for(int i=0; i<num_faces; i++) {
		region = draw_bounding_box(popup_window.layers[RES_LAYER], 1, 
		                           popup_window.hooks->ohandle,
		                           color, mask, FACE_BBOX_FMT, i);
		/* refresh */
		gtk_widget_queue_draw_area(popup_window.drawing_area,
		                           region.xmin, region.ymin,
		                           region.xsiz, region.ysiz);

	}

}

void *
image_highlight_main(void *ptr)
{
	bbox_t *		cur_bb;
	bbox_list_t		bblist;
	char 			buf[BUFSIZ];
	RGBImage *		hl_img;
	RGBPixel mask = colorMask;
	RGBPixel color = red;
	search_iter_t		iter;
	guint			id;
	int				use_box;
	img_search *	csearch;
	int err;

	err = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	assert(!err);
	err = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	assert(!err);

	ih_get_ref(popup_window.hooks);
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(popup_window.statusbar), "histo");

	/* look at toggle button to see if we use the boxes or shading */
	use_box =
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(popup_window.drawbox));

	/*
	 * go through each of the searches, if highlight is selected,
	 * then eval the regions according to the values.
	 */


	hl_img = popup_window.layers[HIGHLIGHT_LAYER];
	rgbimg_clear(hl_img);

	GUI_THREAD_ENTER();
	gtk_widget_queue_draw_area(popup_window.drawing_area, 0, 0,
	                           hl_img->width, hl_img->height);
	GUI_THREAD_LEAVE();

	popup_window.nselections = 0;

	sset->reset_search_iter(&iter);
	while ((csearch = sset->get_next_search(&iter)) != NULL) {
		/* if highligh isn't selected, then go to next object */
		if (csearch->is_hl_selected() == 0) {
			continue;
		}

		TAILQ_INIT(&bblist);

		snprintf(buf, BUFSIZ, "scanning %s ...", csearch->get_name());
		buf[BUFSIZ - 1] = '\0';
		GUI_THREAD_ENTER();
		gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, buf);
		GUI_THREAD_LEAVE();

		csearch->region_match(popup_window.hooks->img, &bblist);

		snprintf(buf, BUFSIZ, "highlighting %s", csearch->get_name());
		buf[BUFSIZ - 1] = '\0';
		GUI_THREAD_ENTER();
		gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, buf);
		GUI_THREAD_LEAVE();


		/* for each bounding box, draw region on the overlay */
		TAILQ_FOREACH(cur_bb, &bblist, link) {
			if (use_box) {
				image_draw_bbox_scale(hl_img, cur_bb, 1, mask, color);
			} else {
				image_fill_bbox_scale(hl_img, cur_bb, 1, hilitMask, hilit);
			}
			TAILQ_REMOVE(&bblist, cur_bb, link);
			free(cur_bb);
		}

		/* ask the windowing system to redraw the affected region */
		GUI_THREAD_ENTER();
		gtk_widget_queue_draw_area(popup_window.drawing_area, 0, 0,
		                           hl_img->width, hl_img->height);
		GUI_THREAD_LEAVE();

		snprintf(buf, BUFSIZ, "done %s", csearch->get_name());
		buf[BUFSIZ - 1] = '\0';
		GUI_THREAD_ENTER();
		gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, buf);
		GUI_THREAD_LEAVE();
	}

	/* update statusbar */ snprintf(buf, BUFSIZ, "Highlight complete");
	buf[BUFSIZ - 1] = '\0';
	GUI_THREAD_ENTER();
	gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, buf);
	GUI_THREAD_LEAVE();

	pthread_mutex_lock(&highlight_info.mutex);
	highlight_info.thread_running = 0;
	ih_drop_ref(popup_window.hooks);
	pthread_mutex_unlock(&highlight_info.mutex);

	pthread_exit(NULL);
	return NULL;
}






static void
remove_func(GtkWidget *widget, void *container)
{
	GUI_THREAD_CHECK();
	gtk_container_remove(GTK_CONTAINER(container), widget);
}



static void
kill_highlight_thread(int run)
{

	GUI_THREAD_CHECK();

	pthread_mutex_lock(&highlight_info.mutex);
	if(highlight_info.thread_running) {
		int err = pthread_cancel(highlight_info.thread);
		//assert(!err);
		if(!err) {
			/* should do this in a cleanup function XXX */
			ih_drop_ref(popup_window.hooks);
			highlight_info.thread_running = 0;
			/* child should not be inside gui, since we
			 * are in a callback here, and presumable own
			 * the lock. */
			//GUI_THREAD_LEFT(); /* XXX */
			pthread_join(highlight_info.thread, NULL);
		}
	}
	highlight_info.thread_running = run;
	pthread_mutex_unlock(&highlight_info.mutex);
}


static void
cb_clear_select(GtkWidget *widget, GdkEventButton *event, gpointer ptr)
{
	RGBImage *img;

	GUI_CALLBACK_ENTER();

	img = popup_window.layers[SELECT_LAYER];

	rgbimg_clear(img);
	gtk_widget_queue_draw_area(popup_window.drawing_area, 0, 0, img->width,
	                           img->height);
	GUI_CALLBACK_LEAVE();

	popup_window.nselections = 0;
}

static void
cb_clear_highlight_layer(GtkWidget *widget, GdkEventButton *event, gpointer ptr)
{
	RGBImage *img;

	GUI_CALLBACK_ENTER();

	kill_highlight_thread(0);

	img = popup_window.layers[HIGHLIGHT_LAYER];

	rgbimg_clear(img);
	gtk_widget_queue_draw_area(popup_window.drawing_area,
	                           0, 0,
	                           img->width,
	                           img->height);

	GUI_CALLBACK_LEAVE();
}

static void
cb_run_highlight()
{

	GUI_CALLBACK_ENTER();

	kill_highlight_thread(1);
	int err = pthread_create(&highlight_info.thread, PATTR_DEFAULT,
	                         image_highlight_main, NULL);
	assert(!err);

	GUI_CALLBACK_LEAVE();
}

static gboolean
cb_add_to_existing(GtkWidget *widget, GdkEventAny *event, gpointer data)
{
	char buf[BUFSIZ] = "created new scene";
	GtkWidget *	active_item;
	img_search *ssearch;
	guint	id;
	GUI_CALLBACK_ENTER();

	active_item = gtk_menu_get_active(GTK_MENU(popup_window.example_list));
	if (active_item == NULL) {
		goto done;
	}

	ssearch = (img_search *)g_object_get_data(G_OBJECT(active_item),
	          "user data");


	for(int i=0; i<popup_window.nselections; i++) {
		ssearch->add_patch(popup_window.hooks->img,
		                   popup_window.selections[i]);
	}

	/* popup the edit window */
	ssearch->edit_search();
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(popup_window.statusbar),
	                                  "selection");
	gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, buf);

done:
	GUI_CALLBACK_LEAVE();
	return TRUE;
}

void
search_popup_add(img_search *ssearch, int nsearch)
{
	GtkWidget *		item;

	/* see if the popup window exists, if not, then just return */
	if (popup_window.window == NULL) {
		return;
	}
	/* Put the list of searches in the ones we can select in the popup menu */
	item = gtk_menu_item_new_with_label(ssearch->get_name());
	gtk_widget_show(item);
	/* XXX change to obj pointer */
	g_object_set_data(G_OBJECT(item), "user data", (void *)(nsearch - 1));
	gtk_menu_shell_append(GTK_MENU_SHELL(popup_window.example_list), item);

	/* XXX deal with other list later */
}



/*
 * The callback function that takes user selected regions and creates
 * a new search with the list.
 */

static gboolean
cb_add_to_new(GtkWidget *widget, GdkEventAny *event, gpointer data)
{
	GtkWidget *	active_item;
	GtkWidget *	dialog;
	GtkWidget *	label;
	img_search *	ssearch;
	img_factory *	factory;
	int		idx;
	gint		result;
	const char *	sname;
	GUI_CALLBACK_ENTER();

	active_item = gtk_menu_get_active(GTK_MENU(popup_window.search_type));

	/* XXX can't directly get the cast to work ??? */
	idx = (int) g_object_get_data(G_OBJECT(active_item), "user data");
	factory = (img_factory *)idx;

	sname =  gtk_entry_get_text(GTK_ENTRY(popup_window.search_name));
	if (strlen(sname) < 1) {
		dialog = gtk_dialog_new_with_buttons("Filter Name",
		                                     GTK_WINDOW(popup_window.window),
		                                     GTK_DIALOG_DESTROY_WITH_PARENT,
		                                     GTK_STOCK_OK, GTK_RESPONSE_NONE, NULL);
		label = gtk_label_new("Please provide a name");
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
		gtk_widget_show_all(dialog);
		result = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		GUI_CALLBACK_LEAVE();
		return(TRUE);
		/* XXX make sure the name already exists */
	}

	/* create the new search and put it in the global list */
	ssearch = factory->create(sname);
	assert(ssearch != NULL);
	sset->add_search(ssearch);


	/* put the patches into the newly created search */
	for(int i=0; i<popup_window.nselections; i++) {
		ssearch->add_patch(popup_window.hooks->img,
		                   popup_window.selections[i]);
	}

	/* popup the edit window */
	ssearch->edit_search();

	GUI_CALLBACK_LEAVE();
	return TRUE;
}



static void
clear_selection( GtkWidget *widget )
{
	bbox_t bbox;

	GUI_THREAD_CHECK();
	COORDS_TO_BBOX(bbox, popup_window);

	image_fill_bbox_scale(popup_window.layers[SELECT_LAYER], &bbox, 1,
	                      colorMask, clearColor);

	/* refresh */
	gtk_widget_queue_draw_area(popup_window.drawing_area,
	                           bbox.min_x, bbox.min_y,
	                           bbox.max_x - bbox.min_x + 1,
	                           bbox.max_y - bbox.min_y + 1);

}

static void
redraw_selections()
{

	GUI_THREAD_CHECK();
	RGBImage *img = popup_window.layers[SELECT_LAYER];

	rgbimg_clear(img);

	for(int i=0; i<popup_window.nselections; i++) {
		image_fill_bbox_scale(popup_window.layers[SELECT_LAYER],
		                      &popup_window.selections[i],
		                      1, hilitMask, hilitRed);
		image_draw_bbox_scale(popup_window.layers[SELECT_LAYER],
		                      &popup_window.selections[i],
		                      1, colorMask, red);
	}

	gtk_widget_queue_draw_area(popup_window.drawing_area,
	                           0, 0,
	                           img->width,
	                           img->height);
}

static void
draw_selection( GtkWidget *widget )
{
	/*   GdkPixmap* pixmap; */
	bbox_t bbox;

	GUI_THREAD_CHECK();
	COORDS_TO_BBOX(bbox, popup_window);

	image_fill_bbox_scale(popup_window.layers[SELECT_LAYER], &bbox, 1, hilitMask, hilitRed);
	image_draw_bbox_scale(popup_window.layers[SELECT_LAYER], &bbox, 1, colorMask, red);

	/* refresh */
	gtk_widget_queue_draw_area(popup_window.drawing_area,
	                           bbox.min_x, bbox.min_y,
	                           bbox.max_x - bbox.min_x + 1,
	                           bbox.max_y - bbox.min_y + 1);


}

static gboolean
cb_button_press_event( GtkWidget      *widget,
                       GdkEventButton *event )
{

	GUI_CALLBACK_ENTER();

	if (event->button == 1) {
		popup_window.x1 = (int)event->x;
		popup_window.y1 = (int)event->y;
		popup_window.x2 = (int)event->x;
		popup_window.y2 = (int)event->y;
		popup_window.button_down = 1;
	}

	GUI_CALLBACK_LEAVE();
	return TRUE;
}

static gboolean
cb_button_release_event(GtkWidget* widget, GdkEventButton *event)
{

	GUI_CALLBACK_ENTER();

	if (event->button != 1) {
		goto done;
	}

	popup_window.x2 = (int)event->x;
	popup_window.y2 = (int)event->y;
	popup_window.button_down = 0;

	draw_selection(widget);

	gtk_widget_grab_focus(popup_window.select_button);


	if(popup_window.nselections >= MAX_SELECT) {
		popup_window.nselections--;	/* overwrite last one */
	}
	bbox_t bbox;
	COORDS_TO_BBOX(bbox, popup_window);
	img_constrain_bbox(&bbox, popup_window.hooks->img);
	popup_window.selections[popup_window.nselections++] = bbox;

	redraw_selections();

done:
	GUI_CALLBACK_LEAVE();
	return TRUE;
}

static gboolean
cb_motion_notify_event( GtkWidget *widget,
                        GdkEventMotion *event )
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

	if (state & GDK_BUTTON1_MASK && popup_window.button_down) {

		clear_selection(widget);
		popup_window.x2 = x;
		popup_window.y2 = y;

		//draw_brush(widget);
		draw_selection(widget);
	}

	GUI_CALLBACK_LEAVE();
	return TRUE;
}

/*
 * Return a gtk_menu with a list of all all the existing
 * searches that use the example class.
 */

GtkWidget *
get_example_menu(void)
{
	GtkWidget *     menu;
	GtkWidget *     item;
	img_search *	cur_search;
	search_iter_t	iter;

	menu = gtk_menu_new();

	sset->reset_search_iter(&iter);
	while ((cur_search = sset->get_next_search(&iter)) != NULL) {
		if (cur_search->is_example() == 0) {
			continue;
		}
		item = gtk_menu_item_new_with_label(cur_search->get_name());
		gtk_widget_show(item);
		g_object_set_data(G_OBJECT(item), "user data", (void *)cur_search);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	}

	return(menu);
}


/* XXX in import .. should be in seperate library ... */
GtkWidget * get_example_searches_menu(void);

static GtkWidget *
new_search_panel(void)
{
	GtkWidget *box;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *widget;

	frame = gtk_frame_new("Create New Search");

	box = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(box), 10);
	gtk_container_add(GTK_CONTAINER(frame), box);

	/*
	 * Create a hbox that has the controls for
	 * adding a new search with examples to the existing
	 * searches.
	 */

	GtkWidget *button = gtk_button_new_with_label ("Create");
	popup_window.select_button = button;
	g_signal_connect_after(GTK_OBJECT(button), "clicked",
	                       GTK_SIGNAL_FUNC(cb_add_to_new), NULL);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, FALSE, 0);


	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, FALSE, 0);

	widget = gtk_label_new("Type");
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);

	popup_window.search_type =  get_example_searches_menu();
	widget = gtk_option_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(widget),
	                         popup_window.search_type);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);


	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, FALSE, 0);

	widget = gtk_label_new("Name");
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);

	popup_window.search_name = gtk_entry_new();
	gtk_entry_set_activates_default(GTK_ENTRY(popup_window.search_name),
	                                TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), popup_window.search_name,
	                   TRUE, FALSE, 0);

	gtk_widget_show_all(GTK_WIDGET(frame));
	return(frame);
}


static GtkWidget *
existing_search_panel(void)
{
	GtkWidget *box;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *widget;

	frame = gtk_frame_new("Add to Existing Search");

	box = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(box), 10);
	gtk_container_add(GTK_CONTAINER(frame), box);

	/*
	 * Create a hbox that has the controls for
	 * adding a examples to the existing searches.
	 */

	GtkWidget *button = gtk_button_new_with_label ("Add");
	popup_window.select_button = button;
	g_signal_connect_after(GTK_OBJECT(button), "clicked",
	                       GTK_SIGNAL_FUNC(cb_add_to_existing), NULL);
	gtk_box_pack_start (GTK_BOX(box), button, TRUE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, FALSE, 0);

	widget = gtk_label_new("Search");
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);

	/* get the menu of the existing searches */
	popup_window.example_list =  get_example_menu();
	popup_window.existing_menu = gtk_option_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(popup_window.existing_menu),
	                         popup_window.example_list);
	gtk_box_pack_start(GTK_BOX(hbox), popup_window.existing_menu,
	                   TRUE, FALSE, 0);

	gtk_widget_show_all(GTK_WIDGET(frame));
	return(frame);
}


static GtkWidget *
make_highlight_table()
{
	GtkWidget *widget;
	int row = 0;        /* current table row */
	img_search *	csearch;
	search_iter_t	iter;

	// XXX GUI_THREAD_CHECK();

	popup_window.hl_table = gtk_table_new(sset->get_search_count()+2, 3, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(popup_window.hl_table), 2);
	gtk_table_set_col_spacings(GTK_TABLE(popup_window.hl_table), 4);
	gtk_container_set_border_width(GTK_CONTAINER(popup_window.hl_table), 10);

	widget = gtk_label_new("Predicate");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(popup_window.hl_table), widget, 0, 1, row, row+1);

	widget = gtk_label_new("Description");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(popup_window.hl_table), widget, 1, 2, row, row+1);

	widget = gtk_label_new("Edit");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(popup_window.hl_table), widget, 2, 3, row, row+1);


	sset->reset_search_iter(&iter);
	while ((csearch = sset->get_next_search(&iter)) != NULL) {
		row++;
		widget = csearch->get_highlight_widget();
		gtk_table_attach_defaults(GTK_TABLE(popup_window.hl_table), widget, 0, 1, row, row+1);
		widget = csearch->get_config_widget();
		gtk_table_attach_defaults(GTK_TABLE(popup_window.hl_table), widget, 1, 2, row, row+1);
		widget = csearch->get_edit_widget();
		gtk_table_attach_defaults(GTK_TABLE(popup_window.hl_table), widget, 2, 3, row, row+1);
	}

	return(popup_window.hl_table);
}


static GtkWidget *
highlight_select()
{
	GtkWidget *box1;

	GUI_THREAD_CHECK();

	box1 = gtk_vbox_new (FALSE, 0);
	popup_window.hl_frame = gtk_frame_new("Searches");
	popup_window.hl_table = make_highlight_table();
	gtk_container_add(GTK_CONTAINER(popup_window.hl_frame), popup_window.hl_table);
	gtk_box_pack_start(GTK_BOX(box1), popup_window.hl_frame, FALSE, FALSE, 10);
	gtk_widget_show_all(box1);

	return(box1);
}


static GtkWidget *
highlight_panel(void)
{
	GtkWidget	*frame;
	GtkWidget	*box;

	frame = gtk_frame_new("Highlighting");

	/* start button area */
	GtkWidget *box2 = gtk_vbox_new (FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
	gtk_container_add(GTK_CONTAINER(frame), box2);

	GtkWidget *label = gtk_label_new("Highlight regions matching seaches");
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_box_pack_start (GTK_BOX (box2), label, FALSE, TRUE, 0);


	box = highlight_select();
	gtk_box_pack_start(GTK_BOX(box2), box, FALSE, TRUE, 0);

	box = gtk_hbox_new(TRUE, 10);
	gtk_box_pack_start(GTK_BOX(box2), box, FALSE, TRUE, 0);

	popup_window.drawbox = gtk_radio_button_new_with_label(NULL, "Outline");
	gtk_box_pack_start(GTK_BOX(box), popup_window.drawbox, FALSE, TRUE, 0);
	popup_window.drawhl = gtk_radio_button_new_with_label_from_widget(
	                          GTK_RADIO_BUTTON(popup_window.drawbox), "Shade");
	gtk_box_pack_start(GTK_BOX(box), popup_window.drawhl, FALSE, TRUE, 0);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(popup_window.drawhl), TRUE);

	/* Buttons to draw/clear the highlighting */
	box = gtk_hbox_new(TRUE, 10);
	gtk_box_pack_start (GTK_BOX(box2), box, TRUE, TRUE, 0);


	GtkWidget *button = gtk_button_new_with_label ("Highlight");
	g_signal_connect_after(GTK_OBJECT(button), "clicked",
	                       GTK_SIGNAL_FUNC(cb_run_highlight), NULL);
	gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

	button = gtk_button_new_with_label ("Clear");
	g_signal_connect_after(GTK_OBJECT(button), "clicked",
	                       GTK_SIGNAL_FUNC(cb_clear_highlight_layer), NULL);
	gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);

	gtk_widget_show_all(frame);
	return(frame);
}

/*
 * callback function that gets called when the list of searches
 * is updated.  This will redraw some of the windows and their
 * associated that depend on the list of exsiting searches.
 */

void
popup_update_searches(search_set *set
                     )
{
	if(popup_window.window == NULL) {
		return;
	}

	/* update the list of existing searches */
	gtk_option_menu_remove_menu(GTK_OPTION_MENU(popup_window.existing_menu));
	popup_window.example_list =  get_example_menu();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(popup_window.existing_menu),
	                         popup_window.example_list);
	gtk_widget_show_all(popup_window.existing_menu);

	/* update the list of searches for high lighting */

	/* update the list of searches for high lighting */
	gtk_container_remove(GTK_CONTAINER(popup_window.hl_frame),
	                     popup_window.hl_table);
	popup_window.hl_table = make_highlight_table();
	gtk_container_add(GTK_CONTAINER(popup_window.hl_frame),
	                  popup_window.hl_table);
	gtk_widget_show_all(popup_window.hl_frame);



}

static void
cb_popup_window_close(GtkWidget *window)
{
	GUI_CALLBACK_ENTER();

	kill_highlight_thread(0);
	sset->un_register_update_fn(popup_update_searches);
	popup_window.window = NULL;
	ih_drop_ref(popup_window.hooks);
	popup_window.hooks = NULL;

	GUI_CALLBACK_LEAVE();
}



void
do_img_popup(GtkWidget *widget, search_set *set
            )
{
	thumbnail_t *thumb;
	GtkWidget *eb;
	GtkWidget *frame;
	GtkWidget *image;
	GtkWidget *button;

	sset = set
		       ;
	thumb = (thumbnail_t *)gtk_object_get_user_data(GTK_OBJECT(widget));

	/* the data gpointer passed in seems to not be the data
	 * pointer we gave gtk. instead, we save a pointer in the
	 * widget. (maybe the prototype didn't match) -RW */

	if(!thumb->img) {
		goto done;
	}

	if(!popup_window.window) {
		popup_window.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(popup_window.window), "Image");
		gtk_window_set_default_size(GTK_WINDOW(popup_window.window), 850, 350);
		g_signal_connect(G_OBJECT(popup_window.window), "destroy",
		                 G_CALLBACK(cb_popup_window_close), NULL);

		GtkWidget *box1 = gtk_vbox_new(FALSE, 0);

		popup_window.statusbar = gtk_statusbar_new();
		gtk_box_pack_end(GTK_BOX(box1), popup_window.statusbar, FALSE, FALSE, 0);
		GtkWidget *pane = gtk_hpaned_new();
		gtk_box_pack_start(GTK_BOX(box1), pane, TRUE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(popup_window.window), box1);

		/* box to hold controls */
		box1 = gtk_vbox_new(FALSE, 10);
		gtk_container_set_border_width(GTK_CONTAINER(box1), 4);
		gtk_paned_pack1(GTK_PANED(pane), box1, FALSE, TRUE);

		frame = gtk_frame_new("Search Results");
		gtk_box_pack_start(GTK_BOX(box1), frame, FALSE, FALSE, 0);
		GtkWidget *box3 = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box3);

		popup_window.face_cb_area = gtk_vbox_new(FALSE, 0);
		popup_window.histo_cb_area = gtk_vbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX (box3),
		                   popup_window.face_cb_area, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX (box3),
		                   popup_window.histo_cb_area, FALSE, FALSE, 0);

		/*
		 * Refinement
		 */
		{
			frame = gtk_frame_new("Search Update");
			gtk_box_pack_end(GTK_BOX(box1), frame, FALSE, FALSE, 0);

			GtkWidget *box2 = gtk_vbox_new (FALSE, 10);
			gtk_container_set_border_width(GTK_CONTAINER(box2), 10);
			gtk_container_add(GTK_CONTAINER(frame), box2);

			/* hbox */
			GtkWidget *hbox = gtk_hbox_new(FALSE, 10);
			gtk_box_pack_start (GTK_BOX(box2), hbox, TRUE, FALSE, 0);

			/* couple of buttons */

			GtkWidget *buttonbox = gtk_vbox_new(FALSE, 10);
			gtk_box_pack_start(GTK_BOX(box2), buttonbox, TRUE, FALSE,0);

			/*
			  * Create a hbox that has the controls for
			  * adding examples to new or existing searches.
			 */

			hbox = gtk_hbox_new(FALSE, 10);
			gtk_box_pack_start(GTK_BOX(buttonbox), hbox, TRUE, FALSE, 0);

			/* control for creating a new search */
			widget = new_search_panel();
			gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);


			/* control for adding to an existing search */
			widget = existing_search_panel();
			gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);

			/* control button to clear selected regions */
			hbox = gtk_hbox_new(FALSE, 10);
			gtk_box_pack_start (GTK_BOX(buttonbox), hbox, TRUE, FALSE, 0);
			button = gtk_button_new_with_label ("Clear");
			g_signal_connect_after(GTK_OBJECT(button), "clicked",
			                       GTK_SIGNAL_FUNC(cb_clear_select), NULL);
			gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 0);

		}

		/* Get highlighting state */
		frame = highlight_panel();
		gtk_box_pack_end (GTK_BOX (box1), frame, FALSE, FALSE, 0);

		popup_window.image_area = gtk_viewport_new(NULL, NULL);
		gtk_paned_pack2(GTK_PANED(pane), popup_window.image_area, TRUE, TRUE);

		gtk_widget_show_all(popup_window.window);

		sset->register_update_fn(popup_update_searches);

	} else {
		kill_highlight_thread(0);
		gtk_container_remove(GTK_CONTAINER(popup_window.image_area),
		                     popup_window.scroll);
		ih_drop_ref(popup_window.hooks);
		gdk_window_raise(GTK_WIDGET(popup_window.window)->window);
	}

	/* note there is a slight race condition here, if the thumb gets overwritten */
	ih_get_ref(thumb->hooks);
	popup_window.hooks = thumb->hooks;
	popup_window.layers[IMG_LAYER] = thumb->hooks->img;
	for(int i=IMG_LAYER+1; i<MAX_LAYERS; i++) {
		popup_window.layers[i] = rgbimg_new(thumb->hooks->img); /* XXX */
	}
	//gtk_object_set_user_data(GTK_OBJECT(popup_window.window), thumb->fullimage);


	char buf[MAX_NAME];
	sprintf(buf, "Image: %s", thumb->name);
	gtk_window_set_title(GTK_WINDOW(popup_window.window), buf);

	image = popup_window.drawing_area = gtk_drawing_area_new();
	GTK_WIDGET_UNSET_FLAGS (image, GTK_CAN_DEFAULT);
	gtk_drawing_area_size(GTK_DRAWING_AREA(image),
	                      popup_window.hooks->img->width,
	                      popup_window.hooks->img->height);
	gtk_signal_connect(GTK_OBJECT(image), "expose-event",
	                   GTK_SIGNAL_FUNC(expose_event), popup_window.hooks->img);
	gtk_signal_connect(GTK_OBJECT(image), "realize",
	                   GTK_SIGNAL_FUNC(realize_event), NULL);

	popup_window.scroll = gtk_scrolled_window_new(NULL, NULL);

	eb = gtk_event_box_new();
	gtk_object_set_user_data(GTK_OBJECT(eb), NULL);
	gtk_container_add(GTK_CONTAINER(eb), image);
	gtk_widget_show(eb);

	/* additional events for selection */
	g_signal_connect (G_OBJECT (eb), "motion_notify_event",
	                  G_CALLBACK (cb_motion_notify_event), NULL);
	g_signal_connect (G_OBJECT (eb), "button_press_event",
	                  G_CALLBACK (cb_button_press_event), NULL);
	g_signal_connect (G_OBJECT (eb), "button_release_event",
	                  G_CALLBACK (cb_button_release_event), NULL);
	gtk_widget_set_events (eb, GDK_EXPOSURE_MASK
	                       | GDK_LEAVE_NOTIFY_MASK
	                       | GDK_BUTTON_PRESS_MASK
	                       | GDK_POINTER_MOTION_MASK
	                       | GDK_POINTER_MOTION_HINT_MASK);

	popup_window.nselections = 0;

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(popup_window.scroll),
	                                      eb);
	gtk_widget_show(image);
	gtk_widget_show(popup_window.scroll);
	gtk_container_add(GTK_CONTAINER(popup_window.image_area), popup_window.scroll);


	/*
	 * add widgets to show search results 
	 */
	gtk_container_foreach(GTK_CONTAINER(popup_window.face_cb_area),
	                      remove_func, popup_window.face_cb_area);
	gtk_container_foreach(GTK_CONTAINER(popup_window.histo_cb_area),
	                      remove_func, popup_window.histo_cb_area);

	{
		GtkWidget *button = NULL;

		popup_window.nfaces = thumb->nfaces;
		if (thumb->nfaces) {
			sprintf(buf, "faces (%d)", thumb->nfaces);
			button = gtk_check_button_new_with_label(buf);
			g_signal_connect (G_OBJECT(button), "toggled",
			                  G_CALLBACK(cb_draw_res_layer), NULL);
			gtk_box_pack_start(GTK_BOX(popup_window.face_cb_area), button,
			                   FALSE, FALSE, 0);
			gtk_widget_show(button);
		}

		for(int i=0; i<thumb->nboxes; i++) {
			GtkWidget *widget;
			widget = describe_hbbox(popup_window.hooks->ohandle,
			                        i, &button);
			gtk_box_pack_start(GTK_BOX(popup_window.histo_cb_area), widget,
			                   FALSE, FALSE, 0);
		}
	}

	gtk_widget_queue_resize(popup_window.window);
	gtk_widget_show(popup_window.window);

done:
	return;
}


