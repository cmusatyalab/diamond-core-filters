


#ifdef __cplusplus
extern "C" {
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
