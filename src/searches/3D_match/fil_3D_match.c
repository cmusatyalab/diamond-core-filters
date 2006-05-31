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
 * 3D object matching filter
 */

#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <sys/param.h>

#include "lib_filter.h"
#include "snapfind_consts.h"
#include "fil_3D_match.h"
#include "sys_attr.h"


int
f_init_3D_match(int numarg, char **args, int blob_len,
                    void *blob, const char *fname, void **data)
{
	match_config_t *fconfig;
	int i;

	/*
	 * filter initialization
	 */
	fconfig = (match_config_t *)malloc(sizeof(*fconfig));
	assert(fconfig);

	assert(numarg > 11);
	fconfig->name = strdup(args[0]);
	assert(fconfig->name != NULL);

	fconfig->distance = atof(args[1]);
        for (i = 0; i < NUM_FEATURES; i++) {
            fconfig->features[i] = atof(args[i+2]);
        }

	/*
	 * save the data pointer 
	 */
	*data = (void *) fconfig;
	return (0);
}

int
f_fini_3D_match(void *data)
{
	match_config_t *fconfig = (match_config_t *) data;
	free(fconfig);

	return (0);
}


int
f_eval_3D_match(lf_obj_handle_t ohandle, void *f_data)
{
	int             err;
	int i;
	match_config_t     *fconfig = (match_config_t *) f_data;
	size_t		len = COMMON_MAX_NAME;
	unsigned char   name[COMMON_MAX_NAME];
	char           *suffix;
	int             rv = 0;     /* return value */
	float           features[NUM_FEATURES];
	FILE *fid;

	lf_log(LOGL_TRACE, "f_eval_3D_match: enter");

	err = lf_read_attr(ohandle, DISPLAY_NAME, &len, (unsigned char *) name); 
	name[len] = '\0';
	suffix = strrchr((char *) name, '.');
	strcpy(suffix, ".jpg.query");
	fid = fopen((char *) name, "r");

	/* get the image features */
	for (i = 0; i < NUM_FEATURES; i++) {
	  /* this should work but doesn't */
	  /* sprintf(&featurename, "f%i", i);  
	     err = lf_read_attr(ohandle, featurename, &buflen, (unsigned char *) buf); 
	     features[i] = atof(buf); */
	  fscanf (fid, "%f", &features[i]);
	}
	fclose(fid);

	rv = isNeighbor(fconfig->distance, features, 
			fconfig->features, NUM_FEATURES);
	return rv;
}



/*******************************************************************************
int isNeighbor (float th, float *q, float *f, int size);
--> input parameters:

    th: threshold placed on euclidean distance to determine 
    if a given object is a neighbor of the posed query or not

    *q: pointer indicating the location of array containing 
    feature vector corresponding to query object

    *f: pointer indicating the location of array containing 
    feature vector corresponding to object in database

    size: length of feature vector (same as size of array at 
    *q and array at *f) used to represent the objects

--> neighbor: binary value (0 or 1).
    1 indicates the tested object is a neighbor of the posed 
    query (is close enough - as defined by the threshold th)

    0 indicates otherwise

Devi Parikh, May 24th 2006
*********************************************************************************/

int isNeighbor (float th, float *q, float *f, int size)
{
	float distance = 0;
	int i;
	for(i=0;i<size;i++)
	{
		distance=distance+pow((q[i]-f[i]),2);
	}

	if (distance < pow(th,2))
		return 1;
	else
		return 0;

}

