
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include <libgen.h>             /* dirname */
#include <assert.h>
#include <stdint.h>

#include <opencv/cv.h>

#include "queue.h"
#include "ring.h"
#include "rtimer.h"

// #include "face_search.h"
#include "face_tools.h"
#include "face_image.h"
#include "fil_face.h"
#include "rgb.h"
#include "face.h"
#include "fil_tools.h"
#include "gui_thread.h"
#include "image_tools.h"
#include "texture_tools.h"
#include "arena_tools.h"
#include "facedet.h"

/*
 ********************************************************************** */

static pthread_mutex_t ih_mutex = PTHREAD_MUTEX_INITIALIZER;


image_hooks_t  *
ih_new_ref(RGBImage * img, HistoII * histo_ii, ls_obj_handle_t ohandle)
{
    image_hooks_t  *ptr = (image_hooks_t *) calloc(1, sizeof(image_hooks_t));
    assert(ptr);
    ptr->refcount = 1;

    ptr->img = img;
    ptr->histo_ii = histo_ii;
    ptr->ohandle = ohandle;

    return ptr;
}

void
ih_get_ref(image_hooks_t * ptr)
{
    pthread_mutex_lock(&ih_mutex);
    ptr->refcount++;
    pthread_mutex_unlock(&ih_mutex);
}

void
ih_drop_ref(image_hooks_t * ptr, lf_fhandle_t fhandle)
{

    pthread_mutex_lock(&ih_mutex);
    assert(ptr->refcount > 0);
    ptr->refcount--;
    if (ptr->refcount == 0) {
        ft_free(fhandle, (char *) ptr->img);
        if (ptr->histo_ii) {
            ft_free(fhandle, (char *) ptr->histo_ii);
        }
        ls_release_object(fhandle, ptr->ohandle);
        free(ptr);
    }
    pthread_mutex_unlock(&ih_mutex);
}

