/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "lib_results.h"
#include "lib_sfimage.h"
#include "snapfind_consts.h"

/*
 * note: should make these functions localized 
 */
static char    *
skip_space_comments(char *buf, char *endbuf)
{
	int             eol = 0;

	assert(buf < endbuf);
	/*
	 * skip spaces 
	 */
	while (buf < endbuf && isspace(*buf)) {
		eol = (*buf == '\n');
		buf++;
	}

	/*
	 * skip any comments 
	 */
	while (buf < endbuf && eol && *buf == '#') {
		while (buf < endbuf && *buf != '\n') {
			buf++;
		}
		if (buf < endbuf)
			buf++;
	}
	/*
	 * skip spaces 
	 */
	while (buf < endbuf && isspace(*buf)) {
		eol = (*buf == '\n');
		buf++;
	}

	assert(buf < endbuf);
	return buf;
}


/*
 * read a portable anymap header from a bunch of bytes
 * returns 0 or error status
 */
int
pnm_parse_header(char *buf, size_t buflen,
                 int *width, int *height,
                 image_type_t * magic, int *headerlen)
{
	int             err = 0;
	char           *endptr;
	char           *startbuf,
	*endbuf;
	int             maxval;

	/*
	 * we are assuming that buflen > sizeof the whole header XXX 
	 */

	startbuf = buf;
	endbuf = buf + buflen;

	if (strncmp((char *)buf, "P5", 2) == 0) {
		*magic = IMAGE_PGM;
	} else if (strncmp((char *)buf, "P6", 2) == 0) {
		*magic = IMAGE_PPM;
	} else {
		*magic = IMAGE_UNKNOWN;
		/* XXXX */
		fprintf(stderr, "pnm_parse_hdear: unknown type %c%c \n",
			buf[0], buf[1]);
		return 1;
	}
	// fprintf(stderr, "read_header: got pgm file\n");

	/*
	 * we now assume the file is well-formed XXX 
	 */
	buf = skip_space_comments(buf + 2, endbuf);

	/*
	 * width, whitespace 
	 */
	*width = strtol(buf, &endptr, 0);
	if (*width == 0) {
		fprintf(stderr, "invalid image width (%.70s)\n", buf);
		assert(0);
	}
	buf = skip_space_comments(endptr, endbuf);

	/*
	 * height, whitespace 
	 */
	*height = strtol(buf, &endptr, 0);
	if (*height == 0) {
		fprintf(stderr, "invalid image height (%.70s)\n", buf);
		assert(0);
	}
	buf = skip_space_comments(endptr, endbuf);

	/*
	 * maxval 
	 */
	maxval = strtol(buf, &endptr, 0);
	buf = endptr;

	/*
	 * single whitespace 
	 */
	buf++;

	assert(buf - startbuf < (int) buflen);

	*headerlen = buf - startbuf;

	return err;
}
