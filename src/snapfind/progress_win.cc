/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include "face_consts.h"
#include "gui_thread.h"
#include "rtimer.h"
#include "queue.h"
#include "ring.h"
#include "sfind_search.h"
#include "graph_win.h"


#define	MAX_DEVICES	24

// #define SUMMARY_FONT_NAME "helvetica 14"


static pthread_t       stats_thread;
static int             thread_close;

static GtkWidget      *progress_window = NULL;
static GtkWidget      *progress_box = NULL;

static int      verbose_mode = 1;

static GtkWidget	*progress_bar;

#define		PROGRESS_X_SIZE		600
#define		PROGRESS_Y_SIZE		400

graph_win	*gwin;

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



typedef struct filt_data {
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
} filt_data_t;


typedef struct filter_page {
    LIST_ENTRY(filter_page) fp_link;
    char           *fp_name;
    GtkWidget      *fp_table;
    filt_data_t     fp_data[MAX_DEVICES];
} filter_page_t;



typedef struct device_progress {
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

    filt_data_t     filt_stats[MAX_FILT];
} device_progress_t;




device_progress_t devinfo[MAX_DEVICES];



typedef struct page_master {
    LIST_HEAD(pm_pages, filter_page) pm_pages;
    GtkWidget      *pm_notebook;
} page_master_t;

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
                                  new_page->fp_data[i].frame, 0, 1, (i + 1),
                                  (i + 2));
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
    gtk_widget_show(fdata->frame);

    fdata->box = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(fdata->box), 1);
    gtk_container_add(GTK_CONTAINER(fdata->frame), fdata->box);
    gtk_widget_show(fdata->box);


    fdata->sbox1 = gtk_hbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(fdata->sbox1), 1);
    gtk_box_pack_start(GTK_BOX(fdata->box), fdata->sbox1, FALSE, TRUE, 0);
    gtk_widget_show(fdata->sbox1);


    fdata->sbox2 = gtk_hbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(fdata->sbox2), 1);
    gtk_box_pack_start(GTK_BOX(fdata->box), fdata->sbox2, FALSE, TRUE, 0);
    gtk_widget_show(fdata->sbox2);


    fdata->sbox3 = gtk_hbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(fdata->sbox3), 1);
    gtk_box_pack_start(GTK_BOX(fdata->box), fdata->sbox3, FALSE, TRUE, 0);
    gtk_widget_show(fdata->sbox3);


    sprintf(data, "%s", "Avg Time (ms):");
    fdata->time_label = gtk_label_new(data);
    gtk_box_pack_start(GTK_BOX(fdata->sbox1),
                       fdata->time_label, FALSE, TRUE, 0);
    gtk_widget_show(fdata->time_label);

    sprintf(data, "%10.5f", 0.0);
    fdata->time_val = gtk_label_new(data);
    gtk_box_pack_start(GTK_BOX(fdata->sbox1),
                       fdata->time_val, FALSE, TRUE, 0);
    gtk_widget_show(fdata->time_val);


    sprintf(data, "%s", "Objs Proc:");
    fdata->proc_label = gtk_label_new(data);
    gtk_box_pack_start(GTK_BOX(fdata->sbox2), fdata->proc_label,
                       FALSE, TRUE, 0);
    gtk_widget_show(fdata->proc_label);


    sprintf(data, "%8d", 0);
    fdata->proc_val = gtk_label_new(data);
    gtk_box_pack_start(GTK_BOX(fdata->sbox2),
                       fdata->proc_val, FALSE, TRUE, 0);
    gtk_widget_show(fdata->proc_val);


    sprintf(data, "%s", "Objs Drop:");
    fdata->drop_label = gtk_label_new(data);
    gtk_box_pack_start(GTK_BOX(fdata->sbox3), fdata->drop_label,
                       FALSE, TRUE, 0);
    gtk_widget_show(fdata->drop_label);

    sprintf(data, "%s", "Bypass Obj:");
    fdata->nproc_label = gtk_label_new(data);
    gtk_box_pack_start(GTK_BOX(fdata->sbox3), fdata->nproc_label,
                       FALSE, TRUE, 0);
    gtk_widget_show(fdata->nproc_label);


    sprintf(data, "%8d", 0);
    fdata->drop_val = gtk_label_new(data);
    gtk_box_pack_start(GTK_BOX(fdata->sbox3), fdata->drop_val, FALSE, TRUE,
                       0);
    gtk_widget_show(fdata->drop_val);


}




void
update_filt_data(filt_data_t * fdata, filter_stats_t * fstats)
{

    char            data[60];
    double          elap_time;

    GUI_THREAD_CHECK();

    elap_time = rt_time2secs(fstats->fs_avg_exec_time);
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
    ls_dev_handle_t dev_list[MAX_DEVICES];
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

    dstats = (dev_stats_t *) malloc(DEV_STATS_SIZE(MAX_FILT));
    assert(dstats);

    while (1) {
        if (thread_close) {
            pthread_exit(0);
        }
    	/*
     	 * Get a list of the devices.
     	 */
    	num_dev = MAX_DEVICES;
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
			len = DEV_STATS_SIZE(MAX_FILT);
			err = ls_get_dev_stats(shandle, dev_list[i], dstats, &len);
			// dump_stats(dstats);
			if (err) {
				printf("failed to get dev stats %d \n", err);
				exit(1);
			}
			total += (double)dstats->ds_objs_total;
			done += (double)dstats->ds_objs_processed;
			// XXX printf("done %f \n", done);
		}
	
   
		completed = done/total; 
        if (!(completed <= 1.0)) {
            completed = 1.0;
        }
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar),
                                 completed);

		gettimeofday(&tv, &tz);
		start = timeval_to_double(&search_start);
		stop = timeval_to_double(&tv);
		time = stop - start;
		// XXX printf("time %f \n", time);

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
         * don't show window until we add all the components (for correct
         * sizing) 
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
    }
}




/*
 * Create the region that will display the results of the 
 * search.
 */
void
create_progress_win(ls_search_handle_t shandle, int verbose)
{
    int             err;

    /*
     * if we already have the window, don't do anything.
     */
    if (progress_window != NULL) {
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

    open_progress_win();


    /*
     * Create a thread that processes gets the statistics.
     */
    thread_close = 0;
    err = pthread_create(&stats_thread, PATTR_DEFAULT, stats_main,
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
        create_progress_win(shandle, verbose);
    }
}
