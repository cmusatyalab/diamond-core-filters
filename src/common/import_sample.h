#ifndef _IMPORT_SAMPLE_H_
#define _IMPORT_SAMPLE_H_	1



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

enum import_layers_t {
	IMP_IMG_LAYER = 0,
	IMP_RES_LAYER,
	IMP_HIGHLIGHT_LAYER,
	IMP_SELECT_LAYER,
	IMP_MAX_LAYERS
};

typedef	struct import_win {
  GtkWidget 	*window;
  GtkWidget 	*drawing_area;
  GdkPixbuf       *pixbufs[IMP_MAX_LAYERS];
  RGBImage        *layers[IMP_MAX_LAYERS];
  GtkWidget       *statusbar;
  GtkWidget 	*scroll;
  GtkWidget 	*image_area;
  GtkWidget 	*drawbox;
  GtkWidget 	*drawhl;
  GtkWidget 	*example_list;
  GtkWidget 	*opt_menu;
  GtkWidget 	*hl_frame;
  GtkWidget 	*hl_table;
  GtkWidget 	*search_type;
  GtkWidget *	search_name;
  int             nfaces;
  GtkWidget *select_button;
  int x1, y1, x2, y2;
  int button_down;
  GtkWidget *color, *texture;
  bbox_t selections[MAX_SELECT];
  int    nselections;
  RGBImage *	img;
} import_win_t;

void open_import_window();
void import_update_searches();

#endif	/* ! _IMPORT_SAMPLE_H_ */
