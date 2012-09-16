#ifndef __SIGNALS__
#define __SIGNALS__

// ��������� ������������� � ��������
#define SHORT_MAX 1
#define LONG_MAX 5
#define MEANDR_MAX 3
// ������
#define OFF 0
#define ON 1
#define SHORT 2
#define LONG 3
#define MEANDR 4
// �������� ��������� ��������� ���� ��������
struct st_signal {
    unsigned char buz_mode;      // ����� ������ ��������� ����������: 0 - ��� �����, 1 - �������� ���, 2 - ������� ���, 3 - ������� ������
    unsigned char buz_timer;
    unsigned char buz_status;
    // ����� ������ ��������� ����������: 0 - ����., 1 - �������� ��������, 2 - ������� ��������, 3 - ����������� ��������, 4 - ���. ���������
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
// �������� ������� �������
extern void update_signal_status(void);         // ���������� ������ �� �������. ��������� ���������� �������������� �� ������� �����.
// extern void init_signal(void);                  // ����������������� ����� ���� �������� ???
extern void signal_buz(unsigned char);
extern void signal_green(unsigned char);
extern void signal_red(unsigned char);
extern void signal_white(unsigned char);
extern void signal_printallbytes (void);

#endif