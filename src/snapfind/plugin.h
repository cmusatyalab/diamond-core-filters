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

#ifndef _PLUGIN_H_
#define _PLUGIN_H_	1


#ifdef __cplusplus
extern "C"
{
#endif

/* from plugin.cc */
void load_plugins();
char * first_searchlet_lib(void ** cookie);
char * next_searchlet_lib(void **cookie);
void register_searchlet_lib(char *lib_name);


/* from plugin_conf.l */
void process_plugin_conf(char *dir, char *file);

#ifdef __cplusplus
}
#endif


#endif	/* ! _PLUGIN_H_ */
