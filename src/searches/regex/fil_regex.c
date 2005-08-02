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


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <assert.h>
#include <string.h>
#include "filter_api.h"
#include "fil_regex.h"
#include "face_consts.h"

// #define VERBOSE 1


int
f_init_regex(int argc, char **args, int blob_len, void *blob_data,
             const char *fname, void **fdatap)
{
	int             i;
	fdata_regex_t  *fconfig;
	int             err;


	fconfig = (fdata_regex_t *) malloc(sizeof(*fconfig));
	assert(fconfig != NULL);

	assert(argc > 1);
	fconfig->num_attrs = atoi(args[0]);
	fconfig->num_regexs = argc - fconfig->num_attrs - 1;

	fconfig->attr_names =
	    (char **) malloc(sizeof(char *) * fconfig->num_attrs);
	assert(fconfig->attr_names != NULL);
	fconfig->regex_names = (char **)
	                       malloc(sizeof(char *) * fconfig->num_regexs);
	assert(fconfig->regex_names != NULL);

	fconfig->regs = (regex_t *) malloc(sizeof(regex_t) * fconfig->num_regexs);
	assert(fconfig->regs != NULL);

	for (i = 0; i < fconfig->num_attrs; i++) {
		fconfig->attr_names[i] = strdup(args[i + 1]);
		assert(fconfig->attr_names[i] != NULL);
	}

	for (i = 0; i < fconfig->num_regexs; i++) {
		fconfig->regex_names[i] = strdup(args[i + 1 + fconfig->num_attrs]);
		assert(fconfig->regex_names[i] != NULL);
		err = regcomp(&fconfig->regs[i], fconfig->regex_names[i],
		              REG_ICASE | REG_NOSUB);
		assert(err == 0);
	}

	*fdatap = fconfig;
	return (0);
}

int
f_fini_regex(void *fdata)
{
	fdata_regex_t  *fconfig = (fdata_regex_t *) fdata;
	int             i;


	for (i = 0; i < fconfig->num_regexs; i++) {
		regfree(&fconfig->regs[i]);
		free(fconfig->regex_names[i]);
	}

	for (i = 0; i < fconfig->num_attrs; i++) {
		free(fconfig->attr_names[i]);
	}

	free(fconfig->regs);
	free(fconfig->regex_names);
	free(fconfig->attr_names);
	free(fconfig);
	return (0);
}

/*
 */
int
f_eval_regex(lf_obj_handle_t ohandle, void *fdata)
{
	int             pass = 0;   /* if we decided to pass */
	int             termpass[MAX_REGEX];
	int             i,
	j;
	fdata_regex_t  *fconfig = (fdata_regex_t *) fdata;
	int             err;

	assert(fconfig->num_regexs < MAX_REGEX);
	for (i = 0; i < fconfig->num_regexs; i++) {
		termpass[i] = 0;
	}

	for (i = 0; i < fconfig->num_attrs; i++) {
		char            buf[BUFSIZ];
		off_t           bsize = BUFSIZ;

		err = lf_read_attr(ohandle, fconfig->attr_names[i],
		                   &bsize, (char *) buf);

		for (j = 0; !err && j < fconfig->num_regexs; j++) {
			if (!termpass[j]) {
				if (regexec(&fconfig->regs[j], buf, 0, NULL, 0) == 0) {
					char            buf[BUFSIZ];
					int             val = 1;
					termpass[j] = 1;
					/*
					 * add an attribute to say we passed this regex (for fun) 
					 */
					sprintf(buf, "regex_%s", fconfig->regex_names[j]);
					err = lf_write_attr(ohandle, buf, sizeof(int),
					                    (char *) &val);
					assert(!err);
				}
			}
		}
	}

	pass = 1;
	for (j = 0; j < fconfig->num_regexs; j++) {
		pass &= termpass[j];
	}

	return pass;
}
