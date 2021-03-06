#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>
#include "i8254.h"


#define KB1_IRQ 		1

//commands
#define KBC_CMD_R 		0x20		/**< @brief command to read*/
#define KBC_CMD_W 		0x60		/**< @brief command to write*/

//register read and right
#define STAT_REG 		0x64
#define IN_BUF			0x64 		/**< @brief register to write 	(input)*/
#define OUT_BUF 		0x60		/**< @brief register to read	(output)*/
#define WRITE_CMD		0x60

//bits and keycodes 
#define KBC_IBF			BIT(1)		/**< @brief --input bufffer bit-- to check if it's full*/
#define KBC_OBF			BIT(0)		/**< @brief outout buffer bit-- is full and can be accessed */
#define KBC_PAR_ERR		BIT(7)		/**< @brief parity error bit--	check if there is an error of parity*/
#define KBC_TO_ERR		BIT(6)		/**< @brief timeout error bit*/
#define KBC_MOUSE 		BIT(5)		/**< @brief mouse bit */
#define KBC_KC_ESC 		0x81
#define KBC_CMD_INT		BIT(0)

//enough?
#define TRIES	 		20
#define DELAY_US    	20000

#endif /* _LCOM_I8042_H */
