%option nounput
%{
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "queue.h"
#include "fil_histo.h"
#include "img_search.h"
#include "search_support.h"
#include "search_set.h"
#include "read_config.h"

static char *eat_token(char *tok);
static void new_search(char *tok);
static void error();

/* #define VERBOSE 1 */

static img_search *cur_search = NULL;
static search_set *set = NULL;

typedef struct factory_map {
	struct factory_map  *	fm_next;
	char * 					fm_name;
 	img_factory  *			fm_factory;
} factory_map_t;

static factory_map_t * fmap = NULL;
%}


	
%option noyywrap

FLOAT (([0-9]*"."[0-9]+)|([0-9]+))
FNAME (([0-9]*"."[0-9]+)|([0-9]+))

%%
#.*\n
SEARCH[ \t]+[[:alnum:]_]+[ \t]+[[:alnum:]_]+ {new_search(eat_token(yytext));}
TESTX[ \t]+[0-9]+ { cur_search->handle_config(TESTX_TOK, eat_token(yytext));}
TESTY[ \t]+[0-9]+ { cur_search->handle_config(TESTY_TOK, eat_token(yytext));}
STRIDE[ \t]+[0-9]+ { cur_search->handle_config(STRIDE_TOK, eat_token(yytext));}
SCALE[ \t]+{FLOAT} { cur_search->handle_config(SCALE_TOK, eat_token(yytext));}
METRIC[ \t]+{FLOAT} { cur_search->handle_config(METRIC_TOK, eat_token(yytext));}
MATCHES[ \t]+[0-9]+ { cur_search->handle_config(MATCHES_TOK, eat_token(yytext));}
CHANNEL[ \t]+[0-9]+ { cur_search->handle_config(CHANNEL_TOK, eat_token(yytext));}
NUMFACE[ \t]+[0-9]+ { cur_search->handle_config(NUMF_TOK, eat_token(yytext));}
START[ \t]+[0-9]+ { cur_search->handle_config(START_TOK, eat_token(yytext));}
END[ \t]+[0-9]+ { cur_search->handle_config(END_TOK, eat_token(yytext));}
MERGE[ \t]+[0-9]+ { cur_search->handle_config(MERGE_TOK, eat_token(yytext));}
SUPPORT[ \t]+[0-9]+ { cur_search->handle_config(SUPPORT_TOK, eat_token(yytext));}
OVERLAP[ \t]+{FLOAT} { cur_search->handle_config(OVERLAP_TOK, eat_token(yytext));}
METHOD[ \t]+{FLOAT} { cur_search->handle_config(METHOD_TOK, eat_token(yytext));}
PATCHFILE[ \t]+[[:alnum:]_"."-/]+[ \t]+[0-9]+[ \t]+[0-9]+[ \t]+[0-9]+[ \t]+[0-9]+ { cur_search->handle_config(PATCHFILE_TOK, eat_token(yytext));}
[[:blank:]]
\n
.					{ error(); }
%%


static void
error() 
{
	fprintf(stderr, "unknown token: %s\n", yytext);
	exit(1);
}


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

void
read_config_register(const char *string,  img_factory *factory)
{

	factory_map_t *new_map;

	new_map = (factory_map_t *)malloc(sizeof(*new_map));
	assert(new_map != NULL);

	new_map->fm_name = strdup(string);
	assert(new_map->fm_name != NULL);
	new_map->fm_factory = factory;

	/* XXX find dups ?? */

	new_map->fm_next = fmap;
	fmap = new_map;

}

static img_factory *
find_factory(const char *name)
{
	factory_map_t *cur_map;

	cur_map = fmap;
	for (cur_map = fmap; cur_map != NULL; cur_map = cur_map->fm_next) {
		if (strcmp(name, cur_map->fm_name) == 0) {
			return(cur_map->fm_factory);
		}
	}
	return(NULL);
}


static void
new_search(char *str)
{
	char *name;
	char *type;
	int		i, maxlen;
	img_factory *	fac;

	maxlen = strlen(str) + 1;
	for (i=0; i < maxlen; i++) {
		if (isspace(str[i]) || (str[i] == '\0')) {
			break;
		}
	}
	if (i > maxlen) {
		printf("no end of string \n");
		assert(0);
	}
	type = (char *)malloc(i + 1);
	assert(type != NULL);

	strncpy(type, str, i);
	type[i] = '\0';
	name = eat_token(str);		


	fac = find_factory(type);
	/* XXX more graceful way to handle */
	assert(fac != NULL);

	cur_search = fac->create(name);	
    assert(cur_search != NULL);

	set->add_search(cur_search);
}




/*
 * Read the search configuration file.
 */

int
read_search_config(char *fname, search_set *read_set)
{
	int err;

	set = read_set;

	yyin = fopen(fname, "r");
	if(!yyin) {
		perror(fname);
		return 1;
	}
	err = yylex();

	return(0);
}
