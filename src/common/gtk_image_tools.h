#ifndef	_GTK_IMAGE_TOOLS_H_
#define	_GTK_IMAGE_TOOLS_H_	1

#include <stdint.h>
#include "face.h"
#include "rgb.h"
#include <opencv/cv.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

GdkPixbuf* rgbimage_to_pixbuf(RGBImage *img);

GtkWidget* rgbimage_to_gtkimage(RGBImage *img);

	
#ifdef __cplusplus
}
#endif

#endif	/* !_GTK_IMAGE_TOOLS_H_ */
