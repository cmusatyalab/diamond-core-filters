/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *   Copyright (c) 2006 Larry Huston <larry@thehustons.net>
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <assert.h>
#include <string.h>

#include "lib_tools.h"
#include "lib_log.h"
#include "log_socket.h"
#include "searchlet_api.h"
#include "gui_thread.h"
#include "rtimer.h"
#include "queue.h"
#include "ring.h"
#include "lib_results.h"
#include "snapfind_consts.h"
#include "snapfind.h"

#include "sfind_search.h"
#include "graph_win.h"

#define	LOGWIN_XSIZE	600
#define	LOGWIN_YSIZE	450

static uint32_t	log_level = LOGL_CRIT|LOGL_ERR;
static uint32_t	log_type = LOGT_ALL;
static int	log_fd = -1;



static pthread_t       stats_thread;

static GtkWidget      *log_window = NULL;
static GtkWidget      *log_box = NULL;
static GtkWidget      *text_view = NULL;

static GtkTextBuffer *text_buf;

static GtkTextTag 	*crit_tag;
static GtkTextTag 	*err_tag;
static GtkTextTag 	*info_tag;
static GtkTextTag 	*trace_tag;

static GtkWidget *cb_crit;
static GtkWidget *cb_err;
static GtkWidget *cb_info;
static GtkWidget *cb_trace;

static GtkWidget *cb_app;
static GtkWidget *cb_disk;
static GtkWidget *cb_filt;
static GtkWidget *cb_bg;
static GtkWidget *cb_utility;
static GtkWidget *cb_net;

/*
 * Convert the level to a human readable string.
 */
static void
get_level_string(uint32_t level, char *string, int max, GtkTextTag **tagp)
{


	switch(level) {
		case LOGL_CRIT:
			snprintf(string, max, "%s", "Crit");
			*tagp = crit_tag;
			break;

		case LOGL_ERR:
			snprintf(string, max, "%s", "Err");
			*tagp = err_tag;
			break;

		case LOGL_INFO:
			snprintf(string, max, "%s", "Info");
			*tagp = info_tag;
			break;

		case LOGL_TRACE:
			snprintf(string, max, "%s", "Trace");
			*tagp = trace_tag;
			break;

		default:
			snprintf(string, max, "%s", "Unknown");
			break;
	}
}

static void
get_type_string(uint32_t level, char *string, int max)
{


	switch(level) {

		case LOGT_APP:
			snprintf(string, max, "%s", "App");
			break;

		case LOGT_DISK:
			snprintf(string, max, "%s", "Disk");
			break;

		case LOGT_FILT:
			snprintf(string, max, "%s", "Filt");
			break;

		case LOGT_BG:
			snprintf(string, max, "%s", "Bkgrnd");
			break;

		case LOGT_UTILITY:
			snprintf(string, max, "%s", "Utility");
			break;

		case LOGT_NET:
			snprintf(string, max, "%s", "NET");
			break;

		default:
			snprintf(string, max, "%s", "Unknown");
			break;
	}
}

#define MAX_LEVEL       6
#define MAX_TYPE        8

void
process_log(log_msg_t *lheader, const char *data)
{
	char*		source;
	char *		host_id;
	struct in_addr 	iaddr;
	log_ent_t *	log_ent;
	uint32_t	level;
	uint32_t	type;
	uint32_t	dlen;
	char		level_string[MAX_LEVEL];
	char		type_string[MAX_TYPE];
	char		buf[2048];
	int		cur_offset, total_len;
	GtkTextIter   	iter;
	static int	count = 0;
	GtkTextTag  *	tag = trace_tag;
	struct hostent *hent;

	/*
	 * Setup the source of the data.
	 */
	if (lheader->log_type == LOG_SOURCE_BACKGROUND) {
		source = "HOST";
	} else {
		source = "DISK";
	}


	iaddr.s_addr = lheader->dev_id;
	host_id = inet_ntoa(iaddr);

	total_len = lheader->log_len;
	cur_offset = 0;

	GUI_THREAD_ENTER();

	while (cur_offset < total_len) {
		log_ent = (log_ent_t *)&data[cur_offset];

		level = ntohl(log_ent->le_level);
		type = ntohl(log_ent->le_type);

		/* update the offset now in case we decide to skip this */
		cur_offset += ntohl(log_ent->le_nextoff);
		dlen = ntohl(log_ent->le_dlen);
		log_ent->le_data[dlen - 1] = '\0';

		get_level_string(level, level_string, MAX_LEVEL, &tag);
		get_type_string(type, type_string, MAX_TYPE);

		sprintf(buf, "%-5s %-15s %-7s %-8s %s \n", source, host_id,
	    	    level_string, type_string, log_ent->le_data);


		/* Acquire an iterator */
		gtk_text_buffer_get_end_iter(text_buf, &iter);
		count++;
		gtk_text_buffer_insert_with_tags(text_buf, &iter, 
			buf, -1, tag, NULL);

	}
	if (text_view != NULL) {
		gtk_text_buffer_get_end_iter(text_buf, &iter);
		gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(text_view), &iter,
		    0.0, FALSE, 0.0, 1.0);

	}
	GUI_THREAD_LEAVE();
}

void
read_log(int fd, unsigned int level_flags, unsigned int src_flags)
{
	int	len;
	log_msg_t	lheader;
	char 	*data;

	while (1) {
		len = recv(fd, &lheader, sizeof(lheader), MSG_WAITALL);
		if (len != sizeof(lheader)) {
			return;
		}
		data = (char *)malloc(lheader.log_len);
		if (data == NULL) {
			perror("Failed malloc \n");
			exit(1);
		}

		len = recv(fd, data, lheader.log_len, MSG_WAITALL);
		if (len != lheader.log_len) {
			return;
		}

		process_log(&lheader, data);
		free(data);
	}
}


static void
set_log_flags()
{
	log_set_level_t		log_msg;
	int			len;

	if (log_fd == -1) {
		return;
	}
	log_msg.log_op = LOG_SETLEVEL_ALL;
	log_msg.log_level = htonl(log_level);
	log_msg.log_src = htonl(log_type);
	log_msg.dev_id = 0;

	len = send(log_fd, &log_msg, sizeof(log_msg), MSG_WAITALL);
	if (len != sizeof(log_msg)) {
		if (len == -1) {
			perror("failed to set log flags ");
		}
		return;
	}
}

static void           *
log_main(void *data)
{
	int	fd, err;
  	struct sockaddr_un sa;
	uint32_t        level_flags = (LOGL_ERR|LOGL_CRIT);
	uint32_t        src_flags = LOGT_ALL;
	char 	user_name[MAX_USER_NAME];


	while (1) {
		fd = socket(PF_UNIX, SOCK_STREAM, 0);

		/* bind the socket to a path name */
		get_user_name(user_name);
		sprintf(sa.sun_path, "%s.%s", SOCKET_LOG_NAME, user_name);

		sa.sun_family = AF_UNIX;

		err = connect(fd, (struct sockaddr *)&sa, sizeof (sa));
		if (err < 0) {
			/*
			 * If the open fails, then nobody is
			 * running, so we sleep for a while
			 * and retry later.
			 */
			sleep(1);
		} else {
			/*
			 * We sucessfully connection, now we set
			 * the parameters we want to log and
			 * read the log.
			 */
			log_fd = fd;
			set_log_flags();
			read_log(log_fd, level_flags, src_flags);
			log_fd = -1;
		}
		close(fd);
	}
}


static void
cb_set_type(GtkButton *item, gpointer data)
{
	uint32_t	new_type = 0;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_app))) {
		new_type |= LOGT_APP;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_disk))) {
		new_type |= LOGT_DISK;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_filt))) {
		new_type |= LOGT_FILT;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_bg))) {
		new_type |= LOGT_BG;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_utility))) {
		new_type |= LOGT_UTILITY;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_net))) {
		new_type |= LOGT_NET;
	}

	log_type = new_type;
	set_log_flags();

}

static void
cb_set_level(GtkButton *item, gpointer data)
{
	uint32_t	new_level = 0;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_crit))) {
		new_level |= LOGL_CRIT;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_err))) {
		new_level |= LOGL_ERR;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_info))) {
		new_level |= LOGL_INFO;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_trace))) {
		new_level |= LOGL_TRACE;
	}

	log_level = new_level;
	set_log_flags();
}

GtkWidget *
create_type_cb(char *name, int val)
{
	GtkWidget *widget;
	widget = gtk_check_button_new_with_label(name);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), val);
	g_signal_connect(G_OBJECT(widget), "toggled",
		 G_CALLBACK(cb_set_type), NULL);
	return(widget);
}	


GtkWidget *
create_level_cb(char *name, int val)
{
	GtkWidget *widget;
	widget = gtk_check_button_new_with_label(name);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), val);
	g_signal_connect(G_OBJECT(widget), "toggled",
		 G_CALLBACK(cb_set_level), NULL);
	return(widget);
}	

static void
log_close(GtkWidget * window)
{
	log_window = NULL;
	text_view = NULL;
}

static void
open_log_win()
{
	GtkWidget *scroll;
	GtkWidget *hbox;
	GtkWidget *widget;

	if (!log_window) {

		GUI_THREAD_CHECK();

		log_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(log_window), "Log Output");
		gtk_widget_set_name(log_window, "log_window");

		gtk_window_set_default_size(GTK_WINDOW(log_window),
			LOGWIN_XSIZE, LOGWIN_YSIZE);
		g_signal_connect(G_OBJECT(log_window), "destroy",
		                 G_CALLBACK(log_close), NULL);
		/*
		 * don't show window until we add all the components 
		 * (for correct sizing) 
		 */

		log_box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(log_window), log_box);

		/* setup controls for the log levels */
        	hbox = gtk_hbox_new(FALSE, 10);
		gtk_box_pack_start(GTK_BOX(log_box), hbox, FALSE, FALSE, 0);

		widget = gtk_label_new("Log Levels");
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);

	        /* create the check boxes for the log levels */
		cb_crit = create_level_cb("Critical", log_level & LOGL_CRIT);
		gtk_box_pack_start(GTK_BOX(hbox), cb_crit, FALSE, FALSE, 0);
		cb_err = create_level_cb("Err", log_level & LOGL_ERR);
		gtk_box_pack_start(GTK_BOX(hbox), cb_err, FALSE, FALSE, 0);
		cb_info = create_level_cb("Info", log_level & LOGL_INFO);
		gtk_box_pack_start(GTK_BOX(hbox), cb_info, FALSE, FALSE, 0);
		cb_trace = create_level_cb("Trace", log_level & LOGL_TRACE);
		gtk_box_pack_start(GTK_BOX(hbox), cb_trace, FALSE, FALSE, 0);



		/* setup controls for the log types */
        	hbox = gtk_hbox_new(FALSE, 10);
		gtk_box_pack_start(GTK_BOX(log_box), hbox, FALSE, FALSE, 0);

		widget = gtk_label_new("Log Types");
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);

	        /* create the check boxes for the log levels */
		cb_app = create_type_cb("Application", log_type & LOGT_APP);
		gtk_box_pack_start(GTK_BOX(hbox), cb_app, FALSE, FALSE, 0);

		cb_disk = create_type_cb("Disk", log_type & LOGT_DISK);
		gtk_box_pack_start(GTK_BOX(hbox), cb_disk, FALSE, FALSE, 0);

		cb_filt = create_type_cb("Filt Eval", log_type & LOGT_FILT);
		gtk_box_pack_start(GTK_BOX(hbox), cb_filt, FALSE, FALSE, 0);

		cb_bg = create_type_cb("Background", log_type & LOGT_BG);
		gtk_box_pack_start(GTK_BOX(hbox), cb_bg, FALSE, FALSE, 0);

		cb_utility = create_type_cb("Utility", 
		    log_type & LOGT_UTILITY);
		gtk_box_pack_start(GTK_BOX(hbox), cb_utility, FALSE, FALSE, 0);

		cb_net = create_type_cb("Network", 
		    log_type & LOGT_NET);
		gtk_box_pack_start(GTK_BOX(hbox), cb_net, FALSE, FALSE, 0);


		scroll = gtk_scrolled_window_new(NULL, NULL);
		gtk_box_pack_start(GTK_BOX(log_box), scroll, TRUE, TRUE, 0);

		text_view = gtk_text_view_new_with_buffer(text_buf);
		gtk_container_add(GTK_CONTAINER(scroll), text_view);
		
		gtk_widget_show_all(log_window);
	} else {
		/* raise the window if it already exists */
		gdk_window_raise(GTK_WIDGET(log_window)->window);
	}
}


static void
close_log_win()
{
	if (log_window) {
		gtk_object_destroy(GTK_OBJECT(log_window));
	}
}


void
toggle_log_win(ls_search_handle_t shandle)
{
	if (log_window) {
		close_log_win();
	} else {
		open_log_win();
	}
}

void
init_logging()
{
	int	err;

	/* Create a model. */
	text_buf = gtk_text_buffer_new(NULL);

	/*
	 * setup color tags to differnt logging levels to see what is
	 * going on.
	 */
	crit_tag = gtk_text_buffer_create_tag(text_buf, "red_foreground",
		    "foreground", "red", NULL);

	err_tag = gtk_text_buffer_create_tag(text_buf, "maroon_foreground",
		    "foreground", "maroon", NULL);

	info_tag = gtk_text_buffer_create_tag(text_buf, "blue_foreground",
		    "foreground", "darkblue", NULL);

	trace_tag = gtk_text_buffer_create_tag(text_buf, "black_foreground",
		    "foreground", "black", NULL);
	/*
	 * Create a thread that processes gets the statistics.
	 */
	err = pthread_create(&stats_thread, PATTR_DEFAULT, log_main,
	                     (NULL));
	if (err) {
		perror("create log background thread:");
		exit(1);
	}
}
