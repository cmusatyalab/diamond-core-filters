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

#include "lib_results.h"
#include "rgb.h"
#include "img_search.h"
#include "sfind_tools.h"
#include "search_support.h"
#include "snapfind.h"
#include "gtk_image_tools.h"
#include "search_set.h"
#include "read_config.h"
#include "plugin.h"
#include "attr_decode.h"
#include "plugin-runner.h"
#include "readme.h"

/* number of thumbnails to show */
static const int TABLE_COLS = 3;
static const int TABLE_ROWS = 2;

static thumblist_t thumbnails = TAILQ_HEAD_INITIALIZER(thumbnails);

typedef struct export_threshold_t
{
	char *name;
	double distance;
	int index;			/* index into scapes[] */
	TAILQ_ENTRY(export_threshold_t) link;
}
export_threshold_t;



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


/*
 * some globals that we need to find a place for
 */
GAsyncQueue *	to_search_thread;
GAsyncQueue *	from_search_thread;
int		pend_obj_cnt = 0;
int		tobj_cnt = 0;
int		sobj_cnt = 0;
int		dobj_cnt = 0;



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





static img_search *current_codec = NULL;

img_search *get_current_codec(void)
{
  return current_codec;
}





img_patches_t * 
get_patches(ls_obj_handle_t ohandle, char *fname)
{
        char            buf[BUFSIZ];
       size_t          a_size;
       int             err;
       unsigned char * dptr;
       img_patches_t * patches;

        sprintf(buf, FILTER_MATCHES, fname);
        err = ls_ref_attr(ohandle, buf, &a_size, &dptr);
       patches = (img_patches_t *)dptr;
       if (err) {
               return(NULL);
       } else {
               return(patches);
       }
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

	load_plugins();

	/*
	 * Decide what we are doing
	 */
	const char *cmd;
	if (argc == 1) {
		cmd = "help";
	} else {
		cmd = argv[1];
	}

	if (sc(cmd, "help")) {
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
	} else if (sc(cmd, "normalize-plugin-config")) {
		// check parameters
		if (argc < 4) {
			printf("Missing arguments\n");
			return 1;
		}
		return normalize_plugin_config(argv[2], argv[3]);
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
