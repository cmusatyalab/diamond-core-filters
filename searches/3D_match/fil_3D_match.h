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

#ifndef	_FIL_3D_MATCH_H_
#define _FIL_3D_MATCH_H_

#include <stdint.h>
#include "lib_filter.h"
#include "snapfind_consts.h"
#include <sys/queue.h>

#define NUM_FEATURES 10
#define FEATURE_LENGTH 32

typedef struct match_config
{
  char *  name;       /* name of this search */
  float   distance;   /* threshold on quality of match */
  float   features[NUM_FEATURES];  /* features computed on object */

} match_config_t;


#ifdef __cplusplus
extern "C"
{
#endif

int isNeighbor (float th, float *q, float *f, int size);

#ifdef __cplusplus
}
#endif

#endif	/* !defined _FIL_3D_MATCH_H_ */
