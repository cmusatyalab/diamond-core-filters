
/*
 * filter to make an integral image from raw data in pgm format
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "filter_api.h"
#include "fil_file.h"
//#include "fil_image_tools.h"
#include "face.h"
#include "rgb.h"
#include "fil_data2ii.h"
#include "fil_tools.h"
#include "fil_assert.h"

#define MAXCOLS 1281

/*
 * read byte-data from file, and fill in sumarr, sumsqarr. col1, row1 are 0
 * (excl sum) 
 */
int
read_sum_data(ffile_t * file,
              u_int32_t * sumarr,
              float *sumsqarr,
              size_t iiwidth, size_t iiheight, image_type_t imgtype)
{
    int             err = 0;
    off_t           bytes;
    char           *fdata;
    size_t          nb;
    size_t          row,
                    col;
    u_int32_t       colsum[MAXCOLS];    /* keep a column total */
    float           colsumsq[MAXCOLS];
    u_int32_t      *sumarr_end = sumarr + (iiwidth * iiheight);
    int             isRGBA;

    ASSERTX(err = 1, iiwidth < MAXCOLS);

    switch (imgtype) {
        /*
         * case IMAGE_PPM: 
         */
        /*
         * isRGBA = 1; 
         */
        /*
         * break; 
         */
    case IMAGE_PGM:
        isRGBA = 0;
        break;
    default:
        /*
         * unknown type 
         */
        return 1;               /* error */
    }

    for (col = 0; col < iiwidth; col++) {
        colsum[col] = 0;
        colsumsq[col] = 0;
    }

    for (col = 0; col < iiwidth; col++) {
        sumarr[col] = 0;
        sumsqarr[col] = 0;
    }
    for (row = 0; row < iiheight; row++) {
        sumarr[row * iiwidth] = 0;
        sumsqarr[row * iiwidth] = 0;
    }

    row = 1;                    /* excl */
    col = 1;                    /* excl */
    sumarr += 1 + iiwidth;      /* excl */
    sumsqarr += 1 + iiwidth;    /* excl */
    bytes = (iiwidth - 1) * (iiheight - 1); /* excl */
    if (isRGBA) {
        bytes *= 4;
    }

    while (!err && bytes) {
        nb = ff_read(file, &fdata, bytes);
        if (!nb) {
            err = 1;
            break;
        }
        bytes -= nb;
        while (nb) {
            int             value;

            if (isRGBA) {
                value = *((unsigned char *) fdata + 0);
                value += *((unsigned char *) fdata + 1);
                value += *((unsigned char *) fdata + 2);
                fdata += 4;
                nb -= 4;
            } else {            /* greyscale */
                value = *((unsigned char *) fdata);
                fdata++;
                nb--;
            }


            colsum[col] += value;
            colsumsq[col] += value * value;
            *sumarr = colsum[col] + (col ? *(sumarr - 1) : 0);
            *sumsqarr = colsumsq[col] + (col ? *(sumsqarr - 1) : 0);
            // fprintf(stderr, " %f", *sumsqarr); /* XXX */
            sumarr++;
            ASSERTX(err = 1, sumarr <= sumarr_end);
            sumsqarr++;
            col++;
            ASSERTX(err = 1, col <= iiwidth);
            if (col == iiwidth) {
                sumarr++;       /* excl */
                sumsqarr++;     /* excl */
                col = 1;        /* excl */
                row++;
                ASSERTX(err = 1, row <= iiheight);
                // fprintf(stderr, "\n");//XXX
            }
        }
    }
    ASSERTX(err = 1, sumarr == sumarr_end + 1);
  done:
    return err;
}





int
rgb_integrate(RGBImage * img,
              u_int32_t * sumarr,
              float *sumsqarr, size_t iiwidth, size_t iiheight)
{
    int             err = 0;
    int             count;
    int             i;
    size_t          nb;
    size_t          row,
                    col;
    u_int32_t       colsum[MAXCOLS];    /* keep a column total */
    float          colsumsq[MAXCOLS];
    u_int32_t      *sumarr_end = sumarr + (iiwidth * iiheight);
    unsigned char  *fdata = NULL;

    ASSERTX(err = 1, iiwidth < MAXCOLS);

    for (col = 0; col < iiwidth; col++) {
        colsum[col] = 0;
        colsumsq[col] = 0;
    }

    for (col = 0; col < iiwidth; col++) {
        sumarr[col] = 0;
        sumsqarr[col] = 0;
    }
    for (row = 0; row < iiheight; row++) {
        sumarr[row * iiwidth] = 0;
        sumsqarr[row * iiwidth] = 0;
    }

    row = 1;                    /* excl */
    col = 1;                    /* excl */
    sumarr += 1 + iiwidth;      /* excl */
    sumsqarr += 1 + iiwidth;    /* excl */
    count = (iiwidth - 1) * (iiheight - 1); /* excl */

    fdata = (unsigned char *) img->data;    /* XXX */
    for (i = 0; i < count; i++) {
        u_int32_t       value;

        value = fdata[0];
        value += fdata[1];
        value += fdata[2];
        value /= 3;
        fdata += 4;             /* skip alpha */
        nb -= 4;

        colsum[col] += value;
        colsumsq[col] += value * value;
        *sumarr = colsum[col] + (col ? *(sumarr - 1) : 0);
        *sumsqarr = colsumsq[col] + (col ? *(sumsqarr - 1) : 0);
        // fprintf(stderr, " %f", *sumsqarr); /* XXX */

        sumarr++;
        ASSERTX(err = 1, sumarr <= sumarr_end);
        sumsqarr++;

        col++;
        ASSERTX(err = 1, col <= iiwidth);
        if (col == iiwidth) {
            sumarr++;           /* excl */
            sumsqarr++;         /* excl */
            col = 1;            /* excl */
            row++;
            ASSERTX(err = 1, row <= iiheight);
            // fprintf(stderr, "\n");//XXX
        }
    }
    ASSERTX(err = 1, sumarr == sumarr_end + 1);
  done:
    return err;
}





#ifndef NDEBUG
void
print_ii(ii_image_t * ii)
{
    int             row,
                    col;
    for (row = 0; row < ii->height; row++) {
        for (col = 0; col < ii->width; col++) {
            printf(" %03d", ii->data[row * ii->width + col]);
        }
        printf("\n");
    }
}
#endif


int
f_init_integrate(int numarg, char **args, int blob_len, void *blob_data,
                 void **fdatap)
{

    *fdatap = NULL;
    return (0);
}

int
f_fini_integrate(void *fdata)
{
    return (0);
}

int
f_eval_integrate(lf_obj_handle_t ohandle, int numout,
                 lf_obj_handle_t * ohandles, void *fdata)
{
    ii_image_t     *img = NULL;
    ii2_image_t    *img2 = NULL;
    // ffile_t file;
    int             err = 0,
        pass = 1;
    lf_fhandle_t    fhandle = 0;
    off_t           bytes;
    int             width,
                    height,
                    headerlen;
    // image_type_t magic;

    lf_log(fhandle, LOGL_TRACE, "\nf_integrate: enter\n");

    /*
     * get image 
     */
    RGBImage       *rgbimg =
        (RGBImage *) ft_read_alloc_attr(fhandle, ohandle, RGB_IMAGE);
    FILTER_ASSERT(rgbimg, "error getting RGB_IMAGE");
    ASSERTX(pass = 0, rgbimg);  /* XXX */
    width = rgbimg->width;
    height = rgbimg->height;

    /*
     * read the header and figure out the dimensions 
     */

    lf_log(fhandle, LOGL_TRACE, "got image: width=%d, height=%d\n", width,
           height);

    /*
     * save some attribs 
     */
    lf_write_attr(fhandle, ohandle, IMG_HEADERLEN, sizeof(int),
                  (char *) &headerlen);
    lf_write_attr(fhandle, ohandle, ROWS, sizeof(int), (char *) &height);
    lf_write_attr(fhandle, ohandle, COLS, sizeof(int), (char *) &width);

    /*
     * create image to hold the integral image 
     */
    bytes = sizeof(ii_image_t) + sizeof(u_int32_t) * (width + 1) * (height + 1);
    err = lf_alloc_buffer(fhandle, bytes, (char **) &img);
    FILTER_ASSERT(!err, "alloc");
    img->nbytes = bytes;
    img->width = width + 1;
    img->height = height + 1;

    /*
     * create image to hold the squared-integral image 
     */
    bytes = sizeof(ii2_image_t) + sizeof(float) * (width + 1) * (height + 1);
    err = lf_alloc_buffer(fhandle, bytes, (char **) &img2);
    FILTER_ASSERT(!err, "alloc");
    img2->nbytes = bytes;
    img2->width = width + 1;
    img2->height = height + 1;

    /*
     * make ii 
     */
    // err = read_sum_data(&file, img->data, img2->data, width+1, height+1,
    // magic);
    rgb_integrate(rgbimg, img->data, img2->data, width + 1, height + 1);
    FILTER_ASSERT(!err, "read data");
    // ff_close(&file);

    /*
     * save img as an attribute? 
     */
    err = lf_write_attr(fhandle, ohandle, II_DATA, img->nbytes, (char *) img);
    FILTER_ASSERT(!err, "write_attr");
    err = lf_write_attr(fhandle, ohandle, II_SQ_DATA, img2->nbytes,
                      (char *) img2);
    FILTER_ASSERT(!err, "write_attr");


done:
    if (rgbimg) {
        ft_free(fhandle, (char *) rgbimg);
    }
    if (img) {
        ft_free(fhandle, (char *) img);
    }
    if (img2) {
        ft_free(fhandle, (char *) img2);
    }
    lf_log(fhandle, LOGL_TRACE, "f_integrate: done\n");
    return pass;
}
