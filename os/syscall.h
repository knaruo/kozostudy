#ifndef SYSCALL_H_
#define SYSCALL_H_

#include <defines.h>


/* ---
 * Data type definitions
 * --- */
/* System call types */
typedef enum {
    KZ_SYSCALL_TYPE_RUN = 0,
    KZ_SYSCALL_TYPE_EXIT,
    KZ_SYSCALL_TYPE_WAIT,
    KZ_SYSCALL_TYPE_SLEEP,
    KZ_SYSCALL_TYPE_WAKEUP,
    KZ_SYSCALL_TYPE_GETID,
    KZ_SYSCALL_TYPE_CHPRI,
} kz_syscall_type_t;


/* types for workspace variable for each system calls */
typedef struct {
    union {
        struct {
            kz_func_t   func;
            char        *name;
            int         priority;
            int         stacksize;
            int         argc;
            char        **argv;
            kz_thread_id_t ret;
        } run;
        struct {
            int dummy;
        } exit;
        struct {
            int ret;
        } wait;
        struct {
            int ret;
        } sleep;
        struct {
            kz_thread_id_t id;
            int ret;
        } wakeup;
        struct {
            kz_thread_id_t ret;
        } getid;
        struct {
            int priority;
            int ret;
        } chpri;
    } un;
} kz_syscall_param_t;


#endif
