/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_GRAPH_WIN_H_
#define	_GRAPH_WIN_H_	1

#include <iostream>
#include <vector>

#include <gtk/gtk.h>
#include "snapfind_consts.h"
#include "rgb.h"

using	namespace	std;

/* different pixel offset for placing text and other goodies */
#define		X_ZERO_OFFSET		70
#define		X_FIRST_TEXT_OFFSET	10
#define		X_END_OFFSET		30
#define		X_END_TEXT_OFFSET	20

#define		Y_ZERO_OFFSET		20
#define		Y_END_OFFSET		20
#define		Y_TEXT_GAP		5
#define		Y_ZERO_TEXT_OFFSET	10

#define		Y_LABEL_GAP		5


/* The maximum number of series the graph supports */
#define		GW_MAX_SERIES		8

enum gw_layers_t {
    GW_IMG_LAYER = 0,
    GW_RES_LAYER,
    GW_HIGHLIGHT_LAYER,
    GW_SELECT_LAYER,
    GW_MAX_LAYERS
};

typedef struct
{
	double	x;
	double	y;
}
point_t;

typedef	vector<point_t>::iterator	point_iter_t;

typedef struct
{
	GdkGC *		gc;
	GdkColor	color;
	int			lastx;
	int			lasty;
	/* XXX hold the points when I am cool */
	vector<point_t>	points;
}
series_info_t;


/*
 * This is a c++ class that creates a gtk graph window.
 */
class graph_win
{
public:
	graph_win(const double xmin, const double xmax, const double ymin, const double ymax);

	virtual ~graph_win();

	GtkWidget *	get_graph_display(const int xsize, const int ysize);


	void	add_point(const double x, const double y, int series);
	void	draw_point(const double x, const double y, int series);
	void	clear_series(int series);

	void	clear_graph();
	void 	event_realize();
	void 	draw_res(GtkWidget *widget);
	void 	process_expose(GtkWidget *widget, GdkEventExpose *event);
	void 	init_window();




private:
	void	get_series_color(int series, GdkColor * color);
	void	init_series();
	void	redraw_series();
	void	redraw_series(int series);
	void	scale_x_up(double x);
	void	scale_y_up(double y);


	/* XXX create some state to keep track of the points */

	double	gw_xmin;
	double	gw_xmax;
	double	gw_xspan;
	double	gw_orig_xmax;

	double	gw_ymin;
	double	gw_ymax;
	double	gw_yspan;
	double	gw_orig_ymax;


	int		gw_width;
	int		gw_height;
	int		gw_xdisp;
	int		gw_ydisp;

	int		gw_active_win;

	series_info_t	gw_series[GW_MAX_SERIES];

	char *	display_name;
	char *	descript;
	int		search_selected;
	int		hl_selected;
	GtkWidget *	gw_image_area;
	GtkWidget *	gw_drawingarea;
	GtkWidget *	search_label;
	GtkWidget *	adjust_label;

	GdkPixbuf  *    gw_pixbufs[GW_MAX_LAYERS];
	RGBImage   *    gw_layers[GW_MAX_LAYERS];
	RGBImage   *    gw_cur_img;
	GdkPixmap  *    gw_pixmap;


};


#endif	/* !_GRAPH_WIN_H_ */
