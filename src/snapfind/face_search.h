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



typedef struct main_scape_t {
	char	name[MAX_NAME];
/*   	char 	path[MAX_PATH]; */

	fsp_histo_t fsp_info;
} main_scape_t;

/* this struct is used to send info from the GUI to the search
 * thread.  */
typedef	struct topo_region {
	int 			min_faces;
	int			face_levels;

	int			nscapes;
	main_scape_t		scapes[MAX_SCAPE];
//	int                     req_scapes;
        int                     ngids;
        groupid_t               gids[MAX_ALBUMS];
        char                    search_string[MAX_STRING];
} topo_region_t;


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
char *build_filter_spec(char *file, topo_region_t *main_region);
void do_search(topo_region_t *main_region, char *opt_filename);

#ifdef __cplusplus
}
#endif



#endif /* _FACE_SEARCH_H_ */
