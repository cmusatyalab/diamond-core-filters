
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>
#include <libgen.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>

#include "snap_search.h"
int
main(int argc, char **argv)
{
	rgb_histo_search *	rgbs;
	char *			name;
	rgbs = new rgb_histo_search("foo");

	name = rgbs->get_name();

	printf("name <%s>\n", name);

	delete rgbs;
}




