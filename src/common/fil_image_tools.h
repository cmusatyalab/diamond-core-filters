
#include "image_tools.h"
#include "fil_file.h"

/* 
 * some interfaces to read images from a ffile_t
 */

/* read in header, anymap */
int pnm_file_read_header(ffile_t *file, int *width, int *height,
			 image_type_t *type, int *headerlen);

/* read data, graymap (8 bit) */
int pgm_file_read_data_plus(ffile_t *file, uint8_t *data,
		       size_t width, size_t height);

/* read data, pixmap (8x8x8 bit) */
int ppm_file_read_data(ffile_t *file, RGBImage *img);


/* read data, greymap (8 bit) */
int pgm_file_read_data(ffile_t *file, RGBImage *img);

/* wrapper around p*n_file_read_data */
int pnm_file_read_data(ffile_t *file, RGBImage *img);

RGBImage * get_rgb_img(lf_obj_handle_t ohandle);

