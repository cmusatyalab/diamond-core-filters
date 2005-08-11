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

#ifndef _SNAP_FIND_H_
#define _SNAP_FIND_H_	1

#include "sfind_tools.h"	/* XXX can I remove this ??? */
static const int THUMBSIZE_X = 200;
static const int THUMBSIZE_Y = 150;

/* this is a misnomer; the thumbnail keeps everything we want to know
 * about an image */
typedef struct thumbnail_t
{
	RGBImage 	*img;	/* the thumbnail image */
	GtkWidget 	*viewport; /* the viewport that contains the image */
	GtkWidget 	*gimage; /* the image widget */
	TAILQ_ENTRY(thumbnail_t) link;
	char 		name[COMMON_MAX_NAME];		/* name of image */
	char 		device[COMMON_MAX_NAME];	/* name of device */
	int		nboxes;	/* number of histo regions */
	int             nfaces;	/* number of faces */
	image_hooks_t   *hooks;
	int              marked; /* if the user 'marked' this thumbnail */
	GtkWidget       *frame;
} thumbnail_t;

typedef TAILQ_HEAD(thumblist_t, thumbnail_t) thumblist_t;
extern thumblist_t thumbnails;
extern thumbnail_t *cur_thumbnail;

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif


#endif	/* ! _SNAP_FIND_H_ */
