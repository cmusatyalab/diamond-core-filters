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

#ifndef _FACTORY_H_
#define _FACTORY_H_	1

#include "img_search.h"

typedef struct factory_map {
	struct factory_map  *	fm_next;
 	img_factory  *		fm_factory;
} factory_map_t;


diamond_public
void factory_register(img_factory *factory);

diamond_public
void factory_register_codec(img_factory *factory);

img_factory * find_factory(const char *name);
img_factory * find_codec_factory(const char *name);
img_factory * get_first_factory(void **cookie);
img_factory * get_first_codec_factory(void **cookie);
img_factory * get_next_factory(void **cookie);

#endif	/* ! _FACTORY_H_ */

