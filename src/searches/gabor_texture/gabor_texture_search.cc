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


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include "queue.h"
#include "rgb.h"
#include "image_tools.h"
#include "texture_tools.h"
#include "img_search.h"
#include "search_set.h"
#include "gabor.h"
#include "read_config.h"
#include "gabor_texture_search.h"

#define	MAX_DISPLAY_NAME	64


void
gabor_texture_init()
{
        gabor_texture_factory *fac;
        printf("gabor_texture init \n");
                                                                                
        fac = new gabor_texture_factory;
                                                                                
        read_config_register("gabor_texture_factory", fac);
}



gabor_texture_search::gabor_texture_search(const char *name, char *descr)
		: example_search(name, descr)
{
	edit_window = NULL;
	simularity = 0.93;
	channels = 3;
	num_angles = 4;
	num_freq = 2;
	radius = 32;
	min_freq = 0.2;
	max_freq = 1.0;

	/* disble some stride and scale controls in the window search */
	set_scale_control(0);
	set_size_control(0);

	distance_metric = TEXTURE_DIST_PAIRWISE;
}

gabor_texture_search::~gabor_texture_search()
{
	return;
}


void
gabor_texture_search::set_simularity(char *data)
{
	simularity = atof(data);
	if (simularity < 0) {
		simularity = 0.0;
	} else if (simularity > 1.0) {
		simularity = 1.0;
	}
	return;
}

void
gabor_texture_search::set_simularity(double sim)
{
	simularity = sim;
	if (simularity < 0) {
		simularity = 0.0;
	} else if (simularity > 1.0) {
		simularity = 1.0;
	}
	return;
}

/* this is currently not used, but we may want to add it back in */
void
gabor_texture_search::set_channels(int num)
{
	assert((num == 1) || (num == 3));
	channels = num;
}

void
gabor_texture_search::set_channels(char *data)
{
	int	 num;
	num = atoi(data);
	set_channels(num);
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
gabor_texture_search::handle_config(config_types_t conf_type, char *data)
{
	int	err;

	switch (conf_type) {
		case METRIC_TOK:
			set_simularity(data);
			err = 0;
			break;

		case CHANNEL_TOK:
			set_channels(data);
			err = 0;
			break;

		default:
			err = example_search::handle_config(conf_type, data);
			break;
	}
	return(err);
}



static void
cb_update_menu_select(GtkWidget* item, GtkUpdateType  policy)
{
	/* XXXX do something ?? */
}


static GtkWidget *
create_slider_entry(char *name, float min, float max, int dec, float initial,
                    float step, GtkObject **adjp)
{
	GtkWidget *container;
	GtkWidget *scale;
	GtkWidget *button;
	GtkWidget *label;



	container = gtk_hbox_new(FALSE, 10);

	label = gtk_label_new(name);
	gtk_box_pack_start(GTK_BOX(container), label, FALSE, FALSE, 0);

	if (max <= 1.0) {
		max += 0.1;
		*adjp = gtk_adjustment_new(min, min, max, step, 0.1, 0.1);
	} else if (max < 50) {
		max++;
		*adjp = gtk_adjustment_new(min, min, max, step, 1.0, 1.0);
	} else {
		max+= 10;
		*adjp = gtk_adjustment_new(min, min, max, step, 10.0, 10.0);
	}
	gtk_adjustment_set_value(GTK_ADJUSTMENT(*adjp), initial);

	scale = gtk_hscale_new(GTK_ADJUSTMENT(*adjp));
	gtk_widget_set_size_request (GTK_WIDGET(scale), 200, -1);
	gtk_range_set_update_policy (GTK_RANGE(scale), GTK_UPDATE_CONTINUOUS);
	gtk_scale_set_draw_value (GTK_SCALE(scale), FALSE);
	gtk_box_pack_start (GTK_BOX(container), scale, TRUE, TRUE, 0);
	gtk_widget_set_size_request(scale, 120, 0);

	button = gtk_spin_button_new(GTK_ADJUSTMENT(*adjp), step, dec);
	gtk_box_pack_start(GTK_BOX(container), button, FALSE, FALSE, 0);

	gtk_widget_show(container);
	gtk_widget_show(label);
	gtk_widget_show(scale);
	gtk_widget_show(button);

	return(container);
}

/* XXX redunant, with rgb */

static GtkWidget *
make_menu_item (gchar* name, GCallback callback, gpointer  data)
{
	GtkWidget *item;

	item = gtk_menu_item_new_with_label(name);
	g_signal_connect (G_OBJECT (item), "activate", callback, (gpointer) data);
	gtk_widget_show(item);

	return item;
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
	GtkWidget * opt;
	GtkWidget * item;
	GtkWidget * frame;
	GtkWidget * hbox;
	GtkWidget * container;
	GtkWidget * menu;
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

	widget = create_slider_entry("Min Simularity", 0.0, 1.0, 2,
	                             simularity, 0.05, &sim_adj);
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





#ifdef	XXX
	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(container), hbox, FALSE, TRUE, 0);

	gray_widget = gtk_radio_button_new_with_label(NULL, "Grayscale");
	gtk_box_pack_start(GTK_BOX(hbox), gray_widget, FALSE, TRUE, 0);
	rgb_widget = gtk_radio_button_new_with_label_from_widget(
	                 GTK_RADIO_BUTTON(gray_widget), "Color");
	gtk_box_pack_start(GTK_BOX(hbox), rgb_widget, FALSE, TRUE, 0);

	if (channels == 3) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rgb_widget), TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gray_widget), TRUE);
	}
#endif

#ifdef	XXX
	distance_menu = gtk_option_menu_new();
	menu = gtk_menu_new();

	/* these must be declared as the order of the enum  */
	item = gtk_menu_item_new_with_label("Maholonibis");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	item = gtk_menu_item_new_with_label("Variance");
	gtk_menu_shell_append(GTK_MENU_SHELL (menu), item);
	item = gtk_menu_item_new_with_label("Pairwise");
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item);

	gtk_option_menu_set_menu(GTK_OPTION_MENU(distance_menu), menu);
	gtk_box_pack_start(GTK_BOX(container), distance_menu, FALSE, TRUE, 0);
	/* set the default value in the GUI */
	gtk_option_menu_set_history(GTK_OPTION_MENU(distance_menu), (guint)distance_metric);




	opt = gtk_option_menu_new();
	menu = gtk_menu_new();

	item = make_menu_item("Difference of Gaussians",
	                      G_CALLBACK(cb_update_menu_select), GINT_TO_POINTER(0));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	gtk_option_menu_set_menu(GTK_OPTION_MENU (opt), menu);
	gtk_box_pack_start(GTK_BOX(container), opt, FALSE, TRUE, 0);
#endif

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

	/* get the simularity and save */
	fval = gtk_adjustment_get_value(GTK_ADJUSTMENT(sim_adj));
	set_simularity(fval);

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

#ifdef	XXX
	color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rgb_widget));
	if (color) {
		set_channels(3);
	} else {
		set_channels(1);
	}

	distance_metric =
	    (texture_dist_t)gtk_option_menu_get_history(GTK_OPTION_MENU(distance_menu));

#endif
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
	RGBImage	*	rimg;
	float *			respv;
	int			err;
	int	i;

	gargs->name = strdup(get_name());
	assert(gargs->name != NULL);

	gargs->scale = get_scale();	/* XXX ignored for now */
	gargs->step = get_stride();
	gargs->min_matches = get_matches();
	gargs->max_distance = 1.0 - simularity;
	gargs->num_angles = num_angles;
	gargs->num_freq = num_freq;
	gargs->radius = radius;
	gargs->max_freq = max_freq;
	gargs->min_freq = min_freq;

	patch_size = 2 * radius + 1;
	/* count the number of patches of the appropriate size*/
	samples = 0;
	TAILQ_FOREACH(cur_patch, &ex_plist, link) {
		if ((cur_patch->patch_image->width < patch_size) ||
		    (cur_patch->patch_image->height < patch_size)) {
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

		/* XXX scale this later */
		rimg = create_rgb_subimage(cur_patch->patch_image,
		                           0, 0, patch_size, patch_size);

		respv = (float *) malloc(sizeof(float) * num_resp);
		assert(respv != NULL);
		gargs->response_list[i] = respv;

		err = gargs->gobj->get_responses(rimg, 0, 0, num_resp, respv);
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
	img_search *	rgb;
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
	fprintf(ostream, "THRESHOLD %d \n", (int)(100.0 * simularity));
	fprintf(ostream, "EVAL_FUNCTION  f_eval_gab_texture \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_gab_texture \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_gab_texture \n");
	fprintf(ostream, "ARG  %s  # name (helps debug) \n", get_name());

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

	fprintf(ostream, "ARG  %f  # simularity \n", 0.0);
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

	rgb = new rgb_img("RGB image", "RGB image");
	(this->get_parent())->add_dep(rgb);
}

void
gabor_texture_search::write_config(FILE *ostream, const char *dirname)
{

	save_edits();

	/* create the search configuration */
	fprintf(ostream, "\n\n");
	fprintf(ostream, "SEARCH texture %s\n", get_name());

	fprintf(ostream, "METRIC %f \n", simularity);

	fprintf(ostream, "CHANNEL %d \n", channels);

	example_search::write_config(ostream, dirname);
	return;
}


void
gabor_texture_search::region_match(RGBImage *rimg, bbox_list_t *blist)
{
	int		pass;
	gtexture_args_t	gargs;

	save_edits();

	/* get the gabor argument data structure */
	pass = gen_args(&gargs);
	if (pass == 0) {
		fprintf(stderr, "No patches of large enough size \n");
		return;
	}
	gargs.min_matches = INT_MAX;	/* get all the matches */

	/* generate list of bounding boxes */
	pass = gabor_test_image(rimg, &gargs, blist);
	/* cleanup */
	release_args(&gargs);
	return;
}

