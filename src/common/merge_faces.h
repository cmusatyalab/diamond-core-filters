
#ifdef __cplusplus
extern "C" {
#endif

int merge_boxes(region_t in_bbox_list[], region_t out_bbox_list[], int num,
		double over_thresh);

#ifdef __cplusplus
}
#endif
