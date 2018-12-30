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
    start, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    intr_syscall, intr_softerr, intr_softerr, intr_softerr,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,
};
