
#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>
#include "i8042.h"
int KBC_hook_id = 1;
uint8_t data = 0x00;
uint8_t code_bytes[2];
uint8_t size = 0;
int sys_count = 0;
bool isTwoBytes = false;
bool type;

int keyboard_subscribe(uint8_t *bit_no){
	*bit_no = BIT(KBC_hook_id);
	return sys_irqsetpolicy(KBC_IRQ,IRQ_REENABLE|IRQ_EXCLUSIVE,&KBC_hook_id);
}

int keyboard_unsubscribe(){
	if(sys_irqdisable(&KBC_hook_id)) return 1;
	return sys_irqrmpolicy(&KBC_hook_id);
}

int cnt_sys_inb(uint8_t adress,uint8_t *st){
	sys_count++;
	return util_sys_inb(adress,st);
}

int keyboard_read(){
	uint8_t stat;
	for(int i =0;i<TRIES;i++){
		if(cnt_sys_inb(STAT_REG,&stat)) continue;
		if(stat & KBC_OBF){
			if(cnt_sys_inb(OUT_BUF,&data)) return 1;
			if(!(stat & (KBC_MOUSE | KBC_PARITY | KBC_TOUT))) return 0;
			else return 1; 
		}
		tickdelay(micros_to_ticks(DELAYUS));

	}
	return 1;
}

void (kbc_ih)(){
	keyboard_read();
}

int keyboard_write(uint8_t addr,uint8_t cmd){
	uint8_t stat;
	for(int i =0;i<TRIES;i++){
		if(cnt_sys_inb(IN_BUF,&stat)) continue;
		if(!(stat & KBC_IBF)){
			if(sys_outb(addr,cmd)) continue;
			if(!(stat & (KBC_PARITY | KBC_TOUT))) return 0;
			else return 1;
		}
	}
	return 1;

}


int parse_data(){
	if(data == 0xE0){
		isTwoBytes = true;
		code_bytes[1] = 0xE0;
	}
	else if(isTwoBytes){
		code_bytes[0] = data;
		type = !(code_bytes[0] * 0x80);
		size = 2;
		isTwoBytes = false;

	}
	else{
		size = 1;
		code_bytes[1] = 0;
		code_bytes[0] = data;
		type = !(data & 0x80);
		isTwoBytes=false;
	}
	return 0;

}
