#ifndef COSMOPOLITAN_LIBC_RUNTIME_PTHREAD_H_
#define COSMOPOLITAN_LIBC_RUNTIME_PTHREAD_H_
#include "libc/calls/struct/timespec.h"
#include "libc/intrin/kprintf.h"
#if !(__ASSEMBLER__ + __LINKER__ + 0)
COSMOPOLITAN_C_START_

#define PTHREAD_ONCE_INIT 0

#define PTHREAD_MUTEX_DEFAULT    PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_NORMAL     0
#define PTHREAD_MUTEX_RECURSIVE  1
#define PTHREAD_MUTEX_ERRORCHECK 2
#define PTHREAD_MUTEX_STALLED    0
#define PTHREAD_MUTEX_ROBUST     1

/* clang-format off */
#define PTHREAD_MUTEX_INITIALIZER {PTHREAD_MUTEX_DEFAULT}
#define PTHREAD_RWLOCK_INITIALIZER {{{0}}}
#define PTHREAD_COND_INITIALIZER {{{0}}}
/* clang-format on */

typedef unsigned long *pthread_t;
typedef int pthread_once_t;

typedef struct {
  int attr;
  int reent;
  _Atomic(int) owner;
  _Atomic(int) waits;
} pthread_mutex_t;

typedef struct {
  int attr;
} pthread_mutexattr_t;

typedef struct {
  int attr;
} pthread_condattr_t;

typedef struct {
  int attr[2];
} pthread_rwlockattr_t;

typedef struct {
  union {
    int __i[9];
    volatile int __vi[9];
    unsigned __s[9];
  } __u;
} pthread_attr_t;

typedef struct {
  union {
    int __i[12];
    volatile int __vi[12];
    void *__p[12];
  } __u;
} pthread_cond_t;

typedef struct {
  union {
    int __i[8];
    volatile int __vi[8];
    void *__p[8];
  } __u;
} pthread_rwlock_t;

wontreturn void pthread_exit(void *);
pureconst pthread_t pthread_self(void);
int pthread_create(pthread_t *, const pthread_attr_t *, void *(*)(void *),
                   void *);
int pthread_yield(void);
int pthread_detach(pthread_t);
int pthread_join(pthread_t, void **);
int pthread_equal(pthread_t, pthread_t);
int pthread_once(pthread_once_t *, void (*)(void));
int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
int pthread_mutex_lock(pthread_mutex_t *);
int pthread_mutex_unlock(pthread_mutex_t *);
int pthread_mutex_trylock(pthread_mutex_t *);
int pthread_mutex_timedlock(pthread_mutex_t *, const struct timespec *);
int pthread_mutex_destroy(pthread_mutex_t *);
int pthread_mutex_consistent(pthread_mutex_t *);
int pthread_mutexattr_init(pthread_mutexattr_t *);
int pthread_mutexattr_destroy(pthread_mutexattr_t *);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *, int *);
int pthread_mutexattr_settype(pthread_mutexattr_t *, int);
int pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *);
int pthread_cond_destroy(pthread_cond_t *);
int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
int pthread_cond_timedwait(pthread_cond_t *, pthread_mutex_t *,
                           const struct timespec *);
int pthread_cond_broadcast(pthread_cond_t *);
int pthread_cancel(pthread_t);
int pthread_cond_signal(pthread_cond_t *);
int pthread_rwlock_init(pthread_rwlock_t *, const pthread_rwlockattr_t *);
int pthread_rwlock_destroy(pthread_rwlock_t *);
int pthread_rwlock_rdlock(pthread_rwlock_t *);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *);
int pthread_rwlock_timedrdlock(pthread_rwlock_t *, const struct timespec *);
int pthread_rwlock_wrlock(pthread_rwlock_t *);
int pthread_rwlock_trywrlock(pthread_rwlock_t *);
int pthread_rwlock_timedwrlock(pthread_rwlock_t *, const struct timespec *);
int pthread_rwlock_unlock(pthread_rwlock_t *);

COSMOPOLITAN_C_END_
#endif /* !(__ASSEMBLER__ + __LINKER__ + 0) */
#endif /* COSMOPOLITAN_LIBC_RUNTIME_PTHREAD_H_ */
