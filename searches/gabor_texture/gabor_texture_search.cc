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
#include <stdlib.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <sys/queue.h>
#include "lib_results.h"
#include "rgb.h"
#include "gabor.h"
#include "gabor_texture_search.h"
#include "factory.h"

#define	MAX_DISPLAY_NAME	64

/* name we use to indentify this type of search */
#define	SEARCH_NAME	"gabor_texture"

/* key words for the config file */
#define	SIMILARITY_ID	"SIMILARITY"
#define	NANGLE_ID	"NUM_ANGLES"
#define	NFREQ_ID	"NUM_FREQ"
#define	RADIUS_ID	"RADIUS"
#define	MIN_FREQ_ID	"MIN_FREQ"
#define	MAX_FREQ_ID	"MAX_FREQ"

extern "C" {
	diamond_public
	void search_init();
}
                                                                                
void
search_init()
{
        gabor_texture_factory *fac;
        fac = new gabor_texture_factory;
        factory_register(fac);
}



gabor_texture_search::gabor_texture_search(const char *name, const char *descr)
		: example_search(name, descr)
{
	edit_window = NULL;
	similarity = 0.93;
	num_angles = 4;
	num_freq = 2;
	radius = 16;
	min_freq = 0.2;
	max_freq = 1.0;

	/* disble scale control in the window search */
	set_scale_control(0);

}


gabor_texture_search::~gabor_texture_search()
{
	return;
}


void
gabor_texture_search::set_similarity(char *data)
{
	similarity = atof(data);
	if (similarity < 0) {
		similarity = 0.0;
	} else if (similarity > 1.0) {
		similarity = 1.0;
	}
	return;
}

void
gabor_texture_search::set_similarity(double sim)
{
	similarity = sim;
	if (similarity < 0) {
		similarity = 0.0;
	} else if (similarity > 1.0) {
		similarity = 1.0;
	}
	return;
}

void
gabor_texture_search::set_radius(int num)
{
	radius = num;
}

void
gabor_texture_search::set_radius(char *data)
{
	int	 num;
	num = atoi(data);
	set_radius(num);
}

void
gabor_texture_search::set_num_angle(int num)
{
	num_angles = num;
}

void
gabor_texture_search::set_num_angle(char *data)
{
	int	 num;
	num = atoi(data);
	set_num_angle(num);
}

void
gabor_texture_search::set_num_freq(int num)
{
	num_freq = num;
}

void
gabor_texture_search::set_num_freq(char *data)
{
	int	 num;
	num = atoi(data);
	set_num_freq(num);
}

void
gabor_texture_search::set_min_freq(float num)
{
	min_freq = num;
}

void
gabor_texture_search::set_min_freq(char *data)
{
	float	 num;
	num = atof(data);
	set_min_freq(num);
}
void
gabor_texture_search::set_max_freq(float num)
{
	max_freq = num;
}

void
gabor_texture_search::set_max_freq(char *data)
{
	float	 num;
	num = atof(data);
	set_max_freq(num);
}





int
gabor_texture_search::handle_config(int num_conf, char **confv)
{
	int	err;

	if (strcmp(SIMILARITY_ID, confv[0]) == 0) {
		if (num_conf < 2) {
			err = 1;
		} else {
			set_similarity(confv[1]);
			err = 0;
		}
	} else if (strcmp(NANGLE_ID, confv[0]) == 0) {
		if (num_conf < 2) {
			err = 1;
		} else {
			set_num_angle(confv[1]);
			err = 0;
		}
	} else if (strcmp(NFREQ_ID, confv[0]) == 0) {
		if (num_conf < 2) {
			err = 1;
		} else {
			set_num_freq(confv[1]);
			err = 0;
		}
	} else if (strcmp(RADIUS_ID, confv[0]) == 0) {
		if (num_conf < 2) {
			err = 1;
		} else {
			set_radius(confv[1]);
			err = 0;
		}
	} else if (strcmp(MIN_FREQ_ID, confv[0]) == 0) {
		if (num_conf < 2) {
			err = 1;
		} else {
			set_min_freq(confv[1]);
			err = 0;
		}
	} else if (strcmp(MAX_FREQ_ID, confv[0]) == 0) {
		if (num_conf < 2) {
			err = 1;
		} else {
			set_max_freq(confv[1]);
			err = 0;
		}
	} else {
		err = example_search::handle_config(num_conf, confv);
	}
	return(err);
}


static void
cb_close_edit_window(GtkWidget* item, gpointer data)
{
	gabor_texture_search *	search;

	search = (gabor_texture_search *)data;
	search->close_edit_win();
}


void
gabor_texture_search::close_edit_win()
{

	/* save any changes from the edit windows */
	save_edits();

	/* call the parent class to give them change to cleanup */
	example_search::close_edit_win();

	edit_window = NULL;

}

static void
edit_search_done_cb(GtkButton *item, gpointer data)
{
	GtkWidget * widget = (GtkWidget *)data;
	gtk_widget_destroy(widget);
}


void
gabor_texture_search::edit_search()
{
	GtkWidget * widget;
	GtkWidget * box;
	GtkWidget * frame;
	GtkWidget * hbox;
	GtkWidget * container;
	char		name[MAX_DISPLAY_NAME];

	/* see if it already exists */
	if (edit_window != NULL) {
		/* raise to top ??? */
		gdk_window_raise(GTK_WIDGET(edit_window)->window);
		return;
	}

	edit_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(name, MAX_DISPLAY_NAME - 1, "Edit %s", get_name());
	name[MAX_DISPLAY_NAME -1] = '\0';
	gtk_window_set_title(GTK_WINDOW(edit_window), name);
	g_signal_connect(G_OBJECT(edit_window), "destroy",
	                 G_CALLBACK(cb_close_edit_window), this);
	box = gtk_vbox_new(FALSE, 10);


	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, TRUE, 0);

	widget = gtk_button_new_with_label("Close");
	g_signal_connect(G_OBJECT(widget), "clicked",
	                 G_CALLBACK(edit_search_done_cb), edit_window);
	GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);


	/*
		 * Get the controls from the img_search.
	 */
	widget = img_search_display();
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);

	/*
	 	 * Create the texture parameters.
	 */

	frame = gtk_frame_new("Texture Params");
	container = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), container);

	widget = create_slider_entry("Min Similarity", 0.0, 1.0, 2,
	                             similarity, 0.05, &sim_adj);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

	widget = create_slider_entry("Radius", 0.0, 80.0, 0,
	                             radius, 2.0, &rad_adj);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

	widget = create_slider_entry("Num Angles", 1.0, 16.0, 0,
	                             num_angles, 1.0, &nangle_adj);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

	widget = create_slider_entry("Num Frequency", 1.0, 16.0, 0,
	                             num_freq, 1.0, &nfreq_adj);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

	widget = create_slider_entry("Min Frequency", 0.0, 5.0, 1,
	                             min_freq, 0.5, &minfreq_adj);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

	widget = create_slider_entry("Max Frequency", 0.0, 5.0, 1,
	                             max_freq, 0.5, &maxfreq_adj);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);

	/*
	 * Get the controls from the window search class.
		 */
	widget = get_window_cntrl();
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	/*
		 * Get the controls from the example search class.
	 */
	widget = example_display();
	gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(edit_window), box);
	gtk_widget_show_all(edit_window);

}

/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
gabor_texture_search::save_edits()
{
	double		fval;
	int		ival;

	/* no active edit window, so return */
	if (edit_window == NULL) {
		return;
	}

	/* get the similarity and save */
	fval = gtk_adjustment_get_value(GTK_ADJUSTMENT(sim_adj));
	set_similarity(fval);

	ival = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(rad_adj));
	set_radius(ival);

	ival = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(nangle_adj));
	set_num_angle(ival);

	ival = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(nfreq_adj));
	set_num_freq(ival);

	fval = gtk_adjustment_get_value(GTK_ADJUSTMENT(minfreq_adj));
	set_min_freq(fval);

	fval = gtk_adjustment_get_value(GTK_ADJUSTMENT(maxfreq_adj));
	set_max_freq(fval);

	/* call the parent class */
	example_search::save_edits();
}

void
gabor_texture_search::release_args(gtexture_args_t *gargs)
{

	int	i;
	delete	gargs->gobj;
	
	for (i=0; i < gargs->num_samples; i++) {
		free(gargs->response_list[i]);
	}
	free(gargs->response_list);
}

int
gabor_texture_search::gen_args(gtexture_args_t *gargs)
{
	int		samples, patch_size, num_resp;
	example_patch_t	*	cur_patch;
	float *			respv;
	int			err;
	int	i;

	gargs->name = strdup(get_name());
	assert(gargs->name != NULL);

	gargs->step = get_stride();
	gargs->xdim = get_testx();
	gargs->ydim = get_testy();
	gargs->min_matches = get_matches();
	gargs->max_distance = 1.0 - similarity;
	gargs->num_angles = num_angles;
	gargs->num_freq = num_freq;
	gargs->radius = radius;
	gargs->max_freq = max_freq;
	gargs->min_freq = min_freq;

	patch_size = 2 * radius + 1;
	/* count the number of patches of the appropriate size*/
	samples = 0;
	TAILQ_FOREACH(cur_patch, &ex_plist, link) {
		if ((cur_patch->patch_image->width < (patch_size+1)) ||
		    (cur_patch->patch_image->height < (patch_size+1))) {
				continue;
		} else {
			samples++;
		}
	}

	/* if no samples we skip out */
	if (samples == 0) {
		return(0);
	}
	
	gargs->num_samples = samples;
	num_resp = num_angles * num_freq;
	gargs->response_list = (float **)malloc(sizeof(float *) * samples);


	gargs->gobj = new gabor(gargs->num_angles, gargs->radius, 
		gargs->num_freq, gargs->max_freq, gargs->min_freq);

	i = 0;
	TAILQ_FOREACH(cur_patch, &ex_plist, link) {
		if ((cur_patch->patch_image->width < patch_size) ||
		    (cur_patch->patch_image->height < patch_size)) {
			continue;
		}

		respv = (float *) malloc(sizeof(float) * num_resp);
		assert(respv != NULL);
		gargs->response_list[i] = respv;

		err = gabor_patch_response(cur_patch->patch_image, gargs, num_resp, respv);
		if (err) {
			fprintf(stderr, "get_response failed\n");
			/* XXX */
			continue;
		}
		i++;
	}
	return(1);
}

/*
 * This write the relevant section of the filter specification file
 * for this search.
 */

void
gabor_texture_search::write_fspec(FILE *ostream)
{
	int		i = 0;
	int 		j;
	int		err;
	int		num_resp;
	gtexture_args_t	gargs;

	save_edits();

	err = gen_args(&gargs);
	if (err == 0) {
		fprintf(stderr, "No patches of large enough size \n");
		return;
	}
	/*
	 * First we write the header section that corrspons
	 * to the filter, the filter name, the assocaited functions.
	 */

	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER %s \n", get_name());
	fprintf(ostream, "THRESHOLD %f \n", 100.0 * similarity);
	fprintf(ostream, "SIGNATURE @\n");

	/*
	 * Next we write call the parent to write out the releated args,
	 * not that since the args are passed as a vector of strings
	 * we need keep the order the args are written constant or silly
	 * things will happen.
	 */
	example_search::write_fspec(ostream);

	/*
	 * Now write the state needed that is just dependant on the histogram
	 * search.  This will have the histo releated parameters
	 * as well as the linearized histograms.
	 */

	fprintf(ostream, "ARG  %f  # similarity \n", 0.0);
	fprintf(ostream, "ARG  %d  # num_angles \n", gargs.num_angles);
	fprintf(ostream, "ARG  %d  # num_freq \n", gargs.num_freq);
	fprintf(ostream, "ARG  %d  # radius \n", gargs.radius);
	fprintf(ostream, "ARG  %f  # max_freq \n", gargs.max_freq);
	fprintf(ostream, "ARG  %f  # min_freq \n", gargs.min_freq);
	fprintf(ostream, "ARG  %d  # num examples \n", gargs.num_samples);

	num_resp = num_angles * num_freq;

	for (i=0; i < gargs.num_samples; i++) {
		for (j=0; j < num_resp; j++) {
			fprintf(ostream, "ARG  %f  # sample %d resp %d \n",
			        gargs.response_list[i][j], i, j);
		}
	}

	release_args(&gargs);

	fprintf(ostream, "REQUIRES  RGB # dependencies \n");
	fprintf(ostream, "MERIT  100 # some relative cost \n");
}

void
gabor_texture_search::write_config(FILE *ostream, const char *dirname)
{
	save_edits();

	fprintf(ostream, "\n\n");
	fprintf(ostream, "SEARCH %s %s\n", SEARCH_NAME, get_name());
	fprintf(ostream, "%s %f \n", SIMILARITY_ID, similarity);
	fprintf(ostream, "%s %d \n", NANGLE_ID, num_angles);
	fprintf(ostream, "%s %d \n", NFREQ_ID, num_freq);
	fprintf(ostream, "%s %d \n", RADIUS_ID, radius);
	fprintf(ostream, "%s %f \n", MIN_FREQ_ID, min_freq);
	fprintf(ostream, "%s %f \n", MAX_FREQ_ID, max_freq);


	example_search::write_config(ostream, dirname);
	return;
}


void
gabor_texture_search::region_match(RGBImage *rimg, bbox_list_t *blist)
{
	int			pass;
	size_t			bsize;
	gabor_ii_img_t *	gii_img;
	gtexture_args_t		gargs;

	save_edits();

	/* get the gabor argument data structure */
	pass = gen_args(&gargs);
	if (pass == 0) {
		fprintf(stderr, "No patches of large enough size \n");
		return;
	}
	gargs.min_matches = INT_MAX;	/* get all the matches */

	bsize = (GII_SIZE(rimg->width, rimg->height, &gargs));

  	gii_img = (gabor_ii_img_t *)malloc(bsize);

        assert(gii_img != NULL);
        gabor_init_ii_img(rimg->width, rimg->height, &gargs, gii_img);
        gabor_compute_ii_img(rimg, &gargs, gii_img);

	/* generate list of bounding boxes */
	pass = gabor_test_image(gii_img, &gargs, blist);

	/* cleanup */
	release_args(&gargs);
	return;
}

bool
gabor_texture_search::is_editable(void)
{
	return true;
}
