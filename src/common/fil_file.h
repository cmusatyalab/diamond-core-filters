
#ifndef _FIL_FILE_H
#define _FIL_FILE_H

/*
 * FILE-like interface to filter objects
 */


#include "filter_api.h"

typedef struct ffile {
	lf_fhandle_t fhandle;
	lf_obj_handle_t obj_handle;

	off_t len;		/* size of buf */
	char *buf;		/* buffer of data */
	off_t pos;

  int type;			/* hack; tracks image_type_t */
} ffile_t;


void ff_open(lf_fhandle_t fhandle, lf_obj_handle_t obj_handle, ffile_t *);
void ff_close(ffile_t *);

size_t ff_read(ffile_t *, char **data, size_t size);

/* utility functions */

/* setup a pointer into the current buffer, len */
void ff_getbuf(ffile_t *, char **buf, size_t *len);

/* mark some number of bytes as read */
void ff_consume(ffile_t *, size_t size);


#endif /* _FIL_FILE_H */
