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

#ifndef _FACE_SEARCH_H_
#define _FACE_SEARCH_H_

/* this file defines the structures exchanged between the search
 * thread and the main application. (face_search.c and facemain.c)  */

#include <glib.h>
#include "snapfind_consts.h"

/*
 * inherited from topo.h  -- cleanup!
 */



typedef enum message_ops {
    START_SEARCH,
    TERM_SEARCH,
    NEXT_OBJECT,
    DONE_OBJECTS,
    SET_USER_BUSY,
    SET_USER_WAITING,
	DEFINE_SCOPE,
} message_ops_t;

typedef struct
{
	message_ops_t	type;
	void *		data;
}
message_t;



/*
 * Global data structures shared among the files.
 */
extern GAsyncQueue * to_search_thread;
extern GAsyncQueue * from_search_thread;
extern int		pend_obj_cnt;
extern int		tobj_cnt;
extern int		sobj_cnt;
extern int		dobj_cnt;


/*
 * Key words for the filter spec file.
 */
#define	FILTER_DEF	"\nFILTER"
#define THRESHOLD	"THRESHOLD"
#define MERIT		"MERIT"
#define EVAL_FUNCTION	"EVAL_FUNCTION"
#define INIT_FUNCTION	"INIT_FUNCTION"
#define FINI_FUNCTION	"FINI_FUNCTION"
#define FUNCTION				"HELP"
#define ARGUMENT	"ARG"
#define REQUIRES        "REQUIRES"


#ifdef __cplusplus
extern "C"
{
#endif

void * sfind_search_main(void *);
char *build_filter_spec(char *file);
void do_search(char *opt_filename);



#ifdef __cplusplus
}
#endif



#endif /* _FACE_SEARCH_H_ */