/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <opencv/cv.h>

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

RGBImage       *
get_rgb_img(lf_obj_handle_t ohandle)
{
    RGBImage       *img = NULL;
    int             err = 0;
    lf_fhandle_t    fhandle = 0;
    int             width, height, headerlen;
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

    ff_close(&file);
    return (img);

}

RGBImage       *
get_attr_rgb_img(lf_obj_handle_t ohandle, char *attr_name)
{
    int             err = 0;
    lf_fhandle_t    fhandle = 0;
    char           *image_buf;
    off_t           bsize;
    IplImage       *srcimage;
    IplImage       *image;
    RGBImage       *rgb;

    /*
     * assume this attr > 0 size
     */

    bsize = 0;
    err = lf_read_attr(fhandle, ohandle, attr_name, &bsize, (char *) NULL);
    if (err != ENOMEM) {
        return NULL;
    }

    err = lf_alloc_buffer(fhandle, bsize, (char **) &image_buf);
    if (err) {
        return NULL;
    }

    err = lf_read_attr(fhandle, ohandle, attr_name, &bsize,
                       (char *) image_buf);
    if (err) {
        return NULL;
    }


    srcimage = (IplImage *) image_buf;
    image = cvCreateImage(cvSize(srcimage->width, srcimage->height),
                          srcimage->depth, srcimage->nChannels);

    memcpy(image->imageDataOrigin, image_buf + sizeof(IplImage),
           image->imageSize);


    rgb = convert_ipl_to_rgb(image);
    cvReleaseImage(&image);

    return (rgb);
}
