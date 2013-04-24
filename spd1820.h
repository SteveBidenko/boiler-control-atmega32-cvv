/*
  EXAMPLE FOR PORTB:

    #asm
        .equ __w1_port=0x18
        .equ __w1_bit=3
    #endasm
    #include <ds1820.h>
*/

#ifndef _SPD1820_INCLUDED_
#define _SPD1820_INCLUDED_
#include <1wire.h>
#define DS1820_FAMILY_CODE 0x10
#define DS1820_SEARCH_ROM_CMD 0xf0
#define DS1820_ALARM_SEARCH_CMD 0xec
#define DS1820_ALL_DELAY 200   // изменение с 700
#define OUR_SIGNATURE -69
#define DELTA_MAX 1000
#define INIT_MODE 1
#define DUTY_MODE 0
#define NOT_FOUND -9999
#pragma used+
// Описание внешних переменных (was cnt_rem,cnt_c,)
extern struct __ds1820_scratch_pad_struct {
	unsigned char 	temp_lsb,temp_msb;
    signed char 	temp_high,temp_low;
    unsigned char 	conf,
					res1,res2,res3,
					crc;
} __ds1820_scratch_pad;
extern struct st_terms {
    int t;               // Текущее значение температуры * 100
    unsigned char id;   // Где находится (0 - где-то рядом) ;-)
    int t_last;          // Предыдущее значение температуры
    unsigned char err;  // Количество невероятных данных
} termometers[MAX_DS1820];
extern byte ds1820_rom_codes[MAX_DS1820][9];
// Описание внешних функций
unsigned char sync_ds1820_eeprom(void);
void sync_eeprom_ds1820(void);
void read_all_terms(unsigned char measure_mode);
unsigned char ds1820_select(unsigned char *addr);
unsigned char ds1820_read_spd(unsigned char *addr);
unsigned char ds1820_write_spd(unsigned char *addr);
unsigned char ds1820_run_measure(unsigned char *addr);
int ds1820_temperature(unsigned char *addr);            // без запуска на измерения
int ds1820_temperature_10(unsigned char *addr);         // вычитываем температуру и запускаем на измерения 
int ds1820_temperature_10lh(unsigned char *addr, signed char *temp_low, signed char *temp_high);
unsigned char ds1820_set_alarm(unsigned char *addr, signed char temp_low, signed char temp_high);
signed char ds1820_get_alarm(unsigned char *addr, signed char *temp_low, signed char *temp_high);
unsigned char ds1820_set_resolution(unsigned char *addr, unsigned char resolution);
signed char ds1820_get_resolution(unsigned char *addr, unsigned char *resolution);
// void print_scratch_pad (void);
unsigned char *ds1820_show_spd();
#pragma used-

#endif

