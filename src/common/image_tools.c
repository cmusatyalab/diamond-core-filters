
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <opencv/cv.h>
#include <gtk/gtk.h>

#include "face.h"
#include "histo.h"
#include "fil_histo.h"
#include "image_tools.h"
#include "sf_consts.h"

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
create_rgb_image(char *filename)
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
        state = pnm_state_new(img);
        char           *bufp = &buf[headerlen]; /* skip header */
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
    char            path[SNAP_MAX_PATH];


    /*
     * get the full path name 
     */
    err = snprintf(path, SNAP_MAX_PATH, "%s/%s", dir, filename);
    if (err >= SNAP_MAX_PATH) {
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
        fwrite(&img->data[i], 3, 1, fp);
    }

    fclose(fp);
    return (0);
}


RGBImage       *
create_rgb_subimage(RGBImage * old_img, int xoff, int yoff, int xsize,
                    int ysize)
{
    RGBImage       *new_img;
    int             bytes;
    const RGBPixel *oldp;
    RGBPixel       *newp;
    int             i,
                    j;


    assert(old_img->width >= (xoff + xsize));
    assert(old_img->height >= (yoff + ysize));

    bytes = sizeof(RGBImage) + xsize * ysize * sizeof(RGBPixel);
    new_img = (RGBImage *) malloc(bytes);

    assert(new_img != NULL);

    new_img->nbytes = bytes;
    new_img->height = ysize;
    new_img->width = xsize;
    new_img->type = old_img->type;


    for (j = 0; j < ysize; j++) {
        oldp = old_img->data + ((yoff + j) * old_img->width) + xoff;
        newp = new_img->data + (j * new_img->width);

        for (i = 0; i < xsize; i++) {
            *newp++ = *oldp++;
        }
    }

    return new_img;
}
void
release_rgb_image(RGBImage * img)
{
    free(img);
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
    if (buflen < 1023) {
        return (1);
    }

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
