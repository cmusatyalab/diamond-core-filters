/*
 *  Diamond Core Filters - collected filters for the Diamond platform
 *
 *  Copyright (c) 2011 Carnegie Mellon University
 *  All rights reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <archive.h>
#include <archive_entry.h>

#include "lib_filimage.h"

#define EXAMPLE_DIR "examples"

void load_examples(const void *data, size_t len, example_list_t *examples)
{
	example_patch_t *patch;
	struct archive *arch;
	struct archive_entry *ent;
	const char *path;
	FILE *fp;
	char *buf;
	size_t size;
	char block[4096];
	ssize_t count;
	int ret;

	assert(data != NULL);
	arch = archive_read_new();
	assert(arch != NULL);
	if (archive_read_support_format_zip(arch))
		abort();
	if (archive_read_open_memory(arch, (void *) data, len))
		abort();
	while (!(ret = archive_read_next_header(arch, &ent))) {
		if (!S_ISREG(archive_entry_filetype(ent)))
			continue;
		path = archive_entry_pathname(ent);
		if (strncmp(path, EXAMPLE_DIR "/", strlen(EXAMPLE_DIR) + 1))
			continue;
		/* archive_entry_size() is not reliable for Zip files */
		fp = open_memstream(&buf, &size);
		if (fp == NULL)
			abort();
		while ((count = archive_read_data(arch, block,
					sizeof(block))) > 0)
			if (fwrite(block, 1, count, fp) != (size_t) count)
				abort();
		if (count < 0) {
			fprintf(stderr, "%s\n", archive_error_string(arch));
			abort();
		}
		fclose(fp);
		patch = malloc(sizeof(*patch));
		patch->image = read_rgb_image(buf, size);
		if (patch->image == NULL) {
			fprintf(stderr, "Couldn't decode example %s\n", path);
			abort();
		}
		TAILQ_INSERT_TAIL(examples, patch, link);
		free(buf);
	}
	assert(ret == ARCHIVE_EOF);
	if (archive_read_finish(arch))
		abort();
}

void free_examples(example_list_t *examples)
{
	example_patch_t *patch;

	while (!TAILQ_EMPTY(examples)) {
		patch = TAILQ_FIRST(examples);
		TAILQ_REMOVE(examples, patch, link);
		free(patch->image);
		free(patch);
	}
}
