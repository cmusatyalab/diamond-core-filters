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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>
#include <libgen.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <gtk/gtk.h>
#include "attr_ent.h"


static void
format_hex_data(unsigned char *data, size_t dlen, char *string, size_t slen)
{
	size_t	doff = 0, soff = 0;

	/* make sure we have data and room for the data, we need at
	 * least 3 bytes to continue because we need 2 for the next text
	 * and 1 for the termination.
	 */
	while ((doff < dlen) && (soff < (slen - 3))) {
		sprintf(string + soff, "%02x", data[doff]);
		doff++;
		soff += 2;
	}
	string[soff] = 0;
}

static int
format_text_data(unsigned char *data, size_t dlen, char *string, size_t slen)
{
	size_t	doff = 0, soff = 0;
	int	ret = 0;

	/* make sure we have data and room for the data, we need at
	 * least 2 bytes to continue because we need 1 for the next char
	 * and 1 for the termination.
	 */
	while ((doff < dlen) && (soff < (slen - 2))) {
		if (!isprint(data[doff])) {
			if ((data[doff] == 0) && (doff == (dlen - 1))) {
				break;
			} else {
				sprintf(string + soff, ".");
				ret = 1;
			}
		} else {
			sprintf(string + soff, "%c", data[doff]);
		}
		doff++;
		soff++;
	}
	string[soff] = 0;
	return(ret);
}

static int
format_int_data(unsigned char *data, size_t dlen, char *string, size_t slen)
{
	int	dval;

	dval = *((int *)data);
	sprintf(string, "%d", dval);
	return(0);
}

attr_ent::attr_ent(const char *name, void *data, size_t dlen)
{
	int	err;

	strncpy(display_name, name, MAX_DISP_ATTR - 1);
	display_name[MAX_DISP_ATTR - 1] = 0;

	err = format_text_data((unsigned char *)data, dlen, display_data, 	
		DUMP_WIDTH);
	if (err) {
		format_hex_data((unsigned char *)data, dlen, display_data, 	
			DUMP_WIDTH);
		fmt_type = FORMAT_TYPE_HEX;
	} else {
		fmt_type = FORMAT_TYPE_STRING;
	}
	rawdata = data;
	data_size = dlen;
}


attr_ent::~attr_ent()
{
	return;
}

const char *
attr_ent::get_name() const
{
	return(display_name);
}

GtkWidget *
attr_ent::get_name_widget()
{
	GtkWidget *	hbox;
	GtkWidget *	widget;

	hbox = gtk_hbox_new(FALSE, 10);
	widget = gtk_label_new(get_name());
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	return(hbox);

}

const char *
attr_ent::get_dstring() const
{
	return(display_data);
}

GtkWidget *
attr_ent::get_data_widget()
{
	GtkWidget *	widget;
	GtkWidget *	hbox;

	hbox = gtk_hbox_new(FALSE, 10);
	widget = gtk_label_new(get_dstring());

	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	data_label = widget;

	return(hbox);	
}


int
attr_ent::get_len() 
{
	return(data_size);
}


GtkWidget *
attr_ent::get_len_widget() 
{
	GtkWidget *	widget;
	GtkWidget *	hbox;
	char		str[50];	/* XXX */

	snprintf(str, 50 - 1, "%d", get_len());
	str[49] = 0;

	hbox = gtk_hbox_new(FALSE, 10);
	widget = gtk_label_new(str);
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	return(hbox);	
}


GtkWidget *
get_type_menu(void)
{
        GtkWidget *     menu;
        GtkWidget *     item;

        menu = gtk_menu_new();

	/* XXX typedefs */
	item = gtk_menu_item_new_with_label("text");
	g_object_set_data(G_OBJECT(item), "user data", 
		(void *)FORMAT_TYPE_STRING);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label("hex");
	g_object_set_data(G_OBJECT(item), "user data", (void *)FORMAT_TYPE_HEX);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label("int");
	g_object_set_data(G_OBJECT(item), "user data", (void *)FORMAT_TYPE_INT);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

        return(menu);
}


static void
type_changed_cb(GtkButton *item, gpointer data)
{
	attr_ent *aent = (attr_ent *)data;
	aent->update_type();
}



GtkWidget *
attr_ent::get_type_widget()
{
	GtkWidget *	widget;
	GtkWidget *	hbox;

	hbox = gtk_hbox_new(FALSE, 10);
	type_menu = get_type_menu();
	gtk_menu_set_active(GTK_MENU(type_menu), fmt_type);

	widget = gtk_option_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(widget), type_menu);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);

        /* put callback to update changes to the name box */
        g_signal_connect(G_OBJECT(widget), "changed",
                         G_CALLBACK(type_changed_cb), this);

	return(hbox);	
}

void
attr_ent::update_type()
{
	GtkWidget *	active;
	int		type;

	active = gtk_menu_get_active(GTK_MENU(type_menu));
     	type = (int) g_object_get_data(G_OBJECT(active), "user data");

	switch(type) {
		case FORMAT_TYPE_STRING:
			format_text_data((unsigned char *)rawdata, data_size, 
			    display_data, DUMP_WIDTH - 1);
			break;
			
		case FORMAT_TYPE_HEX:
			format_hex_data((unsigned char *)rawdata, data_size, 
			    display_data, DUMP_WIDTH - 1);
			break;

		case FORMAT_TYPE_INT:
			format_int_data((unsigned char *)rawdata, data_size, 
			    display_data, DUMP_WIDTH - 1);
			break;
	}
	gtk_label_set_text(GTK_LABEL(data_label), display_data);
}

attr_ent &
attr_ent::operator = (const attr_ent &rhs)
{
	return *this;
}


int
attr_ent::operator < (const attr_ent &rhs) const
{
	const char * t1;
	const char * t2;

	t1 = get_name();
	t2 = rhs.get_name();

	if (strcmp(t1, t2) < 0) {
		return(1);
	} else {
		return(0);
	}
}

int
attr_ent::operator == (const attr_ent &rhs) const
{
	const char * t1;
	const char * t2;

	t1 = get_name();
	t2 = rhs.get_name();

	return(!strcmp(t1, t2));
}


