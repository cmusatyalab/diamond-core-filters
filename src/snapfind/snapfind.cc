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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef linux
#include <getopt.h>
#else
#ifndef HAVE_DECL_GETOPT
#define HAVE_DECL_GETOPT 1
#endif
#include <gnugetopt/getopt.h>
#endif

#include "filter_api.h"
#include "searchlet_api.h"
#include "gui_thread.h"

#include "queue.h"
#include "ring.h"
#include "rtimer.h"
#include "sf_consts.h"

#include "face_search.h"
#include "face_tools.h"
#include "face_image.h"
#include "rgb.h"
#include "face.h"
#include "fil_tools.h"
#include "image_tools.h"
#include "face_widgets.h"
#include "texture_tools.h"
#include "snap_search.h"
#include "sfind_search.h"

/* number of thumbnails to show */
static const int TABLE_COLS = 3;
static const int TABLE_ROWS = 2;

static int default_min_faces = 0;
static int default_face_levels = 37;


/* XXX move these later to common header */
#define	TO_SEARCH_RING_SIZE		512
#define	FROM_SEARCH_RING_SIZE		512


int expert_mode = 0;		/* global (also used in face_widgets.c) */
int dump_attributes = 0;
char *dump_spec_file = NULL;		/* dump spec file and exit */
int dump_objects = 0;		/* just dump all the objects and exit (no gui) */
GtkTooltips *tooltips = NULL;
char *read_spec_filename = NULL;



typedef struct export_threshold_t {
  char *name;
  double distance;
  int index;			/* index into scapes[] */
  TAILQ_ENTRY(export_threshold_t) link;
} export_threshold_t;


TAILQ_HEAD(export_list_t, export_threshold_t) export_list = TAILQ_HEAD_INITIALIZER(export_list);

static struct {
  GtkWidget *main_window;
  
  GtkWidget *min_faces;
  GtkWidget *face_levels;

  GtkWidget *start_button;
  GtkWidget *stop_button;
  GtkWidget *search_box;
  GtkWidget *search_widget;
  GtkWidget *attribute_cb, *attribute_entry;
/* GtkWidget *andorbuttons[2]; */
  
  GtkWidget *scapes_tables[2];

} gui;



/* 
 * scapes entries. sorta contant
 */
/* XXXX help */
#define	MAX_SEARCHES	64
static snap_search * snap_searches[MAX_SEARCHES];
static int num_searches = 0;


static lf_fhandle_t fhandle = 0;	/* XXX */


/* ******************************************************************************** */

/*
 * state required to support popup window to show fullsize img
 */

enum layers_t {
	IMG_LAYER = 0,
	RES_LAYER,
	HIGHLIGHT_LAYER,
	SELECT_LAYER,
	MAX_LAYERS
};


struct {
  GtkWidget 	*window;
  image_hooks_t   *hooks;
  GtkWidget 	*drawing_area;
  GdkPixbuf       *pixbufs[MAX_LAYERS];
  RGBImage        *layers[MAX_LAYERS];
  GtkWidget       *statusbar;
  GtkWidget 	*scroll;
  GtkWidget 	*image_area;
  GtkWidget 	*face_cb_area;
  GtkWidget 	*histo_cb_area;
  int             nfaces;
  GtkWidget *select_button;
  int x1, y1, x2, y2;
  int button_down;
  GtkWidget *scape_entry;
  GtkWidget *color, *texture;
  bbox_t selections[MAX_SELECT];
  int    nselections;

} popup_window = {NULL, NULL, NULL};

/* ********************************************************************** */

/* some stats for user study */

struct {
  int total_seen, total_marked;
} user_measurement = { 0, 0 };

/* ********************************************************************** */
static const int THUMBSIZE_X = 200;
static const int THUMBSIZE_Y = 150;

/* this is a misnomer; the thumbnail keeps everything we want to know
 * about an image */
typedef struct thumbnail_t {
	RGBImage 	*img;	/* the thumbnail image */
	GtkWidget 	*viewport; /* the viewport that contains the image */
	GtkWidget 	*gimage; /* the image widget */
	TAILQ_ENTRY(thumbnail_t) link;
	char 		name[MAX_NAME];	/* name of image */
	int		nboxes;	/* number of histo regions */
	int             nfaces;	/* number of faces */
	//ls_obj_handle_t ohandle;
	//RGBImageRC	*fullimage; /* the full-sized image */
	image_hooks_t   *hooks;
        int              marked; /* if the user 'marked' this thumbnail */
        GtkWidget       *frame;
} thumbnail_t;

typedef TAILQ_HEAD(thumblist_t, thumbnail_t) thumblist_t;
static thumblist_t thumbnails = TAILQ_HEAD_INITIALIZER(thumbnails);
static thumbnail_t *cur_thumbnail = NULL;

static void clear_thumbnails();
/* ******************************************************************************** */



typedef enum {
	CNTRL_ELEV,
	//CNTRL_SLOPE,
	CNTRL_WAIT,
	CNTRL_NEXT,
} control_ops_t;

typedef	 struct {
	GtkWidget *	parent_box;
	GtkWidget *	control_box;
	GtkWidget *	next_button;
	GtkWidget *	zbutton;
	control_ops_t 	cur_op;
	int	 	zlevel;
} image_control_t;

typedef struct {
	GtkWidget *	parent_box;
	GtkWidget *	info_box1;
	GtkWidget *	info_box2;
	GtkWidget *	name_tag;
	GtkWidget *	name_label;
	GtkWidget *	count_tag;
	GtkWidget *	count_label;

	GtkWidget *     qsize_label; /* no real need to save this */
	GtkWidget *     tobjs_label; /* Total objs being searche */ 
	GtkWidget *     sobjs_label; /* Total objects searched */
	GtkWidget *     dobjs_label; /* Total objects dropped */
} image_info_t;


/* XXX */
static image_control_t		image_controls;
static image_info_t		image_information;


/*
 * some globals that we need to find a place for
 */
ring_data_t *	to_search_thread;
ring_data_t *	from_search_thread;
int		pend_obj_cnt = 0;
int		tobj_cnt = 0;
int		sobj_cnt = 0;
int		dobj_cnt = 0;

static pthread_t	display_thread_info;
static int		display_thread_running = 0;

/* 
 * global state used for highlighting (running filters locally)
 */
static struct {
	pthread_mutex_t mutex;
	int 		thread_running;
	pthread_t 	thread;
	GtkWidget       *progress_bar;
} highlight_info = { PTHREAD_MUTEX_INITIALIZER, 0 };


/*
 * Display the cond variables.
 */
static pthread_cond_t	display_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t	display_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	ring_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t	thumb_mutex = PTHREAD_MUTEX_INITIALIZER;



/* 
 * some prototypes 
 */

static region_t draw_bounding_box(RGBImage *img, int scale, 
				  lf_fhandle_t fhandle, ls_obj_handle_t ohandle,
				  RGBPixel color, RGBPixel mask, char *fmt, int i);
static GtkWidget *describe_hbbox(lf_fhandle_t fhandle, ls_obj_handle_t ohandle,
					int i, GtkWidget **button);
static GtkWidget *make_gimage(RGBImage *img, int w, int h);

/* from read_config.l */
extern int read_search_config(char *fname, snap_search **list, int *num);

/* from face_search.c */
extern void drain_ring(ring_data_t *ring);

static void read_histo_tabs(fsp_histo_t *fsp, int i);
static void *histo_scan_main(void *ptr);
static void highlight_progress_f(void *widget, int val, int total);
static void kill_highlight_thread(int run);
static void cb_draw_res_layer(GtkWidget *widget, gpointer ptr);


/* ********************************************************************** */

struct collection_t {
  char *name;
  //int id;
  int active;
};

struct collection_t collections[MAX_ALBUMS+1] = {
/*   {"Local",   1, 0}, */
/*   {"All remote",   2, 1}, */
/*   {"diamond01 only", 3, 0}, */
/*   {"diamond02 only",  4, 0}, */
/*   {"diamond03 only",  5, 0}, */
/*   {"diamond04 only",  6, 0}, */
  {NULL}
};




/* ********************************************************************** */
/* utility functions */
/* ********************************************************************** */


/*
 * make pixbuf from img
 */
static GdkPixbuf*
pb_from_img(RGBImage *img) {
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
 * make a gtk image from an img
 */
static GtkWidget *
make_gimage(RGBImage *img, int dest_width, int dest_height) {
  GdkPixbuf *pbuf; // *scaled_pbuf;

	//fprintf(stderr, "gimage called\n");

	GUI_THREAD_CHECK(); 

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

//	scaled_pbuf = gdk_pixbuf_scale_simple(pbuf, dest_width, dest_height, GDK_INTERP_BILINEAR);

	GtkWidget *image;
	//image = gtk_image_new_from_pixbuf(scaled_pbuf);
	image = gtk_image_new_from_pixbuf(pbuf);
	assert(image);

	return image;
}


/* 
 * read i-th entry values from widgets into fsp struct.
 */
static void
read_histo_tabs(fsp_histo_t *fsp, int i) 
{
	double d;

	GUI_THREAD_CHECK(); 

	/* negate */
#ifdef	XXX_NOW
	fsp->negate = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(scapes[i].notb));
	/* distance */
	d = (100.0 - gtk_range_get_value(GTK_RANGE(scapes[i].slider))) / 100;
	if(fsp->negate) {
		fsp->ndistance = d;
	} else {
		fsp->pdistance = d;
	}

	/* dx, dy */
	fsp->dx = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(scapes[i].xstride));
	fsp->dy = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(scapes[i].ystride));
	
#endif
}


static void
clear_image_info(image_info_t *img_info)
{
	char	data[BUFSIZ];

	GUI_THREAD_CHECK(); 

	sprintf(data, "%-60s", " ");
	gtk_label_set_text(GTK_LABEL(img_info->name_label), data);

	sprintf(data, "%-3s", " ");
	gtk_label_set_text(GTK_LABEL(img_info->count_label), data);

	//gtk_label_set_text(GTK_LABEL(img_info->qsize_label), data);

}


static void
write_image_info(image_info_t *img_info, char *name, int count)
{
	char	txt[BUFSIZ];

	GUI_THREAD_CHECK(); 

	sprintf(txt, "%-60s", name);
	gtk_label_set_text(GTK_LABEL(img_info->name_label), txt);

	sprintf(txt, "%-3d", count);
	gtk_label_set_text(GTK_LABEL(img_info->count_label), txt);
}


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

	
/*
 * This enables all the buttons in the image control section
 * of the display.  This will be called when there is a new image to 
 * manipulated.
 */
static void
enable_image_control(image_control_t *img_cntrl)
{

	GUI_THREAD_CHECK(); 
	gtk_widget_set_sensitive(img_cntrl->next_button, TRUE);
	gtk_widget_grab_default(img_cntrl->next_button);

}


/* 
 * draw a bounding box into image at scale. bbox is read from object(!)
 */
static region_t
draw_bounding_box(RGBImage *img, int scale, 
		  lf_fhandle_t fhandle, ls_obj_handle_t ohandle,
		  RGBPixel color, RGBPixel mask, char *fmt, int i) {
	search_param_t 	param;	
	int 		err;
	bbox_t		bbox;

	err = read_param(fhandle, ohandle, fmt, &param, i);
				
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



char *
build_filter_spec(char *tmp_file, topo_region_t *main_region)
{

	char * 		tmp_storage = NULL;
	FILE *		fspec;	
	int		err;
	int             fd;
	snap_search *		snapobj;
	int					i;

	tmp_storage = (char *)malloc(L_tmpnam);	/* where is the free for this? XXX */
	if (tmp_storage == NULL) {
		printf("XXX failed to alloc memory !!! \n");
		return(NULL);
	}

	if(!tmp_file) {
		tmp_file = tmp_storage;
		sprintf(tmp_storage, "%sXXXXXX", "/tmp/filspec");
		fd = mkstemp(tmp_storage);
	} else {
		fd = open(tmp_file, O_RDWR|O_CREAT|O_TRUNC, 0666);
	}
		
	if(fd < 0) { 
		perror(tmp_file);
		free(tmp_storage);
		return NULL;
	}
	fspec = fdopen(fd, "w+");
	if (fspec == NULL) {
		perror(tmp_file);
		free(tmp_storage);
		return(NULL);
	}

	/* XXX empty dependency list */

	for (i = 0; i < num_searches ; i++) {
		snapobj = snap_searches[i];
		if (snapobj->is_selected()) {
			snapobj->save_edits();
			snapobj->write_fspec(fspec);
		}
	}

	/* XXX write the dependy list */

	err = fclose(fspec);	/* closes fd as well */
	if (err != 0) {
		printf("XXX failed to close file \n");
		free(tmp_storage);
		return(NULL);
	}
	return(tmp_file);
}

/* ********************************************************************** */
/* callback functions */
/* ********************************************************************** */



static void 
cb_next_image(GtkButton* item, gpointer data)
{
	GUI_CALLBACK_ENTER();
	image_controls.zlevel = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON(image_controls.zbutton));
	clear_thumbnails();
	GUI_CALLBACK_LEAVE(); /* need to put this here instead of at
				 end because signal wakes up another
				 thread immediately... */

	pthread_mutex_lock(&display_mutex);
	image_controls.cur_op = CNTRL_NEXT;
	pthread_cond_signal(&display_cond);
	pthread_mutex_unlock(&display_mutex);

}



static void
cb_img_info(GtkWidget *widget, gpointer data) 
{
	thumbnail_t *thumb;

	GUI_CALLBACK_ENTER();
	thumb = (thumbnail_t *)gtk_object_get_user_data(GTK_OBJECT(widget));

	/* the data gpointer passed in seems to not be the data
	 * pointer we gave gtk. instead, we save a pointer in the
	 * widget. -RW */

	//fprintf(stderr, "thumb=%p\n", thumb);

	if(thumb->img) {
		write_image_info(&image_information, thumb->name, thumb->nboxes);
	}
	GUI_CALLBACK_LEAVE();
}


static void
cb_popup_window_close(GtkWidget *window) {
/* 	image_hooks_t *hooks; */
/* 	hooks = (image_hooks_t *)gtk_object_get_user_data(GTK_OBJECT(window)); */
/* 	ih_drop_ref(hooks); */

        GUI_CALLBACK_ENTER();

	kill_highlight_thread(0);

	popup_window.window = NULL;
	ih_drop_ref(popup_window.hooks, fhandle);
	popup_window.hooks = NULL;

        GUI_CALLBACK_LEAVE();
}




static void
remove_func(GtkWidget *widget, void *container) 
{
  GUI_THREAD_CHECK(); 
  gtk_container_remove(GTK_CONTAINER(container), widget);
}


static gboolean
cb_test_region(GtkWidget *widget, GdkEventButton *event, gpointer ptr) 
{

	GUI_CALLBACK_ENTER();
	
	char buf[BUFSIZ];
	sprintf(buf, "mouse click at %.2f, %.2f", event->x, event->y);
	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(popup_window.statusbar),
						"test region");
	gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, buf);


        GUI_CALLBACK_LEAVE();
	return TRUE;
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
			ih_drop_ref(popup_window.hooks, fhandle);
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
	gtk_widget_queue_draw_area(popup_window.drawing_area,
				   0, 0,
				   img->width,
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

	/* XXX */
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(highlight_info.progress_bar), 0);

        GUI_CALLBACK_LEAVE();
}

static void
cb_run_highlight()
{

	GUI_CALLBACK_ENTER();

	kill_highlight_thread(1);
	int err = pthread_create(&highlight_info.thread, PATTR_DEFAULT, histo_scan_main, NULL);
	assert(!err);

	GUI_CALLBACK_LEAVE();
}

static inline int
min(int a, int b) { 
	return ( (a < b) ? a : b );
}
static inline int
max(int a, int b) { 
	return ( (a > b) ? a : b );
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

	gdk_window_clear_area (widget->window,
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

	gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
				   NULL);

 done:
	return TRUE;
}

static gboolean
realize_event(GtkWidget *widget, GdkEventAny *event, gpointer data) {
	
	assert(widget == popup_window.drawing_area);

	for(int i=0; i<MAX_LAYERS; i++) {
		popup_window.pixbufs[i] = pb_from_img(popup_window.layers[i]);
	}
	
	return TRUE;
}

#define COORDS_TO_BBOX(bbox,container) 		\
{						\
  bbox.min_x = min(container.x1, container.x2);	\
  bbox.max_x = max(container.x1, container.x2);	\
  bbox.min_y = min(container.y1, container.y2);	\
  bbox.max_y = max(container.y1, container.y2);	\
}


static gboolean
cb_grab_selection(GtkWidget *widget, GdkEventAny *event, gpointer data) 
{
	int err;
	char buf[BUFSIZ] = "created new scene";
	int n, row;
#ifdef	XXX_NOW
	fsp_histo_t *fsp = &scapes[nscapes].fsp_info;
  int color;			/* if color (else texture) */

  GUI_CALLBACK_ENTER();
  color = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(popup_window.color)));

  if(nscapes >= MAX_SCAPE) {
    sprintf(buf, "ERROR too many scapes!");
    err = 1;
    goto done;
  }

  row = nscapes + 1;	/* 1 is heading */
  n = nscapes;

  memset(&scapes[nscapes].fsp_info, 0, sizeof(fsp_histo_t));
  TAILQ_INIT(&scapes[nscapes].fsp_info.patchlist);
  TAILQ_INIT(&scapes[nscapes].fsp_info.texture_features_list);
  scapes[nscapes].fsp_info.npatches = 0;

  if(color) {
    fsp->xsiz = STD_COLOR_SIZE;
    fsp->ysiz = STD_COLOR_SIZE;
    fsp->dx = fsp->dy = STD_COLOR_SIZE;
    fsp->type = FILTER_TYPE_COLOR;
  } else {
    fsp->xsiz = MIN_TEXTURE_SIZE;
    fsp->ysiz = MIN_TEXTURE_SIZE;
    fsp->dx = fsp->dy = MIN_TEXTURE_SIZE;
    fsp->type = FILTER_TYPE_TEXTURE;
  }
  set_fsp_defaults(fsp);

  {
    const char *name = gtk_entry_get_text(GTK_ENTRY(popup_window.scape_entry));
    if(strlen(name) < 1) {
      sprintf(buf, "ERROR bad name!");
      goto done;
    }
    scapes[nscapes].name = strdup(name);

	{ 
		size_t i;
		for (i=0; i < strlen(scapes[nscapes].name); i++) {
			if (scapes[nscapes].name[i] == ' ') {
				scapes[nscapes].name[i] = '_';
			}
		}
	}
    scapes[nscapes].file = NULL;
  }

  for(int i=0; i<popup_window.nselections; i++) {
      if(color) {
		err = img_inst_histogram(popup_window.hooks->img, 
				 popup_window.selections[i],
				 &scapes[nscapes].fsp_info);
      } else {
#ifdef	XXX_ASK_RAHUL
	err = texture_inst_features_from_img(popup_window.hooks->img, 
					     popup_window.selections[i],
					     &scapes[nscapes].fsp_info);
#endif
      }
      if(err) {
	sprintf(buf, "ERROR creating scene (patch too small?)");    
	goto done;
      }
  }

  /* now we are done. */
  nscapes++;

  add_scape_widgets1(gui.scapes_tables[0], row, &scapes[n]);
  add_scape_widgets2(gui.scapes_tables[1], row, &scapes[n]);

#endif
done:
  guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(popup_window.statusbar),
					  "selection");
  gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, buf);
  
  GUI_CALLBACK_LEAVE();
  return TRUE;
}


static void
clear_selection( GtkWidget *widget ) {
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
redraw_selections() {

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
draw_selection( GtkWidget *widget ) {
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

  gtk_widget_grab_focus (popup_window.select_button);


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


static void
do_img_popup(GtkWidget *widget) {
        thumbnail_t *thumb;
	GtkWidget *eb;
	GtkWidget *frame;
	GtkWidget *image;


	thumb= (thumbnail_t *)gtk_object_get_user_data(GTK_OBJECT(widget));

	/* the data gpointer passed in seems to not be the data
	 * pointer we gave gtk. instead, we save a pointer in the
	 * widget. (maybe the prototype didn't match) -RW */

	if(!thumb->img) goto done;

	if(!popup_window.window) {
		popup_window.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(popup_window.window), "Image");
		gtk_window_set_default_size(GTK_WINDOW(popup_window.window), 750, 350);
		g_signal_connect (G_OBJECT (popup_window.window), "destroy",
				  G_CALLBACK (cb_popup_window_close), NULL);

		GtkWidget *box1 = gtk_vbox_new(FALSE, 0);

		popup_window.statusbar = gtk_statusbar_new();
		gtk_box_pack_end(GTK_BOX(box1), popup_window.statusbar, FALSE, FALSE, 0);
		gtk_widget_show(popup_window.statusbar);

		GtkWidget *pane = gtk_hpaned_new();
		gtk_box_pack_start(GTK_BOX(box1), pane, TRUE, TRUE, 0);
		gtk_widget_show(pane);

		gtk_container_add(GTK_CONTAINER(popup_window.window), box1);
		gtk_widget_show(box1);

		/* box to hold controls */
		box1 = gtk_vbox_new(FALSE, 10);
		gtk_container_set_border_width(GTK_CONTAINER(box1), 4);
		gtk_widget_show(box1);
		gtk_paned_pack1(GTK_PANED(pane), box1, FALSE, TRUE);

		frame = gtk_frame_new("Search Results");
		gtk_box_pack_start(GTK_BOX(box1), frame, FALSE, FALSE, 0);
		gtk_widget_show(frame);
		GtkWidget *box3 = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box3);
		gtk_widget_show(box3);

		popup_window.face_cb_area = gtk_vbox_new(FALSE, 0);
		popup_window.histo_cb_area = gtk_vbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX (box3), popup_window.face_cb_area, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX (box3), popup_window.histo_cb_area, FALSE, FALSE, 0);
		gtk_widget_show(popup_window.face_cb_area);
		gtk_widget_show(popup_window.histo_cb_area);

		/* 
		 * Refinement
		 */			
		{
		  frame = gtk_frame_new("Refinement");
		  gtk_box_pack_end (GTK_BOX (box1), frame, FALSE, FALSE, 0);
		  gtk_widget_show(frame);
		  
		  GtkWidget *box2 = gtk_vbox_new (FALSE, 10);
		  gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
		  gtk_container_add(GTK_CONTAINER(frame), box2);
		  gtk_widget_show (box2);
		  
		  /* hbox */
		  GtkWidget *hbox = gtk_hbox_new(FALSE, 10);
		  gtk_box_pack_start (GTK_BOX(box2), hbox, TRUE, TRUE, 0);
		  gtk_widget_show(hbox);

		  widget = gtk_label_new("Name");
		  gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
		  gtk_widget_show(widget);

		  popup_window.scape_entry = gtk_entry_new();
		  gtk_widget_show(popup_window.scape_entry);
		  gtk_entry_set_activates_default(GTK_ENTRY(popup_window.scape_entry),
						  TRUE);
		  gtk_box_pack_start (GTK_BOX (hbox), popup_window.scape_entry,
				      TRUE, TRUE, 0);

		  /* hbox */
		  hbox = gtk_hbox_new(FALSE, 10);
		  gtk_box_pack_start (GTK_BOX(box2), hbox, FALSE, FALSE, 0);
		  gtk_widget_show(hbox);

		  widget = gtk_label_new("Type");
		  gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
		  gtk_widget_show(widget);

		  popup_window.color = gtk_radio_button_new_with_label(NULL, "Color");
		  popup_window.texture = gtk_radio_button_new_with_label_from_widget
		    (GTK_RADIO_BUTTON(popup_window.color), "Texture");
		  gtk_widget_show(popup_window.color);
		  gtk_widget_show(popup_window.texture);
		  gtk_box_pack_start (GTK_BOX (hbox), popup_window.color,
				      FALSE, FALSE, 0);
		  gtk_box_pack_start (GTK_BOX (hbox), popup_window.texture,
				      FALSE, FALSE, 0);

		  /* couple of buttons */
		  GtkWidget *buttonbox = gtk_hbox_new(TRUE, 10);
		  gtk_box_pack_start (GTK_BOX(box2), buttonbox, TRUE, TRUE, 0);
		  gtk_widget_show(buttonbox);
		  
		  GtkWidget *button = gtk_button_new_with_label ("Make Patch");
		  popup_window.select_button = button;
		  g_signal_connect_after(GTK_OBJECT(button), "clicked",
					 GTK_SIGNAL_FUNC(cb_grab_selection), NULL);
		  gtk_box_pack_start (GTK_BOX (buttonbox), button, TRUE, TRUE, 0);
		  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
		  gtk_widget_show (button);
		  
		  button = gtk_button_new_with_label ("Clear");
		  g_signal_connect_after(GTK_OBJECT(button), "clicked",
					 GTK_SIGNAL_FUNC(cb_clear_select), NULL);
		  gtk_box_pack_start (GTK_BOX (buttonbox), button, TRUE, TRUE, 0);
		  gtk_widget_show (button);
		}

		/* 
		 * Highlighting
		 */
		{
		  frame = gtk_frame_new("Highlighting");
		  gtk_box_pack_end (GTK_BOX (box1), frame, FALSE, FALSE, 0);
		  gtk_widget_show(frame);

		  /* start button area */
		  GtkWidget *box2 = gtk_vbox_new (FALSE, 10);
		  gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
		  gtk_container_add(GTK_CONTAINER(frame), box2);
		  gtk_widget_show (box2);

		  GtkWidget *label = gtk_label_new("This will run color histogram matching"
			  " over the image using parameters from the main search window.");
		  gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		  gtk_box_pack_start (GTK_BOX (box2), label, TRUE, TRUE, 0);
		  gtk_widget_show (label);

		  GtkWidget *pb = gtk_progress_bar_new();
		  gtk_widget_show(pb);
		  highlight_info.progress_bar = pb;
		  gtk_box_pack_start (GTK_BOX (box2), pb, TRUE, TRUE, 0);

		  /* couple of buttons */
		  GtkWidget *buttonbox = gtk_hbox_new(TRUE, 10);
		  gtk_box_pack_start (GTK_BOX(box2), buttonbox, TRUE, TRUE, 0);
		  gtk_widget_show(buttonbox);

		  GtkWidget *button = gtk_button_new_with_label ("Highlight");
		  g_signal_connect_after(GTK_OBJECT(button), "clicked",
					 GTK_SIGNAL_FUNC(cb_run_highlight), NULL);
		  gtk_box_pack_start (GTK_BOX (buttonbox), button, TRUE, TRUE, 0);
		  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
		  gtk_widget_show (button);

		  button = gtk_button_new_with_label ("Clear");
		  g_signal_connect_after(GTK_OBJECT(button), "clicked",
					 GTK_SIGNAL_FUNC(cb_clear_highlight_layer),
					 NULL);
		  gtk_box_pack_start (GTK_BOX (buttonbox), button, TRUE, TRUE, 0);
		  gtk_widget_show (button);
		}

		popup_window.image_area = gtk_viewport_new(NULL, NULL);
		gtk_widget_show(popup_window.image_area);

		gtk_paned_pack2(GTK_PANED(pane), popup_window.image_area, TRUE, TRUE);

	} else {
		kill_highlight_thread(0);
		
		/* XXX */
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(highlight_info.progress_bar), 0);
		gtk_container_remove(GTK_CONTAINER(popup_window.image_area), 
				     popup_window.scroll);
		ih_drop_ref(popup_window.hooks, fhandle);

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
	gtk_container_foreach(GTK_CONTAINER(popup_window.face_cb_area), remove_func, 
			      popup_window.face_cb_area);
	gtk_container_foreach(GTK_CONTAINER(popup_window.histo_cb_area), remove_func, 
			      popup_window.histo_cb_area);

	{
	    GtkWidget *button = NULL;

	    popup_window.nfaces = thumb->nfaces;
	    if(thumb->nfaces) {
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
		widget = describe_hbbox(fhandle, popup_window.hooks->ohandle, 
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

static void
do_img_mark(GtkWidget *widget) {
  thumbnail_t *thumb;

  thumb= (thumbnail_t *)gtk_object_get_user_data(GTK_OBJECT(widget));

  thumb->marked ^= 1;

  /* adjust count */
  user_measurement.total_marked += (thumb->marked ? 1 : -1);

  printf("marked count = %d/%d\n", user_measurement.total_marked, 
	 user_measurement.total_seen);
  
  gtk_frame_set_label(GTK_FRAME(thumb->frame),
		      (thumb->marked) ? "marked" : "");
}

static void
cb_img_popup(GtkWidget *widget, GdkEventButton *event, gpointer data) {

  GUI_CALLBACK_ENTER();

  /* dispatch based on the button pressed */
  switch(event->button) {
  case 1:
    do_img_popup(widget);
    break;
  case 3:
    do_img_mark(widget);
  default:
    break;
  }
  
  GUI_CALLBACK_LEAVE();
}



static int
timeout_write_qsize(gpointer label)
{
	char	txt[BUFSIZ];
	int     qsize;

	qsize = pend_obj_cnt;
	sprintf(txt, "Pending images = %-6d", qsize);

	GUI_THREAD_ENTER(); 
	gtk_label_set_text(GTK_LABEL(label), txt);
	GUI_THREAD_LEAVE();

	return TRUE;
}

static int
timeout_write_tobjs(gpointer label)
{
	char	txt[BUFSIZ];
	int     tobjs;

	tobjs = tobj_cnt;
	sprintf(txt, "Total Objs = %-6d ", tobjs);

	GUI_THREAD_ENTER(); 
	gtk_label_set_text(GTK_LABEL(label), txt);
	GUI_THREAD_LEAVE();
	return TRUE;
}

static int
timeout_write_sobjs(gpointer label)
{
	char	txt[BUFSIZ];
	int     sobjs;

	sobjs = sobj_cnt;
	sprintf(txt, "Searched Objs = %-6d ", sobjs);

	GUI_THREAD_ENTER(); 
	gtk_label_set_text(GTK_LABEL(label), txt);
	GUI_THREAD_LEAVE();
	return TRUE;
}

static int
timeout_write_dobjs(gpointer label)
{
	char	txt[BUFSIZ];
	int     dobjs;

	dobjs = dobj_cnt;
	sprintf(txt, "Dropped Objs = %-6d ", dobjs);

	GUI_THREAD_ENTER(); 
	gtk_label_set_text(GTK_LABEL(label), txt);
	GUI_THREAD_LEAVE();
	return TRUE;
}




/* ********************************************************************** */
/* ********************************************************************** */

static void
highlight_box_f(void *cont, search_param_t *param) {
	RGBImage *img = (RGBImage *)cont;
	bbox_t bbox;

	//fprintf(stderr, "found bbox\n");

	
	bbox.min_x = param->bbox.xmin;
	bbox.min_y = param->bbox.ymin;
	bbox.max_x = param->bbox.xmin + param->bbox.xsiz - 1;
	bbox.max_y = param->bbox.ymin + param->bbox.ysiz - 1;

	image_fill_bbox_scale(img, &bbox, 1, hilitMask, hilit);

	GUI_THREAD_ENTER();
	gtk_widget_queue_draw_area(popup_window.drawing_area,
				   param->bbox.xmin, param->bbox.ymin,
				   param->bbox.xsiz, param->bbox.ysiz);
	GUI_THREAD_LEAVE();
}

static void
highlight_box_f(RGBImage *img, bbox_t bbox) {
	image_fill_bbox_scale(img, &bbox, 1, hilitMask, hilit);

	GUI_THREAD_ENTER();
	gtk_widget_queue_draw_area(popup_window.drawing_area,
				   bbox.min_x, bbox.min_y,
				   bbox.max_x, bbox.max_y);
	GUI_THREAD_LEAVE();
}

/* horribly expensive, but... */
static void
highlight_progress_f(void *widget, int val, int total) 
{
	double fraction;

	fraction = (double)val / total;

	GUI_THREAD_ENTER(); 
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), fraction);
	GUI_THREAD_LEAVE();
}


static void *
histo_scan_main(void *ptr) 
{
	int insp=0, pass=0;
	int err;

	err = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	assert(!err);
	err = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	assert(!err);

	ih_get_ref(popup_window.hooks);
	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(popup_window.statusbar),
						"histo");
#ifdef	XXX_NOW
	for(int i=0; i<nscapes; i++) {
	  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(scapes[i].cb))) {

		  /* XXX this is probably unsafe (pointers in struct) */
 		  fsp_histo_t fsp = scapes[i].fsp_info;

		  /* set the values in the fsp_histo_t we are sending to the search thread.
		   * not reqd if we updated defaults above */
		  GUI_THREAD_ENTER();
		  read_histo_tabs(&fsp, i);
		  GUI_THREAD_LEAVE();

		  /* update statusbar */
		  char buf[BUFSIZ];
		  sprintf(buf, "host scan (%s)...", scapes[i].name);
		  GUI_THREAD_ENTER();
		  gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, buf);
		  GUI_THREAD_LEAVE();

		   /* XXX */
		  patch_t *patch;
		  double distance = (fsp.negate ? fsp.ndistance : fsp.pdistance);
		  if (fsp.type == FILTER_TYPE_TEXTURE) {
		    distance = distance*NUM_LAP_PYR_LEVELS;
		    
		    IplImage* ipl_img;
		    IplImage* dst_img;
		    if (TEXTURE_NUM_CHANNELS==1) {
		      ipl_img = get_gray_ipl_image(popup_window.hooks->img);
		    }
		    else if (TEXTURE_NUM_CHANNELS == 3) {
		      ipl_img = get_rgb_ipl_image(popup_window.hooks->img);
		    }
		    dst_img = cvCreateImage(cvSize(ipl_img->width, ipl_img->height), IPL_DEPTH_8U, 1);
		    cvZero(dst_img);
		    
		    double **samples = new double*[fsp.npatches];
		    texture_features_t* texture_patch;
		    int s = 0;
		    TAILQ_FOREACH(texture_patch, &fsp.texture_features_list, link) {
		      samples[s] = new double[NUM_LAP_PYR_LEVELS*TEXTURE_NUM_CHANNELS];
		      for (int i=0; i<NUM_LAP_PYR_LEVELS*TEXTURE_NUM_CHANNELS; i++) {
			samples[s][i] = texture_patch->feature_values[i];
		      }
		      s++;
		    }
		    int minx, miny;
		    texture_test_entire_image(ipl_img, fsp.npatches, samples, distance, 0, fsp.dx, fsp.dy,
					      fsp.xsiz, fsp.ysiz, &minx, &miny, dst_img);

		    double pass_box;
		    bbox_t bbox;
		    insp = dst_img->width/fsp.dx * dst_img->height/fsp.dy;
		    pass = 0;
		    int num_seen=0;
		    for (int x=0; (x+fsp.xsiz)<dst_img->width; x++) {
		      for (int y=0; (y+fsp.ysiz)<dst_img->height; y++) {
			num_seen++;
			pass_box = cvGetReal2D(dst_img, y, x);
			if (pass_box) {
			  pass++;
			  bbox.min_x = x;
			  bbox.min_y = y;
			  bbox.max_x = x + fsp.xsiz -1;
			  bbox.max_y = y + fsp.ysiz -1;
			  highlight_box_f(popup_window.layers[HIGHLIGHT_LAYER], bbox);
			}
			//highlight_progress_f(highlight_info.progress_bar, num_seen/(fsp.dx*fsp.dy), insp);			
		      }
		    }
		    cvReleaseImage(&dst_img);
		    cvReleaseImage(&ipl_img);
		    for (int i=0; i<s; i++) {
		      delete[] samples[i];
		    }
		    delete[] samples;
		  }
		  
		  else if (fsp.type == FILTER_TYPE_COLOR) {
		    TAILQ_FOREACH(patch, &fsp.patchlist, link) {
		      patch->threshold = distance; 
		    }
		    if(!popup_window.hooks->histo_ii) {
		      popup_window.hooks->histo_ii =
		      (HistoII *)ft_read_alloc_attr(fhandle, 
						    popup_window.hooks->ohandle,
						    HISTO_II);
		    }

		    assert(insp >= pass);
		    histo_scan_image(scapes[i].name,
				     popup_window.hooks->img,
				     popup_window.hooks->histo_ii,
				     &fsp,
				     INT_MAX, /* get all matching bboxes */
				     &insp, &pass,
				     highlight_box_f, popup_window.layers[HIGHLIGHT_LAYER],
				     highlight_progress_f, highlight_info.progress_bar);
		    assert(insp >= pass); 
		  }
			   
		  //fprintf(stderr, "threshold = %.2f\n", distance);
		  highlight_progress_f(highlight_info.progress_bar, 1, 1);

		  GUI_THREAD_ENTER();
		  /* update statusbar */
		  gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, "ready.");
		  GUI_THREAD_LEAVE();
	  }
	}
#endif


	pthread_mutex_lock(&highlight_info.mutex);	
	highlight_info.thread_running = 0;
	ih_drop_ref(popup_window.hooks, fhandle);
	pthread_mutex_unlock(&highlight_info.mutex);	

	/* update statusbar */
	GUI_THREAD_ENTER();
	char buf[BUFSIZ];
	sprintf(buf, "highlighting complete; passed %d of %d area tests (%.0f%%).",
		pass, insp, 100.0 * pass / insp);
	gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, buf);
	GUI_THREAD_LEAVE();

	pthread_exit(NULL);
	return NULL;
}


static void
draw_hbbox_func(GtkWidget *widget, void *ptr) {
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

	region = draw_bounding_box(popup_window.layers[RES_LAYER], 1, fhandle,
				   popup_window.hooks->ohandle,
				   color, mask, HISTO_BBOX_FMT, i);
	
	/* refresh */
	gtk_widget_queue_draw_area(popup_window.drawing_area,
				   region.xmin, region.ymin,
				   region.xsiz, region.ysiz);
}

static void
draw_face_func(GtkWidget *widget, void *ptr) {
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
		region = draw_bounding_box(popup_window.layers[RES_LAYER], 1, fhandle,
					   popup_window.hooks->ohandle,
					   color, mask, FACE_BBOX_FMT, i);
		/* refresh */
		gtk_widget_queue_draw_area(popup_window.drawing_area,
					   region.xmin, region.ymin,
					   region.xsiz, region.ysiz);

	}

}


/* draw all the bounding boxes */
static void
cb_draw_res_layer(GtkWidget *widget, gpointer ptr) {

        GUI_CALLBACK_ENTER();
	
	/* although we clear the pixbuf data here, we still need to
	 * generate refreshes for either the whole image or the parts
	 * that we cleared. */
	rgbimg_clear(popup_window.layers[RES_LAYER]);

	/* draw faces (presumably there's only one checkbox, but that's ok) */
	gtk_container_foreach(GTK_CONTAINER(popup_window.face_cb_area), draw_face_func, 
			      popup_window.face_cb_area);

	/* draw histo bboxes, if on */
	gtk_container_foreach(GTK_CONTAINER(popup_window.histo_cb_area), draw_hbbox_func, 
			      popup_window.histo_cb_area);

        GUI_CALLBACK_LEAVE();
}






static GtkWidget *
describe_hbbox(lf_fhandle_t fhandle, ls_obj_handle_t ohandle, int i,
		      GtkWidget **button) {
	search_param_t 	param;	
	int 		err;

	GUI_THREAD_CHECK(); 
	
	err = read_param(fhandle, ohandle, HISTO_BBOX_FMT, &param, i);
	if (err) {
/* 		label = gtk_label_new("ERR"); */
/* 		gtk_box_pack_start(GTK_BOX(container), label, TRUE, TRUE, 0); */
/* 		gtk_widget_show(label); */
	} else {
		char buf[BUFSIZ];
		
		if(param.type == PARAM_HISTO) {
			sprintf(buf, "%s (similarity %.0f%%)", 
				param.name,
				100 - 100.0*param.distance);
			*button = gtk_check_button_new_with_label(buf);
			g_signal_connect (G_OBJECT(*button), "toggled",
					  G_CALLBACK(cb_draw_res_layer), GINT_TO_POINTER(i));
			gtk_object_set_user_data(GTK_OBJECT(*button), GINT_TO_POINTER(i));


			gtk_widget_show(*button);
		}
	}

	return *button;
}







static void
display_thumbnail(ls_obj_handle_t ohandle)
{
	RGBImage        *rgbimg, *scaledimg;
	char            name[MAX_NAME];
	off_t		bsize;
	lf_fhandle_t	fhandle = 0; /* XXX */
	int		err;
	int		num_face, num_histo;

	while(image_controls.cur_op == CNTRL_WAIT) {
		fprintf(stderr, "GOT WAIT. waiting...\n");
		pthread_mutex_lock(&display_mutex);
		if(image_controls.cur_op == CNTRL_WAIT) {
			pthread_cond_wait(&display_cond, &display_mutex);
		}
		pthread_mutex_unlock(&display_mutex);
	}

	assert(image_controls.cur_op == CNTRL_NEXT ||
	       image_controls.cur_op == CNTRL_ELEV);

	/* get path XXX */
	bsize = MAX_NAME;
	err = lf_read_attr(fhandle, ohandle, DISPLAY_NAME, &bsize, name);
	if(err) {
	  err = lf_read_attr(fhandle, ohandle, OBJ_PATH, &bsize, name);
	}
	if (err) {
		sprintf(name, "%s", "uknown");
		bsize = strlen(name);
	}
	name[bsize] = '\0';	/* terminate string */


	/* get the img */
	rgbimg = (RGBImage*)ft_read_alloc_attr(fhandle, ohandle, RGB_IMAGE);
	assert(rgbimg);
	assert(rgbimg->width);

	/* figure out bboxes to highlight */
	bsize = sizeof(num_histo);
	err = lf_read_attr(fhandle, ohandle, NUM_HISTO, &bsize, (char *)&num_histo);
	if (err) { num_histo = 0; }
	bsize = sizeof(num_face);
	err = lf_read_attr(fhandle, ohandle, NUM_FACE, &bsize, (char *)&num_face);
	if (err) { num_face = 0; }


	int scale = (int)ceil(compute_scale(rgbimg, THUMBSIZE_X, THUMBSIZE_Y));
	scaledimg = image_gen_image_scale(rgbimg, scale);
	assert(scaledimg);


	for(int i=0; i<num_histo; i++) {
		draw_bounding_box(scaledimg, scale, fhandle, ohandle, 
				  green, colorMask, HISTO_BBOX_FMT, i);
	}
	for(int i=0; i<num_face; i++) {
		draw_bounding_box(scaledimg, scale, fhandle, ohandle, 
				  red, colorMask, FACE_BBOX_FMT, i);
	}

	user_measurement.total_seen++;

	GUI_THREAD_ENTER();

	/*
	 * Build image the new data
	 */
	GtkWidget *image = make_gimage(scaledimg, THUMBSIZE_X, THUMBSIZE_Y);
	assert(image);

	/* 
	 * update the display
	 */

	pthread_mutex_lock(&thumb_mutex);

	if(!cur_thumbnail) {
		cur_thumbnail = TAILQ_FIRST(&thumbnails);
	}
	if(cur_thumbnail->img) { /* cleanup */
		gtk_container_remove(GTK_CONTAINER(cur_thumbnail->viewport), 
				     cur_thumbnail->gimage);
		lf_free_buffer(fhandle, (char *)cur_thumbnail->img); /* XXX */
		//lf_free_buffer(fhandle, (char *)cur_thumbnail->fullimage); /* XXX */
		ih_drop_ref(cur_thumbnail->hooks, fhandle);
		//ls_release_object(fhandle, cur_thumbnail->ohandle);		
	}
	gtk_frame_set_label(GTK_FRAME(cur_thumbnail->frame), "");
	cur_thumbnail->marked = 0;
	cur_thumbnail->img = scaledimg;
	cur_thumbnail->gimage = image;
	strcpy(cur_thumbnail->name, name);
	cur_thumbnail->nboxes = num_histo;
	cur_thumbnail->nfaces = num_face;
	cur_thumbnail->hooks = ih_new_ref(rgbimg, (HistoII*)NULL, ohandle);

	gtk_container_add(GTK_CONTAINER(cur_thumbnail->viewport), image);
	gtk_widget_show_now(image);


	cur_thumbnail = TAILQ_NEXT(cur_thumbnail, link);

	/* check if panel is full */
	if(cur_thumbnail == NULL) {
		pthread_mutex_unlock(&thumb_mutex);

		enable_image_control(&image_controls);
		//fprintf(stderr, "WINDOW FULL. waiting...\n");
		GUI_THREAD_LEAVE();

		/* block until user hits next */
		pthread_mutex_lock(&display_mutex);
		image_controls.cur_op = CNTRL_WAIT;
		pthread_cond_wait(&display_cond, &display_mutex);
		pthread_mutex_unlock(&display_mutex);

		GUI_THREAD_ENTER();
		disable_image_control(&image_controls);
		//fprintf(stderr, "WAIT COMPLETE...\n");
		GUI_THREAD_LEAVE();
	} else {
		pthread_mutex_unlock(&thumb_mutex);
		GUI_THREAD_LEAVE();
	}


}	


static void
clear_thumbnails() 
{
	//thumbnail_t *cur_thumbnail;

	clear_image_info(&image_information);

	pthread_mutex_lock(&thumb_mutex);
	TAILQ_FOREACH(cur_thumbnail, &thumbnails, link) {
		if(cur_thumbnail->img) { /* cleanup */
			gtk_container_remove(GTK_CONTAINER(cur_thumbnail->viewport), 
					     cur_thumbnail->gimage);
			free(cur_thumbnail->img); /* XXX */
			ih_drop_ref(cur_thumbnail->hooks, fhandle);
			cur_thumbnail->img = NULL;
	}
		
	}
	cur_thumbnail = NULL;
	pthread_mutex_unlock(&thumb_mutex);
}


static void *
display_thread(void *data)
{
	message_t *		message;
	struct timespec timeout;


	//fprintf(stderr, "DISPLAY THREAD STARTING\n");

	while (1) {

	        pthread_mutex_lock(&ring_mutex);
		message = (message_t *)ring_deq(from_search_thread);
		pthread_mutex_unlock(&ring_mutex);

		if (message != NULL) {
			switch (message->type) {
			case NEXT_OBJECT:

				//fprintf(stderr, "display obj...\n"); /* XXX */
				display_thumbnail((ls_obj_handle_t)message->data);
				//fprintf(stderr, "display obj done.\n");   /* XXX */
				break;

			case DONE_OBJECTS:
				/*
				 * We are done recieving objects.
				 * We need to disable the thread
				 * image controls and enable start
				 * search button.
				 */

				free(message);
				gtk_widget_set_sensitive(gui.start_button, TRUE);
				gtk_widget_set_sensitive(gui.stop_button, FALSE);
				display_thread_running = 0;
				pthread_exit(0);
				break;
				
			default:
				break;

			}
			free(message);
		}

		timeout.tv_sec = 0;
		timeout.tv_nsec = 10000000; /* XXX 10 ms?? */
		nanosleep(&timeout, NULL);

	}
	return 0;
}



static void
build_search_from_gui(topo_region_t *main_region) 
{

	/* 
	 * figure out the args and build message
	 */
	main_region->min_faces = (int)gtk_range_get_value(GTK_RANGE(gui.min_faces));
	main_region->face_levels = (int)gtk_range_get_value(GTK_RANGE(gui.face_levels));
	main_region->ngids = 0;
	int i;
	for(i=0; i<MAX_ALBUMS && collections[i].name; i++) {
	  /* if collection active, figure out the gids and add to out list
	   * allows duplicates XXX */
	  if(collections[i].active) {
	    int err;
	    int num_gids = MAX_ALBUMS;
	    groupid_t gids[MAX_ALBUMS], *gptr;
	    err = nlkup_lookup_collection(collections[i].name, &num_gids, gids);
	    assert(!err);
	    gptr = gids;
	    while(num_gids) {
	      main_region->gids[main_region->ngids++] = *gptr++;
	      num_gids--;
	    }
	    //printf("gid %d active\n",  collections[i].id);
	  }
	}
#ifdef	XXX
	//assert(main_region->ngids);
	main_region->nscapes = 0;
	assert(nscapes < MAX_SCAPE);
	for(int i=0; i<nscapes; i++) {
	  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(scapes[i].cb))) {
	    main_scape_t *scape;

	    scape = &main_region->scapes[main_region->nscapes++];
	    strncpy(scape->name, scapes[i].name, MAX_NAME);

	    /* XXX this is probably unsafe (pointers in struct) */
	    scape->fsp_info = scapes[i].fsp_info;

	    /* set the values in the fsp_histo_t we are sending to the search thread.
	     * not reqd if we updated defaults above */
	    read_histo_tabs(&scape->fsp_info, i);
	  }
	}
	strncpy(main_region->search_string, gtk_entry_get_text(GTK_ENTRY(gui.attribute_entry)),
		MAX_STRING-1);
	main_region->search_string[MAX_STRING-1] = '\0';

#endif

}

static void 
cb_stop_search(GtkButton* item, gpointer data)
{
	message_t *		message;
	int			err;

        GUI_CALLBACK_ENTER();
	//printf("stop search !! \n");

	/*
	 * Toggle the start and stop buttons.
	 */
    	gtk_widget_set_sensitive(gui.start_button, TRUE);
	/* we should not be messing with the default. this is here so
	 * that we can trigger a search from the text entry without a
	 * return-pressed handler.  XXX */
	gtk_widget_grab_default (gui.start_button);
    	gtk_widget_set_sensitive(gui.stop_button, FALSE);

	message = (message_t *)malloc(sizeof(*message));
	if (message == NULL) {
		printf("failed to allocate message \n");
		exit(1);
	}

	message->type = TERM_SEARCH;
	message->data = NULL;

	err = ring_enq(to_search_thread, message);
	if (err) {
		printf("XXX failed to enq message \n");
		exit(1);
	}
	//fprintf(stderr, "facemain: enq TERM_SEARCH\n");

        GUI_CALLBACK_LEAVE();
}
static void 
cb_start_search(GtkButton* item, gpointer data)
{
	message_t *		message;
	int			err;

        GUI_CALLBACK_ENTER();
	printf("starting search !! \n");

	/*
	 * Disable the start search button and enable the stop search
	 * button.
	 */
    	gtk_widget_set_sensitive(gui.start_button, FALSE);
    	gtk_widget_set_sensitive(gui.stop_button, TRUE);
	clear_thumbnails();

#if 0
	/* 
	 * update defaults
	 */
	for(int i=0; i<nscapes; i++) {
		read_histo_tabs(&scapes[i].fsp_info, i);
	}
#endif


	/* another global, ack!! this should be on the heap XXX */
	static topo_region_t main_region; 
	build_search_from_gui(&main_region);

	pthread_mutex_lock(&display_mutex);
	image_controls.cur_op = CNTRL_NEXT;
	pthread_mutex_unlock(&display_mutex);


	/* problem: the signal (below) gets handled before the search
	   thread has a chance to drain the ring. so do it here. */
	pthread_mutex_lock(&ring_mutex);
	drain_ring(from_search_thread);
	pthread_mutex_unlock(&ring_mutex);


	/* 
	 * send the message
	 */
	message = (message_t *)malloc(sizeof(*message));
	if (message == NULL) {
		printf("failed to allocate message \n");
		exit(1);
	}
	message->type = START_SEARCH;
	message->data = (void *)&main_region;
	err = ring_enq(to_search_thread, message);
	if (err) {
		printf("XXX failed to enq message \n");
		exit(1);
	}

	//fprintf(stderr, "facemain: enq START_SEARCH\n");

        GUI_CALLBACK_LEAVE();	/* need to do this before signal (below) */

	if(!display_thread_running) {
		display_thread_running = 1;
		err = pthread_create(&display_thread_info, PATTR_DEFAULT, display_thread, NULL);
		if (err) {
			printf("failed to create  display thread \n");
			exit(1);
		}
	} else {
		pthread_mutex_lock(&display_mutex);
		pthread_cond_signal(&display_cond);
		pthread_mutex_unlock(&display_mutex);
	}

}


/* The file selection widget and the string to store the chosen filename */

static void
cb_save_spec_to_filename(GtkWidget *widget, gpointer user_data) 
{
  topo_region_t main_region;
  GtkWidget *file_selector = (GtkWidget *)user_data;
  const gchar *selected_filename;
  char buf[BUFSIZ];

  GUI_CALLBACK_ENTER();

  selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
  
  build_search_from_gui(&main_region);
  printf("saving spec to: %s\n", selected_filename);
  sprintf(buf, "%s", selected_filename);
  build_filter_spec(buf, &main_region);

  GUI_CALLBACK_LEAVE();
}


static void
write_search_config(const char *dirname, snap_search **searches, int nsearches)
{
	struct stat	buf;
	int			err;
	int			i;
	FILE *		conf_file;
	char		buffer[256];	/* XXX check */


	/* Do some test on the dir */
	/* XXX popup errors */
	err = stat(dirname, &buf);
	if (err != 0) {
		if (errno == ENOENT) {
			err = mkdir(dirname, 0777);
			if (err != 0) {
				perror("Failed to make save directory ");
				assert(0);
			}
		} else {
			perror("open data file: ");
			assert(0);
		}
	} else {
		/* make sure it is a dir */
		if (!S_ISDIR(buf.st_mode)) {
			fprintf(stderr, "%s is not a directory \n", dirname);	
			assert(0);
		}
	}

	sprintf(buffer, "%s/%s", dirname, SEARCH_CONFIG_FILE);
	conf_file = fopen(buffer, "w");

	for (i=0; i < nsearches; i++) {
		snap_searches[i]->write_config(conf_file, dirname);
	}
	fclose(conf_file);
}


static GtkWidget *
create_search_window()
{
    GtkWidget *box2, *box3, *box1;
    GtkWidget *separator;
    GtkWidget *table;
    GtkWidget *frame, *widget;
    GtkWidget *snap_cntrl;
    int row = 0;        /* current table row */
	int			i;

    GUI_THREAD_CHECK(); 

    box1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (box1);


	/* XXX */
    frame = gtk_frame_new("Searches");
    table = gtk_table_new(num_searches+1, 3, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 2);
    gtk_table_set_col_spacings(GTK_TABLE(table), 4);
    gtk_container_set_border_width(GTK_CONTAINER(table), 10);

    widget = gtk_label_new("Predicate");
    gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_widget_show(widget);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, row, row+1);

    widget = gtk_label_new("Description");
    gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_widget_show(widget);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, row, row+1);

    widget = gtk_label_new("Edit");
    gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_widget_show(widget);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 2, 3, row, row+1);

	for (i=0; i < num_searches; i++) {
		row = i + 1;
		widget = snap_searches[i]->get_search_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, row, row+1);
		widget = snap_searches[i]->get_config_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, row, row+1);
		widget = snap_searches[i]->get_edit_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 2, 3, row, row+1);
	}
	gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_box_pack_start(GTK_BOX(box1), frame, FALSE, FALSE, 10);
    gtk_widget_show(frame);
    gtk_widget_show(table);

    /* Add the start and stop buttons */

    box2 = gtk_hbox_new (FALSE, 10);
    gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
    gtk_box_pack_end (GTK_BOX (box1), box2, FALSE, FALSE, 0);
    gtk_widget_show (box2);

    gui.start_button = gtk_button_new_with_label ("Start");
    g_signal_connect_swapped (G_OBJECT (gui.start_button), "clicked",
		    	     G_CALLBACK(cb_start_search), NULL);
    gtk_box_pack_start (GTK_BOX (box2), gui.start_button, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS (gui.start_button, GTK_CAN_DEFAULT);
    gtk_widget_show (gui.start_button);

    gui.stop_button = gtk_button_new_with_label ("Stop");
    g_signal_connect_swapped (G_OBJECT (gui.stop_button), "clicked",
		    	     G_CALLBACK(cb_stop_search), NULL);
    gtk_box_pack_start (GTK_BOX (box2), gui.stop_button, TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS (gui.stop_button, GTK_CAN_DEFAULT);
    gtk_widget_set_sensitive(gui.stop_button, FALSE);
    gtk_widget_show (gui.stop_button);

    separator = gtk_hseparator_new ();
    gtk_box_pack_end (GTK_BOX (box1), separator, FALSE, FALSE, 0);
    gtk_widget_show (separator);

	return(box1);
}


static void
cb_load_search_from_dir(GtkWidget *widget, gpointer user_data) 
{
	topo_region_t main_region;
	GtkWidget *file_selector = (GtkWidget *)user_data;
	const gchar *dirname;
	char *	olddir;
	int			err;
	char buf[BUFSIZ];

	GUI_CALLBACK_ENTER();

	dirname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector));
	sprintf(buf, "%s/%s", dirname, SEARCH_CONFIG_FILE);

	olddir = getcwd(NULL, 0);

	err = chdir(dirname);
	assert(err == 0);

	/* XXXX cleanup all the old searches first */

	printf("Reading scapes: %s ...\n", buf);
	read_search_config(buf, snap_searches, &num_searches);
	printf("Done reading scapes...\n");
	
	err = chdir(olddir);
	assert(err == 0);
	free(olddir);

	gtk_widget_destroy(gui.search_widget);
    gui.search_widget = create_search_window();
    gtk_box_pack_start (GTK_BOX(gui.search_box), gui.search_widget, 
		FALSE, FALSE, 10);

	GUI_CALLBACK_LEAVE();
}

static void
cb_save_search_dir(GtkWidget *widget, gpointer user_data) 
{
	topo_region_t main_region;
	GtkWidget *file_selector = (GtkWidget *)user_data;
	const gchar *dirname;
	char buf[BUFSIZ];

	GUI_CALLBACK_ENTER();

	dirname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector));

	sprintf(buf, "%s/%s", dirname, "search_config");

	/* XXXX cleanup all the old searches first */

	printf("Reading scapes: %s ...\n", buf);
	write_search_config(dirname, snap_searches, num_searches);
	printf("Done reading scapes...\n");

	gtk_widget_destroy(gui.search_widget);
    gui.search_widget = create_search_window();
    gtk_box_pack_start (GTK_BOX(gui.search_box), gui.search_widget, 
		FALSE, FALSE, 10);

	GUI_CALLBACK_LEAVE();
}


static void
cb_load_search() 
{
	GtkWidget *file_selector;

  	GUI_CALLBACK_ENTER();
	printf("load search \n");

	/* Create the selector */
  	file_selector = gtk_file_selection_new("Dir name for search");
  	gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(file_selector));

  	g_signal_connect(GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
		    "clicked", G_CALLBACK(cb_load_search_from_dir),
		    (gpointer) file_selector);
   			   
  	/* 
     * Ensure that the dialog box is destroyed when the user clicks a button. 
	 * Use swapper here to get the right argument to destroy (YUCK).
     */
  	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
			    "clicked",
			    G_CALLBACK(gtk_widget_destroy), 
			    (gpointer) file_selector); 

	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
			    "clicked",
			    G_CALLBACK (gtk_widget_destroy),
			    (gpointer) file_selector); 

	/* Display that dialog */
	gtk_widget_show (file_selector);
	GUI_CALLBACK_LEAVE();
}


static void
cb_save_search() 
{
  GtkWidget *file_selector;

  GUI_CALLBACK_ENTER();
  printf("save search \n");
  GUI_CALLBACK_LEAVE();
  return;

  /* Create the selector */
  file_selector = gtk_file_selection_new("Filename for filter spec.");
  //gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selector),
				  //"sample.spec");
  gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(file_selector));

  g_signal_connect(GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
		    "clicked",
		    G_CALLBACK(cb_save_spec_to_filename),
		    (gpointer) file_selector);
   			   
  /* Ensure that the dialog box is destroyed when the user clicks a button. */
  g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
			    "clicked",
			    G_CALLBACK (gtk_widget_destroy), 
			    (gpointer) file_selector); 

  g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
			    "clicked",
			    G_CALLBACK (gtk_widget_destroy),
			    (gpointer) file_selector); 
   
   /* Display that dialog */
   gtk_widget_show (file_selector);

  GUI_CALLBACK_LEAVE();
}

static void
cb_save_search_as() 
{
	GtkWidget *file_selector;

  	GUI_CALLBACK_ENTER();
	printf("Save search \n");

	/* Create the selector */
  	file_selector = gtk_file_selection_new("Save Directory:");
  	gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(file_selector));

  	g_signal_connect(GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
		    "clicked", G_CALLBACK(cb_save_search_dir),
		    (gpointer) file_selector);
   			   
  	/* 
     * Ensure that the dialog box is destroyed when the user clicks a button. 
	 * Use swapper here to get the right argument to destroy (YUCK).
     */
  	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
			    "clicked",
			    G_CALLBACK(gtk_widget_destroy), 
			    (gpointer) file_selector); 

	g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
			    "clicked",
			    G_CALLBACK (gtk_widget_destroy),
			    (gpointer) file_selector); 

	/* Display that dialog */
	gtk_widget_show (file_selector);
	GUI_CALLBACK_LEAVE();
}



static void 
cb_show_stats(GtkButton* item, gpointer data)
{
    GUI_CALLBACK_ENTER();
    create_stats_win(shandle, expert_mode);
    GUI_CALLBACK_LEAVE();
}

/* For the check button */
static void
cb_toggle_stats( gpointer   callback_data,
		 guint      callback_action,
		 GtkWidget *menu_item )
{
    GUI_CALLBACK_ENTER();
/*     if(GTK_CHECK_MENU_ITEM (menu_item)->active) { */
/*       create_stats_win(shandle, expert_mode); */
/*     } else { */
/*       close_stats_win(); */
/*     } */
    toggle_stats_win(shandle, expert_mode);
    GUI_CALLBACK_LEAVE();
}


/* For the check button */
static void
cb_toggle_dump_attributes( gpointer   callback_data,
			   guint      callback_action,
			   GtkWidget *menu_item )
{
    GUI_CALLBACK_ENTER();
    dump_attributes = GTK_CHECK_MENU_ITEM (menu_item)->active;
    GUI_CALLBACK_LEAVE();
}


/* For the check button */
static void
cb_toggle_expert_mode( gpointer   callback_data,
		       guint      callback_action,
		       GtkWidget *menu_item )
{
    GUI_CALLBACK_ENTER();
    if( GTK_CHECK_MENU_ITEM (menu_item)->active ) {
      expert_mode = 1;
      show_expert_widgets();
    } else {
      expert_mode = 0;
      hide_expert_widgets();
    }
    GUI_CALLBACK_LEAVE();
}


/* ********************************************************************** */
/* widget setup functions */
/* ********************************************************************** */

static void
create_image_info(GtkWidget *container_box, image_info_t *img_info)
{

	char		data[BUFSIZ];

	GUI_THREAD_CHECK(); 

	/*
	 * Now create another region that has the controls
	 * that manipulate the current image being displayed.
	 */

	img_info->parent_box = container_box;
	img_info->info_box1 = gtk_hbox_new (FALSE, 0);
    	gtk_container_set_border_width(GTK_CONTAINER(img_info->info_box1), 10);

	GtkWidget *frame;
	frame = gtk_frame_new("Image Info");
	gtk_container_add(GTK_CONTAINER(frame), img_info->info_box1);
    	gtk_widget_show(frame);

    	gtk_box_pack_start(GTK_BOX(img_info->parent_box), 
			frame, TRUE, TRUE, 0);
    	gtk_widget_show(img_info->info_box1);

/* 	img_info->info_box2 = gtk_hbox_new (FALSE, 10); */
/*     	gtk_container_set_border_width(GTK_CONTAINER(img_info->info_box2), 10); */
/*     	gtk_box_pack_start(GTK_BOX(img_info->parent_box),  */
/* 			img_info->info_box2, FALSE, FALSE, 0); */
/*     	gtk_widget_show(img_info->info_box2); */


	/* 
	 * image name
	 */
	//sprintf(data, "%-10s:\n", "Name:");
	img_info->name_tag = gtk_label_new("Name:");
    	gtk_box_pack_start (GTK_BOX(img_info->info_box1), 
			img_info->name_tag, FALSE, FALSE, 0);
    	gtk_widget_show(img_info->name_tag);


	sprintf(data, "%-60s:", " ");
	img_info->name_label = gtk_label_new(data);
    	gtk_box_pack_start (GTK_BOX(img_info->info_box1), 
			img_info->name_label, TRUE, TRUE, 0);
    	gtk_widget_show(img_info->name_label);

	/*
	 * Place holder and blank spot for the number of bounding
	 * boxes found.
	 */
	sprintf(data, "%-3s", " ");
	img_info->count_label = gtk_label_new(data);
    	gtk_box_pack_end (GTK_BOX(img_info->info_box1), 
			img_info->count_label, FALSE, FALSE, 0);
    	gtk_widget_show(img_info->count_label);

	img_info->count_tag = gtk_label_new("Num Scenes:");
    	gtk_box_pack_end(GTK_BOX(img_info->info_box1), 
			img_info->count_tag, FALSE, FALSE, 0);
    	gtk_widget_show(img_info->count_tag);
}




static void
create_image_control(GtkWidget *container_box,
		     image_info_t *img_info,
		     image_control_t *img_cntrl)
{
	//GtkWidget *label;
    	GtkObject *adj;

	GUI_THREAD_CHECK(); 

	/*
	 * Now create another region that has the controls
	 * that manipulate the current image being displayed.
	 */

	img_cntrl->parent_box = container_box;
	img_cntrl->control_box = gtk_hbox_new (FALSE, 10);
    	gtk_container_set_border_width(GTK_CONTAINER(img_cntrl->control_box), 0);
    	gtk_box_pack_start(GTK_BOX(img_cntrl->parent_box), 
			img_cntrl->control_box, FALSE, FALSE, 0);
    	gtk_widget_show(img_cntrl->control_box);

	img_cntrl->next_button = gtk_button_new_with_label ("Next");
    	g_signal_connect_swapped(G_OBJECT(img_cntrl->next_button), 
			"clicked", G_CALLBACK(cb_next_image), NULL);
    	gtk_box_pack_end(GTK_BOX(img_cntrl->control_box), 
			img_cntrl->next_button, FALSE, FALSE, 0);
    	GTK_WIDGET_SET_FLAGS (img_cntrl->next_button, GTK_CAN_DEFAULT);
    	//gtk_widget_grab_default(img_cntrl->next_button);
    	gtk_widget_show(img_cntrl->next_button);

/* 	label = gtk_label_new ("more images"); */
/*     	gtk_box_pack_end(GTK_BOX(img_cntrl->control_box), label, FALSE, FALSE, 0); */
/*     	gtk_widget_show(label); */


	img_info->qsize_label = gtk_label_new ("");
    	gtk_box_pack_end(GTK_BOX(img_cntrl->control_box), 
			img_info->qsize_label, FALSE, FALSE, 0);
    	gtk_widget_show(img_info->qsize_label);
	gtk_timeout_add(500 /* ms */, timeout_write_qsize, img_info->qsize_label);

	img_info->tobjs_label = gtk_label_new ("");
    	gtk_box_pack_start(GTK_BOX(img_cntrl->control_box), 
			img_info->tobjs_label, FALSE, FALSE, 0);
    	gtk_widget_show(img_info->tobjs_label);
	gtk_timeout_add(500 /* ms */, timeout_write_tobjs, img_info->tobjs_label);

	img_info->sobjs_label = gtk_label_new ("");
    	gtk_box_pack_start(GTK_BOX(img_cntrl->control_box), 
			img_info->sobjs_label, FALSE, FALSE, 0);
    	gtk_widget_show(img_info->sobjs_label);
	gtk_timeout_add(500 /* ms */, timeout_write_sobjs, img_info->sobjs_label);

	img_info->dobjs_label = gtk_label_new ("");
    	gtk_box_pack_start(GTK_BOX(img_cntrl->control_box), 
			img_info->dobjs_label, FALSE, FALSE, 0);
    	gtk_widget_show(img_info->dobjs_label);
	gtk_timeout_add(500 /* ms */, timeout_write_dobjs, img_info->dobjs_label);

	
	
/* 	label = gtk_label_new ("Zoom"); */
/*     	gtk_box_pack_start (GTK_BOX(img_cntrl->control_box),  */
/* 			label, FALSE, FALSE, 0); */
    	//gtk_widget_show(label);


    	adj = gtk_adjustment_new(2.0, 1.0, 10.0, 1.0, 1.0, 1.0);
	img_cntrl->zlevel = 2;
    	img_cntrl->zbutton = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
    	gtk_box_pack_start (GTK_BOX(img_cntrl->control_box),img_cntrl->zbutton, 
			FALSE, FALSE, 0);
    	//gtk_widget_show(img_cntrl->zbutton);


	img_cntrl->cur_op = CNTRL_ELEV;

}

/*
 * Create the region that will display the results of the 
 * search.
 */
static void
create_display_region(GtkWidget *main_box)
{

	GtkWidget *	box2;
 	GtkWidget *	separator;
	GtkWidget *	x;

	GUI_THREAD_CHECK(); 

	/*
	 * Create a box that holds the following sub-regions.
	 */
    	x = gtk_hbox_new(FALSE, 0);
    	gtk_container_set_border_width(GTK_CONTAINER(x), 10);
    	gtk_box_pack_start(GTK_BOX(main_box), x, TRUE, TRUE, 0);
    	gtk_widget_show(x);

/*     	separator = gtk_vseparator_new(); */
/*     	gtk_box_pack_start(GTK_BOX(x), separator, FALSE, FALSE, 0); */
/*     	gtk_widget_show (separator); */

/*     	separator = gtk_vseparator_new(); */
/*     	gtk_box_pack_end(GTK_BOX(x), separator, FALSE, FALSE, 0); */
/*     	gtk_widget_show (separator); */

	GtkWidget *result_box;
    	result_box = gtk_vbox_new(FALSE, 10);
    	gtk_container_set_border_width(GTK_CONTAINER(result_box), 0);
    	gtk_box_pack_start(GTK_BOX(x), result_box, TRUE, TRUE, 0);
    	gtk_widget_show(result_box);

	/* create the image information area */
	create_image_info(result_box, &image_information);

    	separator = gtk_hseparator_new ();
    	gtk_box_pack_start (GTK_BOX(result_box), separator, FALSE, FALSE, 0);
    	gtk_widget_show (separator);

	/*
	 * Create the region that will hold the current results
	 * being displayed.
	 */
    	box2 = gtk_vbox_new(FALSE, 10);
    	gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
    	gtk_box_pack_start (GTK_BOX (result_box), box2, TRUE, TRUE, 0);
    	gtk_widget_show(box2);


/* 	image_window = gtk_scrolled_window_new(NULL, NULL); */
/*     	gtk_box_pack_start(GTK_BOX (box2), image_window, TRUE, TRUE, 0); */
/*     	gtk_widget_show(image_window); */

/* 	image_view = gtk_viewport_new(NULL, NULL); */
/* 	gtk_container_add(GTK_CONTAINER(image_window), image_view); */
/*     	gtk_widget_show(image_view); */

	GtkWidget *thumbnail_view;
	thumbnail_view = gtk_table_new(TABLE_ROWS, TABLE_COLS, TRUE);
	gtk_box_pack_start(GTK_BOX (box2), thumbnail_view, TRUE, TRUE, 0);
	for(int i=0; i< TABLE_ROWS; i++) {
		for(int j=0; j< TABLE_COLS; j++) {
			GtkWidget *widget, *eb;
			GtkWidget *frame;

			eb = gtk_event_box_new();
			
			frame = gtk_frame_new(NULL);
			gtk_frame_set_label(GTK_FRAME(frame), "");
			gtk_container_add(GTK_CONTAINER(eb), frame);
			gtk_widget_show(frame);

			widget = gtk_viewport_new(NULL, NULL);
			gtk_widget_set_size_request(widget, THUMBSIZE_X, THUMBSIZE_Y);
			gtk_container_add(GTK_CONTAINER(frame), widget);
			gtk_table_attach_defaults(GTK_TABLE(thumbnail_view), eb,
						  j, j+1, i, i+1);
			gtk_widget_show(widget);
			gtk_widget_show(eb);

			thumbnail_t *thumb = (thumbnail_t *)malloc(sizeof(thumbnail_t));
			thumb->marked = 0;
			thumb->img = NULL;
			thumb->viewport = widget;
			thumb->frame = frame;
			thumb->name[0] = '\0';
			thumb->nboxes = 0;
			thumb->nfaces = 0;
			thumb->hooks = NULL;
			TAILQ_INSERT_TAIL(&thumbnails, thumb, link);

			//fprintf(stderr, "thumb=%p\n", thumb);

			/* the data pointer mechanism seems to be
			 * broken, so use the object user pointer
			 * instead */
			gtk_object_set_user_data(GTK_OBJECT(eb), thumb);
			gtk_signal_connect(GTK_OBJECT(eb),
					   "enter-notify-event",
					   GTK_SIGNAL_FUNC(cb_img_info),
					   (gpointer)thumb);
			gtk_signal_connect(GTK_OBJECT(eb),
					   "button-press-event",
					   GTK_SIGNAL_FUNC(cb_img_popup),
					   (gpointer)thumb);
		}
	}
    gtk_widget_show(thumbnail_view);


	/* create the image control area */
	create_image_control(result_box, &image_information, &image_controls);


	clear_image_info(&image_information);
	disable_image_control(&image_controls);
}

/* set check button when activated */
/* static void */
/* cb_enable_check_button(GtkWidget *widget) { */
/*   GUI_CALLBACK_ENTER(); */

/*   GtkWidget *button  */
/*     = (GtkWidget *)gtk_object_get_user_data(GTK_OBJECT(widget)); */
/*   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE); */

/*   GUI_CALLBACK_LEAVE(); */
/* } */




static void
cb_quit() {
  printf("MARKED: %d of %d seen\n", user_measurement.total_marked, 
	 user_measurement.total_seen);
  gtk_main_quit();
}




/* Our menu, an array of GtkItemFactoryEntry structures that defines each menu item */
static GtkItemFactoryEntry menu_items[] = { /* XXX */

  { "/_File",         NULL,         NULL,           0, "<Branch>" },
  //  { "/File/_New",     "<control>N", (GtkItemFactoryCallback)print_hello,    0, "<StockItem>", GTK_STOCK_NEW },
  //  { "/File/_Open",    "<control>O", (GtkItemFactoryCallback)print_hello,    0, "<StockItem>", GTK_STOCK_OPEN },
  //  { "/File/_Save",    "<control>S", (GtkItemFactoryCallback)print_hello,    0, "<StockItem>", GTK_STOCK_SAVE },
  //  { "/File/Save _As", NULL,         NULL,           0, "<Item>" },
  { "/File/Load Search",   NULL,  G_CALLBACK(cb_load_search),   0, "<Item>" },
  { "/File/Save Search",   NULL,  G_CALLBACK(cb_save_search),   0, "<Item>" },
  { "/File/Save Search As.",   NULL,  G_CALLBACK(cb_save_search_as),  0, "<Item>" },
  { "/File/sep1",     NULL,         NULL,           0, "<Separator>" },
  { "/File/_Quit",    "<CTRL>Q", (GtkItemFactoryCallback)cb_quit, 0, "<StockItem>", GTK_STOCK_QUIT },
  { "/_View",                NULL,  NULL,                                  0, "<Branch>" },
  { "/_View/Stats Window",   "<CTRL>I",  G_CALLBACK(cb_toggle_stats),   0, "<Item>" },

  { "/Options",                 NULL, NULL, 0, "<Branch>" },
  { "/Options/sep1",            NULL, NULL, 0, "<Separator>" },
  //{ "/Options/Expert Mode",     NULL, G_CALLBACK(cb_toggle_expert_mode), 0, "<CheckItem>" },
  { "/Options/Expert Mode",     "<CTRL>E", G_CALLBACK(cb_toggle_expert_mode), 0, "<CheckItem>" },
  { "/Options/Dump Attributes", NULL, G_CALLBACK(cb_toggle_dump_attributes), 0, "<CheckItem>" },

  { "/Albums",                  NULL, NULL, 0, "<Branch>" },
  { "/Albums/tear",             NULL, NULL, 0, "<Tearoff>" },

  //  { "/Options/tear",  NULL,         NULL,           0, "<Tearoff>" },
  //  { "/Options/Check", NULL,         (GtkItemFactoryCallback)print_toggle,   1, "<CheckItem>" },
  //  { "/Options/sep",   NULL,         NULL,           0, "<Separator>" },
  //  { "/Options/Rad1",  NULL,         (GtkItemFactoryCallback)print_selected, 1, "<RadioItem>" },
  //  { "/Options/Rad2",  NULL,         (GtkItemFactoryCallback)print_selected, 2, "/Options/Rad1" },
  //  { "/Options/Rad3",  NULL,         (GtkItemFactoryCallback)print_selected, 3, "/Options/Rad1" },
  //  { "/_Help",         NULL,         NULL,           0, "<LastBranch>" },
  //  { "/_Help/About",   NULL,         NULL,           0, "<Item>" },
  { NULL, NULL, NULL }
};

static void 
cb_collection(gpointer callback_data, guint callback_action, 
	GtkWidget  *menu_item ) 
{

  /* printf("cb_collection: #%d\n", callback_action); */

  if(GTK_CHECK_MENU_ITEM(menu_item)->active) {
    collections[callback_action].active = 1;
  } else {
    collections[callback_action].active = 0;
  }
}


/* Returns a menubar widget made from the above menu */
static GtkWidget *
get_menubar_menu( GtkWidget  *window )
{
  GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;
  gint nmenu_items;

  /* Make an accelerator group (shortcut keys) */
  accel_group = gtk_accel_group_new ();

  /* Make an ItemFactory (that makes a menubar) */
  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
                                       accel_group);

  GtkItemFactoryEntry *tmp_menu;
  
  for(tmp_menu = menu_items, nmenu_items = 0;
      (tmp_menu->path);
      tmp_menu++) {
    nmenu_items++;
  }

  /* This function generates the menu items. Pass the item factory,
     the number of items in the array, the array itself, and any
     callback data for the the menu items. */
  gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);

  /* create more menu items */
  struct collection_t *tmp_coll;
  for(tmp_coll = collections; tmp_coll->name; tmp_coll++) {
    GtkItemFactoryEntry entry;
    char buf[BUFSIZ];

    sprintf(buf, "/Albums/%s", tmp_coll->name);
    for(char *s=buf; *s; s++) {
      if(*s == '_') {
	*s = ' ';
      }
    }
    entry.path = strdup(buf);
    entry.accelerator = NULL;
    entry.callback = G_CALLBACK(cb_collection);
    entry.callback_action = tmp_coll - collections;
    entry.item_type = "<CheckItem>";
    gtk_item_factory_create_item(item_factory,
				 &entry,
				 NULL,
				 1); /* XXX guess, no doc */

    GtkWidget *widget = gtk_item_factory_get_widget(item_factory, buf);
    //gtk_widget_set_sensitive(widget, FALSE);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), tmp_coll->active);

    //tmp_menu++;
    //nmenu_items++;
  }

  /* Attach the new accelerator group to the window. */
  gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

  /* Finally, return the actual *menu* bar created by the item factory. */
  return gtk_item_factory_get_widget (item_factory, "<main>");
}



/* 
 * makes the main window 
 */

static void 
create_main_window(void)
{
	GtkWidget * separator;
    GtkWidget *main_vbox;
    GtkWidget *menubar;
    GtkWidget *main_box;

    GUI_THREAD_CHECK(); 

    /*
     * Create the the main window.
     */
    gui.main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT (gui.main_window), "destroy",
		      G_CALLBACK (cb_quit), NULL);

    gtk_window_set_title(GTK_WINDOW (gui.main_window), "Diamond SnapFind");

    main_vbox = gtk_vbox_new (FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 1);
    gtk_container_add (GTK_CONTAINER (gui.main_window), main_vbox);
    gtk_widget_show(main_vbox);

    menubar = get_menubar_menu (gui.main_window);
    gtk_box_pack_start (GTK_BOX (main_vbox), menubar, FALSE, TRUE, 0);
    gtk_widget_show(menubar);

    main_box = gtk_hbox_new (FALSE, 0);
    //gtk_container_add (GTK_CONTAINER (main_vbox), main_box);
    gtk_box_pack_end (GTK_BOX (main_vbox), main_box, FALSE, TRUE, 0);
    gtk_widget_show (main_box);

    gui.search_box = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX(main_box), gui.search_box, FALSE, FALSE, 10);
    gtk_widget_show (gui.search_box);

    gui.search_widget = create_search_window();
    gtk_box_pack_start (GTK_BOX(gui.search_box), gui.search_widget, 
		FALSE, FALSE, 10);

    separator = gtk_vseparator_new();
    gtk_box_pack_start(GTK_BOX(main_box), separator, FALSE, FALSE, 0);
    gtk_widget_show (separator);

    create_display_region(main_box);

    gtk_widget_show(gui.main_window);
}





static void
usage(char **argv) 
{
  fprintf(stderr, "usage: %s [options]\n", basename(argv[0]));
  fprintf(stderr, "  -h        - help\n");
  fprintf(stderr, "  -e        - expert mode\n");
  fprintf(stderr, "  -s<file>  - histo index file\n");
  fprintf(stderr, "  --similarity <filter>=<val>  - set default threshold (repeatable)\n");
  fprintf(stderr, "  --min-faces=<num>       - set min number of faces \n");
  fprintf(stderr, "  --face-levels=<num>     - set face detector levels \n");
  fprintf(stderr, "  --dump-spec=<file>      - dump spec file and exit \n");
  fprintf(stderr, "  --dump-objects          - start search and dump objects\n");
  fprintf(stderr, "  --read-spec=<file>      - use spec file. requires dump-objects\n");
}



static int
set_export_threshold(char *arg) {
  char *s = arg;

  while(*s && *s != '=') {
    s++;
  }
  if(!*s) return -1;		/* error */
  *s++ = '\0';

  export_threshold_t *et = (export_threshold_t *)malloc(sizeof(export_threshold_t));
  assert(et);
  et->name = arg;
  double d = atof(s);
  if(d > 1) d /= 100.0;
  if(d > 1 || d < 0) {
    fprintf(stderr, "bad distance: %s\n", s);
    return -1;
  }
  et->distance = 1.0 - d;	/* similarity */
  et->index = -1;

  TAILQ_INSERT_TAIL(&export_list, et, link);

  return 0;
}

int 
main(int argc, char *argv[])
{

	pthread_t 	search_thread;
	int		err;
	char *scapeconf = "histo/search_config";
	int c;
	static const char *optstring = "hes:f:";
	struct option long_options[] = {
	  {"dump-spec", required_argument, NULL, 0},
	  {"dump-objects", no_argument, NULL, 0},
	  {"read-spec", required_argument, NULL, 0},
	  {"similarity", required_argument, NULL, 'f'},
	  {"min-faces", required_argument, NULL, 0},
	  {"face-levels", required_argument, NULL, 0},
	  {0, 0, 0, 0}
	};
	int option_index = 0;

	/*
	 * Start the GUI
	 */

	GUI_THREAD_INIT();
    	gtk_init(&argc, &argv);
	gtk_rc_parse("gtkrc");
	gdk_rgb_init();
	printf("Starting main\n");

	while((c = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
		switch(c) {
		case 0:
		  if(strcmp(long_options[option_index].name, "dump-spec") == 0) {
		    dump_spec_file = optarg;
		  } else if(strcmp(long_options[option_index].name, "dump-objects") == 0) {
		    dump_objects = 1;
		  } else if(strcmp(long_options[option_index].name, "read-spec") == 0) {
		    read_spec_filename = optarg;
		  } else if(strcmp(long_options[option_index].name, "min-faces") == 0) {
		    default_min_faces = atoi(optarg);
		  } else if(strcmp(long_options[option_index].name, "face-levels") == 0) {
		    default_face_levels = atoi(optarg);
		  }
		  break;
		case 'e':
		  fprintf(stderr, "expert mode must now be turned on with the menu option\n");
		  //expert_mode = 1;
		  break;
		case 's':
		  scapeconf = optarg;
		  break;
		case 'f':
		  if(set_export_threshold(optarg) < 0) {
		    usage(argv);
		    exit(1);
		  }
		  break;
		case ':':
		  fprintf(stderr, "missing parameter\n");
		  exit(1);
		case 'h':
		case '?':
		default:
		  usage(argv);
		  exit(1);
		  break;
		}		
	}

	printf("Initializing communciation rings...\n");

	/*
	 * Initialize communications rings with the thread
	 * that interacts with the search thread.
	 */
	err = ring_init(&to_search_thread, TO_SEARCH_RING_SIZE);
	if (err) {
		exit(1);
	}

	err = ring_init(&from_search_thread, FROM_SEARCH_RING_SIZE);
	if (err) {
		exit(1);
	}

#ifdef	XXX
	printf("Reading scapes: %s ...\n", scapeconf);
	read_search_config(scapeconf, snap_searches, &num_searches);
	printf("Done reading scapes...\n");
#endif

#ifdef	XXX_NOW
	/* read the histograms while we are at it */
	for(int i=0; i<nscapes; i++) {
		char buf[MAX_PATH];

		sprintf(buf, "%s", scapes[i].file);
		printf("processing %s\n", buf);

		switch(scapes[i].fsp_info.type) {
		case FILTER_TYPE_COLOR:
		  err = patch_spec_make_histograms(dirname(buf), &scapes[i].fsp_info);
		  break;
		case FILTER_TYPE_TEXTURE:
		  err = texture_make_features(dirname(buf), &scapes[i].fsp_info);
		  break;
		case FILTER_TYPE_ARENA:
		  err = arena_make_vectors(dirname(buf), &scapes[i].fsp_info);
		  break;
		}
		if(err) {
		  scapes[i].disabled = 1;
		}
	}
#endif

	/* 
	 * read the list of collections
	 */
	{
	  void *cookie;
	  char *name;
	  int err;
	  int pos = 0;
	  err = nlkup_first_entry(&name, &cookie);
	  while(!err && pos < MAX_ALBUMS) {
	    collections[pos].name = name;
	    collections[pos].active = pos ? 0 : 1; /* first one active */
	    pos++;
	    err = nlkup_next_entry(&name, &cookie);
	  }
	  collections[pos].name = NULL;
	}

#ifdef	XXX_NOW
	/* 
	 * some special argument processing before we run the gui 
	 */

	printf("scapes present:");
	for(int i=0; i<nscapes; i++) {
		printf(" %s", scapes[i].name);
	}
	printf("\n");

	export_threshold_t *et;
	printf("scapes requested:");
	TAILQ_FOREACH(et, &export_list, link) {
	  printf(" %s (distance=%.2f)", et->name, et->distance);
	  for(int i=0; i<nscapes; i++) {
	    if(strcmp(et->name, scapes[i].name) == 0) {
	      et->index = i;
	    }
	  }
	  if(et->index < 0) {
	    fprintf(stderr, "unrecognized filter: %s\n", et->name);
	    exit(1);
	  }
	}
	printf("\n");

	topo_region_t main_region;

	if(dump_spec_file || dump_objects) {
	  /* build a main_region */

	  main_region.min_faces = default_min_faces;
	  main_region.face_levels = default_face_levels;
	  main_region.nscapes = 0;

	  TAILQ_FOREACH(et, &export_list, link) {
	    main_scape_t *scape;
	    
	    scape = &main_region.scapes[main_region.nscapes++];
	    strncpy(scape->name, scapes[et->index].name, MAX_NAME);
/* 	    strncpy(scape->path, scapes[et->index].file, MAX_PATH); */
	    /* XXX this is probably unsafe (pointers in struct) */
	    scape->fsp_info = scapes[et->index].fsp_info;
	    scape->fsp_info.pdistance = et->distance;
	    
	    /* set the values in the fsp_histo_t we are sending to the search thread.
	     * not reqd if we updated defaults above */
	    //read_histo_tabs(&scape->fsp_info, et->index);
	  }
	}

	if(dump_spec_file) {
	  build_filter_spec(dump_spec_file, &main_region);
	  exit(0);
	}

	if(dump_objects) {
	  init_search();
	  do_search(&main_region, read_spec_filename); /* setup shandle */
	  int err = 0;
	  while(!err) {
	    ls_obj_handle_t cur_obj;
	    err = ls_next_object(shandle, &cur_obj, 0);	/* blocking */
	    if(!err) {
	      printf("\nOBJECT\n");
	      //lf_dump_attr(fhandle, cur_obj);
	      ls_release_object(shandle, cur_obj);
	    } else {
	      fprintf(stderr, "err = %d\n", err);
	    }
	  }
	  exit(0);
	}
#endif


	/*
	 * Create the main window.
	 */
	tooltips = gtk_tooltips_new();
	gtk_tooltips_enable(tooltips);


	GUI_THREAD_ENTER();
    	create_main_window();
	GUI_THREAD_LEAVE();

	/* 
	 * initialize and start the background thread 
	 */
	init_search();
	err = pthread_create(&search_thread, PATTR_DEFAULT, sfind_search_main, NULL);
	if (err) {
		perror("failed to create search thread");
		exit(1);
	}
	//gtk_timeout_add(100, func, NULL);


	/*
	 * Start the main loop processing for the GUI.
	 */

	MAIN_THREADS_ENTER(); 
    	gtk_main();
 	MAIN_THREADS_LEAVE();  

	return(0);
}
