

#ifdef __cplusplus
extern "C" {
#endif

int f_init_integrate(int numarg, char **args, int blob_len, void *blob,
				void **fdatap);
int f_fini_integrate(void *fdatap);
int f_eval_integrate(lf_obj_handle_t ohandle, int numout, 
			lf_obj_handle_t *ohandles, void *fdata);

int rgb_integrate(RGBImage * img, u_int32_t * sumarr, 
	float *sumsqarr, size_t iiwidth, size_t iiheight);

#ifdef __cplusplus
}
#endif
