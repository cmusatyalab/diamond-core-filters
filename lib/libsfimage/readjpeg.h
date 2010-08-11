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

#ifndef _READJPEG_H_
#define _READJPEG_H_

#include <jpeglib.h>	// for toff_t
#include "rgb.h"	// for RGBImage


// libjpeg is documented in /usr/share/doc/libjpeg62-dev


// Before calling TIFFClientOpen() we need to initialize
// the MyTiff structure.

// Pretends to be a file.
//
typedef struct {
  u_char*	buf;	// data -- note this is in bytes
  size_t	bytes;	// number of bytes in buf
} MyJPEG;

#ifdef __cplusplus
extern "C"
{
#endif

  RGBImage* convertJPEGtoRGBImage(MyJPEG* jp);

#ifdef __cplusplus
}
#endif

#endif /* !_READJPEG_H */
