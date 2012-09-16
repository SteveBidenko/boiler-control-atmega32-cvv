#include <mega32.h>
#include <delay.h>
#include "robowater.h"
#include "valcoder.h"

// byte valcoder0, valcoder1;      // Биты прерываний
// word counter0, counter1;        // счетчик кол-ва прерываний от valcoder
// int valcoder;                   // Направление вращения
signed char valcoder;              // Направление вращения

// Внешние прерывания Valcoder'а
interrupt [EXT_INT0] void ext_int0_isr(void) {
    // counter0++;
    // valcoder0 = 1;
    // Проверяем, обработали ли прокрутку valcoder'а
    // if (valcoder == VALCODER_NO_ROTATE) {
    // Проверяем VALCODER_PIN1
    if (VALCODER_PIN1) valcoder--; else valcoder++;
    // }
    // Устраняем дребезг прерывания
    GIFR |= (1<<6);
    // Запрещаем прерывание INT0 (выставляем 0 в 6 бите)
    GICR &= ~(1<<6);
    // Ставим небольшую задержку
    // delay_ms(VALCODER_INT_DELAY);
}
interrupt [EXT_INT1] void ext_int1_isr(void) {
    // counter1++;
    // valcoder1 = 1;
    // Прежде чем разрешать прерывание, делаем небольшую задержку
    // delay_ms(VALCODER_INT_DELAY);
    // Разрешаем прерывание INT0 (выставляем 1 в 6 бите)
    GICR |= (1<<6);
    // GIFR |= (1<<7);
}
