// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>
#include <lcom/vbe.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "menu.h"
#include "game.h"
#include "i8042.h"
#include "sprite.h"
#include "i8254.h"
#include "mouse.h"
#include "keyboard.h"
#include "vbe.h"
#include "graphic.h"
#include "proj.h"
#include "rtc.h"
#include "textInput.h"


int XBOUND=100000;
int YBOUND=100000;
uint8_t counter = 0;
uint32_t data_mouse; 
//mouse variables 
uint16_t byteArray[3];
extern struct packet pp;
//keyboard variables  
extern uint8_t code_bytes[2]; 
extern bool isTwoBytes; 
extern uint8_t data;  
//cannon shot
extern bool doing_shot; 
extern bool cannon_shot; 
//seconds
float segundos; 
extern int contador; 
extern uint8_t * buffer;
//rtc 
uint8_t hour;
uint8_t month;
uint8_t minute;
uint8_t seconds; 
uint8_t month;
bool day = true; 
//textInput
char name1[20] = ""; 
char name2[20] = ""; 
char letter = 'A';
int size1 = 0; 
int size2 = 0; 
int total1 = 0;
int total2 = 0;

bool finish =false; 

/**
 * @file proj.c
 * @brief This is file contains the implementation of the main file in the project
 */


void isDay();


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

static int print_usage() {
  printf("Usage: <mode - hex>\n");

  return 1;
}


int(proj_main_loop)(int argc, char *argv[]) {
    //initializing video 

      uint8_t *mem_video;
      mem_video = vg_init(INDEX_MODE);
      if (mem_video == NULL) return 1;
      vbe_mode_info_t mode_info;
      vbeModeInfo(INDEX_MODE,&mode_info);
      malloc_buffer();
      XBOUND = mode_info.XResolution;
      YBOUND = mode_info.YResolution;

   

    //what windows it is 
    window state = MENU; 

    //interrupts variables--------------
    int mouse_x = 0, mouse_y = 0; 
    int r;
    int irq_set; 
    uint8_t irq_set_timer0; 
    uint8_t irq_set_keyboard;
    int ipc_status;
    message msg;
    int frequency = 64; 
    
    //init sprite-------------------------
    init_menuSprites(); 
    init_gameSprite();

    //rtc--------------------------------- 

    //Setting if it's day or not 
    isDay();
    //interrupts--------------------------
    timer_set_frequency(0, frequency);
    if (issueCommand(REP_ENABLE) != 0) return 1;
    if (mouse_subscribe_int(&irq_set) != 0) return 1;
    if (timer_subscribe_int(&irq_set_timer0)!= 0) return 1; 
    if (keyboard_subscribe(&irq_set_keyboard)) return 1; 

    //interrupt loop----------------------
    while (!pp.rb)

    {
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
        {
        printf("driver_receive failed with: %d\n", r);
        continue;
        }

        if (is_ipc_notify(ipc_status))
        {
        switch (_ENDPOINT_P(msg.m_source))
        {
        case HARDWARE:
            if((msg.m_notify.interrupts & irq_set_keyboard)){
                  keyboard_handler();
                  keyboard_display_scans(data);
                  if (!isTwoBytes) handleControls(); 
                  if (KBC_KC_ESC == code_bytes[0] && state == GAME) state = PAUSE;                    //go back to the menu
                  if (state == INST && code_bytes[0] == KBC_KC_SPACE) state = MENU; //go to instructions 
                  if (code_bytes[0] == KBC_ENTER && state == NAME1) {
                    clear_buffer(); 
                    state = NAME2; 
                  }
                  else if (code_bytes[0] == KBC_ENTER && state == NAME2) state = GAME; 
                
                  if (state == NAME2 || state == NAME1) letter = get_letter(); 
                  if (state == NAME1) askname1();
                  if (state == NAME2) askname2();                   
              }
            
      
            if(msg.m_notify.interrupts & irq_set_timer0){
              if (finish && state == GAME){
                state = MENU;
                finish = false;
              }

              isDay();
              //printf("%d\n",day);
              //demarca o tempo num tiro
              if (doing_shot && state == GAME){
                timer_int_handler(); 
                segundos = (float)contador/(float)frequency; 
              }
              if (state == MENU || state == PAUSE) menuDraw(&mouse_x, &mouse_y); 
              if (state == GAME){
                gameDraw(&mouse_x, &mouse_y);
              }
            }

            if (msg.m_notify.interrupts & irq_set){ 
                mouse_ih();
                handleSync();
                if(counter == 3){
                    counter = 0; 
                    parsePacket();
                    if (state == MENU){
                      if (check_start(mouse_x, mouse_y)) {
                        state = NAME1;
                        askname1();
                      }
                    }
                  
                    if (state == PAUSE){
                      if (check_start(mouse_x, mouse_y)) {
                        state = GAME; 
                        reset_packet();
                      }
                    }
                      
                    if (state == MENU || state == PAUSE){
                    
                      if (check_exit(mouse_x, mouse_y)) return exit_(); 
                      if (check_instructions(mouse_x, mouse_y)) state = INST; 
                    }
                    else if (state == GAME){
                      if (!doing_shot) get_angle();
                      if (pp.lb && !doing_shot) cannon_shot = true;
                      
                    }
                  
                }
            }
          
            break;
        default:
            break;
        }
        }
        else
        { 
        }
    }
    exit_(); 
    return 0; 

  bool const minix3_logo = true;
  bool const grayscale = false;
  uint8_t const delay = 5;
  uint16_t mode;

  if (argc != 1)
    return print_usage();

  // parse mode info (it has some limitations for the sake of simplicity)
  if (sscanf(argv[0], "%hx", &mode) != 1) {
    printf("%s: invalid mode (%s)\n", __func__, argv[0]);

    return print_usage();
  }

  return proj_demo(mode, minix3_logo, grayscale, delay);
}

int exit_(){
  if (mouse_unsubscribe_int() != 0) return 1;
  if (timer_unsubscribe_int() != 0) return 1;
  if (keyboard_unsubscribe()) return 1;
  if (issueCommand(REP_DISABLE) != 0)  return 1;
  if(vg_exit()) return 1; 
  free_buffer();
  return 0; 
}

void isDay(){
    read_rtc(&hour,HOURS);
    read_rtc(&month,MONTH);
    read_rtc(&minute,MINUTES);
    if (month <= 4 && (hour >= 19 || hour <= 8)) day = false; 
    //Summer
    else if (month <= 7 && (hour >= 22 || hour <= 8))  day = false;
    //outono
    else if (month <= 10 && (hour >= 18 || hour <= 8))  day = false;  
    //inverno 
    else if (month <= 12 && (hour >= 17 || hour <= 8))  day = false; 
    else day = true; 
}

