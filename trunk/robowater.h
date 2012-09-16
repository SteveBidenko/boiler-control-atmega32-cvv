#ifndef __ROBOWATER__
#define __ROBOWATER__
#define F_CPU 3686400UL
#define BAUDRATE 115200UL
// Описание битов регистра "А" статуса и управления
#define RXC     7            // Флаг завершения приема USART
#define TXC     6            // Флаг завершения передачи USART
#define UDRE    5            // Флаг "Регистр данных USART пуст"
#define FE      4            // Флаг ошибки кадрирования
#define OVR     3            // Флаг переполнения
#define UPE     2            // Флаг ошибки контроля четности USART
#define MPCM    0            // Режим мультипроцессорного обмена
// Описание битов регистра "B" статуса и управления
#define RXCIE   7            // Разрешение прерывания по завершению приема
#define TXCIE   6            // Разрешение прерывания по завершению передачи
#define RXEN    4            // Разрешение приема
#define TXEN    3            // Разрешение передачи
#define UCSZ2   2            // Установка 9-ти битного режима
#define RXB8    1            // Разряд номер восемь приемного буфера
#define TXB8    0            // Разряд номер восемь буфера передачи.
// Описание статусов последовательных портов
#define FRAMING_ERROR	(1<<FE)
#define PARITY_ERROR    (1<<UPE)
#define DATA_OVERRUN    (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define TX_COMPLETE 	(1<<UDRE)
#define RX_COMPLETE 	(1<<RXC)
// maximum number of DS1820 devices connected to the 1 Wire bus
#define MAX_DS1820 2
#define FORCE_INIT 1           // Значение принудительной инициализации
#define ALL_ALARMS 5           // количество возможных тревог в системе, необходимо синхронизировать с alrarm.h
#define TIMER_INACTIVE 60      // Задается в кол-ве полсекундных интервалов
#define ENTER_CANCEL_OVERFLOW 2      // Задается в кол-ве полсекундных интервалов
//#define TIME_START 300                // Время старта Задается в кол-ве полсекундных интервалов
#define TIME_STOP 10                 // Время остановки Задается в кол-ве полсекундных интервалов
#define TIME_COOL_STOP 20            // Время таймаута для регулировки температуры калорифера в режиме СТОП
#define TIME_BUZ1 1                  // Время Звука 1
#define TIME_BUZ2 3                  // Время Звука 2
#define STRLENGTH 16                // Длина строки параметра
#define COUNT_FAN_MAX 5             // Максимальное количество осчетов для изменения оборотов вентилятора 
#define T_z 255                     // Время открытия - закрытия крана
#define RELE PORTD.4               // Порт.Пин вентилятора
#define POMP PORTD.5                // Порт.Пин насоса
#define TA_IN_NOLIMIT 500L      // Окончание ограничения температуры воздуха на входе (500 = +5 С)
#define TAP_ANGLE_LIMIT 78.0    // Ограничение закрытия трехходового крана при температуре воздуха TA_in_Min (ШИМ для 20% = 51: для 30% = 78)) 
#define MAX_ACCURACY 0b11       // Точность для термометров в бинарном коде: 0b11 = 0.0625, 0b10 = 0.125, 0b01 = 0.25, 0b00 = 0.5 
#define MAX_OFFLINES 5          // Количество сбоев термометров максимальное подряд
#define CHECK_EVENT (event == ev_none)
#define ds1820_devices prim_par.terms
#define TAP_ANGLE prim_par.tap_angle    // Расчетное состояние крана (PWM)
#define ADC_VAR1 prim_par.ADC1          // Текущее состояние крана (АЦП) 
#define TA_MAX prim_par.ta_max
#define TA_MIN prim_par.ta_min
#define TW_MAX prim_par.tw_max
#define TW_MIN prim_par.tw_min

// Описание типов переменных
typedef unsigned char 	byte;	// byte = unsigned char
typedef unsigned int 	word;	// word = unsigned int
typedef char str_val[STRLENGTH];
// Описание перечисления событий, возникающих в системе
enum en_event {
    ev_none = 0,            // [0]  событие отсутствует
    ev_left = 1,            // [1]  используется valcoder -
    ev_enter = 2,           // [2]  полностью нажат/отжат enter
    ev_right = 3,           // [3]  используется valcoder +
    ev_cancel,              // [4]  полностью нажат/отжат cancel
    ev_secunda,             // [5]  ежесекундное событие
    ev_timer,               // [6]  сработал таймер
    ev_start,               // [7]  Ручной ПУСК ТО Крана
    ev_stop,                // [8]  Ручной СТОП ТО Крана 
    ev_tobegin,             // [9]  Начало проведения ТО крана по расписанию
    ev_toend,               // [10] Окончание проведения ТО крана по расписанию и в ручную
    ev_alarm1,              // [11]  События внешних тревог  Авария  остановки котла 
    ev_alarm2,              // [12] События внешних тревог  Остановка котла для чистки  
    ev_term1_nf,            // [13] Термометр 1 не найден
    ev_term2_nf,            // [14] Термометр 2 не найден
    ev_to_nf,               // [15] Невозможно провести ТО Крана
    ev_term1_warning,       // [16] Предупреждение термометр 1 улица
    ev_term2_warning,       // [17] Предупреждение термометр 2 Подача
    ev_tonormal             // [18] Возврат в нормальный режим после ev_alarm1 или ev_alarm2 
};
// Описание режимов работы системы
enum en_mode {
    mo_reg = 0,             //   0 - Регулирование, 
    mo_to = 1,              //   1 - ТО, 
    mo_setup_input = 2,     //   2 - настройка выхода, 
    mo_setup_output = 3,    //   3 - настройка входа, 
    mo_control_line1,       //   4 - контроль входа1
    mo_control_line2        //   5 - контроль входа2
};
// Описание cтруктур
struct st_datetime {
    byte cHH, cMM, cSS;    // Текущее время
    byte cyy, cmo, cdd;    // Текущая дата
    byte dayofweek;        // Текущий день недели (0..6), 0 - воскресенье, 1 - понедельник ...
};
// Структура проведения ТО
struct st_TO {
    byte weekday, hour, minute;
    byte status;                // Если ненулевое значение, то ТО сейчас проводится
};
// Структура использующая для коррекции температуры t = Т(реальная) * 100
struct st_eliminate {
    signed int shift;       // Смещение базовой точки (0 - нет коррекции) [2]
    signed char scale;      // Множитель (-127..0..126), который потом вычисляется по формуле 1+(scale/127) [1]
};
// typedef st_eliminates t_eliminates[MAX_DS1820];
// Структура основных переменных в системе
extern struct st_eeprom_par {
    struct st_eliminate elims[MAX_DS1820]; // [3] * 4 = [12]
    byte tap_angle, ADC1;                   // 0x7F, 205,  [2]
    int Ku, Ki, Kd;                         // 10, 0, 0 [6]
    int set_delta, T_int;                   // 100 set_delta-зона нечуствительности=300 (3 градуса), T_int- Время интегрирования=100 секунд
    int tw_min, tw_max;                     // 6000, 9500, 
    int ta_min, ta_max;                     //-1500, 1500,
    byte alarm_status[ALL_ALARMS];          // 0) Тревога (0 - нет тревоги, > 0 - количество подсчитанных тревог) [12]
    byte function_keys1, function_keys2;    // Функция кнопок KEY_ALARM1 и KEY_ALARM2 ВХОД(1) ТЕСТ(0)      [1]
    byte PWM_lo, PWM_hi, ADC_lo, ADC_hi;    // Установка границ вольтажа входа и выхода
    byte alert;           // Позиция текущего alert в EEPROM [1]
    byte c_alerts;         // Сколько всего зарегистрировано alerts в EEPROM [1]
    byte terms;           // Сколько должно быть термометров в системе
    byte addr[MAX_DS1820][9];  // [36] Найденные терм. (в адресе - 9 байт). Порядковый номер 0 - Помещение, 1 - Улица, 2 - вода вх., 3 - вода выход
    struct st_TO TO; 
} prim_par;

// Структура основных переключателей в системе
extern struct st_mode {
    byte menu;           // 0) Режим меню (00 - главный экран, 01 - меню, 10 - редактирование, 11 - сохраннение/восстановление параметра
    enum en_mode run;    // 1) Текущий режим работы 
    byte stop_sync_dt;    // 2) Флаг запрета синхронизации RTC и глобальной структуры. Если установлен - не синхронизируем
    byte sound;           // 3) Флаг запрета звука (0 = звук выключен, 1 = звук включен
    byte ufo[9];          // 4) Неопознанный термометр (-ы)
    byte print;           // 5) Печать показаний системы регулирования
} mode ;
extern enum en_event event;        // Текущее событие в системе
// Описание функций
extern void init(void);
extern int read_term(byte);
extern void set_cur_dt (void);
extern void get_cur_dt (unsigned char);
// Описание глобальных переменных
extern struct st_datetime s_dt;
// extern struct st_menupos menupos;
extern struct st_mode mode;
extern unsigned int time_integration;
extern byte timer1_valcoder;
extern unsigned int timer_start;
extern byte timer_stop;
#endif
