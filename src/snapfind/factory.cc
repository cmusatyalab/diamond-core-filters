#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h> 
#include <stdlib.h> 
#include <gtk/gtk.h>
#include "queue.h"
#include "lib_results.h"
#include "fil_histo.h"
#include "img_search.h"
#include "search_support.h"
#include "search_set.h"
#include "read_config.h"
#include "factory.h"



static factory_map_t * fmap = NULL;
static factory_map_t * support_fmap = NULL;

void add_new_search_type(img_factory *fact);


void
factory_register_support(img_factory *factory)
{
	factory_map_t *new_map;

	new_map = (factory_map_t *)malloc(sizeof(*new_map));
	assert(new_map != NULL);

	new_map->fm_factory = factory;
	new_map->fm_next = support_fmap;
	support_fmap = new_map;
}


void
factory_register(img_factory *factory)
{

	factory_map_t *new_map;

	new_map = (factory_map_t *)malloc(sizeof(*new_map));
	assert(new_map != NULL);

	new_map->fm_factory = factory;

	/* XXX find dups ?? */

	new_map->fm_next = fmap;
	fmap = new_map;

	add_new_search_type(factory);
}

img_factory *
find_support_factory(const char *name)
{
	factory_map_t *cur_map;

	for (cur_map = support_fmap; cur_map != NULL; cur_map = cur_map->fm_next) {

		if (strcmp(name, cur_map->fm_factory->get_description()) == 0) {
			return(cur_map->fm_factory);
		}
	}
	return(NULL);
}

img_factory *
find_factory(const char *name)
{
	factory_map_t *cur_map;

	for (cur_map = fmap; cur_map != NULL; cur_map = cur_map->fm_next) {
		if (strcmp(name, cur_map->fm_factory->get_description()) == 0) {
			return(cur_map->fm_factory);
		}
	}
	return(NULL);
}


img_factory *
get_first_factory(void **cookie)
{
	factory_map_t *cur_map;

	cur_map = fmap;
	*cookie = (void *)cur_map;
	return(cur_map->fm_factory);
}

img_factory *
get_next_factory(void **cookie)
{
	factory_map_t *cur_map = (factory_map_t *)*cookie;

	if (cur_map == NULL) {
		*cookie = NULL;
		return(NULL);
	} else {
		cur_map = cur_map->fm_next;
		*cookie = (void *)cur_map;
		if (cur_map == NULL) {
			return(NULL);
		} else {
			return(cur_map->fm_factory);
		}
	}
}
