/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_NULL_H_
#define	_NULL_H_	1

#include <gtk/gtk.h>
#include "img_search.h"
#include "snapfind_consts.h"


class null_codec: public img_search {
public:
	null_codec(const char *name, const char *descr);
	~null_codec(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	bool	is_editable(void);
	virtual	int	handle_config(int num_conf, char **datav);
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
};



class null_codec_factory: public img_factory {
public:
	null_codec_factory() {
		set_name("Null codec");
		set_description("null_codec");
	}
	img_search *create(const char *name) {
		return new null_codec(name, "Null codec");
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

#endif	/* !_NULL_H_ */
