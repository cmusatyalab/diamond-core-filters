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

#ifndef	_GABOR_PRIV_H_
#define	_GABOR_PRIV_H_	1

/* XXX fix this !!! */
#define NUM_HISTO        "_nhisto_passed.int"
#define NUM_TEXTURE        "_ntexture_passed.int"
#define GAB_BBOX_FMT   "_h_box%d.search_param"
typedef enum param_type_t {
    PARAM_UNKNOWN = 0,
    PARAM_FACE,
    PARAM_HISTO,
    PARAM_TEXTURE
} param_type_t;
                                                                                
typedef int32_t dim_t;

typedef struct region
{
        dim_t xmin, ymin;
        dim_t xsiz, ysiz;
}
region_t;

/*
 * attributes passed to the test function
 */
#define PARAM_NAME_MAX 15
typedef struct search_param {
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
                                                                                

#endif	/* !_GABOR_PRIV_H_ */
