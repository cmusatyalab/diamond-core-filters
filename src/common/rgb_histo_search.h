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

#ifndef	_RGB_HISTO_SEARCH_H_
#define	_RGB_HISTO_SEARCH_H_	1

#include <gtk/gtk.h>
#include "common_consts.h"
#include "texture_tools.h"
#include "histo.h"
#include "img_search.h"


/*
 * The class for doing windowed searches over
 * the image.  This is a subclass of img_search.
 */
class rgb_histo_search: public example_search {
public:
	rgb_histo_search(const char *name, char *descr);
	~rgb_histo_search(void);

	void 	add_patch();
	void 	remove_patch();
	virtual void 	edit_search();
	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	/* set the simularity metric, either via string or double */
	void 		set_simularity(char *data);
	void 		set_simularity(double sim);

	virtual	int 	handle_config(int num_conf, char **conf);
	void rgb_write_state(void);
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

	void 		set_bins(int new_bins);
	void		close_edit_win();

private:
	int			bins;
	int			metric;
	histo_type_t		htype;
	GtkObject *	bin_adj;
	double		simularity;
	GtkObject *	sim_adj;
	GtkWidget *	edit_window;
	GtkWidget *	interpolated_box;
};


class rgb_histo_factory: public img_factory {
public:
	rgb_histo_factory() {
		set_name("RGB Histogram");
		set_description("rgb_histogram");
	}
	img_search *create(const char *name) {
		return new rgb_histo_search(name, "RGB Histogram");
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

#endif	/* !_RGB_HISTO_SEARCH_H_ */
