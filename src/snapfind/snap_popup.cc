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
#include "search_support.h"
#include "snap_popup.h"
#include "snapfind.h"

/* XXX fix this */
extern snap_search *snap_searches[];
extern int num_searches;
void update_search_entry(snap_search *cur_search, int row);


/* XXX fix this */
static lf_fhandle_t 	fhandle = 0;



/* 
 * global state used for highlighting (running filters locally)
 */
static struct {
	pthread_mutex_t mutex;
	int 		thread_running;
	pthread_t 	thread;
	GtkWidget       *progress_bar;
} highlight_info = { PTHREAD_MUTEX_INITIALIZER, 0 };

/* forward function declarations */

static void kill_highlight_thread(int run);
static void draw_face_func(GtkWidget *widget, void *ptr);
static void draw_hbbox_func(GtkWidget *widget, void *ptr);


static inline int
min(int a, int b) { 
	return ( (a < b) ? a : b );
}
static inline int
max(int a, int b) { 
	return ( (a > b) ? a : b );
}


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
 * draw a bounding box into image at scale. bbox is read from object(!)
 */
region_t
draw_bounding_box(RGBImage *img, int scale, 
		  lf_fhandle_t fhandle, ls_obj_handle_t ohandle,
		  RGBPixel color, RGBPixel mask, char *fmt, int i) 
{
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
	gtk_container_foreach(GTK_CONTAINER(popup_window.histo_cb_area), draw_hbbox_func, 
			      popup_window.histo_cb_area);

        GUI_CALLBACK_LEAVE();
}






static GtkWidget *
describe_hbbox(lf_fhandle_t fhandle, ls_obj_handle_t ohandle, int i,
		      GtkWidget **button) 
{
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




static gboolean
realize_event(GtkWidget *widget, GdkEventAny *event, gpointer data) {
	
	assert(widget == popup_window.drawing_area);

	for(int i=0; i<MAX_LAYERS; i++) {
		popup_window.pixbufs[i] = pb_from_img(popup_window.layers[i]);
	}
	
	return TRUE;
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

void *
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

		  /* set the values in the fsp_histo_t we are sending to the
	           *search thread.
		   * update statusbar */
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
cb_popup_window_close(GtkWidget *window) 
{
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

static gboolean
cb_add_to_existing(GtkWidget *widget, GdkEventAny *event, gpointer data) 
{
	char buf[BUFSIZ] = "created new scene";
	GtkWidget *	active_item;
	snap_search *ssearch;
	int	   	idx;
  	GUI_CALLBACK_ENTER();

	active_item = gtk_menu_get_active(GTK_MENU(popup_window.example_list));

	idx = (int)g_object_get_data(G_OBJECT(active_item), "user data");

	assert(idx >= 0);
	assert(idx < num_searches);

	ssearch = snap_searches[idx];

	for(int i=0; i<popup_window.nselections; i++) {
		ssearch->add_patch(popup_window.hooks->img, 
				 popup_window.selections[i]);
  	}

	/* popup the edit window */ 
	ssearch->edit_search();

  guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(popup_window.statusbar),
					  "selection");
  gtk_statusbar_push(GTK_STATUSBAR(popup_window.statusbar), id, buf);
  
  GUI_CALLBACK_LEAVE();
  return TRUE;
}


/*
 * The callback function that takes user selected regions and creates
 * a new search with the list.
 */

static gboolean
cb_add_to_new(GtkWidget *widget, GdkEventAny *event, gpointer data) 
{
	char buf[BUFSIZ] = "created new scene";
	GtkWidget *	active_item;
	snap_search *ssearch;
	search_types_t	stype;
	int		idx;
	const char *	sname;
	GtkWidget *		item;
  	GUI_CALLBACK_ENTER();

	active_item = gtk_menu_get_active(GTK_MENU(popup_window.search_type));

	/* XXX can't directly get the cast to work ??? */ 
	idx = (int) g_object_get_data(G_OBJECT(active_item), "user data");
	stype = (search_types_t )idx;
   
	sname =  gtk_entry_get_text(GTK_ENTRY(popup_window.search_name));
    if(strlen(sname) < 1) {
     	sprintf(buf, "ERROR bad name!");
    	exit(1);
		/* XXXX popup window, and exit */
		/* XXX make sure the name already exists */
    }

	/* create the new search and put it in the global list */
	ssearch = create_search(stype, sname);
	assert(ssearch != NULL);
	snap_searches[num_searches] = ssearch;
	num_searches++;

	/*
	 * Put this in the list of searches in the selection pane.
	 */
	update_search_entry(ssearch, num_searches);


	/* Put the list of searches in the ones we can select in the popup menu */
	item = gtk_menu_item_new_with_label(ssearch->get_name());
	gtk_widget_show(item);
	g_object_set_data(G_OBJECT(item), "user data", (void *)(num_searches - 1));
	gtk_menu_shell_append(GTK_MENU_SHELL(popup_window.example_list), item);


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
	snap_search *	cur_search;
	int		i;
                                                                                
        menu = gtk_menu_new();
                         
	for (i=0;i<num_searches; i++) {
		cur_search = snap_searches[i];
		if (cur_search->is_example() == 0) {
			continue;
		}
        	item = gtk_menu_item_new_with_label(cur_search->get_name());
        	gtk_widget_show(item);
        	g_object_set_data(G_OBJECT(item), "user data", (void *)i);
        	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	}
                                       
        return(menu);
}
                                                                                
/*
 * Return a gtk_menu with a list of all the example
 * based searches.  This should be done using other state
 * instead of statically defined. XXX.
 */
GtkWidget *
get_example_searches_menu(void)
{
        GtkWidget *     menu;
        GtkWidget *     item;
                                                                                
        menu = gtk_menu_new();
                                                                                
        item = gtk_menu_item_new_with_label("Texture Search");
        gtk_widget_show(item);
        g_object_set_data(G_OBJECT(item), "user data", (void *)TEXTURE_SEARCH);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
                                                                                
        item = gtk_menu_item_new_with_label("RGB Histogram");
        gtk_widget_show(item);
        g_object_set_data(G_OBJECT(item), "user data",
                (void *)RGB_HISTO_SEARCH);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
                                                                                
        return(menu);
}
                                                                                


static GtkWidget *
new_search_panel(void)
{
	GtkWidget *box;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *widget;

  	frame = gtk_frame_new("Create New Search");
  	gtk_widget_show(frame);
		  
	box = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(box), 10);
	gtk_container_add(GTK_CONTAINER(frame), box);
	gtk_widget_show(box);

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
	gtk_widget_show(button);


	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, FALSE, 0);
	gtk_widget_show(hbox);

	widget = gtk_label_new("Type");
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);
	gtk_widget_show(widget);

	popup_window.search_type =  get_example_searches_menu();
	widget = gtk_option_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(widget), 
		popup_window.search_type);
	gtk_widget_show(widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);


	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, FALSE, 0);
	gtk_widget_show(hbox);

	widget = gtk_label_new("Name");
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);
	gtk_widget_show(widget);

	popup_window.search_name = gtk_entry_new();
	gtk_widget_show(popup_window.search_name);
	gtk_entry_set_activates_default(GTK_ENTRY(popup_window.search_name),
		TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), popup_window.search_name, 
		TRUE, FALSE, 0);

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
  	gtk_widget_show(frame);
		  
	box = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(box), 10);
	gtk_container_add(GTK_CONTAINER(frame), box);
	gtk_widget_show(box);

	/*
	 * Create a hbox that has the controls for
	 * adding a new search with examples to the existing
	 * searches.
	 */ 

	GtkWidget *button = gtk_button_new_with_label ("Add");
	popup_window.select_button = button;
	g_signal_connect_after(GTK_OBJECT(button), "clicked",
	   GTK_SIGNAL_FUNC(cb_add_to_existing), NULL);
	gtk_box_pack_start (GTK_BOX(box), button, TRUE, FALSE, 0);
	gtk_widget_show(button);

	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, FALSE, 0);
	gtk_widget_show(hbox);

	widget = gtk_label_new("Search");
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);
	gtk_widget_show(widget);

	popup_window.example_list =  get_example_menu();
	widget = gtk_option_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(widget), 
		popup_window.example_list);
	gtk_widget_show(widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);

	return(frame);
}

void
do_img_popup(GtkWidget *widget) 
{
        thumbnail_t *thumb;
	GtkWidget *eb;
	GtkWidget *frame;
	GtkWidget *image;
	GtkWidget *button;


	thumb = (thumbnail_t *)gtk_object_get_user_data(GTK_OBJECT(widget));

	/* the data gpointer passed in seems to not be the data
	 * pointer we gave gtk. instead, we save a pointer in the
	 * widget. (maybe the prototype didn't match) -RW */

	if(!thumb->img) goto done;

	if(!popup_window.window) {
		popup_window.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(popup_window.window), "Image");
		gtk_window_set_default_size(GTK_WINDOW(popup_window.window), 850, 350);
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
		  frame = gtk_frame_new("Search Update");
		  gtk_box_pack_end(GTK_BOX (box1), frame, FALSE, FALSE, 0);
		  gtk_widget_show(frame);
		  
		  GtkWidget *box2 = gtk_vbox_new (FALSE, 10);
		  gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
		  gtk_container_add(GTK_CONTAINER(frame), box2);
		  gtk_widget_show (box2);
		  
		  /* hbox */
		  GtkWidget *hbox = gtk_hbox_new(FALSE, 10);
		  gtk_box_pack_start (GTK_BOX(box2), hbox, TRUE, FALSE, 0);
		  gtk_widget_show(hbox);

		  /* couple of buttons */
		
		  GtkWidget *buttonbox = gtk_vbox_new(FALSE, 10);
		  gtk_box_pack_start(GTK_BOX(box2), buttonbox, TRUE, FALSE,0);
		  gtk_widget_show(buttonbox);
		
		  /*
 		   * Create a hbox that has the controls for
 		   * adding a new search with examples to the existing
		   * searches.
		   */ 

		  hbox = gtk_hbox_new(FALSE, 10);
		  gtk_box_pack_start (GTK_BOX(buttonbox), hbox, TRUE, FALSE, 0);
		  gtk_widget_show(hbox);

		  widget = new_search_panel();
		  gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);

		  widget = existing_search_panel();
		  gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, FALSE, 0);


		  /* control button to clear selected regions */
		  hbox = gtk_hbox_new(FALSE, 10);
		  gtk_box_pack_start (GTK_BOX(buttonbox), hbox, TRUE, FALSE, 0);
		  gtk_widget_show(hbox);

		  button = gtk_button_new_with_label ("Clear");
		  g_signal_connect_after(GTK_OBJECT(button), "clicked",
				 GTK_SIGNAL_FUNC(cb_clear_select), NULL);
		  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 0);
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


