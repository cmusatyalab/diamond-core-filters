/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sys/queue.h>
#include "snapfind_consts.h"
#include "rgb.h"
#include "lib_results.h"
#include "lib_sfimage.h"
#include "gtk_image_tools.h"
#include "img_search.h"
#include "import_sample.h"
#include "plugin-runner.h"

/* tokens for the config file */
#define	PATCH_ID	"PATCHFILE"

example_search::example_search(const char *name, const char *descr)
		: window_search(name, descr)
{
	TAILQ_INIT(&ex_plist);
	num_patches = 0;
	patch_table = NULL;
	patch_holder = NULL;
}

example_search::~example_search()
{
	/* XXX example search destruct */
	return;
}

static void
cb_remove_patch(GtkWidget *item, gpointer data)
{
	example_patch_t		*cur_patch;
	example_search *	es;

	/* get the parent object and invoke the remove method */
	cur_patch = (example_patch_t *)data;
	es = (example_search *)cur_patch->parent;
	es->remove_patch(cur_patch);
}

GtkWidget *
example_search::build_patch_table(void)
{
	GtkWidget *			image;
	GtkWidget *			del_button;
	example_patch_t *	cur_patch;
	int					i;

	patch_table = gtk_table_new(num_patches + 1, 3, FALSE);

	gtk_table_set_row_spacings(GTK_TABLE(patch_table), 2);
	gtk_table_set_col_spacings(GTK_TABLE(patch_table), 10);
	gtk_container_set_border_width(GTK_CONTAINER(patch_table), 10);

	i = 0;
	TAILQ_FOREACH(cur_patch, &ex_plist, link) {
		image = rgbimage_to_gtkimage(cur_patch->patch_image);

		gtk_table_attach(GTK_TABLE(patch_table), image, i, i + 1, 0, 1,
		                 (GtkAttachOptions)0,(GtkAttachOptions) 0, 0, 0);

		del_button = gtk_button_new_with_label("Remove");
		g_signal_connect(G_OBJECT(del_button), "clicked",
		                 G_CALLBACK(cb_remove_patch), cur_patch);

		gtk_table_attach(GTK_TABLE(patch_table), del_button, i, i + 1, 1, 2,
		                 (GtkAttachOptions)0,(GtkAttachOptions) 0, 0, 0);

		i++;
	}

	g_object_ref(G_OBJECT(patch_table));
	gtk_widget_show_all(patch_table);
	return(patch_table);

}

int
example_search::add_patch(RGBImage *img, bbox_t bbox)
{
	example_patch_t *	cur_patch;

	num_patches++;
	cur_patch = (example_patch_t *)malloc(sizeof(*cur_patch));
	assert(cur_patch != NULL);


	/* XXX file name history */
	cur_patch->file_name = NULL;
	cur_patch->xoff = bbox.min_x;
	cur_patch->yoff = bbox.min_y;
	cur_patch->xsize = bbox.max_x - bbox.min_x;
	cur_patch->ysize = bbox.max_y - bbox.min_y;

	/* point to the base class */
	cur_patch->parent = this;

	/* Create the patch from base image*/
	cur_patch->patch_image = create_rgb_subimage(img, cur_patch->xoff,
	                         cur_patch->yoff, cur_patch->xsize, cur_patch->ysize);

	/* put it on the list */
	TAILQ_INSERT_TAIL(&ex_plist, cur_patch, link);

	update_display();

	return(1);
}

int
example_search::is_example()
{
	return(1);
}

int
example_search::add_patch(char *fname, char *xoff, char *yoff, char *xsize,
                          char *ysize)
{
	example_patch_t *	cur_patch;
	RGBImage  *			img;

	num_patches++;
	cur_patch = (example_patch_t *)malloc(sizeof(*cur_patch));
	assert(cur_patch != NULL);

	cur_patch->file_name = (char *)malloc(strlen(fname)+1);
	assert(cur_patch->file_name != NULL);

	strncpy(cur_patch->file_name, fname, (strlen(fname) + 1));
	cur_patch->file_name[strlen(fname)] = '\0';


	cur_patch->xoff = atoi(xoff);
	cur_patch->yoff = atoi(yoff);
	cur_patch->xsize = atoi(xsize);
	cur_patch->ysize = atoi(ysize);

	/* point to the base class */
	cur_patch->parent = this;

	/*
	 * We assume that the current working directory has been
	 * changed, so we can use the relative path.
	 */
	img = create_rgb_image(cur_patch->file_name);
	/* XXX do popup and terminate ??? */
	if (img == NULL) {
		fprintf(stderr, "Failed to read patch file %s \n",
		        cur_patch->file_name);
		free(cur_patch);
		return(0);
	}

	cur_patch->patch_image = create_rgb_subimage(img, cur_patch->xoff,
	                         cur_patch->yoff, cur_patch->xsize, cur_patch->ysize);
	/* XXX failure cases ??*/

	release_rgb_image(img);

	/* put it on the list */
	TAILQ_INSERT_TAIL(&ex_plist, cur_patch, link);

	return(0);
}

int
example_search::handle_config(int nconf, char **data)
{
	int		err;

	if (strcmp(PATCH_ID, data[0]) == 0) {
		assert(nconf > 5);
		add_patch(data[1], data[2], data[3], data[4], data[5]);
		err = 0;
	} else {
		err = window_search::handle_config(nconf, data);
	}
	return(err);
}

void
example_search::update_display(void)
{
	GtkWidget *	table;

	if (patch_holder != NULL) {
		g_object_unref(G_OBJECT(patch_table));
		gtk_widget_destroy(patch_table);
		table = build_patch_table();
		gtk_box_pack_start(GTK_BOX(patch_holder), table, FALSE, TRUE, 0);
	}
}

/*
 * This removes the specified example patch.
 */
void
example_search::remove_patch(example_patch_t *patch)
{

	/* remove this from the list of patches */
	TAILQ_REMOVE(&ex_plist, patch, link);

	/* clean up the memory */
	if (patch->file_name != NULL) {
		free(patch->file_name);
	}
	free(patch);

	num_patches--;

	update_display();
}

static void
cb_import(GtkWidget *item, gpointer data)
{
	//open_import_window();
}


GtkWidget *
example_search::example_display()
{
	GtkWidget *			window;
	GtkWidget *			table;
	GtkWidget *			hbox;
	GtkWidget *			import_button;
	GtkWidget *			label;
	GtkWidget *			sep;

	window = gtk_scrolled_window_new(NULL, NULL);

	patch_holder = gtk_vbox_new(FALSE, 10);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(window), patch_holder);


	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(patch_holder), hbox, FALSE,
	                   TRUE, 10);

	label = gtk_label_new("Example Patches: ");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 10);


	import_button = gtk_button_new_with_label("Import");
	g_signal_connect(G_OBJECT(import_button), "clicked",
	                 G_CALLBACK(cb_import), this);

	gtk_box_pack_start(GTK_BOX(hbox), import_button, FALSE,
	                   TRUE, 10);


	sep = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(patch_holder), sep, FALSE, TRUE, 10);


	table = build_patch_table();
	gtk_box_pack_start(GTK_BOX(patch_holder), table, FALSE,
	                   TRUE, 10);

	gtk_widget_show_all(window);
	return(window);
}

void
example_search::close_edit_win()
{
	patch_holder = NULL;
	window_search::close_edit_win();

}

void
example_search::save_edits()
{
	/* XXX any state cleanup */
	window_search::save_edits();
}


void
example_search::write_fspec(FILE *ostream)
{
	window_search::write_fspec(ostream);
	return;
}

void
example_search::write_config(FILE *ostream, const char *dirname)
{
	example_patch_t *		cur_patch;
	int				i;
	int				err;
	char				fname[COMMON_MAX_PATH];


	window_search::write_config(ostream, dirname);

	/* for each of the samples write out the data */
	i = 0;
	TAILQ_FOREACH(cur_patch, &ex_plist, link) {
		if (is_plugin_runner_mode()) {
			char *ppm;
			size_t ppm_size;
			FILE *memfile = open_memstream(&ppm, &ppm_size);
			rgb_write_image_file(cur_patch->patch_image, memfile);
			fclose(memfile);
			char *key = g_strdup_printf("patch-%d", i);
			print_key_value(key, ppm_size, ppm);
			g_free(key);
			free(ppm);
		} else {
			err = sprintf(fname, "%s.ex%d.ppm", get_name(), i);
			if (err >= COMMON_MAX_PATH) {
				fprintf(stderr, "COMMON_MAX_PATH too short increase to %d \n",
					err);
				assert(0);
			}

			rgb_write_image(cur_patch->patch_image, fname, dirname);
			fprintf(ostream, "%s %s 0 0 %d %d \n", PATCH_ID, fname,
				cur_patch->xsize, cur_patch->ysize);
		}
		i++;
	}

	if (is_plugin_runner_mode()) {
		print_key_value("patch-count", i);
	}
	return;
}

