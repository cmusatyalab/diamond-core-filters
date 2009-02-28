/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2008 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _SNAPFIND_CONSTS_H_
#define _SNAPFIND_CONSTS_H_	1


/* used to get buffers for computing path names */
#define	COMMON_MAX_PATH		256
#define SF_MAX_PATH 		COMMON_MAX_PATH

#define	COMMON_MAX_NAME		128
#define SF_MAX_NAME 		COMMON_MAX_NAME

/* max number of regions that can be selected in zoomed window */
#define MAX_SELECT 32

/* number of albums we support */
#define MAX_ALBUMS 32

/* presumed max number of active filters */
#define SF_MAX_FILTERS 	64

/* presumed max number of active devices */
#define SF_MAX_DEVICES 	64

/*
 * some values for the configuratino information 
 */

/* name of the snapfindrc file */
#define	SNAPFIND_RC			"snapfindrc"

/* name of the snapfindrc file */
#define	SNAPFIND_AMAP			"attr_map"

/* path relative to $HOME where we assume the SNAPFIND_RC is stored */
#define	SNAPFIND_CONF_DEFAULT		".diamond/snapfind/"

/* environmental value where SNAPFIND_RC is stored */
#define	SNAPFIND_ENV_NAME		"SNAPFIND_CONF"

/* 
 * the extension that identifies plugin directories.  This must start
 * with '.' or the matching algorithm will break.
 */
#define	SNAPFIND_CONF_EXTENSION		".sf_conf"

/* thumbnail attribute name and default size */
#define THUMBNAIL_ATTR			"thumbnail.jpeg"
#define THUMBSIZE_X			200
#define THUMBSIZE_Y			150
#define THUMBNAIL_JPEG_QUALITY          95

#endif	/* ! _SNAPFIND_CONSTS_H_ */

