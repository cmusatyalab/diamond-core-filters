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
#include <assert.h>

#include "fil_file.h"
#include "fil_assert.h"


void
ff_open(lf_fhandle_t fhandle, lf_obj_handle_t obj_handle, ffile_t * fh)
{
	fh->fhandle = fhandle;
	fh->obj_handle = obj_handle;
	fh->len = 0;
	fh->buf = NULL;
	fh->pos = 0;
}


void
ff_close(ffile_t * fh)
{
	if (fh->buf) {
		lf_free_buffer(fh->fhandle, fh->buf);
	}
}


#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#define BLOCKSIZE (4<<10)

static int
ff_get_block(ffile_t * fh, size_t size)
{
	int             err;
	fh->len = max(size, BLOCKSIZE);
	err = lf_next_block(fh->fhandle, fh->obj_handle, 1, &fh->len, &fh->buf);
	/*
	 * assert(!err && "lf_next_block"); 
	 */
	ASSERTX((fh->len == 0), !err);

done:
	return err;
}


size_t
ff_read(ffile_t * fh, char **data, size_t size)
{
	size_t          nbytes = 0;
	size_t          remaining;
	int             err = 0;

	ASSERTX(nbytes = 0, size);

	if (fh->pos >= fh->len) {
		if (fh->buf) {
			lf_free_buffer(fh->fhandle, fh->buf);
			fh->buf = NULL;
		}
		err = ff_get_block(fh, size);   /* if err, rest works */
		if (err) {
			log_message(LOGT_FILT, LOGL_ERR,
			            "error %d getting block: ff_get_block (%d bytes)",
			            err, size);
			fprintf(stderr,
			        "ERROR %d getting block: ff_get_block (%d bytes)\n", err,
			        size);
		}
		fh->pos = 0;
	}

	*data = &fh->buf[fh->pos];
	remaining = fh->len - fh->pos;
	nbytes = min(remaining, size);
	fh->pos += nbytes;

	assert(!err || !nbytes);    /* err => (nbytes == 0) */

done:
	return nbytes;
}




/*
 * setup a pointer into the current buffer, len 
 */
void
ff_getbuf(ffile_t * fh, char **buf, size_t * len)
{
	if (!fh->buf) {
		ff_get_block(fh, BLOCKSIZE);
	}

	*buf = fh->buf;             /* buffer */
	*len = fh->len - fh->pos;   /* bytes remaining */
}

/*
 * mark some number of bytes as read 
 */
void
ff_consume(ffile_t * fh, size_t size)
{

	ASSERTX(assert(0), (off_t) (fh->pos + size) < fh->len);
	fh->pos += size;

done:
	return;
}
