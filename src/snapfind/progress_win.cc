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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <assert.h>
#include <string.h>

#include "searchlet_api.h"
#include "gui_thread.h"
#include <sys/queue.h>
#include "lib_results.h"

#include "sfind_search.h"
#include "graph_win.h"



// #define SUMMARY_FONT_NAME "helvetica 14"


static pthread_t       stats_thread;
static int             thread_close;

static GtkWidget      *progress_window = NULL;
static GtkWidget      *progress_box = NULL;


static GtkWidget	*progress_bar;

#define		PROGRESS_X_SIZE		600
#define		PROGRESS_Y_SIZE		400

graph_win	*gwin;

void
dump_stats(dev_stats_t * dstats)
{

	int             i;
	printf("Total objs: %d \n", dstats->ds_objs_total);
	printf("proc objs: %d \n", dstats->ds_objs_processed);
	printf("dropped objs: %d \n", dstats->ds_objs_dropped);

	for (i = 0; i < dstats->ds_num_filters; i++) {

		printf("\t Filter     : %s \n", dstats->ds_filter_stats[i].fs_name);
		printf("\t processed  : %d \n",
		       dstats->ds_filter_stats[i].fs_objs_processed);
		printf("\t dropped   : %d \n",
		       dstats->ds_filter_stats[i].fs_objs_dropped);
		printf("\t time      : %lld \n",
		       dstats->ds_filter_stats[i].fs_avg_exec_time);
	}
}


static char    *name_unknown = "Unknown Name";

char           *
get_dev_name(ls_search_handle_t shandle, ls_dev_handle_t dev_handle)
{
	device_char_t   dchar;
	struct hostent *hent;
	int             err;

	err = ls_dev_characteristics(shandle, dev_handle, &dchar);
	if (err) {
		/*
		 * XXX debug 
		 */
		printf("failed to get dev chars \n");
		return (name_unknown);
	}

	hent = gethostbyaddr((char *) &dchar.dc_devid, sizeof(dchar.dc_devid),
	                     AF_INET);
	if (hent == NULL) {
		/*
		 * XXX debug 
		 */
		printf("host by addr failed \n");
		return (name_unknown);
	}

	return (hent->h_name);
}


double
timeval_to_double(struct timeval *tv)
{
	double 	val;
	val = (double)tv->tv_usec/1000000.0;
	val += (double)tv->tv_sec;
	return(val);
}

/* XXX global for status window, fix  this someday */
extern int      search_active;
extern int      search_number;
extern struct timeval  search_start;

void           *
stats_main(void *data)
{
	ls_search_handle_t shandle;
	int             num_dev;
	ls_dev_handle_t dev_list[SF_MAX_DEVICES];
	int             i, err, len, id;
	dev_stats_t    *dstats;
	double			time = 0.0;
	double			done;
	double			total;
	double			start;
	double			stop;
	double			completed;
	struct	timeval	tv;
	struct	timezone tz;
	int			last_id = -1;


	shandle = (ls_search_handle_t *) data;

	dstats = (dev_stats_t *) malloc(DEV_STATS_SIZE(SF_MAX_FILTERS));
	assert(dstats);

	while (1) {
		if (thread_close) {
			pthread_exit(0);
		}
		/*
			 * Get a list of the devices.
			 */
		num_dev = SF_MAX_DEVICES;
		err = ls_get_dev_list(shandle, dev_list, &num_dev);
		if (err != 0) {
			printf("ls_get_dev_list: %d ", err);
			exit(1);
		}


		done = 0.0;
		total = 0.0;

		if (!search_active) {
			goto wait;
		}

		for (i = 0; i < num_dev; i++) {
			len = DEV_STATS_SIZE(SF_MAX_FILTERS);
			err = ls_get_dev_stats(shandle, dev_list[i], dstats, &len);
			// dump_stats(dstats);
			if (err) {
				printf("failed to get dev stats %d \n", err);
				exit(1);
			}
			total += (double)dstats->ds_objs_total;
			done += (double)dstats->ds_objs_processed;
		}


		completed = done/total;
		if (completed > 1.0) {
			completed = 1.0;
		}

		/* get currnent time and compute elapsed time */
		gettimeofday(&tv, &tz);
		start = timeval_to_double(&search_start);
		stop = timeval_to_double(&tv);
		time = stop - start;

		GUI_THREAD_ENTER();
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar),
		                              completed);


		id = search_number % 8;

		if (last_id != id) {
			//printf("clearing series \n");
			gwin->clear_series(id);
		}
		last_id = id;

		if (time > 3.5)  {
			// XXX printf("done %f \n", done);
			gwin->add_point((double)time, done, id);
		}
		GUI_THREAD_LEAVE();
wait:
		sleep(1);
	}
}



void
stats_close(GtkWidget * window)
{
	progress_window = NULL;
	thread_close = 1;
}


void
open_progress_win()
{
	GtkWidget *	gwidget;
	GtkWidget *	hbox;
	GtkWidget *	label;

	if (!progress_window) {

		GUI_THREAD_CHECK();

		progress_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(progress_window), "Stats");
		gtk_widget_set_name(progress_window, "progress_window");
		g_signal_connect(G_OBJECT(progress_window), "destroy",
		                 G_CALLBACK(stats_close), NULL);
		/*
		 * don't show window until we add all the components 
		 * (for correct sizing) 
		 */

		progress_box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(progress_window), progress_box);

		gwin = new graph_win(0.0, 30.0, 0.0, 5000.0);

		gwidget = gwin->get_graph_display(PROGRESS_X_SIZE, PROGRESS_Y_SIZE);
		gtk_box_pack_start(GTK_BOX(progress_box), gwidget, FALSE, TRUE, 0);


		hbox =  gtk_hbox_new(FALSE, 10);
		gtk_box_pack_start(GTK_BOX(progress_box), hbox, FALSE, TRUE, 0);

		label =  gtk_label_new("Current Progress");
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);

		progress_bar = gtk_progress_bar_new();
		gtk_box_pack_start(GTK_BOX(hbox), progress_bar, TRUE, TRUE, 0);
		gtk_widget_show_all(progress_window);
	} else {
		/* raise the window if it already exists */
		gdk_window_raise(GTK_WIDGET(progress_window)->window);
	}

}


/*
 * Create the region that will display the results of the 
 * search.
 */
void
create_progress_win(ls_search_handle_t shandle)
{
	int             err;

	/*
	 * if we already have the window, don't do anything.
	 */
	if (progress_window != NULL) {
		return;
	}

	/*
	 * First we create a section that keeps track
	 * of the progress and lets us know where we stand in
	 * the search.
	 */
	open_progress_win();

	/*
	 * Create a thread that processes gets the statistics.
	 */
	thread_close = 0;
	err = pthread_create(&stats_thread, NULL, stats_main,
	                     (void *) shandle);
	if (err) {
		perror("create_stats_regions:");
		exit(1);
	}
}

void
close_progress_win()
{
	if (progress_window) {
		gtk_object_destroy(GTK_OBJECT(progress_window));
		// stats_close(NULL);
	}
}


void
toggle_progress_win(ls_search_handle_t shandle, int verbose)
{
	if (progress_window) {
		close_progress_win();
	} else {
		create_progress_win(shandle);
	}
}
