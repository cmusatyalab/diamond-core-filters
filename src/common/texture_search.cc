
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include "queue.h"
#include "rgb.h"
//#include "histo.h"
#include "image_tools.h"
#include "texture_tools.h"
#include "img_search.h"
//#include "snapfind.h"

#define	MAX_DISPLAY_NAME	64

texture_search::texture_search(const char *name, char *descr)
	: example_search(name, descr)
{
	edit_window = NULL;
	simularity = 0.70;
	channels = 3;

}

texture_search::~texture_search()
{
	return;
}


void
texture_search::set_simularity(char *data)
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
texture_search::set_simularity(double sim)
{
	simularity = sim;
	if (simularity < 0) {
		simularity = 0.0;
	} else if (simularity > 1.0) {
		simularity = 1.0;
	}
	return;
}

void
texture_search::set_channels(int num)
{
	assert((num == 1) || (num == 3));
	channels = num;
}

void
texture_search::set_channels(char *data)
{
	int	 num;
	num = atoi(data);
	set_channels(num);
}

int
texture_search::handle_config(config_types_t conf_type, char *data)
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
                                                                               
	item = gtk_menu_item_new_with_label (name); 
	g_signal_connect (G_OBJECT (item), "activate", callback, (gpointer) data); 
	gtk_widget_show(item); 
                                                                               
	return item; 
}

static void
cb_close_edit_window(GtkWidget* item, gpointer data)
{
	texture_search *	search;

	search = (texture_search *)data;
	search->close_edit_win();
}


void
texture_search::close_edit_win()
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
texture_search::edit_search()
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

	opt = gtk_option_menu_new();
	menu = gtk_menu_new();
 
	item = make_menu_item("Difference of Gaussians",
		G_CALLBACK(cb_update_menu_select), GINT_TO_POINTER(0));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item); 

	gtk_option_menu_set_menu(GTK_OPTION_MENU (opt), menu);
	gtk_box_pack_start(GTK_BOX(container), opt, FALSE, TRUE, 0);

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
texture_search::save_edits()
{
	double	sim;
	int		color;

	/* no active edit window, so return */
	if (edit_window == NULL) {
		return;
	}

	/* get the simularity and save */
	sim = gtk_adjustment_get_value(GTK_ADJUSTMENT(sim_adj));
	set_simularity(sim);

	color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rgb_widget));
	if (color) {
		set_channels(3);
	} else {
		set_channels(1);
	}

	/* call the parent class */	
	example_search::save_edits();
}

/*
 * This write the relevant section of the filter specification file
 * for this search.
 */

void
texture_search::write_fspec(FILE *ostream)
{
	IplImage	*img;
	double		feature_vals[NUM_LAP_PYR_LEVELS *TEXTURE_MAX_CHANNELS];
	example_patch_t	*cur_patch;
	img_search *	rgb;
	int		i = 0;

	save_edits();
	/*
 	 * First we write the header section that corrspons
 	 * to the filter, the filter name, the assocaited functions.
 	 */

	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER %s \n", get_name());
	fprintf(ostream, "THRESHOLD %d \n", 1);	/* XXX should we change */
	fprintf(ostream, "EVAL_FUNCTION  f_eval_texture_detect \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_texture_detect \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_texture_detect \n");
	fprintf(ostream, "ARG  %s  # name \n", get_name());
	
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

	 fprintf(ostream, "ARG  %f  # simularity \n", simularity);
	 fprintf(ostream, "ARG  %d  # channels \n", channels);
	 fprintf(ostream, "ARG  %d  # num examples \n", num_patches);

	TAILQ_FOREACH(cur_patch, &ex_plist, link) {
		int j;
		if (channels == 1) {
			img = get_gray_ipl_image(cur_patch->patch_image);
		} else {
			img = get_rgb_ipl_image(cur_patch->patch_image);
		}

	  	texture_get_lap_pyr_features_from_subimage(img, channels, 0, 0,
				cur_patch->xsize, cur_patch->ysize, 
		   		feature_vals);

		for (j=0; j < (NUM_LAP_PYR_LEVELS*channels); j++) {
	 		fprintf(ostream, "ARG  %f  # sample %d val %d \n", 
				feature_vals[j], i, j);
		}
		i++;	/* count thenumber of samples for debugging */
	}
	 fprintf(ostream, "REQUIRES  RGB # dependencies \n");
	 fprintf(ostream, "MERIT  1000 # some relative cost \n");

    rgb = new rgb_img("RGB image", "RGB image");
    ss_add_dep(rgb);
}

void
texture_search::write_config(FILE *ostream, const char *dirname)
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
texture_search::region_match(RGBImage *rimg, bbox_list_t *blist)
{
	texture_args_t	targs;
	example_patch_t	*cur_patch;
	double		feature_vals[NUM_LAP_PYR_LEVELS *TEXTURE_MAX_CHANNELS];
	IplImage *		iimg;
	int				i;
	int				pass;
	double *		data_arr;

	save_edits();

	targs.name = strdup(get_name());
	assert(targs.name != NULL);

	targs.box_width = get_testx();
	targs.box_height = get_testy();
	targs.step = get_stride(); 
	targs.scale = get_scale();
	targs.min_matches = INT_MAX; 	/* get all bounding boxes */
	targs.max_distance = (1.0 - simularity) * NUM_LAP_PYR_LEVELS ; 
	targs.num_channels = channels;

	i = 0;
	TAILQ_FOREACH(cur_patch, &ex_plist, link) {
		i++;
	}

	targs.num_samples = i;

	targs.sample_values = (double **)malloc(sizeof(double *) * targs.num_samples);

	i = 0;
	TAILQ_FOREACH(cur_patch, &ex_plist, link) {
		int j;
		if (channels == 1) {
			iimg = get_gray_ipl_image(cur_patch->patch_image);
		} else {
			iimg = get_rgb_ipl_image(cur_patch->patch_image);
		}

	  	texture_get_lap_pyr_features_from_subimage(iimg, channels, 0, 0,
				cur_patch->xsize, cur_patch->ysize, 
		   		feature_vals);

		/* XXX free iimg */
		data_arr = (double *)malloc(sizeof(double) * NUM_LAP_PYR_LEVELS *TEXTURE_MAX_CHANNELS);
		assert(data_arr != NULL);
		for (j=0; j < (NUM_LAP_PYR_LEVELS*channels); j++) {
			data_arr[j] = feature_vals[j];
		}
		targs.sample_values[i] = data_arr;
		i++;	/* count thenumber of samples for debugging */
	}
	if (channels == 1) {
		iimg = get_gray_ipl_image(rimg);
	} else {
		iimg = get_rgb_ipl_image(rimg);
	}

	//pass = texture_test_entire_image(iimg, &targs, blist);
	pass = old_texture_test_entire_image(iimg, &targs, blist);
    

	                                                                                 
	/* XXX cleanup */

	return;
}

