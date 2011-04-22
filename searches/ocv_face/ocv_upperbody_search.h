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

#ifndef	_OCV_UPPERBODY_SEARCH_H_
#define	_OCV_UPPERBODY_SEARCH_H_	1

#include "snapfind_consts.h"
#include "ocv_search.h"


class ocv_upperbody_search: public ocv_search {
public:
	ocv_upperbody_search(const char *name, const char *descr);
	virtual ~ocv_upperbody_search();

	void	write_config(FILE* stream, const char *data_dir);
	void    write_fspec(FILE* stream);
	bool	is_editable(void);
};

class ocv_upperbody_factory: public ocv_factory {
public:
	ocv_upperbody_factory() {
		set_name("Body (upper)");
		set_description("ocv_upperbody_search");
	}

	img_search *create(const char *name) {
		return new ocv_upperbody_search(name, "Body (upper)");
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

#endif	/* !_OCV_UPPERBODY_SEARCH_H_ */
