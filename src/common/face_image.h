#ifndef	_FACE_IMAGE_H_
#define	_FACE_IMAGE_H_


#include "face_tools.h"
#include "rgb.h"

#ifdef __cplusplus
extern "C" {
#endif

	

RGBImage* image_gen_image_scale(RGBImage *, int scale);
void image_draw_bbox_scale(RGBImage *, bbox_t *bbox, int scale,
				  RGBPixel mask, RGBPixel color);

void image_fill_bbox_scale(RGBImage *, bbox_t *bbox, int scale,
				  RGBPixel mask, RGBPixel color);


#ifdef __cplusplus
}
#endif

#endif	/* _FACE_IMAGE_H_ */

