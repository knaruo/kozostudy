#include <defines.h>
#include <serial.h>
#include <lib.h>
#include <xmodem.h>
#include <elf.h>
#include <interrupt.h>


/* Function prototypes */
static int init(void);
static int dump(char *buf, long size);
static void wait(void);


/* ---
 * Functions
 * --- */

/* system initialization */
static int init(void)
{
    /* symbols from linker script */
    extern int erodata, data_start, edata, bss_start, ebss;

    // Initialize data RAM section
    memcpy(&data_start, &erodata, (long)&edata - (long)&data_start);
    // Initialize BSS section
    memset(&bss_start, 0, (long)&ebss - (long)&bss_start);

    // Initialize the software interrupt
    softvec_init();

    // Initialize the serial device
    serial_init(SERIAL_DEFAULT_DEVICE);

    return 0;

}


/* Data dump service for debugging */
static int dump(char *buf, long size)
{
    long i;

    if (size<0) {
        puts("no data.\n");
        return -1;
    }

    for (i = 0; i<size; i++) {
        putxval(buf[i], 2);
        // new line needed?
        if ((i &0xf)==15) {
            puts("\n");
        }
        else {
            // extra space neede?
            if ((i &0xf)==7) puts(" ");
            puts(" ");
        }
    }
    puts("\n");
    return 0;
}


/* just wait and do nothing */
static void wait(void)
{
    volatile long i;
    for (i = 0; i < 300000; i++);   // just wait
}


/* Main routine */
int main(void)
{
    static char buf[16];
    static long size = -1;
    static unsigned char *loadbuf = NULL;
    extern int buffer_start;    // defined in the linker script
    char *entry_point;
    void (*f)(void);

    INTR_DISABLE;

    // Initialization
    init();

    puts("kzload (kozos boot loader) started.\n");

    while (1) {
        puts("kzload> ");
        gets(buf);

        /* Available commands:
            load
            dump
        */
        if (!strcmp(buf, "load")) {
            loadbuf = (char *)(&buffer_start);
            size = xmodem_recv(loadbuf);
            wait();
            if (size < 0) {
                puts("\nXMODEM receive error!\n");
            }
            else {
                puts("\nXMODEM receive succeeded!\n");
            }
        }
        else if (!strcmp(buf, "dump")) {
            puts("size: ");
            putxval(size, 0);
            puts("\n");
            dump(loadbuf, size);
        }
        else if (!strcmp(buf, "run")) {
            entry_point = elf_load(loadbuf);
            if (!entry_point) {
                puts("run error!");
            }
            else {
                puts("starting from entry point: ");
                putxval((unsigned long)entry_point, 0);
                puts("\n");
                f = (void (*)(void))entry_point;
                f();
            }
        }
        else {
            puts("unknown.\n");
        }
    }

    return 0;
}
