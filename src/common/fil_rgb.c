
/*
 * color histogram filter * rgb reader
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#include "face.h"
#include "common_consts.h"
#include "filter_api.h"
#include "fil_file.h"
#include "fil_image_tools.h"
#include "rgb.h"
#include "histo.h"
#include "fil_histo.h"



#define ASSERT(exp)							\
if(!(exp)) {								\
  lf_log(fhandle, LOGL_ERR, "Assertion %s failed at ", #exp);		\
  lf_log(fhandle, LOGL_ERR, "%s, line %d.", __FILE__, __LINE__);	\
  pass = -1;								\
  goto done;								\
}

int
f_init_attr2rgb(int numarg, char **args, int blob_len, void *blob, void **data)
{

    assert(numarg == 0);
    *data = NULL;
    return (0);
}

int
f_fini_attr2rgb(void *data)
{
    return (0);
}

/*
 * filter eval function to create an RGB_IMAGE attribute
 */

int
f_eval_attr2rgb(lf_obj_handle_t ohandle, int numout,
               lf_obj_handle_t * ohandles, void *user_data)
{
    RGBImage       *img;
    int             err = 0, pass = 1;
    lf_fhandle_t    fhandle = 0;

    lf_log(fhandle, LOGL_TRACE, "f_eval_attr2rgb: enter");


    img = get_attr_rgb_img(ohandle, "DATA0");
    if (img == NULL) {
		return(0);
    }

    /*
     * save some attribs 
     */
    lf_write_attr(fhandle, ohandle, ROWS, sizeof(int), (char *) &img->height);
    lf_write_attr(fhandle, ohandle, COLS, sizeof(int), (char *) &img->width);

    /*
     * save img as an attribute 
     */
    err =
        lf_write_attr(fhandle, ohandle, RGB_IMAGE, img->nbytes, (char *) img);
    ASSERT(!err);
done:
    if (img)
        lf_free_buffer(fhandle, (char *) img);
    lf_log(fhandle, LOGL_TRACE, "f_pnm2rgb: done");
    return pass;
}


