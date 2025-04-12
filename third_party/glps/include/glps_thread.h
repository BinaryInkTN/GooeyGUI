
#ifndef GLPS_THREAD_H
#define GLPS_THREAD_H

#if defined(GLPS_USE_WIN32)
#define GLPS_THREAD_WIN32
#elif (defined(GLPS_USE_WAYLAND) || defined(GLPS_USE_X11))
#define GLPS_THREAD_POSIX
#endif

#ifdef GLPS_THREAD_WIN32
#include <windows.h>
typedef HANDLE gthread_t;
typedef DWORD gthread_attr_t;
typedef CRITICAL_SECTION gthread_mutex_t;
typedef int gthread_mutexattr_t;
typedef CONDITION_VARIABLE gthread_cond_t;
typedef int gthread_condattr_t;
#else
#include <pthread.h>
typedef pthread_t gthread_t;
typedef pthread_attr_t gthread_attr_t;
typedef pthread_mutex_t gthread_mutex_t;
typedef pthread_mutexattr_t gthread_mutexattr_t;
typedef pthread_cond_t gthread_cond_t;
typedef pthread_condattr_t gthread_condattr_t;
#endif

int glps_thread_create(gthread_t *thread, const gthread_attr_t *attr,
                       void *(*start_routine)(void *), void *arg);
int glps_thread_join(gthread_t thread, void **retval);
int glps_thread_detach(gthread_t thread);
void glps_thread_exit(void *retval);
gthread_t glps_thread_self(void);
int glps_thread_equal(gthread_t t1, gthread_t t2);

int glps_thread_mutex_init(gthread_mutex_t *mutex, const gthread_mutexattr_t *attr);
int glps_thread_mutex_destroy(gthread_mutex_t *mutex);
int glps_thread_mutex_lock(gthread_mutex_t *mutex);
int glps_thread_mutex_unlock(gthread_mutex_t *mutex);
int glps_thread_mutex_trylock(gthread_mutex_t *mutex);

int glps_thread_cond_init(gthread_cond_t *cond, const gthread_condattr_t *attr);
int glps_thread_cond_destroy(gthread_cond_t *cond);
int glps_thread_cond_wait(gthread_cond_t *cond, gthread_mutex_t *mutex);
int glps_thread_cond_signal(gthread_cond_t *cond);
int glps_thread_cond_broadcast(gthread_cond_t *cond);

int glps_thread_attr_init(gthread_attr_t *attr);
int glps_thread_attr_destroy(gthread_attr_t *attr);
int glps_thread_attr_setdetachstate(gthread_attr_t *attr, int detachstate);
int glps_thread_attr_getdetachstate(const gthread_attr_t *attr, int *detachstate);

#endif