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

static void
print_key_value(const char *key,
		const char *value) {
	printf("K %d\n%s\n", strlen(key), key);
	printf("V %d\n%s\n", strlen(value), value);
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
}
