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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>
#include <libgen.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>

#include "searchlet_api.h"
#include "lib_results.h"
#include "sf_consts.h"
#include "ring.h"
#include "face_search.h"
#include "search_set.h"
#include "image_tools.h"
#include "sfind_search.h"

extern pthread_mutex_t ring_mutex;
/*
 * this need to be global because we share it with the main
 * GUI.  XXX hack, do it right later.
 */
ls_search_handle_t shandle;

/*
 * state the is global to all of these functions.
 */
/* XXX global for status window, fix  this someday */
int      search_active = 0;
static int     active  = 0;
int      search_number = 0;
struct timeval	search_start;

static search_set *sset = NULL;



/*
 * This initializes the search  state.
 */

void
init_search()
{

	shandle = ls_init_search();
	if (shandle == NULL) {
		printf("failed to initialize:  !! \n");
		exit(1);
	}
}

void
set_searchlist(int n, groupid_t * gids)
{
	int             err;

	err = ls_set_searchlist(shandle, n, gids);
	if (err) {
		printf("Failed to set searchlist on  err %d \n", err);
		exit(1);
	}
}


/*
 * This function initiates the search by building a filter
 * specification, setting the searchlet and then starting the search.
 */
void
do_search(gid_list_t * main_region, char *fspec)
{
	int             err;
	char           *filter_name;
	char           *dir_name;
	char           *res;
	int             len;

	if (!fspec) {
		fspec = sset->build_filter_spec(NULL);
		if (fspec == NULL) {
			printf("failed to build filter specification \n");
			exit(1);
		}
	}

	filter_name = (char *) malloc(SF_MAX_PATH);
	if (filter_name == NULL) {
		exit(1);                /* XXX */
	}
	dir_name = (char *) malloc(SF_MAX_PATH);
	if (dir_name == NULL) {
		exit(1);                /* XXX */
	}

	res = getcwd(dir_name, SF_MAX_PATH);
	if (res == NULL) {
		exit(1);
	}

	len = snprintf(filter_name, SF_MAX_PATH, "%s/%s", dir_name,
	               SEARCHLET_OBJ_NAME);
	if (len >= SF_MAX_PATH) {
		fprintf(stderr, "SF_MAX_PATH is too small, please extend \n");
		assert(0);
	}

	set_searchlist(main_region->ngids, main_region->gids);

	err = ls_set_searchlet(shandle, DEV_ISA_IA32, filter_name, fspec);
	if (err) {
		printf("Failed to set searchlet on err %d \n", err);
		exit(1);
	}

	/* XXX current ugly hack */
	err = ls_add_filter_file(shandle, DEV_ISA_IA32,
	"./fil_gabor_texture.so");
	if (err) {
		printf("Failed to set searchlet on err %d \n", err);
		exit(1);
	}
	err = ls_add_filter_file(shandle, DEV_ISA_IA32,
	"./fil_regex.so");
	if (err) {
		printf("Failed to set searchlet on err %d \n", err);
		exit(1);
	}

	err = ls_add_filter_file(shandle, DEV_ISA_IA32, "./fil_ocv_face.so");
	if (err) {
		printf("Failed to set searchlet on err %d \n", err);
		exit(1);
	}

	/*
	 * Go ahead and start the search.
	 */
	err = ls_start_search(shandle);
	if (err) {
		printf("Failed to start search on err %d \n", err);
		exit(1);
	}
}

/*
 *  Stops a currently executing search.
 */
static void
stop_search(void *data)
{
	int             err;
	err = ls_terminate_search(shandle);
	if (err != 0) {
		printf("XXX failed to terminate search \n");
		exit(1);
	}
}


void
drain_ring(ring_data_t * ring)
{
	while (!ring_empty(ring)) {
		void           *msg = ring_deq(ring);
		free(msg);
	}
	assert(ring_empty(ring));

}


/*
 * This processes a message (command) from the interface
 * GUI to control the processing.
 */

void
handle_message(message_t * new_message)
{
	struct timezone tz;
	int				err;

	switch (new_message->type) {
		case START_SEARCH:

			assert(ring_empty(from_search_thread));
			/*
			 * delete old msgs 
			 */
			pthread_mutex_lock(&ring_mutex);
			drain_ring(from_search_thread);
			pthread_mutex_unlock(&ring_mutex);
			do_search((gid_list_t *) new_message->data, NULL);
			active = 1;
			search_active = 1;
			search_number++;
			err = gettimeofday(&search_start, &tz);
			assert(err == 0);
			break;

		case TERM_SEARCH:
			stop_search(new_message->data);
			search_active = 0;
			break;

			/*
			 * XXX get stats ??? 
			 */

		default:
			printf("XXX unknown message %d \n", new_message->type);
			assert(0);
			break;
	}
}



void
update_stats()
{
	int             num_dev;
	ls_dev_handle_t dev_list[SF_MAX_DEVICES];
	int             i, err, len;
	dev_stats_t    *dstats;
	int             tobj = 0, sobj = 0, dobj = 0;

	dstats = (dev_stats_t *) malloc(DEV_STATS_SIZE(SF_MAX_FILTERS));
	assert(dstats);

	num_dev = SF_MAX_DEVICES;

	err = ls_get_dev_list(shandle, dev_list, &num_dev);
	if (err != 0) {
		printf("update states: %d \n", err);
		exit(1);
	}

	for (i = 0; i < num_dev; i++) {
		len = DEV_STATS_SIZE(SF_MAX_FILTERS);
		err = ls_get_dev_stats(shandle, dev_list[i], dstats, &len);
		if (err) {
			printf("Failed to get dev stats \n");
			exit(1);
		}
		tobj += dstats->ds_objs_total;
		sobj += dstats->ds_objs_processed;
		dobj += dstats->ds_objs_dropped;
	}
	free(dstats);

	tobj_cnt = tobj;
	sobj_cnt = sobj;
	dobj_cnt = dobj;
}


/*
 * This is main function that is called thread interacting with the
 * search subsystem.  
 */

void           *
sfind_search_main(void *foo)
{

	message_t      *message;
	ls_obj_handle_t cur_obj;
	int             err;
	int             temp;

	struct timespec timeout;

	sset = (search_set *)foo;

	/*
	 * XXX init_search(); 
	 */

	while (1) {
		message = (message_t *) ring_deq(to_search_thread);
		if (message != NULL) {
			handle_message(message);
			free(message);
		}
		if ((active) && (ring_count(from_search_thread) < 7)) {
			err = ls_next_object(shandle, &cur_obj, LSEARCH_NO_BLOCK);
			if (err == EWOULDBLOCK) {
				/*
				 * no data is available, sleep for a small amount of time. 
				 */
				timeout.tv_sec = 0;
				timeout.tv_nsec = 10000000; /* XXX 10ms ?? */
				nanosleep(&timeout, NULL);
			} else if (err == ENOENT) {
				fprintf(stderr, "No more objects \n");  /* XXX */
				search_active = 0;
				active = 0;
				message = (message_t *) malloc(sizeof(*message));
				if (message == NULL) {
					fprintf(stderr, "non mem\n");
					exit(1);
				}
				message->type = DONE_OBJECTS;
				message->data = NULL;

				pthread_mutex_lock(&ring_mutex);
				err = ring_enq(from_search_thread, message);
				pthread_mutex_unlock(&ring_mutex);
				if (err) {
					fprintf(stderr, "XXX failed to enq \n");
					exit(1);
				}

				timeout.tv_sec = 0;
				timeout.tv_nsec = 100000000;    /* XXX 100ms ?? */
				nanosleep(&timeout, NULL);

			} else if (err != 0) {
				/*
				 * XXX 
				 */
				fprintf(stderr, "get_next_obj: failed on %d \n", err);
				exit(1);
			} else {
				message = (message_t *) malloc(sizeof(*message));
				assert(message != NULL);

				message->type = NEXT_OBJECT;
				message->data = cur_obj;
				pthread_mutex_lock(&ring_mutex);
				err = ring_enq(from_search_thread, message);
				pthread_mutex_unlock(&ring_mutex);
				if (err) {
					/*
					 * drop this on the floor 
					 */
					ls_release_object(shandle, cur_obj);
					free(message);
				}
			}
		} else {
			timeout.tv_sec = 0;
			timeout.tv_nsec = 100000000;    /* XXX 100ms ?? */
			nanosleep(&timeout, NULL);
		}
		err = ls_num_objects(shandle, &temp);
		temp += ring_count(from_search_thread);

		pend_obj_cnt = temp;
		update_stats();
	}
	return (0);
}
