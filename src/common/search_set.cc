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
#include "queue.h"
#include "rgb.h"
#include "image_tools.h"
#include "img_search.h"
#include "search_set.h"

search_set::search_set()
{
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


                                                                                
void
search_set::reset_search_iter(search_iter_t *iter)
{
	*iter = ss_search_list.begin();
}


img_search *
search_set::get_next_search(search_iter_t *iter)
{
	img_search  *	dsearch;

	if (*iter == ss_search_list.end()) {
		return(NULL);
	}

	dsearch = **iter;
	(*iter)++;
	return(dsearch);
}


void
search_set::reset_dep_iter(search_iter_t *iter)
{
	*iter = ss_dep_list.begin();
}


img_search *
search_set::get_next_dep(search_iter_t *iter)
{
	img_search  *	dsearch;

	if (*iter == ss_dep_list.end()) {
		return(NULL);
	}
	dsearch = **iter;
	(*iter)++;
	return(dsearch);	
}

int
search_set::get_search_count()
{
	return(ss_search_list.size());
}


