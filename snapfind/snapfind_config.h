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

#ifndef _SNAP_FIND_CONFIG_H_
#define _SNAP_FIND_CONFIG_H_	1

#include <lib_filter.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* From snapfind_config.l */
diamond_public
char * sfconf_get_plugin_dir();

#ifdef __cplusplus
}
#endif

#endif	/* ! _SNAP_FIND_CONFIG_H_ */
