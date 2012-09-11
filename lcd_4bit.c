// #include <avr_compiler.h>
#include <io.h>
#include <delay.h>
#include "robowater.h"
#include "lcd_4bit.h"

#define _BV(x) (1 << (x))
// #define lcd_e_delay()   #asm("rjmp 1f\ 1:")
#define lcd_e_delay()   delay_us(250)
#define lcd_e_high()    LCD_E_PORT  |=  _BV(LCD_E_PIN);
#define lcd_e_low()     LCD_E_PORT  &= ~_BV(LCD_E_PIN);
#define lcd_rw_high()   LCD_RW_PORT |=  _BV(LCD_RW_PIN)
#define lcd_rw_low()    LCD_RW_PORT &= ~_BV(LCD_RW_PIN)
#define lcd_rs_high()   LCD_RS_PORT |=  _BV(LCD_RS_PIN)
#define lcd_rs_low()    LCD_RS_PORT &= ~_BV(LCD_RS_PIN)

#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_2LINES
#pragma used+
// Устанавливаем коротку задержку в 32 цикла процессора
static void _lcd_delay(void) {
#asm
    ldi   r31,15
__lcd_delay0:
    dec   r31                   ; 1 cycle
    brne  __lcd_delay0          ; 1 cycle
#endasm
}
// Устанавливаем длинную задержку в 262140 (4*65535) циклов процессора
static void _long_delay(void) {
#asm
    clr   r26
    clr   r27
__long_delay0:
    sbiw  r26,1         ;2 cycles
    brne  __long_delay0 ;2 cycles
#endasm
}
// Анализ состояния LCD
/* void _lcd_ready(void)
{
#asm
    in    r26,__lcd_direction
    andi  r26,0xf                 ;set as input
    out   __lcd_direction,r26
    sbi   __lcd_port,__lcd_rd     ;RD=1
    cbi   __lcd_port,__lcd_rs     ;RS=0
__lcd_busy:
#endasm
    _lcd_delay();
#asm
    sbi   __lcd_port,__lcd_enable ;EN=1
#endasm
_lcd_delay();
#asm
    in    r26,__lcd_pin
    cbi   __lcd_port,__lcd_enable ;EN=0
#endasm
    _lcd_delay();
#asm
    sbi   __lcd_port,__lcd_enable ;EN=1
#endasm
    _lcd_delay();
#asm
    cbi   __lcd_port,__lcd_enable ;EN=0
    sbrc  r26,__lcd_busy_flag
    rjmp  __lcd_busy
#endasm
}
*/
#pragma used-

static void lcd_e_toggle(void);
// #define delay(us) _delayFourCycles( ( ( 1*(_MCU_CLOCK_FREQUENCY_/4000) )*us)/1000 )

static void lcd_e_toggle(void) {
    lcd_e_high();
    lcd_e_delay();
    lcd_e_low();
}

static void lcd_write(byte data, byte rs) {
    /* write data (RS=1, RW=0) or write instruction (RS=0, RW=0) */
    if (rs) lcd_rs_high(); else lcd_rs_low();
    lcd_rw_low();
    /* configure data pins as output */
    LCD_DATA0_DDR |= _BV(LCD_DATA0_PIN);
    LCD_DATA1_DDR |= _BV(LCD_DATA1_PIN);
    LCD_DATA2_DDR |= _BV(LCD_DATA2_PIN);
    LCD_DATA3_DDR |= _BV(LCD_DATA3_PIN);
    /* output high nibble first */
    LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
    LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
    LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
    LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);
    if(data & 0x80) LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
    if(data & 0x40) LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
    if(data & 0x20) LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
    if(data & 0x10) LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
    lcd_e_toggle();
    /* output low nibble */
    LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
    LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
    LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
    LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);
    if(data & 0x08) LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
    if(data & 0x04) LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
    if(data & 0x02) LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
    if(data & 0x01) LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
    lcd_e_toggle();
    /* all data pins high (inactive) */
    LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
    LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
    LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
    LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
}

static byte lcd_read(byte rs) {
    byte data;

    if (rs)
        lcd_rs_high();                       /* RS=1: read data      */
    else
        lcd_rs_low();                        /* RS=0: read busy flag */
    lcd_rw_high();                           /* RW=1  read mode      */
    /* configure data pins as input */
    LCD_DATA0_DDR &= ~_BV(LCD_DATA0_PIN);
    LCD_DATA1_DDR &= ~_BV(LCD_DATA1_PIN);
    LCD_DATA2_DDR &= ~_BV(LCD_DATA2_PIN);
    LCD_DATA3_DDR &= ~_BV(LCD_DATA3_PIN);
    /* read high nibble first */
    lcd_e_high();
    lcd_e_delay();
    data = 0;
    if ( LCD_DATA0_DPIN & _BV(LCD_DATA0_PIN) ) data |= 0x10;
    if ( LCD_DATA1_DPIN & _BV(LCD_DATA1_PIN) ) data |= 0x20;
    if ( LCD_DATA2_DPIN & _BV(LCD_DATA2_PIN) ) data |= 0x40;
    if ( LCD_DATA3_DPIN & _BV(LCD_DATA3_PIN) ) data |= 0x80;
    lcd_e_low();
    lcd_e_delay();                       /* Enable 500ns low       */
    /* read low nibble */
    lcd_e_high();
    lcd_e_delay();
    if ( LCD_DATA0_DPIN & _BV(LCD_DATA0_PIN) ) data |= 0x01;
    if ( LCD_DATA1_DPIN & _BV(LCD_DATA1_PIN) ) data |= 0x02;
    if ( LCD_DATA2_DPIN & _BV(LCD_DATA2_PIN) ) data |= 0x04;
    if ( LCD_DATA3_DPIN & _BV(LCD_DATA3_PIN) ) data |= 0x08;
    lcd_e_low();
    return data;
}

static unsigned char lcd_waitbusy(void) {
	// register unsigned char c;
    while (lcd_read(0) & (1<<LCD_BUSY)) {}
    delay_us(10);
    return (lcd_read(0));
}

static inline void lcd_newline(byte pos) {
    register unsigned char addressCounter;


    if ( pos < (LCD_START_LINE2) )
        addressCounter = LCD_START_LINE2;
    else
        addressCounter = LCD_START_LINE1;
    lcd_command((1<<LCD_DDRAM)+addressCounter);
}

void lcd_command(byte cmd) {
    lcd_waitbusy();
    lcd_write(cmd,0);
}

void lcd_data(byte data) {
    lcd_waitbusy();
    lcd_write(data,1);
}

void lcd_gotoxy(byte x, byte y) {
    if ( y==0 )
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
}

#pragma used+
int lcd_getxy(void) {
    return lcd_waitbusy();
}
#pragma used-

void lcd_clrscr(void) {
    lcd_command(1<<LCD_CLR);
}

void lcd_home(void) {
    lcd_command(1<<LCD_HOME);
}

void lcd_putc(byte c) {
    byte pos;
    // byte charoffset;            // байт смещения от начала таблицы
    byte lcd_symtable[64] = {   // 192..255
    //{'А','Б','В','Г','Д','Е','Ж','З','И','Й','К','Л','М','Н','О','П','Р','С','Т','У','Ф','Х','Ц','Ч','Ш','Щ','Ъ','Ы','Ь','Э','Ю','Я',
    // 'а','б','в','г','д','е','ж','з','и','й','к','л','м','н','о','п','р','с','т','у','ф','х','ц','ч','ш','щ','ъ','ы','ь','э','ю','я'},
        65,160, 66,161,224, 69,163,164,165,166, 75,167, 77, 72, 79,168, 80, 67, 84,169,170, 88,225,171,172,226,173,174,173,175,176,177,
        97,178,179,180,227,101,182,183,184,185,186,187,188,189,111,190,112, 99,191,121,228,120,229,192,193,230,194,195,196,197,198,199
    };
    pos = lcd_waitbusy();   // read busy-flag and address counter
    if (c=='\n') lcd_newline(pos);
    else
        if (c < 192) lcd_write(c, 1);
        else lcd_write(lcd_symtable[c-192], 1);
}

void lcd_puts(const char *s) {
    while (*s != 0) lcd_putc(*s++);
}
// Функция печати на LCD строки меню. Если submenu = 1, то печатаем символ '>' на 16 позиции
void lcd_line_menu(const char *s, byte submenu) {
    register byte i;
    for(i=0; i<(LCD_DISP_LENGTH-1); i++) {
        if (*s != 0) lcd_putc(*s++); else lcd_putc(0x20);
    }
    // Если включен флаг submenu, печатаем символ '>', иначе - ' '
    if (submenu) lcd_putc('>'); else lcd_putc(0x20);
}

void lcd_init(byte dispAttr) {
	/* configure all port bits as output (LCD data and control lines on different ports */
    LCD_RS_DDR    |= _BV(LCD_RS_PIN);
    LCD_RW_DDR    |= _BV(LCD_RW_PIN);
    LCD_E_DDR     |= _BV(LCD_E_PIN);
    LCD_DATA0_DDR |= _BV(LCD_DATA0_PIN);
    LCD_DATA1_DDR |= _BV(LCD_DATA1_PIN);
    LCD_DATA2_DDR |= _BV(LCD_DATA2_PIN);
    LCD_DATA3_DDR |= _BV(LCD_DATA3_PIN);
    delay_ms(16);        /* wait 16ms or more after power-on       */

    /* initial write to lcd is 8bit */
    LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);  // _BV(LCD_FUNCTION)>>4;
    LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);  // _BV(LCD_FUNCTION_8BIT)>>4;
    lcd_e_toggle();
    delay_ms(41);         /* delay, busy flag can't be checked here */

    /* repeat last command */
    lcd_e_toggle();
    delay_us(64);           /* delay, busy flag can't be checked here */

    /* repeat last command a third time */
    lcd_e_toggle();
    delay_us(64);           /* delay, busy flag can't be checked here */

    /* now configure for 4bit mode */
    LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);   // LCD_FUNCTION_4BIT_1LINE>>4
    lcd_e_toggle();
    delay_us(64);           /* some displays need this additional delay */

    lcd_command(LCD_FUNCTION_DEFAULT);      /* function set: display lines  */
    lcd_command(LCD_DISP_OFF);              /* display off                  */
    lcd_clrscr();                           /* display clear                */
    lcd_command(LCD_MODE_DEFAULT);          /* set entry mode               */
    lcd_command(dispAttr);                  /* display/cursor control       */
}
