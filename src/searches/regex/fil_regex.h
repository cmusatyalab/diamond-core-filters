
#include "filter_api.h"


typedef struct {
	int			num_attrs;
	int			num_regexs;
	char **		attr_names;
	char **		regex_names;
	regex_t	*	regs; 

} fdata_regex_t;

#ifdef __cplusplus
extern "C" {
#endif

int f_init_regex(int numarg, char **args, int blob_len, void *blob,
			void **fdatap);
int f_fini_regex(void *fdata);
int f_eval_regex(lf_obj_handle_t ohandle, int numout, lf_obj_handle_t *ohandles,
        void *fdata);

#ifdef __cplusplus
}
#endif
