#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>
//#include "utils.c"
#include "i8254.h"
int count = 0;
int hook_id = 0;
int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if(timer < 0 || timer > 2) return 1;
  uint8_t control_word;
  uint16_t x;
  uint8_t temp;
  x = TIMER_FREQ/freq;
  timer_get_conf(timer,&control_word);
  control_word &= 0x0F;
  control_word |= TIMER_LSB_MSB;
  switch(timer){
    case 0:
      control_word|=TIMER_SEL0;
      break;
    case 1:
      control_word|=TIMER_SEL1;
      break;
    case 2:
      control_word|=TIMER_SEL2;
      break;
    default:
    break;
  }
  if(sys_outb(TIMER_CTRL,control_word)) return 1;
  util_get_LSB(x,&temp);
  if(sys_outb(TIMER_0+timer,temp)) return 1;
  util_get_MSB(x,&temp);
  if(sys_outb(TIMER_0+timer,temp)) return 1;
  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
  *bit_no = BIT(hook_id);
  return sys_irqsetpolicy(TIMER0_IRQ,IRQ_REENABLE,&hook_id);
}

int (timer_unsubscribe_int)() {
  if(sys_irqdisable(&hook_id)) return 1;
  return sys_irqrmpolicy(&hook_id);
}

void (timer_int_handler)() {
  count++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  if(!st) return 1;
  uint8_t readback = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(TIMER_0+timer);
  if(sys_outb(TIMER_CTRL,readback)) return 1;
  if(util_sys_inb(TIMER_0+timer,st)) return 1;
  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
  if(timer < 0 || timer > 2) return 1;
  if(timer_get_conf(timer,&st)) return 1;
  uint8_t timer_init;
  uint8_t timer_base;
  uint8_t timer_mode;
  uint8_t timer_output;
  //uint8_t timer_nullcount;
  union timer_status_field_val fieldVal;
  uint8_t mode_mask = 0x0E;
  uint8_t init_mask = 0x30;
  timer_base = st&BIT(0);
  timer_init = (st&init_mask)>>4;
  timer_mode = (st&mode_mask)>>1;
  timer_output = (st&BIT(7))>>7;
  if(timer_mode == 7 || timer_mode == 6) timer_mode-=4;
  switch(field){
    case tsf_all:
      fieldVal.byte = st;
      timer_print_config(timer,tsf_all,fieldVal);
      break;
    case tsf_mode:
      fieldVal.count_mode = timer_mode;
      timer_print_config(timer,tsf_mode,fieldVal);
      break;
    case tsf_initial:
      fieldVal.in_mode = timer_init;
      timer_print_config(timer,tsf_initial,fieldVal);
      break;
    case tsf_base:
    fieldVal.bcd = timer_base;
    timer_print_config(timer,tsf_base,fieldVal);
    break;
    default:
      return 1;
  }

  return 0;
}
