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

#ifndef	_OCV_PROFILE_SEARCH_H_
#define	_OCV_PROFILE_SEARCH_H_	1

#include "snapfind_consts.h"
#include "ocv_search.h"


class ocv_profile_search: public ocv_search {
public:
	ocv_profile_search(const char *name, char *descr);
	virtual ~ocv_profile_search();

	void	write_config(FILE* stream, const char *data_dir);
	void    write_fspec(FILE* stream);
	int     matches_filter(char *name);
};

class ocv_profile_factory: public ocv_factory {
public:
	ocv_profile_factory() {
		set_name("Face (profile)");
		set_description("ocv_profile_search");
	}

	img_search *create(const char *name) {
		return new ocv_profile_search(name, "Face (profile)");
	}
	int is_example() {
		return(0);
	}
};



#ifdef __cplusplus
extern "C"
{
#endif

/* this needs to be provide by someone calling this library */
void ss_add_dep(img_search *dep);

#ifdef __cplusplus
}
#endif

#endif	/* !_OCV_PROFILE_SEARCH_H_ */
