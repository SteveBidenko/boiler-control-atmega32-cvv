#ifndef __LCD__
#define __LCD__

#define LCD_LINES           2
#define LCD_DISP_LENGTH    16
#define LCD_LINE_LENGTH  0x40
#define LCD_START_LINE1  0x00
#define LCD_START_LINE2  0x40
#define LCD_WRAP_LINES      0

#define LCD_PORT         PORTA
#define LCD_DDR          DDRA
#define LCD_PIN          PINA
#define LCD_DATA0_PORT   LCD_PORT
#define LCD_DATA0_DDR    LCD_DDR    // Подстановка для каждой линии управления
#define LCD_DATA0_DPIN   LCD_PIN    // DDRA
#define LCD_DATA1_PORT   LCD_PORT
#define LCD_DATA1_DDR    LCD_DDR    // DDRB
#define LCD_DATA1_DPIN   LCD_DDR    //
#define LCD_DATA2_PORT   LCD_PORT
#define LCD_DATA2_DDR    LCD_DDR
#define LCD_DATA2_DPIN   LCD_DDR    //
#define LCD_DATA3_PORT   LCD_PORT
#define LCD_DATA3_DDR    LCD_DDR
#define LCD_DATA3_DPIN   LCD_DDR    //
#define LCD_DATA0_PIN    4
#define LCD_DATA1_PIN    5
#define LCD_DATA2_PIN    6
#define LCD_DATA3_PIN    7

#define LCD_RS_PORT      PORTB
#define LCD_RS_DDR       DDRB
#define LCD_RS_PIN       5
#define LCD_RW_PORT      PORTB
#define LCD_RW_DDR       DDRB
#define LCD_RW_PIN       7      //  ! Внимание новая платаю. Было 6
#define LCD_E_PORT       PORTB
#define LCD_E_DDR        DDRB
#define LCD_E_PIN        6     //   ! Внимание новая платаю. Было 7

#define LCD_CLR               0      /* DB0: clear display                  */
#define LCD_HOME              1      /* DB1: return to home position        */
#define LCD_ENTRY_MODE        2      /* DB2: set entry mode                 */
#define LCD_ENTRY_INC         1      /*   DB1: 1=increment, 0=decrement     */
#define LCD_ENTRY_SHIFT       0      /*   DB2: 1=display shift on           */
#define LCD_ON                3      /* DB3: turn lcd/cursor on             */
#define LCD_ON_DISPLAY        2      /*   DB2: turn display on              */
#define LCD_ON_CURSOR         1      /*   DB1: turn cursor on               */
#define LCD_ON_BLINK          0      /*     DB0: blinking cursor ?          */
#define LCD_MOVE              4      /* DB4: move cursor/display            */
#define LCD_MOVE_DISP         3      /*   DB3: move display (0-> cursor) ?  */
#define LCD_MOVE_RIGHT        2      /*   DB2: move right (0-> left) ?      */
#define LCD_FUNCTION          5      /* DB5: function set                   */
#define LCD_FUNCTION_8BIT     4      /*   DB4: set 8BIT mode (0->4BIT mode) */
#define LCD_FUNCTION_2LINES   3      /*   DB3: two lines (0->one line)      */
#define LCD_FUNCTION_10DOTS   2      /*   DB2: 5x10 font (0->5x7 font)      */
#define LCD_CGRAM             6      /* DB6: set CG RAM address             */
#define LCD_DDRAM             7      /* DB7: set DD RAM address             */
#define LCD_BUSY              7      /* DB7: LCD is busy                    */

/* set entry mode: display shift on/off, dec/inc cursor move direction */
#define LCD_ENTRY_DEC            0x04   /* display shift off, dec cursor move dir */
#define LCD_ENTRY_DEC_SHIFT      0x05   /* display shift on,  dec cursor move dir */
#define LCD_ENTRY_INC_           0x06   /* display shift off, inc cursor move dir */
#define LCD_ENTRY_INC_SHIFT      0x07   /* display shift on,  inc cursor move dir */

/* display on/off, cursor on/off, blinking char at cursor position */
#define LCD_DISP_OFF             0x08   /* display off                            */
#define LCD_DISP_ON              0x0C   /* display on, cursor off                 */
#define LCD_DISP_ON_BLINK        0x0D   /* display on, cursor off, blink char     */
#define LCD_DISP_ON_CURSOR       0x0E   /* display on, cursor on                  */
#define LCD_DISP_ON_CURSOR_BLINK 0x0F   /* display on, cursor on, blink char      */

/* move cursor/shift display */
#define LCD_MOVE_CURSOR_LEFT     0x10   /* move cursor left  (decrement)          */
#define LCD_MOVE_CURSOR_RIGHT    0x14   /* move cursor right (increment)          */
#define LCD_MOVE_DISP_LEFT       0x18   /* shift display left                     */
#define LCD_MOVE_DISP_RIGHT      0x1C   /* shift display right                    */

/* function set: set interface data length and number of display lines */
#define LCD_FUNCTION_4BIT_1LINE  0x20   /* 4-bit interface, single line, 5x7 dots */
#define LCD_FUNCTION_4BIT_2LINES 0x28   /* 4-bit interface, dual line,   5x7 dots */
#define LCD_FUNCTION_8BIT_1LINE  0x30   /* 8-bit interface, single line, 5x7 dots */
#define LCD_FUNCTION_8BIT_2LINES 0x38   /* 8-bit interface, dual line,   5x7 dots */

#define LCD_MODE_DEFAULT     ((1<<LCD_ENTRY_MODE) | (1<<LCD_ENTRY_INC) )
#pragma used+
// Описание функций
extern void lcd_init(unsigned char dispAttr); // ?
extern void lcd_clrscr(void);
extern void lcd_home(void);
extern void lcd_gotoxy(unsigned char x, unsigned char y);
extern void lcd_putc(char c);
extern void lcd_puts(const char *s);
extern void lcd_line_menu(const char *s, unsigned char submenu);
// extern void lcd_puts_p(const char *progmem_s);
extern void lcd_command(unsigned char cmd);
extern void lcd_data(unsigned char data);
#pragma used-
#define lcd_puts_P(__s) lcd_puts_p(PSTR(__s))

#endif
