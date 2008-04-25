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

#ifndef _LIB_RESULTS_H_
#define _LIB_RESULTS_H_	1

#include <sys/types.h>
#include <errno.h>
#include <stdint.h>
#include "queue.h"
#include "lib_filter.h"

/* image dimension */
typedef int32_t dim_t;

/*
 * bounding box of region to consider
 */
typedef struct region {
	dim_t xmin, ymin;
	dim_t xsiz, ysiz;
} region_t;

typedef enum param_type_t {
    PARAM_UNKNOWN = 0,
    PARAM_FACE,
    PARAM_HISTO,
    PARAM_TEXTURE
} param_type_t;

typedef struct patch {
    int                                 min_x;
    int                                 min_y;
    int                                 max_x;
    int                                 max_y;
} patch_t;

typedef struct bbox {
    int                                 min_x;
    int                                 min_y;
    int                                 max_x;
    int                                 max_y;
    double                              distance;
    TAILQ_ENTRY(bbox)   link;
} bbox_t;

typedef TAILQ_HEAD(bbox_list_t, bbox)   bbox_list_t;



/*
 * attributes passed to the test function
 */
#define PARAM_NAME_MAX 15
typedef struct search_param
{
        param_type_t type;
        int lev1, lev2;         /* range of tests to apply (face) */
        region_t bbox;          /* bounding box */
        double scale;           /* (face) */
        union {
                double img_var; /* (face) */
                double distance; /* (histo) */
        };
        char name[PARAM_NAME_MAX+1];
        int id;
} search_param_t;


typedef struct img_patches {
	int		num_patches;
	double		distance;
	patch_t		patches[0];	
} img_patches_t;

#define	IMG_PATCH_SZ(num) (sizeof(img_patches_t) + ((num) * sizeof(patch_t)))



/*
 * some more attributes
 */

//#define NUM_BBOXES "_num_bboxes.int"
//#define BBOX_FMT   "_bbox%d.search_param"

#define NUM_HISTO        "_nhisto_passed.int"
#define NUM_FACE         "_nface_passed.int"
#define NUM_TEXTURE      "_ntexture_passed.int"
#define HISTO_BBOX_FMT   "_h_box%d.search_param"
#define FACE_BBOX_FMT    "_f_box%d.search_param"

#define FILTER_MATCHES  "_filter.%s.patches"

#define POISON_FN  "poison_%s"

#define II_DATA    "_integral_img.ii_image"
#define II_SQ_DATA "_integral_img_sq.ii_image"
#define RGB_IMAGE  "_rgb_image.rgbimage"
#define HISTO_II   "integral_histo"

#define ROWS       "_rows.int"
#define COLS       "_cols.int"
#define IMG_HEADERLEN "_headerlen.int"

#define RGBA_IPL_IMAGE "_rgba_image.ipl_image"
#define GRAY_IPL_IMAGE "_gray_image.ipl_image"


#define KEYWORDS "Keywords"
#define CONTENT_TYPE "Content-Type"
#define PARENT_OIT "Parent-OID"



#define FILTER_TYPE_COLOR 0
#define FILTER_TYPE_TEXTURE 1
#define FILTER_TYPE_ARENA 2

#ifdef	__cplusplus
extern "C" {
#endif

int write_param(lf_obj_handle_t ohandle, char *fmt,
                        search_param_t *param, int i);
int read_param(lf_obj_handle_t ohandle, char *fmt,
                       search_param_t *param, int i);


img_patches_t * get_patches(lf_obj_handle_t ohandle, char *fname);
void save_patches(lf_obj_handle_t ohandle, char *fname, bbox_list_t *blist);


char *ft_read_alloc_attr(lf_obj_handle_t ohandle, const char *name);
void ft_free(char *ptr);


#ifdef	__cplusplus
}
#endif


#endif /*  _LIB_RESULTS_H_ */
