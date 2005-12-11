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
	decoder_map_t *	cur_map;

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
