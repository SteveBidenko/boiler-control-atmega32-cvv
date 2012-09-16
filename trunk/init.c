/*****************************************************
This program was produced by the
CodeWizardAVR V2.03.9 Standard
Automatic Program Generator
� Copyright 1998-2008 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : Robo Water
Version : 1.0
Date    : 02.03.2011
Author  : Admin
Company : Microsoft
Comments:


Chip type               : ATmega32
Program type            : Application
AVR Core Clock frequency: 3686400 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 512
*****************************************************/
#include <mega32.h>
// I2C Bus functions
#asm
   .equ __i2c_port=0x15 ;PORTC
   .equ __sda_bit=1
   .equ __scl_bit=0
#endasm
#include <i2c.h>
// DS1307 Real Time Clock functions
#include <ds1307.h>
// 1 Wire Bus functions
#asm
   .equ __w1_port=0x18 ;PORTB
   .equ __w1_bit=0
#endasm
#include <1wire.h>
// Standard Input/Output functions
#include <stdio.h>
#include <stdlib.h>
#include <delay.h>
#include "robowater.h"
#include "spd1820.h"
#include "valcoder.h"
#include "lcd_4bit.h"
#include "menu.h"
#include "signals.h"
#include "at2404.h"
#include "keys.h"
#define NODEBUG
// �������� ��������� �������
unsigned int read_adc(unsigned char);
// ���������� timer1
byte timer1_valcoder = 0;
unsigned int timer_start = 0;
byte timer_stop = 0;

// ������������ ����������
interrupt [EXT_INT2] void ext_int2_isr(void) {
    #ifndef NODEBUG
    // printf ("%02u:%02u ������ ���������� �������...", s_dt.cMM, s_dt.cSS);
    // printf (".");
    #endif
    if (mode.run == mo_to) {
        if (timer_start) {
            timer_start--;
                // ���� �������� ����, �� ������� ev_to_nf
            if (!timer_start && (ADC_VAR1 < prim_par.ADC_hi)) {
                printf ("����������� ����� �������� \n");
                event = ev_to_nf;
            }
        }
        if (timer_stop) {
            timer_stop--;
            if (ADC_VAR1 <= (prim_par.ADC_lo + 5)) {
               printf ("�������� ���������� �� �����. \n");
               event = ev_stop;
               }
               else { 
            // ���� �������� ����, �� ������� ev_to_nf
                if (!timer_stop && (ADC_VAR1 > (prim_par.ADC_lo + 10))) {         
                    printf ("����������� ����� �������� \n");
                    event = ev_to_nf;
                }    
            }
        }
    }   
   
     // ���� timer1_counter > 0, ��������� ��� ��������
    if (timer1_valcoder) {
        timer1_valcoder--;
        // ���� �������� ����, �� ������������� ������������ valcoder
        if (!timer1_valcoder) {
            if (event)
                timer1_valcoder++;
            else
                event = ev_timer;
        }
    }
    if (!mode.stop_sync_dt) get_cur_dt(0);
    read_all_terms(DUTY_MODE);
    // ���������� ���
    //ADC_VAR2 = read_adc(1)/4;
    ADC_VAR1 = read_adc(0)/4;
    if (time_integration) time_integration--;
    #ifndef NODEBUG
    // printf ("����� � %02u:%02u\n", s_dt.cMM, s_dt.cSS);
    #endif
    if CHECK_EVENT {
        // ����� ����� ������� ���� ������� �������, ���� �� ����� 
        // if (s_dt.
        event = ev_secunda;
        #ifndef NODEBUG
        // printf ("%02u:%02u ������������� ������� �������\n", s_dt.cMM, s_dt.cSS);
        #endif
    }
    #ifndef NODEBUG
    else
        printf ("������� ������� = %u\n", (unsigned char)event);
    #endif
    // parameters[0].val_data = termometers[0].t;
}
// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void) {
// ������ ������������. ������������� ��� PWM
}
// Timer 1 overflow interrupt service routine
interrupt [TIM1_OVF] void timer1_ovf_isr(void) {
    update_signal_status();
    if (t_key) t_key--;
}
// Timer 2 overflow interrupt service routine
interrupt [TIM2_OVF] void timer2_ovf_isr(void) {
// ������ ������������. ������������� ��� PWM
}
#define ADC_VREF_TYPE 0xC0
// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input) {
    ADMUX=adc_input | (ADC_VREF_TYPE & 0xff);
    delay_us(10);   // Delay needed for the stabilization of the ADC input voltage
    ADCSRA|=0x40;   // Start the AD conversion
    while ((ADCSRA & 0x10)==0); // Wait for the AD conversion to complete
    ADCSRA|=0x10;
    return ADCW;
}
#pragma used+
void set_cur_dt (void) {
    #asm("cli")
    rtc_set_time(s_dt.cHH, s_dt.cMM, s_dt.cSS);
    rtc_set_date(s_dt.cdd, s_dt.cmo, s_dt.cyy);
    #asm("sei")
    delay_ms (200);
    get_cur_dt(FORCE_INIT);
    // rtc_set_date(3, 13, 11); 	// 03.13.2011
    // rtc_set_time(17, 22, 0); 	// 17:22:00
}

void get_cur_dt (unsigned char force) {
    if (force) {
        rtc_init(0,1,0);            // ������������� ������������� ������������ ���������
        #ifndef NODEBUG
        printf ("������������������� ������������ ����������\n");
        #endif
    }
    // �����
    rtc_get_time(&s_dt.cHH,&s_dt.cMM,&s_dt.cSS);
    // ����
    rtc_get_date(&s_dt.cdd,&s_dt.cmo,&s_dt.cyy);
}

#pragma used-
// ������������� UART. �������� �� ����� �������� #define BAUDRATE
void init_uart(void) {
	word bauddiv;
	// USART initialization Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART Receiver: On
    // USART Transmitter: On
	UCSRA=0x00; UCSRC=0x86; UCSRB = (1<<TXEN) | (1<<RXEN);
    bauddiv = ((_MCU_CLOCK_FREQUENCY_ + (BAUDRATE * 8L))/(BAUDRATE * 16L) - 1);
    UBRRL = (unsigned char)bauddiv;
	UBRRH = bauddiv >> 8;
}
// ������������� ���� ����������� � �������
void init_terms(void) {
    printf ("����� ���� ����������� �� ���� 1-Wire. �������: ");
    ds1820_devices = w1_search(0xf0,ds1820_rom_codes);
    delay_ms (DS1820_ALL_DELAY);
    printf ("%d ����\n", ds1820_devices);
}
// �������� ����������������� ������
void init(void) {
    // byte tmp_ret; // byte resolution;
    // signed char tmp_alarm_h, tmp_alarm_l;
    // Port A initialization
    /* Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=Out Func1=In Func0=In
    // State7=0 State6=0 State5=0 State4=0 State3=0 State2=0 State1=T State0=T
    */
    PORTA=0x00; DDRA=0xFC;

    // Port B initialization
    /* Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=In Func1=In Func0=In
    // State7=0 State6=0 State5=0 State4=0 State3=0 State2=P State1=P State0=P
    */
    PORTB=0x07; DDRB=0xF8;

    // Port C initialization
    /* Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
    // State7=P State6=P State5=P State4=P State3=P State2=P State1=P State0=P
    */
    PORTC=0xFF; DDRC=0x00;

    // Port D initialization
    /* Func7=Out Func6=In Func5=Out Func4=Out Func3=In Func2=In Func1=Out Func0=In
    // State7=0 State6=P State5=0 State4=0 State3=P State2=P State1=0 State0=P
    PORTD=0x4D; DDRD=0xB2;
    */
    /* Func7=Out Func6=Out Func5=Out Func4=Out Func3=In Func2=In Func1=Out Func0=In
    // State7=0 State6=0 State5=0 State4=0 State3=P State2=P State1=0 State0=P
     */
    PORTD=0x0D; DDRD=0xF2;
    // Timer/Counter 0 initialization
    /* Clock source: System Clock
    // Clock value: 31,250 kHz
    // Mode: Fast PWM top=FFh
    // OC0 output: Non-Inverted PWM
    */
    TCCR0=0x6b;
    // TCCR0=0x6C; ������� �� TCCR0=0x6b 18.07.2012
    TCNT0=0x00; OCR0=0x00;
    // Timer/Counter 1 initialization
    /* Clock source: System Clock
    // Clock value: 460,000 kHz (142 mc - 1 cycle)
    // Mode: Normal top=FFFFh
    // OC1A output: Discon.
    // OC1B output: Discon.
    // Noise Canceler: Off
    // Input Capture on Falling Edge
    // Timer 1 Overflow Interrupt: On
    // Input Capture Interrupt: Off
    // Compare A Match Interrupt: Off
    // Compare B Match Interrupt: Off
    */
    TCCR1A=0x00; TCCR1B=0x02; TCNT1H=0x00; TCNT1L=0x00; 
    ICR1H=0x00; ICR1L=0x00; 
    OCR1AH=0x00; OCR1AL=0x00; OCR1BH=0x00; OCR1BL=0x00;
    // Timer/Counter 2 initialization
    /* Clock source: System Clock
    // Clock value: 31,250 kHz
    // Mode: Fast PWM top=FFh
    // OC2 output: Inverted PWM
    */
    // ASSR=0x00; TCCR2=0x7E; TCNT2=0x00; OCR2=0x00;
    ASSR=0x00;
    // TCCR2=0x6c; ������� �� TCCR0=0x6b 18.07.2012
    TCCR2=0x6b; 
    TCNT2=0x00; OCR2=0x00;
    // External Interrupt(s) initialization
    /*
    INT0: On INT0 Mode: Falling Edge
    INT1: On INT1 Mode: Falling Edge
    INT2: On  INT2 Mode: Falling Edge
    MCUCR=0x0A;
    INT0: On INT0 Mode: Rising Edge
    INT1: On INT1 Mode: Rising Edge
    INT2: On  INT2 Mode: Falling Edge
    MCUCR=0x0F;
    INT0: On INT0 Mode: Rising Edge
    INT1: On INT1 Mode: Any change
    INT2: On  INT2 Mode: Falling Edge
    MCUCR=0x07;
    */
    GICR|=0xE0; MCUCR=0x06; MCUCSR=0x00; GIFR=0xE0;
    // Timer(s)/Counter(s) Interrupt(s) initialization
    TIMSK=0x45;

    // USART initialization
    init_uart ();

    // Analog Comparator initialization
    /* Analog Comparator: Off
    // Analog Comparator Input Capture by Timer/Counter 1: Off
    */
    ACSR=0x80; SFIOR=0x00;

    // ADC initialization
    /* ADC Clock frequency: 125,000 kHz
    // ADC Voltage Reference: Int., cap. on AREF
    // ADC Auto Trigger Source: Free Running
    */
    ADMUX=ADC_VREF_TYPE & 0xff; ADCSRA=0xA6; SFIOR&=0x1F;
    printf ("����� ������������� ���������...\n"); delay_ms(500);
    // I2C Bus initialization
    i2c_init();
    // DS1307 Real Time Clock initialization
    /* Square wave output on pin SQW/OUT: On
    // Square wave frequency: 1Hz
    */
    // �������������� ���� ��������� ������� ����� ���������� ���� ���������
    mode.stop_sync_dt = 0;      // ������� ���� ������� �������������
    get_cur_dt(FORCE_INIT);                   // (1)
    // �������������� ��� ����������
    init_terms();
    // �������������� �������
    lcd_init(LCD_DISP_ON);
    // �������������� valcoder
    valcoder = VALCODER_NO_ROTATE;
    // �������������� ����
    init_curr_menu(&main_menu[0], NUM_MENU);
    // �������������� ��� �������
    signal_green(OFF); signal_red(OFF); signal_buz(OFF); signal_white(OFF);
} // ����� ������� init
