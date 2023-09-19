/*bin/echo   ' -*- mode:sh; indent-tabs-mode:nil; tab-width:8; coding:utf-8 -*-│
│vi: set net ft=sh ts=2 sts=2 sw=2 fenc=utf-8                               :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2020 Justine Alexandra Roberts Tunney                              │
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
╚────────────────────────────────────────────────────────────────'>/dev/null #*/
mkdir -p $dir
rm -f $dir/*.s $dir/*.S

IFS=","

if [ x"$(((0x8d18fc8d128e6807 & 0x7ff000) >> 12))" != x230 ]; then
  echo "error: $SHELL does not support bitwise expressions" >&2
  exit 1
fi

scall() {
  {
    name=$1
    amd=$2
    arm_linux=$(($3 + 0))
    arm_xnu=$((($amd & 0xfff000) >> 12))
    if [ $arm_xnu != 4095 ]; then
      arm_xnu=$(($arm_xnu & 0xfff))
    fi
    shift 3
    set -- "$name" "$amd" "$arm_linux" "$arm_xnu" "$*"
    echo "#include \"libc/sysv/macros.internal.h\""
    echo ".scall" "$*"
  } >"$dir/${1/$/-}.S"
}

syscon() {
  {
    echo "#include \"libc/sysv/consts/syscon.internal.h\""
    echo ".syscon" "$*"
    if [ -f "libc/sysv/dos2errno/${2/$/-}.S" ]; then
      echo "#ifdef __x86_64__"
      echo ".yoink kDos2Errno.${2/$/-}"
      echo "#endif"
    fi
  } >"$dir/${2/$/-}.S"
}

sysstr() {
  {
    echo "#include \"libc/sysv/macros.internal.h\""
    echo ".sysstr $1 \"$2\""
  } >"$dir/${1/$/-}.S"
}

errfun() {
  NAME="$1"
  ERRNO="$2"
  {
    printf '#include "libc/macros.internal.h"\n.text.unlikely\n\n'
    printf '\t.ftrace1\n'
    printf '%s:\n' "$NAME"
    printf '\t.ftrace2\n'
    printf '#ifdef __x86_64__
	.leafprologue
	mov	%s(%%rip),%%ecx
	jmp	__errfun
#elif defined(__aarch64__)
	adrp	x1,%s
	ldrsw	x0,[x1,#:lo12:%s]
	b	__errfun
#else
#error "unsupported architecture"
#endif
	.endfn	%s,globl,hidden
' "$ERRNO" "$ERRNO" "$ERRNO" "$NAME"
  } >"$dir/${1/$/-}.S"
}

dos() {
  DOS="$1"
  UNIX="$2"
  if ! [ -f "$dir/${UNIX/$/-}.S" ]; then
    cat <<EOF >"$dir/${UNIX/$/-}.S"
//	generated by libc/sysv/dos2errno.sh
#include "libc/nt/errors.h"
#ifndef __x86_64__
        .end
#endif
	.macro	.e doscode systemv
	.short	\doscode
	.long	\systemv
	.endm
	.section .sort.rodata.dos2errno.2,"a",@progbits
	.globl	kDos2Errno.${UNIX}
	.type	kDos2Errno.${UNIX},@object
kDos2Errno.${UNIX}:
EOF
  fi
  printf %s\\n "	.e	${DOS},${UNIX}" >>"$dir/${UNIX/$/-}.S"
}
