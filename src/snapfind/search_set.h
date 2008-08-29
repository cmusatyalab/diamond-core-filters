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

#ifndef	_SEARCH_SET_H_
#define	_SEARCH_SET_H_	1

#include <iostream>
#include <list>
#include <vector>
//#include <iterator>
#include "rgb.h"
#include "lib_searchlet.h"
#include "img_search.h"

using namespace	std;


/* forward declaration */
class search_set;

typedef  list<img_search *>::iterator	search_iter_t;

/* function proto type for a callback when the set changes */
typedef  void (*sset_notify_fn)(search_set *set
                               );
typedef  vector<sset_notify_fn>::iterator	cb_iter_t;

class search_set
{
public:
	search_set();
	virtual 	~search_set();

	void		add_search(img_search *new_search);
	void		remove_search(img_search *old_search);

	void		reset_search_iter(search_iter_t *iter);
	img_search *get_next_search(search_iter_t *iter);

	img_search     *find_search(char *name, search_iter_t *iter);

	/* get the number of searches in the set */
	int		get_search_count();


	/* calls to register/unregister callback function */
	void		register_update_fn(sset_notify_fn cb_fn);
	void		un_register_update_fn(sset_notify_fn cb_fn);

	/* method that tells all the callbacks when set is modified */
	void		notify_update();

	char * 		build_filter_spec(char *tmp_file);
	void		write_blobs(ls_search_handle_t shandle);

	GtkWidget *     build_edit_table();

private:
	list<img_search *>		ss_search_list;
	vector<sset_notify_fn>	ss_cb_vector;
};




#endif	/* !_SEARCH_SET_H_ */
