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

#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>		/* dirname */
#include <assert.h>
#include <stdint.h>
#include <signal.h>


#include "lib_searchlet.h"
#include "gui_thread.h"

#include "lib_filter.h"
#include "lib_log.h"

#include "queue.h"
#include "ring.h"
#include "rtimer.h"

#include "lib_results.h"
#include "rgb.h"
#include "lib_sfimage.h"
#include "img_search.h"
#include "search_support.h"
#include "import_sample.h"
#include "search_set.h"
#include "snapfind.h"
#include "plugin.h"

typedef struct slib_entry {
	char *			slib_name;
	struct slib_entry *	next;
} slib_entry_t;


static slib_entry_t *slib_ents = NULL; 

void
register_searchlet_lib(char *lib_name)
{
	slib_entry_t	*new_ent;

	new_ent = (slib_entry_t *)malloc(sizeof(*new_ent));
	if (new_ent == NULL) {
		fprintf(stderr, "Unable to allocate memory \n");
		exit(1);
	}
	new_ent->slib_name = strdup(lib_name);
	new_ent->next = slib_ents;

	slib_ents= new_ent;
}


char *
first_searchlet_lib(void ** cookie)
{
	char *	name;

	if (slib_ents == NULL) {
		return(NULL);
	}

	*cookie = (void *)slib_ents->next;
	name = strdup(slib_ents->slib_name);
	return(name);
}

char *
next_searchlet_lib(void **cookie)
{
	char *	name;
	slib_entry_t	*new_ent;

	new_ent = (slib_entry_t *)*cookie;
	if (new_ent == NULL) {
		return(NULL);
	}

	*cookie = (void *)new_ent->next;
	name = strdup(new_ent->slib_name);
	return(name);
}


/*
 * test file name to see if it is an search config file by
 * looking at it's extension.
 */ 
int
is_conf_file(char *fname)
{
	char *cp;

	cp = rindex(fname, '.');
	if (cp == NULL) {
		return(0);
	}

	if (strcmp(cp, SNAPFIND_CONF_EXTENSION) == 0) {
		return(1);
	} else {
		return(0);
	}
}

/*
 * Load all the plugins in the data directory
 */

void
load_plugins()
{
	char *	plugin_path;
	struct dirent  *cur_ent;
	DIR *	dir;


	plugin_path = sfconf_get_plugin_dir();
	if (plugin_path == NULL) {
		return;
	}

	dir = opendir(plugin_path);
        if (dir == NULL) {
                fprintf(stderr, "unable to open confdir <%s> ", plugin_path);
		perror(":");
                return;
        }

        while ((cur_ent = readdir(dir)) != NULL) {

		/* If this isn't a file then we skip the entry.  */
                if ((cur_ent->d_type != DT_REG)) {
                        continue;
                }
                                                                                
		if (is_conf_file(cur_ent->d_name) == 0) {
                        continue;

		}

		process_plugin_conf(plugin_path, cur_ent->d_name);

	}
}

