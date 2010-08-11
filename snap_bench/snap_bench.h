/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */


#ifndef _SNAP_BENCH_H
#define _SNAP_BENCH_H	1

int run_config_script(char *fname);
void dump_filtstats();
char * load_file(char *fname, int *len);


void dump_name(ls_obj_handle_t handle);
void dump_device(ls_obj_handle_t handle);


#endif	/* !_SNAP_BENCH_H */
