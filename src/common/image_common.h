/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_IMAGE_COMMON_H_
#define	_IMAGE_COMMON_H_

#include "queue.h"

typedef	struct bbox {
	int					min_x;
	int					min_y;
	int					max_x;
	int					max_y;
	double				distance;
	TAILQ_ENTRY(bbox)	link;
} bbox_t;

typedef	TAILQ_HEAD(bbox_list_t, bbox)	bbox_list_t;



#ifdef __cplusplus
extern "C" {
#endif

	
#ifdef __cplusplus
}
#endif
#endif	/* _IMAGE_COMMON_H_ */

