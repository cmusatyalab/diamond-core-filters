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
typedef  void (*sset_notify_fn)(search_set *set);
typedef  vector<sset_notify_fn>::iterator	cb_iter_t;

class search_set {
public:
	search_set();
	virtual 	~search_set();

	void		add_search(img_search *new_search);
	void		remove_search(img_search *old_search);

	void		reset_search_iter(search_iter_t *iter);
	img_search *get_next_search(search_iter_t *iter);


	/* calls to add new dependency or reset the list */
	void		add_dep(img_search *dep_search);
	void		clear_deps();


	/* get pointers to the current iteration */
	/* XXX this can probably be done better.  */
	void 		reset_dep_iter(search_iter_t *iter);
	img_search *get_next_dep(search_iter_t *iter);

	/* get the number of searches in the set */
	int		get_search_count();


	/* calls to register/unregister callback function */
	void		register_update_fn(sset_notify_fn cb_fn);
	void		un_register_update_fn(sset_notify_fn cb_fn);

	/* method that tells all the callbacks when set is modified */
	void		notify_update();

	char * 		build_filter_spec(char *tmp_file);

	GtkWidget * search_set::build_edit_table();


private:
	list<img_search *>		ss_search_list;
	list<img_search *>		ss_dep_list;
	vector<sset_notify_fn>	ss_cb_vector;
};




#endif	/* !_SEARCH_SET_H_ */
