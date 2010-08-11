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

#ifndef	_ATTR_INFO_H_
#define	_ATTR_INFO_H_	1

#include <iostream>
#include <list>
#include <vector>
#include "lib_searchlet.h"
#include "attr_ent.h"

using namespace	std;


typedef  list<attr_ent *>::iterator	attr_iter_t;


class attr_info
{
public:
	attr_info();
	virtual 	~attr_info();

	void		update_obj(ls_obj_handle_t ohandle);
	void		flush_display();
	int		num_attrs();

	GtkWidget * 	get_display();

	char *          create_string();

private:
	GtkWidget *	get_table();

	list<attr_ent *>	attr_list;
	int			active_display;	
	GtkWidget * 		display_table;
	GtkWidget * 		container;
};


#endif	/* !_ATTR_INFO_H_ */
