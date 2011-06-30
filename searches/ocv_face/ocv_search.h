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

#ifndef	_OCV_SEARCH_H_
#define	_OCV_SEARCH_H_	1

#include <gtk/gtk.h>
#include "snapfind_consts.h"
#include "img_search.h"


class ocv_search: public window_search {
public:
	ocv_search(const char *name, const char *descr);
	virtual ~ocv_search();

	void	save_edits();
	void 	write_fspec(FILE* stream);

	virtual void 	edit_search();
	virtual void	close_edit_win();

	/* min number of objects required */
	double          get_min_count();
	void 		set_min_count(char *data);
	void 		set_min_count(double new_count);

	/* max number of objects required */
	double          get_max_count();
	void 		set_max_count(char *data);
	void 		set_max_count(double new_count);

	/* min number of faces required */
	int             get_support();
	void 		set_support(char *data);
	void 		set_support(int new_count);

	void            set_classifier(const char *filename);

	virtual	int	handle_config(int num_conf, char **datav);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	double			min_count;
	double			max_count;
	int			support_matches;

	GtkWidget *	edit_window;
	GtkObject *	min_count_widget;
	GtkObject *	min_count_checkbox;
	GtkObject *	max_count_widget;
	GtkObject *	max_count_checkbox;
	GtkObject *	support_widget;

	char *          cascade_file_name;
};

class ocv_factory: public img_factory {
public:
        ocv_factory() {};
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

#endif	/* !_OCV_SEARCH_H_ */
