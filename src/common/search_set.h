#ifndef	_SEARCH_SET_H_
#define	_SEARCH_SET_H_	1

#include <iostream>
#include <list>
#include <iterator>
#include "rgb.h"
#include "lib_searchlet.h"
#include "img_search.h"

/* XXX */
using namespace	std;


class search_set {
public:
	search_set();
	virtual 	~search_set();

	void		add_search(img_search *new_search);
	void		remove_search(img_search *old_search);

	img_search * get_first_search();
	img_search * get_next_search();

	void		add_dep(img_search *dep_search);
	void		clear_deps();

	img_search * get_first_dep();
	img_search * get_next_dep();


	int			get_search_count();

private:
	list<img_search *>				ss_search_list;
	list<img_search *>::iterator	ss_cur_search;

	list<img_search *>				ss_dep_list;
	list<img_search *>::iterator	ss_cur_dep;
};



#endif	/* !_SEARCH_SET_H_ */
