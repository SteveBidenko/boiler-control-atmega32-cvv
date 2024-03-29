#include <stdio.h>
#include <stdlib.h>
#include <delay.h>
#include <1wire.h>
#include "robowater.h"
#include "spd1820.h"
// #define NODEBUG             // ����������� ��� ������, ���� ����������� ����������� �������� ���������� ������
/* #ifndef NODEBUG
// ������� ���������� �������� __ds1820_scratch_pad
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
struct st_terms termometers[MAX_DS1820];  // ������ �������� ����������� � �� ����������������� �����������
// ��������� ��� �������� �������� ��� Dallas
struct __ds1820_scratch_pad_struct __ds1820_scratch_pad;
// ������� ������ ������� ���������� �� ds1820_rom_codes � ��������� EEPROM, � �������������� TH � TL
unsigned char sync_ds1820_eeprom(void) {
    unsigned char result = 0, i, j;
    signed char th, tl, get_alarm;

    for (i = 0; i < ds1820_devices; i++) {
        get_alarm = ds1820_get_alarm(&ds1820_rom_codes[i][0], &tl, &th);                        
        if (get_alarm > 0) {    
            // ����� ���������� [i] ���������� � ������� ������� [0]
            // ���� ��������� ����� ����������� ���������, �� ����������� �������� �� ��������. tl
            // ����� ����������� � ������� ������� (����������� � ���������)
            for (j = 0; j < 9; j++)
                // ���� ��������� ����� "�����" ���������, �� ������������ ��� � ������ ufo
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
// ������� ������ ������� ���������� �� EEPROM � ds1820_rom_codes �� �������
void sync_eeprom_ds1820(void) {
    register byte i, j;
    // ���������� ��� ������ �� ��������� EEPROM � ������ �����������
    for (i = 0; i < ds1820_devices; i++)
        for (j = 0; j < 9; j++)
            ds1820_rom_codes[i][j] = prim_par.addr[i][j];
}
// ������� �������� �������� � ��������� � ���������� ����������� � ��������� �� ������������
void rectification(int unconverted, unsigned char index, unsigned char measure_mode) {
    // float lt;       // �������� ��� ��������� �����������
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
        // ��������� ������� ����� ���������� ���������� � �������
        if (abs(termometers[index].t - termometers[index].t_last) < DELTA_MAX) {
            termometers[index].t_last = termometers[index].t;
            termometers[index].err = 0;
        } else {
            termometers[index].t = termometers[index].t_last;
            termometers[index].err++;
        }
    }
}
// ������ ���� Dallas � ����������
void read_all_terms(unsigned char measure_mode) {
    register byte i;
    // ������ �� EEPROM ������� ������� ����������� � ������ �������������
    if (measure_mode == INIT_MODE) {
        sync_eeprom_ds1820();
        for(i = 0; i < ds1820_devices; i++) {
            ds1820_set_resolution(&ds1820_rom_codes[i][0], MAX_ACCURACY);
            delay_ms (DS1820_ALL_DELAY); 
            // tmp_ret = ds1820_get_resolution(&ds1820_rom_codes[i][0], &resolution);
            ds1820_run_measure(&ds1820_rom_codes[i][0]); 
            delay_ms (DS1820_ALL_DELAY);
    }    }
    // ���������� �������� ������� ���������� � ����� ��������� ��� �� ���������
    for (i = 0; i < ds1820_devices; i++) {
        int curr_t = ds1820_temperature_10(&ds1820_rom_codes[i][0]);    // ����� ���������� [i] ���������� � ������� ������� [0]
        rectification (curr_t, i, measure_mode);
        // delay_ms(10);
    }
    // ����� ���� ��������� ���, ��������� � ���, ��� � ������� ����������� �� �������� th � tl
}
unsigned char *ds1820_show_spd() {
    return (char *) &__ds1820_scratch_pad;
}
// ������� ������ ����������� DALLAS
unsigned char ds1820_select(unsigned char *addr) {
	unsigned char i;
	if (w1_init()==0) return 0;		// �������������� ���� 1Wire
	if (addr) {						// ���� ����� ����� dallas
		w1_write(0x55);				// ������ ����� dallas (8 ����) �� ���� 1Wire
		i=0;
		do
			w1_write(*(addr++));
		while (++i<8);
	} else
		w1_write(0xcc);
	return 1;
}
// ������� ������ ScratchPAD (���) Dallas (8 ���� + CRC)
unsigned char ds1820_read_spd(unsigned char *addr) {
	unsigned char i, *p;
    unsigned char count = 0, result;
    do {
        ds1820_select(addr);                	// �������� ���������� Dallas
        w1_write(0xbe);							// ���� ������� "������ ������"
        i = 0;
        p = (char *) &__ds1820_scratch_pad;		// ������������� ��������� �� ��� �������� Dallas
        do
            *(p++)=w1_read();
        while (++i<9);
        // ��������� ����������� ����� �� 9-� ����� � ������������ � ���. ���� ��� � �������, ���������� ��������� ��������
        result = !w1_dow_crc8(&__ds1820_scratch_pad,9);
        count++;
    } while	((result == 0) && (count < 3));
    return result;
}
// ������� ���������� ScratchPAD � ��� Dallas (3 �����)
unsigned char ds1820_write_spd(unsigned char *addr) {
	register unsigned char i;
	unsigned char *p;
	if (ds1820_select(addr)==0) return 0;	// �������� ���������� Dallas
    w1_write(0x4e);                         // ������� �� ������ � ���
    i=0;
    p=(char *) &__ds1820_scratch_pad.temp_high; // ������������� ��������� �� 2-� ��� ��������� Th
    do
        w1_write(*(p++));
    while (++i<3);
    return 1;
}
// �������, ������ ������� �� ��������� ��������� Dallas
unsigned char ds1820_run_measure(unsigned char *addr) {
	if (ds1820_select(addr)==0) return 0;	// �������� ���������� Dallas
	w1_write(0x44);
	return w1_init();
}
// �������, ��������� ��������� ����������� �� ScratchPAD (���) Dallas
int ds1820_temperature(unsigned char *addr) {
    unsigned char values[16] = { 0,6,12,19,25,31,38,44,50,56,63,69,75,81,88,94 };
    unsigned char fract; 
	int t10; 
    if (!ds1820_read_spd(addr)) return NOT_FOUND;         // ���� ����������, �� ��������� -99.98 ��������
    fract = __ds1820_scratch_pad.temp_lsb & 0xF;           // values[fract] = ����� ������� �����
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
    if (!ds1820_read_spd(addr)) return NOT_FOUND;         // ���� ����������, �� ��������� -99.98 ��������
    fract = __ds1820_scratch_pad.temp_lsb & 0xF;           // values[fract] = ����� ������� �����
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
// �������, ��������� ��������� ����������� � ����� ��������� ����� ���������
int ds1820_temperature_10(unsigned char *addr) {
	int t10;									// ������� ��������������� ��������
    t10 = ds1820_temperature(addr);
	(void) ds1820_run_measure(addr);
	return t10;
}
// �������, ��������� ��������� ����������� �� ScratchPAD (���) Dallas ������ � Th � Tl
int ds1820_temperature_10lh(unsigned char *addr, signed char *temp_low, signed char *temp_high) {
	int t10;									// ������� ��������������� ��������
    t10 = ds1820_temperature(addr);
    *temp_low = __ds1820_scratch_pad.temp_low;  // ��������� ������ ������� Alarm
    *temp_high = __ds1820_scratch_pad.temp_high;// ���������� ������� ������� Alarm
	(void) ds1820_run_measure(addr);
	return t10;
}
// ������� ��������� ���������� �� � �L ���������� DALLAS
unsigned char ds1820_set_alarm(unsigned char *addr, signed char temp_low, signed char temp_high) {
	if (!ds1820_read_spd(addr)) return 0;
    __ds1820_scratch_pad.temp_low = temp_low;
    __ds1820_scratch_pad.temp_high = temp_high;
    if (ds1820_write_spd(addr)==0) return 0;
	if (!ds1820_read_spd(addr)) return 0;
	if ((__ds1820_scratch_pad.temp_low!=temp_low) || (__ds1820_scratch_pad.temp_high!=temp_high)) return 0;
    if (ds1820_select(addr)==0) return 0;
	w1_write(0x48);                             // ���� ������� �� ������ � ���
	delay_ms(15);
	return w1_init();
}
// ������� ��������� 12.03.2010
// �������, ������������ �� ���������� Dallas temp_high � temp_low
// ���������� � ������ ��������� ������ ��������� ��������
// ������� ������ ���������� �� � �L ���������� DALLAS
signed char ds1820_get_alarm(unsigned char *addr, signed char *temp_low, signed char *temp_high) {
    if (!ds1820_read_spd(addr)) return -98;         // ���� ����������, �� ��������� -99.98 ��������
	// if (ds1820_read_spd(addr)==0) return 0;     // ���������� ScratchPAD (���) Dallas
    *temp_low = __ds1820_scratch_pad.temp_low;  // ��������� ������ ������� Alarm
    *temp_high = __ds1820_scratch_pad.temp_high;// ���������� ������� ������� Alarm
    // print_scratch_pad();
    return 1;
}
// ������� ��������� �������� ���������� DALLAS
unsigned char ds1820_set_resolution(unsigned char *addr, unsigned char resolution) {
    unsigned char temp_resolution;
    if (!ds1820_read_spd(addr)) return 0; // ���������� ������ ���������� DALLAS
    temp_resolution = (resolution << 5) | 0x1F;
    // temp_resolution = 0x7F;
    __ds1820_scratch_pad.conf = temp_resolution;
    // print_scratch_pad();
    if (ds1820_write_spd(addr)==0) return 0;
    ds1820_read_spd(addr);
    if (__ds1820_scratch_pad.conf!=temp_resolution) return 0;
    // print_scratch_pad();
	ds1820_select(addr);
	w1_write(0x48);             // ���������� � ��� Dallas
	delay_ms(15);
    return w1_init();
}
// ������� ������ ���������� �� � �L ���������� DALLAS
signed char ds1820_get_resolution(unsigned char *addr, unsigned char *resolution) {
    if (!ds1820_read_spd(addr)) return -99;         // ���� ����������, �� ��������� -99.98 ��������
	// if (ds1820_read_spd(addr)==0) return 0;     // ���������� ScratchPAD (���) Dallas
    // *resolution = __ds1820_scratch_pad.conf >> 5;  // ��������� resolution
    *resolution = __ds1820_scratch_pad.conf;  // ��������� resolution
    // print_scratch_pad();
    return 1;
}
