/* ------
 * Includes
 * ------ */
#include "defines.h"
#include "kozos.h"
#include "intr.h"
#include "interrupt.h"
#include "syscall.h"
#include "memory.h"
#include "lib.h"

/* ------
 * Constants
 * ------ */
#define THREAD_NUM  6
#define THREAD_NAME_SIZE    15
#define PRIORITY_NUM    16
#define KZ_THREAD_FLAG_READY    (1 << 0)

/* ------
 * Data types
 * ------ */
typedef struct _kz_context {
    uint32 sp;
} kz_context;

typedef struct _kz_thread {
    struct _kz_thread  *next;
    char    name[THREAD_NAME_SIZE + 1];
    int     priority;
    char    *stack;
    uint32  flags;

    struct {
        kz_func_t   func;
        int argc;
        char **argv;
    } init;

    struct {
        kz_syscall_type_t   type;
        kz_syscall_param_t  *param;
    } syscall;

    kz_context  context;

} kz_thread;

static struct {
    kz_thread   *head;
    kz_thread   *tail;
} readyque[PRIORITY_NUM];

static kz_thread    *current;
static kz_thread    threads[THREAD_NUM];
static kz_handler_t handlers[SOFTVEC_TYPE_NUM];


/* ------
 * Function prototypes
 * ------ */
void dispatch(kz_context *context); // defined in startup.s
static int pop_current_from_readyque(void);
static int push_current_to_readyque(void);
static void thread_end(void);
static void thread_init(kz_thread *thp);
static kz_thread_id_t thread_run(kz_func_t func, char *name, int priority, int stacksize, int argc, char *argv[]);
static int thread_exit(void);
static int thread_wait(void);
static int thread_sleep(void);
static int thread_wakeup(kz_thread_id_t id);
static kz_thread_id_t thread_getid(void);
static int thread_chpri(int priority);
static int setintr(softvec_type_t type, kz_handler_t handler);
static void call_functions(kz_syscall_type_t type, kz_syscall_param_t *p);
static void syscall_proc(kz_syscall_type_t type, kz_syscall_param_t *p);
static void schedule(void);
static void syscall_intr(void);
static void softerr_intr(void);
static void thread_intr(softvec_type_t type, unsigned long sp);
static void *thread_kmalloc(int size);
static int thread_kmfree(char *p);


/* ------
 * Functions
 * ------ */
/* ----------------------------------------------------------
 * Description:
 *  Get (pop) the current thread from the ready queue
 * ---------------------------------------------------------- */
static int pop_current_from_readyque(void)
{
    if (current == NULL) return -1;

    if (!(current->flags & KZ_THREAD_FLAG_READY)) {
        /* if already gone, then just ignore */
        return 1;
    }

    /* Connect the next node to the head of the ready queue */
    readyque[current->priority].head = current->next;

    if (readyque[current->priority].head == NULL) {
        readyque[current->priority].tail = NULL;
    }
    /* Clear READY */
    current->flags &= ~KZ_THREAD_FLAG_READY;

    current->next = NULL;

    return 0;
}


/* ----------------------------------------------------------
 * Description:
 *  Put (push) the current thread into the ready queue
 * ---------------------------------------------------------- */
static int push_current_to_readyque(void)
{
    if (current==NULL) return -1;

    if (current->flags & KZ_THREAD_FLAG_READY) {
        /* if exists, then ignore */
        return 1;
    }

    /* Append the current node to the end of the queue */
    if (readyque[current->priority].tail) {
        readyque[current->priority].tail->next = current;
    }
    else {
        /* in case there is no node in this priority */
        readyque[current->priority].head = current;
    }
    readyque[current->priority].tail = current;
    /* Set READY */
    current->flags |= KZ_THREAD_FLAG_READY;

    return 0;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void thread_end(void)
{
    kz_exit();
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void thread_init(kz_thread *thp)
{
    thp->init.func(thp->init.argc, thp->init.argv);
    thread_end();
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static kz_thread_id_t thread_run(kz_func_t func, char *name, int priority, int stacksize, int argc, char *argv[])
{
    int i;
    kz_thread *thp;
    uint32 *sp;
    extern char userstack;
    static char *thread_stack = &userstack;

    /* Find an available thread */
    for (i = 0; i < THREAD_NUM; i++) {
        thp = &threads[i];
        if (!thp->init.func)
            break;
    }
    /* if not found, then return invalid thread ID */
    if (i==THREAD_NUM)
        return -1;

    memset(thp, 0, sizeof(*thp));

    strcpy(thp->name, name);
    thp->next = NULL;
    thp->priority = priority;
    thp->flags = 0;
    thp->init.func = func;
    thp->init.argc = argc;
    thp->init.argv = argv;

    memset(thread_stack, 0, stacksize);
    thread_stack += stacksize;

    thp->stack = thread_stack;

    sp = (uint32 *)thp->stack;
    *(--sp) = (uint32)thread_end;

    /* If the priority is zero, the thread is interrupt-disabled */
    *(--sp) = (uint32)thread_init | ((uint32)(priority ? 0 : 0xc0) << 24);

    *(--sp) = 0; /* ER6 */
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0; /* ER1 */

    *(--sp) = (uint32)thp; /* ER0 */

    thp->context.sp = (uint32)sp;

    push_current_to_readyque();

    current = thp;

    push_current_to_readyque();

    return (kz_thread_id_t)current;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static int thread_exit(void)
{
    puts(current->name);
    puts(" EXIT.\n");
    memset(current, 0, sizeof(*current));
    return 0;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static int thread_wait(void)
{
    push_current_to_readyque();
    return 0;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static int thread_sleep(void)
{
    return 0;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static int thread_wakeup(kz_thread_id_t id)
{
    /* caller thread goes to ready que */
    push_current_to_readyque();

    /* connect the specified thread to ready que,
       then wake it up */
    current = (kz_thread *)id;
    push_current_to_readyque();

    return 0;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static kz_thread_id_t thread_getid(void)
{
    push_current_to_readyque();
    return (kz_thread_id_t)current;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static int thread_chpri(int priority)
{
    int old = current->priority;
    if (priority >= 0) {
        current->priority = priority;
    }
    push_current_to_readyque();
    return old;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void *thread_kmalloc(int size)
{
    push_current_to_readyque();
    return kzmem_alloc(size);
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static int thread_kmfree(char *p)
{
    kzmem_free(p);
    push_current_to_readyque();
    return 0;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static int setintr(softvec_type_t type, kz_handler_t handler)
{

    softvec_setintr(type, thread_intr);

    handlers[type] = handler;

    return 0;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void call_functions(kz_syscall_type_t type, kz_syscall_param_t *p)
{
    switch (type) {
        case KZ_SYSCALL_TYPE_RUN:
            p->un.run.ret = thread_run(p->un.run.func,
                                       p->un.run.name,
                                       p->un.run.priority,
                                       p->un.run.stacksize,
                                       p->un.run.argc,
                                       p->un.run.argv);
            break;
        case KZ_SYSCALL_TYPE_EXIT:
            thread_exit();
            break;
        case KZ_SYSCALL_TYPE_WAIT:
            p->un.wait.ret = thread_wait();
            break;
        case KZ_SYSCALL_TYPE_SLEEP:
            p->un.sleep.ret = thread_sleep();
            break;
        case KZ_SYSCALL_TYPE_WAKEUP:
            p->un.wakeup.ret = thread_wakeup(p->un.wakeup.id);
            break;
        case KZ_SYSCALL_TYPE_GETID:
            p->un.getid.ret = thread_getid();
            break;
        case KZ_SYSCALL_TYPE_CHPRI:
            p->un.chpri.ret = thread_chpri(p->un.chpri.priority);
            break;
        case KZ_SYSCALL_TYPE_KMALLOC:
            p->un.kmalloc.ret = thread_kmalloc(p->un.kmalloc.size);
            break;
        case KZ_SYSCALL_TYPE_KMFREE:
            p->un.kmfree.ret = thread_kmalloc(p->un.kmfree.p);
            break;
        default:
            break;
    }
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void syscall_proc(kz_syscall_type_t type, kz_syscall_param_t *p)
{
    pop_current_from_readyque();
    call_functions(type, p);
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void schedule(void)
{
    int i;

    /* Search through the ready que from the highest priority (lowest value in priority field) */
    for (i = 0; i < PRIORITY_NUM; i++) {
        if (readyque[i].head) /* found */
            break;
    }
    if (i == PRIORITY_NUM) /* not found */
        kz_sysdown();

    current = readyque[i].head;
}


/* ----------------------------------------------------------
 * Description:
 *  Run currently active system call function
 * ---------------------------------------------------------- */
static void syscall_intr(void)
{
    syscall_proc(current->syscall.type, current->syscall.param);
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void softerr_intr(void)
{
    puts(current->name);
    puts(" DOWN.\n");
    pop_current_from_readyque();
    thread_exit();
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void thread_intr(softvec_type_t type, unsigned long sp)
{
    current->context.sp = sp;

    if (handlers[type])
        handlers[type]();
    
    schedule();

    dispatch(&current->context);
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
void kz_start(kz_func_t func, char *name, int priority, int stacksize,
              int argc, char *argv[])
{
    /* Initialize the dynamic memory area */
    kzmem_init();

    current = NULL;

    memset(readyque, 0, sizeof(readyque));
    memset(threads, 0, sizeof(threads));
    memset(handlers, 0, sizeof(handlers));

    setintr(SOFTVEC_TYPE_SYSCALL, syscall_intr);
    setintr(SOFTVEC_TYPE_SOFTERR, softerr_intr);

    current = (kz_thread *)thread_run(func, name, priority, stacksize, argc, argv);

    dispatch(&current->context);

}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
void kz_sysdown(void)
{
    puts("system error!\n");
    while (1);
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param)
{
    current->syscall.type = type;
    current->syscall.param = param;
    asm volatile ("trapa #0");

}


/* End of the file */
