/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h> 
#include <stdlib.h> 
#include <gtk/gtk.h>
#include <sys/queue.h>
#include "lib_results.h"
#include "img_search.h"
#include "search_support.h"
#include "search_set.h"
#include "read_config.h"
#include "factory.h"



static factory_map_t * fmap = NULL;
static factory_map_t * codec_fmap = NULL;

void add_new_search_type(img_factory *fact);
void add_new_codec(img_factory *fact);


void
factory_register_codec(img_factory *factory)
{
	factory_map_t *new_map;

	new_map = (factory_map_t *)malloc(sizeof(*new_map));
	assert(new_map != NULL);

	new_map->fm_factory = factory;
	new_map->fm_next = codec_fmap;
	codec_fmap = new_map;

	add_new_codec(factory);
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
find_codec_factory(const char *name)
{
	factory_map_t *cur_map;

	//printf("**** find_codec_factory\n");

	for (cur_map = codec_fmap; cur_map != NULL;
	    cur_map = cur_map->fm_next) {
		if (strcmp(name, cur_map->fm_factory->get_description()) == 0) {
			return(cur_map->fm_factory);
		}
	}

	fprintf(stderr, "Could not find a codec factory for %s.\n", name);
	fprintf(stderr, "Is the PLUGIN_DIR configuration setting correct\n");
	fprintf(stderr, "in your snapfindrc file?\n");

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
get_first_factory(void **cookie, factory_map_t *map)
{
	factory_map_t *cur_map;

	cur_map = map;
	if (cur_map == NULL) {
		*cookie = NULL;
		return(NULL);
	} else {
		*cookie = (void *)cur_map;
		return(cur_map->fm_factory);
	}
}

img_factory *
get_first_factory(void **cookie)
{
	return get_first_factory(cookie, fmap);
}

img_factory *
get_first_codec_factory(void **cookie)
{
	return get_first_factory(cookie, codec_fmap);
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
