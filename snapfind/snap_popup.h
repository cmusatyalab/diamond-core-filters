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
    GtkWidget 	*histo_cb_area;
    GtkWidget 	*drawbox;
    GtkWidget 	*drawhl;
    GtkWidget 	*example_list;
    GtkWidget 	*existing_menu;
    GtkWidget 	*search_type;
    GtkWidget *	search_name;
    GtkWidget *	hl_frame;
    GtkWidget *	hl_table;
    attr_info *	ainfo;
    int             nfaces;
    GtkWidget *select_button;
    int x1, y1, x2, y2;
    int button_down;
    GtkWidget *color, *texture;
    bbox_t selections[MAX_SELECT];
    int    nselections;
}
pop_win_t;

extern pop_win_t popup_window;

void do_img_popup(GtkWidget *widget, search_set *set
                 );

void search_popup_add(img_search *ssearch, int nsearch);

#endif	/* ! _SNAP_POPUP_H_ */
