#ifndef __ALARMS__
#define __ALARMS__

// ALARM - тревога, обозначает неисправность или проблему конкретного оборудования
// ALERT - зарегистрированное событие, строка в журнале
#define MAX_ALERTS 12       // сколько может поместиться в журнале событий в EEPROM
#define DT_LENGTH 5         // Сколько байт занимает отпечаток даты/времени
#define ALARM_LENGTH 16     // Максимальная длина строки ALARM
#define MAX_ALARMS 5        // Необходимо синхронизировать вручную с robowater.h #define ALL_ALARMS

typedef char alarm_text[ALARM_LENGTH];
// Описание структуры ALARM
struct st_alarm {
    char datestamp[DT_LENGTH];  // Отпечаток даты/времени ALARM
    int val_preset;             // Заданное значение
    int val_alarm;              // Значение ALARM
    alarm_text text;            // Текстовая расшифровка ALARM
};
// Описание внешних функций
extern void alarm_reg (int reg_preset, int reg_alarm, char *text_alarm, unsigned char);
extern signed char alarm_unreg (unsigned char);
extern void alarm_all_print (void);
extern char *get_alarm_str(unsigned char);
extern void update_alarm_menu (void);

#endif
