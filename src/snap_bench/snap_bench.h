
#ifndef _SNAP_BENCH_H
#define _SNAP_BENCH_H	1

int run_config_script(char *fname);
void dump_filtstats();
char * load_file(char *fname, int *len);


void dump_name(ls_obj_handle_t handle);
void dump_device(ls_obj_handle_t handle);


#endif	/* !_SNAP_BENCH_H */
