/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2008      Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_THUMBNAILER_H_
#define	_THUMBNAILER_H_

#include <gtk/gtk.h>
#include "img_search.h"

class thumbnailer: public img_search {
public:
	thumbnailer(void);
	~thumbnailer(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	bool	is_editable(void);
	virtual	int	handle_config(int num_conf, char **datav);

private:
	unsigned int width;
	unsigned int height;
};

#endif	/* !_THUMBNAILER_H_ */

