/*
 *  Diamond Core Filters - collected filters for the Diamond platform
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
#include "fil_num_attr.h"


static int
f_init_num_attr(int argc, const char * const *args,
		int blob_len, const void *blob_data,
		const char *fname, void **fdatap)
{
	int i;
	fnum_attr_t  *	fconfig;
	fnum_attr_holder_t * fholder;

	fholder = (fnum_attr_holder_t *) malloc(sizeof(fnum_attr_holder_t));
	assert(fholder != NULL);

	fholder-> num_configs = argc / 4;
	assert((argc % 4) == 0);
	
	fholder->fconfigs = (fnum_attr_t *) malloc(sizeof(fnum_attr_t) * 
			fholder->num_configs);
	assert(fholder->fconfigs != NULL);
	
	for (i = 0; i<fholder->num_configs; i++) {
		fconfig = &fholder->fconfigs[i];
		fconfig->attr_name = strdup(args[0+4*i]);
		fconfig->min_value = atof(args[1+4*i]);
		fconfig->max_value = atof(args[2+4*i]);
		fconfig->drop_missing = !strcasecmp(args[3+4*i], "true");
	}

	*fdatap = fholder;
	return (0);
}

/*
 */
static int
f_eval_num_attr(lf_obj_handle_t ohandle, void *fdata)
{
	int 			i;
	const void *		data;
	size_t			dsize;
	float			val;
	fnum_attr_holder_t  *	fholder = (fnum_attr_holder_t *) fdata;
	fnum_attr_t  *		fconfig;
	int             err;

	for (i = 0; i<fholder->num_configs; i++) {
		fconfig = &fholder->fconfigs[i];
		err = lf_ref_attr(ohandle, fconfig->attr_name, &dsize, 
		    &data);
		if (err) {
			if (fconfig->drop_missing) {
				return(0);
			}
		} else {
			val = atof((char *)data);
			if ((val < fconfig->min_value) || (val > fconfig->max_value)) {
				return(0);
			}
		}
	}
	return(1);
}

LF_MAIN(f_init_num_attr, f_eval_num_attr)
