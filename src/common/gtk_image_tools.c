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


#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h> 
#include <opencv/cv.h>
#include <gtk/gtk.h>

#include "rgb.h"
#include "gtk_image_tools.h"

/*
 * build a pixbuf from an rgbimage.
 */
GdkPixbuf      *
rgbimage_to_pixbuf(RGBImage * img)
{
    GdkPixbuf      *pbuf;

    /*
     * NB pixbuf refers to data 
     */
    pbuf = gdk_pixbuf_new_from_data((const guchar *) &img->data[0],
                                    GDK_COLORSPACE_RGB, 1, 8,
                                    img->columns, img->rows,
                                    (img->columns * sizeof(RGBPixel)),
                                    NULL, NULL);
    if (pbuf == NULL) {
        printf("failed to allocate pbuf\n");
        exit(1);
    }
    return pbuf;
}


/*
 * Build GTK image from an RGB image.
 */
GtkWidget      *
rgbimage_to_gtkimage(RGBImage * img)
{
    GdkPixbuf      *pbuf;
    GtkWidget      *image;

    pbuf = gdk_pixbuf_new_from_data((const guchar *) &img->data[0],
                                    GDK_COLORSPACE_RGB, 1, 8,
                                    img->columns, img->rows,
                                    (img->columns * sizeof(RGBPixel)),
                                    NULL, NULL);
    if (pbuf == NULL) {
        printf("failed to allocate pbuf\n");
        exit(1);
    }

    image = gtk_image_new_from_pixbuf(pbuf);
    assert(image);
    return image;
}

void
show_popup_error(const char *source, const char *err_str, GtkWidget *win)
{
	GtkWidget *	dialog;
	GtkWidget *	label;
	int			result;
	
 	dialog = gtk_dialog_new_with_buttons(source, GTK_WINDOW(win),
		GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_STOCK_OK, GTK_RESPONSE_NONE, NULL);
	label = gtk_label_new(err_str);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

}

