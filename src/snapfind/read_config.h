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

#ifndef _READ_CONFIG_H_
#define _READ_CONFIG_H_	1

int read_search_config(char *fname, search_set *read_set);

void read_config_register(const char *string,  img_factory *factory);



#endif	/* ! _READ_CONFIG_H_ */
