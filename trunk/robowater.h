#ifndef __ROBOWATER__
#define __ROBOWATER__
#define F_CPU 3686400UL
#define BAUDRATE 115200UL
// �������� ����� �������� "�" ������� � ����������
#define RXC     7            // ���� ���������� ������ USART
#define TXC     6            // ���� ���������� �������� USART
#define UDRE    5            // ���� "������� ������ USART ����"
#define FE      4            // ���� ������ ������������
#define OVR     3            // ���� ������������
#define UPE     2            // ���� ������ �������� �������� USART
#define MPCM    0            // ����� ������������������� ������
// �������� ����� �������� "B" ������� � ����������
#define RXCIE   7            // ���������� ���������� �� ���������� ������
#define TXCIE   6            // ���������� ���������� �� ���������� ��������
#define RXEN    4            // ���������� ������
#define TXEN    3            // ���������� ��������
#define UCSZ2   2            // ��������� 9-�� ������� ������
#define RXB8    1            // ������ ����� ������ ��������� ������
#define TXB8    0            // ������ ����� ������ ������ ��������.
// �������� �������� ���������������� ������
#define FRAMING_ERROR	(1<<FE)
#define PARITY_ERROR    (1<<UPE)
#define DATA_OVERRUN    (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define TX_COMPLETE 	(1<<UDRE)
#define RX_COMPLETE 	(1<<RXC)
// maximum number of DS1820 devices connected to the 1 Wire bus
#define MAX_DS1820 2
#define FORCE_INIT 1           // �������� �������������� �������������
#define ALL_ALARMS 5           // ���������� ��������� ������ � �������, ���������� ���������������� � alrarm.h
#define TIMER_INACTIVE 60      // �������� � ���-�� ������������ ����������
#define ENTER_CANCEL_OVERFLOW 2      // �������� � ���-�� ������������ ����������
//#define TIME_START 300                // ����� ������ �������� � ���-�� ������������ ����������
#define TIME_STOP 10                 // ����� ��������� �������� � ���-�� ������������ ����������
#define TIME_COOL_STOP 20            // ����� �������� ��� ����������� ����������� ���������� � ������ ����
#define TIME_BUZ1 1                  // ����� ����� 1
#define TIME_BUZ2 3                  // ����� ����� 2
#define STRLENGTH 16                // ����� ������ ���������
#define COUNT_FAN_MAX 5             // ������������ ���������� ������� ��� ��������� �������� ����������� 
#define T_z 255                     // ����� �������� - �������� �����
#define RELE PORTD.4               // ����.��� �����������
#define POMP PORTD.5                // ����.��� ������
#define TA_IN_NOLIMIT 500L      // ��������� ����������� ����������� ������� �� ����� (500 = +5 �)
#define TAP_ANGLE_LIMIT 78.0    // ����������� �������� ������������ ����� ��� ����������� ������� TA_in_Min (��� ��� 20% = 51: ��� 30% = 78)) 
#define MAX_ACCURACY 0b11       // �������� ��� ����������� � �������� ����: 0b11 = 0.0625, 0b10 = 0.125, 0b01 = 0.25, 0b00 = 0.5 
#define MAX_OFFLINES 5          // ���������� ����� ����������� ������������ ������
#define CHECK_EVENT (event == ev_none)
#define ds1820_devices prim_par.terms
#define TAP_ANGLE prim_par.tap_angle    // ��������� ��������� ����� (PWM)
#define ADC_VAR1 prim_par.ADC1          // ������� ��������� ����� (���) 
#define TA_MAX prim_par.ta_max
#define TA_MIN prim_par.ta_min
#define TW_MAX prim_par.tw_max
#define TW_MIN prim_par.tw_min

// �������� ����� ����������
typedef unsigned char 	byte;	// byte = unsigned char
typedef unsigned int 	word;	// word = unsigned int
typedef char str_val[STRLENGTH];
// �������� ������������ �������, ����������� � �������
enum en_event {
    ev_none = 0,            // [0]  ������� �����������
    ev_left = 1,            // [1]  ������������ valcoder -
    ev_enter = 2,           // [2]  ��������� �����/����� enter
    ev_right = 3,           // [3]  ������������ valcoder +
    ev_cancel,              // [4]  ��������� �����/����� cancel
    ev_secunda,             // [5]  ������������ �������
    ev_timer,               // [6]  �������� ������
    ev_start,               // [7]  ������ ���� �� �����
    ev_stop,                // [8]  ������ ���� �� ����� 
    ev_tobegin,             // [9]  ������ ���������� �� ����� �� ����������
    ev_toend,               // [10] ��������� ���������� �� ����� �� ���������� � � ������
    ev_alarm1,              // [11]  ������� ������� ������  ������  ��������� ����� 
    ev_alarm2,              // [12] ������� ������� ������  ��������� ����� ��� ������  
    ev_term1_nf,            // [13] ��������� 1 �� ������
    ev_term2_nf,            // [14] ��������� 2 �� ������
    ev_to_nf,               // [15] ���������� �������� �� �����
    ev_term1_warning,       // [16] �������������� ��������� 1 �����
    ev_term2_warning,       // [17] �������������� ��������� 2 ������
    ev_tonormal             // [18] ������� � ���������� ����� ����� ev_alarm1 ��� ev_alarm2 
};
// �������� ������� ������ �������
enum en_mode {
    mo_reg = 0,             //   0 - �������������, 
    mo_to = 1,              //   1 - ��, 
    mo_setup_input = 2,     //   2 - ��������� ������, 
    mo_setup_output = 3,    //   3 - ��������� �����, 
    mo_control_line1,       //   4 - �������� �����1
    mo_control_line2        //   5 - �������� �����2
};
// �������� c�������
struct st_datetime {
    byte cHH, cMM, cSS;    // ������� �����
    byte cyy, cmo, cdd;    // ������� ����
    byte dayofweek;        // ������� ���� ������ (0..6), 0 - �����������, 1 - ����������� ...
};
// ��������� ���������� ��
struct st_TO {
    byte weekday, hour, minute;
    byte status;                // ���� ��������� ��������, �� �� ������ ����������
};
// ��������� ������������ ��� ��������� ����������� t = �(��������) * 100
struct st_eliminate {
    signed int shift;       // �������� ������� ����� (0 - ��� ���������) [2]
    signed char scale;      // ��������� (-127..0..126), ������� ����� ����������� �� ������� 1+(scale/127) [1]
};
// typedef st_eliminates t_eliminates[MAX_DS1820];
// ��������� �������� ���������� � �������
extern struct st_eeprom_par {
    struct st_eliminate elims[MAX_DS1820]; // [3] * 4 = [12]
    byte tap_angle, ADC1;                   // 0x7F, 205,  [2]
    int Ku, Ki, Kd;                         // 10, 0, 0 [6]
    int set_delta, T_int;                   // 100 set_delta-���� �����������������=300 (3 �������), T_int- ����� ��������������=100 ������
    int tw_min, tw_max;                     // 6000, 9500, 
    int ta_min, ta_max;                     //-1500, 1500,
    byte alarm_status[ALL_ALARMS];          // 0) ������� (0 - ��� �������, > 0 - ���������� ������������ ������) [12]
    byte function_keys1, function_keys2;    // ������� ������ KEY_ALARM1 � KEY_ALARM2 ����(1) ����(0)      [1]
    byte PWM_lo, PWM_hi, ADC_lo, ADC_hi;    // ��������� ������ �������� ����� � ������
    byte alert;           // ������� �������� alert � EEPROM [1]
    byte c_alerts;         // ������� ����� ���������������� alerts � EEPROM [1]
    byte terms;           // ������� ������ ���� ����������� � �������
    byte addr[MAX_DS1820][9];  // [36] ��������� ����. (� ������ - 9 ����). ���������� ����� 0 - ���������, 1 - �����, 2 - ���� ��., 3 - ���� �����
    struct st_TO TO; 
} prim_par;

// ��������� �������� �������������� � �������
extern struct st_mode {
    byte menu;           // 0) ����� ���� (00 - ������� �����, 01 - ����, 10 - ��������������, 11 - �����������/�������������� ���������
    enum en_mode run;    // 1) ������� ����� ������ 
    byte stop_sync_dt;    // 2) ���� ������� ������������� RTC � ���������� ���������. ���� ���������� - �� ��������������
    byte sound;           // 3) ���� ������� ����� (0 = ���� ��������, 1 = ���� �������
    byte ufo[9];          // 4) ������������ ��������� (-�)
    byte print;           // 5) ������ ��������� ������� �������������
} mode ;
extern enum en_event event;        // ������� ������� � �������
// �������� �������
extern void init(void);
extern int read_term(byte);
extern void set_cur_dt (void);
extern void get_cur_dt (unsigned char);
// �������� ���������� ����������
extern struct st_datetime s_dt;
// extern struct st_menupos menupos;
extern struct st_mode mode;
extern unsigned int time_integration;
extern byte timer1_valcoder;
extern unsigned int timer_start;
extern byte timer_stop;
#endif
