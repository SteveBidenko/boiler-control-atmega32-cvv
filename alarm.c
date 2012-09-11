#include <stdio.h>
#include <string.h>
#include "robowater.h"
#include "at2404.h"
#include "spd1820.h"
#include "alarm.h"
#include "bits.h"
#include "menu.h"

struct st_alarm alarm;      // ������� alarm

flash alarm_text all_alarms[MAX_ALARMS] = {
        "����� ����.",      // [0]
        "�� ����� ",        // [1]
        "��� ����.B1 ",     // [2]
        "��� ����.B2 ",     // [3]
        "��� �� �����"      // [4]
};
// ������� ����������� ���������� �������� ������ � ������� ��� ����
void update_alarm_menu (void) {
    register unsigned char i;
    KOL_ALERT = 0;
    for (i = 0; i < MAX_ALARMS; i++) {
        alarms[i].can_edit = prim_par.alarm_status[i];
        if (prim_par.alarm_status[i]) {
            KOL_ALERT++;
            if (alarms[i].val_data == 0) alarms[i].val_data = 1;
        }
    }
    IS_ALERT = KOL_ALERT;
}
// ������� ����������� ALARM
void alarm_reg (int reg_preset, int reg_alarm, char *text_alarm, unsigned char code) {
    sprintf (alarm.datestamp, "%02u:%02u %02u.%02u", s_dt.cHH, s_dt.cMM, s_dt.cdd, s_dt.cmo);
    sprintf (alarm.text, "%s", text_alarm);
    alarm.val_preset = reg_preset;
    alarm.val_alarm = reg_alarm;
    // ������������ ������� � ����� ��������� prim_par
    prim_par.alarm_status[code] = 1;
    alarms[code].val_data = reg_alarm;
    // ���������� ������� ��������������� alarm
    if (prim_par.c_alerts < MAX_ALERTS) prim_par.c_alerts++;
    prim_par.alert++;
    if (prim_par.alert > prim_par.c_alerts) prim_par.alert = 1;
    // ������ ALARM
    eeprom_write_alert ((char *)&alarm, sizeof(alarm), prim_par.alert);
    // ������ ��������� ��������� (��� �������� ������� � ����� ALARM)
    eeprom_write_struct ((char *)&prim_par, sizeof(prim_par));
    update_alarm_menu ();
    return;
}
// ������� ��������� ALARM
signed char alarm_unreg (unsigned char code) {
    // ���� ������� ���, ���������� ������� -1
    if (prim_par.alarm_status[code] == 0) return -1;
    // ��������������� ������� � ����� ��������� prim_par
    prim_par.alarm_status[code] = 0;
    // ������ ��������� ��������� (��� �������� ������� � ����� ALARM)
    eeprom_write_struct ((char *)&prim_par, sizeof(prim_par));
    update_alarm_menu ();
    if (code >= 2 && code <= 3) read_all_terms(INIT_MODE);
    return;
}
// ������� ������ ALARM �� ������ �� EEPROM
signed char alarm_read (unsigned char num) {
    // ���� ������� ���, ���������� ������� -1
    if (num == 0) return -1;
    eeprom_read_alert ((char *)&alarm, sizeof(alarm), num);
}
void alarm_c_print (void) {
    printf("> %s %i %i %s\n", alarm.datestamp, alarm.val_preset, alarm.val_alarm, alarm.text);
}
// ������� ���������� ��� ALARMs
void alarm_all_print (void) {
    register unsigned char i;
    unsigned char c;
    c = prim_par.alert;
    // �������� ��� ������������������ alarms ������� � ������� � � �������� �������
    printf("��� ������:\n");
    for (i = 1; i <= MAX_ALERTS; i++) {
        alarm_read(c--);
        alarm_c_print();
        if (c == 0) c = prim_par.c_alerts;
    }
}
// �������, ������������ ������ �������� ������� �� ����
char *get_alarm_str(unsigned char code) {
    static alarm_text alert_string;
    if (code < MAX_ALARMS) {
        strcpyf (alert_string, all_alarms[code]);
        return alert_string;
    } else {
        return NULL;
    }
}