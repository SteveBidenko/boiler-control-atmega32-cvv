#ifndef _VALCODER_
#define _VALCODER_
// Определение подстановок
#define VALCODER_PIN0 (PIND.2)
#define VALCODER_PIN1 (PIND.3)
#define VALCODER_DISABLE() GICR &= ~(3<<6)
#define VALCODER_ENABLE() GICR |= (3<<6)
// #define VALCODER_TO_LEFT -1
// #define VALCODER_TO_RIGHT 1
#define VALCODER_NO_ROTATE 0
#define VALCODER_DELAY 15
#define VALCODER_SENSITY 1
// Описание функций
// Описание глобальных переменных
// extern byte valcoder0, valcoder1;
// extern word counter0, counter1;
// extern int valcoder;
extern signed char valcoder;
#endif
