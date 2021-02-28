/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2021 Justine Alexandra Roberts Tunney                              │
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
#include "libc/math.h"
#include "libc/runtime/gc.h"
#include "libc/testlib/testlib.h"
#include "libc/x/x.h"

#define asinhl(x) asinhl(VEIL("t", (long double)(x)))
#define asinh(x)  asinh(VEIL("x", (double)(x)))
#define asinhf(x) asinhf(VEIL("x", (float)(x)))

TEST(asinh, test) {
  EXPECT_STREQ(".481211825059603", gc(xdtoa(asinh(+.5))));
  EXPECT_STREQ("-.481211825059603", gc(xdtoa(asinh(-.5))));
  EXPECT_STREQ("0", gc(xdtoa(asinh(0))));
  EXPECT_STREQ("NAN", gc(xdtoa(asinh(NAN))));
  EXPECT_STREQ("INFINITY", gc(xdtoa(asinh(INFINITY))));
}

TEST(asinhf, test) {
  EXPECT_STREQ(".481212", gc(xdtoaf(asinhf(+.5))));
  EXPECT_STREQ("-.481212", gc(xdtoaf(asinhf(-.5))));
  EXPECT_STREQ("0", gc(xdtoaf(asinhf(0))));
  EXPECT_STREQ("NAN", gc(xdtoaf(asinhf(NAN))));
  EXPECT_STREQ("INFINITY", gc(xdtoaf(asinhf(INFINITY))));
}

TEST(asinhl, test) {
  EXPECT_STREQ(".4812118250596034", gc(xdtoal(asinhl(+.5))));
  EXPECT_STREQ("-.4812118250596034", gc(xdtoal(asinhl(-.5))));
  EXPECT_STREQ("0", gc(xdtoal(asinhl(0))));
  EXPECT_STREQ("NAN", gc(xdtoal(asinhl(NAN))));
  EXPECT_STREQ("INFINITY", gc(xdtoal(asinhl(INFINITY))));
}
