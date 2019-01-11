#ifndef DEFINES_H_
#define DEFINES_H_

#define NULL ((void *)0)
#define SERIAL_DEFAULT_DEVICE 1


typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

/* for OS */
typedef uint32 kz_thread_id_t;
typedef int (*kz_func_t)(int argc, char *argv[]);
typedef void (*kz_handler_t)(void);

/* for inter thread messages */
typedef enum {
    MSGBOX_ID_MSGBOX1 = 0,
    MSGBOX_ID_MSGBOX2,
    MSGBOX_ID_NUM
} kz_msgbox_id_t;

#endif
