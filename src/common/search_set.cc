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
#include "queue.h"
#include "rgb.h"
#include "image_tools.h"
#include "img_search.h"
#include "search_set.h"

search_set::search_set()
{
	ss_dep_list.erase(ss_dep_list.begin(), ss_dep_list.end());
	printf("search set: %p \n", this);
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
}


void
search_set::remove_search(img_search *old_search)
{
	/* XXX */
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

	/* we always do rgb, XXX should we ??*/
	rgb = new rgb_img("RGB image", "RGB image");
	add_dep(rgb);
	
	reset_search_iter(&iter);
	while ((srch = get_next_search(&iter)) != NULL) {
		if (srch->is_selected()) {
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
	gtk_widget_show(widget);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, row, row+1);

    widget = gtk_label_new("Description");
    gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_widget_show(widget);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, row, row+1);

    widget = gtk_label_new("Edit");
    gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_widget_show(widget);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 2, 3, row, row+1);


	reset_search_iter(&iter);
	while ((srch = get_next_search(&iter)) != NULL) {
		row++;
		widget = srch->get_search_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, row, row+1);
		widget = srch->get_config_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, row, row+1);
		widget = srch->get_edit_widget();
		gtk_table_attach_defaults(GTK_TABLE(table), widget, 2, 3, row, row+1);
	}
    gtk_widget_show(table);
	return(table);
}
