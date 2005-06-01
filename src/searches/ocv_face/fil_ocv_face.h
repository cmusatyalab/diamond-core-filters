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




#ifdef __cplusplus
extern "C"
{
#endif

	int f_eval_detect(lf_obj_handle_t ohandle, int numout, lf_obj_handle_t *ohandles,
	                  void *fdata);
	int f_init_detect(int numarg, char **args, int blob_len, void *blob,
	                  void **fdatap);
	int f_fini_detect(void *fdatap);


	int f_init_bbox_merge(int numarg, char **args, int blob_len, void *blob,
	                      void **fdatap);
	int f_fini_bbox_merge(void *fdata);
	int f_eval_bbox_merge(lf_obj_handle_t ohandle, int numout,
	                      lf_obj_handle_t *ohandles, void *fdata);


#ifdef __cplusplus
}
#endif
