#ifndef	_FIL_HISTO_H_
#define _FIL_HISTO_H_

#include "filter_api.h"
#include "common_consts.h"
#include "histo.h"
#include "queue.h"
#include "face.h"
#include "image_common.h"



#ifdef __cplusplus
extern "C" {
#endif


int f_init_pnm2rgb(int numarg, char **args, int blob_len, void *blob, 
		void **data);
int f_fini_pnm2rgb(void *data);
int f_eval_pnm2rgb(lf_obj_handle_t ihandle, int numout, lf_obj_handle_t *ohandles,
		void *user_data);



int f_init_histo_detect(int numarg, char **args, int blob_len, void *blob, 
		void **data);
int f_fini_histo_detect(void *data);
int f_eval_histo_detect(lf_obj_handle_t ihandle, int numout, 
			lf_obj_handle_t *ohandles, void *user_data);


int f_init_hpass(int numarg, char **args, int blob_len, void *blob, 
		void **data);
int f_fini_hpass(void *data);
int f_eval_hpass(lf_obj_handle_t ihandle, int numout, 
			lf_obj_handle_t *ohandles, void *user_data);


int f_init_hintegrate(int numarg, char **args, int blob_len, void *blob, 
		void **data);
int f_fini_hintegrate(void *data);
int f_eval_hintegrate(lf_obj_handle_t ihandle, int numout, 
			lf_obj_handle_t *ohandles, void *user_data);



#ifdef __cplusplus
}
#endif

#endif	/* !defined _FIL_HISTO_H_ */
