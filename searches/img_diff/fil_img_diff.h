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

#ifndef	_FIL_IMG_DIFF_H_
#define _FIL_IMG_DIFF_H_

#include <stdint.h>
#include "lib_filter.h"
#include <sys/queue.h>
#include "lib_sfimage.h"

typedef struct img_diff_config
{
  const char *fname;
  example_list_t examples;

} img_diff_config_t;

#endif	/* !defined _FIL_IMG_DIFF_H_ */
