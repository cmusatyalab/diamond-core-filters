// Rajiv: here's my merge_faces stuff
// I can redo the interface
//

#include <assert.h>
#include <stdlib.h>	// for calloc
#include <stdio.h>	// for fprintf

#include "face.h"
#include "merge_faces.h"

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

// Given two bboxes, returns whether they are similar enough to be merged
// Basic idea:
// 1) Calculate area of intersection between boxes
//    Return 0 if either dimension is negative (i.e., no overlap)
// 2) Calculate area of union of boxes (bbox surrounding the boxes)
// 3) If intersection/union > threshold (e.g., 80%) then the
//    boxes are sufficiently similar.
//
int 
similar_bbox(region_t* bb1, region_t* bb2, double overlap_thresh) 
{
  // Intersection: tl = "top-left"; br = "bottom-right"
  dim_t i_tlx = MAX(bb1->xmin, bb2->xmin);
  dim_t i_tly = MAX(bb1->ymin, bb2->ymin);
  dim_t i_brx = MIN(bb1->xmin + bb1->xsiz, bb2->xmin + bb2->xsiz);
  dim_t i_bry = MIN(bb1->ymin + bb1->ysiz, bb2->ymin + bb2->ysiz);
  double overlap;
  if (i_brx < i_tlx) { return 0; }	// since area will be zero
  if (i_bry < i_tly) { return 0; }	// since area will be zero
  int i_area = (i_brx - i_tlx) * (i_bry - i_tly);

  // Union: tl = "top-left"; br = "bottom-right"
  dim_t u_tlx = MIN(bb1->xmin, bb2->xmin);
  dim_t u_tly = MIN(bb1->ymin, bb2->ymin);
  dim_t u_brx = MAX(bb1->xmin + bb1->xsiz, bb2->xmin + bb2->xsiz);
  dim_t u_bry = MAX(bb1->ymin + bb1->ysiz, bb2->ymin + bb2->ysiz);
  assert(u_brx >= u_tlx);
  assert(u_bry >= u_tly);
  int u_area = (u_brx - u_tlx) * (u_bry - u_tly);

  assert(u_area >= i_area);	// sanity check that union >= intersection

  // integer arithmetic version of saying
  // (i_area/u_area) should be >= 80%
#if 0
  fprintf(stderr, "merge_faces: i_area=%d u_area=%d ratio=%f... ",
      i_area, u_area, 100.0 * i_area/(double)u_area);
  if (4 * i_area >= 3 * u_area) {
      fprintf(stderr, " MERGE!\n");
  } else {
      fprintf(stderr, " leave\n");
  }
#endif
  overlap = (double)u_area/(double)i_area;

  return(overlap >= overlap_thresh);
}

// Returns the number of merged boxes
// Does a pairwise match between bbox i & bbox j
// At the end, we do a union-find and merge all boxes that
// were sufficiently similar.  They are returned in out_bbox_list
// (which should have been allocated by the caller)
// 
int merge_boxes(region_t in_bbox_list[], region_t out_bbox_list[], int num,
	double overlap_thresh) 
{	
  assert(num >= 0);
  int i, j,k;
  int cnum = 0; 	// number of final clusters

  if (num == 0) { return 0; }	// no work needs to be done
  if (num == 1) {
    out_bbox_list[0] = in_bbox_list[0];
    return 1;
  }

  // cluster_id[i] tells you what cluster in_bbox_list[i]
  // should be assigned to.
  int* cluster_ids = (int*) calloc(num, sizeof(int));
  assert(cluster_ids);
  for ( i=0; i<num-1; i++) {
    cluster_ids[i] = i;	// assign each bbox to its own cluster
  }

  // This is a somewhat inefficient algorithm for union-find
  // but I'm feeling lazy.
  //
  for ( i=0; i<num-1; i++) {
    for ( j=i+1; j<num; j++) {
      if (similar_bbox(&in_bbox_list[i], &in_bbox_list[j], overlap_thresh)) {
	// these two bboxes are similar enough that we want to
	// merge their sets
	for ( k=0; k<num; k++) {
	  if (cluster_ids[k] == j) { cluster_ids[k] = i; }
	}
      }
    }
  }

  // At the end of this, cluster_ids[] contains a labeling of the
  // clusters to be averaged together.  We can compute averages
  // in a single pass through the in_box_list
  //
  double* tlxs = (double*) calloc(num, sizeof(double));
  double* tlys = (double*) calloc(num, sizeof(double));
  double* brxs = (double*) calloc(num, sizeof(double));
  double* brys = (double*) calloc(num, sizeof(double));
  int* nums = (int*) calloc(num, sizeof(int));
  assert(tlxs); assert(tlys); assert(brxs); assert(brys);

  for ( k=0; k<num; k++) {
    int cid = cluster_ids[k];
    tlxs[cid] += in_bbox_list[k].xmin;
    assert(tlxs[cid] >= 0);
    tlys[cid] += in_bbox_list[k].ymin;
    brxs[cid] += in_bbox_list[k].xmin + in_bbox_list[k].xsiz;
    brys[cid] += in_bbox_list[k].ymin + in_bbox_list[k].ysiz;
    nums[cid]++;
    if (cid >= cnum) { cnum = cid+1; }
  }

  int oid=0;
  for ( k=0; k<cnum; k++) {
    // A cluster index could have no members (due to merging)
    // In this case, we skip that cluster.
    int d = (int)nums[k];	// Better rounding needed?
    if (d == 0) { continue; }
    assert(d > 0.0);
    out_bbox_list[oid].xmin = (dim_t)(tlxs[k]/d);
    assert(out_bbox_list[oid].xmin >= 0);
    out_bbox_list[oid].ymin = (dim_t)(tlys[k]/d);
    out_bbox_list[oid].xsiz = (dim_t)((brxs[k]-tlxs[k])/d);
    out_bbox_list[oid].ysiz = (dim_t)((brys[k]-tlys[k])/d);
    oid++;
  }

  free(nums);	nums = NULL;
  free(brys);	brys = NULL;
  free(brxs);	brxs = NULL;
  free(tlys); 	tlys = NULL;
  free(tlxs);	tlxs = NULL;
  free(cluster_ids);	cluster_ids = NULL;

  // fprintf(stderr, "merge_faces: oid=%d\n", oid);
  return oid;
}
