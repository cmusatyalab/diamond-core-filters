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
#include <sys/queue.h>
#include "rgb.h"
#include "lib_results.h"
#include "lib_sfimage.h"
#include "lib_results.h"
#include "img_search.h"
#include "search_set.h"
#include "factory.h"
#include "snapfind.h"

/* global state allocation */
search_name_t *	active_searches = NULL;
search_name_t **active_end = &active_searches;

search_set::search_set()
{
	ss_dep_list.erase(ss_dep_list.begin(), ss_dep_list.end());
	return;

}

search_set::~search_set()
{
	/* XXX free any images */
	return;
}

void
search_set::add_search(img_search *new_search)
{
	new_search->set_parent(this);
	ss_search_list.push_back(new_search);

	/* invoke the update cb to tell everyone the set has changed */
	notify_update();
}


void
search_set::remove_search(img_search *old_search)
{
	/* XXX */
	notify_update();
}

void
search_set::add_dep(img_search *dep_search)
{
	img_search *check;
	search_iter_t iter;

	reset_dep_iter(&iter);
	while((check = get_next_dep(&iter)) != NULL) {
		if (*dep_search == *check) {
			delete dep_search;
			return;
		}
	}

	dep_search->set_parent(this);
	ss_dep_list.push_back(dep_search);
}


void
search_set::clear_deps()
{
	img_search *old;

	while (ss_dep_list.size() > 0) {
		old = ss_dep_list.back();
		ss_dep_list.pop_back();
		delete old;
	}
}



void
search_set::reset_search_iter(search_iter_t *iter)
{
	*iter = ss_search_list.begin();
}


img_search *
search_set::get_next_search(search_iter_t *iter)
{
	img_search  *	dsearch;

	if (*iter == ss_search_list.end()) {
		return(NULL);
	}

	dsearch = **iter;
	(*iter)++;
	return(dsearch);
}

img_search *
search_set::find_search(char *name, search_iter_t *iter)
{
    img_search *cur;
    while ((cur = get_next_search(iter)) != NULL) {
      if (cur->matches_filter(name)) {
	  return(cur);
      }
    }
    return(NULL);
}

void
search_set::reset_dep_iter(search_iter_t *iter)
{
	*iter = ss_dep_list.begin();
}


img_search *
search_set::get_next_dep(search_iter_t *iter)
{
	img_search  *	dsearch;

	if (*iter == ss_dep_list.end()) {
		return(NULL);
	}
	dsearch = **iter;
	(*iter)++;
	return(dsearch);
}

int
search_set::get_search_count()
{
	return(ss_search_list.size());
}

/*
 * Add a new callback function to the list of callbacks.
 */
void
search_set::register_update_fn(sset_notify_fn cb_fn)
{
	ss_cb_vector.push_back(cb_fn);
}

/*
 * go through the list of callback functions and remove the entry
 * that has the same value (if it exists).
 */
void
search_set::un_register_update_fn(sset_notify_fn cb_fn)
{
	cb_iter_t	cur;

	for (cur = ss_cb_vector.begin(); cur != ss_cb_vector.end(); cur++) {
		if (*cur == cb_fn) {
			ss_cb_vector.erase(cur);
			return;
		}
	}
}

/*
 * go through the list of callback functions and let everyone know
 * that the set has been updated.
 */
void
search_set::notify_update()
{
	cb_iter_t	cur;

	for (cur = ss_cb_vector.begin(); cur != ss_cb_vector.end(); cur++) {
		(**cur)(this);
	}

}

static void
clear_search_list()
{
	search_name_t	*cur;

	while (active_searches != NULL) {
		cur = active_searches;
		active_searches = cur->sn_next;

		free(cur->sn_name);
		free(cur);
	}
	active_end = &active_searches;
}

static void
add_search_to_list(img_search *srch)
{
	search_name_t	*cur;

	cur = (search_name_t *)malloc(sizeof(*cur));
	assert(cur != NULL);

	cur->sn_name = strdup(srch->get_name());
	assert(cur->sn_name != NULL);

	cur->sn_next = NULL;
	*active_end = cur;
	active_end = &cur->sn_next;
}


/*
 * Build the filters specification into the temp file name
 * "tmp_file".  We walk through all the activated regions and
 * all the them to write out the file.
 */

char *
search_set::build_filter_spec(char *tmp_file)
{
	char * 		tmp_storage = NULL;
	FILE *		fspec;
	int			err;
	int         fd;
	img_search *		srch;
	img_search *		rgb;
	img_factory *		ifac;
	search_iter_t		iter;

	/* XXX who frees this ?? */
	tmp_storage = (char *)malloc(L_tmpnam);
	if (tmp_storage == NULL) {
		printf("XXX failed to alloc memory !!! \n");
		return(NULL);
	}

	if(!tmp_file) {
		tmp_file = tmp_storage;
		sprintf(tmp_storage, "%sXXXXXX", "/tmp/filspec");
		fd = mkstemp(tmp_storage);
	} else {
		fd = open(tmp_file, O_RDWR|O_CREAT|O_TRUNC, 0666);
	}

	if(fd < 0) {
		perror(tmp_file);
		free(tmp_storage);
		return NULL;
	}
	fspec = fdopen(fd, "w+");
	if (fspec == NULL) {
		perror(tmp_file);
		free(tmp_storage);
		return(NULL);
	}

	/* clear the dependancies */
	clear_deps();

     	ifac = find_rgbimage_factory("rgb_image");
        assert(ifac != NULL);
        rgb = ifac->create("RGB image");
	add_dep(rgb);

	clear_search_list();

	for (iter = ss_search_list.begin(); iter != ss_search_list.end(); 
	    iter++) {
		srch = *iter;
		if (srch->is_selected()) {
			add_search_to_list(srch);
			srch->save_edits();
			srch->write_fspec(fspec);
		}
	}

	/* write dependency list */
	reset_dep_iter(&iter);
	while ((srch = get_next_dep(&iter)) != NULL) {
		srch->write_fspec(fspec);
	}

	fprintf(fspec, "FILTER  APPLICATION  # dependancies \n");
	fprintf(fspec, "REQUIRES  RGB  # dependancies \n");

	/* close the file */
	err = fclose(fspec);
	if (err != 0) {
		printf("XXX failed to close file \n");
		free(tmp_storage);
		return(NULL);
	}

	return(tmp_file);
}

void
search_set::write_blobs(ls_search_handle_t shandle)
{
	img_search * srch;
	search_iter_t iter;


	for (iter = ss_search_list.begin(); iter != ss_search_list.end(); 
	    iter++) {
		srch = *iter;
		if (srch->is_selected() &&
			(srch->get_auxiliary_data() != NULL)) {
			if (ls_set_blob(shandle, (char *)srch->get_name(), 
			     srch->get_auxiliary_data_length(), 
			     srch->get_auxiliary_data())) {
				fprintf(stderr, "failed to write blob \n");
			}
		}
	}

}

GtkWidget *
search_set::build_edit_table()
{

	GtkWidget *	table;
	GtkWidget *	widget;
	int			row = 0;
	img_search *srch;
	search_iter_t	iter;

	table = gtk_table_new(get_search_count()+1, 3, FALSE);

	gtk_table_set_row_spacings(GTK_TABLE(table), 2);
	gtk_table_set_col_spacings(GTK_TABLE(table), 4);
	gtk_container_set_border_width(GTK_CONTAINER(table), 10);

	widget = gtk_label_new("Predicate");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, row, row+1);

	widget = gtk_label_new("Description");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, row, row+1);

	widget = gtk_label_new("Edit");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 2, 3, row, row+1);


	for (iter = ss_search_list.begin(); iter != ss_search_list.end(); 
	    iter++) {
		srch = *iter;
		row++;
		widget = srch->get_search_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 
			row, row+1);
		widget = srch->get_config_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 	
			row, row+1);
		widget = srch->get_edit_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 2, 3, 
			row, row+1);
	}
	gtk_widget_show_all(table);
	return(table);
}
