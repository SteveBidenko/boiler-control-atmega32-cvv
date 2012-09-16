#ifndef __ROBOWATERMENU__
#define __ROBOWATERMENU__
#include "lcd_4bit.h"
#include "alarm.h"
// Определение макроподстановок
#define NUM_MENU        7
#define NUM_PARAMETERS  5
#define NUM_SETTINGS    22
#define NUM_DT 6

#define SYNC_TO_MENU 1
#define SYNC_FROM_MENU 0

#define IS_ALERT main_menu[5].can_edit     // Если есть аварии
#define KOL_ALERT main_menu[5].val_data    // Их количество
#define TW_x parameters[0].val_data         // Измеренная температура воды на Подаче 
#define TA_x parameters[1].val_data         // Измеренная Tемпература воздуха на улице 
#define TW_CALC parameters[2].val_data      // Расчетная температура подачи График
#define SETTINGS_OPEN main_menu[6].can_edit
// Описание типов
typedef char lcd_str[LCD_DISP_LENGTH];        // массив из 16 символов для строки LCD
// Прототип перечисления
enum en_type {
    e_empty = 0,              // Пустой тип. Ничего не добавляем для вывода
    e_clatsman,               // Переключатель (bit) ВХОД./ТЕСТ.
    e_percent,                // Процент (0..1000 = 0,0%..100,0%)
    e_temperature,            // Положительное или отрицательное значение температуры умноженное на 100 (int)
    e_room,                   // Задаваемая + температура в помещении умноженное на 100 (int) шагом 0.1
    e_world,                  // Задаваемое мин. значение температуры на улице с шагом 1 градус
    e_water,                  // Завадваемое значение температуры воды с шагом 1 градус.
    e_coef,                   // Положительные значения от 00.0 до 25.5
    e_scale,                  // Множитель шкалы (-126..127)
    e_stime,                  // Время в секундах
    e_mtime,                  // Время в формате мм:сс
    e_minute,                 // Время в минутах [00..59]
    e_hour,                   // Время в часах [00..23]
    e_day,                    // Дни [01..31]
    e_month,                  // Месяцы [01..12]
    e_year,                   // Года [00..99]
    e_weekday,                // День недели [0..6], 0=Воскресенье, 1=Понедельник, ...
    e_dt,                     // Мнемоника Вр./Дат.
    e_time,                   // Время в формате чч:мм:сс
    e_date,                   // Время в формате dd.mm.yy
    e_sync,                   // Признак установки даты-времени в микросхеме часов
    e_sysok,                  // Признак отсутствия аварий
    e_alarm,                  // Конкретная тревога
    e_password,               // Числовой пароль -32766..32767
    e_PWM,                    // ШИМ  Только положительное значение (0..255)
    e_ADC                     // АЦП  Только положительное значение (0..255)
     
};
// Определение структур
// Прототип одного параметра системы. Нужен для организации массива параметров системы
    // lcd_str *val_name;                   // строковое название параметра для LCD
struct st_parameter {
    // byte id;                         // порядковый номер в системе для ускорения поиска параметров
    signed int val_data;                // само значение параметра (-32768..32767
    enum en_type val_type;              // тип значения параметра
    unsigned char can_edit;             // Признак того, что val_data можно редактировать
                                        // 0 - результат измерений/подменю отсутствует
                                        // 1 - редактирование разрешено/ есть подменю
    unsigned char str_num;              // Порядковый номер строки меню в функции getmenustr
};
struct st_array_pos {
    signed char line0;          // Текущая позиция в массиве (меню)
    signed char line1;          // Следующая позиция в массиве (меню)
    unsigned char lcd;          // Текущая активная строка дисплея (0 - 1 строка, 1 - 2 строка)
    struct st_parameter *menu;  // Указатель на нулевой элемент текущего меню
    unsigned char maximum;      // Максимальное кол-во элементов в меню
    // Для редактирования параметров необходимо завести еще пару элементов структуры
    struct st_parameter *par;   // Указатель на текущий параметр
    int val_data;               // редактируемое значение параметра
    unsigned char level;        // текущий уровень меню (0 - главное меню, 1 - второстепенное меню)
};
// Описание внешних переменных
// extern lcd_str linestr;             // Строка для LCD
extern char linestr[];                // Строка для LCD
extern int menu_value;                // текущее значение изменяемого в меню параметра
// extern signed char curr_menu_level1, next_menu_level1;  // Текущий и следующий пункт меню
extern struct st_parameter main_menu[NUM_MENU];
extern struct st_parameter parameters[NUM_PARAMETERS];
extern struct st_parameter alarms[MAX_ALARMS];
extern struct st_array_pos curr_menu;                // Позиции в меню
// Описание внешних функций
extern void sync_set_par(unsigned char);
extern void init_curr_menu(struct st_parameter *lmenu, unsigned char lmax);
extern void lcd_init_edit(void);
extern void lcd_save_edit(void);
extern void lcd_esc_edit(void);
extern void lcd_menu(signed char);
extern void lcd_edit(signed char);
// extern void print_curr_menu2(signed char);
// extern void print_curr_menu(void);
extern char *param_str(unsigned char, struct st_parameter parameter[]);
extern char *par_str(struct st_parameter *, unsigned char, int);
extern char *getmenustr(unsigned char menu_num_pp);
extern int calc_percent(unsigned char, unsigned char, unsigned char);
// extern unsigned char next_menu(signed char *menu_level, signed char menu_choice);
#endif
