%option nounput
%option noyywrap
%option prefix="pluginconf"
%{
/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2009 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

// keep lex from printing garbage to stdout
#define ECHO

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
#include <assert.h>


#include "snapfind_consts.h"
#include "plugin.h"
#include "factory.h"

/*
 * This file parses the snapfind RC file to find the local
 * configuration information.
 */

static int  have_error();
static void  have_gui_lib(char *name);
static void  have_searchlet_lib(char *name);

static char * eat_token(char *str);


static int err = 0;
static char *cur_dir;

static char *gui_lib;
static char *searchlet_lib;

%}
%pointer


FNAME	[a-zA-Z0-9_\.\-\/]+
                                                                                
%%
\n
#.*\n
GUI_LIB[ \t]+{FNAME}		{have_gui_lib(eat_token(yytext));}
SEARCHLET_LIB[ \t]+{FNAME}	{have_searchlet_lib(eat_token(yytext));}
.				{have_error(); }
%%


                                                                                
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




typedef void (*search_init_t)();
                                                                                
static void
load_library(char *libname)
{
        void *handle;
        char *  err;
                                                                                
        search_init_t fp;
                                                                                
        handle = dlopen(libname, RTLD_NOW);
        if (handle == NULL) {
		fprintf(stderr, "failed to open <%s>\n", libname);
                err = dlerror();
                fprintf(stderr, "err: %s \n", err);
        } else {
                fp = (search_init_t) dlsym(handle, "search_init");
                if (fp == NULL) {
                        fprintf(stderr, "Failed to find search_init\n");
                } else {
                        (*fp)();
                }
        }
}
                                                                                

 


static void
have_gui_lib(char *lib)
{
	assert(gui_lib == NULL);
	gui_lib = strdup(lib);
}

static void
have_searchlet_lib(char *lib)
{
	assert(searchlet_lib == NULL);
	searchlet_lib = strdup(lib);
}


static void
commit_libs()
{
	char	fname[SF_MAX_PATH];
	int	len;

	assert(gui_lib);
	assert(searchlet_lib);

	if (searchlet_lib[0] == '/') {
		register_searchlet_lib(searchlet_lib);
		factory_set_searchlet_lib_path(searchlet_lib);
	} else {
	
		len = snprintf(fname, SF_MAX_PATH, "%s/%s", cur_dir, searchlet_lib);
		if (len >= (SF_MAX_PATH - 1)) {
			fprintf(stderr, 
				"plugin_conf: path exceeds SF_MAX_PATH\n");
			return;
		}
		register_searchlet_lib(fname);
		factory_set_searchlet_lib_path(fname);
	}

	if (gui_lib[0] == '/') {
		load_library(gui_lib);
	} else {
	
		len = snprintf(fname, SF_MAX_PATH, "%s/%s", cur_dir, gui_lib);
		if (len >= (SF_MAX_PATH - 1)) {
			fprintf(stderr, 
				"plugin_conf: path exceeds SF_MAX_PATH\n");
			return;
		}
		load_library(fname);
	}

	factory_set_searchlet_lib_path(NULL);
}



void
process_plugin_conf(char *dir, char *file)
{
	int 	ret;
	char	fname[SF_MAX_PATH];
	int	len;


	len = snprintf(fname, SF_MAX_PATH, "%s/%s", dir, file);
	if (len >= (SF_MAX_PATH - 1)) {
		fprintf(stderr, "process_plugin: path exceeds SF_MAX_PATH\n");
		return;
	}

	yyin = fopen(fname, "r");
	if (yyin == NULL) {
		fprintf(stderr, "process_plugin: unable to open <%s>\n", fname);
		perror("");
		return;
	}

	/* make the directory available to all the functions */
	cur_dir = dir;

	ret = pluginconflex();
	if (ret) {
		fprintf(stderr, "process_plugin: error processing <%s> \n", 
			fname);
	}

	/* commit */
	commit_libs();

	/* clean up names */
	free(gui_lib);
	gui_lib = NULL;
	free(searchlet_lib);
	searchlet_lib = NULL;

	fclose(yyin);
	return;
}
