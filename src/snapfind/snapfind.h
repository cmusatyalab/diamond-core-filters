/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _SNAP_FIND_H_
#define _SNAP_FIND_H_	1

#include "sfind_tools.h"	/* XXX can I remove this ??? */
#include "snapfind_config.h"

static const int THUMBSIZE_X = 200;
static const int THUMBSIZE_Y = 150;

// number of thumbnails displayed per screen
static const int THUMBNAIL_DISPLAY_SIZE = 6;  

/* this is a misnomer; the thumbnail keeps everything we want to know
 * about an image */
typedef struct thumbnail_t {
	RGBImage 	*img;	/* the thumbnail image */
	GtkWidget 	*viewport; /* the viewport that contains the image */
	GtkWidget 	*gimage; /* the image widget */
	TAILQ_ENTRY(thumbnail_t) link;
	char 		name[COMMON_MAX_NAME];		/* name of image */
	char 		device[COMMON_MAX_NAME];	/* name of device */
	ls_obj_handle_t img_obj;	/* object being veiwed */
	image_hooks_t   *hooks;
	int              marked; /* if the user 'marked' this thumbnail */
	GtkWidget       *frame;
} thumbnail_t;

typedef TAILQ_HEAD(thumblist_t, thumbnail_t) thumblist_t;
extern thumblist_t thumbnails;
extern thumbnail_t *cur_thumbnail;

/*
 * keep list of running searches.
 */
typedef struct search_name {
	char *			sn_name;
	struct search_name *	sn_next;
} search_name_t;

extern search_name_t * active_searches;

typedef struct user_stats {
	int total_seen, total_marked;
} user_stats_t;

extern user_stats_t user_measurement;

img_search *get_current_codec(void);

#ifdef __cplusplus
extern "C"
{
#endif

/* from log_win.cc */
void init_logging(void);

#ifdef __cplusplus
}
#endif


#endif	/* ! _SNAP_FIND_H_ */
