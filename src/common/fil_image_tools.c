
#include <stdio.h>

#include "fil_image_tools.h"
#include "fil_assert.h"

/*
 * read a portable anymap header from a file
 * returns 0 or error status
 */
int
pnm_file_read_header(ffile_t * file,
                     int *width, int *height,
                     image_type_t * magic, int *headerlen)
{
    char           *buf;
    size_t          buflen;
    int             err;

    ff_getbuf(file, &buf, &buflen);
    err = pnm_parse_header(buf, buflen, width, height, magic, headerlen);
    assert(!err);
    file->type = *magic;

    ff_consume(file, *headerlen);

done:
    return err;
}



/*
 * read ppm-data from file, and fill in data. 
 */
int
ppm_file_read_data(ffile_t * file, RGBImage * img)
{
    int             err = 0;
    off_t           bytes;
    char           *fdata;
    size_t          nb;
    size_t          width = img->width;
    size_t          height = img->height;
    uint8_t        *data = (uint8_t *) img->data;   /* XXX */
    RGBPixel       *data_end;
    int             parity;
    int             bytes_read = 0;

    /*
     * XXX this function should be cleaned up 
     */


    bytes = width * height * 3; /* 3 bytes per pixel */
    assert(sizeof(RGBPixel) >= 4);
    data_end = img->data + (width * height);
    parity = 0;

    ASSERTX(err = 4, file->type == IMAGE_PPM);

    while (!err && bytes) {
        nb = ff_read(file, &fdata, bytes);
        ASSERTX(err = 1, nb);

        bytes -= nb;
        bytes_read += nb;
        while (nb) {
            *data = *((uint8_t *) fdata);
            fdata++;
            data++;
            ASSERTX(err = 2, data <= (uint8_t *) data_end);
            parity++;
            if (parity == 3) {
                *data = 255;    /* fake alpha value */
                data++;
                parity = 0;
            }
            nb--;
        }
    }
    ASSERTX(err = 3, data == (uint8_t *) data_end);
  done:
    /*
     * if(err) { 
     */
    /*
     * fprintf(stderr, "ppm_file_read_data: #%d read %d bytes, %ld
     * remaining\n", 
     */
    /*
     * err, bytes_read, bytes); 
     */
    /*
     * fprintf(stderr, "\timage width=%d, height=%d\n", width, height); 
     */
    /*
     * fprintf(stderr, "\tdata=%p, end_data=%p\n", data, data_end); 
     */
    /*
     * } 
     */
    return err;

}

int
pgm_file_read_data(ffile_t * file, RGBImage * img)
{
    int             err = 0;
    off_t           bytes;
    char           *fdata;
    size_t          nb;
    size_t          width = img->width;
    size_t          height = img->height;
    RGBPixel       *data = img->data;
    RGBPixel       *data_end;


    ASSERTX(err = 4, file->type == IMAGE_PGM);

    /*
     * XXX this function should be cleaned up 
     */

    bytes = width * height;     /* 1 byte per pixel */
    data_end = img->data + (width * height);
    while (!err && bytes) {
        nb = ff_read(file, &fdata, bytes);
        ASSERTX(err = 1, nb);

        bytes -= nb;
        while (nb) {
            data->r = data->g = data->b = *((uint8_t *) fdata);
            data->a = 255;      /* fake alpha */
            fdata++;
            data++;
            ASSERTX(err = 1, data <= data_end);
            nb--;
        }
    }
    ASSERTX(err = 1, data == data_end);
  done:
    return err;
}

int
pnm_file_read_data(ffile_t * file, RGBImage * img)
{
    int             err;

    switch (file->type) {
    case IMAGE_PGM:
        err = pgm_file_read_data(file, img);
        break;
    case IMAGE_PPM:
        err = ppm_file_read_data(file, img);
        break;
    default:
        ASSERTX(err = 1, 0);
        break;
    }

  done:
    return err;
}

RGBImage *
get_rgb_img(lf_obj_handle_t ohandle)
{
    RGBImage       *img = NULL;
    int             err = 0,
        pass = 1;
    lf_fhandle_t    fhandle = 0;
    int             width,
                    height,
                    headerlen;
    off_t           bytes;
    image_type_t    magic;
    ffile_t         file;


    /*
     * read the header and figure out the dimensions 
     */
    ff_open(fhandle, ohandle, &file);
    err = pnm_file_read_header(&file, &width, &height, &magic, &headerlen);
    assert(!err);

    /*
     * create image to hold the data 
     */
    bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);
    err = lf_alloc_buffer(fhandle, bytes, (char **) &img);
    assert(!err);
    assert(img);
    img->nbytes = bytes;
    img->height = height;
    img->width = width;
    img->type = magic;

    /*
     * read the data into img 
     */
    /*
     * this should be elsewhere... 
     */
    switch (img->type) {
    case IMAGE_PPM:
        err = ppm_file_read_data(&file, img);
        assert(!err);
        break;

    case IMAGE_PGM:
        err = pgm_file_read_data(&file, img);
        assert(!err);
        break;

    default:
        assert(0 && "unsupported image format");
        /*
         * should close file as well XXX 
         */
    }

done:
	ff_close(&file);
    return(img);
   
}
