
#include <pthread.h>
#include "gui_thread.h"

#ifndef DISABLE_G_LOCKING

int             __gui_enter_flag = 0;
int             __gui_callback_flag = 0;
pthread_t       __gui_thread_id = THREAD_NULL;

#endif
