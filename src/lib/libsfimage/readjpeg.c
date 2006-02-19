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



#include <stdio.h>
#include <assert.h>
#include <stdlib.h>	// for malloc
#include <string.h>	// for memcpy
#include <stdio.h>

// For stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <jpeglib.h>	// for JPEG library routines

#include "rgb.h"	// for RGBImage, RGBPixel
#include "readjpeg.h"

#include "lib_filter.h"	// for lf_log(...) debugging function
#include "lib_log.h"	// for lf_log's defines like LOGL_TRACE

// Function to call:
// RGBImage* convertJPEGtoRGBImage(TIFF* tif);


// Following are helper routines to allow jpeglib to work with
// data in a non-file

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
  // do nothing?
}

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
  return !0;
}

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  if (num_bytes <= 0) { return; }
  cinfo->src->next_input_byte += (size_t) num_bytes;
  cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
}

// we don't bother defining the resync_to_restart()

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
  // no work necessary here
}


// --------------------------------------------------
// Note that we malloc and return an RGBImage* so it is the
// caller's responsibility to dispose of the memory.
//
RGBImage*
convertJPEGtoRGBImage(MyJPEG* jp)
{
	assert(jp);

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	lf_log(LOGL_TRACE, "Entering convertJPEGtoRGBImage");
	printf("Entering convertJPEGtoRGBImage!");


	// XXX TODO XXX
	// Maybe we could allocate these statically and not have
	// to do it each time the object is called.
	//
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// get_rgb_from_jpeg has already set up jp->buf and jp->bytes
	// assert(cinfo.src);
	struct jpeg_source_mgr source;
	cinfo.src = &source;
	cinfo.src->next_input_byte = jp->buf;
	cinfo.src->bytes_in_buffer = jp->bytes;
	cinfo.src->init_source = init_source;
	cinfo.src->fill_input_buffer = fill_input_buffer;
	cinfo.src->skip_input_data = skip_input_data;
	cinfo.src->term_source = term_source;

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	// at this point, the following variables are available:
	// cinfo.output_width
	// cinfo.output_height
	// cinfo.output_components	(3 for RGB, 1 for greyscale)
	assert(cinfo.output_components == 3);

	int w = cinfo.output_width;
	int h = cinfo.output_height;
	RGBImage* rgbimg = rgbimg_blank_image(w, h);	// output image

        // XXX WARNING XXX
	// This  might not be OK since it is called after
	// jpeg_start_decompress()
	//
	// Note this is a funny datastructure.
	// buffer is an array of size 1, containing a single scanline.
// 	buffer = (*cinfo.mem->alloc_sarray)
// 	  		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	u_char* scanline = (u_char*) malloc( w * 3 * sizeof(u_char));
	JSAMPARRAY buffer = &scanline;

	RGBPixel* outpix = rgbimg->data;
	while (cinfo.output_scanline < h) {
	  // process a line
	  //
	  jpeg_read_scanlines(&cinfo, buffer, 1);
	  u_char* curpix = buffer[0];	// [0] refers to 1st scanline
	  int c;
	  for (c=0; c < w; c++) {	// write scanline [note 3*w]
	    // Note each curpix is a single R, G or B value
	    outpix->r = *curpix++;
	    outpix->g = *curpix++;
	    outpix->b = *curpix++;
	    outpix->a = 255;
	    outpix++;
	  }
	}
	free(scanline);
	scanline = NULL;

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return rgbimg;
}


