/*
 *  Diamond Core Filters - collected filters for the Diamond platform
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_FIL_NUM_ATTR_H_
#define	_FIL_NUM_ATTR_H_	1


typedef struct {
	char *		attr_name;
	float	 	max_value;
	float	 	min_value;
	int	 	drop_missing;
} fnum_attr_t;

typedef struct {
	int		num_configs;
	fnum_attr_t *	fconfigs;
} fnum_attr_holder_t;

#endif	/* !_FIL_NUM_ATTR_H_ */
