#ifndef CONSDRV_H_
#define CONSDRV_H_

#define CONSDRV_DEVICE_NUM  1
#define CONSDRV_CMD_USE     'u' /* start using console driver */
#define CONSDRV_CMD_WRITE   'w' /* output to console */

int consdrv_main(int argc, char *argv[]);

#endif
