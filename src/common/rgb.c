
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "rgb.h"

RGBImage       *
rgbimg_new(RGBImage * imgsrc)
{
    RGBImage       *img;

    img = (RGBImage *) calloc(imgsrc->nbytes, 1);
    assert(img);
    memcpy(img, imgsrc, sizeof(RGBImage));  /* only copy header */

    return img;
}

RGBImage       *
rgbimg_dup(RGBImage * imgsrc)
{
    RGBImage       *img;

    img = (RGBImage *) malloc(imgsrc->nbytes);
    assert(img);
    memcpy(img, imgsrc, imgsrc->nbytes);

    return img;
}

void
rgbimg_clear(RGBImage * img)
{
    size_t          nb;

    nb = img->width * img->height * sizeof(RGBPixel);
    memset(img->data, 0, nb);
}
