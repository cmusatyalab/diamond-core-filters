
#include <stdio.h>

#include "rtimer.h"
#include "fil_tools.h"
#include "rgb.h"

/*
 * this is the wrong place for these... 
 */

#ifdef __cplusplus
extern          "C" {
#endif

    void            search_param_sprint(char *buf, search_param_t * param);
    void            int_sprint(char *buf, int *v);
    void            float_sprint(char *buf, float *v);
    void            ptr_sprint(char *buf, void *ptr);
    void            cstring_sprint(char *buf, char *str);
    void            time_sprint(char *buf, rtime_t * tm);

    void            ii_image_sprint(char *buf, ii_image_t * img);
    void            rgbimage_sprint(char *buf, RGBImage * img);

#ifdef __cplusplus
}
#endif
void
search_param_sprint(char *buf, search_param_t * param)
{
    sprintf(buf, "[lev=%d-%d; %d,%d, %dx%d; s=%f]", param->lev1, param->lev2,
            param->bbox.xmin, param->bbox.ymin, param->bbox.xsiz,
            param->bbox.ysiz, param->scale);
}

void
int_sprint(char *buf, int *v)
{
    sprintf(buf, "%d", *v);
}

void
float_sprint(char *buf, float *v)
{
    sprintf(buf, "%f", *v);
}


void
ptr_sprint(char *buf, void *ptr)
{
    sprintf(buf, "%p", ptr);
}


void
cstring_sprint(char *buf, char *str)
{
    sprintf(buf, "%s", str);
}


void
time_sprint(char *buf, rtime_t * tm)
{
    sprintf(buf, "%f", rt_time2secs(*tm));
}

void
ii_image_sprint(char *buf, ii_image_t * img)
{
    sprintf(buf, "%dx%d", img->width, img->height);
}

void
rgbimage_sprint(char *buf, RGBImage * img)
{
    sprintf(buf, "%dx%d", img->width, img->height);
}
