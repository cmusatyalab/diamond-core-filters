
#ifndef _SNAP_BENCH_H
#define _SNAP_BENCH_H	1

int run_config_script(char *fname);
char * load_file(char *fname, int *len);


void dump_matches(ls_obj_handle_t handle);
void dump_name(ls_obj_handle_t handle);
void dump_id(ls_obj_handle_t handle);


#endif	/* !_SNAP_BENCH_H */
