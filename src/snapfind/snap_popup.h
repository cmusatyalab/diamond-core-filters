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
  GtkWidget 	*existing_menu;
  GtkWidget 	*search_type;
  GtkWidget *	search_name;
  GtkWidget *	hl_frame;
  GtkWidget *	hl_table;
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
