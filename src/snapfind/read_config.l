%{
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "queue.h"
#include "fil_histo.h"
#include "snap_search.h"

static fsp_histo_t *vs = NULL;

static char *eat_token(char *);
static void new_search(char *);
static void error();

/* #define VERBOSE 1 */

static snap_search *cur_search = NULL;

static snap_search **sslist;
static int		     num_ss = 0;
static int		     max_ss = 64;	/* XXX fix this, should be MAX_SEARCHES */

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
PATCHFILE[ \t]+[[:alnum:]_"."-]+[ \t]+[0-9]+[ \t]+[0-9]+[ \t]+[0-9]+[ \t]+[0-9]+ { cur_search->handle_config(PATCHFILE_TOK, eat_token(yytext));}
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

static void
new_search(char *str)
{
	char *name;
	char *type;
	int		i, maxlen;

	printf("XXX new search !! %s \n", str);

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

	printf("search <name %s> <type %s> \n", name, type);

	if (num_ss >= max_ss) {
		printf("too many searches \n");
		exit(1);
	}

	if (strcmp(type, "rgb_histogram") == 0) {
		cur_search = new rgb_histo_search(name, "RGB Histogram");
	} else {
		printf("uknown type: %s \n", type);
		exit(1);
	}

	sslist[num_ss] = cur_search;
	num_ss++;
}


static void
patchfile() 
{
	char path[129];
	int x, y;
	patch_t *patch;
	texture_features_t *texture_features;


	sscanf(yytext, "%128s%d%d", path, &x, &y);
#ifdef VERBOSE
	printf("%s %d %d\n", path, x, y);
#endif

	vs->npatches++;

	if (vs->type == FILTER_TYPE_COLOR) {
		patch = (patch_t *)malloc(sizeof(patch_t));
		assert(patch);
		strncpy(patch->name, path, MAX_PATH);
		patch->minx = x;
		patch->miny = y;

		TAILQ_INSERT_TAIL(&vs->patchlist, patch, link);
	} else if (vs->type == FILTER_TYPE_TEXTURE) {
		texture_features = (texture_features_t *)
				malloc(sizeof(texture_features_t));
        assert(texture_features);
		strncpy(texture_features->name, path, MAX_PATH);
		texture_features->minx = x;
		texture_features->miny = y;
		TAILQ_INSERT_TAIL(&vs->texture_features_list, texture_features, link);
	}
}

static void
arena_file() {
        char path[129];
        char sign[129];
        int positive = 0;
        int x, y;
        arena_training_t *arena_training;

	sscanf(yytext, "%128s%128s%d%d", path, sign, &x, &y);
	
	vs->npatches++;
        if (vs->type == FILTER_TYPE_ARENA) {

        arena_training = (arena_training_t *)malloc(sizeof(arena_training_t));
        assert(arena_training);
	strncpy(arena_training->name, path, MAX_PATH);
	if (strcmp("POSITIVE", sign) == 0)
	    positive = 1;
	if (strcmp("NEGATIVE", sign) == 0)
	    positive = 0;
	arena_training->positive = positive;
	arena_training->minx = x;
	arena_training->miny = y;

	TAILQ_INSERT_TAIL(&vs->arena_training_list, arena_training, link);
   }    
}


/*
 * Read the search configuration file.
 */

int
read_search_config(char *fname, snap_search **list, int *num)
{
	int err;

	num_ss = 0;
	sslist = list;

	yyin = fopen(fname, "r");
	if(!yyin) {
		perror(fname);
		return 1;
	}
	err = yylex();

	*num = num_ss;

	return(0);
}
