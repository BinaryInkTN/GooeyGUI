#include "glps_thread.h"
#include <stdlib.h>

#ifdef GLPS_THREAD_WIN32

struct thread_wrapper_args
{
    void *(*start_routine)(void *);
    void *arg;
};

static DWORD WINAPI thread_wrapper(LPVOID lpParam)
{
    struct thread_wrapper_args *args = (struct thread_wrapper_args *)lpParam;
    void *result = args->start_routine(args->arg);
    free(args);
    return (DWORD)(uintptr_t)result;
}

int glps_thread_create(gthread_t *thread, const gthread_attr_t *attr,
                       void *(*start_routine)(void *), void *arg)
{
    struct thread_wrapper_args *args = malloc(sizeof(*args));
    if (!args)
        return ERROR_NOT_ENOUGH_MEMORY;

    args->start_routine = start_routine;
    args->arg = arg;

    *thread = CreateThread(NULL, 0, thread_wrapper, args, 0, NULL);
    if (*thread == NULL)
    {
        int err = GetLastError();
        free(args);
        return err;
    }
    return 0;
}

int glps_thread_join(gthread_t thread, void **retval)
{
    DWORD result = WaitForSingleObject(thread, INFINITE);
    if (result != WAIT_OBJECT_0)
        return GetLastError();

    if (retval)
    {
        DWORD exit_code;
        GetExitCodeThread(thread, &exit_code);
        *retval = (void *)(uintptr_t)exit_code;
    }

    CloseHandle(thread);
    return 0;
}

int glps_thread_detach(gthread_t thread)
{
    CloseHandle(thread);
    return 0;
}

void glps_thread_exit(void *retval)
{
    ExitThread((DWORD)(uintptr_t)retval);
}

gthread_t glps_thread_self(void)
{
    return GetCurrentThread();
}

int glps_thread_equal(gthread_t t1, gthread_t t2)
{
    return GetThreadId(t1) == GetThreadId(t2);
}

#else

int glps_thread_create(gthread_t *thread, const gthread_attr_t *attr,
                       void *(*start_routine)(void *), void *arg)
{
    return pthread_create(thread, attr, start_routine, arg);
}

int glps_thread_join(gthread_t thread, void **retval)
{
    return pthread_join(thread, retval);
}

int glps_thread_detach(gthread_t thread)
{
    return pthread_detach(thread);
}

void glps_thread_exit(void *retval)
{
    pthread_exit(retval);
}

gthread_t glps_thread_self(void)
{
    return pthread_self();
}

int glps_thread_equal(gthread_t t1, gthread_t t2)
{
    return pthread_equal(t1, t2);
}

#endif

#ifdef GLPS_THREAD_WIN32

int glps_thread_mutex_init(gthread_mutex_t *mutex, const gthread_mutexattr_t *attr)
{
    (void)attr;
    InitializeCriticalSection(mutex);
    return 0;
}

int glps_thread_mutex_destroy(gthread_mutex_t *mutex)
{
    DeleteCriticalSection(mutex);
    return 0;
}

int glps_thread_mutex_lock(gthread_mutex_t *mutex)
{
    EnterCriticalSection(mutex);
    return 0;
}

int glps_thread_mutex_unlock(gthread_mutex_t *mutex)
{
    LeaveCriticalSection(mutex);
    return 0;
}

int glps_thread_mutex_trylock(gthread_mutex_t *mutex)
{
    return TryEnterCriticalSection(mutex) ? 0 : EBUSY;
}

#else

int glps_thread_mutex_init(gthread_mutex_t *mutex, const gthread_mutexattr_t *attr)
{
    return pthread_mutex_init(mutex, attr);
}

int glps_thread_mutex_destroy(gthread_mutex_t *mutex)
{
    return pthread_mutex_destroy(mutex);
}

int glps_thread_mutex_lock(gthread_mutex_t *mutex)
{
    return pthread_mutex_lock(mutex);
}

int glps_thread_mutex_unlock(gthread_mutex_t *mutex)
{
    return pthread_mutex_unlock(mutex);
}

int glps_thread_mutex_trylock(gthread_mutex_t *mutex)
{
    return pthread_mutex_trylock(mutex);
}

#endif

#ifdef GLPS_THREAD_WIN32

int glps_thread_cond_init(gthread_cond_t *cond, const gthread_condattr_t *attr)
{
    (void)attr;
    InitializeConditionVariable(cond);
    return 0;
}

int glps_thread_cond_destroy(gthread_cond_t *cond)
{

    (void)cond;
    return 0;
}

int glps_thread_cond_wait(gthread_cond_t *cond, gthread_mutex_t *mutex)
{
    return SleepConditionVariableCS(cond, mutex, INFINITE) ? 0 : GetLastError();
}

int glps_thread_cond_signal(gthread_cond_t *cond)
{
    WakeConditionVariable(cond);
    return 0;
}

int glps_thread_cond_broadcast(gthread_cond_t *cond)
{
    WakeAllConditionVariable(cond);
    return 0;
}

#else

int glps_thread_cond_init(gthread_cond_t *cond, const gthread_condattr_t *attr)
{
    return pthread_cond_init(cond, attr);
}

int glps_thread_cond_destroy(gthread_cond_t *cond)
{
    return pthread_cond_destroy(cond);
}

int glps_thread_cond_wait(gthread_cond_t *cond, gthread_mutex_t *mutex)
{
    return pthread_cond_wait(cond, mutex);
}

int glps_thread_cond_signal(gthread_cond_t *cond)
{
    return pthread_cond_signal(cond);
}

int glps_thread_cond_broadcast(gthread_cond_t *cond)
{
    return pthread_cond_broadcast(cond);
}

#endif

#ifdef GLPS_THREAD_WIN32

int glps_thread_attr_init(gthread_attr_t *attr)
{
    *attr = 0;
    return 0;
}

int glps_thread_attr_destroy(gthread_attr_t *attr)
{
    (void)attr;
    return 0;
}

int glps_thread_attr_setdetachstate(gthread_attr_t *attr, int detachstate)
{
    (void)attr;
    (void)detachstate;
    return 0;
}

int glps_thread_attr_getdetachstate(const gthread_attr_t *attr, int *detachstate)
{
    (void)attr;
    *detachstate = 0;
    return 0;
}

#else

int glps_thread_attr_init(gthread_attr_t *attr)
{
    return pthread_attr_init(attr);
}

int glps_thread_attr_destroy(gthread_attr_t *attr)
{
    return pthread_attr_destroy(attr);
}

int glps_thread_attr_setdetachstate(gthread_attr_t *attr, int detachstate)
{
    return pthread_attr_setdetachstate(attr, detachstate);
}

int glps_thread_attr_getdetachstate(const gthread_attr_t *attr, int *detachstate)
{
    return pthread_attr_getdetachstate(attr, detachstate);
}

#endif