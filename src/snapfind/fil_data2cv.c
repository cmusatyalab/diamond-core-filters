

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <opencv/cv.h>

#include "filter_api.h"
#include "fil_file.h"
#include "face.h"
#include "fil_image_tools.h"
#include "fil_data2cv.h"
#include "fil_assert.h"

int
f_init_get_rgb_ipl_image(int numarg, char **args, int blob_len,
                         void *blob_data, void **fdatap)
{

    *fdatap = NULL;
    return (0);
}

int
f_fini_get_rgb_ipl_image(void *fdata)
{
    return (0);
}


int
f_get_rgba_ipl_Image(lf_obj_handle_t ohandle, int numout,
                     lf_obj_handle_t * ohandles, int numarg, void *fdata)
{
    IplImage       *img;
    RGBImage       *tmp_img;
    int             err = 0,
        pass = 0;
    lf_fhandle_t    fhandle = 0;
    ffile_t         file;
    int             width,
                    height,
                    headerlen;
    image_type_t    img_type = IMAGE_PPM;
    off_t           bytes;

    /*
     * read the header and figure out the dimensions 
     */
    ff_open(fhandle, ohandle, &file);
    err = pnm_file_read_header(&file, &width, &height, &img_type, &headerlen);
    FILTER_ASSERT(!err, "read header");
    fprintf(stderr, "got image: width=%d, height=%d\n", width, height);

    /*
     * save some attribs 
     */
    lf_write_attr(fhandle, ohandle, IMG_HEADERLEN, sizeof(int),
                  (char *) &headerlen);
    lf_write_attr(fhandle, ohandle, ROWS, sizeof(int), (char *) &height);
    lf_write_attr(fhandle, ohandle, COLS, sizeof(int), (char *) &width);

    bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);
    err = lf_alloc_buffer(fhandle, bytes, (char **) &tmp_img);

    tmp_img->width = width;
    tmp_img->height = height;
    /*
     * create image to hold the data 
     */
    img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
    FILTER_ASSERT(img, "cvCreateImage");

    /*
     * read the data into img 
     */
    err = pnm_file_read_data(&file, tmp_img);
    memcpy(img->imageData, tmp_img->data, img->imageSize);
    FILTER_ASSERT(!err, "read data");

    lf_write_attr(fhandle, ohandle, RGBA_IPL_IMAGE, sizeof(char *),
                  (char *) img);

    pass = 1;

  done:

    release_rgb_image(tmp_img);


    return pass;

}


int
f_init_get_gray_ipl_image(int numarg, char **args, int blob_len,
                          void *blob_data, void **fdatap)
{

    *fdatap = NULL;
    return (0);
}

int
f_fini_get_gray_ipl_image(void *fdatap)
{
    return (0);
}


int
f_eval_get_gray_ipl_image(lf_obj_handle_t ohandle, int numout,
                          lf_obj_handle_t * ohandles, void *fdatap)
{
    IplImage       *gray_img = NULL;
    RGBImage       *tmp_img = NULL;
    ffile_t         file;
    int             err = 0,
        pass = 0;
    lf_fhandle_t    fhandle = 0;
    int             width,
                    height,
                    headerlen;
    image_type_t    img_type = IMAGE_PPM;
    off_t           bytes;

    /*
     * read the header and figure out the dimensions 
     */
    ff_open(fhandle, ohandle, &file);
    err = pnm_file_read_header(&file, &width, &height, &img_type, &headerlen);
    FILTER_ASSERT(!err, "read header");

    /*
     * save some attribs 
     */
    lf_write_attr(fhandle, ohandle, IMG_HEADERLEN, sizeof(int),
                  (char *) &headerlen);
    lf_write_attr(fhandle, ohandle, ROWS, sizeof(int), (char *) &height);
    lf_write_attr(fhandle, ohandle, COLS, sizeof(int), (char *) &width);

    bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);
    err = lf_alloc_buffer(fhandle, bytes, (char **) &tmp_img);

    tmp_img->nbytes = bytes;
    tmp_img->type = img_type;
    tmp_img->width = width;
    tmp_img->height = height;

    err = pnm_file_read_data(&file, tmp_img);
    FILTER_ASSERT(!err, "pnm_file_read_data");

    err =
        lf_write_attr(fhandle, ohandle, RGB_IMAGE, tmp_img->nbytes,
                      (char *) tmp_img);
    ASSERTX(pass = 0, !err);

    gray_img = get_gray_ipl_image(tmp_img);

    err =
        lf_write_attr(fhandle, ohandle, GRAY_IPL_IMAGE, gray_img->nSize,
                      (char *) gray_img);
    ASSERTX(pass = 0, !err);

    pass = 1;

  done:

    if (tmp_img) {
        lf_free_buffer(fhandle, (char *) tmp_img);
    }
    /*
     * releasing the cvimage causes wierd things to happen if we read it
     * back. pointers somewhere? 
     */
    if (gray_img) {
        cvReleaseImageHeader(&gray_img);
    }
    ff_close(&file);
    return pass;

}


/*
 * int 
 */
/*
 * f_makecvii(lf_obj_handle_t ohandle, int numout, lf_obj_handle_t *ohandles, 
 */
/*
 * int numarg, char **args) 
 */
/*
 * { 
 */
/*
 * IplImage *img, *sum, *sum2; 
 */
/*
 * //ii_image_t *img, *img2; 
 */
/*
 * ffile_t file; 
 */
/*
 * int err = 0, pass = 1; 
 */
/*
 * lf_fhandle_t fhandle = 0; 
 */
/*
 * int width, height, headerlen; 
 */

/*
 * fprintf(stderr, "\nf_integrate: enter\n"); 
 */

/*
 * /\* read the header and figure out the dimensions *\/ 
 */
/*
 * ff_open(fhandle, ohandle, &file); 
 */
/*
 * err = pgm_read_header(&file, &width, &height, &headerlen); 
 */
/*
 * FILTER_ASSERT(!err, "read header"); 
 */
/*
 * fprintf(stderr, "got image: width=%d, height=%d\n", width, height); 
 */

/*
 * /\* save some attribs *\/ 
 */
/*
 * lf_write_attr(fhandle, ohandle, IMG_HEADERLEN, sizeof(int), (char
 * *)&headerlen); 
 */
/*
 * lf_write_attr(fhandle, ohandle, ROWS, sizeof(int), (char *)&height); 
 */
/*
 * lf_write_attr(fhandle, ohandle, COLS, sizeof(int), (char *)&width); 
 */

/*
 * /\* create image to hold the data *\/ 
 */
/*
 * img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1); 
 */
/*
 * FILTER_ASSERT(img, "cvCreateImage"); 
 */

/*
 * /\* read the data into img *\/ 
 */
/*
 * err = pgm_read_data(&file, img->imageData, width, height); 
 */
/*
 * FILTER_ASSERT(!err, "read data"); 
 */

/*
 * /\* create image for integral, sq-integral *\/ 
 */
/*
 * sum = cvCreateImage(cvSize(width, height), IPL_DEPTH_32S, 1); 
 */
/*
 * FILTER_ASSERT(sum, "cvCreateImage"); 
 */
/*
 * sum2 = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1); 
 */
/*
 * FILTER_ASSERT(sum, "cvCreateImage"); 
 */

/*
 * /\* compute integrals *\/ 
 */
/*
 * cvIntegral(img, sum, sum2, NULL); 
 */

/*
 * #if 0 
 */
/*
 * /\* release base image *\/ 
 */
/*
 * cvReleaseImage(&img); 
 */
/*
 * #else 
 */
/*
 * /\* save img as an attribute *\/ 
 */
/*
 * lf_write_attr(fhandle, ohandle, CV_IMAGE, sizeof(char*), (char *)img); 
 */
/*
 * #endif 
 */
/*
 * lf_write_attr(fhandle, ohandle, II_DATA, sizeof(char*), (char *)&sum); /\* 
 * XXX *\/ 
 */
/*
 * lf_write_attr(fhandle, ohandle, II_SQ_DATA, sizeof(char*), (char *)&sum2); 
 * /\* XXX *\/ 
 */

/*
 * done: 
 */
/*
 * fprintf(stderr, "f_integrate: done\n\n"); 
 */
/*
 * return pass; 
 */
/*
 * } 
 */
