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
