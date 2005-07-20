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

#ifndef	_REGEX_SEARCH_H
#define	_REGEX_SEARCH_H	1

#include <gtk/gtk.h>
#include "img_search.h"

class regex_search: public img_search {
public:
	regex_search(const char *name, char *descr);
	~regex_search(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	virtual	int	handle_config(int num_conf, char **datav);
	void	close_edit_win();
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	char *	search_string;
	GtkWidget	*string_entry;
	GtkWidget	*edit_window;
};

class regex_factory: public img_factory {
public:
	regex_factory() {
		set_name("Regular expression");
		set_description("regex");
	}
	img_search *create(const char *name) {
		return new regex_search(name, "Regex");
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

#endif	/* !_REGEX_SEARCH_H */
