#include <stdio.h>
#include <stdlib.h>
#include <delay.h>
#include <1wire.h>
#include "robowater.h"
#include "spd1820.h"
// #define NODEBUG             // Комментируя эту строку, даем возможность компилятору включить отладочный сервис
/* #ifndef NODEBUG
// Функция печатающая значение __ds1820_scratch_pad
void print_scratch_pad (void) {
    register unsigned char i;
    unsigned char *pointer;
    pointer = (char *) &__ds1820_scratch_pad;
    printf("[ ");
    for (i=0; i<9; i++) printf ("%02X ", *pointer++);
    printf("]");
}
#endif */
// DS1820 devices ROM code storage area, 9 bytes are used for each device (see the w1_search function description in the help)
byte ds1820_rom_codes[MAX_DS1820][9];
struct st_terms termometers[MAX_DS1820];  // Массив значений термометров с их корректировочными параметрами
// Структура для хранения текущего ОЗУ Dallas
struct __ds1820_scratch_pad_struct __ds1820_scratch_pad;
// Функция записи адресов термометра из ds1820_rom_codes в структуру EEPROM, с использованием TH и TL
unsigned char sync_ds1820_eeprom(void) {
    unsigned char result = 0, i, j;
    signed char th, tl, get_alarm;

    for (i = 0; i < ds1820_devices; i++) {
        get_alarm = ds1820_get_alarm(&ds1820_rom_codes[i][0], &tl, &th);                        
        if (get_alarm > 0) {    
            // Адрес термометра [i] начинается с нулевой позиции [0]
            // Если термометр имеет необходимую сигнатуру, то присваеваем значение по смещению. tl
            // Иначе присваиваем в нулевой элемент (температура в помещении)
            for (j = 0; j < 9; j++)
                // Если термометр имеет "левую" сигнатуру, то переписываем его в массив ufo
                if (th == OUR_SIGNATURE)
                    prim_par.addr[abs(tl) - 2][j] = ds1820_rom_codes[i][j];
                else {
                    result++;
                    mode.ufo[j] = ds1820_rom_codes[i][j];
                }
        }
    }
    return result;
}
// Функция чтения адресов термометра из EEPROM в ds1820_rom_codes по порядку
void sync_eeprom_ds1820(void) {
    register byte i, j;
    // Перегоняем все данные из структуры EEPROM в массив термометров
    for (i = 0; i < ds1820_devices; i++)
        for (j = 0; j < 9; j++)
            ds1820_rom_codes[i][j] = prim_par.addr[i][j];
}
// Функция внесения поправки в измерение и сохранения температуры с проверкой на допустимость
void rectification(int unconverted, unsigned char index, unsigned char measure_mode) {
    // float lt;       // параметр для коррекции температуры
    if (index >= MAX_DS1820) return;
    // lt = (float)prim_par.elims[index].scale / 128 + 1;
    // lt = lt * unconverted + prim_par.elims[index].shift;
    // termometers[index].t = (int)lt;
    termometers[index].t = unconverted;
    if (measure_mode == INIT_MODE) {
        if (unconverted == NOT_FOUND)
            termometers[index].err = MAX_OFFLINES;
        else 
            termometers[index].err = 0;
        termometers[index].t_last = termometers[index].t;
    } else {
        // Проверяем разницу между предыдущим измерением и текущим
        if (abs(termometers[index].t - termometers[index].t_last) < DELTA_MAX) {
            termometers[index].t_last = termometers[index].t;
            termometers[index].err = 0;
        } else {
            termometers[index].t = termometers[index].t_last;
            termometers[index].err++;
        }
    }
}
// Чтение всех Dallas с коррекцией
void read_all_terms(unsigned char measure_mode) {
    register byte i;
    // Запись из EEPROM массива адресов термометров в режиме инициализации
    if (measure_mode == INIT_MODE) {
        sync_eeprom_ds1820();
        for(i = 0; i < ds1820_devices; i++) {
            ds1820_set_resolution(&ds1820_rom_codes[i][0], MAX_ACCURACY);
            delay_ms (DS1820_ALL_DELAY); 
            // tmp_ret = ds1820_get_resolution(&ds1820_rom_codes[i][0], &resolution);
            ds1820_run_measure(&ds1820_rom_codes[i][0]); 
            delay_ms (DS1820_ALL_DELAY);
    }    }
    // Вычитываем значение каждого термометра и сразу запускаем его на измерение
    for (i = 0; i < ds1820_devices; i++) {
        int curr_t = ds1820_temperature_10(&ds1820_rom_codes[i][0]);    // Адрес термометра [i] начинается с нулевой позиции [0]
        rectification (curr_t, i, measure_mode);
        // delay_ms(10);
    }
    // Здесь есть возможный баг, связанный с тем, что в датчике температуры не прописан th и tl
}
unsigned char *ds1820_show_spd() {
    return (char *) &__ds1820_scratch_pad;
}
// Функция выбора конкретного DALLAS
unsigned char ds1820_select(unsigned char *addr) {
	unsigned char i;
	if (w1_init()==0) return 0;		// Инициализируем шину 1Wire
	if (addr) {						// Если задан адрес dallas
		w1_write(0x55);				// Выдаем адрес dallas (8 байт) на шину 1Wire
		i=0;
		do
			w1_write(*(addr++));
		while (++i<8);
	} else
		w1_write(0xcc);
	return 1;
}
// Функция чтения ScratchPAD (ОЗУ) Dallas (8 байт + CRC)
unsigned char ds1820_read_spd(unsigned char *addr) {
	unsigned char i, *p;
    unsigned char count = 0, result;
    do {
        ds1820_select(addr);                	// Выбираем конкретный Dallas
        w1_write(0xbe);							// Даем команду "Чтение памяти"
        i = 0;
        p = (char *) &__ds1820_scratch_pad;		// Устанавливаем указатель на ОЗУ текущего Dallas
        do
            *(p++)=w1_read();
        while (++i<9);
        // Сравнение контрольной суммы на 9-м байте с подсчитанным в ОЗУ. Если все в порядке, возвращаем ненулевое значение
        result = !w1_dow_crc8(&__ds1820_scratch_pad,9);
        count++;
    } while	((result == 0) && (count < 3));
    return result;
}
// Функция загоняющая ScratchPAD в ОЗУ Dallas (3 байта)
unsigned char ds1820_write_spd(unsigned char *addr) {
	register unsigned char i;
	unsigned char *p;
	if (ds1820_select(addr)==0) return 0;	// Выбираем конкретный Dallas
    w1_write(0x4e);                         // Команда на запись в ОЗУ
    i=0;
    p=(char *) &__ds1820_scratch_pad.temp_high; // Устанавливаем указатель на 2-й бат структуры Th
    do
        w1_write(*(p++));
    while (++i<3);
    return 1;
}
// Функция, дающая команду на измерение заданного Dallas
unsigned char ds1820_run_measure(unsigned char *addr) {
	if (ds1820_select(addr)==0) return 0;	// Выбираем конкретный Dallas
	w1_write(0x44);
	return w1_init();
}
// Функция, снимающая показания температуры со ScratchPAD (ОЗУ) Dallas
int ds1820_temperature(unsigned char *addr) {
    unsigned char values[16] = { 0,6,12,19,25,31,38,44,50,56,63,69,75,81,88,94 };
    unsigned char fract; 
	int t10; 
    if (!ds1820_read_spd(addr)) return NOT_FOUND;         // Если безуспешно, то вовращаем -99.98 градусов
    fract = __ds1820_scratch_pad.temp_lsb & 0xF;           // values[fract] = нашей дробной части
    t10 = __ds1820_scratch_pad.temp_msb;
	t10 = (((t10 << 8) | __ds1820_scratch_pad.temp_lsb) >> 4) * 100; 
    t10 += values[fract];
	return t10;
}
/*
int ds1820_temperature_debug(unsigned char *addr) {
    unsigned char values[16] = { 0,6,12,19,25,31,38,44,50,56,63,69,75,81,88,94 };
    unsigned char fract; 
	int t10; 
    if (!ds1820_read_spd(addr)) return NOT_FOUND;         // Если безуспешно, то вовращаем -99.98 градусов
    fract = __ds1820_scratch_pad.temp_lsb & 0xF;           // values[fract] = нашей дробной части
    t10 = __ds1820_scratch_pad.temp_msb;
	t10 = (t10 << 8) | __ds1820_scratch_pad.temp_lsb;
    #ifndef NODEBUG
    printf("{ t10 = %i[%02X]; ", t10, t10);
    #endif
    t10 = (t10 >> 4) * 100; printf("t10 = %i[%02X]; ", t10, t10);
    // t10 = t10 * 100 + values[fract];
    t10 += values[fract]; printf("%i[%02X]; }", t10, t10);
	return t10;
}
*/
// Функция, снимающая показания температуры и сразу запускает новые измерения
int ds1820_temperature_10(unsigned char *addr) {
	int t10;									// текущее преобразованное значение
    t10 = ds1820_temperature(addr);
	(void) ds1820_run_measure(addr);
	return t10;
}
// Функция, снимающая показания температуры со ScratchPAD (ОЗУ) Dallas вместе с Th и Tl
int ds1820_temperature_10lh(unsigned char *addr, signed char *temp_low, signed char *temp_high) {
	int t10;									// текущее преобразованное значение
    t10 = ds1820_temperature(addr);
    *temp_low = __ds1820_scratch_pad.temp_low;  // Вовращаем нижнюю границу Alarm
    *temp_high = __ds1820_scratch_pad.temp_high;// Возвращаем верхнюю границу Alarm
	(void) ds1820_run_measure(addr);
	return t10;
}
// Функция установки параметров ТН и ТL выбранного DALLAS
unsigned char ds1820_set_alarm(unsigned char *addr, signed char temp_low, signed char temp_high) {
	if (!ds1820_read_spd(addr)) return 0;
    __ds1820_scratch_pad.temp_low = temp_low;
    __ds1820_scratch_pad.temp_high = temp_high;
    if (ds1820_write_spd(addr)==0) return 0;
	if (!ds1820_read_spd(addr)) return 0;
	if ((__ds1820_scratch_pad.temp_low!=temp_low) || (__ds1820_scratch_pad.temp_high!=temp_high)) return 0;
    if (ds1820_select(addr)==0) return 0;
	w1_write(0x48);                             // Даем команду на запись в ПЗУ
	delay_ms(15);
	return w1_init();
}
// Функция добавлена 12.03.2010
// Функция, вычитывающая из указанного Dallas temp_high и temp_low
// Возвращает в случае успешного чтения ненулевое значение
// Функция чтения параметров ТН и ТL выбранного DALLAS
signed char ds1820_get_alarm(unsigned char *addr, signed char *temp_low, signed char *temp_high) {
    if (!ds1820_read_spd(addr)) return -98;         // Если безуспешно, то вовращаем -99.98 градусов
	// if (ds1820_read_spd(addr)==0) return 0;     // Вычитываем ScratchPAD (ОЗУ) Dallas
    *temp_low = __ds1820_scratch_pad.temp_low;  // Вовращаем нижнюю границу Alarm
    *temp_high = __ds1820_scratch_pad.temp_high;// Возвращаем верхнюю границу Alarm
    // print_scratch_pad();
    return 1;
}
// Функция установки точности выбранного DALLAS
unsigned char ds1820_set_resolution(unsigned char *addr, unsigned char resolution) {
    unsigned char temp_resolution;
    if (!ds1820_read_spd(addr)) return 0; // Вычитываем память выбранного DALLAS
    temp_resolution = (resolution << 5) | 0x1F;
    // temp_resolution = 0x7F;
    __ds1820_scratch_pad.conf = temp_resolution;
    // print_scratch_pad();
    if (ds1820_write_spd(addr)==0) return 0;
    ds1820_read_spd(addr);
    if (__ds1820_scratch_pad.conf!=temp_resolution) return 0;
    // print_scratch_pad();
	ds1820_select(addr);
	w1_write(0x48);             // Записываем в ПЗУ Dallas
	delay_ms(15);
    return w1_init();
}
// Функция чтения параметров ТН и ТL выбранного DALLAS
signed char ds1820_get_resolution(unsigned char *addr, unsigned char *resolution) {
    if (!ds1820_read_spd(addr)) return -99;         // Если безуспешно, то вовращаем -99.98 градусов
	// if (ds1820_read_spd(addr)==0) return 0;     // Вычитываем ScratchPAD (ОЗУ) Dallas
    // *resolution = __ds1820_scratch_pad.conf >> 5;  // Вовращаем resolution
    *resolution = __ds1820_scratch_pad.conf;  // Вовращаем resolution
    // print_scratch_pad();
    return 1;
}
