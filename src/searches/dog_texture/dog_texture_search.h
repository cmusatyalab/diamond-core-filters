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

#ifndef	_DOG_TEXTURE_SEARCH_H_
#define	_DOG_TEXTURE_SEARCH_H_	1

#include <gtk/gtk.h>
#include "snapfind_consts.h"
#include "texture_tools.h"
#include "img_search.h"

class texture_search: public example_search {
public:
	texture_search(const char *name, char *descr);
	~texture_search(void);

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

	virtual	int	handle_config(int num_conf, char **datav);

	void		set_matches(char *matches);
	void		set_matches(int matches);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	int					method;
	double				simularity;
	int					channels;
	texture_dist_t		distance_metric;
	GtkWidget *			distance_menu;

	GtkObject *	sim_adj;
	GtkWidget *	edit_window;
	GtkWidget *	gray_widget;
	GtkWidget *	rgb_widget;
};

class texture_factory: public img_factory {
public:
	texture_factory() {
		set_name("DOG Texture");
		set_description("texture");
	}
	img_search *create(const char *name) {
		return new texture_search(name, "DOG Texture");
	}
	int 	is_example() {
		return(1);
	}
};



#ifdef __cplusplus
extern "C"
{
#endif

/* this needs to be provide by someone calling this library */
void ss_add_dep(img_search *dep);

#ifdef __cplusplus
}
#endif

#endif	/* !_DOG_TEXTURE_SEARCH_H_ */
