#include <mega32.h>
#include <stdlib.h>  // for abs
#include <stdio.h>
#include <delay.h>
#include "lcd_4bit.h"
#include "menu.h"
#include "robowater.h"
// DS1820 ������� �������������� �������������� (�������+�������)
#include "spd1820.h"
#include "valcoder.h"
#include "at2404.h"
#include "alarm.h"
#include "signals.h"
#include "keys.h"
#include "dayofweek.h"
// ��������� ����������������
#define MAJOR_VERSION 1
#define MINOR_VERSION 4
// #define NODEBUG
// enum
// ����������� ������� ��������
// �������� ��������� ����������
// TW_x     - ���������� ����������� ����
// ��_x     - ���������� ����������� ������� �� �����
// TA_MIN   - ����������� ������� �� �����, ������ ����� �������� �������
// TW_MAX   - ����������� ������ ������������� ��� ������ ����� �������� �������
// TA_MAX   - ����������� ������� �� �����, ������� ����� �������� �������
// TW_MIN   - ����������� ������ ������������� ��� ������� ����� �������� �������
// TA_out_prs -����������� ������� �� ������ ������������� +20 �.(��������)
unsigned char key_treated[7] = {0,0,0,0,0,0,0} ; 
struct st_mode mode = {0, mo_reg, 0, 1, 0.0, 0};  // ������� ����� ������
struct st_datetime s_dt;
// ��������� ��������� ��������� �������� ����������
struct st_eeprom_par prim_par={
    {{(int)0, 0}, {(int)0, 0}}, // struct st_eliminate  ��������� ������������ ��� ��������� �����������
    0xFF, 0x7F,                  // tap_angle, ADC1, (0-255)
    (int)10, (int)0, (int)0,    // Ku, Ki, Kd;
    (int)300, (int)100,         // set_delta, T_int;
    (int)6000, (int)9500,       // TW_Min, TW_Max;
    (int)-2000, (int)1500,      // TA_Min, TA_Max, 
    {0, 0, 0, 0, 0},            //alarm_status[ALL_ALARMS] 5
    0, 0, // // ������� ������ KEY_ALARM1 � KEY_ALARM2 ����(1) ����(0)
    48, 250, 48, 250,
    0,  // ������� �������� alarm � EEPROM
    0,   // ������� ����� ���������������� � EEPROM
    MAX_DS1820, // ������� ������ ���� ����������� � �������
    // ������ �����������
    {
        //{0x28,0xb9,0xf8,0x93,0x03,0x00,0x00,0x9a,0x01},   // 28 B9 F8 93 3 0 0 9A 1;	FF	FE - RT serial number 0. �����
        //{0x28,0x36,0x29,0x94,0x03,0x00,0x00,0x41,0x01}    // 28 36 29 94 3 0 0 41 1;	FF	FD - RT serial number 0. ������
        {0x28,0x81,0x3b,0x94,0x03,0x00,0x00,0x3c,0x01},     // 28 81 3B 94 3 0 0 3C 1;  FF	FE - RT serial number 1. �����
        {0x28,0x66,0xf5,0x93,0x03,0x00,0x00,0x96,0x01}      // 28 66 F5 93 3 0 0 96 1;  FF	FD - RT serial number 1. ������
        //{0x28,0x26,0x15,0x94,0x03,0x00,0x00,0xbf,0x01},     // 28 26 15 94 3 0 0 BF 1;  FF	FE - RT serial number 2. �����
        //{0x28,0x0e,0x3a,0x94,0x03,0x00,0x00,0x3f,0x01}      // 28 0E 3A 94 3 0 0 3F 1;  FF	FD - RT serial number 2. ������
     },
     {2,9,0,0} //struct st_TO TO; weekday = 2, hour 9 , minute = 0, status = 0;
};
unsigned int time_integration=0;
enum en_event event;                          // ������� ������� � �������
// �������� �������
void printallterms(void); void lcd_primary_screen(void);
void print_prim_par(unsigned char *, unsigned char);
// ������� ��������� �����
void check_serial(void);
void check_peripheral(void);
void event_processing(void);
void mode_processing(void);
int check_range(int);
//void update_P(int);
void update_PID(int);
char high_time_TO(void);

/* ��� ������� ������ ������� ����� � HEX 
void printl_hex (long int lx) {
    union pr_long {
        long int x;
        char cx[4];
    } prlg;
    byte i;
    
    prlg.x = lx;
    for (i = 0; i < sizeof(long int); i++) {
        printf(" 0x%X", prlg.cx[i]);
    }
}
*/

int graphic(int air_x) {
    long int Wmax, Wmin, Sum, res;
    int DeltaMax, DeltaMin, Delta;

    if (air_x >= TA_MAX) 
        res = TW_MIN;
    else {
        if (air_x <= TA_MIN) 
            res = TW_MAX;
        else {
            DeltaMax = TA_MAX - air_x;
            DeltaMin = air_x - TA_MIN;
            Delta = TA_MAX - TA_MIN;
            Wmax = (long int) TW_MAX * (long int) DeltaMax;
            Wmin = (long int) TW_MIN * (long int) DeltaMin;
            // printf("Wmax ="); printl_hex(Wmax);
            // printf("; Wmin ="); printl_hex(Wmin);
            Sum = Wmax + Wmin;
            res = Sum/Delta;
            // printf("Res ="); printl_hex(res);
            // printf("\n");
        }
    }
    return ((int) res);
}

// �������� ���������
void main(void) {
    // register byte i;
    byte size_prim_par;
    
    init();                  // ������������� ���� ���������
    #asm("sei")             // Global enable interrupts
    //printf ("������������� ������...\n"); 
    init_keys();
    printf("RT serial number 1. ������ %u.%02u. ������� %u �����������. �����!\n", MAJOR_VERSION, MINOR_VERSION, ds1820_devices);
    // ��������� � EEPROM ��������� prim_par
    // ppr_par = &prim_par;
    size_prim_par = sizeof(prim_par);
    // ����� ���������� � EEPROM � ��������� ����������
    if (!KEY_CANCEL && !KEY_ENTER) {
        unsigned char term_status;
        ds1820_devices = MAX_DS1820;
        // ������������ ������� ����������� � EEPROM
        term_status = sync_ds1820_eeprom();
        eeprom_write_struct ((unsigned char *)&prim_par, size_prim_par);
        // ��������� ������ ������������
        printf(" [�����]+[�����]+[�����] = ������ � EEPROM ��������� ���������.\n ������������ ����������� = %u\n", term_status);
    } else {
        unsigned char size_addr = size_prim_par - sizeof(prim_par.addr);
        if (!KEY_CANCEL) {
            ds1820_devices = MAX_DS1820;
            printf("[�����]+[�����] = ������ � EEPROM ��������� �� ���������.\n");
            eeprom_write_struct ((unsigned char *)&prim_par, size_addr);
        }
        if (!KEY_ENTER) {
            unsigned char term_status;
            eeprom_read_struct ((char *)&prim_par, size_addr);
            ds1820_devices = MAX_DS1820;
            // ������������ ������� ����������� � EEPROM
            term_status = sync_ds1820_eeprom();
            printf("[�����]+[�����] = ������ � EEPROM %u ����� �����������.\n", term_status);
            eeprom_write_struct ((unsigned char *)&prim_par, size_prim_par);
        }
    }
    // ��������������� �� EEPROM ��������� prim_par
    eeprom_read_struct ((char *)&prim_par, size_prim_par);
    // printf("\n");
    read_all_terms(INIT_MODE);
    // print_all_menu();       // ������� �� ���������� ������� ��� ������ ����
    sync_set_par(SYNC_TO_MENU); // �������������� ���� � ����������� �����������
    update_alarm_menu();        // ��������� ���� alarms
    //printf("����� ������ �� EEPROM �������� Tw2=%u\n", prim_par.Tw2);
    // printf("���� %u, ����� ����� ������ �� EEPROM size=%u\n", sizeof(prim_par), size_prim_par);
    #ifndef NODEBUG
    signal_buz(LONG);
    // ���� ���� ����������, �� ������� �� ��������
    printallterms();
    #endif
    lcd_primary_screen();       // ������� ��������� �������� �� ��������
    if (!IS_ALERT == 0) {
        RELE = 1; signal_red(SHORT); signal_buz(MEANDR);
    }
   
    while(1) {
        // ��������! ���� �� ��������� ����� WHILE ��� �� ���������!!!
        check_serial();
        poll_keys();
        // if (event == ev_none) check_peripheral();
        /* if (0 == 1) */ check_peripheral();
        /* if (0 == 1) */ event_processing();
        /* if (0 == 1) */ mode_processing();
    }; // while (1)

} // void main(void)
// ������� ������ ��������� � ��������� ������� �� ����������� ������, � ����� �� ����������� ����������
void check_peripheral(void) {
    // � ���� ������� ��������� (!) ������������� ������
    // ������������ ������� valcoder'�
    if ((abs(valcoder)-VALCODER_SENSITY) >= 0) {        // ���� �������� valcoder
        // printf ("������������� �������� (%i)...\n", valcoder);
        if (valcoder < 0)
            event = ev_left;
        else
            event = ev_right;
    }                                 
    // ���� �������� �� ����, ���������� ������ �����
    if (CHECK_EVENT && (!KEY_ENTER)) event = ev_enter;
    if (CHECK_EVENT && (!KEY_CANCEL)) event = ev_cancel;
    // ����������� ������ ����
    //if (CHECK_EVENT && (!timer_stop) && (!KEY_STOP)) event = ev_stop;
    if (CHECK_EVENT &&  (!KEY_STOP)) event = ev_stop;
    // ����������� ������ �����, �������� ��� ���� ������� �����-���� ������
    if (CHECK_EVENT && (IS_ALERT == 0) && (!KEY_START)) event = ev_start;
    // ��� ����� "�����" ���������� �������, ���� ������� �������������
    // �������� ���������
    if (CHECK_EVENT && (!prim_par.alarm_status[0]) && (KEY_ALARM1) && (prim_par.function_keys1)) event = ev_alarm1; // ��������� �����
    if (CHECK_EVENT && (!prim_par.alarm_status[1]) && (KEY_ALARM2) && (prim_par.function_keys2)) event = ev_alarm2; // �� �����
    // �������� �����������
    if (CHECK_EVENT && (!prim_par.alarm_status[2]) && (termometers[0].err >= MAX_OFFLINES)) {
        // printf("��� ���������� �1 (�����): %d, err=%d", prim_par.alarm_status[2], termometers[0].err);
        event = ev_term1_nf;
    } 
    if (CHECK_EVENT && (!prim_par.alarm_status[3]) && (termometers[1].err >= MAX_OFFLINES)) { 
        // printf("��� ���������� �2 (������): %d, err=%d", prim_par.alarm_status[3], termometers[1].err);
        event = ev_term2_nf;
    }
    // �������� ����� ��������� �������, ���� ������� ����� mo_control_line1 ��� mo_control_line2 
    switch (mode.run) {
        case mo_control_line1:
            if (CHECK_EVENT && (!KEY_ALARM1)) event = ev_tonormal;
            break;
        case mo_control_line2:
            if (CHECK_EVENT && (!KEY_ALARM2)) event = ev_tonormal;
            break;
        default:
            break;
    }
    // ����� ������������ ��������� ��� ��������� �������
    #ifndef NODEBUG
    // if (event) printf ("������������� ����� �������: %u\n", (unsigned char)event);
    //if (event) printf ("-");
    #endif
}
// ��������� ������� � �������
void event_processing(void) {
    // ������ ����� ����� ������ ������. �� ����� ������ ���������� ���������, ������ ���������
    #ifndef NODEBUG
    // if (event != ev_none)
        // printf ("%02u:%02u ������� ������� = %u\n", s_dt.cMM, s_dt.cSS, (unsigned char)event);
    #endif
    // ����� ����� ����������� ����������������� �������� ��� ���������: ���./���� ���������, ��������� ���������, ���������� ������ ���� � �.�.
    switch (event) {
        case ev_secunda:                // ������������ ������������ �������.
            s_dt.dayofweek = dayofweek(s_dt.cdd, s_dt.cmo, s_dt.cyy);
            if (prim_par.function_keys1) {
                TA_x = read_term(0);
            } else {
               if (KEY_ALARM1) TA_x = read_term(0);
            }        
            if(prim_par.function_keys2) {
                TW_x = read_term(1);
            } else {
                if (KEY_ALARM2) TW_x = read_term(1);
            }
            switch (mode.menu) {
                case 0: lcd_primary_screen(); break;
                case 1: lcd_menu(0); break;
                default: ;
            }
            if (!mode.print && (mode.run == mo_to)) printf("-");
            TW_CALC = graphic(TA_x);
            // ������������� ���� ����� ���� "��������� ���� ����� � ���������"
            main_menu[2].val_data = parameters[3].val_data; 
            parameters[4].val_data = calc_percent(ADC_VAR1, prim_par.ADC_lo, prim_par.ADC_hi);
            // ���� ���� ��������� ��, �� ���������� �������
            if (high_time_TO()) {
                printf ("���� ��������� ��\n");
                prim_par.TO.status = 1;
                event = ev_start;
            } else {
                event = ev_none;            // ������� �������
            }
            break;
        case ev_left:                   // printf ("������������ ��������� valcoder �����\n");
        case ev_right:                  // printf ("������������ ��������� valcoder ������\n");
            // ��������� ������ ������������
            //timer1_valcoder = TIMER_INACTIVE;
            timer1_valcoder = TIMER_INACTIVE; // prim_par.T_z
            valcoder = VALCODER_NO_ROTATE;
            signal_buz(OFF);
            // printf ("������������ ��������� valcoder (%d), � ������ %d - ", event-2, mode.menu);
            switch (mode.menu) {
                case 0:
                    sync_set_par(SYNC_TO_MENU);
                    lcd_menu(mode.menu++); break;   // ������� ���� ��� ��������� ������� printf ("entering...");
                case 1: lcd_menu(event-2); break;       // printf ("navigating...");
                case 2: lcd_edit(event-2); break;       // printf ("editing...");
                default: ;                              // printf ("defaulting...");
            }
            // printf ("\n");
            event = ev_none;            // ������� �������
            break;
        case ev_enter:                  // ���� ����� Enter
            // LAMP_ECHO_PORT |= (1<<LAMP_ECHO_PIN); timer1_lamp = ENTER_CANCEL_OVERFLOW; if (mode.menu <= 2)
            key_treated[2]=1;
            event = ev_none;            // ������� �������
            signal_buz(SHORT);
            timer1_valcoder = TIMER_INACTIVE;     // prim_par.T_z ��������� ������ ������������
            switch (mode.menu) {
                // lcd_primary_screen();
                // ������������ ������� enter c ������ ����, ��� �������� mode.menu ��� ������
                case 0:
                    sync_set_par(SYNC_TO_MENU);
                    lcd_menu(mode.menu++); break;           // ���� ���������� � ������� ������, ��������� ���������� ����
                case 1: lcd_init_edit(); break;             // ���� ���������� � ����, �� �����������
                case 2: lcd_save_edit(); mode.menu = 1; break;
            };
            if (mode.run == mo_setup_input || mode.run == mo_setup_output) mode.run = mo_reg;
            break;
        case ev_timer:
            // ��������� ������ ������������
            if (mode.menu) timer1_valcoder = TIMER_INACTIVE; // prim_par.T_z.....TIMER_INACTIVE;
        case ev_cancel:
            if (event == ev_cancel) {
                signal_buz(SHORT);
                key_treated[3]=1;
                timer1_valcoder = TIMER_INACTIVE;     //prim_par.T_z  ....��������� ������ ������������ ()    
            }
            event = ev_none;            // ������� �������
            // print_prim_par((unsigned char *)&prim_par, sizeof(prim_par));
            switch (mode.menu) {
                case 0: lcd_primary_screen(); break;
                case 1: --mode.menu; lcd_esc_edit(); break;
                case 2: --mode.menu; lcd_esc_edit(); break;
                case 3: mode.menu = 1; break;
            };
            if (mode.run == mo_setup_input || mode.run == mo_setup_output) mode.run = mo_reg;
            lcd_clrscr();
            break;
        case ev_start:      // ���� ��
            key_treated[1]=1;
            printf ("������ ������ ����.\n");  
            switch (mode.run) {
                case mo_reg:  // ������� �� �����
                    mode.run = mo_to;                // �� �����
                    timer_start = T_z; // ��������� ������ STRT
                    TAP_ANGLE = prim_par.PWM_hi;
                    printf("�� �����. ���������� LIMIT = %d, ����� �� ���������� = %d\n", TAP_ANGLE, timer_start);
                    signal_green(MEANDR);
                    signal_white(OFF);
                    time_integration = 0;
                    signal_buz(LONG);
                    timer_stop = 0; 
                    break;
                case mo_to: 
                    signal_buz(SHORT);
                    printf("�� �����.\n"); 
                    break; //mode.run = mo_to;
                default: break;
            };  
            event = ev_none;
            break;
        case ev_to_nf:  // ���������� �������� �� �����
            alarm_reg(0, 1, get_alarm_str(4), 4);
            printf ("������: %s\n", get_alarm_str(4));
            RELE = 1;
            signal_red(ON);
            signal_green(OFF);
            signal_buz(MEANDR);
        case ev_stop:
            key_treated[0]=1;
            printf ("������ ������ ����. ");
            switch (mode.run) {
                case mo_reg: 
                    signal_buz(SHORT); signal_green(ON); 
                    printf(" ������ �� �������.\n"); 
                    break; // mode.run = mo_reg ;
                case mo_to: 
                    mode.run = mo_reg;
                    timer_start = 0;
                    timer_stop = 0;
                    if (RELE == 0) signal_buz(LONG);
                    printf ("��������� �� �����\n");
                    time_integration = 0;
                    prim_par.TO.status = 0; 
                    break; 
                default: break;
            };
            event = ev_none;
            break;
        case ev_alarm1:   // �������� ����� �1
            signal_red(MEANDR); signal_buz(MEANDR);  
            mode.run = mo_control_line1;
            TAP_ANGLE = prim_par.PWM_hi;
            alarm_reg(0, 1, get_alarm_str(0), 0);
            if (!mode.print) printf ("������: %s\n", get_alarm_str(0));
            event = ev_none;
            break;
        case ev_alarm2:   // �������� ����� �2
            signal_red(MEANDR); signal_buz(MEANDR);
            mode.run = mo_control_line2;
            TAP_ANGLE = prim_par.PWM_lo;
            alarm_reg(0, 1, get_alarm_str(1), 1);
            if (!mode.print) printf ("������: %s\n", get_alarm_str(1));
            event = ev_none;
            break;
        case ev_tonormal:
            signal_red(OFF); signal_buz(OFF);
            mode.run = mo_reg;
            alarm_unreg(0); alarm_unreg(1);
            if (!mode.print) printf ("������ ���������!\n");
            event = ev_none;
            break;
        case ev_term1_nf:   // ��������� �1 ���������
            alarm_reg(MAX_OFFLINES, termometers[0].err, get_alarm_str(2), 2);
            signal_red(ON); signal_buz(MEANDR);
            signal_green(OFF);
            mode.run = mo_reg;
            RELE = 1;
            TAP_ANGLE = prim_par.PWM_hi;
            printf ("������: %s\n", get_alarm_str(2));
            event = ev_none;
            break;
        case ev_term2_nf:  // ��������� �2 �����
            alarm_reg(MAX_OFFLINES, termometers[1].err, get_alarm_str(3), 3);
            signal_red(ON); signal_buz(MEANDR);
            signal_green(OFF);
            mode.run = mo_reg;
            RELE = 1;
            TAP_ANGLE = prim_par.PWM_hi;
            printf ("������: %s\n", get_alarm_str(3));
            event = ev_none;
            break;
        default:
            break;
    };
}
// ��������� ������ ������ �������
void mode_processing(void) {
    // � ���� ������� ��������� ��������� ������� � ��������� �������. ������ ���������!
    switch (mode.run) {
        case mo_reg:     // �������� �����
            if (IS_ALERT == 0) {
                RELE = 0; signal_red(OFF);
            }    
            if (time_integration == 0) {
                if ((prim_par.Ki == 0) && (prim_par.Kd == 0)) {
                    if ( abs(TW_CALC - TW_x) >= prim_par.set_delta)  
                        update_PID(TW_CALC - TW_x); // ������� ����� T ��� � � ���������  
                } else update_PID(TW_CALC - TW_x); // ������� ����� T ��� � � ��������� 
                    if (OCR0 == prim_par.PWM_hi ) signal_white(ON);
                    else {
                        if (OCR0 == prim_par.PWM_lo ) 
                            signal_white(SHORT);
                        else    
                            signal_white(MEANDR);
                    }
                parameters[3].val_data = calc_percent(TAP_ANGLE, prim_par.PWM_lo, prim_par.PWM_hi);            
                time_integration = prim_par.T_int;
                OCR0 = (unsigned char)TAP_ANGLE;
                signal_green(ON);
            }
            break;
        case mo_to:     // �� �����  ADC_VAR1, prim_par.ADC_lo, prim_par.ADC_hi
            if (timer_stop == 0) {
                if (ADC_VAR1 >= (prim_par.ADC_hi - 10)) {
                   TAP_ANGLE = prim_par.PWM_lo;
                   timer_start = 0;
                   timer_stop = T_z;
                   printf("�� �����. ���������� LIMIT = %d, ����� �� ���������� = %d\n", TAP_ANGLE, timer_stop);
                }
            }    
            OCR0 = TAP_ANGLE;
            parameters[3].val_data = calc_percent(TAP_ANGLE, prim_par.PWM_lo, prim_par.PWM_hi);
            //if (!mode.print) printf("-");
            break;
        case mo_setup_input:     // ��������� ������
            OCR0 = menu_value;
            break;
        case mo_setup_output:     
            break;
        case mo_control_line1:
        case mo_control_line2:
            OCR0 = TAP_ANGLE;
            break;
        default:    // ��������� �����
            break;
    };
}
// ��������� �� �������������� ��������� ������������ �����������
int check_range(int result) { 
    if (result < prim_par.PWM_lo) {
        result = prim_par.PWM_lo;
       }
    else          
        if (result > prim_par.PWM_hi) {
            result = prim_par.PWM_hi;
           }        
    return((unsigned char)result);
}
/* ������� ����������������� �������������
void update_P(int error) {
    TAP_ANGLE = TAP_ANGLE + (long) prim_par.Ku * error / 1000;
    TAP_ANGLE = check_range(TAP_ANGLE);
    if (mode.print == 2)
        printf(
            "�������� ����������: %d, TAP_ANGLE :%d, time_integration :%d \n",
            error, ((TAP_ANGLE*100)/0xFF), time_integration
        );
}
*/
// ������� PID �������������
#define iMIN -32763
#define iMAX 32764
void update_PID(int error) {
    static long dState = 0, iState = 0;
    long pTerm, dTerm, iTerm, result;
    int tmp_angle;
    pTerm = (long) error * prim_par.Ku;    // calculate the proportional term
    iState += error;                // calculate the integral state with appropriate limiting
     // ��������� �� �������������� ��������� 
     if (iState > iMAX || iState < iMIN) { 
        iState = (iState > iMAX) ? iMAX : iMIN;
    }
    iTerm = ((long) prim_par.Ki * iState) / 100;    // calculate the integral term
    dTerm = (long) (prim_par.Kd) * (dState - TW_x);
    dState = TW_x;
    result = (pTerm + iTerm + dTerm) / 1000;
    tmp_angle = TAP_ANGLE + result;
    TAP_ANGLE = tmp_angle;
    TAP_ANGLE = check_range(tmp_angle);
    switch (mode.print) {
        case 1: 
            // ����������� ��������, ����������� ����������, Delta, ���� ����� ���������, ���� ���������������, ���� �����������, result, pTerm, iTerm, dTerm, ������� ���������,������� �����������\n"
            printf("%02u:%02u:%02u, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
                s_dt.cHH, s_dt.cMM, s_dt.cSS, 
                TW_CALC, TW_x, error, 
                TAP_ANGLE, ADC_VAR1,
                pTerm, iTerm, dTerm, (pTerm + iTerm + dTerm), result
            );
            break;
        case 2:          
            printf(
                "Delta (x100) = %d, TAP_ANGLE = %d, result = %d, pTerm = %d, iTerm = %d, dTerm = %d, TW_x = %d\n", 
                error, TAP_ANGLE, result, pTerm/1000, iTerm/1000, dTerm/1000, TW_x
            );
            break;
        case 0:
        default: break;
    }    
}
// ������ ���� �����������
void printallterms(void) {
    int term;
    register byte i;
    unsigned char resolution;
    signed char th, tl;
    unsigned char *spd;

    if(!ds1820_devices) return;			// ���� ���������� �� ���������� - ������ ������� �� �������
    spd = ds1820_show_spd();         
    // printf("\t");						// �������� ���� ��������� � ���������
    for(i=0; i<ds1820_devices; i++) {
        ds1820_get_resolution(&ds1820_rom_codes[i][0], &resolution);
    	term = ds1820_temperature(&ds1820_rom_codes[i][0]);
        tl = th = 0;
        ds1820_get_alarm(&ds1820_rom_codes[i][0], &tl, &th);
        printf(" t%-u = %i(%-i.%-u)C[%02X%02X:%02x]%X:%X; ", i+1, term, term/100, abs(term%100), spd[1], spd[0], resolution, tl, th);
    }
    printf("\n");
}
// ������ ����������� (�������� - ����� ���������� ������� � 0)
int read_term(byte num) {
    // float lt;
    // lt = termometers[num].scale / 127;
    // lt = (float)prim_par.elims[num].scale / 128 + 1;
    // lt = (termometers[num].t * lt) + prim_par.elims[num].shift;
    return termometers[num].t;
};
// ��������� ����������� ������� �����
void lcd_primary_screen(void) {
    char run_mod [8];
    register unsigned char i;
    unsigned char c_alerts = 0;
    // unsigned char sign = '+';
    int lcd_term = TW_x;
    #ifndef NODEBUG
    // printf ("%02u:%02u ������ ���������� ������...", s_dt.cMM, s_dt.cSS);
    #endif
    // ������������ ���-�� �������� ������ � �������
    for (i = 0; i < MAX_ALARMS; i++) {
        if (prim_par.alarm_status[i]) c_alerts++;
    }
    switch (mode.run) {
        case mo_reg: sprintf(run_mod, "������  "); break;
        case mo_to: sprintf(run_mod, "���� TO "); break;
        default: break;
    };
    if (c_alerts) {
        sprintf(run_mod, "������ ");
    }
    else {
        signal_red(OFF);  signal_buz(OFF); // signal_green(OFF);
    }
    lcd_command(LCD_DISP_ON);       // ������� ������ � LCD
    lcd_gotoxy(0,0);        // ������������� ������ � ������� 0 ������ ������
    sprintf(linestr, "%02u:%02u:%02u %02u.%02u %01u", s_dt.cHH, s_dt.cMM, s_dt.cSS, s_dt.cdd, s_dt.cmo, s_dt.dayofweek);
    lcd_puts(linestr);
    lcd_gotoxy(0,1);                // ������������� ������ � ������� 0 ������ 2
    // ������� ���������� � �����������
    if(ds1820_devices) {
        // sign = (lcd_term < 0) ? '-' : '+';
        // sprintf(linestr, "t=%c%02u.%01uC %s", sign, abs(lcd_term)/100, (abs(lcd_term)%100)/10, run_mod);
        sprintf(linestr, "t=%3u.%01uC %s", abs(lcd_term)/100, (abs(lcd_term)%100)/10, run_mod);
        // linestr += "panarin";
    } else {
        sprintf(linestr, "��� �����������");
    }
    lcd_puts(linestr);
    #ifndef NODEBUG
    // printf ("����� � %02u:%02u\n", s_dt.cMM, s_dt.cSS);
    #endif
}
// ������ ��������� prim_par
void print_prim_par(unsigned char *struct_data, unsigned char size) {
    register unsigned char i;
    unsigned char s_byte; 
    printf("��������� prim_par [%d bytes]: ", size);
    for (i=0; i<size; i++) {
        s_byte = *struct_data++;
        if (i < (size - 36)) 
            printf(" %d", s_byte);
        else 
            printf(" %X", s_byte);
        //
    }
    printf("\n");
}
void ask_turn_off(void) {
    printf("��������� ������� � ���������� ��� ������������\n");
    while(1);
}
// ��������� ������ ���������� ������������� �� 1-� ����������.
void init_force_term(signed char number) {
    // �������� ��������� ����������
    byte inbyte = 0;
    // ������
    #asm("cli")
    printf("��������� ��� ����������, ����� [%02x] � ������� ������� SHIFT-'~'\n����� �� ������ - ���������� �������\n", number);
    printallterms();
    do
        if (UCSRA & RX_COMPLETE) inbyte = UDR;
    while (inbyte != 0x7E);  // 0x7E = SHIFT-'~'
    ds1820_devices = w1_search(0xf0, ds1820_rom_codes);
    delay_ms (DS1820_ALL_DELAY);
   
    if (!ds1820_devices)
        printf("���������� �� �������\n");
    else
        if (ds1820_set_alarm(&ds1820_rom_codes[0][0], number, OUR_SIGNATURE))
            printf("��������� [%02x] ������� ������������������\n", number);
        else
            printf("�� ���� ������������������� ����� ��������� [%02x]\n", number);
    printallterms();
    ask_turn_off();
    #asm("sei")
}
void set_term(signed char number, signed char sign) {
    if (sign == OUR_SIGNATURE) {
        if (ds1820_set_alarm(&ds1820_rom_codes[number][0], -2 - number, sign))
            printf("��������� [%02x] ������� ������������������\n", number);
        else
            printf("�� ���� ������������������� ��������� [%02x]\n", number);
    } else { 
        if (ds1820_set_alarm(&ds1820_rom_codes[number][0], 1, sign))
            printf("��������� [%02x] ������� ������\n", number);
        else
            printf("�� ���� �������� ��������� [%02x]\n", number);
    }
}
// ������������ �����
void toggle_sound(void) {
    if (mode.sound)
        printf("���� ��������\n");
    else
        printf("���� �������\n");
    mode.sound = !mode.sound;
}
// ������������ ������
void toggle_print(void) {
    mode.print++;
    if (mode.print > 2) mode.print = 0;
    if (mode.print)
        printf("������ ��������. ����� %d\n", mode.print); 
        //printf("������ ���������\n �����, ����������� ��������, ����������� ����������, Delta, ���� ����� ���������, ���� ���������������, ���� �����������, result, pTerm, iTerm, dTerm, ������� ���������, ������� �����������\n");
    else
        printf("������ ���������\n");
        //printf("������ ��������\n");
}
// ��������� ������� �� ��������� �����
void check_serial(void) {
    byte inbyte;    // �������� ��������� ����������
    register byte i;
    // ������������ ���������������� ����
    if (UCSRA & RX_COMPLETE) { // ������ �� ���-������
        inbyte = UDR;
        switch (inbyte) {
            case 0x7A:  /* 'z' */ 
                printf("�����: %02u:%02u:%02u, ����:%02u.%02u.%02u, ������� %u �����������\n",
                        s_dt.cHH, s_dt.cMM, s_dt.cSS, s_dt.cdd, s_dt.cmo, s_dt.cyy, ds1820_devices);
                printallterms();
                break;
            case 0x73:  /* 's' */     // ������������ �����
                toggle_sound(); break;   
            case 0x77:  /* 'w' */     // ������ 
                printf ("C������ = %u\n", (unsigned char)event);
                break;
            case 0x78:  // ������ 'x' 
                toggle_print(); break;
                // print_curr_menu(); break; // �������� ������� ����                                            
            case 0x65:  // ������ 'e'
                alarm_all_print();
                break;
            case 0x64:  // ������ 'd'
                for (i=0; i < MAX_ALARMS; i++) {
                    if (!(alarm_unreg(i))) {
                       printf ("������� �������� �������: %s\n", get_alarm_str(i));
                    }
                }
                printf ("�������������� ��� ����������!\n");
                read_all_terms(INIT_MODE);
                break;
            case 0x63:                // ������ 'c'
                print_prim_par((unsigned char *)&prim_par, sizeof(prim_par));
                break;
            case 0x6D:                // ������ 'm'
                for(i=0; i<NUM_PARAMETERS; i++) printf("%s\t", param_str(i, parameters));
                printf("\n");
                break;
            case 0x6E:                // ������ 'n'
                for(i=0; i<NUM_MENU; i++) printf("%s\t", param_str(i, main_menu));
                printf("\n");
                break;
            case 0x4a:              // ������ 'j'
                poll_keys();
                break;
            case 0x75:                // ������ 'u' ������ �� �����
                init_force_term(0xFE); break;
            case 0x69:                // ������ 'i' ����    ������ �� �����
                init_force_term(0xFD); break;
            case 0x6F:                // ������ 'o' ���� 
                init_force_term(0xFC); break;
            case 0x70:                // ������ 'p'
                init_force_term(0xFB); break;
            case 0x3e:              /* Shift + '>' */
                set_term(1, 1); break;
            case 0x3c:              /* Shift + '<' */
                set_term(0, 1); break;
            case 0x4d:              /* Shift + 'M' */
                set_term(0, OUR_SIGNATURE); break;
            case 0x3f:              /* Shift + '?'*/
                set_term(1, OUR_SIGNATURE); break;
            case 0x39:              // ������ '9'
                signal_printallbytes();
                break;
            default:
                printf("����� ������ 0x%x\n", inbyte);
        };
    }
}
// ������� ����������� ������� ���������� ��, � ���� ���� - ���������� ��������� ��������
char high_time_TO(void) {
    if (TA_x > prim_par.ta_max) { 
        if (prim_par.TO.weekday == s_dt.dayofweek) {
            if (prim_par.TO.hour == s_dt.cHH) {
                if (prim_par.TO.minute == s_dt.cMM) {
                    if (prim_par.TO.status == 0) {
                        return (1);
                    }
                }
            }
        }
    }    
    return (0);
}