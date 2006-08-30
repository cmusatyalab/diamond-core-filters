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
#include <string.h>
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
#include "lib_searchlet.h"
#include "lib_filter.h"
#include "lib_log.h"
#include "lib_results.h"
#include "ring.h"
#include "sys_attr.h"
#include "snap_bench.h"


#define	MAX_PATH	128
#define	MAX_NAME	128

/*
 * this need to be global because we share it with the main
 * GUI.  XXX hack, do it right later.
 */
extern ls_search_handle_t shandle;
int verbose;


#ifdef 	XXX	

int
load_searchlet(bench_config_t *bconfig)
{
	int             err;
	char           *full_spec;
	char           *full_obj;
	char           *path_name;
	char           *res;

	full_spec = (char *) malloc(MAX_PATH);
	if (full_spec == NULL) {
		exit(1);                /* XXX */
	}
	full_obj = (char *) malloc(MAX_PATH);
	if (full_obj == NULL) {
		exit(1);                /* XXX */
	}

	path_name = (char *) malloc(MAX_PATH);
	if (path_name == NULL) {
		exit(1);                /* XXX */
	}

	res = getcwd(path_name, MAX_PATH);
	if (res == NULL) {
		exit(1);
	}

	if (bconfig->obj_files[0][0] != '/')
		sprintf(full_obj, "%s/%s", path_name, bconfig->obj_files[0]);
	else 
		sprintf(full_obj, "%s", bconfig->obj_files[0]);

	if (bconfig->searchlet_config[0] != '/')
		sprintf(full_spec, "%s/%s", path_name, 
		    bconfig->searchlet_config);
	else
		sprintf(full_spec, "%s", bconfig->searchlet_config);


	if (verbose)
		fprintf(stdout, "load obj %s spec %s \n", full_obj, full_spec);
	err = ls_set_searchlet(shandle, DEV_ISA_IA32, full_obj, full_spec);
	if (err) {
		fprintf(stderr, "Failed to set searchlet on err %d \n", err);
		exit(1);
	}
	/* XXX do for all objs */
	return (0);
}
#endif

#define	MAX_DEVS	24
#define	MAX_FILTS	24

void
dump_dev_stats(dev_stats_t * dev_stats, int stat_len)
{
	int             i;

	fprintf(stdout, "Total objects: %d \n", dev_stats->ds_objs_total);
	fprintf(stdout, "Processed objects: %d \n", dev_stats->ds_objs_processed);
	fprintf(stdout, "Dropped objects: %d \n", dev_stats->ds_objs_dropped);
	fprintf(stdout, "System Load: %d \n", dev_stats->ds_system_load);
	fprintf(stdout, "Avg obj time: %lld \n", dev_stats->ds_avg_obj_time);

	for (i = 0; i < dev_stats->ds_num_filters; i++) {
		fprintf(stdout, "\tFilter: %s \n",
		        dev_stats->ds_filter_stats[i].fs_name);
		fprintf(stdout, "\tProcessed: %d \n",
		        dev_stats->ds_filter_stats[i].fs_objs_processed);
		fprintf(stdout, "\tDropped: %d \n",
		        dev_stats->ds_filter_stats[i].fs_objs_dropped);
		fprintf(stdout, "\tCACHE Dropped: %d \n",
		        dev_stats->ds_filter_stats[i].fs_objs_cache_dropped);
		fprintf(stdout, "\tCACHE Passed: %d \n",
		        dev_stats->ds_filter_stats[i].fs_objs_cache_passed);
		fprintf(stdout, "\tCACHE Compute: %d \n",
		        dev_stats->ds_filter_stats[i].fs_objs_compute);
		fprintf(stdout, "\tAvg Time: %lld \n\n",
		        dev_stats->ds_filter_stats[i].fs_avg_exec_time);

	}
}


char *
load_file(char *fname, int *len)
{

	int             fd;
	int             err;
	char           *buf;
	struct stat     stats;
	size_t          rsize;


	fd = open(fname, O_RDONLY);
	if (fd < 0) 
		return(NULL);
	
	err = fstat(fd, &stats);

	if (err < 0) {
		close(fd);
		return(NULL);
	}

	buf = (char *) malloc(stats.st_size);
	assert(buf != NULL);

	rsize = read(fd, buf, stats.st_size);
	if (rsize < stats.st_size) {
		free(buf);
		close(fd);
		return(NULL);
	}
	close(fd);
	*len = rsize;
	return(buf);
}

void
dump_state()
{
	int             err,
	i;
	int             dev_count;
	ls_dev_handle_t dev_list[MAX_DEVS];
	dev_stats_t    *dev_stats;
	int             stat_len;


	dev_count = MAX_DEVS;
	err = ls_get_dev_list(shandle, dev_list, &dev_count);
	if (err) {
		fprintf(stderr, "Failed to get device list %d \n", err);
		exit(1);
	}

	fprintf(stdout, "active devices %d \n", dev_count);
	dev_stats = (dev_stats_t *) malloc(DEV_STATS_SIZE(MAX_FILTS));
	assert(dev_stats != NULL);

	for (i = 0; i < dev_count; i++) {
		stat_len = DEV_STATS_SIZE(MAX_FILTS);
		err = ls_get_dev_stats(shandle, dev_list[i], dev_stats, &stat_len);
		if (err) {
			fprintf(stderr, "Failed to get device list %d \n", err);
			exit(1);
		}
		dump_dev_stats(dev_stats, stat_len);
	}

	free(dev_stats);
	fflush(stdout);

}

void
dump_device(ls_obj_handle_t handle)
{
	size_t          size;
	int             err;
	char            big_buffer[MAX_NAME];

	size = MAX_NAME;
	err = lf_read_attr(handle, DEVICE_NAME, &size,
	    (unsigned char *)big_buffer);
	if (err) {
		fprintf(stdout, "device Unknown ");
	} else {
		fprintf(stdout, "device %s ", big_buffer);
	}
}

void
dump_name(ls_obj_handle_t handle)
{
	size_t           size;
	int             err;
	int             i;
	char            big_buffer[MAX_NAME];

	size = MAX_NAME;
	err = lf_read_attr(handle, DISPLAY_NAME, &size,
	    (unsigned char *)big_buffer);
	if (err) {
		fprintf(stdout, "name Unknown ");
	} else {
		for (i = 0; i < strlen(big_buffer); i++) {
			if (big_buffer[i] == ' ') {
				big_buffer[i] = '_';
			}
		}
		fprintf(stdout, "name %s ", big_buffer);
	}
}

void
usage()
{
	fprintf(stdout, "snap_bench -c <config_file> <-v> \n");
}


#define	MAX_GROUPS	64

int
main(int argc, char **argv)
{
	int             err;
	char           *config_file = NULL;
	struct timeval  tv1, tv2;
	struct timezone tz;
	int             secs;
	int             usec;
	int             c;


	/*
	 * Parse the command line args.
	 */
	while (1) {
		c = getopt(argc, argv, "c:hv");
		if (c == -1) {
			break;
		}

		switch (c) {

			case 'c':
				config_file = optarg;
				break;

			case 'h':
				usage();
				exit(0);
				break;

			case 'v':
				verbose++;
				break;	
			default:
				fprintf(stderr, "unknown option %c\n", c);
				usage();
				exit(1);
				break;
		}
	}

	if (config_file == NULL) {
		usage();
		exit(1);
	}

			
	run_config_script(config_file);


	sleep(1);
	dump_state();

	exit(0);
}
