#ifndef __SIGNALS__
#define __SIGNALS__

// Константы длительностей и периодов
#define SHORT_MAX 1
#define LONG_MAX 5
#define MEANDR_MAX 3
// Режимы
#define OFF 0
#define ON 1
#define SHORT 2
#define LONG 3
#define MEANDR 4
// Описание структуры состояния всех сигналов
struct st_signal {
    unsigned char buz_mode;      // Режим работы звукового оповещения: 0 - нет звука, 1 - Короткий бип, 2 - Длинный бип, 3 - длинный меандр
    unsigned char buz_timer;
    unsigned char buz_status;
    // Режим работы светового оповещения: 0 - выкл., 1 - короткое моргание, 2 - длинное моргание, 3 - равномерное моргание, 4 - вкл. постоянно
    unsigned char red_mode;
    unsigned char red_timer;
    unsigned char red_status;
    unsigned char green_mode;
    unsigned char green_timer;
    unsigned char green_status;
    unsigned char white_mode;
    unsigned char white_timer;
    unsigned char white_status;
};
// Описание внешних функций
extern void update_signal_status(void);         // Вызывается только из таймера. Необходим равномерно распределенный по времени вызов.
// extern void init_signal(void);                  // Инициализационная часть всех сигналов ???
extern void signal_buz(unsigned char);
extern void signal_green(unsigned char);
extern void signal_red(unsigned char);
extern void signal_white(unsigned char);
extern void signal_printallbytes (void);

#endif