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

#ifndef _LIB_RESULTS_H_
#define _LIB_RESULTS_H_	1

#include <sys/types.h>
#include <stdint.h>
#include <sys/queue.h>

#include "lib_filter.h"

/* image dimension */
typedef int32_t dim_t;

typedef struct patch {
    dim_t                               min_x;
    dim_t                               min_y;
    dim_t                               max_x;
    dim_t                               max_y;
} patch_t;

typedef struct bbox {
    dim_t                               min_x;
    dim_t                               min_y;
    dim_t                               max_x;
    dim_t                               max_y;
    double                              distance;
    TAILQ_ENTRY(bbox)   link;
} bbox_t;

typedef TAILQ_HEAD(bbox_list_t, bbox)   bbox_list_t;



typedef struct img_patches {
	dim_t		num_patches;
	double		distance;
	patch_t		patches[0];	
} img_patches_t;

#define	IMG_PATCH_SZ(num) (sizeof(img_patches_t) + ((num) * sizeof(patch_t)))



/*
 * some more attributes
 */

#define NUM_HISTO        "_nhisto_passed.int"

#define FILTER_MATCHES  "_filter.%s.patches"

#define RGB_IMAGE  "_rgb_image.rgbimage"
#define HISTO_II   "integral_histo"

#define ROWS       "_rows.int"
#define COLS       "_cols.int"


#ifdef	__cplusplus
extern "C" {
#endif

void save_patches(lf_obj_handle_t ohandle, const char *fname,
                  bbox_list_t *blist);


#ifdef	__cplusplus
}
#endif


#endif /*  _LIB_RESULTS_H_ */
