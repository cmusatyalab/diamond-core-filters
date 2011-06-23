%option nounput
%option noyywrap
%option prefix="searchconf"
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

#include <string.h>
#include <ctype.h>
#include <assert.h> 
#include <stdlib.h> 
#include <gtk/gtk.h>
#include <sys/queue.h>
#include "lib_results.h"
#include "img_search.h"
#include "read_config.h"
#include "factory.h"

static char *eat_token(char *tok);
static void new_config(char *yyt);

/* #define VERBOSE 1 */

#define	MAX_ARG		64
static img_search *cur_search = NULL;

%}



	

FLOAT (([0-9]*"."[0-9]+)|([0-9]+))
FNAME (([0-9]*"."[0-9]+)|([0-9]+))

%%
#.*\n
SEARCH[ \t]+[[:alnum:]_]+[ \t]+[[:alnum:]_]+
[[:blank:]]
\n	
.*				{ new_config(yytext);}
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


static void
new_config(char *yyt)
{
	char *	argv[MAX_ARG];
	char *	cur_str;
	char *	arg;
	int	arg_offset;
	int	maxlen;
	int	i;
	int  	err;


	cur_str = yyt;
	arg_offset = 0;

	while ((maxlen = strlen(cur_str) + 1) > 1) {
		for (i=0; i < maxlen; i++) {
			if (isspace(cur_str[i]) || (cur_str[i] == '\0')) {
				break;
			}
		}
		if (i > maxlen) {
			printf("no end of string \n");
			assert(0);
		}
		arg = (char *)malloc(i + 1);
		assert(arg != NULL);

		strncpy(arg, cur_str, i);
		arg[i] = '\0';


		argv[arg_offset] = arg;
		arg_offset++;

		cur_str = eat_token(cur_str);
	}

	if (arg_offset == 0) {
		return;
	}
	err = cur_search->handle_config(arg_offset, argv);

	if (err) {
		fprintf(stderr, "config for %s not handled \n", 
			cur_search->get_name());
		for (i=0; i < arg_offset; i++) {
			fprintf(stderr, "<%s> ", argv[i]);
		}
		fprintf(stderr, "\n");
	}
}



int
read_search_config_for_plugin_runner(void *mem, int len, img_search *search)
{
	FILE *f = fmemopen(mem, len, "r");
	if(!f) {
		perror("fmemopen");
		return 1;
	}

	cur_search = search;
	yyin = f;
	yylex();
	fclose(f);
	return 0;
}
