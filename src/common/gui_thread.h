/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

