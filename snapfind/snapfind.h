/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _SNAP_FIND_H_
#define _SNAP_FIND_H_	1

#include "snapfind_config.h"
#include "snapfind_consts.h"

img_patches_t * get_patches(lf_obj_handle_t ohandle, char *fname);

void print_key_value(const char *key, bool value);
void print_key_value(const char *key, double value);
void print_key_value(const char *key, int value);
void print_key_value(const char *key, const char *value);
void print_key_value(const char *key, int value_len, void *value);

#endif	/* ! _SNAP_FIND_H_ */
