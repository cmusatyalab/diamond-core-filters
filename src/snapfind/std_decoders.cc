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
#include "attr_ent.h"
#include "attr_decode.h"


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
