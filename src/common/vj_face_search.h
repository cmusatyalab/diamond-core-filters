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

#ifndef	_VJ_FACE_SEARCH_H_
#define	_VJ_FACE_SEARCH_H_	1

#include <gtk/gtk.h>
#include "image_common.h"
#include "common_consts.h"
#include "texture_tools.h"
#include "histo.h"
#include "img_search.h"


class vj_face_search: public window_search {
public:
	vj_face_search(const char *name, char *descr);
	~vj_face_search(void);

	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	virtual void 	edit_search();
	virtual void	close_edit_win();

	/* set the min number of faces required */
	void 		set_face_count(char *data);
	void 		set_face_count(int new_count);

	/* set starting detector level */
	void 		set_start_level(char *data);
	void 		set_start_level(int slevel);

	/* set ending detector level */
	void 		set_end_level(char *data);
	void 		set_end_level(int elevel);

	void 		update_toggle();


	virtual	int	handle_config(int num_conf, char **datav);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	int			face_count;
	int			start_stage;
	int			end_stage;
	int			do_merge;
	float		overlap_val;

	GtkWidget *	edit_window;
	GtkObject *	count_widget;
	GtkObject *	start_widget;
	GtkObject *	end_widget;
	GtkWidget *	face_merge;
	GtkObject *	merge_overlap;
	GtkWidget *	overlap_widget;
};

class vj_face_factory: public img_factory {
public:
	vj_face_factory() {
		set_name("VJ Face");
		set_description("vj_face_search");
	}
	img_search *create(const char *name) {
		return new vj_face_search(name, "VJ Face");
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

#endif	/* !_VJ_FACE_SEARCH_H_ */
