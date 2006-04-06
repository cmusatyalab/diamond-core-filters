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


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <sys/stat.h>

#include "queue.h"
#include "rgb.h"
#include "lib_results.h"
#include "snapfind_consts.h"
#include "img_search.h"
#include "ocv_body_search.h"
#include "opencv_face_tools.h"
#include "search_set.h"
#include "factory.h"
#include "snapfind_config.h"

/* config tokens  */
#define	NUMFACE_ID	"NUMFACE"
#define	SUPPORT_ID	"SUPPORT"

#define	SEARCH_NAME	"ocv_body_search"
#define CLASSIFIER_NAME       "haarcascade_fullbody"

extern "C" {
	void search_init();
}

void
search_init()
{
	ocv_body_factory *fac;

	fac = new ocv_body_factory;
	factory_register(fac);
}


ocv_body_search::ocv_body_search(const char *name, char *descr)
		: ocv_search(name, descr)
{
	set_scale(1.20);
	set_stride(1);
	set_testx(24);
	set_testy(24);
	set_classifier(CLASSIFIER_NAME);
}

ocv_body_search::~ocv_body_search()
{
  printf("destroying OCV body");
}

void
ocv_body_search::write_config(FILE *ostream, const char *dirname)
{
	save_edits();

	/* create the search configuration */
	fprintf(ostream, "\n\n");
	fprintf(ostream, "SEARCH %s %s\n", SEARCH_NAME, get_name());
	fprintf(ostream, "%s %d \n", NUMFACE_ID, get_count());
	fprintf(ostream, "%s %d \n", SUPPORT_ID, get_support());
	window_search::write_config(ostream, dirname);
	return;
}

void ocv_body_search::write_fspec(FILE *ostream) 
{
	/*
	 * First we write the header section that corresponds
	 * to the filter, the filter name, the associated functions.
	 */
	save_edits();
	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER %s \n", get_name());

	fprintf(ostream, "EVAL_FUNCTION  %s \n", "f_eval_opencv_bdetect");
	fprintf(ostream, "INIT_FUNCTION  %s \n", "f_init_opencv_bdetect");
	fprintf(ostream, "FINI_FUNCTION  %s \n", "f_fini_opencv_bdetect");

	ocv_search::write_fspec(ostream);
	return;
}

int 
ocv_body_search::matches_filter(char *name) 
{
	void *handle;
	void *initfn = NULL;
                                                                                
	handle = dlopen(name, RTLD_NOW);
	if (handle) {
		initfn = dlsym(handle, "f_init_opencv_bdetect");
		dlclose(handle);
	}
	
	return ((int) initfn);
}
