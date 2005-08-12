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

#ifndef	_GTK_IMAGE_TOOLS_H_
#define	_GTK_IMAGE_TOOLS_H_	1

#include <stdint.h>
#include "rgb.h"
#include <opencv/cv.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C"
{
#endif

	GdkPixbuf* rgbimage_to_pixbuf(RGBImage *img);
	GtkWidget* rgbimage_to_gtkimage(RGBImage *img);
	void show_popup_error(const char *lable, const char *err_str, GtkWidget *win);


#ifdef __cplusplus
}
#endif

#endif	/* !_GTK_IMAGE_TOOLS_H_ */
