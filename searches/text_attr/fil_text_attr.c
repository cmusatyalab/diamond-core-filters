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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <regex.h>
#include <assert.h>
#include <string.h>
#include "lib_filter.h"
#include "text_attr_priv.h"
#include "fil_text_attr.h"


int
f_init_text_attr(int argc, char **args, int blob_len, void *blob_data,
             const char *fname, void **fdatap)
{
	fdata_text_attr_t  *fconfig;
	int             err;


	fconfig = (fdata_text_attr_t *) malloc(sizeof(*fconfig));
	assert(fconfig != NULL);

	assert(argc == 4);

	fconfig->attr_name = strdup(args[0]);
	assert(fconfig->attr_name != NULL);

	fconfig->string = strdup(args[1]);
	assert(fconfig->string != NULL);

	fconfig->exact_match = atoi(args[2]);
	fconfig->drop_missing = atoi(args[3]);

	if (!fconfig->exact_match) {
		err = regcomp(&fconfig->regex, fconfig->string,
		              REG_ICASE | REG_NOSUB);
		assert(err == 0);
	}

	*fdatap = fconfig;
	return (0);
}

int
f_fini_text_attr(void *fdata)
{
	fdata_text_attr_t  *	fconfig = (fdata_text_attr_t *) fdata;

	if (!fconfig->exact_match) {
		regfree(&fconfig->regex);
	}

	free(fconfig->attr_name);
	free(fconfig->string);
	free(fconfig);

	return (0);
}

int
f_eval_text_attr(lf_obj_handle_t ohandle, void *fdata)
{
	fdata_text_attr_t  *fconfig = (fdata_text_attr_t *) fdata;
	int             err;
	size_t		bsize;
	const void *	buf; 

	err = lf_ref_attr(ohandle, fconfig->attr_name, &bsize, &buf);
	/* handle case where attributed doesn't exist */
	if (err) {
		if (fconfig->drop_missing) {
			return(0);
		} else {
			return(1);
		}

	}

	/* for exact match use strcmp */
	/* handle wierd data XXX */
	if (fconfig->exact_match) {
		if (strcmp(fconfig->string, (char *)buf) == 0) {
			return(1);
		} else {
			return(0);
		}
	} else {
		if (regexec(&fconfig->regex, (char *)buf, 0, NULL, 0) == 0) {
			return(1);
		} else {
			return(0);
		}
	}
}

