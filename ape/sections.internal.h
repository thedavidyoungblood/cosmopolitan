#ifndef COSMOPOLITAN_APE_SECTIONS_INTERNAL_H_
#define COSMOPOLITAN_APE_SECTIONS_INTERNAL_H_
#if !(__ASSEMBLER__ + __LINKER__ + 0)
COSMOPOLITAN_C_START_

extern unsigned char __executable_start[] __attribute__((__weak__));
extern unsigned char _ehead[] __attribute__((__weak__));
extern unsigned char _etext[] __attribute__((__weak__));
extern unsigned char _edata[] __attribute__((__weak__));
extern unsigned char _ezip[] __attribute__((__weak__));
extern unsigned char _end[] __attribute__((__weak__));
extern unsigned char _ereal[] __attribute__((__weak__));
extern unsigned char _tdata_start[] __attribute__((__weak__));
extern unsigned char _tdata_end[] __attribute__((__weak__));
extern unsigned char _tbss_start[] __attribute__((__weak__));
extern unsigned char _tbss_end[] __attribute__((__weak__));
extern unsigned char _tls_align[] __attribute__((__weak__));
extern unsigned char __privileged_start[] __attribute__((__weak__));
extern unsigned char __privileged_addr[] __attribute__((__weak__));
extern unsigned char __privileged_size[] __attribute__((__weak__));
extern unsigned char __privileged_end[] __attribute__((__weak__));
extern unsigned char __test_start[] __attribute__((__weak__));
extern unsigned char __ro[] __attribute__((__weak__));
extern uint8_t __zip_start[] __attribute__((__weak__));
extern uint8_t __zip_end[] __attribute__((__weak__));
extern uint8_t __data_start[] __attribute__((__weak__));
extern uint8_t __data_end[] __attribute__((__weak__));
extern uint8_t __bss_start[] __attribute__((__weak__));
extern uint8_t __bss_end[] __attribute__((__weak__));

COSMOPOLITAN_C_END_
#endif /* !(__ASSEMBLER__ + __LINKER__ + 0) */
#endif /* COSMOPOLITAN_APE_SECTIONS_INTERNAL_H_ */
