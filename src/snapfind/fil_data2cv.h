
//#ifndef _FIL_DATA2CV_H_
//#define _FIL_DATA2CV_H_

#include <opencv/cv.h>

#ifdef __cplusplus
extern "C" {
#endif

int f_init_get_rgba_ipl_image(int numarg, char **args, int blob_len, 
				void *blob_data, void **fdatap);

int f_fini_get_rgba_ipl_image(void *fdata);

int f_eval_get_rgba_ipl_image(lf_obj_handle_t ohandle, int numout, 
				lf_obj_handle_t * ohandles, void *fdata);




int f_init_get_gray_ipl_image(int numarg, char **args, int blob_len,
    void *blob_data, void **fdatap);

int f_fini_get_gray_ipl_image(void *fdata);

int f_eval_get_gray_ipl_image(lf_obj_handle_t ohandle, int numout, 
				lf_obj_handle_t * ohandles, void *fdata);


#ifdef __cplusplus
}
#endif

//#endif /* _FIL_DATA2CV_H_ */
