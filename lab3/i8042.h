#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>

/** @defgroup i8254 i8254
 * @{
 *
 * Constants for programming the i8254 Timer. Needs to be completed.
 */
#define KBC_IRQ 1
#define KBC_CMD_R 0x20
#define KBC_CMD_W 0x60
#define ACK 0xFA

#define WRITE_CMD 0x60
#define OUT_BUF 0x60
#define IN_BUF 0x64
#define ESC_BR_CODE 0x81
#define STAT_REG 0x64



#define KBC_PARITY BIT(7)
#define KBC_TOUT BIT(6)
#define KBC_OBF BIT(0)
#define KBC_IBF BIT(1)
#define KBC_MOUSE BIT(5)



#define TRIES 20
#define DELAYUS 20000
#endif








