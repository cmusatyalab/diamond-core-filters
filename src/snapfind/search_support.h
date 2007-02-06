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

#ifndef	_SEARCH_SUPPORT_H_
#define	_SEARCH_SUPPORT_H_	1

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

int search_exists(const char *name, search_set *set);
void search_add_list(img_search *new_search, search_set *set);



#ifdef __cplusplus
}
#endif

#endif	/* ! _SEARCH_SUPPORT_H_ */
