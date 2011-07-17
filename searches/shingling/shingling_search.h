/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_SHINGLING_SEARCH_H_
#define	_SHINGLING_SEARCH_H_	1

#include <gtk/gtk.h>
#include "img_search.h"

class shingling_search: public img_search {
public:
	shingling_search(const char *name, const char *descr);
	~shingling_search(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	virtual	int	handle_config(int num_conf, char **datav);
	void	close_edit_win();
	bool	is_editable(void);

private:
	char *		search_string;
	double		similarity;
	int		shingle_size;	
	GtkWidget	*edit_window;
	GtkWidget	*string_entry;
	GtkWidget	*shingle_size_cb;
	GtkObject	*sim_adj;
};


class shingling_factory: public img_factory {
public:
	shingling_factory() {
		set_name("W-Shingling");
		set_description("shingling");
	}
	img_search *create(const char *name) {
		return new shingling_search(name, "W-Shingling");
	}
	int is_example() {
		return(0);
	}
};

#endif	/* !_SHINGLING_SEARCH_H_ */

