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

#ifndef	_OCV_FACE_SEARCH_H_
#define	_OCV_FACE_SEARCH_H_	1

#include <gtk/gtk.h>
#include "image_common.h"
#include "common_consts.h"
#include "texture_tools.h"
#include "histo.h"
#include "img_search.h"


class ocv_face_search: public window_search {
public:
	ocv_face_search(const char *name, char *descr);
	~ocv_face_search(void);

	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	virtual void 	edit_search();
	virtual void	close_edit_win();

	/* set the min number of faces required */
	void 		set_face_count(char *data);
	void 		set_face_count(int new_count);

	/* set the min number of faces required */
	void 		set_support(char *data);
	void 		set_support(int new_count);

	virtual	int	handle_config(int num_conf, char **datav);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	int			face_count;
	int			support_matches;

	GtkWidget *	edit_window;
	GtkObject *	count_widget;
	GtkObject *	support_widget;
};

class ocv_face_factory: public img_factory {
public:
	ocv_face_factory() {
		set_name("OCV Face");
		set_description("ocv_face_search");
	}
	img_search *create(const char *name) {
		return new ocv_face_search(name, "OCV Face");
	}
	int is_example() {
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

#endif	/* !_OCV_FACE_SEARCH_H_ */
