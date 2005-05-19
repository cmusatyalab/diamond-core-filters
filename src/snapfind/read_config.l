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
static void new_config(char *yyt);

/* #define VERBOSE 1 */

#define	MAX_ARG		64
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
[[:blank:]]
\n	
.*	{ new_config(yytext);}
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
new_config(char *yyt)
{
	char *	argv[MAX_ARG];
	int	cur_arg;
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

	err = cur_search->handle_config(arg_offset, argv);

	/* XXX more graceful way to handle */
	if (err) {
		fprintf(stderr, "config not handled: <%d>\n", arg_offset);
		for (i=0; i < arg_offset; i++) {
			fprintf(stderr, " %d -> <%s> \n", i, argv[i]);
		}
	}
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
