/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _FACE_SEARCH_H_
#define _FACE_SEARCH_H_

/* this file defines the structures exchanged between the search
 * thread and the main application. (face_search.c and facemain.c)  */

#include "ring.h"
#include "face_consts.h"
#include "face.h"

/* 
 * inherited from topo.h  -- cleanup!
 */



/* this struct is used to send info from the GUI to the search
 * thread.  */
typedef	struct gid_list {
        int                     ngids;
        groupid_t               gids[MAX_ALBUMS];
} gid_list_t;


typedef enum message_ops {
	START_SEARCH,
	TERM_SEARCH,
	NEXT_OBJECT,
	DONE_OBJECTS,
} message_ops_t;

typedef struct {
	message_ops_t	type;
	void *		data;
} message_t;



/*
 * Global data structures shared among the files.
 */
extern ring_data_t * to_search_thread;
extern ring_data_t * from_search_thread;
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
extern "C" {
#endif
                                                                               
void * sfind_search_main(void *);
char *build_filter_spec(char *file);
void do_search(gid_list_t *main_region, char *opt_filename);

#ifdef __cplusplus
}
#endif



#endif /* _FACE_SEARCH_H_ */
