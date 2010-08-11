/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _PLUGIN_RUNNER_H_
#define _PLUGIN_RUNNER_H_  1

#include <cstdlib>

void print_key_value(const char *key, bool value);
void print_key_value(const char *key, double value);
void print_key_value(const char *key, int value);
void print_key_value(const char *key, const char *value);
void print_key_value(const char *key, int value_len, void *value);

void list_plugins(void);
int get_plugin_initial_config(const char *type,
			      const char *internal_name);
int edit_plugin_config(const char *type,
		       const char *internal_name);
int normalize_plugin_config(const char *type,
			    const char *internal_name);
int run_plugin(const char *type,
	       const char *internal_name);

#endif  /* _PLUGIN_RUNNER_H_ */
