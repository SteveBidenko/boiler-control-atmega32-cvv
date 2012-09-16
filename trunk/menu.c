#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// DS1307 Real Time Clock functions
#include <ds1307.h>
#include <delay.h>

#include "robowater.h"
#include "menu.h"
#include "spd1820.h"
#include "alarm.h"
#include "at2404.h"

// ���������� ����������������
// #define MENU_PARAMETERS 12

// ��������� �������� ����������
// extern struct st_prim_par prim_par;

// signed char curr_menu_level1 = 0, next_menu_level1 = 1;  // ������� � ��������� ����� ����
struct st_array_pos curr_menu;                // ������� � ����
struct st_array_pos dt_curr_menu;             // ����� ��������� ������� ���� �� ����� ����� � ���� ��������� ���� � �������
// struct st_array_pos menu_level2;  // ������� � ���� ������� ������
//    {1, e_clatsman, "���� "},              // [9] ����� �������� (1 - ����, 0 -����)
struct st_parameter main_menu[NUM_MENU]= {   // ���� ������� ������
    {-9900, e_temperature, 0, 1},       // [0] T���������� �� ����� � 40
    {-9900, e_temperature, 0, 2},       // [1] ��������� ����������� ������ 
    {0, e_percent, 0, 3},               // [2] ��������� ���� �������� 
    {0, e_dt, 1, 4},                    // [3] ��������� ���� � �������
    {0, e_empty, 1, 5},                 // [4] ���� � ���� ��������� ����������
    {0, e_sysok, 0, 6},                 // [5] ������ ���
    {0, e_empty, 0, 7}                  // [6] ���� � ���� ������ �������� " �������"
};

struct st_parameter sdt[NUM_DT]= {   // ���� ��������� ������� � ����
    {20, e_hour, 1, 8},             // [0]
    {05, e_minute, 1, 9},           // [1]
    {01, e_day, 1, 10},             // [2]
    {06, e_month, 1, 11},           // [3]
    {12, e_year, 1, 12},            // [4]
    {0, e_password, 1, 33}          // [5]
  };
// environment
struct st_parameter parameters[NUM_PARAMETERS]= {
    {6000, e_temperature, 1, 13},   // [0] ���������� ����������� ���� �� ������ t=
    {0, e_temperature, 1, 14},   // [1] ���������� ����������� ������� �� ����� ��.� =
    {7200, e_temperature, 0, 15},   // [2] ��������� ����������� ������ ������ �=
    {0, e_percent, 0, 16},         // [3] ��������� ���� ����� � ��������� (��������� PWM)
    {0, e_percent, 0, 17}          // [4] ������� ��������� ����� (���)   
};
// naladka
struct st_parameter settings[NUM_SETTINGS]={
    {-2500, e_world, 1, 18},        // [0] ����������� ������� ��� ta_min ������ ����� �������� �������
    {10500, e_water, 1, 19},        // [1] ����������� ������ ������������� tw_max ��� ������ ����� �������� �������
    {1000, e_world, 1, 20},         // [2] ����������� ������� �� ����� ta_max ������� ����� �������� �������
    {3000, e_water, 1, 21},         // [3] ����������� ������ ������������� ta_min ��� ������� ����� �������� �������
    {30, e_room, 1, 22},            // [4] ������ ���������� ������ 1-10 �������� ������� 
    {10, e_stime, 1, 23},           // [5] ����� �������������� T_int ������ 1-2048 ��� 
    {1, e_coef, 1, 24},             // [6] ����������� �������� Ku
    {0, e_coef, 1, 25},             // [7] ����������� �������������� Ki
    {0, e_coef, 1, 26},             // [8] ����������� ����������������� Kd
    {0, e_temperature, 1, 27},      // [9] �������� TW ��������� ������
    {0, e_scale, 1, 28},            // [10] ��������� TW ��������� ������
    {0, e_temperature, 1, 29},      // [11] �������� TA ��������� �����
    {0, e_scale, 1, 30},            // [12] ��������� TA ��������� �����
    {1, e_weekday, 1, 40},          // [13] ��������� ��� ������
    {0, e_hour, 1, 31},             // [14] ��������� ���� ���������� �� �����
    {0, e_minute, 1, 32},           // [15] ��������� ������ ���������� �� �����
    {48, e_PWM, 1, 34},             // [16] ��������� ������ ������� ������
    {250, e_PWM, 1, 35},            // [17] ��������� ������� ������� ������
    {48, e_ADC, 1, 36},             // [18] ��������� ������ ������� �����
    {250, e_ADC, 1, 37},            // [19] ��������� ������� ������� �����
    {0, e_clatsman, 1, 38},         // [20] ������� ������ KEY_ALARM1
    {0, e_clatsman, 1, 39}          // [21] ������� ������ KEY_ALARM2
};                                          
#define ALARM_POS 61
struct st_parameter alarms[MAX_ALARMS] = {
    {0, e_alarm, 0, ALARM_POS},
    {0, e_alarm, 0, ALARM_POS + 1},
    {0, e_alarm, 0, ALARM_POS + 2},
    {0, e_alarm, 0, ALARM_POS + 3},
    {0, e_alarm, 0, ALARM_POS + 4}   
};
flash lcd_str all_menu_str[] = {
        "��. t= ",          // [1]  T���������� �� ����� � 
        "������ t= ",       // [2]  ��������� ����������� ������ 
        "���� ����.",       // [3]  ��������� ���� �������� � ���������
        "���. ",            // [4]  ��������� �������, ����, ������
        "��������� ",       // [5]  �������� ����������
        "������ ",          // [6]  �������� ������
        "�������...",       // [7]  ������� ���������� ����������
        "���.���. ",        // [8]
        "��� ��� ",         // [9]
        "���.����. ",       // [10]
        "��� ���. ",        // [11]
        "��� ���. ",        // [12]
        "t = ",             // [13] ���������� ����������� ���� �� ������ t=
        "��. t = ",         // [14] ���������� ����������� ������� �� ����� 
        "������ t= ",       // [15] ��������� ����������� ������ 
        "���� ����.",       // [16] ��������� ���� �������� � ���������
        "���� ���. ",       // [17] ���������� ���� �������� � ���������
        "TA Min ",          // [18] ����������� ������� �� ����� ta_min ������ ����� �������� ������� 
        "TW Max ",          // [19] ����������� ������ ������������� tw_max ��� ������ ����� �������� �������
        "TA Max ",          // [20] ����������� ������� �� ����� TA_Max ������� ����� �������� �������
        "TW Min ",          // [21] ����������� ������ ������������� TA_Min ��� ������� ����� �������� �������
        "������ t=  ",      // [22] ������ ���������� ������ 1-10 �������� �������
        "��.���. ",         // [23] ����� �������������� T_int ������ 1-2048 ��� 
        "��=",              // [34] ����������� �������� Ku
        "��=",              // [25] ����������� �������������� Ki
        "��=",              // [26] ����������� ����������������� Kd
        "��.�.=",           // [27] 0xFE
        "��.�.=",           // [28] 0xFE
        "��.��.=",          // [29] 0xFD
        "��.��.=",          // [30] 0xFD
        "��� ��= ",         // [31] ��������� ���� ���������� �� ����� 
        "����� ��= ",       // [32] ��������� ������ ���������� �� �����
        "������=",          // [33] ����������� �� ���� ������
        "���.�����= ",      // [34] ������������ ��� ���������� �����
        "���.������= ",     // [35] ������������ ��� ���������� �����
        "��.�����= ",       // [36] ������������ ��� ���������� �����
        "��.������= ",      // [37] ������������ ��� ���������� �����
        "������ 1=",        // [38] ������� ������ KEY_ALARM1
        "������ 2=",        // [39] ������� ������ KEY_ALARM2
        "���� �� ="         // [40] ���� ������ ��
    };
char linestr[20];           // ������ ��� LCD
bit need_eeprom_write;      // ����, ���� ���������� �������� � EEPROM
int menu_value;             // ������� �������� �������������� ���������
// ������� ������������� ��������� �������� ����������
void sync_set_par(byte sync) {
    register byte i;
    // ���� ������ � ����
    if (sync == SYNC_TO_MENU) {
        // main_menu[2].val_data = prim_par.season;        // ���� ����-����
        settings[0].val_data = prim_par.ta_min;     //
        settings[1].val_data = prim_par.tw_max;     //
        settings[2].val_data = prim_par.ta_max;     //
        settings[3].val_data = prim_par.tw_min;     //
        settings[4].val_data = prim_par.set_delta;  //
        settings[5].val_data = prim_par.T_int;      //
        settings[6].val_data = prim_par.Ku;         //
        settings[7].val_data = prim_par.Ki;         //
        settings[8].val_data = prim_par.Kd;         //
        settings[9].val_data = prim_par.Kd;
        settings[13].val_data = prim_par.TO.weekday;
        settings[14].val_data = prim_par.TO.hour;
        settings[15].val_data = prim_par.TO.minute;
        settings[16].val_data = prim_par.PWM_lo;
        settings[17].val_data = prim_par.PWM_hi;
        settings[18].val_data = prim_par.ADC_lo;
        settings[19].val_data = prim_par.ADC_hi;
        settings[20].val_data = prim_par.function_keys1; // ���������������� ������ KEY_ALARM1
        settings[21].val_data = prim_par.function_keys2;  // ���������������� ������ KEY_ALARM2
        main_menu[0].val_data = parameters[1].val_data; // ������������� ���� ����� 
        main_menu[1].val_data = parameters[2].val_data; // ������������� ���� �����
        //parameters[1].val_data = TW_x;
        // printf("����� ������� � ���� \n");
        sdt[0].val_data = s_dt.cHH;
        sdt[1].val_data = s_dt.cMM;
        sdt[2].val_data = s_dt.cdd;
        sdt[3].val_data = s_dt.cmo;
        sdt[4].val_data = s_dt.cyy;
    } else {
        if (mode.stop_sync_dt) {
            if ((s_dt.cHH != sdt[0].val_data) || (s_dt.cMM != sdt[1].val_data)) {
                s_dt.cHH = sdt[0].val_data;
                s_dt.cMM = sdt[1].val_data;
                s_dt.cSS = 0;
                set_cur_dt();
                printf("������ ������� %02u:%02u:00\n",sdt[0].val_data, sdt[1].val_data);
                printf("���� %02u.%02u.20%02u\n", sdt[2].val_data, sdt[3].val_data, sdt[4].val_data);
            };
            if ((s_dt.cdd != sdt[2].val_data) || (s_dt.cmo != sdt[3].val_data) || (s_dt.cyy != sdt[4].val_data)) {
                s_dt.cdd = sdt[2].val_data;
                s_dt.cmo = sdt[3].val_data;
                s_dt.cyy = sdt[4].val_data;
                set_cur_dt();
                printf("������ ���� %02u.%02u.20%02u\n", sdt[2].val_data, sdt[3].val_data, sdt[4].val_data);
                printf("����� %02u:%02u:00\n",sdt[0].val_data, sdt[1].val_data);
            };
        } else {
            // ��������� ������ �������� � ���� �� ���������� �� ����������, �� ���������� ���� ������������� ������ � EEPROM
            if (prim_par.ta_min != settings[0].val_data) {
                prim_par.ta_min = settings[0].val_data; need_eeprom_write = 1;
            }
            if (prim_par.tw_max != settings[1].val_data) {
                prim_par.tw_max = settings[1].val_data; need_eeprom_write = 1;
            }
            if (prim_par.ta_max != settings[2].val_data) {
                prim_par.ta_max = settings[2].val_data; need_eeprom_write = 1;
            }
            if (prim_par.tw_min != settings[3].val_data) {
                prim_par.tw_min = settings[3].val_data; need_eeprom_write = 1;
            }
            if (prim_par.set_delta != settings[4].val_data) {
                prim_par.set_delta = settings[4].val_data; need_eeprom_write = 1;
            }
            if (prim_par.T_int != settings[5].val_data) {
                prim_par.T_int = settings[5].val_data; need_eeprom_write = 1;
            }
            if (prim_par.Ku != settings[6].val_data) {
                prim_par.Ku = settings[6].val_data; need_eeprom_write = 1;
            }
            if (prim_par.Ki != settings[7].val_data) {
                prim_par.Ki = settings[7].val_data; need_eeprom_write = 1;
            }
            if (prim_par.Kd != settings[8].val_data) {
                prim_par.Kd = settings[8].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TO.weekday != settings[13].val_data) {
                prim_par.TO.weekday = settings[13].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TO.hour != settings[14].val_data) {
                prim_par.TO.hour = settings[14].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TO.minute != settings[15].val_data) {
                prim_par.TO.minute = settings[15].val_data; need_eeprom_write = 1;
            }
            if (prim_par.PWM_lo != settings[16].val_data) {
                prim_par.PWM_lo = settings[16].val_data; need_eeprom_write = 1;
            }
            if (prim_par.PWM_hi != settings[17].val_data) {
                prim_par.PWM_hi = settings[17].val_data; need_eeprom_write = 1;
            }
            if (prim_par.ADC_lo != settings[18].val_data) {
                prim_par.ADC_lo = settings[18].val_data; need_eeprom_write = 1;
            }
            if (prim_par.ADC_hi != settings[19].val_data) {
                prim_par.ADC_hi = settings[19].val_data; need_eeprom_write = 1;
            }
            if (prim_par.function_keys1 != settings[20].val_data) {
                prim_par.function_keys1 = settings[20].val_data; need_eeprom_write = 1;
            }
            if (prim_par.function_keys2 != settings[21].val_data) {
                prim_par.function_keys2 = settings[21].val_data; need_eeprom_write = 1;
            }
            for (i = 0; i < MAX_ALARMS; i++) {
                if (prim_par.alarm_status[i] && (alarms[i].val_data == 0)) {
                 alarm_unreg (i);
                 printf ("������� �������� �������: %s\n", get_alarm_str(i));
                 read_all_terms(INIT_MODE);
                 printf ("�������������� ��� ����������!\n");
                 }
            }
        };
     };
}
// �������, ������� ���������� ������ �� ��������� ��������� ���������, � ��� �������� ���������
// ��������� - ����� �������� ������� � ��������� �� ������
// ������� - ��������� �� ������ linestr
char *param_str(byte num_line, struct st_parameter parameter[]) {
    int pr_data;
    struct st_parameter *st_pointer;
    st_pointer = &parameter[num_line];          // �������� ���������� ��������� �� ��������� �������� �������� �������
    pr_data = st_pointer->val_data;
    /* struct st_parameter *pnt; pnt = parameter + num_line; */
    return (par_str(st_pointer, 0, pr_data));
}
// ������������� ������� ������������ ������ � ��������������� ������� ���������.
// 1) �������� - ��������� �� ��������� ���������
// 2) ���� - ������� ������ ������ ��������. (0 - �������� ��������� ��������, 1 - �� �������� ��������� ��������)
// 3) ������, ������� ����� ����������
// ������� - ��������� �� ������ linestr
char *par_str(struct st_parameter *st_pointer, unsigned char only_val, int pr_data) {
    char *pr_name;
    char prompt[] = "=> ";
    unsigned char sign = (pr_data < 0) ? '-' : '+';
    if (only_val) pr_name = prompt; else pr_name = getmenustr(st_pointer->str_num);
    switch (st_pointer->val_type) {
        case e_empty:
            sprintf(linestr, "%s", pr_name);
            break;
        case e_clatsman:
            // ���� ������ ��� �������, �� �������� �� ������ ���. ��� ����.
            if(pr_data)
                sprintf(linestr, "%s���� ", pr_name);
            else
                sprintf(linestr, "%s����", pr_name);
            break;
        case e_percent:
            // ���� ������ ��� ��������, �� �������� ��� ��������
            sprintf(linestr, "%s%u.%-01u%%", pr_name, abs(pr_data)/10, abs(pr_data%10));
            //sprintf(linestr, "%s%u%%", pr_name, pr_data);
            break;
        case e_PWM: // ���� ������ ��� e_PWM ��� e_ADC, �� �������� ��� ������ �����   
        case e_ADC:
            sprintf(linestr, "%s%u",pr_name, pr_data);
            break;
        case e_coef: // ����������� ��������� Kp,Ki,Kd
            sprintf(linestr, "%s%u.%-01u%",pr_name, abs(pr_data)/10, abs(pr_data%10));
            break;
            // ���� ������ ��� �����, �� �������� ��� �������� ���� (-128..127)
            sprintf(linestr, "%s%d", pr_name, pr_data);
            break;    
        case e_scale:
            // ���� ������ ��� �����, �� �������� ��� �������� ���� (-128..127)
            sprintf(linestr, "%s%d", pr_name, pr_data);
            break;
        case e_password:
            // ���� ������ ������� ����� ������, �� ������ ��������� ��������� �������� '**'
            if (only_val) sprintf(linestr, "%s%u", pr_name, pr_data);
            else sprintf(linestr, "%s **", pr_name, pr_data);
            break;
        case e_sysok:
            // ���� ������ ������� ������, �� ������� ���� ����� ���, ���� ���������� ������ � '!'
            if (pr_data) sprintf(linestr, "%s%u!", pr_name, pr_data);
            else sprintf(linestr, "%s���", pr_name);
            break;
        case e_alarm:
            if (pr_data) sprintf(linestr, "%s%u!", pr_name, pr_data);
            else sprintf(linestr, "%s", pr_name);
            break;
        case e_time:
            // ���� ������ ��� �����, �� �������� �� ��������� ������� � �������� ��� ��.��.��
            sprintf(linestr, "%s%02u:%02u", pr_name, s_dt.cHH, s_dt.cMM);
            break;
        case e_date:
            // ���� ������ ��� ����, �� �������� �� ��������� ���� � �������� ��� ��.��.��
            sprintf(linestr, "%s%02u.%02u.%02u", pr_name, s_dt.cdd, s_dt.cmo, s_dt.cyy);
            break;
        case e_dt:
            sprintf(linestr, "%s ��./���.", pr_name);
            break;
        case e_stime:
            // ���� ������ ��� ����, �� �������� �� ��������� ���� � �������� ��� ��.��.��
            sprintf(linestr, "%s%03u", pr_name, pr_data);
            break;
        case e_temperature:
        case e_room:
        case e_world:
        case e_water:
            // ���� ������ ��� �����������, �� �������� ��� �����������
            sprintf(linestr, "%s%c%u.%-01uC", pr_name, sign, abs(pr_data)/100, abs(pr_data%100)/10);
            break;
        case e_hour:
        case e_minute:
        case e_day:
        case e_month:
        case e_weekday:
            // ���� ������ ��� [����, ������, ����, �����], �� �������� ��� ����
            sprintf(linestr, "%s%u", pr_name, pr_data);
            break;
        case e_year:
            // ���� ������ ��� ���, �� �������� ��� ���� � ������������
            sprintf(linestr, "%s20%u", pr_name, pr_data);
            break;
        default:
            linestr[0] = 0;
    };
    return(linestr);
}
// �������, ���������������� menu_level1
void init_curr_menu(struct st_parameter *lmenu, unsigned char lmax) {
    curr_menu.line0 = 0; curr_menu.line1 = 1;

    // curr_menu.menu = &main_menu[0];         // ������� ��������� ������������� �� ������� ����
    curr_menu.menu = lmenu;         // ������� ��������� ������������� �� ������� ����
    // curr_menu.maximum = MENU_LEVEL1;
    curr_menu.maximum = lmax;
    curr_menu.lcd = 0; // curr_menu.value = lmenu->val_data;
}
// �������, �������������� ��������� ���������� ��������� �� ������� � ��������� �������. direction - (-1, 0, 1)
void calc_pos(struct st_array_pos *array_pos, byte maxlines, signed char direction) {
    // ����������� ��� ��������� ������� ������� ������� �����
    // array_pos->line0 = array_pos->line0 + direction;
    array_pos->line0 += direction;      // ������� �������� ��������� �� ������ array_pos
    // �������� �������� �� �������������� � ���������
    if (array_pos->line0 >= maxlines) array_pos->line0 = 0;
    if (array_pos->line0 < 0) { array_pos->line0 = maxlines - 1; array_pos->line1 = 0; return; }
    // ����������� ��, ��� �� ����� ����������
    if ((array_pos->line0 + 1) == maxlines) array_pos->line1 = 0; else array_pos->line1 = array_pos->line0 + 1;
}
// ������� ������ �������� � ���������� ������ ����
void lcd_menu(signed char direction) {
    // printf ("Syncing...\n");

    // ���� valcoder �������� ������ � ������ ��� �� 1 �����, �� ������ ��� ������� �������� �� 2 �����
    if ((curr_menu.lcd == 0) && (direction > 0))    // �����������, ��� ��������� ������ � ���� ������ valcoder
        curr_menu.lcd = 1;
    else
        // ���� valcoder �������� ����� � ������ ��� �� 2 �����, �� ������ ��� ������� �������� �� 1 �����
        if ((curr_menu.lcd == 1) && (direction < 0))
           curr_menu.lcd = 0;
        else
            // ���� ����������� ���������� �� ����, �� ������������ ������� ������ ����
            if (direction) calc_pos(&curr_menu, curr_menu.maximum, direction);
    // printf("LCDLine=%u, dir=%u, line0=%u, line1=%u\n", curr_menu.lcd, direction, curr_menu.line0, curr_menu.line1);
    lcd_gotoxy(0,0);        // ������������� ������ � ������� 0 ������ 0
    lcd_line_menu(param_str(curr_menu.line0, curr_menu.menu), curr_menu.menu[curr_menu.line0].can_edit);
    lcd_gotoxy(0,1);        // ������������� ������ � ������� 0 ������ 1
    lcd_line_menu(param_str(curr_menu.line1, curr_menu.menu), curr_menu.menu[curr_menu.line1].can_edit);
    // ������������� ������ � �� ������, �� ������� ��������� ����� curr_menu.lcd
    if (curr_menu.lcd) lcd_gotoxy(LCD_DISP_LENGTH-1,1); else lcd_gotoxy(LCD_DISP_LENGTH-1,0);
    // ���� ������� �� �������� ��������
    lcd_command(LCD_DISP_ON_CURSOR_BLINK);
}
// �������, ���������������� ����� ��������������
void lcd_init_edit(void) {
    struct st_parameter *lmenu;     // ��������� ����������, ������� ����� ��������� �� ��������� ������� ����
    byte num_line;                  // ��������� ����������, ���������� ������� ������� ����
    // ������������� ��������� �� ��������� � ����������� �� ������ �������
    if (curr_menu.lcd) num_line = curr_menu.line1; else num_line = curr_menu.line0;
    // ���� ������ ������� ������� ���� � ��������� ������� num_line > 4
    if ((curr_menu.level == 0) && (num_line >= 3)) {
        sync_set_par(SYNC_TO_MENU); // �������������� ���� � ����������� �����������
        switch (num_line) {
            case 3:
                    dt_curr_menu = curr_menu;
                    init_curr_menu(&sdt[0], NUM_DT);
                    curr_menu.level = 1; // printf("���� � ���� ��������� ���� � �������\n");
                    mode.stop_sync_dt = 1;
                    // sync_set_taym(SYNC_TO_MENU);
                    break;
            case 4: init_curr_menu(&parameters[0], NUM_PARAMETERS);
                    curr_menu.level = 1; // printf("���� � ���� ���������\n");
                    break;
            case 5:
                    if (IS_ALERT) {
                        init_curr_menu(&alarms[0], MAX_ALARMS);
                        curr_menu.level = 1; // printf("���� � ���� ALERTS\n");
                    }
                    break;
            case 6: 
                    if (SETTINGS_OPEN) {
                        init_curr_menu(&settings[0], NUM_SETTINGS);
                        curr_menu.level = 1 ;//printf ("���� � ���� ���������\n");
                    }
                    break;
            default: ;
        }
        mode.menu = 1;              // ����� ����� - ����
        lcd_menu(0);
        return;
    }
    lmenu = &(curr_menu.menu)[num_line];
    if (lmenu->can_edit == 0) {
        lcd_menu(0);
        return;
    } else {
        mode.menu += lmenu->can_edit; // mode.menu = mode.menu + lmenu->can_edit (�������� �������� can_edit ��������� lmenu �� ���������)
    }
    curr_menu.par = lmenu;
    curr_menu.val_data = lmenu->val_data;        // ������������� � ������� ���� �������� ��� ��������������
    lcd_edit(0);
}
// �������, �������������� ����� �� Enter
void lcd_save_edit (void) {
    struct st_parameter *lmenu;     // ��������� ����������, ������� ����� ��������� �� ��������� ������� ����
    byte num_line;                  // ��������� ����������, ���������� ������� ������� ����
    // ������������� ��������� �� ��������� � ����������� �� ������ �������
    if (curr_menu.lcd) num_line = curr_menu.line1; else num_line = curr_menu.line0;
    lmenu = &(curr_menu.menu)[num_line];
    curr_menu.par = lmenu;
    lmenu->val_data = curr_menu.val_data;
    sync_set_par(SYNC_FROM_MENU);
    if (mode.stop_sync_dt) {
        // printf ("���� � ���� main_menu \n"); // ������������� �� ������
        // init_curr_menu(&main_menu[0], NUM_MENU);
        curr_menu = dt_curr_menu;
        curr_menu.level = 0;
        mode.stop_sync_dt = 0;      // ������� ���� ������� �������������
    }
    // ����� �������� ���� �������� � EEPROM !!!!
    if (need_eeprom_write) {
        eeprom_write_struct ((char *)&prim_par, sizeof(prim_par));
        // ������� ���� ������������� ������ � EEPROM
        need_eeprom_write = 0;
    }
    lcd_menu(0);
    return;
}
// �������, ����������������/������������������ ����� �������������� (0 - �������������, 1 - ����� �� Enter, -1 - ����� �� Cancel
void lcd_esc_edit(void) {
    struct st_parameter *lmenu;     // ��������� ����������, ������� ����� ��������� �� ��������� ������� ����
    byte num_line;                  // ��������� ����������, ���������� ������� ������� ����
    // ������������� ��������� �� ��������� � ����������� �� ������ �������
    if (curr_menu.lcd) num_line = curr_menu.line1; else num_line = curr_menu.line0;
    // ��������� ���� � lcd_init_edit
    lmenu = &(curr_menu.menu)[num_line];
    // ���� �� ������� ������� � ������ ������ Cancel ��� ev_timer
    // printf("INIT EDIT(%d): %s = %i-->%i [%u]\n", initmode, lmenu->val_name, curr_menu.val_data, lmenu->val_data, curr_menu.level);
    if (curr_menu.level || mode.stop_sync_dt) {
        // printf ("���� � ���� main_menu \n"); // ������������� �� ������
        if (mode.stop_sync_dt)
            curr_menu = dt_curr_menu;
        else
            init_curr_menu(&main_menu[0], NUM_MENU);
        curr_menu.level = 0;
        mode.stop_sync_dt = 0;
        return;
    }
    // ��������� ���� � lcd_init_edit
    curr_menu.par = lmenu;
    // printf("INIT EDIT(%d): %s = %i [%i]\n", initmode, lmenu->val_name, lmenu->val_data, curr_menu.val_data);
    curr_menu.val_data = 0;
    lcd_menu(0);
    return;
}
// ������� ������������� ������� �������� ����. � �������� ��������� ���������� �������� valcoder
void lcd_edit(signed char direction) {
    struct st_parameter *lmenu;
    lmenu = curr_menu.par;
    // printf("EDIT (%d): LCDLine=%u, line0=%u, line1=%u\n", direction, curr_menu.lcd, curr_menu.line0, curr_menu.line1);
    // (��-)������������� ������ ���� � ����������� �� ���� ����������.
    switch (lmenu->val_type) {
        case e_empty:
            // curr_menu.val_data - �� ������ ���� ��������
            break;
        case e_clatsman:
            // curr_menu.val_data - ��������� �������� 0 ��� 1
            if (direction) if(curr_menu.val_data) curr_menu.val_data = 0; else curr_menu.val_data = 1;
            // curr_menu.val_data = ~curr_menu.val_data;
            break;
        case e_temperature:
            // ��������� ����������� ���� � ����� +/-0,1 �������
            curr_menu.val_data += (int)direction * 10;
            if (curr_menu.val_data > 10000) curr_menu.val_data = -3000;
            if (curr_menu.val_data < -3000) curr_menu.val_data = 10000;
            break;
        case e_room:
            // ��������� ����������� ���� � ����� +/-0,1 �������
            curr_menu.val_data += (int)direction * 10;
            if (curr_menu.val_data > 1000) curr_menu.val_data = 10;
            if (curr_menu.val_data < 10) curr_menu.val_data = 1000;
            break;
        case e_world:
            // ��������� ����������� ���� � ����� +/-1 ������
            curr_menu.val_data += (int)direction * 100;
            if (curr_menu.val_data > 3000) curr_menu.val_data = -3000;
            if (curr_menu.val_data < -3000) curr_menu.val_data = 3000;
            break;
        case e_water:
            // ��������� ����������� ���� � ����� +/-1 ������
            curr_menu.val_data += (int)direction * 100;
            if (curr_menu.val_data > 10000) curr_menu.val_data = 500;
            if (curr_menu.val_data < 500) curr_menu.val_data = 10000;
            break;
        case e_percent:
            // ��������� ��������� ���� � ����� +/-1 %
            curr_menu.val_data += direction;
            if (curr_menu.val_data > 1000) curr_menu.val_data = 0;
            if (curr_menu.val_data < 0) curr_menu.val_data = 1000;
            break;
        case e_ADC:
            mode.run = mo_setup_output;
            curr_menu.val_data = ADC_VAR1;
            break;
        case e_PWM:
            mode.run = mo_setup_input;
        case e_coef:
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0 ) curr_menu.val_data = 0xFF;
            if (curr_menu.val_data > 0xFF) curr_menu.val_data = 0;
            break;
        case e_scale:
            curr_menu.val_data += direction;
            if (curr_menu.val_data < -128 ) curr_menu.val_data = 127;
            if (curr_menu.val_data > 127) curr_menu.val_data = -128;
            break;
        case e_stime:
            if (curr_menu.val_data < 100) curr_menu.val_data += direction; else
                if (curr_menu.val_data < 1000) curr_menu.val_data += (int)direction*10; else
                    curr_menu.val_data += (int)direction*100;
            if (curr_menu.val_data < 0) curr_menu.val_data = 3600;
            if (curr_menu.val_data > 3600) curr_menu.val_data = 0;
            break;
        case e_minute:          // ����� � ������� [00..59]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0) curr_menu.val_data = 59;
            if (curr_menu.val_data > 59) curr_menu.val_data = 0;
            break;
        case e_hour:            // ����� � ����� [00..23]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0) curr_menu.val_data = 23;
            if (curr_menu.val_data > 23) curr_menu.val_data = 0;
            break;
        case e_day:              // ��� [01..31]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 1) curr_menu.val_data = 31;
            if (curr_menu.val_data > 31) curr_menu.val_data = 1;
            break;
        case e_month:            // ������ [01..12]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 1) curr_menu.val_data = 12;
            if (curr_menu.val_data > 12) curr_menu.val_data = 1;
            break;
        case e_year:             // ���� [00..99]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 11) curr_menu.val_data = 99;
            if (curr_menu.val_data > 99) curr_menu.val_data = 11;
            break;
        case e_weekday:             // ���� [00..99]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0) curr_menu.val_data = 6;
            if (curr_menu.val_data > 6) curr_menu.val_data = 0;
            break;
        case e_alarm:
            if (direction) curr_menu.val_data = 0;
            break;
        case e_password:
            curr_menu.val_data += direction;
            SETTINGS_OPEN = (curr_menu.val_data == 20); 
            if (curr_menu.val_data < 0) curr_menu.val_data = 99;
            if (curr_menu.val_data > 99) curr_menu.val_data = 0;
            break;
        default:
            curr_menu.val_data += direction;
    }
    // ��� ������� � ����� ����������� ������ ��� ���������� ��������
    menu_value = curr_menu.val_data;
    // ������ ��� ������.
    lcd_gotoxy(0,0);        // ������������� ������ � ������� 0 ������ 1
    lcd_line_menu(par_str(lmenu, 0, lmenu->val_data), 0);
    lcd_gotoxy(0,1);        // ������������� ������ � ������� 0 ������ 1
    lcd_line_menu(par_str(lmenu, 1, curr_menu.val_data), 0);
    lcd_command(LCD_DISP_ON_CURSOR_BLINK);
}
#pragma used+
// ������� ���������� ������� � ��������� ������� ���� � �������������� ����������� (-1, 0, 1)
void print_curr_menu2(signed char direction) {
    calc_pos(&curr_menu, curr_menu.maximum, direction);
    printf("����� ���� [%u, %u]", curr_menu.line0, curr_menu.line1);
    printf(" 1) ""%s"",\t2) ""%s""\n", main_menu[curr_menu.line0], main_menu[curr_menu.line1]);
}
// ������ ���� �������� ���� ���������� � ���������� ���������
void print_curr_menu(void) {
    register byte i;
    // ������� �� ���������� ������� ��� ������ �������� (!) ����
    printf ("������� ����: ");
    for (i=0; i<curr_menu.maximum; i++) printf ("%s\t", param_str(i, curr_menu.menu));
    printf ("\n");
}
#pragma used-
// �������, ������������ ��������� �� ������ ���� �� ����������� ������
char *getmenustr(unsigned char menu_num_pp) {
    static lcd_str menustr;
    if (menu_num_pp) {
        if (menu_num_pp < ALARM_POS)
            strcpyf (menustr, all_menu_str[menu_num_pp-1]);
        else
            strcpy (menustr, get_alarm_str(menu_num_pp - ALARM_POS));
        return menustr;
    } else
        return NULL;
}
// ������� �������������� ����������� �������� ������������ ������ � %  
int calc_percent(unsigned char x, unsigned char lo, unsigned char hi) {
    long result;
    if (x <= lo ) result = 0;
        else {
        if (x >= hi ) result = 1000;
            else  
            result = (long)(x - lo)* 1000 / (hi - lo);
        }
    
    return ((int) result);
}
