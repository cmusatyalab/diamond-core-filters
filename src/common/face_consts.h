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



/* the number of filter stages in the face detector. we'd like more
 * than one, but this keep coming back to bite us, so use 1 for
 * now. */
#define NUM_FACE_DETECTORS 1

#define STRIDE_DEFAULT 32

#define PDISTANCE_DEFAULT 0.25
#define NDISTANCE_DEFAULT 0.8

#define MAX_NAME 128
#define MAX_PATH 256

#define MAX_SCAPE 64
#define MAX_FILT (MAX_SCAPE - 12)
/* fudge factor of 12 to cover utility+face filters */

/* #define NUM_LAP_PYR_LEVELS 4 */
/* XXX #define TEXTURE_NUM_CHANNELS 3 */
#define FILTER_TYPE_DEFAULT 0

#define MAX_ALBUMS 32
#define MAX_STRING 1024

/* max number of regexes in search */
#define MAX_REGEX 32

/* max number of regions that can be selected in zoomed window */
#define MAX_SELECT 32


/* dimension of a patch (assume square) */
#define MIN_TEXTURE_SIZE 32
#define STD_COLOR_SIZE   16
