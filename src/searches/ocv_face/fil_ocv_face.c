
/*
 * face detector 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "filter_api.h"
#include "fil_face.h"
#include "face.h"
#include "face_tools.h"
#include "facedet.h"
#include "merge_faces.h"
#include "fil_tools.h"
#include "image_common.h"

typedef struct {
    double          over_thresh;
} overlap_state_t;

// #define VERBOSE 1

static int
process_region(ii_image_t * ii, int lev1, int lev2, region_t * bboxp,
               double scale, double m)
{
    int             found = 0;
    /*
     * test the region 
     */
    found = test_region(ii, lev1, lev2, bboxp->xmin, bboxp->ymin, scale, m)
        ? 1 : 0;

    return found;
}


int
f_init_detect(int numarg, char **args, int blob_len, void *blob_data,
              void **fdatap)
{

    fconfig_fdetect_t *fconfig;
    lf_fhandle_t    fhandle = 0;    /* XXX */

    if (numarg != 9) {
        lf_log(fhandle, LOGL_TRACE, "bad args in fdetect\n");
        assert(0);
        return (EINVAL);
    }

    fconfig = (fconfig_fdetect_t *) malloc(sizeof(*fconfig));
    assert(fconfig != NULL);

    fconfig->name = strdup(args[0]);
    assert(fconfig->name != NULL);
    fconfig->scale_mult = atof(args[1]);
    fconfig->xsize = atof(args[2]);
    fconfig->ysize = atof(args[3]);
    fconfig->stride = atof(args[4]);
    /*
     * XXX skip 5 for now ?? 
     */
    /*
     * XXX skip 6 for now ?? 
     */
    fconfig->lev1 = atoi(args[7]);
    fconfig->lev2 = atoi(args[8]);

    if (fconfig->lev2 > 37) {
	    fconfig->lev2 = 37;
    }

    if (fconfig->scale_mult <= 1) {
        lf_log(fhandle, LOGL_TRACE,
               "scale multiplier must be > 1; got %f\n", fconfig->scale_mult);
        exit(1);
    }

    *fdatap = fconfig;
    return (0);
}

int
f_fini_detect(void *fdata)
{
    fconfig_fdetect_t *fconfig = (fconfig_fdetect_t *) fdata;

    free(fconfig);
    return (0);
}

/*
 * loop over all the bboxes, and call the test function to determine if we
 * should accept them or not. if the initial list is null, we have to
 * sythesize the universe of possible bboxes. returns total number of
 * (potential) faces detected. 
 */

int
f_eval_detect(lf_obj_handle_t ohandle, int numout, lf_obj_handle_t * ohandles,
              void *fdata)
{
    int             pass = 0;
    ii_image_t     *ii;
    ii2_image_t    *ii2;
    search_param_t  param;
    fconfig_fdetect_t *fconfig = (fconfig_fdetect_t *) fdata;
    int             count;
    off_t           bsize;
    int             err;
    lf_fhandle_t    fhandle = 0;    /* XXX */
    double          last_scale = 0.0;
	bbox_list_t		blist;
    double          xsiz,
                    ysiz;       /* size of window px */
    dim_t           width,
                    height;     /* size of image px */
    static int      inited = 0;

    lf_log(fhandle, LOGL_TRACE, "f_detect: enter\n");

    if (!inited) {
        init_classifier();
        inited = 1;
    }

    xsiz = fconfig->xsize;
    ysiz = fconfig->ysize;

    /*
     * get the ii 
     */
    ii = (ii_image_t *) ft_read_alloc_attr(fhandle, ohandle, II_DATA);
    assert(ii);
    ii2 = (ii2_image_t *) ft_read_alloc_attr(fhandle, ohandle, II_SQ_DATA);
    assert(ii2);

    /*
     * get width, height 
     */
    bsize = sizeof(int);
    err = lf_read_attr(fhandle, ohandle, ROWS, &bsize, (char *) &height);
    assert(!err);
    bsize = sizeof(int);
    err = lf_read_attr(fhandle, ohandle, COLS, &bsize, (char *) &width);
    assert(!err);



    /*
     * get count 
     */
    bsize = sizeof(int);
    err = lf_read_attr(fhandle, ohandle, NUM_FACE, &bsize, (char *) &count);
    if (err)
        count = 0;              /* XXX */

    if (count > 0) {
        int             i;
        /*
         * foreach 'param in attribs 
         */
        for (i = 0; i < count; i++) {
            read_param(fhandle, ohandle, FACE_BBOX_FMT, &param, i);
            /*
             * We keep track of the last scale we made to the table
             * if it is not what we needed then we scale it now.  Ideally
             * these are going to be ordered by the scale.
             */
            if (last_scale != param.scale) {
                scale_feature_table(param.scale);
            }
            if (process_region(ii, fconfig->lev1, fconfig->lev2, &param.bbox,
                               param.scale, param.img_var)) {
                param.lev1 = fconfig->lev1;
                param.lev2 = fconfig->lev2;
                /*
                 * ok to potentially overwrite since we just read this anyway 
                 */
                write_param(fhandle, ohandle, FACE_BBOX_FMT, &param, pass);
                pass++;
            }
        }
    } else {
		bbox_t *		cur_box;
		int				i;
		search_param_t	param;


		TAILQ_INIT(&blist);
		pass = face_scan_image(ii, ii2, fconfig, &blist, height, width);	

		i = 0;
		while (!(TAILQ_EMPTY(&blist))) {
			cur_box = TAILQ_FIRST(&blist);
        	param.type = PARAM_FACE;
	  		param.bbox.xmin = cur_box->min_x;
        	param.bbox.ymin = cur_box->min_y;
        	param.bbox.xsiz = cur_box->max_x - cur_box->min_x;
        	param.bbox.ysiz = cur_box->max_y - cur_box->min_y;
			write_param(fhandle, ohandle, FACE_BBOX_FMT, &param, i);
			TAILQ_REMOVE(&blist, cur_box, link);
        	free(cur_box);
			i++;
		}
    }

    /*
     * save 'pass in attribs 
     */
    err = lf_write_attr(fhandle, ohandle, NUM_FACE, sizeof(int),
                      (char *) &pass);
    assert(!err);
    lf_log(fhandle, LOGL_TRACE, "found %d faces\n", pass);
    /*
     * save some stats 
     */
    if (ii) {
        ft_free(fhandle, (char *) ii);
    }

    /*
     * delete ii2 (just put a null until we have a delete) XXX 
     */
    /*
     * don't need the ii2 now because we saved the variance in the param
     * struct. 
     */
    ft_free(fhandle, (char *) ii2);
    ii2 = NULL;
    err = lf_write_attr(fhandle, ohandle, II_SQ_DATA, sizeof(char *), (char *) &ii2);   /* pseudo 
                                                                                         * delete 
                                                                                         */
    assert(!err);

    lf_log(fhandle, LOGL_TRACE, "f_detect: done\n");
    return pass;
}


int
f_init_bbox_merge(int numarg, char **args, int blob_len, void *blob,
                  void **fdatap)
{
    overlap_state_t *ostate;

    ostate = malloc(sizeof(*ostate));
    ostate->over_thresh = atof(args[0]);

    *fdatap = ostate;
    return (0);
}


int
f_fini_bbox_merge(void *fdata)
{
    overlap_state_t *ostate = (overlap_state_t *) fdata;
    free(ostate);
    return (0);
}


int
f_eval_bbox_merge(lf_obj_handle_t ohandle, int numout,
                  lf_obj_handle_t * ohandles, void *fdata)
{
    int             count,
                    i;
    off_t           bsize;
    search_param_t  param;
    int             err = 0;
    lf_fhandle_t    fhandle = 0;    /* XXX */
    region_t       *in_bbox_list,
                   *out_bbox_list;
    overlap_state_t *ostate = (overlap_state_t *) fdata;

    /*
     * get count 
     */
    bsize = sizeof(int);
    err = lf_read_attr(fhandle, ohandle, NUM_FACE, &bsize, (char *) &count);
    if (err)
        count = 0;              /* XXX */

    lf_log(fhandle, LOGL_TRACE, "bbox_merge: incount = %d\n", count);

    /*
     * allocate arrays 
     */
    bsize = count * sizeof(region_t);
    err = lf_alloc_buffer(fhandle, bsize, (char **) &in_bbox_list);
    assert(!err);
    err = lf_alloc_buffer(fhandle, bsize, (char **) &out_bbox_list);
    assert(!err);

    /*
     * foreach 'param in attribs 
     */
    for (i = 0; i < count; i++) {
        read_param(fhandle, ohandle, FACE_BBOX_FMT, &param, i);
        in_bbox_list[i] = param.bbox;
        assert(param.bbox.xmin >= 0);
    }

    printf("merge %f \n", ostate->over_thresh);
    count =
        merge_boxes(in_bbox_list, out_bbox_list, count, ostate->over_thresh);

    for (i = 0; i < count; i++) {
        param.bbox = out_bbox_list[i];
        assert(param.bbox.xmin >= 0);
        write_param(fhandle, ohandle, FACE_BBOX_FMT, &param, i);
    }

    lf_free_buffer(fhandle, (char *) in_bbox_list);
    lf_free_buffer(fhandle, (char *) out_bbox_list);

    err =
        lf_write_attr(fhandle, ohandle, NUM_FACE, sizeof(int),
                      (char *) &count);
    assert(!err);

    lf_log(fhandle, LOGL_TRACE, "bbox_merge: outcount = %d\n", count);

    return count;
}
