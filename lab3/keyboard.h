#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>





int keyboard_subscribe(uint8_t *bit_no);

int keyboard_unsubscribe();
int cnt_sys_inb(uint8_t adress,uint8_t *st);

int keyboard_read();

void (kbc_ih)();

int keyboard_write(uint8_t addr,uint8_t cmd);

int parse_data();

