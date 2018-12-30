#include <defines.h>
#include "kozos.h"
#include <lib.h>
#include <interrupt.h>


kz_thread_id_t test09_1_id;
kz_thread_id_t test09_2_id;
kz_thread_id_t test09_3_id;


// static function prototypes
static int start_threads(int argc, char *argv[]);


/* ---
 * Functions
 * --- */
static int start_threads(int argc, char *argv[])
{
    test09_1_id = kz_run(test09_1_main, "test09_1", 1, 0x100, 0, NULL);
    test09_2_id = kz_run(test09_2_main, "test09_2", 2, 0x100, 0, NULL);
    test09_3_id = kz_run(test09_3_main, "test09_3", 3, 0x100, 0, NULL);

    kz_chpri(15);   // lower this thread's priority
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

    kz_start(start_threads, "idle", 0, 0x100, 0, NULL);

    return 0;
}
