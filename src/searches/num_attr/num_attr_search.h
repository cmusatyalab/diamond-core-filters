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

#ifndef	_NUM_ATTR_SEARCH_H
#define	_NUM_ATTR_SEARCH_H	1

#include <gtk/gtk.h>
#include "img_search.h"

class num_attr_search: public img_search {
public:
	num_attr_search(const char *name, char *descr);
	~num_attr_search(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	virtual	int	handle_config(int num_conf, char **datav);
	void	close_edit_win();
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	char *		attr_name;
	double		min_value;	
	double		max_value;	
	int		drop_missing;
	GtkWidget	*attr_entry;
	GtkWidget	*min_spinner;
	GtkWidget	*max_spinner;
	GtkWidget	*edit_window;
	GtkWidget	*dropcb;
};

class num_attr_factory: public img_factory {
public:
	num_attr_factory() {
		set_name("Numeric Attribute");
		set_description("num_attr");
	}
	img_search *create(const char *name) {
		return new num_attr_search(name, "Numeric Attribute");
	}
	int is_example() {
		return(0);
	}
};


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif	/* !_NUM_ATTR_SEARCH_H */
