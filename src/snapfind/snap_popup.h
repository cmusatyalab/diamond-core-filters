#ifndef _SNAP_POPUP_H_
#define _SNAP_POPUP_H_	1

#define COORDS_TO_BBOX(bbox,container)          \
{                                               \
  bbox.min_x = min(container.x1, container.x2); \
  bbox.max_x = max(container.x1, container.x2); \
  bbox.min_y = min(container.y1, container.y2); \
  bbox.max_y = max(container.y1, container.y2); \
}
                                                                                
                                                                                



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

typedef	struct pop_win {
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
  GtkWidget 	*drawbox;
  GtkWidget 	*drawhl;
  GtkWidget 	*example_list;
  GtkWidget 	*search_type;
  GtkWidget *	search_name;
  int             nfaces;
  GtkWidget *select_button;
  int x1, y1, x2, y2;
  int button_down;
  GtkWidget *color, *texture;
  bbox_t selections[MAX_SELECT];
  int    nselections;
} pop_win_t;

extern pop_win_t popup_window;

void do_img_popup(GtkWidget *widget, search_set *set);

void search_popup_add(img_search *ssearch, int nsearch);

#endif	/* ! _SNAP_POPUP_H_ */
