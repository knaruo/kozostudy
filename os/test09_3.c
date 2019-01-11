#include "defines.h"
#include "kozos.h"
#include "lib.h"


int test09_3_main(int argc, char *argv[])
{
    /* touch on unused arguments */
    (void)argc;
    (void)argv;

    puts("test09_3 started.\n");
    puts("test09_3 wakes up _01.\n");
    kz_wakeup(test09_1_id);
    puts("test09_3 wakes up _01 - done.\n");
    puts("test09_3 wakes up _02.\n");
    kz_wakeup(test09_2_id);
    puts("test09_3 wakes up _02 - done.\n");
    puts("test09_3 wait in.\n");
    kz_wait();
    puts("test09_3 wait out.\n");
    puts("test09_3 exit in.\n");
    kz_exit();
    puts("test09_3 exit out.\n");

    return 0;
}
