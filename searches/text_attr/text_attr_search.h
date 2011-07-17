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

#ifndef	_TEXT_ATTR_SEARCH_H_
#define	_TEXT_ATTR_SEARCH_H_	1

#include <gtk/gtk.h>
#include "img_search.h"

class text_attr_search: public img_search {
public:
	text_attr_search(const char *name, const char *descr);
	~text_attr_search(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	virtual	int	handle_config(int num_conf, char **datav);
	void	close_edit_win();
	bool	is_editable(void);

private:
	char *		attr_name;
	char *		search_string;
	int		drop_missing;	
	int		exact_match;	
	GtkWidget	*edit_window;
	GtkWidget	*string_entry;
	GtkWidget	*attr_entry;
	GtkWidget	*exact_cb;
	GtkWidget	*drop_cb;
};


class text_attr_factory: public img_factory {
public:
	text_attr_factory() {
		set_name("Text Attributes");
		set_description("text_attr");
	}
	img_search *create(const char *name) {
		return new text_attr_search(name, "Text Attributes");
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

#endif	/* !_TEXT_ATTR_SEARCH_H_ */
