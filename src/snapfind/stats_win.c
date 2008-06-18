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
#include "snapfind_consts.h"
#include "gui_thread.h"
#include <sys/queue.h>
#include "ring.h"
#include "sfind_search.h"


// #define SUMMARY_FONT_NAME "helvetica 14"


pthread_t       stats_thread;
int             thread_close;

GtkWidget      *stats_window = NULL;
GtkWidget      *stats_box = NULL;
GtkWidget      *stats_table = NULL;

static int      verbose_mode = 1;


void
set_summary_attr(GtkWidget * widget)
{
#ifdef SUMMARY_FONT_NAME
	PangoFontDescription *font = NULL;
	font = pango_font_description_from_string(SUMMARY_FONT_NAME);
	gtk_widget_modify_font(widget, font);
	pango_font_description_free(font);
#endif

	gtk_widget_set_name(widget, "summary");
}



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


typedef struct filter_page
{
	LIST_ENTRY(filter_page) fp_link;
	char           *fp_name;
	GtkWidget      *fp_table;
	filt_data_t     fp_data[SF_MAX_DEVICES];
}
filter_page_t;



typedef struct device_progress
{
	GtkWidget      *name_frame;
	GtkWidget      *name_box;
	GtkWidget      *name_label;
	GtkWidget      *prog_frame;
	GtkWidget      *prog_box;
	GtkWidget      *progress_bar;
	GtkWidget      *total_label;
	GtkWidget      *total_text;
	GtkWidget      *proc_label;
	GtkWidget      *drop_text;
	GtkWidget      *drop_label;
	GtkWidget      *nproc_text;
	GtkWidget      *nproc_label;
	filt_data_t     filt_stats[SF_MAX_FILTERS];
}
device_progress_t;




device_progress_t devinfo[SF_MAX_DEVICES];



typedef struct page_master
{
	LIST_HEAD(pm_pages, filter_page) pm_pages;
	GtkWidget      *pm_notebook;
}
page_master_t;

static page_master_t page_master;


void            init_filt_data(filt_data_t * fdata);


static filter_page_t *
find_page(char *name)
{

	filter_page_t  *cur_page;

	LIST_FOREACH(cur_page, &page_master.pm_pages, fp_link) {
		if (strcmp(cur_page->fp_name, name) == 0) {
			return (cur_page);
		}
	}
	return (NULL);
}


void
new_page(char *name, int num_dev)
{
	filter_page_t  *new_page;
	GtkWidget      *new_lab;
	int             i;

	/*
	 * make sure this page doesn't already exist 
	 */
	new_page = find_page(name);
	assert(new_page == NULL);


	new_page = (filter_page_t *) malloc(sizeof(*new_page));

	new_page->fp_name = strdup(name);
	assert(new_page->fp_name != NULL);

	LIST_INSERT_HEAD(&page_master.pm_pages, new_page, fp_link);

	new_page->fp_table = gtk_table_new(1, num_dev + 1, FALSE);

	gtk_widget_show(new_page->fp_table);


	for (i = 0; i < num_dev; i++) {
		init_filt_data(&new_page->fp_data[i]);
		gtk_table_attach_defaults(GTK_TABLE(new_page->fp_table),
	  	    new_page->fp_data[i].frame, 0, 1, (i + 1), (i + 2));
	}


	new_lab = gtk_label_new(name);

	gtk_notebook_append_page(GTK_NOTEBOOK(page_master.pm_notebook),
	                         new_page->fp_table, new_lab);
}



void
remove_page(char *name)
{
	filter_page_t  *old_page;

	/*
	 * make sure this page doesn't already exist 
	 */
	old_page = find_page(name);
	assert(new_page != NULL);

	LIST_REMOVE(old_page, fp_link);
	free(old_page->fp_name);
	free(old_page);

}




void
init_filt_data(filt_data_t * fdata)
{
	char            data[60];

	GUI_THREAD_CHECK();
	/*
	 * Now create another region that has the controls
	 * that manipulate the current image being displayed.
	 */
	fdata->frame = gtk_frame_new(NULL);

	fdata->box = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(fdata->box), 1);
	gtk_container_add(GTK_CONTAINER(fdata->frame), fdata->box);


	fdata->sbox1 = gtk_hbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(fdata->sbox1), 1);
	gtk_box_pack_start(GTK_BOX(fdata->box), fdata->sbox1, FALSE, FALSE, 0);


	fdata->sbox2 = gtk_hbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(fdata->sbox2), 1);
	gtk_box_pack_start(GTK_BOX(fdata->box), fdata->sbox2, FALSE, FALSE, 0);


	fdata->sbox3 = gtk_hbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(fdata->sbox3), 1);
	gtk_box_pack_start(GTK_BOX(fdata->box), fdata->sbox3, FALSE, FALSE, 0);


	sprintf(data, "%s", "Avg Time (ms):");
	fdata->time_label = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(fdata->sbox1),
	                   fdata->time_label, FALSE, FALSE, 0);

	sprintf(data, "%10.5f", 0.0);
	fdata->time_val = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(fdata->sbox1),
	                   fdata->time_val, FALSE, FALSE, 0);


	sprintf(data, "%s", "Objs Proc:");
	fdata->proc_label = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(fdata->sbox2), fdata->proc_label,
	                   FALSE, FALSE, 0);


	sprintf(data, "%8d", 0);
	fdata->proc_val = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(fdata->sbox2),
	                   fdata->proc_val, FALSE, FALSE, 0);


	sprintf(data, "%s", "Objs Drop:");
	fdata->drop_label = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(fdata->sbox3), fdata->drop_label,
	                   FALSE, FALSE, 0);

	sprintf(data, "%s", "Bypass Obj:");
	fdata->nproc_label = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(fdata->sbox3), fdata->nproc_label,
	                   FALSE, FALSE, 0);


	sprintf(data, "%8d", 0);
	fdata->drop_val = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(fdata->sbox3), fdata->drop_val, FALSE, FALSE,
	                   0);

	gtk_widget_show_all(fdata->frame);

}




void
update_filt_data(filt_data_t * fdata, filter_stats_t * fstats)
{

	char            data[60];
	double          elap_time;

	GUI_THREAD_CHECK();

	elap_time = fstats->fs_avg_exec_time / 1000000000.0;
	elap_time = elap_time * 1000.0; /* convert to ms */
	sprintf(data, "%.1f", elap_time);
	gtk_label_set_text(GTK_LABEL(fdata->time_val), data);


	sprintf(data, "%-8d", fstats->fs_objs_processed);
	gtk_label_set_text(GTK_LABEL(fdata->proc_val), data);

	sprintf(data, "%-8d", fstats->fs_objs_dropped);
	gtk_label_set_text(GTK_LABEL(fdata->drop_val), data);
}




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


void
new_dev_status(ls_search_handle_t shandle, int dev,
               ls_dev_handle_t dev_handle)
{

	char            data[60];
	device_progress_t *dstatus;
	char           *name;
	int             i;

	GUI_THREAD_CHECK();

	dstatus = &devinfo[dev];

	dstatus->name_frame = gtk_frame_new(NULL);
	dstatus->name_box = gtk_hbox_new(FALSE, 10);

	gtk_container_set_border_width(GTK_CONTAINER(dstatus->name_box), 4);
	gtk_widget_show(dstatus->name_box);
	gtk_container_add(GTK_CONTAINER(dstatus->name_frame), dstatus->name_box);
	gtk_widget_show(dstatus->name_frame);

	name = get_dev_name(shandle, dev_handle);
	dstatus->name_label = gtk_label_new(name);
	gtk_widget_set_name(dstatus->name_label, "diskname");
	gtk_box_pack_start(GTK_BOX(dstatus->name_box), dstatus->name_label,
	                   FALSE, FALSE, 0);
	gtk_widget_show(dstatus->name_label);

	dstatus->prog_frame = gtk_frame_new(NULL);
	dstatus->prog_box = gtk_hbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(dstatus->prog_box), 4);
	gtk_widget_show(dstatus->prog_box);
	gtk_container_add(GTK_CONTAINER(dstatus->prog_frame), dstatus->prog_box);
	gtk_widget_show(dstatus->prog_frame);

	dstatus->progress_bar = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(dstatus->prog_box), dstatus->progress_bar,
	                   FALSE, FALSE, 0);
	gtk_widget_show(dstatus->progress_bar);


	sprintf(data, "%-8d", 0);
	dstatus->proc_label = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(dstatus->prog_box),
	                   dstatus->proc_label, FALSE, FALSE, 0);
	gtk_widget_show(dstatus->proc_label);
	set_summary_attr(dstatus->proc_label);

	sprintf(data, "%s", "of");
	dstatus->total_text = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(dstatus->prog_box),
	                   dstatus->total_text, FALSE, FALSE, 0);
	gtk_widget_show(dstatus->total_text);
	set_summary_attr(dstatus->total_text);

	sprintf(data, "%-8d", 0);
	dstatus->total_label = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(dstatus->prog_box),
	                   dstatus->total_label, FALSE, FALSE, 0);
	gtk_widget_show(dstatus->total_label);
	set_summary_attr(dstatus->total_label);

	sprintf(data, "%s", "Drops:");
	dstatus->drop_text = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(dstatus->prog_box),
	                   dstatus->drop_text, FALSE, FALSE, 0);
	gtk_widget_show(dstatus->drop_text);
	set_summary_attr(dstatus->drop_text);

	sprintf(data, "%-8d", 0);
	dstatus->drop_label = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(dstatus->prog_box),
	                   dstatus->drop_label, FALSE, FALSE, 0);
	gtk_widget_show(dstatus->drop_label);
	set_summary_attr(dstatus->drop_label);

	sprintf(data, "%s", "Deferred:");
	dstatus->nproc_text = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(dstatus->prog_box),
	                   dstatus->nproc_text, FALSE, FALSE, 0);
	gtk_widget_show(dstatus->nproc_text);
	set_summary_attr(dstatus->nproc_text);

	sprintf(data, "%-8d", 0);
	dstatus->nproc_label = gtk_label_new(data);
	gtk_box_pack_start(GTK_BOX(dstatus->prog_box),
	                   dstatus->nproc_label, FALSE, FALSE, 0);
	gtk_widget_show(dstatus->nproc_label);
	set_summary_attr(dstatus->nproc_label);


	for (i = 0; i < SF_MAX_FILTERS; i++) {
		init_filt_data(&dstatus->filt_stats[i]);
	}
}




void
attach_data_rows(int dev, int num_filt, int verbose)
{

	device_progress_t *dstatus;
	// int i;

	GUI_THREAD_CHECK();

	dstatus = &devinfo[dev];

	gtk_table_attach_defaults(GTK_TABLE(stats_table), dstatus->name_frame,
	                          0, 1, (dev + 1), (dev + 2));

	gtk_table_attach_defaults(GTK_TABLE(stats_table), dstatus->prog_frame,
	                          1, 2, (dev + 1), (dev + 2));

#ifdef  XXX

	if (verbose) {
		for (i = 0; i < num_filt; i++) {
			gtk_table_attach_defaults(GTK_TABLE(stats_table),
			                          dstatus->filt_stats[i].frame, (i + 2),
			                          (i + 3), (dev + 1), (dev + 2));
			/*
			 * XXX init_filt_data(&dstatus->filt_stats[i]); 
			 */
		}
	}
#endif
}



void
update_dev_status(int dev, dev_stats_t * dstats, int verbose)
{
	char            data[60];
	device_progress_t *dstatus;
	double          done;
	int             i;

	GUI_THREAD_CHECK();

	dstatus = &devinfo[dev];

	if (dstats->ds_objs_total == 0) {
		done = 0.0;
	} else {
		done = ((double) dstats->ds_objs_processed +
		        (double) dstats->ds_objs_nproc) /
		       (double) dstats->ds_objs_total;
		if (!(done <= 1.0)) {
			done = 1.0;
		}
	}
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(dstatus->progress_bar),
	                              done);

	/*
	 * dump_stats(dstats); 
	 */


	sprintf(data, "%-8d", dstats->ds_objs_processed + dstats->ds_objs_nproc);
	gtk_label_set_text(GTK_LABEL(dstatus->proc_label), data);

	sprintf(data, "%-8d", dstats->ds_objs_total);
	gtk_label_set_text(GTK_LABEL(dstatus->total_label), data);

	sprintf(data, "%-8d", dstats->ds_objs_dropped);
	gtk_label_set_text(GTK_LABEL(dstatus->drop_label), data);

	sprintf(data, "%-8d", dstats->ds_objs_nproc);
	gtk_label_set_text(GTK_LABEL(dstatus->nproc_label), data);

	for (i = 0; i < dstats->ds_num_filters; i++) {
		update_filt_data(&dstatus->filt_stats[i],
		                 &dstats->ds_filter_stats[i]);
	}
}


/*
 * Setup the table for each of the devices.
 */

void
setup_table(dev_stats_t * dstats, int num_dev, int verbose)
{
	int             col,
	row;
	int             i;
	GtkWidget      *frame;
	GtkWidget      *box;
	GtkWidget      *label;

	GUI_THREAD_CHECK();

	col = 3;                    /* machine+progress+filter_stats */
	row = num_dev + 1;          /* 1=headings */

	stats_table = gtk_table_new(row, col, FALSE);
	gtk_box_pack_start(GTK_BOX(stats_box), stats_table, FALSE, TRUE, 0);


	frame = gtk_frame_new(NULL);
	box = gtk_hbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), box);

	label = gtk_label_new("Devices");
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, TRUE, 0);
	gtk_widget_set_name(label, "heading");
	gtk_table_attach_defaults(GTK_TABLE(stats_table), frame, 0, 1, 0, 1);



	frame = gtk_frame_new(NULL);
	box = gtk_hbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), box);

	label = gtk_label_new("Progress");
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, TRUE, 0);
	gtk_widget_set_name(label, "heading");
	gtk_table_attach_defaults(GTK_TABLE(stats_table), frame, 1, 2, 0, 1);

#ifdef  XXX
	/*
	 * create the heading for the filter statistics 
	 */
	frame = gtk_frame_new(NULL);
	box = gtk_hbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), box);

	label = gtk_label_new("Filters");
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, TRUE, 0);
	gtk_widget_set_name(label, "heading");
	if (verbose) {
		gtk_widget_show(label);
	}
	gtk_table_attach_defaults(GTK_TABLE(stats_table), frame, 2, 3, 0, 1);
#endif

	/*
	 * create the notebook for the filter statistics 
	 */
	LIST_INIT(&page_master.pm_pages);

	// XXX frame = gtk_frame_new(NULL);
	box = gtk_hbox_new(FALSE, 10);
	// XXX gtk_container_add(GTK_CONTAINER(frame), box);
	// XXX gtk_widget_show(frame);

	page_master.pm_notebook = gtk_notebook_new();

	gtk_box_pack_start(GTK_BOX(box), page_master.pm_notebook, FALSE, TRUE,
	                   0);
	gtk_widget_set_name(label, "heading");
	if (verbose) {
		gtk_widget_show(page_master.pm_notebook);
		gtk_table_attach_defaults(GTK_TABLE(stats_table), box, 2, 3, 0, row);
	}

	for (i = 0; i < dstats->ds_num_filters; i++) {
		new_page(dstats->ds_filter_stats[i].fs_name, num_dev);
	}


	for (i = 0; i < num_dev; i++) {
		attach_data_rows(i, dstats->ds_num_filters, verbose);
	}

	gtk_widget_show_all(stats_table);
}


void           *
stats_main(void *data)
{
	ls_search_handle_t shandle;
	int             num_dev;
	ls_dev_handle_t dev_list[SF_MAX_DEVICES];
	int             i,
	err,
	len;
	dev_stats_t    *dstats;
	char           *emsg;

	shandle = (ls_search_handle_t *) data;

	dstats = (dev_stats_t *) malloc(DEV_STATS_SIZE(SF_MAX_FILTERS));
	assert(dstats);
	/*
	 * Get a list of the devices.
	 */
	num_dev = SF_MAX_DEVICES;
	err = ls_get_dev_list(shandle, dev_list, &num_dev);
	if (err != 0) {
		printf("ls_get_dev_list: %d ", err);
		exit(1);
	}

	GUI_THREAD_ENTER();
	for (i = 0; i < num_dev; i++) {
		new_dev_status(shandle, i, dev_list[i]);
	}
	GUI_THREAD_LEAVE();


	/*
	 * get the first copy of the stats to build the list 
	 */
	len = DEV_STATS_SIZE(SF_MAX_FILTERS);
	err = ls_get_dev_stats(shandle, dev_list[0], dstats, &len);
	if (err) {
		/*
		 * XXX 
		 */
		emsg = strerror(err);
		printf("failed to get stats: %s \n", emsg);
		pthread_exit(0);
	}

	/*
	 * use the stats to populate the table. XXX deal with
	 * changes in layout later.
	 */
	GUI_THREAD_ENTER();
	setup_table(dstats, num_dev, verbose_mode);

	/*
	 * only show the stats window after we add all the components, so it gets 
	 * the right size 
	 */
	gtk_widget_show(stats_window);
	GUI_THREAD_LEAVE();

	while (1) {

		if (thread_close) {
			pthread_exit(0);
		}

		for (i = 0; i < num_dev; i++) {
			len = DEV_STATS_SIZE(SF_MAX_FILTERS);
			err = ls_get_dev_stats(shandle, dev_list[i], dstats, &len);
			// dump_stats(dstats);
			if (err) {
				printf("failed to get dev stats %d \n", err);
				exit(1);
			}
			GUI_THREAD_ENTER();
			update_dev_status(i, dstats, verbose_mode);
			GUI_THREAD_LEAVE();
		}
		sleep(1);
	}
}



void
stats_close(GtkWidget * window)
{
	stats_window = NULL;

	thread_close = 1;
}

void
open_stats_window()
{
	if (!stats_window) {

		GUI_THREAD_CHECK();

		stats_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(stats_window), "Stats");

		gtk_widget_set_name(stats_window, "stats_window");
		g_signal_connect(G_OBJECT(stats_window), "destroy",
		                 G_CALLBACK(stats_close), NULL);
		/*
		 * don't show window until we add all the components (for correct
		 * sizing) 
		 */
		// gtk_widget_show(stats_window);

		stats_box = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(stats_window), stats_box);
		gtk_widget_show(stats_box);
	}
}




/*
 * Create the region that will display the results of the 
 * search.
 */
void
create_stats_win(ls_search_handle_t shandle, int verbose)
{
	int             err;

	/*
	 * if we already have the window, don't do anything.
	 */
	if (stats_window != NULL) {
		return;
	}
	//verbose_mode = verbose;
	/*
	 * XXX 
	 */
	//verbose_mode = 0;
	verbose_mode = 1;

	/*
	 * First we create a section that keeps track
	 * of the progress and lets us know where we stand in
	 * the search.
	 */

	open_stats_window();


	/*
	 * Create a thread that processes gets the statistics.
	 */
	thread_close = 0;
	err =
	    pthread_create(&stats_thread, NULL, stats_main,
	                   (void *) shandle);
	if (err) {
		perror("create_stats_regions:");
		exit(1);
	}
}





void
close_stats_win()
{
	if (stats_window) {
		gtk_object_destroy(GTK_OBJECT(stats_window));
		// stats_close(NULL);
	}
}



void
toggle_stats_win(ls_search_handle_t shandle, int verbose)
{
	if (stats_window) {
		close_stats_win();
	} else {
		create_stats_win(shandle, verbose);
	}
}
