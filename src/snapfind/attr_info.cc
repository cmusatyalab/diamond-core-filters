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

/*
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */


#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <assert.h>
#include "lib_searchlet.h"
#include "lib_filter.h"
#include "attr_info.h"

attr_info::attr_info()
{
	active_display = 0;
	attr_list.erase(attr_list.begin(), attr_list.end());
	return;

}

attr_info::~attr_info()
{
	attr_list.erase(attr_list.begin(), attr_list.end());
	return;
}



void
attr_info::update_obj(ls_obj_handle_t ohandle)
{
	attr_ent *aent;
	int	  err;
	char *	  aname;
	unsigned char *	  data;
	size_t    len;
	void *	  cookie;

	/* clear out the old list */
	attr_list.erase(attr_list.begin(), attr_list.end());

	/* put in all the current attributes */
	err = lf_first_attr(ohandle, &aname, &len, &data, &cookie);
	while (!err) {
		aent = new attr_ent(aname, data, len);
		attr_list.push_back(aent);
		err = lf_next_attr(ohandle, &aname, &len, &data, &cookie);
	}

	/* if active, update the window */
	if (active_display) {
        	gtk_container_remove(GTK_CONTAINER(container), display_table);
        	display_table = get_table();
		gtk_box_pack_start(GTK_BOX(container), display_table, 
			FALSE, FALSE, 0);
		gtk_widget_show_all(container);
	}

}

int
attr_info::num_attrs()
{
	return(attr_list.size());
}


GtkWidget *
attr_info::get_table()
{

	GtkWidget *	table;
	GtkWidget *	widget;
	GtkWidget *	hbox;
	int		row = 0;
	attr_iter_t	iter;

	/* XXX put table in container */
	table = gtk_table_new(num_attrs()+1, 4, FALSE);

	gtk_table_set_row_spacings(GTK_TABLE(table), 2);
	gtk_table_set_col_spacings(GTK_TABLE(table), 4);
	gtk_container_set_border_width(GTK_CONTAINER(table), 4);

	hbox = gtk_hbox_new(FALSE, 10);
	widget = gtk_label_new("Type");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(table), hbox, 0, 1, row, row+1);

	hbox = gtk_hbox_new(FALSE, 10);
	widget = gtk_label_new("Attr Name");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(table), hbox, 1, 2, row, row+1);

	hbox = gtk_hbox_new(FALSE, 10);
	widget = gtk_label_new("Len");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(table), hbox, 2, 3, row, row+1);

	hbox = gtk_hbox_new(FALSE, 10);
	widget = gtk_label_new("Data");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(table), hbox, 3, 4, row, row+1);

	for (iter = attr_list.begin(); iter != attr_list.end(); iter++) {
		row++;
		widget = (*iter)->get_type_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 
			row, row+1);
		widget = (*iter)->get_name_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 	
			row, row+1);
		widget = (*iter)->get_len_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 2, 3,
			row, row+1);
		widget = (*iter)->get_data_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 3, 4,
			row, row+1);
	}
	gtk_widget_show_all(table);
	return(table);
}

GtkWidget *
attr_info::get_display()
{

	active_display = 1;
	container = gtk_vbox_new(FALSE, 10);
	display_table = get_table();
	gtk_box_pack_start(GTK_BOX(container), display_table, FALSE, FALSE, 0);
	gtk_widget_show_all(container);
	return(container);
}
