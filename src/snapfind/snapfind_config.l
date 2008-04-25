%option nounput
%option noyywrap
%option prefix="sfconf"
%{
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

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>
#include <netdb.h>
#include <assert.h>


#include "snapfind_consts.h"

/*
 * This file parses the snapfind RC file to find the local
 * configuration information.
 */

static int  have_error();
static int  set_plugin_dir(char *name);

static char * eat_token(char *str);
static void do_init();

typedef struct	snapfind_conf {
	char *		sf_plugin_dir;
} snapfind_conf_t;


static int err = 0;
static snapfind_conf_t *sfconf = NULL;

%}
%pointer


NAME	[a-zA-Z][a-zA-Z0-9_-]*
DIRNAME	[a-zA-Z0-9_\/\.-]+
                                                                                
FLOAT (([0-9]*"."[0-9]+)|([0-9]+))
FNAME (([0-9]*"."[0-9]+)|([0-9]+))
                                                                                
%%
#.*\n
PLUGIN_DIR[ \t]+{DIRNAME}	{set_plugin_dir(eat_token(yytext));}
.				{have_error(); }
%%

#ifndef SNAPFIND_PLUGIN_DIR
#error SNAPFIND_PLUGIN_DIR must be defined
#endif

                                                                                
static char *
eat_token(char *str)
{
        char * cur = str;
                                                                                
        while (!isspace(*cur)) {
                cur++;
        }
        while (isspace(*cur)) {
                cur++;
        }
        return(cur);
}

int
have_error()
{
	fprintf(stderr, "unknown text %s\n", yytext);
	err = 1;
	return (1);
}





/*
 * We have a new names.  We allocate a structure to keep track
 * of it.
 */
 

static int
set_plugin_dir(char *dir)
{
	sfconf->sf_plugin_dir = strdup(dir);
	return(0);
}

char *
sfconf_get_plugin_dir()
{
	char *dir;
	do_init();
	dir = strdup(sfconf->sf_plugin_dir);
	return(dir);
}

/*
 * This looks for the config file in a couple of places.
 * the first place is in $DIAMOND_CONFIG.  Next it looks in
 * the local directory, and third it looks in the system defined
 * location.
 */

FILE *
sfconfig_open_config_file()
{
	FILE *  new_file;
	char *  lkup;
	int	len;
	char	fname[SF_MAX_PATH];

	/* try the environment variable */
	lkup = getenv(SNAPFIND_ENV_NAME);
	if (lkup != NULL) {
		len = snprintf(fname, SF_MAX_PATH, "%s/%s", lkup,
			SNAPFIND_RC);
		if (len >= (SF_MAX_PATH - 1)) {
			fprintf(stderr, "open_config: Exceeded SF_MAX_PATH\n");
			exit(1);
		}
		new_file = fopen(fname, "r");
		if (new_file != NULL) {
			return(new_file);
		}
	}

	/* try the user's home directory */
	lkup = getenv("HOME");
	if (lkup != NULL) {
		len = snprintf(fname, SF_MAX_PATH, "%s/%s/%s", lkup,
			SNAPFIND_CONF_DEFAULT, SNAPFIND_RC);
		if (len >= SF_MAX_PATH) {
			fprintf(stderr, "open_config: Exceeded SF_MAX_PATH\n");
			exit(1);
		}

		new_file = fopen(fname, "r");
		if (new_file != NULL) {
			return(new_file);
		}
	}
	return(NULL);
}


static void
do_init()
{
	int ret;

	if (sfconf != NULL) {
		return;	
	}

	sfconf = malloc(sizeof(*sfconf));
	assert(sfconf != NULL);

	yyin = sfconfig_open_config_file();
	if (yyin == NULL) {
		fprintf(stderr, "open_config: No snapfindrc file found, using default \n");
		set_plugin_dir(SNAPFIND_PLUGIN_DIR);
	} else {
		ret = sfconflex();
		if (ret) {
			free(sfconf);
			sfconf = NULL;
			exit(1);
		}
	}
	return;
}
