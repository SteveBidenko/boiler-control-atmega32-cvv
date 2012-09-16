#include <mega32.h>
#include <stdio.h>
#include "robowater.h"
#include "signals.h"
// Лапы сигналов
#define BUZ PORTD.6     // Порт звука
#define RED PORTA.3     // Порт красной лампочки
#define GREEN PORTB.4   // Порт зеленой лампочки
#define WHITE PORTA.2   // Порт белой лампочки
// Описание модульных переменных
struct st_signal signals;      // Текущее состояние всех сигналов
// Функция выводящая состояние всей структуры сигналов
void signal_printallbytes (void) {
    register unsigned char i;
    unsigned char *status = &signals.buz_mode;
    printf ("Стуктура signals =");
    for (i = 0; i < sizeof(signals); i++) {
        printf (" %x", *status);
        status++;
    }
    printf ("\n");
}
// Функция инициализация статуса любого светика
unsigned char init_lamp_status (unsigned char *lamp_mode) {
    unsigned char *lamp_timer = lamp_mode + 1;
    unsigned char *lamp_status = lamp_mode + 2;
    switch (*lamp_mode) {
        case SHORT:
            *lamp_timer = SHORT_MAX;
            *lamp_status = ON;
            break;
        case LONG:
            *lamp_timer = LONG_MAX;
            *lamp_status = ON;
            break;
        case MEANDR:
            *lamp_timer = MEANDR_MAX;
            *lamp_status = ON;
            break;
        case ON:
            *lamp_status = ON;
            break;
        default:
            *lamp_status = OFF;
            break;
    };
    return (*lamp_status);
}
// Функция обновления статуса любого светика
unsigned char update_lamp_status (unsigned char *lamp_mode) {
    unsigned char *lamp_timer = lamp_mode + 1;
    unsigned char *lamp_status = lamp_mode + 2;
    switch (*lamp_mode) {
        case SHORT:
            if (*lamp_timer) {
                *lamp_timer = *lamp_timer - 1;
                if (*lamp_timer == 0) {
                    if (*lamp_status) {
                        *lamp_status = OFF;
                        *lamp_timer = LONG_MAX;
                    } else {
                        *lamp_status = ON;
                        *lamp_timer = SHORT_MAX;
                    }
                }
            }
            break;
        case LONG:
            if (*lamp_timer) {
                *lamp_timer = *lamp_timer - 1;
                if (*lamp_timer == 0) {
                    if (*lamp_status) {
                        *lamp_status = OFF;
                        *lamp_timer = SHORT_MAX;
                    } else {
                        *lamp_status = ON;
                        *lamp_timer = LONG_MAX;
                    }
                }
            }
            break;
        case MEANDR:
            if (*lamp_timer) {
                *lamp_timer = *lamp_timer - 1;
                if (*lamp_timer == 0) {
                    *lamp_status = !(*lamp_status);
                    *lamp_timer = MEANDR_MAX;
                }
            }
            break;
        case ON:
            *lamp_status = ON;
            break;
        case OFF:
        default:
            *lamp_status = OFF;
            *lamp_mode = OFF;
            break;
    };
    return (*lamp_status);
}
// Функция обновления статуса
// Вызывается только из таймера. Необходим равномерно распределенный по времени вызов.
void update_signal_status(void) {
    switch (signals.buz_mode) {
        case SHORT:
        case LONG:
            signals.buz_status = OFF;
            if (signals.buz_timer) {
                signals.buz_timer--;
                if (signals.buz_timer) signals.buz_status = ON; else signals.buz_mode = OFF;
            }
            break;
        case MEANDR:
            if (signals.buz_timer) {
                signals.buz_timer--;
                if (signals.buz_timer == 0) {
                    signals.buz_status = !signals.buz_status;
                    signals.buz_timer = MEANDR_MAX;
                }
            }
            break;
        default:
            signals.buz_status = OFF;
            signals.buz_mode = OFF;
            break;
    }
    if (mode.sound) BUZ = signals.buz_status;
    RED = update_lamp_status(&signals.red_mode);
    GREEN = update_lamp_status(&signals.green_mode);
    WHITE = update_lamp_status(&signals.white_mode);
    // signal_printallbytes();
}
// Функция смены режима звукового оповещения
void signal_buz(unsigned char signal_mode) {
    signals.buz_mode = signal_mode;
    switch (signal_mode) {
        case SHORT:
            signals.buz_timer = SHORT_MAX;
            signals.buz_status = ON;
            break;
        case LONG:
            signals.buz_status = ON;
            signals.buz_timer = LONG_MAX;
            break;
        case MEANDR:
            signals.buz_status = ON;
            signals.buz_timer = MEANDR_MAX;
            break;
        default:
            signals.buz_status = OFF;
            signals.buz_timer = OFF;
            break;
    };
    if (mode.sound) BUZ = signals.buz_status;
}
// Функция смены режима светового оповещения красной лампочки
void signal_red(unsigned char signal_mode) {
    signals.red_mode = signal_mode;
    RED = init_lamp_status(&signals.red_mode);
}
// Функция смены режима светового оповещения зеленой лампочки
void signal_green(unsigned char signal_mode) {
    signals.green_mode = signal_mode;
    GREEN = init_lamp_status(&signals.green_mode);
}
// Функция смены режима светового оповещения белой лампочки
void signal_white(unsigned char signal_mode) {
    signals.white_mode = signal_mode;
    WHITE = init_lamp_status(&signals.white_mode);
}
