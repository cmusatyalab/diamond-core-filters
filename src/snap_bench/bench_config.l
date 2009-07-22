%option nounput 
%option noyywrap 
%option prefix="benchconfig" 
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

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <assert.h> 
#include <stdlib.h> 
#include <sys/time.h>
#include <time.h> 
#include <errno.h>
#include <sys/queue.h>
#include "lib_searchlet.h"
#include "lib_results.h"
#include "snapfind_consts.h"
#include "snap_bench.h"
#include "sys_attr.h"
#include "lib_filter.h"
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <glib.h>
#include <fcntl.h>
#include <libgen.h>


static char *eat_token(char *tok);
static void init_search(char *tok);
static void set_searchlist(char *tok);
static void set_searchlet(char *tok);
static void add_filterfile(char *tok);
static void start_search(char *tok);
static void term_search(char *tok);
static void set_blob(char *tok);
static void set_user_state(char *tok);
static void next_object(char *tok);
static void all_objects(char *tok);
static void sleep_ms(char *tok);
static void think_ms(char *tok);
static void run_script(char *tok);
static void have_error(char *tok);


void set_matches_stop(char *string);
int object_matches(ls_obj_handle_t handle);
void run_query(char *string);
void cleanup_search(void);
void reexecute_filters(void);

extern int verbose;

ls_search_handle_t shandle;

struct timeval	start;
struct timeval  think;


%}



HEX8    [[:xdigit:]][[:xdigit:]]
HEX16   {HEX8}:{HEX8} 
HEX32   {HEX16}:{HEX16}
HEX64   {HEX32}:{HEX32}
NAME    [a-zA-Z0-9][a-zA-Z0-9_\.-]

%%
#.*\n
ls_init_search.*\n   				{init_search(yytext);}
ls_set_searchlist[ \t]+id[ \t]+[[:digit:]].*\n  {set_searchlist(yytext);}
ls_set_searchlet[ \t]+id[ \t]+[[:digit:]].*\n  {set_searchlet(yytext);}
ls_add_filter_file[ \t]+id[ \t]+[[:digit:]].*\n  {add_filterfile(yytext);}
ls_start_search[ \t]+id[ \t]+[[:digit:]].*\n  {start_search(yytext);}
ls_terminate_search[ \t]+id[ \t]+[[:digit:]].*\n  {term_search(yytext);}
ls_next_object[ \t]+id[ \t]+[[:digit:]].*\n  	{next_object(yytext);}
ls_set_blob[ \t]+id[ \t]+[[:digit:]].*\n  	{set_blob(yytext);}
ls_set_user_state[ \t]+id[ \t]+[[:digit:]].*\n  {set_user_state(yytext);}
sleep[ \t]+[[:digit:]].*\n  			{sleep_ms(yytext);}
think[ \t]+[[:digit:]].*\n			{think_ms(yytext);}
run_script[ \t]+.*\n 				{run_script(yytext);}
dump_filterstats[ \t]+.*\n 			{dump_filtstats();}
all_objects[ \t]+.*\n 				{all_objects(yytext);}
set_matches_stop[ \t]+[[:digit:]]+[ \t]+.*\n	{set_matches_stop(yytext);}
run_query[ \t]+[[:digit:]]+.*\n			{run_query(yytext);}
ls_reexecute_filters.*\n				{reexecute_filters();}
[[:blank:]]
\n
. 		                              {have_error(yytext); }

%%

#define MAX_NAME		128
#define IMAGES_PER_SCREEN	1
long think_time		 =	1000; /*ms*/
int cur_search_match_min =	0;
char *cur_search_matches =	NULL;
long cur_search_matches_len =	0;
GHashTable *matches_hash = NULL;
char *obj_id		= NULL;

void set_matches_stop(char *string){
   char *token;
   char path[PATH_MAX], c;
   int f;
   long i;

   token = eat_token(string);
   if( sscanf(token, "%d", &cur_search_match_min) < 1){
      fprintf(stderr, "set_matches_stop found invalid threshold %s.\n", token);
      return;
      }

   token = eat_token(token);
   if( sscanf(token, "%s\n", path) < 1){
      fprintf(stderr, "set_matches_stop found invalid maches file: %s\n", token);
      return;
      }

   f = open( path, O_RDONLY );
   if( cur_search_matches != NULL){
      /*free( cur_search_matches );*/
      g_hash_table_destroy(matches_hash);
      matches_hash = NULL;
      }

   /*
   cur_search_matches_len = lseek(f, 0, SEEK_END);

   cur_search_matches = malloc( sizeof(char) * (cur_search_matches_len + 1) );
   lseek(f, 0, SEEK_SET);

   if( read(f, cur_search_matches, cur_search_matches_len) < 0){
      fprintf(stderr, "set_matches_stop could not read from matches file %s.\n", path);
      return;
      }

   cur_search_matches[cur_search_matches_len] = '\0';
   

   for( i = 0 ; i < cur_search_matches_len; ++i)
      if(cur_search_matches[i] == '\n')
         cur_search_matches[i] = '\0';
   */

   matches_hash = g_hash_table_new(g_str_hash, g_str_equal);
   i = 0;
   while(read(f, &c, 1)){
      if( c == '\n' ){
         path[i] = '\0';
         g_hash_table_insert( matches_hash, g_strdup( path ), "");
         i = 0;
         }
      else{   
          path[i]= c;
          ++i;
          }
      if(i >= PATH_MAX)
          i = 0;
      }

    if(i > 0 ){
       path[i] = '\0';
       g_hash_table_insert( matches_hash, g_strdup( path ) , "");
       }


   close(f);


   fflush(stdout);
   }

int object_matches(ls_obj_handle_t handle){
   char path[MAX_NAME], *cur, *name;
   size_t size = MAX_NAME;
   int err;
   long len = 0;
 
   err = lf_read_attr( handle, DISPLAY_NAME, &size,
                       (unsigned char *)path);
   name = basename( path );

   /*print current file*/
   dump_device( handle );

   if(err)
      fprintf(stdout, "name Unknown");
   else {
      printf( "name %s", name);

      /*cur = cur_search_matches;
      while( len < cur_search_matches_len ){
         if(strcmp(name, cur) == 0){
            printf( "... matches.\n" );
            return 1;
            }

         cur = strchr( cur, '\0');
         ++cur;
         len = cur - cur_search_matches;
         }
        */
      if( g_hash_table_lookup (matches_hash, name) != NULL){
         printf( "... matches.\n" );
         fflush(stdout);
         return 1;
         }
      }

   printf( "\n");
   fflush(stdout);
   return 0;
   }


void reexecute_filters(void){
    ls_obj_handle_t obj;

    ls_reexecute_filters(shandle, obj_id, NULL, &obj);

    ls_release_object(shandle, obj);

   }


/*While query is on and Not EOS, fetch all objects and put on Q*/
/*static void prefetch_objects(){

  }*/

/*TODO: start new thread?*/
/* Keep getting an object from Q until we find target objects. *
 * Response time is the time to fill screen with (6) objects.    */
void run_query(char *string){
   ls_obj_handle_t  objs[IMAGES_PER_SCREEN ];
   int err, i;
   int has_objs[IMAGES_PER_SCREEN]={0};
   struct timeval start, end;
   long long  response_t;
   char sleep_cmd[18], *token;
   int has_more = 1;

   token = eat_token(string);

   if( sscanf(token, "%ld", &think_time) < 1){
      fprintf(stderr, "think_time specified is invalid %s.\n", token);
      return;
      }

   /*Keep getting next object until I find required 
     number of matching images*/
   while( cur_search_match_min > 0 && has_more){
      gettimeofday(&start, NULL); 
      /*set_user_state( "ls_set_user_state id 0 state WAITING\n" );*/

      /*fetch six objects at a time -- emulating Snapfind */
      for( i = 0; i < IMAGES_PER_SCREEN; ++i){ 

         err = ls_next_object(shandle, &objs[i], LSEARCH_NO_BLOCK);
         if (err == ENOENT){
            fprintf(stderr, "run_query: no more objects \n");
            has_more = 0;
            break;
            }
         else if (err == 0) {
            /*all good...*/
            has_objs[i] = 1;
            } 
         else if(err == EWOULDBLOCK)
           --i;
         else {
            fprintf(stderr, "run_query: failed on %d \n", err);
            has_more = 0;
            break;
            }

         }

    if( obj_id )
       free( obj_id );
    ls_get_objectid(shandle, objs[i - 1], &obj_id);

     /* set_user_state( "ls_set_user_state id 0 state BUSY\n" );*/

      gettimeofday(&end, NULL);
      response_t = ((long long) (end.tv_sec - start.tv_sec) * 1000) 
                 + (end.tv_usec - start.tv_usec) / 1000;

      for( i = 0; i < IMAGES_PER_SCREEN; ++i){
         if( has_objs[i] ){
            if(object_matches(objs[i]) )
               --cur_search_match_min;

            ls_release_object(shandle, objs[i]);
            }

         has_objs[i] = 0;
         }
    
      printf( "Response time: %lld ms\n", response_t );
      fflush(stdout); 
      /*Think -- I define it as time to look at screen*/
      /*TODO: subtract obj matches overhead time*/
      sprintf( sleep_cmd, "sleep %ld\n", think_time ); 
      sleep_ms( sleep_cmd );
      }
   }

void cleanup_search(void){
   FILE *f;
   /*fork first !!
     int err = execl( "/bin/rm", "rm", "-r", "/tmp/DiamondCache", NULL );
     fprintf(stderr, "Error cleaning up cache -- ");
   */

   f = popen( "/bin/rm -r /tmp/DiamondCache", "r" );
   /*while(fgetc(f));*/
   pclose( f );
   fprintf( stderr, "/tmp/DiamondCache removed.\n");
   }



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

        const char *alist[] = {DISPLAY_NAME, DEVICE_NAME, 
                               THUMBNAIL_ATTR, COLS, ROWS, NULL};
        ls_set_push_attributes(shandle, alist);

        gettimeofday(&start, NULL);
	err = ls_start_search(shandle);
        if (err) {
                fprintf(stderr, "Failed to start search on err %d \n", err);
                exit(1);
        }
	if (verbose)
                fprintf(stdout, "starting new search at %lu.%06lu\n",
        		start.tv_sec, start.tv_usec);
}

void
term_search(char *string)
{
	int	err;
	int	secs, usec;

	struct timeval end;
	err = ls_terminate_search(shandle);
        if (err) {
                fprintf(stderr, "Failed to terminate search on err %d\n", err);
                exit(1);
	}
        gettimeofday(&end, NULL);

        secs = end.tv_sec - start.tv_sec;
        usec = end.tv_usec - start.tv_usec;
        if (usec < 0) {
                usec += 1000000;
                secs -= 1;
        }
        if (verbose) {
          	fprintf(stdout, "stopping search at %lu.%06lu\n", 
    			end.tv_sec, end.tv_usec);
        }
       /*cleanup diamond cache
       cleanup_search();*/
}

void
set_user_state(char *string)
{
	int  err;
	char *token;
	char *state;
	user_state_t us;
	struct timeval stamp;

	token = eat_token(string); 
	token = eat_token(token); // id 
	token = eat_token(token); // id number
	token = eat_token(token); // state
	state = copy_to_whitespace(token);  // state string
	if (strncmp(state, "BUSY", strlen("BUSY")) == 0) {
		us = USER_BUSY;
	    gettimeofday(&think, NULL);
	} else {
		us = USER_WAITING;
		timerclear(&think);
	}
	
	gettimeofday(&stamp, NULL);
	err = ls_set_user_state(shandle, us);
    if (err) {
    	fprintf(stderr, "Error setting user state: %d \n", err);
        exit(1);
    }
	if (verbose)
        fprintf(stdout, "user state is %s at %lu.%06lu\n", 
        		state, stamp.tv_sec, stamp.tv_usec);
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
think_ms(char *string)
{
	char *		token;
	long		ms;
	long		remainder;
	struct timeval	wanted;
	struct timeval now;
	struct timeval done;
	struct timeval todo;
	struct timeval zero;
	struct timespec todospec;
	long todoms;

	token = eat_token(string);
	ms = strtol(token, NULL, 10);
	if ((ms == 0) && (errno == EINVAL)) {
        fprintf(stderr, "Failed to convert sleep argument %s\n", token);
		exit(1);
	}
	wanted.tv_sec = ms / 1000;	
	remainder = ms - (wanted.tv_sec * 1000);
	wanted.tv_usec = remainder * 1000;	

	gettimeofday(&now, NULL);
	timersub(&now, &think, &done);
	timersub(&wanted, &done, &todo);
	timerclear(&zero);
	
	if (verbose) {
		if (timercmp(&todo, &zero, >)) {
			todoms = todo.tv_sec * 1000 + todo.tv_usec / 1000;
			fprintf(stdout, "think_ms: sleeping %ld ms of %ld ms\n", 
					todoms, ms);
		} else {
			fprintf(stdout, 
				"think_ms: user think time %ld exceeded, skipping\n",
				ms);
		}
	}
	
	if (timercmp(&todo, &zero, >)) {
		todospec.tv_sec = todo.tv_sec;
		todospec.tv_nsec = todo.tv_usec * 1000;
		nanosleep(&todospec, NULL);
	}
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
		fprintf(stdout, "get_next_obj: ");
		dump_device(cur_obj);
		dump_name(cur_obj);

		fprintf(stdout, "\n");
		fflush(stdout);
		ls_release_object(shandle, cur_obj);
	} else {
		fprintf(stderr, "get_next_obj: failed on %d \n", err);
		exit(1);
	}
        

}

void
all_objects(char *string)
{
	ls_obj_handle_t	cur_obj;
	int		err;

 	while ((err = ls_next_object(shandle, &cur_obj, 0)) == 0) {
		fprintf(stdout, "get_next_obj: ");
		dump_device(cur_obj);
		dump_name(cur_obj);
		fprintf(stdout, "\n");
		fflush(stdout);
		ls_release_object(shandle, cur_obj);
	}
	if (err != ENOENT) {
		fprintf(stderr, "all_objects: error %d fetching object\n", err);
		exit(1);
	}

}

void
run_script(char *string)
{
	char *token;
	char *fname;
	int err;

	token = eat_token(string);
	fname = copy_to_whitespace(token);
	if (fname == NULL) {
		fprintf(stderr, "run script failed: invalid file name\n");
		return;
	}
	if (verbose)
		fprintf(stdout, "run script: fname %s \n", fname);

	err = system(fname);
	if (err)  {
		fprintf(stderr, "run script fail: failed to execute system\n");
	}
		
	free(fname);	
        return;
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
