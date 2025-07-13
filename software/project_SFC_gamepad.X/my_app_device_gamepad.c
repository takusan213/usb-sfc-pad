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
#include "mapping.h"

typedef struct _Flags{
    uint8_t crosskey_flag :2 ;
    uint8_t sw_flag :1 ;
    uint8_t :5 ;
} Flags;


Flags flags;

/* ────────────────────────────────────────────────────────────────────────────
   usageByte[usage]: 
     usage (1–14) が INPUT_CONTROLS.val[] の何バイト目に対応するか
   ──────────────────────────────────────────────────────────────────────────── */
static const uint8_t usageByte[15] = {
    /*  0: 未使用 (無効) */  0,
    /*  1: A            */  0,    // val[0].bit0
    /*  2: B            */  0,    // val[0].bit1
    /*  3: X            */  0,    // val[0].bit2
    /*  4: Y            */  0,    // val[0].bit3
    /*  5: L1           */  0,    // val[0].bit4
    /*  6: R1           */  0,    // val[0].bit5
    /*  7: Select       */  0,    // val[0].bit6
    /*  8: Start        */  0,    // val[0].bit7
    /*  9: L2           */  1,    // val[1].bit0
    /* 10: R2           */  1,    // val[1].bit1
    /* 11: Home         */  1,    // val[1].bit2
    /* 12: Right Stick  */  1,    // val[1].bit3
    /* 13: Left Stick   */  1,    // val[1].bit4
    /* 14: Z (unused)   */  1     // val[1].bit5
};

/* ────────────────────────────────────────────────────────────────────────────
   usageMask[usage]:
     各 usage (1–14) がそのバイト内で何ビット目かを表すマスク
   ──────────────────────────────────────────────────────────────────────────── */
static const uint8_t usageMask[15] = {
    /*  0: 無効      */  0x00,
    /*  1: A         */  1 << 0,  // val[0] bit0 → a
    /*  2: B         */  1 << 1,  // val[0] bit1 → b
    /*  3: X         */  1 << 2,  // val[0] bit2 → x
    /*  4: Y         */  1 << 3,  // val[0] bit3 → y
    /*  5: L1        */  1 << 4,  // val[0] bit4 → L1
    /*  6: R1        */  1 << 5,  // val[0] bit5 → R1
    /*  7: Select    */  1 << 6,  // val[0] bit6 → select
    /*  8: Start     */  1 << 7,  // val[0] bit7 → start
    /*  9: L2        */  1 << 0,  // val[1] bit0 → L2
    /* 10: R2        */  1 << 1,  // val[1] bit1 → R2
    /* 11: Home      */  1 << 2,  // val[1] bit2 → home
    /* 12: RightStick*/  1 << 3,  // val[1] bit3 → right_stick
    /* 13: LeftStick */  1 << 4,  // val[1] bit4 → left_stick
    /* 14: Z (unused)*/  1 << 5   // val[1] bit5 → unused
};

/* 物理 idx をハードボタンに変換して押下を調べる関数 */
static bool isPhysPressed(uint8_t phys)
{
    switch(phys){
        case PHYS_BTN_A:      return BUTTON_IsPressed(BUTTON_A);
        case PHYS_BTN_B:      return BUTTON_IsPressed(BUTTON_B);
        case PHYS_BTN_X:      return BUTTON_IsPressed(BUTTON_X);
        case PHYS_BTN_Y:      return BUTTON_IsPressed(BUTTON_Y);
        case PHYS_BTN_L:      return BUTTON_IsPressed(BUTTON_TL);
        case PHYS_BTN_R:      return BUTTON_IsPressed(BUTTON_TR);
        case PHYS_BTN_START:  return BUTTON_IsPressed(BUTTON_START);
        case PHYS_BTN_SELECT: return BUTTON_IsPressed(BUTTON_SELECT);
        default: return false;   // no physical button for this index
    }
}

void App_DeviceGamepadInit(void){
    flags.crosskey_flag = 0;
    flags.sw_flag = false;    
}

void App_DeviceGamepadAct(INPUT_CONTROLS* gamepad_input){

    // Clear all button fields and data by zeroing all bytes
    memset(gamepad_input->val, 0, sizeof(gamepad_input->val));
    
    // D-Padの状態を取得（全ての処理で使えるように上部で定義）
    bool up = BUTTON_IsPressed(BUTTON_UP);
    bool down = BUTTON_IsPressed(BUTTON_DOWN);
    bool left = BUTTON_IsPressed(BUTTON_LEFT);
    bool right = BUTTON_IsPressed(BUTTON_RIGHT);

    // マッピングテーブル駆動でボタン処理
    for (uint8_t phys = 0; phys < NUM_BUTTONS; phys++){
        if(!isPhysPressed(phys)) continue;          // 押されていなければスキップ

        uint8_t usage = Mapping_GetUsage(phys, flags.sw_flag);  // sw_flagでモード選択
        if(!usage || usage >= 15) continue;             // 無効は無視

        uint8_t idx  = usageByte[usage];               // val のバイト番号
        uint8_t mask = usageMask[usage];               // 立てるビット
        gamepad_input->val[idx] |= mask;               // ワンショットでセット
    }


    // アナログスティック処理
    // ハットスイッチはデフォルトでNULL(8)に設定
    gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_NULL;

    // X/Y 初期化
    gamepad_input->members.analog_stick.X = 0x80;  // デフォルト中央
    gamepad_input->members.analog_stick.Y = 0x80;  // デフォルト中央

    // Z/Rz 初期化
    gamepad_input->members.analog_stick.Z = 0x80;   // 中央位置 (128)
    gamepad_input->members.analog_stick.Rz = 0x80;  // 中央位置 (128)

    // クロスキーモード処理
    switch(flags.crosskey_flag) {
        // モード0: アナログX/Y
        case 0:
            // X軸 (左右)
            if(left){
                gamepad_input->members.analog_stick.X = 0x00;   // 左
            }else if(right){
                gamepad_input->members.analog_stick.X = 0xFF; // 右
            }
            
            // Y軸 (上下)
            if(up){
                gamepad_input->members.analog_stick.Y = 0x00;   // 上
            }else if(down){
                gamepad_input->members.analog_stick.Y = 0xFF; // 下
            }
            break;
            
        // モード1: HATスイッチ
        case 1:
            // HATスイッチ設定
            if(up && left) {
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_NORTH_WEST;
            } else if(up && right) {
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_NORTH_EAST;
            } else if(down && left) {
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_SOUTH_WEST;
            } else if(down && right) {
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_SOUTH_EAST;
            } else if(up) {
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_NORTH;
            } else if(right) {
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_EAST;
            } else if(down) {
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_SOUTH;
            } else if(left) {
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_WEST;
            } else {
                gamepad_input->members.hat_switch.hat_switch = HAT_SWITCH_NULL;
            }
            break;
            
        // モード2: Z/RZ
        case 2:
            // アナログスティックは中立に設定済み
            gamepad_input->members.analog_stick.Z = left ? 0 : (right ? 255 : 128);
            gamepad_input->members.analog_stick.Rz = up ? 0 : (down ? 255 : 128);
    
            break;
            
        // 不明なモード: モード0と同じ
        default:
            // ここには何も記述しない - 全てのモード処理は上のクロスキーモード処理で完了
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
    
    if (BUTTON_IsPressed(BUTTON_START)&&BUTTON_IsPressed(BUTTON_TR)){
        INTCONbits.TMR0IF = 0;          // reset timer0 interrupt flag
        TMR0bits.TMR0 = (uint8_t)5;
        cnt_timer = 0;
        while(BUTTON_IsPressed(BUTTON_START)&&BUTTON_IsPressed(BUTTON_TR)){
            if(INTCONbits.TMR0IF){          // INTCONbits.TMR0IF happens every 4ms
                cnt_timer++;
                if(cnt_timer >=250){        // 2s
                    flags.sw_flag = ~(flags.sw_flag);
                    cnt_timer =0;
                    while(BUTTON_IsPressed(BUTTON_START)&&BUTTON_IsPressed(BUTTON_TR));
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
    
    if (BUTTON_IsPressed(BUTTON_START)&&BUTTON_IsPressed(BUTTON_TL)){
        INTCONbits.TMR0IF = 0;          // reset timer0 interrupt flag
        TMR0bits.TMR0 = (uint8_t)5;
        cnt_timer = 0;
        while(BUTTON_IsPressed(BUTTON_START)&&BUTTON_IsPressed(BUTTON_TL)){
            if(INTCONbits.TMR0IF){          // INTCONbits.TMR0IF happens every 4ms
                cnt_timer++;
                if(cnt_timer >=250){        // 2s
                    switch(flags.crosskey_flag){
                        case 0: flags.crosskey_flag =1; break;
                        case 1: flags.crosskey_flag =2; break;
                        case 2: flags.crosskey_flag =0; break;
                    }
                    cnt_timer =0;
                    while(BUTTON_IsPressed(BUTTON_START)&&BUTTON_IsPressed(BUTTON_TL));
                }
                INTCONbits.TMR0IF = 0;
                TMR0bits.TMR0 = (uint8_t)5;
            }
        }
    }
        
    return;
}

#endif	/* MY_APP_DEVICE_GAMEPAD_C */