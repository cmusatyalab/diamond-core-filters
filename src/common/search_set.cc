#include <pthread.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <assert.h>
#include "map_display.h"
#include "queue.h"
#include "rgb.h"
#include "image_tools.h"
#include "event.h"
#include "event_search.h"
#include "img_search.h"
#include "search_set.h"

search_set::search_set()
{
	ss_cur_search = ss_search_list.end();
	ss_cur_dep = ss_dep_list.end();
	return;
}

search_set::~search_set()
{
	/* XXX free any images */
	return;
}

void
search_set::add_search(img_search *new_search)
{

	ss_search_list.push_back(new_search);

}


void
search_set::remove_search(img_search *old_search)
{
	/* XXX */

}

void
search_set::add_dep(img_search *dep_search)
{
	ss_dep_list.push_back(dep_search);
}


void
search_set::clear_deps()
{
	/* XXX */

}


                                                                                
img_search *
search_set::get_first_search()
{
	img_search *	fsearch;

	fsearch = ss_search_list.front();
	ss_cur_search = ss_search_list.begin();

	return(fsearch);
}


img_search *
search_set::get_next_search()
{
	return(NULL);	/* XXX */
}


img_search *
search_set::get_first_dep()
{
	img_search  *	dsearch;

	dsearch = ss_dep_list.front();
	ss_cur_dep = ss_dep_list.begin();

	return(dsearch);
}



img_search *
search_set::get_next_dep()
{

	return(NULL);	/* XXX */

}

int
search_set::get_search_count()
{

	return(ss_search_list.size());
}




