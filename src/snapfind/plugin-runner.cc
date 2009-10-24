/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <cstdlib>
#include <cstdio>

#include "plugin-runner.h"
#include "factory.h"
#include "read_config.h"
#include "lib_results.h"
#include "lib_sfimage.h"

static const char THUMBNAIL_NAME[] = "THUMBNAIL";
static const char CODEC_NAME[] = "RGB";
static const char FILTER_NAME[] = "*";

static bool
sc(const char *a, const char *b) {
	return strcmp(a, b) == 0;
}

void
print_key_value(const char *key,
		const char *value) {
	printf("K %d\n%s\n", strlen(key), key);
	printf("V %d\n%s\n", strlen(value), value);
}

void
print_key_value(const char *key,
		double d) {
	char buf[G_ASCII_DTOSTR_BUF_SIZE];
	print_key_value(key, g_ascii_dtostr(buf, sizeof (buf), d));
}

void
print_key_value(const char *key,
		int i) {
	char *value = g_strdup_printf("%d", i);
	print_key_value(key, value);
	g_free(value);
}

void
print_key_value(const char *key,
		int value_len,
		void *value) {
	printf("K %d\n%s\n", strlen(key), key);
	printf("V %d\n", value_len);
	fwrite(value, value_len, 1, stdout);
	printf("\n");
}

static void
print_plugin(const char *type, img_factory *imgf) {
	print_key_value("type", type);
	print_key_value("display-name", imgf->get_name());
	print_key_value("internal-name", imgf->get_description());
	printf("\n");
}

void
list_plugins(void) {
	void *cookie;

	img_factory *imgf;

	imgf = get_first_factory(&cookie);
	if (imgf != NULL) {
		do {
			print_plugin("filter", imgf);
		} while((imgf = get_next_factory(&cookie)));
	}

	imgf = get_first_codec_factory(&cookie);
	if (imgf != NULL) {
		do {
			print_plugin("codec", imgf);
		} while((imgf = get_next_factory(&cookie)));
	}

	img_search *thumb = get_thumbnail_filter();
	if (thumb != NULL) {
		print_key_value("type", "thumbnail");
		print_key_value("display-name", "thumbnail");
		print_key_value("internal-name", "thumbnail");
	}
}

static img_search
*get_plugin(const char *type,
	    const char *internal_name) {
	img_factory *imgf = NULL;
	img_search *search = NULL;

	const char *name;

	if (sc(type, "filter")) {
		imgf = find_factory(internal_name);
		name = FILTER_NAME;
	} else if (sc(type, "codec")) {
		imgf = find_codec_factory(internal_name);
		name = CODEC_NAME;
	} else if (sc(type, "thumbnail")) {
		search = get_thumbnail_filter();
		name = THUMBNAIL_NAME;
	} else {
		printf("Invalid type\n");
		return NULL;
	}

	if (imgf) {
		search = imgf->create(name);
	}

	if (!imgf && !search) {
		return NULL;
	}

	if (imgf) {
		search->set_searchlet_lib_path(imgf->get_searchlet_lib_path());
	}

	search->set_plugin_runner_mode(true);
	return search;
}

static void
print_search_config(img_search *search) {
	// editable?
	print_key_value("is-editable", search->is_editable() ? "true" : "false");

	// print blob
	print_key_value("blob", search->get_auxiliary_data_length(),
			search->get_auxiliary_data());

	// print config (also prints patches)
	if (search->is_editable()) {
		char *config;
		size_t config_size;
		FILE *memfile = open_memstream(&config, &config_size);
		search->write_config(memfile, NULL);
		fclose(memfile);
		print_key_value("config", config_size, config);
		free(config);
	}

	// print fspec
	char *fspec;
	size_t fspec_size;
	FILE *memfile = open_memstream(&fspec, &fspec_size);
	search->write_fspec(memfile);
	fclose(memfile);
	print_key_value("fspec", fspec_size, fspec);
	free(fspec);

	// print diamond files
	print_key_value("searchlet-lib-path", search->get_searchlet_lib_path());
}

struct len_data {
	int len;
	void *data;
};

static int
expect_token_get_size(char token) {
	char *line = NULL;
	size_t n;
	int result = -1;
	int c;

	// expect token
	c = getchar();
	//  g_debug("%c", c);
	if (c != token) {
		goto OUT;
	}
	c = getchar();
	//  g_debug("%c", c);
	if (c != ' ') {
		goto OUT;
	}

	// read size
	if (getline(&line, &n, stdin) == -1) {
		goto OUT;
	}
	result = atoi(line);
	//  g_debug("size: %d", result);

 OUT:
	free(line);
	return result;
}

static void
populate_search(img_search *search, GHashTable *user_config) {
	struct len_data *ld;

	// config
	ld = (struct len_data *) g_hash_table_lookup(user_config, "config");
	if (ld) {
		read_search_config_for_plugin_runner(ld->data, ld->len, search);
	}

	// patches
	ld = (struct len_data *) g_hash_table_lookup(user_config, "patch-count");
	if (ld) {
		char *value = (char *) g_malloc(ld->len + 1);
		value[ld->len] = '\0';
		memcpy(value, ld->data, ld->len);
		int patch_count = atoi(value);
		//fprintf(stderr, "patch_count: %d\n", patch_count);
		g_free(value);

		for (int i = 0; i < patch_count; i++) {
			char *key = g_strdup_printf("patch-%d", i);
			//fprintf(stderr, "looking up %s\n", key);
			ld = (struct len_data *) g_hash_table_lookup(user_config, key);
			g_free(key);
			if (ld) {
				RGBImage *img = read_rgb_image((unsigned char *) ld->data,
							       ld->len);
				if (img) {
					bbox_t bbox;
					bbox.distance = 0;
					bbox.min_x = 0;
					bbox.min_y = 0;
					bbox.max_x = img->width;
					bbox.max_y = img->height;

					//fprintf(stderr, "adding patch\n");
					search->add_patch(img, bbox);
				}
				free(img);
			}
		}
	}
}

static void
destroy_len_data(gpointer data) {
	struct len_data *ld = (struct len_data *) data;
	g_free(ld->data);
	g_free(data);
}

static GHashTable *
read_key_value_pairs() {
	GHashTable *ht = g_hash_table_new_full(g_str_hash, g_str_equal,
					       g_free, destroy_len_data);

	while(true) {
		// read key size
		int keysize = expect_token_get_size('K');
		if (keysize == -1) {
			break;
		}

		// read key + \n
		char *key = (char *) g_malloc(keysize + 1);
		if (keysize > 0) {
			if (fread(key, keysize + 1, 1, stdin) != 1) {
				g_free(key);
				break;
			}
		}
		key[keysize] = '\0';  // key is a string

		// read value size
		int valuesize = expect_token_get_size('V');
		if (valuesize == -1) {
			g_free(key);
		}

		// read value + \n
		void *value = g_malloc(valuesize);
		if (valuesize > 0) {
			if (fread(value, valuesize, 1, stdin) != 1) {
				g_free(key);
				g_free(value);
				break;
			}
		}
		getchar();           // value is not null terminated

		// add entry
		//fprintf(stderr, "key: %s, valuesize: %d\n", key, valuesize);
		struct len_data *ld = g_new(struct len_data, 1);
		ld->len = valuesize;
		ld->data = value;
		g_hash_table_insert(ht, key, ld);
	}

	return ht;
}

int
get_plugin_initial_config(const char *type,
			  const char *internal_name) {
	img_search *search = get_plugin(type, internal_name);
	if (search == NULL) {
		printf("Can't find %s\n", internal_name);
		return 1;
	}

	print_search_config(search);

	return 0;
}

int
edit_plugin_config(const char *type,
		   const char *internal_name) {
	img_search *search = get_plugin(type, internal_name);
	if (search == NULL) {
		printf("Can't find %s\n", internal_name);
		return 1;
	}

	GHashTable *user_config = read_key_value_pairs();
	populate_search(search, user_config);
	g_hash_table_unref(user_config);

	if (search->is_editable()) {
		search->edit_search();
		gtk_main();
	}

	if (sc(type, "filter")) {
		search->set_name(FILTER_NAME);
	} else if (sc(type, "codec")) {
		search->set_name(CODEC_NAME);
	} else if (sc(type, "thumbnail")) {
		search->set_name(THUMBNAIL_NAME);
	}

	print_search_config(search);

	return 0;
}

static void
print_bounding_boxes(bbox_list_t *bblist) {
	bbox_t *cur_bb;

	TAILQ_FOREACH(cur_bb, bblist, link) {
	  print_key_value("min-x", cur_bb->min_x);
	  print_key_value("min-y", cur_bb->min_y);
	  print_key_value("max-x", cur_bb->max_x);
	  print_key_value("max-y", cur_bb->max_y);
	  print_key_value("distance", cur_bb->distance);
	  printf("\n");
	}
}

int
run_plugin(const char *type,
	   const char *internal_name) {
	img_search *search = get_plugin(type, internal_name);
	if (search == NULL) {
		printf("Can't find %s\n", internal_name);
		return 1;
	}

	GHashTable *user_config = read_key_value_pairs();
	populate_search(search, user_config);

	// get the image to process
	struct len_data *ld
	  = (struct len_data *) g_hash_table_lookup(user_config, "target-image");
	if (!ld) {
		g_hash_table_unref(user_config);
		printf("target-image not specified\n");
		return 1;
	}

	// convert to RGBImage
	RGBImage *img = read_rgb_image((unsigned char *) ld->data, ld->len);

	// destroy the hash table now
	g_hash_table_unref(user_config);

	if (!img) {
		printf("Can't read target-image\n");
		return 1;
	}

	// run plugin
	bbox_list_t bblist;
	TAILQ_INIT(&bblist);
	search->region_match(img, &bblist);
	print_bounding_boxes(&bblist);

	return 0;
}
