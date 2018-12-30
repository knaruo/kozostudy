#include <defines.h>
#include "kozos.h"
#include <lib.h>
#include <interrupt.h>


// static function prototypes
static int start_threads(int argc, char *argv[]);


/* ---
 * Functions
 * --- */
static int start_threads(int argc, char *argv[])
{
    kz_run(test08_1_main, "command", 0, 0x100, 0, NULL);
    return 0;
}


/* Main routine */
int main(void)
{
    // disable interrupt while initialization
    INTR_DISABLE;

    puts("kozos boot succeed!\n");

    kz_start(start_threads, "start", 0, 0x100, 0, NULL);

    return 0;
}
