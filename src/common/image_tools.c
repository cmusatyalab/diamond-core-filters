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


#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <opencv/cv.h>
#include <gtk/gtk.h>

#include "image_tools.h"
#include "common_consts.h"
#include "fil_file.h"

/*
 * note: should make these functions localized 
 */
static char    *
skip_space_comments(char *buf, char *endbuf)
{
    int             eol = 0;

    assert(buf < endbuf);

    // fprintf(stderr, "buf=%.32s\n", buf);

    /*
     * skip spaces 
     */
    while (buf < endbuf && isspace(*buf)) {
        eol = (*buf == '\n');
        buf++;
    }

    // fprintf(stderr, "buf=%.32s; eol=%d\n", buf, eol);

    /*
     * skip any comments 
     */
    while (buf < endbuf && eol && *buf == '#') {
        // fprintf(stderr, "skipping comment: %.32s\n", buf);
        while (buf < endbuf && *buf != '\n') {
            buf++;
        }
        if (buf < endbuf)
            buf++;
    }

    // fprintf(stderr, "buf=%p, endbuf=%p\n", buf, endbuf);
    // fprintf(stderr, "buf=%.32s\n", buf);

    /*
     * skip spaces 
     */
    while (buf < endbuf && isspace(*buf)) {
        eol = (*buf == '\n');
        buf++;
    }

    // fprintf(stderr, "buf=%.32s\n", buf);

    assert(buf < endbuf);
    return buf;
}


/*
 ********************************************************************** */
/*
 * create/read a rgb image from the given file.
 */

RGBImage       *
create_rgb_image(const char *filename)
{
    RGBImage       *img;
    int             err;
    FILE           *fp;
    char           *buf;
    int             buflen;
    int             width,
                    height;
    image_type_t    magic;
    int             headerlen;
    int             bytes;
    const int       read_buffer_size = 128 << 10;

    fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        return NULL;
    }

    buf = (char *) malloc(read_buffer_size);
    assert(buf);

    buflen = fread(buf, 1, read_buffer_size, fp);
    if (!buflen) {
        perror(filename);
        return NULL;
    }

    /*
     * read the header and figure out the dimensions 
     */
    err = pnm_parse_header(buf, buflen, &width, &height, &magic, &headerlen);
    if (err) {
        fprintf(stderr, "%s: parse error\n", filename);
        return NULL;
    }

    if (magic != IMAGE_PPM) {
        fprintf(stderr, "%s: only ppm format supported\n", filename);
        return NULL;
    }

    /*
     * create image to hold the data 
     */
    bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);
    img = (RGBImage *) malloc(bytes);
    if (!img) {
        fprintf(stderr, "out of memory!\n");
        return NULL;
    }
    img->nbytes = bytes;
    img->height = height;
    img->width = width;
    img->type = magic;

    /*
     * read the data into img 
     */
    {
        pnm_state_t    *state;
        char           *bufp = &buf[headerlen]; /* skip header */
        state = pnm_state_new(img);
        buflen -= headerlen;
        do {
            err = ppm_add_data(state, bufp, buflen);
            if (err) {
                free(img);
                img = NULL;
                fprintf(stderr, "error\n");
            }
            /*
             * done with this bufp; 
             */
            bufp = buf;

            // fprintf(stderr, "added %d bytes\n", buflen);
            buflen = fread(bufp, 1, read_buffer_size, fp);
            // fprintf(stderr, "read %d bytes\n", buflen);

        } while (!err && buflen);
        // fprintf(stderr, "complete\n");
        pnm_state_delete(state);
    }

    free(buf);
    fclose(fp);
    // fprintf(stderr, "done\n");
    return img;
}


int
rgb_write_image(RGBImage * img, const char *filename, const char *dir)
{
    int             err;
    FILE           *fp;
    int             i;
    char            path[COMMON_MAX_PATH];


    /*
     * get the full path name 
     */
    err = snprintf(path, COMMON_MAX_PATH, "%s/%s", dir, filename);
    if (err >= COMMON_MAX_PATH) {
        fprintf(stderr, "MAX path exeeded, need at least %d bytes \n", err);
        assert(0);
    }


    fp = fopen(path, "w");
    if (!fp) {
        perror(path);
        return (EINVAL);
    }

    /*
     * now we write out the header 
     */
    fprintf(fp, "P6 \n");
    fprintf(fp, "%d %d \n", img->width, img->height);
    fprintf(fp, "255\n");

    for (i = 0; i < (img->height * img->width); i++) {
        err = fwrite(&img->data[i], 3, 1, fp);
	assert(err == 1);
    }

    fclose(fp);
    return (0);
}



/*
 ********************************************************************** */
/*
 * a collection of interfaces to incrementally build an RGBImage from
 * a stream of bytes.
 */

pnm_state_t    *
pnm_state_new(RGBImage * img)
{
    pnm_state_t    *state;

    state = (pnm_state_t *) malloc(sizeof(pnm_state_t));
    assert(state);
    state->img = img;
    state->parity = 0;
    state->bytes_remaining = img->width * img->height * 3;  /* 3 bytes per
                                                             * pixel */
    assert(img->nbytes > (size_t) (img->width * img->height * 4));
    state->img_cur = (uint8_t *) state->img->data;

    /*
     * fprintf(stderr, "pnm: bytes_remaining = %d; img capacity = %d\n",
     * state->bytes_remaining, state->img->nbytes); 
     */

    return state;
}


void
pnm_state_delete(pnm_state_t * state)
{
    free(state);
}



/*
 * read a portable anymap header from a bunch of bytes
 * returns 0 or error status
 */
int
pnm_parse_header(char *buf, size_t buflen,
                 int *width, int *height,
                 image_type_t * magic, int *headerlen)
{
    int             err = 0;
    char           *endptr;
    char           *startbuf,
                   *endbuf;
    int             maxval;

    /*
     * we are assuming that buflen > sizeof the whole header XXX 
     */

    startbuf = buf;
    endbuf = buf + buflen;

    if (strncmp(buf, "P5", 2) == 0) {
        *magic = IMAGE_PGM;
    } else if (strncmp(buf, "P6", 2) == 0) {
        *magic = IMAGE_PPM;
    } else {
        *magic = IMAGE_UNKNOWN;
        assert(0 && "bad header");
        return 1;
    }
    // fprintf(stderr, "read_header: got pgm file\n");

    /*
     * we now assume the file is well-formed XXX 
     */
    buf = skip_space_comments(buf + 2, endbuf);

    /*
     * width, whitespace 
     */
    *width = strtol(buf, &endptr, 0);
    if (*width == 0) {
        fprintf(stderr, "invalid image width (%.70s)\n", buf);
        assert(0);
    }
    buf = skip_space_comments(endptr, endbuf);

    /*
     * height, whitespace 
     */
    *height = strtol(buf, &endptr, 0);
    if (*height == 0) {
        fprintf(stderr, "invalid image height (%.70s)\n", buf);
        assert(0);
    }
    buf = skip_space_comments(endptr, endbuf);

    /*
     * maxval 
     */
    maxval = strtol(buf, &endptr, 0);
    buf = endptr;

    /*
     * single whitespace 
     */
    buf++;

    assert(buf - startbuf < (int) buflen);

    *headerlen = buf - startbuf;

    return err;
}



/*
 * read ppm-data from file, and fill in data. 
 */
int
ppm_add_data(pnm_state_t * state, char *buf, size_t nb)
{
    uint8_t        *fdata = (uint8_t *) buf;
    uint8_t        *imgdata = state->img_cur;
    int             parity = state->parity;

    assert(nb <= state->bytes_remaining);
    state->bytes_remaining -= nb;
    while (nb--) {
        *(imgdata++) = *(fdata++);
        parity++;
        if (parity == 3) {      /* XXX assumes structure of RGBPixel */
            *(imgdata++) = 255; /* fake alpha value */
            parity = 0;
        }
    }
    // fprintf(stderr, "rgb grew %d bytes\n", imgdata - state->img_cur);
    state->img_cur = imgdata;
    state->parity = parity;
    return 0;
}


/*
 ********************************************************************** */
/*
 * create/read a Ipl Image (OpenCV) from the given file and convert to gray
 * scale.
 */

IplImage       *
create_gray_ipl_image(char *filename)
{
    IplImage       *rgba_img;
    IplImage       *gray_img;
    RGBImage       *img;
    int             err;
    FILE           *fp;
    char           *buf;
    int             buflen;
    int             width,
                    height;
    image_type_t    magic;
    int             headerlen;
    int             bytes;
    const int       read_buffer_size = 128 << 10;

    fp = fopen(filename, "r");

    if (!fp) {
        perror(filename);
        return NULL;
    }

    buf = (char *) malloc(read_buffer_size);
    assert(buf);

    buflen = fread(buf, 1, read_buffer_size, fp);
    if (!buflen) {
        perror(filename);
        return NULL;
    }

    /*
     * read the header and figure out the dimensions 
     */
    err = pnm_parse_header(buf, buflen, &width, &height, &magic, &headerlen);
    if (err) {
        fprintf(stderr, "%s: parse error\n", filename);
        return NULL;
    }
    // fprintf(stderr, "create_rgb_image: width=%d, height=%d\n", width,
    // height); /* XXX */

    if (magic != IMAGE_PPM) {
        fprintf(stderr, "%s: only ppm format supported\n", filename);
        return NULL;
    }

    /*
     * create image to hold the data 
     */
    bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);
    img = (RGBImage *) malloc(bytes);
    if (!img) {
        fprintf(stderr, "out of memory!\n");
        return NULL;
    }
    img->nbytes = bytes;
    img->height = height;
    img->width = width;
    img->type = magic;

    /*
     * read the data into img 
     */
    {
        pnm_state_t    *state;
        state = pnm_state_new(img);
        char           *bufp = &buf[headerlen];
        buflen -= headerlen;
        do {
            err = ppm_add_data(state, bufp, buflen);
            if (err) {
                free(img);
                img = NULL;
                fprintf(stderr, "error\n");
            }
            // fprintf(stderr, "added %d bytes\n", buflen);
            buflen = fread(bufp, 1, read_buffer_size, fp);
            // fprintf(stderr, "read %d bytes\n", buflen);
        } while (!err && buflen);
        // fprintf(stderr, "complete\n");
        pnm_state_delete(state);
    }

    free(buf);
    fclose(fp);

    /*
     * create rgba ipl image 
     */
    rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
    memcpy(rgba_img->imageData, img->data, rgba_img->imageSize);
    // rgba_img->imageData = (char*)&img->data;

    /*
     * create grayscale image from rgb image 
     */
    gray_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    cvCvtColor(rgba_img, gray_img, CV_RGBA2GRAY);

    cvReleaseImage(&rgba_img);
    release_rgb_image(img);

    return gray_img;
}


IplImage       *
create_rgb_ipl_image(char *filename)
{
    IplImage       *rgba_img;
    IplImage       *ipl_rgb_img;
    RGBImage       *img;
    // int err;
    // FILE *fp;
    // char *buf;
    // int buflen;
    int             width,
                    height;
    // image_type_t magic;
    // int headerlen;
    // int bytes;
    // const int read_buffer_size = 128<<10;

    img = create_rgb_image(filename);

    height = img->height;
    width = img->width;

    /*
     * create rgba ipl image 
     */
    rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
    memcpy(rgba_img->imageData, img->data, rgba_img->imageSize);
    // rgba_img->imageData = (char*)&img->data;

    /*
     * create grayscale image from rgb image (HUH?? -RW) 
     */
    ipl_rgb_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
    cvCvtColor(rgba_img, ipl_rgb_img, CV_RGBA2RGB);

    cvReleaseImage(&rgba_img);

    release_rgb_image(img);

    return ipl_rgb_img;
}


IplImage       *
get_gray_ipl_image(RGBImage * rgb_img)
{
    IplImage       *ipl_rgba_img;
    IplImage       *ipl_gray_img;
    int             width = rgb_img->width;
    int             height = rgb_img->height;

    ipl_rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
    ipl_gray_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

    /*
     * read the data into img 
     */

    memcpy(ipl_rgba_img->imageData, rgb_img->data, ipl_rgba_img->imageSize);

    /*
     * create grayscale image from rgb image 
     */
    cvCvtColor(ipl_rgba_img, ipl_gray_img, CV_RGBA2GRAY);
    cvReleaseImage(&ipl_rgba_img);
    return ipl_gray_img;
}

IplImage       *
get_rgb_ipl_image(RGBImage * rgb_img)
{
    IplImage       *ipl_rgba_img;
    IplImage       *ipl_rgb_img;
    int             width = rgb_img->width;
    int             height = rgb_img->height;

    ipl_rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
    ipl_rgb_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

    /*
     * read the data into img 
     */

    memcpy(ipl_rgba_img->imageData, rgb_img->data, ipl_rgba_img->imageSize);

    /*
     * create grayscale image from rgb image 
     */
    cvCvtColor(ipl_rgba_img, ipl_rgb_img, CV_RGBA2RGB);
    cvReleaseImage(&ipl_rgba_img);

    return ipl_rgb_img;
}

RGBImage *
convert_ipl_to_rgb(IplImage * ipl)
{
    IplImage       *ipl_rgba_img;
    int             width = ipl->width;
    int             height = ipl->height;
	RGBImage *		rgb;
	int				bytes;
	int				i;


    ipl_rgba_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
    cvCvtColor(ipl, ipl_rgba_img, CV_RGB2RGBA);

    bytes = sizeof(RGBImage) + width * height * sizeof(RGBPixel);
	rgb = (RGBImage *)malloc(bytes);

    rgb->nbytes = bytes;
    rgb->height = height;
    rgb->width = width;
    rgb->type = IMAGE_PPM;

    memcpy(rgb->data, ipl_rgba_img->imageData, ipl_rgba_img->imageSize);

    /*
     * create grayscale image from rgb image 
     */
    cvReleaseImage(&ipl_rgba_img);

	/* set all the alpha to 255 */
	for (i=0; i < (width * height); i++) {
		char 	tmp;
		tmp = rgb->data[i].r;
		rgb->data[i].r = rgb->data[i].b;
		rgb->data[i].b = tmp;
		rgb->data[i].a = 255;
	}

    return(rgb);
}


void
img_constrain_bbox(bbox_t * bbox, RGBImage * img)
{
                                                                              
    if (bbox->min_x >= img->width) {
        bbox->min_x = img->width - 1;
    }
    if (bbox->min_y >= img->height) {
        bbox->min_y = img->height - 1;
    }
                                                                              
    if (bbox->max_x >= img->width) {
        bbox->max_x = img->width - 1;
    }
    if (bbox->max_y >= img->height) {
        bbox->max_y = img->height - 1;
    }
}

