/* ------
 * Includes
 * ------ */
#include "defines.h"
#include "kozos.h"
#include "intr.h"
#include "interrupt.h"
#include "serial.h"
#include "lib.h"
#include "consdrv.h"


/* ------
 * Constants
 * ------ */
#define CONS_BUFFER_SIZE    24


/* ------
 * Data types
 * ------ */
/* ------
 * Variables
 * ------ */
static struct consreg {
    kz_thread_id_t id;
    int index;

    char *send_buf;
    char *recv_buf;
    int  send_len;
    int  recv_len;

    /* padding ... */
    long dummy[3];
} consreg[CONSDRV_DEVICE_NUM];


/* ------
 * Function prototypes
 * ------ */
static void send_char(struct consreg *cons);
static void send_string(struct consreg *cons, char *str, int len);
static int consdrv_intrproc(struct consreg *cons);
static void consdrv_intr(void);
static int consdrv_init(void);
static int consdrv_command(struct consreg *cons, kz_thread_id_t id, int index, int size, char *command);


/* ------
 * Functions
 * ------ */
/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void send_char(struct consreg *cons)
{
    int i;
    /* send the first byte and shorten the send buffer */
    serial_send_byte(cons->index, cons->send_buf[0]);
    cons->send_len--;
    for (i = 0; i < cons->send_len; i++) {
        cons->send_buf[i] = cons->send_buf[i + 1];
    }
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void send_string(struct consreg *cons, char *str, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        if (str[i] == '\n')
            /* \n to \r\n */
            cons->send_buf[cons->send_len++] = '\r';
        cons->send_buf[cons->send_len++] = str[i];
    }
    if (cons->send_len && !serial_intr_is_send_enable(cons->index)) {
        serial_intr_send_enable(cons->index);  /* enable send interrupt */
        send_char(cons); /* start sending */
    }
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static int consdrv_intrproc(struct consreg *cons)
{
    unsigned char c;
    char *p;

    if (serial_is_recv_enable(cons->index)) {
        c = serial_recv_byte(cons->index);
        if (c == '\r')
            /* \r to \n */
            c = '\n';
        send_string(cons, &c, 1);   /* echo back */

        if (cons->id) {
            if (c != '\n') {
                /* if not new line, store it to receive buffer */
                cons->recv_buf[cons->recv_len++] = c;
            }
            else {
                p = kx_kmalloc(CONS_BUFFER_SIZE);
                memcpy(p, cons->recv_buf, cons->recv_len);
                kx_send(MSGBOX_ID_CONSINPUT, cons->recv_len, p);
                cons->recv_len = 0;
            }
        }
    }

    if (serial_is_send_enable(cons->index)) {
        if (!cons->id || !cons->send_len) {
            /* no data to send. end of sending process */
            serial_intr_send_disable(cons->index);
        }
        else {
            /* there is data to send. keep going */
            send_char(cons);
        }
    }

    return 0;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static void consdrv_intr(void)
{
    int i;
    struct consreg *cons;

    for (i = 0; i < CONSDRV_DEVICE_NUM; i++) {
        cons = &consreg[i];
        if (cons->id) {
            if (serial_is_send_enable(cons->index) ||
                serial_is_recv_enable(cons->index))
                /* call interrupt process */
                consdrv_intrproc(cons);
        }
    }
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static int consdrv_init(void)
{
    memset(consreg, 0, sizeof(consreg));
    return 0;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
static int consdrv_command(struct consreg *cons, kz_thread_id_t id,
                           int index, int size, char *command)
{
    switch (command[0]) {
        case CONSDRV_CMD_USE:
            cons->id = id;
            cons->index = command[1] - '0';  /* text to decimal */
            cons->send_buf = kz_kmalloc(CONS_BUFFER_SIZE);
            cons->recv_buf = kz_kmalloc(CONS_BUFFER_SIZE);
            cons->send_len = 0;
            cons->recv_len = 0;
            serial_init(cons->index);
            serial_intr_recv_enable(cons->index);  /* enable rx interrupt (start of rx) */
            break;

        case CONSDRV_CMD_WRITE:
            INTR_DISABLE;
            send_string(cons, command + 1, size - 1);
            INTR_ENABLE;
            break;

        default:
            break;
    }
    return 0;
}


/* ----------------------------------------------------------
 * Description:
 * ---------------------------------------------------------- */
int consdrv_main(int argc, char *argv[])
{
    int size, index;
    kz_thread_id_t id;
    char *p;

    consdrv_init();
    kz_setintr(SOFTVEC_TYPE_SERINTR, consdrv_intr);

    while (1) {
        id = kz_recv(MSGBOX_ID_CONSOUTPUT, &size, &p);
        index = p[0] - '0';
        consdrv_command(&consreg[index], id, index, size - 1, p + 1);
        kz_kmfree(p);
    }

    return 0;
}


/* end of the file */
