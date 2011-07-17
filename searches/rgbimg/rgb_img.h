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

#ifndef	_RGB_IMG_H_
#define	_RGB_IMG_H_	1

#include <gtk/gtk.h>
#include "img_search.h"
#include "snapfind_consts.h"


class rgb_img: public img_search {
public:
	rgb_img(const char *name, const char *descr);
	~rgb_img(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	bool	is_editable(void);
	virtual	int	handle_config(int num_conf, char **datav);

private:
};



class rgb_img_factory: public img_factory {
public:
	rgb_img_factory() {
		set_name("Built-in");
		set_description("rgb_image");
	}
	img_search *create(const char *name) {
		return new rgb_img(name, "Built-in");
	}
	int 	is_example() {
		return(0);
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

#endif	/* !_RGB_IMG_H_ */
