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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>	// for malloc
#include <string.h>	// for memcpy

// For stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <tiffio.h>

#include "rgb.h"	// for RGBImage, RGBPixel
#include "readtiff.h"


// Important note:
// TIFF images have their origin at the BOTTOM-LEFT
// which makes things unnecessarily tricky :-)
//
// See:
// http://www.remotesensing.org/libtiff/man/TIFFRGBAImage.3tiff.html
//
// For that reason, I wanted to use TIFFReadRGBAImageOriented()
// rather than TIFFReadRGBAImage(), except that default libtiff
// on Intel machines doesn't seem to be sufficiently new.

// TODO: parse magic numbers
// Magic numbers:
// TIFF big endian starts 4D 4D 00 2A
// TIFF little endian 49 49 2A 00
//
// We could have Diamond check the first 8 bytes



// --------------------------------------------------
// Stuff for TIFFClientOpen(..) to work

// Before calling TIFFClientOpen() we need to initialize
// the MyTiff structure.

// semantics analogous to read(2)
//
tsize_t
myTIFFread(thandle_t fhandle, tdata_t buf, tsize_t count)
{
	if (count == 0) {
		return 0;
	}
	assert(fhandle);
	MyTIFF* f = (MyTIFF*)fhandle;
	assert(f->offset+count <= f->bytes);
	memcpy(buf, f->buf + f->offset, count);
	f->offset += count;
	return count;
}

tsize_t
myTIFFwrite(thandle_t f, tdata_t buf, tsize_t count)
{
	// We do not currently support writing XXX
	assert(0);
	return 0;
}

// semantics analogous to fseek(2)
//
toff_t
myTIFFseek(thandle_t f, toff_t offset, int whence)
{
	MyTIFF* file = (MyTIFF*)f;
	switch(whence) {
		case SEEK_SET:
			file->offset = offset;
			break;
		case SEEK_CUR:
			file->offset += offset;
			break;
		case SEEK_END:
			file->offset = file->bytes -1 + offset;
			break;
		default:	// ERROR
			fprintf(stderr, "Unknown whence in myTIFFseek()\n");
			return (toff_t)-1;
			break;
	}
	return file->offset;
}

// semantics analogous to fclose(2)
//
int
myTIFFclose(thandle_t f)
{
	/*
	 * No need to free file->buf since we didn't allocate it
	 */
	return 0;
}

// get size of file
//
toff_t
myTIFFsize(thandle_t f)
{
	MyTIFF* file = (MyTIFF*)f;
	return file->bytes;
}

// semantics analogous to mmap(2)
int
myTIFFmmap(thandle_t f, tdata_t* data, toff_t* length)
{
	MyTIFF* file = (MyTIFF*)f;
	*data = file->buf;
	return 0;	// don't know what to return.
}

// semantics analogous to munmap(2)
void
myTIFFunmap(thandle_t f, tdata_t data, toff_t length)
{
	// don't need to do anything
}


// --------------------------------------------------
// Note that we malloc and return an RGBImage* so it is the
// caller's responsibility to dispose of the memory.
//
RGBImage*
convertTIFFtoRGBImage(MyTIFF* tp)
{
	assert(tp);

	TIFF* tif = TIFFClientOpen("/dev/null", "r", tp,
	                           myTIFFread, myTIFFwrite, myTIFFseek, myTIFFclose,
	                           myTIFFsize, myTIFFmmap, myTIFFunmap);
	assert(tif);

#if 0
	// Here is code for counting the number of directories in a
	// TIFF file, in case future versions need it.
	//
	int dirs;
	for (dirs=1; TIFFReadDirectory(tif); dirs++) {
		TIFFPrintDirectory(tif);
	}
	fprintf(stderr, "dirs=%d\n", dirs);
#endif // 0

	uint32 w, h;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	size_t npixels = w*h;

	// I don't know how to avoid the following copy
	//
	uint32* raster = (uint32*)_TIFFmalloc(npixels * sizeof(uint32));
	assert(raster);

	// If we had a newer libtiff, I'd use this call:
	// valid=TIFFReadRGBAImageOriented(tif, w, h, raster, ORIENTATION_TOPLEFT, 0);
	// and avoid all the row-swapping crap below.
	//
	int valid = TIFFReadRGBAImage(tif, w, h, raster, 0);
	assert(valid);

	// ---------------- PROCESSING -----------------
	// There is something strange about TIFF files:
	// their raster order appears to be backwards...
	// (i.e., top line is bottom line)

	uint32* p = raster;
	RGBImage* rgbimg = rgbimg_blank_image(w, h);	// output image
	uint32 r, c;
	for (r=0; r<h; r++) {
		for (c=0; c<w; c++) {

			// Only needed since we don't have newer libtiff
			// else idx is simply r*w+c which means ++ would have worked
			//
			size_t idx = (h-1-r)*w + c;	// needed for default orientation
			uint32 pix = *p;
			RGBPixel* outpix = rgbimg->data + idx;

			outpix->r = TIFFGetR(pix);
			outpix->g = TIFFGetG(pix);
			outpix->b = TIFFGetB(pix);
			outpix->a = TIFFGetA(pix);

			p++;
		}
	}

	_TIFFfree(raster);
	TIFFClose(tif);
	return rgbimg;
}

#if 0
void
write_RGBImage_to_file(const RGBImage* rgbimg, const char* filename)
{
	assert(rgbimg);
	const RGBPixel* outpix = rgbimg->data;
	FILE* fout;	// XXX
	fout = fopen(filename, "w");
	assert(fout);
	fprintf(fout, "P3\n%d %d\n255\n", rgbimg->width, rgbimg->height);
	int r, c;
	for (r=0; r < rgbimg->height; r++) {
		for (c=0; c < rgbimg->width; c++) {
			fprintf(fout, "%d %d %d\n", outpix->r, outpix->g, outpix->b);
			outpix++;
		}
	}
	fclose(fout);				// XXX
}

/*
 * Given a ffile_t, examines the first 8 bytes to try to guess
 * whether it is a TIFF, PNM, etc.  Doesn't "read" the file per se.
 */
image_type_t
determine_image_type(const u_char* buf)
{
	const u_char pbm_ascii[2]	= "P1";
	const u_char pbm_raw[2]	= "P4";
	const u_char pgm_ascii[2]	= "P2";
	const u_char pgm_raw[2] 	= "P5";
	const u_char ppm_ascii[2]	= "P3";
	const u_char ppm_raw[2]	= "P6";
	const u_char tiff_big_endian[4] = {
	                                      0x4d, 0x4d, 0x00, 0x2a
	                                  };
	const u_char tiff_lit_endian[4] = {
	                                      0x49, 0x49, 0x2a, 0x00
	                                  };

	image_type_t type = IMAGE_UNKNOWN;
	if	  (0 == memcmp(buf, pbm_ascii, 2))	 {
		type = IMAGE_PBM;
	} else if (0 == memcmp(buf, pbm_raw, 2))	 {
		type = IMAGE_PBM;
	} else if (0 == memcmp(buf, pgm_ascii, 2))	 {
		type = IMAGE_PGM;
	} else if (0 == memcmp(buf, pgm_raw, 2))	 {
		type = IMAGE_PGM;
	} else if (0 == memcmp(buf, ppm_ascii, 2))	 {
		type = IMAGE_PPM;
	} else if (0 == memcmp(buf, ppm_raw, 2))	 {
		type = IMAGE_PPM;
	} else if (0 == memcmp(buf, tiff_big_endian, 4)) {
		type = IMAGE_TIFF;
	} else if (0 == memcmp(buf, tiff_lit_endian, 4)) {
		type = IMAGE_TIFF;
	}

	return type;
}

int
main(int argc, char* argv[])
{
	if (argc != 3) {
		printf("Usage: %s input.tiff output.ppm\n", argv[0]);
		return 1;
	}

	fprintf(stderr, "%s: infile=<%s> outfile=<%s>\n",
	        argv[0], argv[1], argv[2]);

	struct stat statbuf;
	int statsez = stat(argv[1], &statbuf);
	assert(statsez == 0);
	printf("stat says %s is %u bytes long\n",
	       argv[1], (unsigned int)statbuf.st_size);

	FILE* tfile = fopen(argv[1], "r");

	MyTIFF mytiffstruct;
	MyTIFF* tp = &mytiffstruct;
	tp->offset = 0;
	tp->buf = malloc(statbuf.st_size * sizeof(u_char));
	tp->bytes = fread(tp->buf, sizeof(u_char), 1+statbuf.st_size, tfile);
	printf("Read in %u bytes from %s\n", (unsigned int)tp->bytes, argv[1]);
	assert(feof(tfile));	// we should have read the entire image
	fclose(tfile);

	image_type_t type = determine_image_type(tp->buf);
	assert(type == IMAGE_TIFF);

	RGBImage* rgbimg = convertTIFFtoRGBImage(tp);
	write_RGBImage_to_file(rgbimg, argv[2]);

	free(tp->buf);
	return 0;
}
#endif // 0
