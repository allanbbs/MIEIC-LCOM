#include <lcom/lcf.h>

#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
	if(!lsb) return 1;
  	*lsb = val&0x00FF;
  	return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  if(!msb) return 1;
  *msb = (val & 0xff00)>>8;
  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {
	uint32_t* x = (uint32_t *) value;
 	return sys_inb(port,x);
}
