%option nounput
%option prefix="attr_map"
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


#include <pthread.h>
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
#include "attr_decode.h"
#include "decoder.h"


static void have_mapping(char *input);
static void have_error();


static char  attr_name[SF_MAX_PATH];
static char  attr_decode[SF_MAX_PATH];

static char *amap_name = NULL;
static char *amap_name_tmp = NULL;



%}
%array
NAME	\"[^\"]+\"
VALUE	\"[^\"]*\"
WS	[ \t]*

%%
^#.*\n						{}
\n					
{WS}{NAME}{WS}={WS}{VALUE}.*\n			{ have_mapping(yytext);   }
.						{ have_error(); }
%%


static void 
have_error()
{
	fprintf(stderr, "unknown text %s\n", yytext);
}


int
extract_name_value(char *input, char *name, char *value)
{
	int	inlen = strlen(input);
	int	offset = 0;
	int	soffset = 0;

	/* remove leading data before the " */
	while(input[offset] != '"') {
		offset++;
		if (offset >= inlen) return(1);
	}

	/* skip the " */
	offset++;

	/* get the rest of the string */
	soffset = 0;
	while (input[offset] != '"') {
		name[soffset++] = input[offset++];
		if (offset >= inlen) return(1);
		if (soffset >= SF_MAX_PATH) {
			fprintf(stderr, "Data too large for storage \n");
			return(1);
		}
	}
	name[soffset] = '\0';

	/* skip the " */
	offset++;
	
	/* look for = or @ */
	while ((input[offset] != '=') && (input[offset] != '@')) {
		offset++;
		if (offset >= inlen) return(1);
	}
	/* skip the key value */
	offset++;
	
	/* remove leading data before the " */
	while(input[offset] != '"') {
		offset++;
		if (offset >= inlen) return(1);
	}

	/* skip the " */
	offset++;
	
	/* get the rest of the string */
	soffset = 0;
	while (input[offset] != '"') {
		value[soffset++] = input[offset++];
		if (offset >= inlen) return(1);
		if (soffset >= SF_MAX_PATH) {
			fprintf(stderr, "Data too large for storage \n");
			return(1);
		}
	}
	value[soffset] = '\0';
	return(0);
}



static void
have_mapping(char *input)
{
	int		err;
	err = extract_name_value(input, attr_name, attr_decode);
	if (err) {
		fprintf(stderr, "Failed to parse string <%s> \n", input);
		return;
	}
	add_decode_hint(attr_name, attr_decode);
}



int attr_mapwrap() {
	return(1);
}

/*
 * This looks for the config file in a couple of places.
 * the first place is in $DIAMOND_CONFIG.  Next it looks in
 * the local directory, and third it looks in the system defined
 * location.
 */

static FILE *
open_attr_map()
{
	FILE *  new_file;
	char *  lkup;
	int	len;
	char	fname[SF_MAX_PATH];
	char	tmpfname[SF_MAX_PATH];

	/* try the environment variable */
	lkup = getenv(SNAPFIND_ENV_NAME);
	if (lkup != NULL) {
		len = snprintf(fname, SF_MAX_PATH, "%s/%s", lkup,
			SNAPFIND_RC);
		if (len >= SF_MAX_PATH) {
			fprintf(stderr, "open_config: Exceeded SF_MAX_PATH\n");
			exit(1);
		}
		new_file = fopen(fname, "a+");
		if (new_file != NULL) {
			len = snprintf(tmpfname, SF_MAX_PATH, "%s.%s", fname,
				"tmp");
			if (len >= (SF_MAX_PATH - 1)) {
				fprintf(stderr, "path exceeds SF_MAX_PATH\n");
				exit(1);
			}
			/* save file names for later use */
			amap_name = strdup(fname);
			assert(amap_name != NULL);
			amap_name_tmp = strdup(tmpfname);
			assert(amap_name != NULL);
			return(new_file);
		}
	}

	/* try the user's home directory */
	lkup = getenv("HOME");
	if (lkup != NULL) {
		len = snprintf(fname, SF_MAX_PATH, "%s/%s/%s", lkup,
			SNAPFIND_CONF_DEFAULT, SNAPFIND_AMAP);
		if (len >= (SF_MAX_PATH - 1)) {
			fprintf(stderr, "open_config: Exceeded SF_MAX_PATH\n");
			exit(1);
		}

		new_file = fopen(fname, "a+");
		if (new_file != NULL) {
			len = snprintf(tmpfname, SF_MAX_PATH, "%s.%s", fname,
				"tmp");
			if (len >= (SF_MAX_PATH - 1)) {
				fprintf(stderr, "path exceeds SF_MAX_PATH\n");
				exit(1);
			}
			/* save file names for later use */
			amap_name = strdup(fname);
			assert(amap_name != NULL);
			amap_name_tmp = strdup(tmpfname);
			assert(amap_name != NULL);
			return(new_file);
		}
	}
	return(NULL);
}


void
load_attr_map()
{
	int ret;

	yyin = open_attr_map();

	/* if no file then exit */
	if (yyin == NULL) {
		return;
	}

	ret = yylex();
	if (ret) {
		fprintf(stderr, "Problems during lex\n");
	}

	fclose(yyin);

}

void
save_attr_map()
{
	FILE *  new_file;
	int	err;

	new_file = fopen(amap_name_tmp, "w+");
	if (new_file == NULL) {
		fprintf(stderr, "save_attr_map: Unable to open %s \n", 
		    amap_name_tmp);
		return;
	}

	write_decodes(new_file);

	err = fclose(new_file);
	if (err) {
		fprintf(stderr, "save_attr_map: failed closing %s \n", 
		    amap_name_tmp);
		return;
	}

	err = rename(amap_name_tmp, amap_name);
	if (err) {
		fprintf(stderr, "save_attr_map: failed during rename\n");
		return;
	}


}
