#ifndef __at2404__
#define __at2404__

#define M24C02_0    0xA0    // Bank memory 0
#define M24C02_1    0xA2    // Bank memory 1
#define I2C_READ    1
#define I2C_WRITE   0

#pragma used+
char eeprom_write(unsigned char eeprom_adresse, unsigned char eeprom_input, unsigned char bank);
char eeprom_read(unsigned char eeprom_adresse, unsigned char bank);
// �������, ���������� � EEPROM ��������� �������� size
void eeprom_write_struct (unsigned char *struct_data, unsigned char size);
// �������, ���������� �� EEPROM � ��������� �������� size
void eeprom_read_struct (unsigned char *struct_data, unsigned char size);
// �������, ���������� � EEPROM ������� ALARM-��������� �������� � size
void eeprom_write_alert (unsigned char *struct_data, unsigned char size, unsigned char num_alarm);
// �������, ���������� �� EEPROM � ��������� ALARM �������� size
void eeprom_read_alert (unsigned char *struct_data, unsigned char size, unsigned char num_alarm);
#pragma used-

#endif
