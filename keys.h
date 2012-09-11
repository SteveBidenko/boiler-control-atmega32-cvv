#ifndef _KEYS_
#define _KEYS_

// ��� �������������
#define KEY_STOP  key(0)    // [0] KEY_STOP   PINC.3
//#define KEY_STOP_RANGE  range(0)   // [0] KEY_STOP   ������
#define KEY_START key(1)    // [1] KEY_START  PINC.5
#define KEY_ENTER key(2)    // [5] KEY_ENTER PINC.6
#define KEY_CANCEL key(3)   // [6] KEY_CANCEL PINC.7
#define KEY_ALARM1 key(4)   // [2] KEY_ALARM1 PINC.2
#define KEY_ALARM2 key(5)   // [3] KEY_ALARM2 PINC.4
#define KEY_FILTER key(6)   // [4] KEY_FILTER PINB.1
// �������� �������� �������
#define MAX_POLL 100
// ��������� �������
#define KEY_ON 0            // ����� ��������������� ������� �������
#define KEY_OFF 1           // ����� ��������������� ������� �������
#define KEY_POLL -1         //
#define KEY_INACTIVE 10     // ������, ������������ ����������������� ������
// ���-�� ������
#define NUM_KEYS 7
// ���������� ����������
extern unsigned char t_key; // ��� �������
// �������� �������
void init_keys(void);       // ������������� ���� ������
void poll_keys(void);       // ����� ���� ������
void poll_key(unsigned char); // ����� ���������� ������
unsigned char key(unsigned char);
extern unsigned char key_treated[7] ;

#endif
