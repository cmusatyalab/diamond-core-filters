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

#ifndef _DISPLAY_IMG_H_
#define _DISPLAY_IMG_H_	1

#include "image_common.h"


#define	MAX_SELECT	24

/*
 * state required to support popup window to show fullsize img
 */

enum display_layers_t {
	DI_IMG_LAYER = 0,
	DI_RES_LAYER,
	DI_HIGHLIGHT_LAYER,
	DI_SELECT_LAYER,
	DI_MAX_LAYERS
};


class display_img {
public:
	display_img(int width, int height);
	virtual ~display_img();

	GtkWidget *get_display();

	void 		set_image(RGBImage *img);
	GtkWidget *clear_image();
	void		process_expose(GtkWidget *widget,
				GdkEventExpose *event);
	void		event_realize();
	void		draw_res(GtkWidget *widget);
	void		motion_notify(GtkWidget *widget,
				GdkEventMotion *event);
	void		button_press(GtkWidget *widget,
				GdkEventButton *event);
	void		button_release(GtkWidget *widget,
				GdkEventButton *event);


	void 		redraw_selections();
	void 		draw_selection(GtkWidget *widget);
	void 		clear_cur_selection(GtkWidget *widget);


	void 		clear_selections();
	void 		clear_highlight();



private:
	GtkWidget *	di_drawingarea;
	GtkWidget *	di_image_area;
  	GdkPixbuf  *	di_pixbufs[DI_MAX_LAYERS];
  	RGBImage   *	di_layers[DI_MAX_LAYERS];
  	bbox_t 		di_selections[MAX_SELECT];
  	int    		di_nselections;
	RGBImage   *	di_cur_img;
  	int 		x1, y1, x2, y2;
	pthread_mutex_t	di_mutex;
  	int 		di_button_down;
  	int 		di_width;
  	int 		di_height;
};

#endif	/* ! _DISPLAY_IMG_H_ */
