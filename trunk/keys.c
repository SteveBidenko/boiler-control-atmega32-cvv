#include <mega32.h>
#include <stdio.h>
#include "keys.h"
#include "robowater.h"
#include "valcoder.h"
#include "bits.h"
#include "signals.h"
#define NODEBUG
// Состояние всех клавиш
// Структура статуса кнопки. Устраняется дребезг и помехи.
struct st_key {
    signed char status;     // текущий статус кнопки (0 - off, 1 - on, -1 - идет выбор)
    unsigned char last;     // последний статус копки
    unsigned char is_work;  // Признак работоспособности кнопки (0 - не работает)
    unsigned char range;  // Признак выбора 0-выбрано 1- нет
    unsigned int on;        // кол-во 1 (см. мажоритарный клапан)
    unsigned int off;       // кол-во 0 (см. мажоритарный клапан)
    unsigned char reg;      // указатель на регистр кнопки
    unsigned char pin;      // нога регистра кнопки
} keys[NUM_KEYS]= {
    //{-1, 0, 0, 0, 0, 0, 0x33, 3},     // [0] KEY_STOP   PINC.3
    {-1, 0, 0, 0, 0, 0, 0x36, 1},     // [1] ! 9.08.12 Замена кнопок STOP PINC.3 на FILTER PINB.1 
    {-1, 0, 0, 0, 0, 0, 0x33, 5},     // [1] KEY_START  PINC.5
    {-1, 0, 0, 0, 0, 0, 0x33, 6},     // [2] KEY_ENTER  PINC.6
    {-1, 0, 0, 0, 0, 0, 0x33, 7},     // [3] KEY_CANCEL PINC.7
    {-1, 0, 0, 0, 0, 0, 0x33, 4},     // [4] KEY_ALARM1 PINC.2
    {-1, 0, 0, 0, 0, 0, 0x33, 2},     // [5] KEY_ALARM2 PINC.4
    {-1, 0, 0, 0, 0, 0, 0x33, 3},     // [6] !9.08.12 Замена кнопок FILTER PINB.1 на STOP PINC.3
};
// Счетчик срабатываний таймера 1
unsigned char t_key = 0; 
// Функция, возвращающая значение клавиши
unsigned char key(unsigned char i) {
    // return ((keys[i].status == KEY_POLL) ? keys[i].last : keys[i].status);
    if (i <= 3 ) {
         if (keys[i].status == KEY_POLL) {
                return KEY_OFF;
         } else {
                if (keys[i].status == KEY_ON) {
                    //printf ("кнопкa #%d. Статус = %d\n", i, key_treated[i]);
                    if (key_treated[i] == 0) {
                        signal_buz(SHORT); 
                        return KEY_ON;
                    } else return KEY_OFF;
                 }
                if (keys[i].status == KEY_OFF) {
                    key_treated[i] = 0;
                    //printf ("Опросили кнопку #%d. Статус = %d\n", i, key_treated[i]);
                    return KEY_OFF;
                  }   
         } 
    } else {
        if (keys[i].status == KEY_POLL) {
            return (keys[i].last);
        } else {
            //signal_buz(SHORT);
            return (keys[i].status);
        }             
    }
}
// Функция инициализации всех кнопок
void init_keys(void) {
    register unsigned char i;
    for (i=0; i<NUM_KEYS; i++) {
        t_key = KEY_INACTIVE;
        while (keys[i].status == KEY_POLL) {
            poll_key(i);
            if(keys[i].status != KEY_POLL && keys[i].last != keys[i].status) {
                keys[i].last = keys[i].status;
                keys[i].status = KEY_POLL;
                if (t_key == 0) break;        // printf ("сработал таймер\n");
            } else {
                keys[i].is_work = 1;
            }
        }
        //printf ("Опросили кнопку #%d. Статус = %d\n", i, keys[i].status);
    }
}
// Функция опроса всех кнопок
void poll_keys(void) {
    register unsigned char i;
    #ifndef NODEBUG
    // printf ("Опрос всех кнопок (NUM_KEYS)\n");
    #endif
    for (i=0; i<NUM_KEYS; i++) {
        poll_key(i);    // if keys[i].status
        //if (key(i) != keys[i].last)
        // printf("Нажата кнопка #%d\n", i);
    }
}
// Опрос конкретной кнопки
void poll_key(unsigned char i) {
    unsigned char pin_val = 0;
    union reg_port {
        unsigned char num[2];
        unsigned int *p;
    } curr_port;
                      
    curr_port.num[0] = keys[i].reg; curr_port.num[1] = 0;
    // printf ("Опрашиваем клавишу 0x%x, нога %d", keys[i].reg, keys[i].pin);
    pin_val = BITSET(*(curr_port.p), keys[i].pin) >> keys[i].pin; 
    // printf (", результат 0x%x\n", pin_val);
    switch (keys[i].status) {
        case KEY_ON:
            if (pin_val == KEY_OFF) {
                keys[i].status = KEY_POLL;
                VALCODER_DISABLE();
                keys[i].on = 0;
                keys[i].off = 1;
                keys[i].last = KEY_ON;    
            } 
            break;
        case KEY_OFF: 
            if (pin_val == KEY_ON) {
                keys[i].status = KEY_POLL;
                VALCODER_DISABLE();
                keys[i].on = 1;
                keys[i].off = 0;
                keys[i].last = KEY_OFF;    
            } 
            break;
        case KEY_POLL:
            (pin_val == KEY_ON) ? keys[i].on++ : keys[i].off++;
            // Выборы окончены?
            if (keys[i].on >= MAX_POLL || keys[i].off >= MAX_POLL) {
                keys[i].status = (keys[i].on > keys[i].off) ? KEY_ON : KEY_OFF;
                //printf(".");
                VALCODER_ENABLE();
                // if (keys[i].status != keys[i].last)
                    // printf ("Адрес: 0x%x, Нога: %d, Значение: %d, предыдущее значение %d\n", 
                    //     keys[i].reg, keys[i].pin, keys[i].status, keys[i].last); 
            } 
            break;
        default:
    };
}