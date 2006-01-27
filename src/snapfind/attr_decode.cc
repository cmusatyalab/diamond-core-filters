/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */


/*
 *  Copyright (c) 2006 Larry Huston <larry@thehustons.net>
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

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
#include <string.h>
#include <gtk/gtk.h>
#include "attr_ent.h"
#include "attr_decode.h"



attr_decode::attr_decode(const char *name) 
{
	if (strlen(name) <= 8) {
		ad_name = (char *)malloc(9);
		sprintf(ad_name, "%-8s", name);
	} else {
		ad_name = strdup(name);
	}
	ad_type = 0;
	
}

attr_decode::~attr_decode() 
{
	free(ad_name);
}

void
attr_decode::set_type(int type) 
{
	ad_type = type;
}

int
attr_decode::get_type()
{
	return(ad_type);
}


const char * 
attr_decode::get_name() 
{ 
	return(ad_name);
}
