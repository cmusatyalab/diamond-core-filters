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
 ********************************************************************** */
/*
 * create/read a rgb image from the given file.
 */

RGBImage       *
create_rgb_image(const char *filename)
{
	RGBImage       *img;
	int             err;
	FILE           *fp;
	char           *buf;
	int             buflen;
	int             width,
	height;
	image_type_t    magic;
	int             headerlen;
	int             bytes;
	const int       read_buffer_size = 128 << 10;

	fp = fopen(filename, "r");
	if (!fp) {
		perror("create_rgb_imag:  failed to read buffer:: ");
		return NULL;
	}

	buf = (char *) malloc(read_buffer_size);
	assert(buf);

	buflen = fread(buf, 1, read_buffer_size, fp);
	if (!buflen) {
		perror(filename);
		return NULL;
	}

	/*
	 * read the header and figure out the dimensions 
	 */
	err = pnm_parse_header(buf, buflen, &width, &height, &magic, &headerlen);
	if (err) {
		lf_log(LOGL_ERR, "%s: parse error", filename);
		return NULL;
	}

	if (magic != IMAGE_PPM) {
		lf_log(LOGL_ERR, "%s: only ppm format supported", filename);
		return NULL;
	}

	/*
	 * create image to hold the data 
	 */
	bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);
	img = (RGBImage *) malloc(bytes);
	if (!img) {
		lf_log(LOGL_ERR, "out%s: only ppm format supported", filename);
		return NULL;
	}
	img->nbytes = bytes;
	img->height = height;
	img->width = width;
	img->type = magic;

	/*
	 * read the data into img 
	 */
	{
		pnm_state_t    *state;
		char           *bufp = &buf[headerlen]; /* skip header */
		state = pnm_state_new(img);
		buflen -= headerlen;
		do
		{
			err = ppm_add_data(state, bufp, buflen);
			if (err) {
				free(img);
				img = NULL;
				fprintf(stderr, "error\n");
			}
			/*
			 * done with this bufp; 
			 */
			bufp = buf;
			buflen = fread(bufp, 1, read_buffer_size, fp);

		} while (!err && buflen);
		pnm_state_delete(state);
	}

	free(buf);
	fclose(fp);
	return img;
}


int
rgb_write_image(RGBImage * img, const char *filename, const char *dir)
{
	int             err;
	FILE           *fp;
	int             i;
	char            path[COMMON_MAX_PATH];


	/*
	 * get the full path name 
	 */
	err = snprintf(path, COMMON_MAX_PATH, "%s/%s", dir, filename);
	if (err >= COMMON_MAX_PATH) {
		fprintf(stderr, "MAX path exeeded, need at least %d bytes \n", err);
		assert(0);
	}


	fp = fopen(path, "w");
	if (!fp) {
		perror(path);
		return (EINVAL);
	}

	/*
	 * now we write out the header 
	 */
	fprintf(fp, "P6 \n");
	fprintf(fp, "%d %d \n", img->width, img->height);
	fprintf(fp, "255\n");

	for (i = 0; i < (img->height * img->width); i++) {
		err = fwrite(&img->data[i], 3, 1, fp);
		assert(err == 1);
	}

	fclose(fp);
	return (0);
}



/*
 ********************************************************************** */
/*
 * a collection of interfaces to incrementally build an RGBImage from
 * a stream of bytes.
 */

pnm_state_t    *
pnm_state_new(RGBImage * img)
{
	pnm_state_t    *state;

	state = (pnm_state_t *) malloc(sizeof(pnm_state_t));
	assert(state);
	state->img = img;
	state->parity = 0;
	state->bytes_remaining = img->width * img->height * 3;  /* 3 bytes per
		                                                             * pixel */
	assert(img->nbytes > (size_t) (img->width * img->height * 4));
	state->img_cur = (uint8_t *) state->img->data;

	/*
	 * fprintf(stderr, "pnm: bytes_remaining = %d; img capacity = %d\n",
	 * state->bytes_remaining, state->img->nbytes); 
	 */

	return state;
}


void
pnm_state_delete(pnm_state_t * state)
{
	free(state);
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



/*
 * read ppm-data from file, and fill in data. 
 */
int
ppm_add_data(pnm_state_t * state, char *buf, size_t nb)
{
	uint8_t        *fdata = (uint8_t *) buf;
	uint8_t        *imgdata = state->img_cur;
	int             parity = state->parity;

	assert(nb <= state->bytes_remaining);
	state->bytes_remaining -= nb;
	while (nb--) {
		*(imgdata++) = *(fdata++);
		parity++;
		if (parity == 3) {      /* XXX assumes structure of RGBPixel */
			*(imgdata++) = 255; /* fake alpha value */
			parity = 0;
		}
	}
	// fprintf(stderr, "rgb grew %d bytes\n", imgdata - state->img_cur);
	state->img_cur = imgdata;
	state->parity = parity;
	return 0;
}



void
img_constrain_bbox(bbox_t * bbox, RGBImage * img)
{

	if (bbox->min_x >= img->width) {
		bbox->min_x = img->width - 1;
	}
	if (bbox->min_y >= img->height) {
		bbox->min_y = img->height - 1;
	}

	if (bbox->max_x >= img->width) {
		bbox->max_x = img->width - 1;
	}
	if (bbox->max_y >= img->height) {
		bbox->max_y = img->height - 1;
	}
}

