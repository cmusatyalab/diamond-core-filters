#ifndef RGB_H
#define RGB_H

#include <stdint.h>
#include <sys/types.h>
#include "lib_filter.h"


typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;	// Alpha value -- mainly just padding
} RGBPixel;



typedef enum image_type_t {
  IMAGE_UNKNOWN = 0,
  IMAGE_PGM,
  IMAGE_PPM
} image_type_t;


typedef struct {
  image_type_t type;
  size_t nbytes;			/* size of this var size struct */
	union {
		int height;
		int rows;
	};
	union {
		int width;
		int columns;
	};
  RGBPixel data[0];		/* var size struct */
} RGBImage;


/* some colors/masks */

static const RGBPixel red = {255, 0, 0, 255};
static const RGBPixel green = {32, 255, 32, 255};
static const RGBPixel blue = {32, 32, 255, 255};
static const RGBPixel clearColor = {0, 0, 0, 0};
static const RGBPixel colorMask = {1, 1, 1, 1};
static const RGBPixel clearMask = {0, 0, 0, 0};

static const RGBPixel hilit =     {255, 255, 255, 128};
static const RGBPixel hilitRed = {255, 0, 0, 32};
static const RGBPixel hilitMask = {1, 1, 1, 1};


#ifdef __cplusplus
extern "C" {
#endif

	

/* make a new image the same size as src */
RGBImage *rgbimg_new(RGBImage *srcimg);

/* make a new image duplicating src */
RGBImage *rgbimg_dup(RGBImage *srcimg);

/* wipe image clean */
void rgbimg_clear(RGBImage *img);

                                                                               
RGBImage * get_rgb_img(lf_obj_handle_t ohandle);


#ifdef __cplusplus
}
#endif

#endif // RGB_H
