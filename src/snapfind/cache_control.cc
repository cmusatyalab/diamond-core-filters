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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>


#include "searchlet_api.h"
#include "lib_results.h"
#include "gui_thread.h"
#include "lib_tools.h"
#include "sfind_search.h"
#include "graph_win.h"



static pthread_t       stats_thread;
static int             thread_close;

static GtkWidget      *ccontrol_window = NULL;

static GtkWidget      *par_cache;
static GtkWidget      *res_cache;

#define		PROGRESS_X_SIZE		600
#define		PROGRESS_Y_SIZE		400



typedef struct filt_data
{
	GtkWidget      *frame;
	GtkWidget      *box;
	GtkWidget      *sbox1;
	GtkWidget      *sbox2;
	GtkWidget      *sbox3;
	GtkWidget      *time_label;
	GtkWidget      *time_val;
	GtkWidget      *proc_label;
	GtkWidget      *proc_val;
	GtkWidget      *drop_label;
	GtkWidget      *nproc_label;
	GtkWidget      *drop_val;
}
filt_data_t;



static char    *name_unknown = "Unknown Name";

static char           *
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




#define	MAX_BUF		512

void
set_for_each_device(ls_search_handle_t shandle, char *rem_string,
                    int len, char *val)
{
	int             num_dev;
	ls_dev_handle_t dev_list[SF_MAX_DEVICES];
	int             i, err;
	char	    big_buf[MAX_BUF];
	char *	   delim;
	char	    node_name[MAX_BUF];
	struct hostent *hent;

	/*
	    	 * Get a list of the devices.
	    	 */
	num_dev = SF_MAX_DEVICES;
	err = ls_get_dev_list(shandle, dev_list, &num_dev);
	if (err != 0) {
		printf("ls_get_dev_list: %d ", err);
		exit(1);
	}
}


void           *
ccontrol_main(void *data)
{
	ls_search_handle_t shandle;
	uint32_t	val;


	shandle = (ls_search_handle_t *) data;


	while (1) {
		if (thread_close) {
			pthread_exit(0);
		}

		val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(res_cache));
		/* first check the clear devices */
		set_for_each_device(shandle, "cache.use_cache_table", sizeof(val),
		                    (char *)&val);


		val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(par_cache));
		/* first check the clear devices */
		set_for_each_device(shandle, "cache.use_cache_oattr", sizeof(val),
		                    (char *)&val);

		sleep(1);
	}
}


void
ccontrol_close(GtkWidget * window)
{
	ccontrol_window = NULL;

	thread_close = 1;
}


void
open_ccontrol_win()
{
	GtkWidget *	main_box;
	GtkWidget *	hbox;


	if (!ccontrol_window) {
		GUI_THREAD_CHECK();

		ccontrol_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(ccontrol_window), "Cache Control");
		gtk_widget_set_name(ccontrol_window, "cache_controL_window");
		g_signal_connect(G_OBJECT(ccontrol_window), "destroy",
		                 G_CALLBACK(ccontrol_close), NULL);

		/*
		 * don't show window until we add all the components (for correct
		 * sizing) 
		 */
		main_box = gtk_vbox_new(FALSE, 10);
		gtk_container_add(GTK_CONTAINER(ccontrol_window), main_box);

		hbox = gtk_hbox_new(FALSE, 10);
		gtk_box_pack_start(GTK_BOX(main_box), hbox, FALSE, TRUE, 5);

		res_cache = gtk_check_button_new_with_label("Result Cache");
		gtk_box_pack_start(GTK_BOX(hbox), res_cache, FALSE, TRUE, 0);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(res_cache), 1);

		hbox = gtk_hbox_new(FALSE, 10);
		gtk_box_pack_start(GTK_BOX(main_box), hbox, FALSE, TRUE, 10);

		par_cache = gtk_check_button_new_with_label("Partial Cache");
		gtk_box_pack_start(GTK_BOX(hbox), par_cache, FALSE, TRUE, 0);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(par_cache), 1);

		gtk_widget_show_all(ccontrol_window);
	}
}




/*
 * Create the region that will display the results of the 
 * search.
 */
void
create_ccontrol_win(ls_search_handle_t shandle, int verbose)
{
	int             err;

	/*
	 * if we already have the window, don't do anything.
	 */
	if (ccontrol_window != NULL) {
		/* XXX raise it */
		return;
	}

	/*
	 * First we create a section that keeps track
	 * of the progress and lets us know where we stand in
	 * the search.
	 */

	open_ccontrol_win();


	/*
	 * Create a thread that processes gets the statistics.
	 */
	thread_close = 0;
	err = pthread_create(&stats_thread, NULL, ccontrol_main,
	                     (void *) shandle);
	if (err) {
		perror("create_stats_regions:");
		exit(1);
	}
}





void
close_ccontrol_win()
{
	if (ccontrol_window) {
		gtk_object_destroy(GTK_OBJECT(ccontrol_window));
		// stats_close(NULL);
	}
}



void
toggle_ccontrol_win(ls_search_handle_t shandle, int verbose)
{
	if (ccontrol_window) {
		close_ccontrol_win();
	} else {
		create_ccontrol_win(shandle, verbose);
	}
}
