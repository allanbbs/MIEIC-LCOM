// IMPORTANT: you must include the following line in all your C files
#include <minix/driver.h>
#include <lcom/lcf.h>
#include <lcom/lab5.h>
#include <stdint.h>
#include <sys/mman.h>
#include <stdio.h>
#include "lcom/pixmap.h"
#include "vbe.h"
#include "graphic.h"
#include "sprite.h"
#include "keyboard.h"
#include "i8042.h"
#include "projsprites.h"




// Any header files included below this line should have been created by you
int XBOUND=100000;
int YBOUND=100000;
extern uint8_t data; 
extern int contador;
int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {

  //getPermission();
  if (init_mode(mode)) return 1; 
  sleep(delay); 
  if (vg_exit()) return 1; 

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {
  

  if (vg_init(mode) == NULL) return 1; 
  vbe_mode_info_t mode_info;
  vbeModeInfo(mode,&mode_info);
  XBOUND = mode_info.XResolution;
  YBOUND = mode_info.YResolution;
  vg_draw_rectangle(x, y, width, height, color);
  
  //vg_draw_rectangle(x, y, width, height, color);                  //draw the rectangle 
  
  message msg; 
  int r; 
  int ipc_status; 
  uint8_t irq_set; 
  

  if(keyboard_subscribe(&irq_set) != 0) return 1;                 //interrupt upon keyboard use

  while( data != KBC_KC_ESC) { 
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      continue;
    }
    if (is_ipc_notify(ipc_status)) { 
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:        
          if (msg.m_notify.interrupts & irq_set) { 
            keyboard_handler();
          }
        break;
        default:
          break;  
      }
    } else{ 
      continue;
    }
  }
  
  if(keyboard_unsubscribe()) return 1;    
  if(vg_exit()) return 1; 
  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  message msg; 
  int r; 
  int ipc_status; 
  uint8_t irq_set; 
  if (vg_init(mode) == NULL) return 1;
  vbe_mode_info_t mode_info;
  vbeModeInfo(mode,&mode_info);
  
  if(drawPattern(mode,no_rectangles,first,step,&mode_info)) return 1;
   
  if(keyboard_subscribe(&irq_set) != 0) return 1;                 //interrupt upon keyboard use

  while( data != KBC_KC_ESC) { 
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      continue;
    }
    if (is_ipc_notify(ipc_status)) { 
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:        
          if (msg.m_notify.interrupts & irq_set) { 
            keyboard_handler();
          }
        break;
        default:
          break;  
      }
    } else{ 
      continue;
    }
  }
  
  if(keyboard_unsubscribe()) return 1;    
  if(vg_exit()) return 1; 
  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  uint8_t *mem_video; 
  mem_video = vg_init(INDEX_MODE);
  
  if (mem_video == NULL) return 1;  
  
  drawXpm(xpm, x, y); 
  message msg; 
  int r; 
  int ipc_status; 
  uint8_t irq_set; 
  

  if(keyboard_subscribe(&irq_set) != 0) return 1;                 //interrupt upon keyboard use

  while( data != KBC_KC_ESC) { 
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      continue;
    }
    if (is_ipc_notify(ipc_status)) { 
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:        
          if (msg.m_notify.interrupts & irq_set) { 
            keyboard_handler();
          }
        break;
        default:
          break;  
      }
    } else{ 
      continue;
    }
  }
  
  if(keyboard_unsubscribe()) return 1;    
  if(vg_exit()) return 1; 
  
  return 0; 
}
//fr_rate: number of frames per second => um interrupt de timer = um frame
//If non-negative: number of pixels between consecutive frames
//If negative: number of frames required for a 1 pixel movement
int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  
  //uint16_t currentFrame = 0;
  bool verify = false;
  uint8_t *mem_video = vg_init(INDEX_MODE);
  vbe_mode_info_t mode_info;
  vbeModeInfo(INDEX_MODE,&mode_info);
  uint32_t interrupt_freq = sys_hz()/fr_rate;
  XBOUND = mode_info.XResolution;
  YBOUND = mode_info.YResolution;
  printf("%d\n",XBOUND);
  printf("%d\n",YBOUND);
  if (mem_video == NULL) return 1;
  if( speed == 0 || fr_rate > 60) return 1;
  Sprite *sprite = create_sprite(PLAYER,xi,yi,speed,speed);
  if(!sprite) return 1;
  drawXpm(PLAYER,xi,yi);
  message msg;
  int r;
  int ipc_status;
  uint8_t irq_set;
  uint8_t irq_set_timer;

  if(timer_subscribe_int(&irq_set_timer)) return 1;
  if(keyboard_subscribe(&irq_set)) return 1;
  while( data != KBC_KC_ESC) {
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
      continue;
    }
    if (is_ipc_notify(ipc_status)) { 
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:        
          if (msg.m_notify.interrupts & irq_set) { 
            keyboard_handler();
            printf("0x\n",data);
            if(data == 17) sprite->y-=speed;verify=true;
            if(data == 30) sprite->x-=speed;verify=true;
            if(data == 31) sprite->y+=speed;verify=true;
            if(data == 32) sprite->x+=speed;verify=true;
          }
          else if (msg.m_notify.interrupts & irq_set_timer) { 
            timer_int_handler(); 
            if(contador == (int)interrupt_freq){
              //if((sprite->x + sprite->width <= mode_info.XResolution) || (sprite->y + sprite->height <= mode_info.YResolution)){
              //currentFrame++;
              //if((sprite->x != xf && sprite->y == yf) || (sprite->x == xf && sprite->y != yf)){
                //currentFrame++;
                if(speed > 0){
                  if(verify)memset(mem_video, 0, get_bytes()*get_hres()*get_vres());
                  drawXpm(PLAYER,sprite->x,sprite->y);
                  verify = false;
                }
                /*else{
                  if(currentFrame == required){
                     memset(mem_video, 0, get_bytes()*get_hres()*get_vres());
                     animate(xpm,sprite,xf,yf,required,&currentFrame,&mode_info);
                     currentFrame = 0;
                  }
                }*/
                //contador = 0;
              //}//}
              contador = 0;
            }
            
          }
        break;
        default:
          break;  
      }
    } else{ 
      continue;
    }
  }
  if(keyboard_unsubscribe()) return 1;    
  if(timer_unsubscribe_int()) return 1;
  destroy_sprite(sprite); 
  if(vg_exit()) return 1; 
  return 0; 
}

int(video_test_controller)() {
  get_controller(); 
  return 0; 
}
