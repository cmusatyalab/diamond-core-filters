#ifndef	_SEARCH_SET_H_
#define	_SEARCH_SET_H_	1

#include <iostream>
#include <list>
//#include <iterator>
#include "rgb.h"
#include "lib_searchlet.h"
#include "img_search.h"

/* XXX */
using namespace	std;
using namespace	__gnu_cxx;


typedef  list<img_search *>::iterator	search_iter_t;

class search_set {
public:
	search_set();
	virtual 	~search_set();

	void		add_search(img_search *new_search);
	void		remove_search(img_search *old_search);

	void		reset_search_iter(search_iter_t *iter);
	img_search * 	get_next_search(search_iter_t *iter);

	void		add_dep(img_search *dep_search);
	void		clear_deps();

	void 		reset_dep_iter(search_iter_t *iter);
	img_search * 	get_next_dep(search_iter_t *iter);

	int		get_search_count();

	char * 	build_filter_spec(char *tmp_file);

	GtkWidget * search_set::build_edit_table();


private:
	list<img_search *>		ss_search_list;
	list<img_search *>		ss_dep_list;
};




#endif	/* !_SEARCH_SET_H_ */
