/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	_GABOR_TEXTURE_SEARCH_H_
#define	_GABOR_TEXTURE_SEARCH_H_	1

#include <gtk/gtk.h>
#include "img_search.h"



class gabor_texture_search: public example_search {
public:
	gabor_texture_search(const char *name, char *descr);
	~gabor_texture_search(void);

	void 	add_patch();
	void 	remove_patch();
	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	virtual void 	edit_search();
	virtual void	close_edit_win();

	/* set the simularity metric, either via string or double */	
	void 		set_simularity(char *data);
	void 		set_simularity(double sim);

	/* set number of channels via string or double */
	void 		set_channels(char *data);
	void 		set_channels(int num);

	virtual	int	handle_config(config_types_t conf_type, char *data);

	void		set_matches(char *matches);
	void		set_matches(int matches);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	int				method;
	double			simularity;
	int				channels;
	texture_dist_t	distance_metric;
	GtkWidget *		distance_menu;

	GtkObject *	sim_adj;
	GtkWidget *	edit_window;
	GtkWidget *	gray_widget;
	GtkWidget *	rgb_widget;
};

#endif	/* !_GABOR_TEXTURE_SEARCH_H_ */
