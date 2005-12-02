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

#ifndef	_ATTR_ENT_H_
#define	_ATTR_ENT_H_	1

#include <iostream>
#include <list>
#include <vector>
#include "attr_ent.h"

#define	DUMP_WIDTH	60
#define	MAX_DISP_ATTR	40

using namespace	std;

enum fmt_types_t {
        FORMAT_TYPE_STRING = 0,
        FORMAT_TYPE_HEX,
        FORMAT_TYPE_INT
};

class attr_ent {
public:
        attr_ent(const char *name, void *data, size_t dlen);
        virtual ~attr_ent();

	const char *	get_name() const;
	const char *	get_dstring() const;
	int		get_len();
	
	GtkWidget *	get_name_widget();
	GtkWidget *	get_data_widget();
	GtkWidget *	get_len_widget();

	GtkWidget *	get_type_widget();
	void 		update_type();


        attr_ent &operator=(const attr_ent &rhs);
        int operator==(const attr_ent &rhs) const;
        int operator<(const attr_ent &rhs) const;

private:
        char   		display_name[MAX_DISP_ATTR];
        char 		display_data[DUMP_WIDTH];
	void *		rawdata;
	int		data_size;
	GtkWidget *	data_label;
	GtkWidget *	type_menu;
	fmt_types_t	fmt_type;
};



#endif	/* !_ATTR_ENT_H_ */
