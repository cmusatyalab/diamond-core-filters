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

#ifndef	_IMG_DIFF_H_
#define	_IMG_DIFF_H_	1

#include <gtk/gtk.h>
#include "snapfind_consts.h"
#include "img_search.h"

/*
 * slider bar goes from 0-100.
 * 0 means images are identical (no difference)
 */
#define MAX_DISTANCE 100  
#define DEFAULT_DISTANCE 0

/*
 * The class for doing windowed searches over
 * the image.  This is a subclass of img_search.
 */
class img_diff: public img_search {
public:
	img_diff(const char *name, char *descr);
	~img_diff(void);

	virtual void 	edit_search();
	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	/* set the similarity metric, either via string or double */
	void 		set_distance(char *data);
	void 		set_distance(int sim);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);
	virtual	int 	handle_config(int num_conf, char **conf);
	void		close_edit_win();
	int		matches_filter(char *name);
	
private:
	float		distance;
	GtkObject *	sim_adj;
	GtkWidget *	edit_window;
};


class img_diff_factory: public img_factory {
public:
	img_diff_factory() {
		set_name("Image difference");
		set_description("Difference between images");
	}
	img_search *create(const char *name) {
		return new img_diff(name, "Image difference");
	}
	int 	is_example() {
		return(1);
	}
};

#endif	/* !_IMG_DIFF_H_ */
