/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_GUI_THREAD_H_
#define	_GUI_THREAD_H_


//#define DISABLE_G_LOCKING 1
//#define DISABLE_G_LOCK_ASSERTS 1



#ifndef DISABLE_G_LOCKING

extern int __gui_enter_flag;
extern int __gui_callback_flag;
extern pthread_t __gui_thread_id;

#ifndef THREAD_NULL
#define THREAD_NULL (pthread_t)(-1)
#endif

#ifndef DISABLE_G_LOCK_ASSERTS
#define ASSERT(x)  assert(x)
#else
#define ASSERT(x) 
#endif

#define GUI_THREAD_INIT()			\
	g_thread_init(NULL);			\
	gdk_threads_init();


#define	GUI_THREAD_ENTER()			\
{						\
	pthread_testcancel();			\
	gdk_threads_enter();			\
	ASSERT(__gui_enter_flag == 0);		\
	__gui_enter_flag = 1;			\
        __gui_thread_id = pthread_self();	\
}

#define	GUI_THREAD_LEAVE()			\
{						\
	ASSERT(__gui_enter_flag == 1);		\
	__gui_enter_flag = 0;			\
	gdk_flush();				\
        __gui_thread_id = THREAD_NULL;		\
	gdk_threads_leave();			\
}

#define	GUI_THREAD_LEFT()			\
{						\
	__gui_enter_flag = 0;			\
	gdk_flush();				\
        __gui_thread_id = THREAD_NULL;		\
	gdk_threads_leave();			\
}



#define	GUI_THREAD_CHECK()						\
{									\
	ASSERT((__gui_enter_flag == 1) || (__gui_callback_flag == 1));	\
        ASSERT(pthread_equal(__gui_thread_id, pthread_self())); \
}


#define	GUI_CALLBACK_ENTER()			\
{						\
        ASSERT(__gui_enter_flag == 0);		\
        ASSERT(__gui_callback_flag == 0);	\
	(__gui_callback_flag = 1);		\
        __gui_thread_id = pthread_self();	\
}

#define	GUI_CALLBACK_LEAVE()				\
{							\
        ASSERT(__gui_callback_flag == 1);		\
	(__gui_callback_flag = 0);			\
        __gui_thread_id = THREAD_NULL;		\
}						

#define MAIN_THREADS_ENTER() 			\
{ 						\
	gdk_threads_enter(); 			\
        __gui_thread_id = pthread_self();	\
}

#define MAIN_THREADS_LEAVE() 			\
{ 						\
        __gui_thread_id = THREAD_NULL;		\
	gdk_threads_leave();			\
}

#else  /* no locking */

#define GUI_THREAD_INIT()    {}
#define GUI_THREAD_ENTER()   {}
#define GUI_THREAD_LEAVE()   {}
#define GUI_THREAD_CHECK()   {}
#define GUI_CALLBACK_ENTER() {}
#define GUI_CALLBACK_LEAVE() {}
#define MAIN_THREADS_ENTER()  {}
#define MAIN_THREADS_LEAVE()  {}

#endif

#endif	/* ! _GUI_THREAD_H_ */

