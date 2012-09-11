#include <mega32.h>
#include <delay.h>
#include "robowater.h"
#include "valcoder.h"

// byte valcoder0, valcoder1;      // ���� ����������
// word counter0, counter1;        // ������� ���-�� ���������� �� valcoder
// int valcoder;                   // ����������� ��������
signed char valcoder;              // ����������� ��������

// ������� ���������� Valcoder'�
interrupt [EXT_INT0] void ext_int0_isr(void) {
    // counter0++;
    // valcoder0 = 1;
    // ���������, ���������� �� ��������� valcoder'�
    // if (valcoder == VALCODER_NO_ROTATE) {
    // ��������� VALCODER_PIN1
    if (VALCODER_PIN1) valcoder--; else valcoder++;
    // }
    // ��������� ������� ����������
    GIFR |= (1<<6);
    // ��������� ���������� INT0 (���������� 0 � 6 ����)
    GICR &= ~(1<<6);
    // ������ ��������� ��������
    // delay_ms(VALCODER_INT_DELAY);
}
interrupt [EXT_INT1] void ext_int1_isr(void) {
    // counter1++;
    // valcoder1 = 1;
    // ������ ��� ��������� ����������, ������ ��������� ��������
    // delay_ms(VALCODER_INT_DELAY);
    // ��������� ���������� INT0 (���������� 1 � 6 ����)
    GICR |= (1<<6);
    // GIFR |= (1<<7);
}
