%option nounput
%option noyywrap
%option prefix="pluginconf"
%{
/*
 * 	Diamond (Release 1.0)
 *      A system for interactive brute-force search
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
#include "plugin.h"

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
                printf("failed to open <%s>\n", libname);
                err = dlerror();
                printf("err: %s \n", err);
        } else {
                fp = (search_init_t) dlsym(handle, "search_init");
                if (fp == NULL) {
                        printf("Failed to find search_init\n");
                } else {
                        (*fp)();
                }
        }
}
                                                                                

 

static void
have_gui_lib(char *lib)
{
	char	fname[SF_MAX_PATH];
	int	len;

	if (lib[0] == '/') {
		load_library(lib);
	} else {
	
		len = snprintf(fname, SF_MAX_PATH, "%s/%s", cur_dir, lib);
		if (len >= (SF_MAX_PATH - 1)) {
			fprintf(stderr, 
				"plugin_conf: path exceeds SF_MAX_PATH\n");
			return;
		}
		load_library(fname);
	}
	return;
} 

static void
have_searchlet_lib(char *lib)
{
	char	fname[SF_MAX_PATH];
	int	len;

	if (lib[0] == '/') {
		register_searchlet_lib(lib);
	} else {
	
		len = snprintf(fname, SF_MAX_PATH, "%s/%s", cur_dir, lib);
		if (len >= (SF_MAX_PATH - 1)) {
			fprintf(stderr, 
				"plugin_conf: path exceeds SF_MAX_PATH\n");
			return;
		}
		register_searchlet_lib(fname);
	}
	return;
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

	fclose(yyin);
	return;
}
