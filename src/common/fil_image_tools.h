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

#ifndef	_FIL_IMAGE_TOOLS_H_
#define	_FIL_IMAGE_TOOLS_H_	1

#include "image_tools.h"
#include "fil_file.h"



#ifdef __cplusplus
extern  "C"
{
#endif


	/*
	* some interfaces to read images from a ffile_t
	*/

	/*
	* read in header, anymap 
	*/
	int             pnm_file_read_header(ffile_t * file, int *width,
	                                     int *height, image_type_t * type,
	                                     int *headerlen);

	/*
	* read data, graymap (8 bit) 
	*/
	int             pgm_file_read_data_plus(ffile_t * file, uint8_t * data,
	                                        size_t width, size_t height);

	/*
	* read data, pixmap (8x8x8 bit) 
	*/
	int             ppm_file_read_data(ffile_t * file, RGBImage * img);


	/*
	* read data, greymap (8 bit) 
	*/
	int             pgm_file_read_data(ffile_t * file, RGBImage * img);

	/*
	* wrapper around p*n_file_read_data 
	*/
	int             pnm_file_read_data(ffile_t * file, RGBImage * img);

	RGBImage       *get_attr_rgb_img(lf_obj_handle_t ohandle, char *name);


	RGBImage       *get_rgb_img(lf_obj_handle_t ohandle);

#ifdef __cplusplus
}
#endif
#endif                          /* ! _FIL_IMAGE_TOOLS_H_ */
