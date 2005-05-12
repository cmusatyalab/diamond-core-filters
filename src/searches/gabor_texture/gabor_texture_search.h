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

#ifndef	_GABOR_TEXTURE_SEARCH_H_
#define	_GABOR_TEXTURE_SEARCH_H_	1

#include <gtk/gtk.h>
#include "img_search.h"
#include "gabor_tools.h"



class gabor_texture_search: public example_search {
public:
	gabor_texture_search(const char *name, char *descr);
	~gabor_texture_search(void);

	void 	add_patch();
	void 	remove_patch();
	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	virtual void 	edit_search();
	virtual void	close_edit_win();

	/* set the simularity metric, either via string or double */	
	void 		set_simularity(char *data);
	void 		set_simularity(double sim);

	/* set number of channels via string or double */
	void 		set_channels(char *data);
	void 		set_channels(int num);

	void 		set_radius(char *data);
	void 		set_radius(int val);

	void 		set_num_angle(char *data);
	void 		set_num_angle(int val);

	void 		set_num_freq(char *data);
	void 		set_num_freq(int val);

	void 		set_min_freq(char *data);
	void 		set_min_freq(float val);

	void 		set_max_freq(char *data);
	void 		set_max_freq(float val);

	void 		set_sigma(char *data);
	void 		set_sigma(float val);


	virtual	int	handle_config(config_types_t conf_type, char *data);

	void		set_matches(char *matches);
	void		set_matches(int matches);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);



private:
	int 	gen_args(gtexture_args_t *gargs);
 	void 	release_args(gtexture_args_t *gargs);

	int			method;
	double			simularity;
	int			channels;
	int			num_angles;
	int			num_freq;
	int			radius;
	float			sigma;
	float			min_freq;
	float			max_freq;

	texture_dist_t	distance_metric;
	GtkWidget *		distance_menu;

	GtkObject *	sim_adj;
	GtkObject *	rad_adj;
	GtkObject *	nangle_adj;
	GtkObject *	nfreq_adj;
	GtkObject *	sigma_adj;
	GtkObject *	minfreq_adj;
	GtkObject *	maxfreq_adj;
	GtkWidget *	edit_window;
	GtkWidget *	gray_widget;
	GtkWidget *	rgb_widget;
};


class gabor_texture_factory: public img_factory {
public:
   gabor_texture_search *create(const char *name) {
                return new gabor_texture_search(name, "Gabor Texture");
   }
};
                                                                                

#endif	/* !_GABOR_TEXTURE_SEARCH_H_ */
