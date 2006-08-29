%option nounput
%option noyywrap 
%option prefix="benchconfig" 
%{ 
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <assert.h> 
#include <stdlib.h> 
#include <time.h> 
#include "queue.h"
#include "lib_searchlet.h"
#include "snap_bench.h"

static char *eat_token(char *tok);
static void init_search(char *tok);
static void set_searchlist(char *tok);
static void set_searchlet(char *tok);
static void add_filterfile(char *tok);
static void start_search(char *tok);
static void term_search(char *tok);
static void set_blob(char *tok);
static void next_object(char *tok);
static void sleep_ms(char *tok);
static void have_error(char *tok);


extern int verbose;

ls_search_handle_t shandle;

%}



HEX8    [[:xdigit:]][[:xdigit:]]
HEX16   {HEX8}:{HEX8} 
HEX32   {HEX16}:{HEX16}
HEX64   {HEX32}:{HEX32}
NAME    [a-zA-Z0-9][a-zA-Z0-9_\.-]*

%%
#.*\n
ls_init_search:.*\n   				{init_search(yytext);}
ls_set_searchlist:[ \t]+id[ \t]+[[:digit:]].*\n  {set_searchlist(yytext);}
ls_set_searchlet:[ \t]+id[ \t]+[[:digit:]].*\n  {set_searchlet(yytext);}
ls_add_filter_file:[ \t]+id[ \t]+[[:digit:]].*\n  {add_filterfile(yytext);}
ls_start_search:[ \t]+id[ \t]+[[:digit:]].*\n  {start_search(yytext);}
ls_terminate_search:[ \t]+id[ \t]+[[:digit:]].*\n  {term_search(yytext);}
ls_next_object:[ \t]+id[ \t]+[[:digit:]].*\n  	{next_object(yytext);}
ls_set_blob:[ \t]+id[ \t]+[[:digit:]].*\n  	{set_blob(yytext);}
sleep[ \t]+[[:digit:]].*\n  			{sleep_ms(yytext);}
.*\n                               {have_error(yytext); }
%%


static void
have_error(char *data) 
{
	fprintf(stderr, "unknown token: %s\n", data);
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

static char *
copy_to_whitespace(char *str)
{
	char *	newstr;
	int	count = 0;
	int	len = strlen(str);

	while ((!isspace(str[count])) && (count < len) )
		count++;

	if (count == 0)
		return(NULL);

	newstr = malloc(count + 1);
	assert(newstr != NULL);

	memcpy(newstr, str, count);
	newstr[count] = '\0';

	return(newstr);
}

static void
init_search(char *data) {
	shandle = ls_init_search();
        if (shandle == NULL) {
                fprintf(stderr, "failed to initialize search \n");
                exit(1);
        }

	if (verbose)
		fprintf(stdout, "init search suceeded\n");
}

#define	MAX_GIDS	256

void
set_searchlist(char *string)
{
	int             err;
	char *		token;
	int		gid_count = 0;
	groupid_t       gid_list[MAX_GIDS];



	token = eat_token(string);
	token = eat_token(token);
	token = eat_token(token);
	token = eat_token(token);

	while (strlen(token) > 0) {
		err = sscanf(token, "%lld", &gid_list[gid_count]);
		if (err == 1)
			gid_count++;
				
		token = eat_token(token);	
	}
        err = ls_set_searchlist(shandle, gid_count, gid_list);
        if (err) {
                fprintf(stderr, "Failed to set searchlist on  err %d \n", err);
                exit(1);
        }

}

static void
set_searchlet(char *string)
{
	int             err;
	char *		token;
	char *spec, *obj;

	token = eat_token(string);
	token = eat_token(token);
	token = eat_token(token);
	token = eat_token(token);

	obj = copy_to_whitespace(token);
	token = eat_token(token);
	token = eat_token(token);
	spec = copy_to_whitespace(token);

	if (verbose)
		fprintf(stdout, "set_searchlet: obj %s spec %s \n", obj, spec);

        err = ls_set_searchlet(shandle, DEV_ISA_IA32, obj, spec);
        if (err) {
                fprintf(stderr, "Failed to set searchlet on err %d \n", err);
                exit(1);
        }
}
static void
add_filterfile(char *string)
{
	int  err;
	char *token;
	char *obj;

	token = eat_token(string);
	token = eat_token(token);
	token = eat_token(token);
	token = eat_token(token);

	obj = copy_to_whitespace(token);
	if (verbose)
		fprintf(stdout, "add_filterfile: obj %s \n", obj);

        err = ls_add_filter_file(shandle, DEV_ISA_IA32, obj);
        if (err) {
                fprintf(stderr, "Failed to set searchlet on err %d \n", err);
                exit(1);
        }
	free(obj);
}

static void
set_blob(char *string)
{
	int  err;
	char *token;
	char *fname, *fdata, *datafile;
	int   len;

	token = eat_token(string);
	token = eat_token(token);
	token = eat_token(token);
	token = eat_token(token);

	fname = copy_to_whitespace(token);
	token = eat_token(token);
	token = eat_token(token);
	datafile = copy_to_whitespace(token);

	fdata = load_file(datafile, &len);
	if (fdata == NULL) {
                fprintf(stderr, "load file %s failed \n", datafile);
                exit(1);
	}

	if (verbose)
		fprintf(stdout, "set_blob on filter %s size %d \n",
		    fname, len);

        err = ls_set_blob(shandle, fname, len, fdata);
        if (err) {
                fprintf(stderr, "Failed to set blob on err %d \n", err);
                exit(1);
        }

	free(fdata);
	free(datafile);
	free(fname);
}
void
start_search(char *string)
{
	int	err;
	err = ls_start_search(shandle);
        if (err) {
                fprintf(stderr, "Failed to start search on err %d \n", err);
                exit(1);
        }
	if (verbose)
                fprintf(stdout, "starting new search\n");
}

void
term_search(char *string)
{
	int	err;
	err = ls_terminate_search(shandle);
        if (err) {
                fprintf(stderr, "Failed to terminate search on err %d\n", err);
                exit(1);
	}
}

void
sleep_ms(char *string)
{
	char *		token;
	long		ms;
	long		remainder;
	struct timespec	ts;

	token = eat_token(string);
	ms = strtol(token, NULL, 10);
	if ((ms == 0) && (errno == EINVAL)) {
                fprintf(stderr, "Failed to convert sleep argument %s\n", token);
		exit(1);
	}

	ts.tv_sec = ms / 1000;	
	remainder = ms - (ts.tv_sec * 1000);
	ts.tv_nsec = remainder * 1000 * 1000;	/* convert to nanosecs */

	if (verbose)
		fprintf(stdout, "sleep_ms: sleeping %ld ms \n", ms);

	nanosleep(&ts, NULL);

}


void
next_object(char *string)
{
	ls_obj_handle_t	cur_obj;
	int		err;


 	err = ls_next_object(shandle, &cur_obj, 0);
	if (err == ENOENT) {
		fprintf(stderr, "get_next_obj: no more objects \n");
	} else if (err == 0) {
		dump_id(cur_obj);
		dump_name(cur_obj);
		dump_matches(cur_obj);
		fprintf(stdout, "\n");
		fflush(stdout);
		ls_release_object(shandle, cur_obj);
	} else {
		fprintf(stderr, "get_next_obj: failed on %d \n", err);
		exit(1);
	}

}



/*
 * Read the search configuration file.
 */
int
run_config_script(char *fname)
{
	int err;

	yyin = fopen(fname, "r");
	if(!yyin) {
		perror(fname);
		return 1;
	}
	err = yylex();

	return(0);
}
