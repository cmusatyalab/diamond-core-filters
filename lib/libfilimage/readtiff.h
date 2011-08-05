/*
 *  Diamond Core Filters - collected filters for the Diamond platform
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _READTIFF_H_
#define _READTIFF_H_

#include <tiffio.h>	// for toff_t
#include "rgb.h"	// for RGBImage


// Before calling TIFFClientOpen() we need to initialize
// the MyTiff structure.

// Pretends to be a file.
//
typedef struct {
  const u_char*	buf;	// data -- note this is in bytes
  toff_t	offset;	// current position in file (bytes)
  toff_t	bytes;	// number of bytes in buf
} MyTIFF;

#ifdef __cplusplus
extern "C"
{
#endif

  RGBImage* convertTIFFtoRGBImage(MyTIFF* tp);

#ifdef __cplusplus
}
#endif

#endif /* !_READTIFF_H */
