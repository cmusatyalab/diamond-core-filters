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

#ifndef	_MATCH_3D_H_
#define	_MATCH_3D_H_	1

#include <gtk/gtk.h>
#include "snapfind_consts.h"
#include "img_search.h"

#define MAX_DISTANCE 10  /* for slider bar */
#define DEFAULT_DISTANCE 2

/*
 * The class for doing windowed searches over
 * the image.  This is a subclass of img_search.
 */
class match_3D: public img_search {
public:
	match_3D(const char *name, char *descr);
	~match_3D(void);

	virtual void 	edit_search();
	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	/* set the simularity metric, either via string or double */
	void 		set_distance(char *data);
	void 		set_distance(double sim);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);
	virtual	int 	handle_config(int num_conf, char **conf);
	void		close_edit_win();
	void            getQueryAttr(char *name, int size, float *q);
	
private:
	double		distance;
	GtkObject *	sim_adj;
	GtkWidget *	edit_window;
	GtkWidget *	interpolated_box;
};


class match_3D_factory: public img_factory {
public:
	match_3D_factory() {
		set_name("3D match");
		set_description("3D object matching");
	}
	img_search *create(const char *name) {
		return new match_3D(name, "3D match");
	}
	int 	is_example() {
		return(1);
	}
};

#endif	/* !_MATCH_3D_H_ */
