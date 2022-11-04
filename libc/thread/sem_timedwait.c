/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2022 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/assert.h"
#include "libc/calls/struct/timespec.h"
#include "libc/calls/struct/timespec.internal.h"
#include "libc/errno.h"
#include "libc/intrin/atomic.h"
#include "libc/limits.h"
#include "libc/sysv/errfuns.h"
#include "libc/thread/semaphore.h"
#include "third_party/nsync/futex.internal.h"

static void sem_delay(int n) {
  volatile int i;
  for (i = 0; i != 1 << n; i++) donothing;
}

// TODO(jart): This should be abstracted by polyfill.
static struct timespec *sem_timeout(struct timespec *memory,
                                    const struct timespec *abstime) {
  struct timespec now;
  if (!abstime) {
    return 0;
  } else if (FUTEX_TIMEOUT_IS_ABSOLUTE) {
    *memory = *abstime;
    return memory;
  } else {
    now = _timespec_real();
    if (_timespec_cmp(now, *abstime) > 0) {
      *memory = (struct timespec){0};
    } else {
      *memory = _timespec_sub(*abstime, now);
    }
    return memory;
  }
}

/**
 * Locks semaphore w/ deadline.
 *
 * @param abstime is absolute deadline or null to wait forever
 * @return 0 on success, or -1 w/ errno
 * @raise EINTR if signal was delivered instead
 * @raise EDEADLK if deadlock was detected
 * @raise ETIMEDOUT if deadline expired
 * @raise EINVAL if `sem` is invalid
 * @cancellationpoint
 */
int sem_timedwait(sem_t *sem, const struct timespec *abstime) {
  int e, i, v, rc;
  struct timespec ts;

  e = errno;
  for (i = 0; i < 7; ++i) {
    rc = sem_trywait(sem);
    if (!rc) {
      return rc;
    } else if (errno == EAGAIN) {
      errno = e;
      sem_delay(i);
    } else {
      return rc;
    }
  }

  _unassert(atomic_fetch_add_explicit(&sem->sem_waiters, +1,
                                      memory_order_acquire) >= 0);

  do {
    if (!(v = atomic_load_explicit(&sem->sem_value, memory_order_relaxed))) {
      rc = nsync_futex_wait_(&sem->sem_value, v, sem->sem_pshared,
                             sem_timeout(&ts, abstime));
      if (rc == -EINTR) {
        rc = eintr();
      } else if (rc == -EAGAIN || rc == -EWOULDBLOCK) {
        rc = 0;
      } else if (rc == -ETIMEDOUT) {
        _npassert(abstime);
        if (_timespec_cmp(*abstime, _timespec_real()) <= 0) {
          rc = etimedout();
        } else {
          rc = 0;
        }
      } else {
        _npassert(!rc);
        rc = 0;
      }
    } else if (v > 0) {
      rc = 0;
    } else {
      _unassert(v > INT_MIN);
      rc = einval();
    }
  } while (!rc && (!v || !atomic_compare_exchange_weak_explicit(
                             &sem->sem_value, &v, v - 1, memory_order_acquire,
                             memory_order_relaxed)));

  _unassert(atomic_fetch_add_explicit(&sem->sem_waiters, -1,
                                      memory_order_release) > 0);

  return rc;
}
