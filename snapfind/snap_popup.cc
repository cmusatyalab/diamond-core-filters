/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  Copyright (c) 2009 Carnegie Mellon University
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

#include "searchlet_api.h"
#include "gui_thread.h"

#include <sys/queue.h>
#include "sf_consts.h"

#include "lib_results.h"
#include "lib_sfimage.h"
#include "face_search.h"
#include "rgb.h"
#include "face_widgets.h"
#include "img_search.h"
#include "sfind_search.h"
#include "search_support.h"
#include "attr_info.h"
#include "sfind_tools.h"
#include "snap_popup.h"
#include "snapfind.h"
#include "search_set.h"

/* XXX fix this */
static search_set *sset;

/*
 * global state used for highlighting (running filters locally)
 */
static struct {
	pthread_mutex_t mutex;
	int 		thread_running;
	pthread_t 	thread;
}
highlight_info = { PTHREAD_MUTEX_INITIALIZER, 0 };



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

#define	POPUP_XSIZE	850
#define	POPUP_YSIZE	350

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
		gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), 	
			id, buf);
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


static GtkWidget *
make_highlight_table()
{
	GtkWidget *widget;
	int row = 0;        /* current table row */
	img_search *	csearch;
	search_iter_t	iter;

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
