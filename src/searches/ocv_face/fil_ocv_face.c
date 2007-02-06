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

/*
 * face detector 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <opencv/cvaux.h>

#include "lib_filter.h"
#include "fil_ocv_common.h"
#include "fil_ocv_face.h"


int
f_init_opencv_fdetect(int numarg, char **args, int blob_len, void *blob_data,
 			const char *fname, void **fdatap)
{
        return f_init_opencv_detect(numarg, args, blob_len, blob_data, fname, fdatap);
}


int
f_fini_opencv_fdetect(void *fdata)
{
	return f_fini_opencv_detect(fdata);
}


int
f_eval_opencv_fdetect(lf_obj_handle_t ohandle, void *fdata)
{
	return f_eval_opencv_detect(ohandle, fdata);
}


