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

#ifndef	_FIL_TEXT_ATTR_H_
#define	_FIL_TEXT_ATTR_H_	1


typedef struct {
	char *		attr_name;
	char *		string;
	int		exact_match;
	int		drop_missing;	
	regex_t		regex;
} fdata_text_attr_t;

#endif	/* !_FIL_TEXT_ATTR_H_ */
