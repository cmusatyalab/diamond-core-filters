
#ifndef _FIL_TOOLS_H_
#define _FIL_TOOLS_H_

#include "filter_api.h"
#include "face.h"

#ifdef __cplusplus
extern "C" {
#endif

char *ft_read_alloc_attr(lf_fhandle_t fhandle, lf_obj_handle_t ohandle, 
			 const char *name);
void ft_free(lf_fhandle_t fhandle, char *ptr);


/* 
 * integral image
 */
typedef struct ii_image {
	size_t    nbytes;	/* size of variable struct */
	dim_t     width, height;
	u_int32_t data[0];	/* variable size struct. width*height
				 * elements of data follows. */
} ii_image_t;


//extern u_int32_t ii_probe(ii_image_t *ii, dim_t x, dim_t y);

#if 0
#define II_PROBE(ii,x,y) ii_probe(ii, x, y)
#endif

#define II_PROBE(ii,x,y) 			\
(assert((x) >= 0), assert((y) >= 0),           \
 assert((x) < (ii)->width),			\
 assert((y) < (ii)->height),			\
 (ii)->data[(y) * ((ii)->width) + (x)])

typedef struct ii2_image {
	size_t    nbytes;	/* size of variable struct */
	dim_t     width, height;
	float    data[0];	/* variable size struct. width*height
				 * elements of data follows. */
} ii2_image_t;



int write_param(lf_fhandle_t fhandle, lf_obj_handle_t ohandle, char *fmt,
		search_param_t *param, int i);
int read_param(lf_fhandle_t fhandle, lf_obj_handle_t ohandle, char *fmt,
	       search_param_t *param, int i);


/* ********************************************************************** */

int log2_int(int x);

#ifdef __cplusplus
}
#endif


#endif /*  _FIL_TOOLS_H_ */
