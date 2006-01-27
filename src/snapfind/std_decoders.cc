/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */


/*
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>
#include <libgen.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <gtk/gtk.h>
#include "rgb.h"
#include "attr_ent.h"
#include "attr_decode.h"
#include "lib_results.h"


int
hex_decode::decode(unsigned char *data, size_t dlen, char *string, size_t slen)
{
	size_t	doff = 0, soff = 0;

	/* make sure we have data and room for the data, we need at
	 * least 3 bytes to continue because we need 2 for the next text
	 * and 1 for the termination.
	 */
	while ((doff < dlen) && (soff < (slen - 3))) {
		sprintf(string + soff, "%02x", data[doff]);
		doff++;
		soff += 2;
	}
	string[soff] = 0;
	return(0);
}

int
hex_decode::is_type(unsigned char *data, size_t dlen)
{
	return(1);
}


int
text_decode::decode(unsigned char *data, size_t dlen, char *string, size_t slen)
{
	size_t	doff = 0, soff = 0;
	int	ret = 0;

	/* make sure we have data and room for the data, we need at
	 * least 2 bytes to continue because we need 1 for the next char
	 * and 1 for the termination.
	 */
	while ((doff < dlen) && (soff < (slen - 2))) {
		if (!isprint(data[doff])) {
			if ((data[doff] == 0) && (doff == (dlen - 1))) {
				break;
			} else {
				sprintf(string + soff, ".");
				ret = 1;
			}
		} else {
			sprintf(string + soff, "%c", data[doff]);
		}
		doff++;
		soff++;
	}
	string[soff] = 0;
	return(ret);
}

int
text_decode::is_type(unsigned char *data, size_t dlen)
{
	unsigned int	i;

	/*
	 * we look at all but the last character to see 
  	 * if they are printable, if so, then this is likely a 
	 * character string.
	 */
	for (i=0; i < (dlen-1); i++) {
		if (!isprint(data[i])) {
			return(0);
		}
	}
	return(2);
}


int
int_decode::decode(unsigned char *data, size_t dlen, char *string, size_t slen)
{
	int	dval;

	dval = *((int *)data);
	sprintf(string, "%d", dval);
	return(0);
}

int
int_decode::is_type(unsigned char *data, size_t dlen)
{
	if (dlen == sizeof(int)) {
		return(2);
	}
	return(0);
}


int
time_decode::decode(unsigned char *data, size_t dlen, char *string, size_t slen)
{
	uint64_t  time;
	uint64_t  secs;
	uint64_t  nsecs;

	time = *((uint64_t *)data);
	secs = time / 1000000000;
	nsecs = time % 1000000000;

	snprintf(string, slen, "%llu sec %llu nsecs", secs, nsecs);
	return(0);
}

int
time_decode::is_type(unsigned char *data, size_t dlen)
{
	if (dlen == sizeof(uint64_t)) {
		return(2);
	}
	return(0);
}


int
rgb_decode::decode(unsigned char *data, size_t dlen, char *string, size_t slen)
{
	RGBImage *rgb;
	size_t	offset = 0;
	int	i;
	


	if (dlen < sizeof(*rgb)) {
		snprintf(string, slen, "Invalid Data");
		return(0);	
	}

	rgb = (RGBImage *)data;

	offset += snprintf(string + offset, slen - offset, 
    	    "rows=%d columns=%d ", rgb->rows, rgb->columns);

	if (offset >= slen-1) {
		return(0);
	}

	for (i = 0; i < rgb->height * rgb->rows; i++) {
		offset += snprintf(string + offset, slen - offset, 
    	    	    "{r=%d, b=%d, g=%d} ",
		    rgb->data[i].r, rgb->data[i].g, rgb->data[i].b);
		if (offset >= slen-1) {
			return(0);
		}
	}

	return(0);
}

int
rgb_decode::is_type(unsigned char *data, size_t dlen)
{
	RGBImage *rgb;
	if (dlen < sizeof(RGBImage)) {
		return(0);
	}

	rgb = (RGBImage *)data;
	if (rgb->nbytes == dlen) {
		return(4);
	}
	return(0);
}


int
patches_decode::decode(unsigned char *data, size_t dlen, char *string, size_t slen)
{
	img_patches_t *patches;
	size_t	offset = 0;
	int	i;
	


	if (dlen < sizeof(*patches)) {
		snprintf(string, slen, "Invalid Data");
		return(0);	
	}

	patches = (img_patches_t *)data;

	offset += snprintf(string + offset, slen - offset, 
    	    "npatches=%d distance=%8.4f ", patches->num_patches,
	    patches->distance);

	if (offset >= slen-1) {
		return(0);
	}

	for (i = 0; i < patches->num_patches; i++) {
		offset += snprintf(string + offset, slen - offset, 
    	    	    "{x_lo=%d x_high=%d y_low=%d y_high=%d ",
		    patches->patches[i].min_x, patches->patches[i].max_x, 
		    patches->patches[i].min_y, patches->patches[i].max_y);
		if (offset >= slen-1) {
			return(0);
		}
	}

	return(0);
}

int
patches_decode::is_type(unsigned char *data, size_t dlen)
{
	img_patches_t *patches;
	if (dlen < sizeof(img_patches_t)) {
		return(0);
	}

	patches = (img_patches_t *)data;
	if (IMG_PATCH_SZ(patches->num_patches) == dlen) {
		return(4);
	}
	return(0);
}

