#ifndef _ASM_INTERNALS_
#define _ASM_INTERNALS_
int atomic_inc(volatile int *m);
int xchg(int *source, int delta);
#endif
