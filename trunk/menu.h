#ifndef __ROBOWATERMENU__
#define __ROBOWATERMENU__
#include "lcd_4bit.h"
#include "alarm.h"
// ����������� ����������������
#define NUM_MENU        7
#define NUM_PARAMETERS  5
#define NUM_SETTINGS    22
#define NUM_DT 6

#define SYNC_TO_MENU 1
#define SYNC_FROM_MENU 0

#define IS_ALERT main_menu[5].can_edit     // ���� ���� ������
#define KOL_ALERT main_menu[5].val_data    // �� ����������
#define TW_x parameters[0].val_data         // ���������� ����������� ���� �� ������ 
#define TA_x parameters[1].val_data         // ���������� T���������� ������� �� ����� 
#define TW_CALC parameters[2].val_data      // ��������� ����������� ������ ������
#define SETTINGS_OPEN main_menu[6].can_edit
// �������� �����
typedef char lcd_str[LCD_DISP_LENGTH];        // ������ �� 16 �������� ��� ������ LCD
// �������� ������������
enum en_type {
    e_empty = 0,              // ������ ���. ������ �� ��������� ��� ������
    e_clatsman,               // ������������� (bit) ����./����.
    e_percent,                // ������� (0..1000 = 0,0%..100,0%)
    e_temperature,            // ������������� ��� ������������� �������� ����������� ���������� �� 100 (int)
    e_room,                   // ���������� + ����������� � ��������� ���������� �� 100 (int) ����� 0.1
    e_world,                  // ���������� ���. �������� ����������� �� ����� � ����� 1 ������
    e_water,                  // ����������� �������� ����������� ���� � ����� 1 ������.
    e_coef,                   // ������������� �������� �� 00.0 �� 25.5
    e_scale,                  // ��������� ����� (-126..127)
    e_stime,                  // ����� � ��������
    e_mtime,                  // ����� � ������� ��:��
    e_minute,                 // ����� � ������� [00..59]
    e_hour,                   // ����� � ����� [00..23]
    e_day,                    // ��� [01..31]
    e_month,                  // ������ [01..12]
    e_year,                   // ���� [00..99]
    e_weekday,                // ���� ������ [0..6], 0=�����������, 1=�����������, ...
    e_dt,                     // ��������� ��./���.
    e_time,                   // ����� � ������� ��:��:��
    e_date,                   // ����� � ������� dd.mm.yy
    e_sync,                   // ������� ��������� ����-������� � ���������� �����
    e_sysok,                  // ������� ���������� ������
    e_alarm,                  // ���������� �������
    e_password,               // �������� ������ -32766..32767
    e_PWM,                    // ���  ������ ������������� �������� (0..255)
    e_ADC                     // ���  ������ ������������� �������� (0..255)
     
};
// ����������� ��������
// �������� ������ ��������� �������. ����� ��� ����������� ������� ���������� �������
    // lcd_str *val_name;                   // ��������� �������� ��������� ��� LCD
struct st_parameter {
    // byte id;                         // ���������� ����� � ������� ��� ��������� ������ ����������
    signed int val_data;                // ���� �������� ��������� (-32768..32767
    enum en_type val_type;              // ��� �������� ���������
    unsigned char can_edit;             // ������� ����, ��� val_data ����� �������������
                                        // 0 - ��������� ���������/������� �����������
                                        // 1 - �������������� ���������/ ���� �������
    unsigned char str_num;              // ���������� ����� ������ ���� � ������� getmenustr
};
struct st_array_pos {
    signed char line0;          // ������� ������� � ������� (����)
    signed char line1;          // ��������� ������� � ������� (����)
    unsigned char lcd;          // ������� �������� ������ ������� (0 - 1 ������, 1 - 2 ������)
    struct st_parameter *menu;  // ��������� �� ������� ������� �������� ����
    unsigned char maximum;      // ������������ ���-�� ��������� � ����
    // ��� �������������� ���������� ���������� ������� ��� ���� ��������� ���������
    struct st_parameter *par;   // ��������� �� ������� ��������
    int val_data;               // ������������� �������� ���������
    unsigned char level;        // ������� ������� ���� (0 - ������� ����, 1 - �������������� ����)
};
// �������� ������� ����������
// extern lcd_str linestr;             // ������ ��� LCD
extern char linestr[];                // ������ ��� LCD
extern int menu_value;                // ������� �������� ����������� � ���� ���������
// extern signed char curr_menu_level1, next_menu_level1;  // ������� � ��������� ����� ����
extern struct st_parameter main_menu[NUM_MENU];
extern struct st_parameter parameters[NUM_PARAMETERS];
extern struct st_parameter alarms[MAX_ALARMS];
extern struct st_array_pos curr_menu;                // ������� � ����
// �������� ������� �������
extern void sync_set_par(unsigned char);
extern void init_curr_menu(struct st_parameter *lmenu, unsigned char lmax);
extern void lcd_init_edit(void);
extern void lcd_save_edit(void);
extern void lcd_esc_edit(void);
extern void lcd_menu(signed char);
extern void lcd_edit(signed char);
// extern void print_curr_menu2(signed char);
// extern void print_curr_menu(void);
extern char *param_str(unsigned char, struct st_parameter parameter[]);
extern char *par_str(struct st_parameter *, unsigned char, int);
extern char *getmenustr(unsigned char menu_num_pp);
extern int calc_percent(unsigned char, unsigned char, unsigned char);
// extern unsigned char next_menu(signed char *menu_level, signed char menu_choice);
#endif
