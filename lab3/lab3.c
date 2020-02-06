#include <lcom/lcf.h>
#include "i8042.h"
#include <lcom/lab3.h>
#include "keyboard.h"
#include <stdbool.h>
#include <stdint.h>
extern uint8_t data;
extern uint8_t code_bytes[2];
extern bool type;
extern bool isTwoBytes;
extern int KBC_hook_id;
extern int sys_count;
extern uint8_t size;
extern int count;
extern int hook_id;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {
  int ipc_status,r;
  message msg;
  uint8_t irq_set;
  if(keyboard_subscribe(&irq_set)) return 1;

 while( data != ESC_BR_CODE ) { /* You may want to use a different condition */
    /* Get a request message. */
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
        printf("driver_receive failed with: %d", r);
        continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
            case HARDWARE: /* hardware interrupt notification */       
                if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
                    kbc_ih();
                    parse_data();
                    if(!isTwoBytes)
                      kbd_print_scancode(type,size,code_bytes);
                }
                break;
            default:
                break; /* no other notifications expected: do nothing */ 
        }
    } else { /* received a standard message, not a notification */
        /* no standard messages expected: do nothing */
    }
  }
  kbd_print_no_sysinb(sys_count);
  return keyboard_unsubscribe();
}


int(kbd_test_poll)() {
  uint8_t command;
  while(data != ESC_BR_CODE){
    while(keyboard_read())
      ;
      parse_data();
      if(!isTwoBytes)
        kbd_print_scancode(type,size,code_bytes);
    
      
  }
  kbd_print_no_sysinb(sys_count);
  keyboard_write(IN_BUF,KBC_CMD_R);
  keyboard_read();
  command = data;
  command|=BIT(0);
  keyboard_write(IN_BUF,KBC_CMD_W);
  keyboard_write(WRITE_CMD,command);
  return 0;

}

int(kbd_test_timed_scan)(uint8_t n) {
  int ipc_status,r;
  message msg;
  uint8_t irq_set,timer_irq;
  if(keyboard_subscribe(&irq_set)) return 1;
  if(timer_subscribe_int(&timer_irq)) return 1;
  int idle = n*60;

 while( data != ESC_BR_CODE && count < idle) { /* You may want to use a different condition */
    /* Get a request message. */
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
        printf("driver_receive failed with: %d", r);
        continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
            case HARDWARE: /* hardware interrupt notification */       
                if (msg.m_notify.interrupts & timer_irq) { /* subscribed interrupt */
                    timer_int_handler();
                }
                if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
                    kbc_ih();
                    parse_data();
                    if(!isTwoBytes)
                      kbd_print_scancode(type,size,code_bytes);
                    count = 0;
                }
                break;
            default:
                break; /* no other notifications expected: do nothing */ 
        }
    } else { /* received a standard message, not a notification */
        /* no standard messages expected: do nothing */
    }
  }
  kbd_print_no_sysinb(sys_count);
  if(timer_unsubscribe_int()) return 1;
  return keyboard_unsubscribe();
}

