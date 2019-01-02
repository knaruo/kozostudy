#include <defines.h>

extern void start(void);    // start up function
extern void intr_softerr(void);
extern void intr_syscall(void);
extern void intr_serintr(void);


/* ---
    Configure interrupt vectors
    this is allocated to the beginning of the memory
    refer to the linker script
--- */
void (*vectors[])(void) = {
    /* 0x00: Reset vector - 0x1c */
    start, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x20 - 0x2c: TRAPA #0 - #3 */
    intr_syscall, intr_softerr, intr_softerr, intr_softerr,
    /* 0x30 - */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x50 - */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x70 - */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x90 - */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xb0 - */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0xd0 - : SCI 0 interrupt */
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,
    /* 0xe0 - : SCI 1 interrupt */
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,
    /* 0xf0 - : SCI 2 interrupt */
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,
};
