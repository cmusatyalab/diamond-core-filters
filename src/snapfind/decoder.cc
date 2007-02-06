/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2005-2006 Larry Huston <larry@thehustons.net>
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
#include "queue.h"
#include "lib_results.h"
#include "img_search.h"
#include "search_support.h"
#include "search_set.h"
#include "read_config.h"
#include "attr_decode.h"
#include "decoder.h"


typedef struct decode_hint {
	char *		dh_name;
	char *  	dh_type;
	attr_decode *	dh_decode;
	struct decode_hint * dh_next;
} decode_hint_t;


static decode_hint_t * dh_list = NULL;
static decode_hint_t ** dh_last = &dh_list;

static decoder_map_t * dmap = NULL;
static decoder_map_t ** dtail = &dmap;
static int  num_map = 0;

void
decoders_init()
{
	attr_decode *new_decode;

	new_decode = new text_decode();
	decoder_register(new_decode);

	new_decode = new hex_decode();
	decoder_register(new_decode);

	new_decode = new int_decode();
	decoder_register(new_decode);

	new_decode = new time_decode();
	decoder_register(new_decode);
	
	new_decode = new rgb_decode();
	decoder_register(new_decode);

	new_decode = new patches_decode();
	decoder_register(new_decode);

}

void
decoder_register(attr_decode *decoder)
{

	decoder_map_t *new_map;

	new_map = (decoder_map_t *)malloc(sizeof(*new_map));
	assert(new_map != NULL);

	new_map->dm_decoder = decoder;

	/* XXX find dups ?? */

	new_map->dm_next = NULL;
	*dtail = new_map;
	dtail = &new_map->dm_next;

	decoder->set_type(num_map);
	num_map++;
}

decode_hint_t *
lookup_decode_hint(const char *aname)
{
	decode_hint_t *	cur_hint;

	for (cur_hint = dh_list; cur_hint != NULL; 
	    cur_hint = cur_hint->dh_next) {
		if (strcmp(cur_hint->dh_name, aname) == 0) {
			return(cur_hint);
		}
	}
	return(NULL);
}

decode_hint_t *
new_decode_hint(const char *aname, const char *dname)
{
	decode_hint_t *	new_hint;

	new_hint = (decode_hint_t *)malloc(sizeof(*new_hint));
	assert(new_hint != NULL);

	new_hint->dh_name = strdup(aname);
	assert(new_hint->dh_name != NULL);

	new_hint->dh_type = strdup(dname);
	assert(new_hint->dh_type != NULL);

	new_hint->dh_decode = NULL;

	/* put on tail of the linked list */
	new_hint->dh_next = NULL;
	*dh_last = new_hint;
	dh_last = &new_hint->dh_next;

	return(new_hint);
}



/* Find a decoder with the specified name */
attr_decode *
find_decode(const char *name)
{
	decoder_map_t *cur_map;

	for (cur_map = dmap; cur_map != NULL; cur_map = cur_map->dm_next) {
		if (strcmp(name, cur_map->dm_decoder->get_name()) == 0) {
			return(cur_map->dm_decoder);
		}
	}
	return(NULL);
}


attr_decode *
get_first_decoder(void **cookie)
{
	decoder_map_t *cur_map;

	cur_map = dmap;
	*cookie = (void *)dmap;
	return(dmap->dm_decoder);
}

attr_decode *
get_next_decoder(void **cookie)
{
	decoder_map_t *cur_map = (decoder_map_t *)*cookie;

	if (cur_map == NULL) {
		*cookie = NULL;
		return(NULL);
	} else {
		cur_map = cur_map->dm_next;
		*cookie = (void *)cur_map;
		if (cur_map == NULL) {
			return(NULL);
		} else {
			return(cur_map->dm_decoder);
		}
	}
}

GtkWidget *
get_decoder_menu(void)
{
        GtkWidget *     menu;
        GtkWidget *     item;
	decoder_map_t *	cur_map;

        menu = gtk_menu_new();

	for (cur_map = dmap; cur_map != NULL; cur_map = cur_map->dm_next) {
        	item = gtk_menu_item_new_with_label(cur_map->dm_decoder->get_name());
        	g_object_set_data(G_OBJECT(item), "user data",
                	(void *)cur_map->dm_decoder);
        	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	}

        return(menu);
}



attr_decode *
guess_decoder(const char *name, unsigned char *data, size_t dlen)
{
	int	val;
	int	best = -1;
	attr_decode *best_decode = NULL;
	attr_decode *decode;
	decoder_map_t *	cur_map;
	decode_hint_t *	hint;

	hint = lookup_decode_hint(name);
	if (hint) {
		if (hint->dh_decode) {
			return(hint->dh_decode);
		}
		decode = find_decode(hint->dh_type);
		if (decode) {
			hint->dh_decode = decode;
			return(decode);
		}
	}

	for (cur_map = dmap; cur_map != NULL; cur_map = cur_map->dm_next) {
		val = cur_map->dm_decoder->is_type(data, dlen);
		if (val > best) {
			best_decode = cur_map->dm_decoder;
			best = val;
		}
	}
	assert(best_decode != NULL);
	return(best_decode);
}


void
decode_update_hint(const char *aname, attr_decode *decode)
{
	decode_hint_t *	hint;

	hint = lookup_decode_hint(aname);

	if (hint != NULL) {
		free(hint->dh_type);
		hint->dh_type = strdup(decode->get_name());
		assert(hint->dh_type != NULL);
		hint->dh_decode = decode;
	} else {
		hint = new_decode_hint(aname, decode->get_name());
		hint->dh_decode = decode;
	}
	
}

void
add_decode_hint(const char *aname, const char *dname)
{
	decode_hint_t *new_decode;
	new_decode = new_decode_hint(aname, dname);
}

void
write_decodes(FILE *ostream)
{
	decode_hint_t *	cur_hint;
	fprintf(ostream, "# mapping between attribute names and decoders\n"); 
	

	for (cur_hint = dh_list; cur_hint != NULL; 
	    cur_hint = cur_hint->dh_next) {
		if (cur_hint->dh_decode == NULL) {
			cur_hint->dh_decode = find_decode(cur_hint->dh_type);
			if (cur_hint->dh_decode == NULL) {
				continue;
			}
		}
		fprintf(ostream, " \"%s\" = \"%s\" \n",
			cur_hint->dh_name,
			cur_hint->dh_decode->get_name());
	}
}
