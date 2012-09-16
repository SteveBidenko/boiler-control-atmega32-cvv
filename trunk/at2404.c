#include <stdio.h>
#include <i2c.h>
#include <delay.h>
#include "at2404.h"

#define OFFSET_FOR_SETTING 0x0
#define OFFSET_FOR_ALARMS 0x0
#define NODEBUG

char eeprom_write(unsigned char eeprom_adresse, unsigned char eeprom_input, unsigned char bank)
{
    i2c_start();
    if (bank)
        i2c_write(M24C02_1+I2C_WRITE);
    else
        i2c_write(M24C02_0+I2C_WRITE);
	i2c_write(eeprom_adresse);
    i2c_write(eeprom_input);
    i2c_stop();
    delay_ms(10);
	return 1;
}

char eeprom_read(unsigned char eeprom_adresse, unsigned char bank)
{
	unsigned char data;
    i2c_start();
    if (bank)
        i2c_write(M24C02_1+I2C_WRITE);
    else
        i2c_write(M24C02_0+I2C_WRITE);
   	i2c_write(eeprom_adresse);
    i2c_start();
    if (bank)
        i2c_write(M24C02_1+I2C_READ);
    else
        i2c_write(M24C02_0+I2C_READ);
    data = i2c_read(0);
    i2c_stop();
	return data;
}
// Функция, загоняющая в EEPROM структуру размером size
void eeprom_write_struct (unsigned char *struct_data, unsigned char size)
{
    register unsigned char i;
    unsigned char s_byte;
    printf("Записываем SETTINGS в EEPROM [0x%x]: ", size);
    #ifdef NODEBUG
    printf("\n");
    #endif
    for (i=0; i<size; i++) {
        s_byte = *struct_data;
        eeprom_write(i + OFFSET_FOR_SETTING, s_byte, 0);
        #ifndef NODEBUG
        printf(" %d", s_byte);
        #endif
        struct_data++;
    }
    #ifndef NODEBUG
    printf("\n");
    #endif
}
// Функция, загоняющая в EEPROM ALARM-структуру размером в size
void eeprom_write_alert (unsigned char *struct_data, unsigned char size, unsigned char num_alarm)
{
    register unsigned char i;
    unsigned char s_byte;
    unsigned char eeprom_offset;
    eeprom_offset = OFFSET_FOR_ALARMS + size * (num_alarm - 1);
    printf("Записываем ALARM #%u в EEPROM [0x%x] по адресу 0x%x:\n", num_alarm, size, eeprom_offset);
    for (i=0; i<size; i++) {
        s_byte = *struct_data;
        eeprom_write(i + eeprom_offset, s_byte, 1); // Записываем во второй банк памяти
        #ifndef NODEBUG
        printf(" %02X", s_byte);
        #endif
        struct_data++;
    }
    #ifndef NODEBUG
    printf("\n");
    #endif
}
// Функция, загоняющая из EEPROM в структуру размером size
void eeprom_read_struct (unsigned char *struct_data, unsigned char size)
{
    register unsigned char i;
    unsigned char s_byte;
    printf("Считываем из EEPROM [0x%x]: ", size);
    #ifdef NODEBUG
    printf("\n");
    #endif
    for (i=0; i<size; i++) {
        s_byte = eeprom_read(i + OFFSET_FOR_SETTING, 0);
        *struct_data = s_byte;
        #ifndef NODEBUG
        printf(" %d", s_byte);
        #endif
        struct_data++;
    }
    #ifndef NODEBUG
    printf("\n");
    #endif
}
// Функция, загоняющая из EEPROM в структуру ALARM размером size
void eeprom_read_alert (unsigned char *struct_data, unsigned char size, unsigned char num_alarm)
{
    register unsigned char i;
    unsigned char s_byte;
    unsigned char eeprom_offset;
    eeprom_offset = OFFSET_FOR_ALARMS + (num_alarm - 1) * size;
    #ifndef NODEBUG
    printf("Считываем ALARM #%u из EEPROM [%d bytes] по адресу 0x%x:\n", num_alarm, size, eeprom_offset);
    #endif
    for (i=0; i<size; i++) {
        s_byte = eeprom_read(i + eeprom_offset, 1);
        *struct_data = s_byte;
        #ifndef NODEBUG
        printf(" %02X", s_byte);
        #endif
        struct_data++;
    }
    #ifndef NODEBUG
    printf("\n");
    #endif
}
