/*******************************************************************************
Copyright 2021 Geeky Fab. (geekyfab.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#ifndef MY_APP_DEVICE_GAMEPAD_C
#define	MY_APP_DEVICE_GAMEPAD_C

#include "app_device_joystick.h"
#include "system.h"
#include "buttons.h"
#include "usb.h"
#include "usb_device_hid.h"

typedef struct _Flags{
    uint8_t crosskey_flag :2 ;
    uint8_t sw_flag :1 ;
    uint8_t :5 ;
} Flags;


Flags flags;

void App_DeviceGamepadInit(void){
    flags.crosskey_flag = 0;
    flags.sw_flag = false;    
}

void App_DeviceGamepadAct(INPUT_CONTROLS* gamepad_input){

    gamepad_input->members.buttons.select = BUTTON_IsPressed(BUTTON_SELECT);
    gamepad_input->members.buttons.start = BUTTON_IsPressed(BUTTON_START);        

    if(flags.sw_flag == false){
        gamepad_input->members.buttons.b = BUTTON_IsPressed(BUTTON_B);
        gamepad_input->members.buttons.y = BUTTON_IsPressed(BUTTON_Y);
        gamepad_input->members.buttons.a = BUTTON_IsPressed(BUTTON_A);
        gamepad_input->members.buttons.x = BUTTON_IsPressed(BUTTON_X);
        gamepad_input->members.buttons.L1 = BUTTON_IsPressed(BUTTON_TL);
        gamepad_input->members.buttons.R1 = BUTTON_IsPressed(BUTTON_TR);


        gamepad_input->members.buttons.L2 = 0;
        gamepad_input->members.buttons.R2 = 0;
        gamepad_input->members.buttons.left_stick = 0;
        gamepad_input->members.buttons.right_stick = 0;
        gamepad_input->members.buttons.home = 0;

    } else if(flags.sw_flag == true){
        gamepad_input->members.buttons.b = 0;
        gamepad_input->members.buttons.y = 0;
        gamepad_input->members.buttons.a = BUTTON_IsPressed(BUTTON_A);
        gamepad_input->members.buttons.x = 0;
        gamepad_input->members.buttons.L1 = 0;
        gamepad_input->members.buttons.R1 = 0;

        gamepad_input->members.buttons.L2 = BUTTON_IsPressed(BUTTON_TL);
        gamepad_input->members.buttons.R2 = BUTTON_IsPressed(BUTTON_TR);
        gamepad_input->members.buttons.left_stick = BUTTON_IsPressed(BUTTON_Y);
        gamepad_input->members.buttons.right_stick = BUTTON_IsPressed(BUTTON_X);
        gamepad_input->members.buttons.home = BUTTON_IsPressed(BUTTON_B);      

    }


    //set xy axis
    if(BUTTON_IsPressed(BUTTON_LEFT)){
        gamepad_input->val[3] = 0x00;
    } else if (BUTTON_IsPressed(BUTTON_RIGHT)){
        gamepad_input->val[3] = 0xFF;
    } else{
        gamepad_input->val[3] = 0x80;
    }

    if(BUTTON_IsPressed(BUTTON_UP)){
        gamepad_input->val[4] = 0x00;
    } else if (BUTTON_IsPressed(BUTTON_DOWN)){
        gamepad_input->val[4] = 0xFF;
    } else{
        gamepad_input->val[4] = 0x80;
    }    
    
    //initialize Hat switch
    gamepad_input->val[2] = 0x08;

    // initialize z axis and rz axis 
    gamepad_input->val[5] = 0x80;
    gamepad_input->val[6] = 0x80;        

    // hat switch mode
    switch(flags.crosskey_flag){
        case 0:     //xy mode
            /*DO NOTHING*/            
            break;
        
        case 1:     //hat sw mode
            if((gamepad_input->val[3] == 0x00) && (gamepad_input->val[4] == 0x80)){
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_WEST;
            } else if((gamepad_input->val[3] == 0x00) && (gamepad_input->val[4] == 0x00)){
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_NORTH_WEST;
            } else if ((gamepad_input->val[3] == 0x80) && (gamepad_input->val[4] == 0x00)){
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_NORTH;
            } else if ((gamepad_input->val[3] == 0xFF) && (gamepad_input->val[4] == 0x00)){
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_NORTH_EAST;
            } else if ((gamepad_input->val[3] == 0xFF) && (gamepad_input->val[4] == 0x80)){
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_EAST;
            } else if ((gamepad_input->val[3] == 0xFF) && (gamepad_input->val[4] == 0xFF)){
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_SOUTH_EAST;
            } else if ((gamepad_input->val[3] == 0x80) && (gamepad_input->val[4] == 0xFF)){
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_SOUTH;
            } else if ((gamepad_input->val[3] == 0x00) && (gamepad_input->val[4] == 0xFF)){
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_SOUTH_WEST;
            } 

            gamepad_input->val[3] = 0x80;
            gamepad_input->val[4] = 0x80;
            
            break;
    
        case 2:     // z rz mode
            gamepad_input->val[5] = gamepad_input->val[3];
            gamepad_input->val[6] = gamepad_input->val[4];

            gamepad_input->val[3] = 0x80;
            gamepad_input->val[4] = 0x80;            
            
            break;
            
        default:
            /*DO NOTHING*/
            break;
    }
    
    return;

}

/*cannot take address of bit-field.*/
//void ChangeSWMode(bool* flag, BUTTON* button){
//    uint16_t cnt_timer =0;
//    
//    if (BUTTON_IsPressed(button)){
//        INTCONbits.TMR0IF = 0;          // reset timer0 interrupt flag
//        TMR0bits.TMR0 = (uint8_t)5;
//
//        while(BUTTON_IsPressed(button)){
//            if(INTCONbits.TMR0IF){          // INTCONbits.TMR0IF happens every 4ms
//                cnt_timer++;
//                if(cnt_timer >=500){        // 2s
//                    *flag = ~(*flag);
//                    cnt_timer =0;
//                    while(BUTTON_IsPressed(button));
//                }
//                INTCONbits.TMR0IF = 0;
//                TMR0bits.TMR0 = (uint8_t)5;
//            }
//        }
//    }
//        
//    return;
//}

void ChangeSWMode_Button_Start(void){
    uint16_t cnt_timer =0;
    
    if (BUTTON_IsPressed(BUTTON_START)){
        INTCONbits.TMR0IF = 0;          // reset timer0 interrupt flag
        TMR0bits.TMR0 = (uint8_t)5;
        cnt_timer = 0;
        while(BUTTON_IsPressed(BUTTON_START)){
            if(INTCONbits.TMR0IF){          // INTCONbits.TMR0IF happens every 4ms
                cnt_timer++;
                if(cnt_timer >=500){        // 2s
                    flags.sw_flag = ~(flags.sw_flag);
                    cnt_timer =0;
                    while(BUTTON_IsPressed(BUTTON_START));
                }
                INTCONbits.TMR0IF = 0;
                TMR0bits.TMR0 = (uint8_t)5;
            }
        }
    }
        
    return;
}

void ChangeSWMode_Button_Select(void){
    uint16_t cnt_timer =0;
    
    if (BUTTON_IsPressed(BUTTON_SELECT)){
        INTCONbits.TMR0IF = 0;          // reset timer0 interrupt flag
        TMR0bits.TMR0 = (uint8_t)5;
        cnt_timer = 0;
        while(BUTTON_IsPressed(BUTTON_SELECT)){
            if(INTCONbits.TMR0IF){          // INTCONbits.TMR0IF happens every 4ms
                cnt_timer++;
                if(cnt_timer >=500){        // 2s
                    switch(flags.crosskey_flag){
                        case 0: flags.crosskey_flag =1; break;
                        case 1: flags.crosskey_flag =2; break;
                        case 2: flags.crosskey_flag =0; break;
                    }
                    cnt_timer =0;
                    while(BUTTON_IsPressed(BUTTON_SELECT));
                }
                INTCONbits.TMR0IF = 0;
                TMR0bits.TMR0 = (uint8_t)5;
            }
        }
    }
        
    return;
}

#endif	/* MY_APP_DEVICE_GAMEPAD_C */