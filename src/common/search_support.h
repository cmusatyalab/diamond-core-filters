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

#ifndef	_SEARCH_SUPPORT_H_
#define	_SEARCH_SUPPORT_H_	1

#include <stdint.h>
#include "face.h"
#include "rgb.h"
#include "fil_histo.h"
#include <opencv/cv.h>

#ifdef __cplusplus
extern "C" {
#endif

/* external declaration to create new search  of specified type */
img_search * create_search(search_types_t type, const char *name);

int search_exists(const char *name, search_set *set);
void search_add_list(img_search *new_search, search_set *set);


	
#ifdef __cplusplus
}
#endif

#endif	/* ! _SEARCH_SUPPORT_H_ */
