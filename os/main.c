#include <defines.h>
#include "kozos.h"
#include <lib.h>
#include <interrupt.h>


/* ---
 * Variables
 * --- */

/* ---
 * Function prototypes
 * --- */
static int idle_task(int argc, char *argv[]);
extern int consdrv_main(int argc, char *argv[]);
extern int command_main(int argc, char *argv[]);


/* ---
 * Functions
 * --- */
static int idle_task(int argc, char *argv[])
{
    /* start each threads */
    kz_run(consdrv_main, "consdrv", 1, 0x200, 0, NULL);
    kz_run(command_main, "command", 1, 0x200, 0, NULL);

    /* lower this thread's priority */
    kz_chpri(15);
    INTR_ENABLE;
    while (1) {
        asm volatile ("sleep");
    }

    return 0;
}


/* Main routine */
int main(void)
{
    // disable interrupt while initialization
    INTR_DISABLE;

    puts("kozos boot succeed!\n");

    kz_start(idle_task, "idle", 0, 0x100, 0, NULL);

    return 0;
}
