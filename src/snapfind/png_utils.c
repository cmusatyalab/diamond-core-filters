/*
 * 	Diamond (Release 1.0)
 *      A system for interactive brute-force search
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <opencv/cv.h>

#include <png.h>


#define HEADER_SIZE 8

void
user_read_fn(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
	ffile_t        *file = (ffile_t *) png_ptr;
	char           *bufp;
	size_t          bytes_read;
	png_uint_32     bytes_remaining = length;

	while (bytes_remaining) {
		bytes_read = ff_read(file, bufp, bytes_remaining);
		assert(bytes_read);
		memcpy(data, bufp, bytes_read);
		bytes_remaining -= bytes_read;
	}
}


/*
 * returns error status 
 */
int
read_header(lf_obj_handle_t ohandle, ffile_t * file, int *width, int *height)
{
	char            buf[32];
	png_structp     png_ptr;
	png_infop       info_ptr;
	png_infop       end_info;
	int             bit_depth,
	color_type;

	if (ff_read(file, buf, HEADER_SIZE) != HEADER_SIZE) {
		return 1;
	}
	is_png = !png_sig_cmp(buf, 0, HEADER_SIZE);
	if (!is_png)
		return 1;

	/*
	 * png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, 
	 */
	/*
	 * (png_voidp)user_error_ptr, 
	 */
	/*
	 * user_error_fn, user_warning_fn, 
	 */
	/*
	 * (png_voidp)user_mem_ptr, 
	 */
	/*
	 * user_malloc_fn, user_free_fn); 
	 */
	/*
	 * png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
	 * (png_voidp)user_error_ptr, 
	 */
	/*
	 * user_error_fn, user_warning_fn); 
	 */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return 1;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr,
		                        (png_infopp) NULL, (png_infopp) NULL);
		return 1;
	}

	end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
		return 1;
	}

	/*
	 * error handler 
	 */
	if (setjmp(png_ptr->jmpbuf)) {
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return 1;
	}

	/*
	 * setup our own read function 
	 */
	png_set_read_fn(png_ptr, (void *) user_io_ptr, user_read_fn);


	/*
	 * say we already read these bytes 
	 */
	png_set_sig_bytes(png_ptr, HEADER_SIZE);

	/*
	 * read and extract info 
	 */
	png_read_info(png_ptr, info_ptr);
	*width = png_get_image_width(png_ptr, info_ptr);
	*height = png_get_image_height(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	if (color_type != PNG_COLOR_TYPE_GRAY) {
		fprintf(stderr, "only grayscale images supported\n");
		assert(0);
		return 1;
	}

	/*
	 * transform to 8 bits 
	 */
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		png_set_expand(png_ptr);
	}

	/*
	 * make sure bit depth does not exceed 8 bits/channel 
	 */
	if (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	png_read_update_info(png_ptr, info_ptr);


	return 0;
}

int
read_data(lf_obj_handle_t ohandle, ffile_t * file, void *data, int width,
          int height)
{

	png_bytep       row_pointers[height];
	int             i;
	char           *rowp = (char *) data;

	for (i = 0; i < height; i++) {
		row_pointers[i] = rowp;
		rowp += width;
	}
	png_read_image(png_ptr, row_pointers);



}



int
f_integrate(lf_obj_handle_t ohandle, int numout, lf_obj_handle_t * ohandles,
            int numarg, char **args)
{
	int             width,
	height;
	IplImage       *img,
	*sum;
	void           *file;
	int             err = 0;

	/*
	 * read the header and figure out the dimensions 
	 */
	file = read_header(ohandle, &width, &height);
	FILTER_ASSERT(file, "read header");

	/*
	 * create image to hold the data 
	 */
	img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	FILTER_ASSERT(img, "cvCreateImage");

	/*
	 * read the data into img 
	 */
	err = read_data(ohandle, file, img->imageData, width, height);
	FILTER_ASSERT(!err, "read data");

	sum = cvCreateImage(cvSize(width, height), IPL_DEPTH_32S, 1);
	FILTER_ASSERT(sum, "cvCreateImage");
	cvIntegral(img, sum, NULL, NULL);

	cvReleaseImage(&img);

	/*
	 * save sum as an attribute 
	 */

	return 1;                   /* always pass */
}
