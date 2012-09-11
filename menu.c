#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// DS1307 Real Time Clock functions
#include <ds1307.h>
#include <delay.h>

#include "robowater.h"
#include "menu.h"
#include "spd1820.h"
#include "alarm.h"
#include "at2404.h"

// Объявление макроподстановок
// #define MENU_PARAMETERS 12

// Структура основных переменных
// extern struct st_prim_par prim_par;

// signed char curr_menu_level1 = 0, next_menu_level1 = 1;  // Текущий и следующий пункт меню
struct st_array_pos curr_menu;                // Позиции в меню
struct st_array_pos dt_curr_menu;             // Здесь сохраняем главное меню во время входа в меню установки даты и времени
// struct st_array_pos menu_level2;  // Позиции в меню второго уровня
//    {1, e_clatsman, "ЗИМА "},              // [9] Режим обогрева (1 - зима, 0 -лето)
struct st_parameter main_menu[NUM_MENU]= {   // Меню первого уровня
    {-9900, e_temperature, 0, 1},       // [0] Tемпература на улице ± 40
    {-9900, e_temperature, 0, 2},       // [1] Расчетная температура подачи 
    {0, e_percent, 0, 3},               // [2] Расчетный угол заслонки 
    {0, e_dt, 1, 4},                    // [3] Установка даты и времени
    {0, e_empty, 1, 5},                 // [4] Вход в меню просмотра параметров
    {0, e_sysok, 0, 6},                 // [5] АВАРИЙ НЕТ
    {0, e_empty, 0, 7}                  // [6] Вход в меню тонких настроек " Наладка"
};

struct st_parameter sdt[NUM_DT]= {   // Меню Установки времени и даты
    {20, e_hour, 1, 8},             // [0]
    {05, e_minute, 1, 9},           // [1]
    {01, e_day, 1, 10},             // [2]
    {06, e_month, 1, 11},           // [3]
    {12, e_year, 1, 12},            // [4]
    {0, e_password, 1, 33}          // [5]
  };
// environment
struct st_parameter parameters[NUM_PARAMETERS]= {
    {6000, e_temperature, 1, 13},   // [0] Измеренная температура воды на Подаче t=
    {0, e_temperature, 1, 14},   // [1] Измеренная температура воздуха на улице УЛ.Т =
    {7200, e_temperature, 0, 15},   // [2] Расчетная температура подачи График Т=
    {0, e_percent, 0, 16},         // [3] Расчетный угол крана в процентах (расчетное PWM)
    {0, e_percent, 0, 17}          // [4] Текущее состояние крана (АЦП)   
};
// naladka
struct st_parameter settings[NUM_SETTINGS]={
    {-2500, e_world, 1, 18},        // [0] температура воздуха мин ta_min нижняя точка перегиба графика
    {10500, e_water, 1, 19},        // [1] Температура подачи теплоносителя tw_max для нижней точки перегиба графика
    {1000, e_world, 1, 20},         // [2] Температура воздуха на улице ta_max верхняя точка перегиба графика
    {3000, e_water, 1, 21},         // [3] Температура подачи теплоносителя ta_min для верхней точки перегиба графика
    {30, e_room, 1, 22},            // [4] Дельта тепературы Подачи 1-10 градусов Цельсия 
    {10, e_stime, 1, 23},           // [5] Время интегрирования T_int секунд 1-2048 сек 
    {1, e_coef, 1, 24},             // [6] Коэффициэнт усиления Ku
    {0, e_coef, 1, 25},             // [7] Коэффициэнт Интегрирования Ki
    {0, e_coef, 1, 26},             // [8] Коэффициэнт Дифференцирования Kd
    {0, e_temperature, 1, 27},      // [9] Смещение TW термометр Подачи
    {0, e_scale, 1, 28},            // [10] Множитель TW термометр Подачи
    {0, e_temperature, 1, 29},      // [11] Смещение TA термометр Улица
    {0, e_scale, 1, 30},            // [12] Множитель TA термометр Улица
    {1, e_weekday, 1, 40},          // [13] Установка дня недели
    {0, e_hour, 1, 31},             // [14] Установка часа проведения ТО Крана
    {0, e_minute, 1, 32},           // [15] Установка минуты проведения ТО Крана
    {48, e_PWM, 1, 34},             // [16] Установка нижней границы выхода
    {250, e_PWM, 1, 35},            // [17] Установка верхней границы выхода
    {48, e_ADC, 1, 36},             // [18] Установка нижней границы входа
    {250, e_ADC, 1, 37},            // [19] Установка верхней границы входа
    {0, e_clatsman, 1, 38},         // [20] Функция кнопки KEY_ALARM1
    {0, e_clatsman, 1, 39}          // [21] Функция кнопки KEY_ALARM2
};                                          
#define ALARM_POS 61
struct st_parameter alarms[MAX_ALARMS] = {
    {0, e_alarm, 0, ALARM_POS},
    {0, e_alarm, 0, ALARM_POS + 1},
    {0, e_alarm, 0, ALARM_POS + 2},
    {0, e_alarm, 0, ALARM_POS + 3},
    {0, e_alarm, 0, ALARM_POS + 4}   
};
flash lcd_str all_menu_str[] = {
        "Ул. t= ",          // [1]  Tемпература на улице ± 
        "График t= ",       // [2]  Расчетная температура подачи 
        "Кран расч.",       // [3]  Расчетный угол заслонки в процентах
        "Уст. ",            // [4]  Установка времени, даты, пароля
        "Параметры ",       // [5]  Просмотр параметров
        "АВАРИЙ ",          // [6]  Просмотр Аварий
        "Наладка...",       // [7]  Наладка инженерных параметров
        "Уст.Час. ",        // [8]
        "Уст Мин ",         // [9]
        "Уст.день. ",       // [10]
        "Уст Мес. ",        // [11]
        "Уст ГОД. ",        // [12]
        "t = ",             // [13] Измеренная температура воды на Подаче t=
        "Ул. t = ",         // [14] Измеренная температура воздуха на улице 
        "График t= ",       // [15] Расчетная температура подачи 
        "Кран Расч.",       // [16] Расчетный угол заслонки в процентах
        "Кран Изм. ",       // [17] Измеренный угол заслонки в процентах
        "TA Min ",          // [18] Температура воздуха на улице ta_min нижняя точка перегиба графика 
        "TW Max ",          // [19] Температура подачи теплоносителя tw_max для нижней точки перегиба графика
        "TA Max ",          // [20] Температура воздуха на улице TA_Max верхняя точка перегиба графика
        "TW Min ",          // [21] Температура подачи теплоносителя TA_Min для верхней точки перегиба графика
        "Дельта t=  ",      // [22] Дельта тепературы Подачи 1-10 градусов Цельсия
        "ВР.ИНТ. ",         // [23] Время интегрирования T_int секунд 1-2048 сек 
        "КУ=",              // [34] Коэффициэнт усиления Ku
        "КИ=",              // [25] Коэффициэнт Интегрирования Ki
        "КД=",              // [26] Коэффициэнт Дифференцирования Kd
        "См.П.=",           // [27] 0xFE
        "Шк.П.=",           // [28] 0xFE
        "См.Ул.=",          // [29] 0xFD
        "Шк.Ул.=",          // [30] 0xFD
        "Час ТО= ",         // [31] Установка часа проведения ТО Крана 
        "Минут ТО= ",       // [32] Установка минуты проведения ТО Крана
        "Пароль=",          // [33] Приглашение на ввод пароля
        "Вых.снизу= ",      // [34] Установление вых напряжение снизу
        "Вых.сверху= ",     // [35] Установление вых напряжение снизу
        "Вх.снизу= ",       // [36] Установление вых напряжение снизу
        "Вх.сверху= ",      // [37] Установление вых напряжение снизу
        "Кнопка 1=",        // [38] Функция кнопки KEY_ALARM1
        "Кнопка 2=",        // [39] Функция кнопки KEY_ALARM2
        "День ТО ="         // [40] День недели ТО
    };
char linestr[20];           // Строка для LCD
bit need_eeprom_write;      // Флаг, если необходимо записать в EEPROM
int menu_value;             // текущее значение настраиваемого параметра
// Функция синхронизации структуры основных переменных
void sync_set_par(byte sync) {
    register byte i;
    // Если входим в меню
    if (sync == SYNC_TO_MENU) {
        // main_menu[2].val_data = prim_par.season;        // Меню зима-лето
        settings[0].val_data = prim_par.ta_min;     //
        settings[1].val_data = prim_par.tw_max;     //
        settings[2].val_data = prim_par.ta_max;     //
        settings[3].val_data = prim_par.tw_min;     //
        settings[4].val_data = prim_par.set_delta;  //
        settings[5].val_data = prim_par.T_int;      //
        settings[6].val_data = prim_par.Ku;         //
        settings[7].val_data = prim_par.Ki;         //
        settings[8].val_data = prim_par.Kd;         //
        settings[9].val_data = prim_par.Kd;
        settings[13].val_data = prim_par.TO.weekday;
        settings[14].val_data = prim_par.TO.hour;
        settings[15].val_data = prim_par.TO.minute;
        settings[16].val_data = prim_par.PWM_lo;
        settings[17].val_data = prim_par.PWM_hi;
        settings[18].val_data = prim_par.ADC_lo;
        settings[19].val_data = prim_par.ADC_hi;
        settings[20].val_data = prim_par.function_keys1; // Функционирование кнопки KEY_ALARM1
        settings[21].val_data = prim_par.function_keys2;  // Функционирование кнопки KEY_ALARM2
        main_menu[0].val_data = parameters[1].val_data; // Синхронизация двух ячеек 
        main_menu[1].val_data = parameters[2].val_data; // Синхронизация двух ячеек
        //parameters[1].val_data = TW_x;
        // printf("Чтени времени и даты \n");
        sdt[0].val_data = s_dt.cHH;
        sdt[1].val_data = s_dt.cMM;
        sdt[2].val_data = s_dt.cdd;
        sdt[3].val_data = s_dt.cmo;
        sdt[4].val_data = s_dt.cyy;
    } else {
        if (mode.stop_sync_dt) {
            if ((s_dt.cHH != sdt[0].val_data) || (s_dt.cMM != sdt[1].val_data)) {
                s_dt.cHH = sdt[0].val_data;
                s_dt.cMM = sdt[1].val_data;
                s_dt.cSS = 0;
                set_cur_dt();
                printf("Запись времени %02u:%02u:00\n",sdt[0].val_data, sdt[1].val_data);
                printf("Дата %02u.%02u.20%02u\n", sdt[2].val_data, sdt[3].val_data, sdt[4].val_data);
            };
            if ((s_dt.cdd != sdt[2].val_data) || (s_dt.cmo != sdt[3].val_data) || (s_dt.cyy != sdt[4].val_data)) {
                s_dt.cdd = sdt[2].val_data;
                s_dt.cmo = sdt[3].val_data;
                s_dt.cyy = sdt[4].val_data;
                set_cur_dt();
                printf("Запись даты %02u.%02u.20%02u\n", sdt[2].val_data, sdt[3].val_data, sdt[4].val_data);
                printf("Время %02u:%02u:00\n",sdt[0].val_data, sdt[1].val_data);
            };
        } else {
            // Проверяем каждый параметр и если он отличается от начального, то выставляем флаг необходимости записи в EEPROM
            if (prim_par.ta_min != settings[0].val_data) {
                prim_par.ta_min = settings[0].val_data; need_eeprom_write = 1;
            }
            if (prim_par.tw_max != settings[1].val_data) {
                prim_par.tw_max = settings[1].val_data; need_eeprom_write = 1;
            }
            if (prim_par.ta_max != settings[2].val_data) {
                prim_par.ta_max = settings[2].val_data; need_eeprom_write = 1;
            }
            if (prim_par.tw_min != settings[3].val_data) {
                prim_par.tw_min = settings[3].val_data; need_eeprom_write = 1;
            }
            if (prim_par.set_delta != settings[4].val_data) {
                prim_par.set_delta = settings[4].val_data; need_eeprom_write = 1;
            }
            if (prim_par.T_int != settings[5].val_data) {
                prim_par.T_int = settings[5].val_data; need_eeprom_write = 1;
            }
            if (prim_par.Ku != settings[6].val_data) {
                prim_par.Ku = settings[6].val_data; need_eeprom_write = 1;
            }
            if (prim_par.Ki != settings[7].val_data) {
                prim_par.Ki = settings[7].val_data; need_eeprom_write = 1;
            }
            if (prim_par.Kd != settings[8].val_data) {
                prim_par.Kd = settings[8].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TO.weekday != settings[13].val_data) {
                prim_par.TO.weekday = settings[13].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TO.hour != settings[14].val_data) {
                prim_par.TO.hour = settings[14].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TO.minute != settings[15].val_data) {
                prim_par.TO.minute = settings[15].val_data; need_eeprom_write = 1;
            }
            if (prim_par.PWM_lo != settings[16].val_data) {
                prim_par.PWM_lo = settings[16].val_data; need_eeprom_write = 1;
            }
            if (prim_par.PWM_hi != settings[17].val_data) {
                prim_par.PWM_hi = settings[17].val_data; need_eeprom_write = 1;
            }
            if (prim_par.ADC_lo != settings[18].val_data) {
                prim_par.ADC_lo = settings[18].val_data; need_eeprom_write = 1;
            }
            if (prim_par.ADC_hi != settings[19].val_data) {
                prim_par.ADC_hi = settings[19].val_data; need_eeprom_write = 1;
            }
            if (prim_par.function_keys1 != settings[20].val_data) {
                prim_par.function_keys1 = settings[20].val_data; need_eeprom_write = 1;
            }
            if (prim_par.function_keys2 != settings[21].val_data) {
                prim_par.function_keys2 = settings[21].val_data; need_eeprom_write = 1;
            }
            for (i = 0; i < MAX_ALARMS; i++) {
                if (prim_par.alarm_status[i] && (alarms[i].val_data == 0)) {
                 alarm_unreg (i);
                 printf ("Удалили активную тревогу: %s\n", get_alarm_str(i));
                 read_all_terms(INIT_MODE);
                 printf ("Инициализируем все термометры!\n");
                 }
            }
        };
     };
}
// Функция, которая возвращает строку со строковым названием параметра, и его числовым значением
// Аргументы - номер элемента массива и указатель на массив
// Возврат - указатель на строку linestr
char *param_str(byte num_line, struct st_parameter parameter[]) {
    int pr_data;
    struct st_parameter *st_pointer;
    st_pointer = &parameter[num_line];          // Передаем конкретный указатель на структуру текущего элемента массива
    pr_data = st_pointer->val_data;
    /* struct st_parameter *pnt; pnt = parameter + num_line; */
    return (par_str(st_pointer, 0, pr_data));
}
// Универсальная функция возвращающая строку с преобразованным типовым значением.
// 1) Аргумент - указатель на структуру параметра
// 2) Флаг - признак вывода только значения. (0 - выводить строковое название, 1 - не выводить строковое название)
// 3) Данные, которые нужно подставить
// Возврат - указатель на строку linestr
char *par_str(struct st_parameter *st_pointer, unsigned char only_val, int pr_data) {
    char *pr_name;
    char prompt[] = "=> ";
    unsigned char sign = (pr_data < 0) ? '-' : '+';
    if (only_val) pr_name = prompt; else pr_name = getmenustr(st_pointer->str_num);
    switch (st_pointer->val_type) {
        case e_empty:
            sprintf(linestr, "%s", pr_name);
            break;
        case e_clatsman:
            // Если указан тип клацман, то печатаем со словом ВКЛ. или Выкл.
            if(pr_data)
                sprintf(linestr, "%sВХОД ", pr_name);
            else
                sprintf(linestr, "%sТЕСТ", pr_name);
            break;
        case e_percent:
            // Если указан тип проценты, то печатаем как проценты
            sprintf(linestr, "%s%u.%-01u%%", pr_name, abs(pr_data)/10, abs(pr_data%10));
            //sprintf(linestr, "%s%u%%", pr_name, pr_data);
            break;
        case e_PWM: // Если указан тип e_PWM или e_ADC, то печатаем как просто число   
        case e_ADC:
            sprintf(linestr, "%s%u",pr_name, pr_data);
            break;
        case e_coef: // Коэффициент умножения Kp,Ki,Kd
            sprintf(linestr, "%s%u.%-01u%",pr_name, abs(pr_data)/10, abs(pr_data%10));
            break;
            // Если указан тип шкалы, то печатаем как знаковый байт (-128..127)
            sprintf(linestr, "%s%d", pr_name, pr_data);
            break;    
        case e_scale:
            // Если указан тип шкалы, то печатаем как знаковый байт (-128..127)
            sprintf(linestr, "%s%d", pr_name, pr_data);
            break;
        case e_password:
            // Если указан признак ввода пароля, то просто указываем несколько символов '**'
            if (only_val) sprintf(linestr, "%s%u", pr_name, pr_data);
            else sprintf(linestr, "%s **", pr_name, pr_data);
            break;
        case e_sysok:
            // Если указан признак аварий, то выводим либо слово нет, либо количество аварий и '!'
            if (pr_data) sprintf(linestr, "%s%u!", pr_name, pr_data);
            else sprintf(linestr, "%sНЕТ", pr_name);
            break;
        case e_alarm:
            if (pr_data) sprintf(linestr, "%s%u!", pr_name, pr_data);
            else sprintf(linestr, "%s", pr_name);
            break;
        case e_time:
            // Если указан тип время, то выбираем из структуры времени и печатаем как чч.мм.сс
            sprintf(linestr, "%s%02u:%02u", pr_name, s_dt.cHH, s_dt.cMM);
            break;
        case e_date:
            // Если указан тип дата, то выбираем из структуры даты и печатаем как дд.мм.гг
            sprintf(linestr, "%s%02u.%02u.%02u", pr_name, s_dt.cdd, s_dt.cmo, s_dt.cyy);
            break;
        case e_dt:
            sprintf(linestr, "%s Вр./Дат.", pr_name);
            break;
        case e_stime:
            // Если указан тип дата, то выбираем из структуры даты и печатаем как дд.мм.гг
            sprintf(linestr, "%s%03u", pr_name, pr_data);
            break;
        case e_temperature:
        case e_room:
        case e_world:
        case e_water:
            // Если указан тип температуры, то печатаем как температуру
            sprintf(linestr, "%s%c%u.%-01uC", pr_name, sign, abs(pr_data)/100, abs(pr_data%100)/10);
            break;
        case e_hour:
        case e_minute:
        case e_day:
        case e_month:
        case e_weekday:
            // Если указан тип [часы, минуты, день, месяц], то печатаем как есть
            sprintf(linestr, "%s%u", pr_name, pr_data);
            break;
        case e_year:
            // Если указан тип год, то печатаем как есть с тысячелетием
            sprintf(linestr, "%s20%u", pr_name, pr_data);
            break;
        default:
            linestr[0] = 0;
    };
    return(linestr);
}
// Функция, инициализирующая menu_level1
void init_curr_menu(struct st_parameter *lmenu, unsigned char lmax) {
    curr_menu.line0 = 0; curr_menu.line1 = 1;

    // curr_menu.menu = &main_menu[0];         // Текущий указатель устанавливаем на главное меню
    curr_menu.menu = lmenu;         // Текущий указатель устанавливаем на главное меню
    // curr_menu.maximum = MENU_LEVEL1;
    curr_menu.maximum = lmax;
    curr_menu.lcd = 0; // curr_menu.value = lmenu->val_data;
}
// Функция, модифицирующая структуру содержащую указатели на текущий и следующий элемент. direction - (-1, 0, 1)
void calc_pos(struct st_array_pos *array_pos, byte maxlines, signed char direction) {
    // Увеличиваем или уменьшаем текущий элемент массива строк
    // array_pos->line0 = array_pos->line0 + direction;
    array_pos->line0 += direction;      // выборка элемента структуры по адресу array_pos
    // Начинаем проверки на укладываемость в диапазоне
    if (array_pos->line0 >= maxlines) array_pos->line0 = 0;
    if (array_pos->line0 < 0) { array_pos->line0 = maxlines - 1; array_pos->line1 = 0; return; }
    // Анализируем то, что мы будем возвращать
    if ((array_pos->line0 + 1) == maxlines) array_pos->line1 = 0; else array_pos->line1 = array_pos->line0 + 1;
}
// Функция вывода текущего и следующего пункта меню
void lcd_menu(signed char direction) {
    // printf ("Syncing...\n");

    // Если valcoder крутится вправо и курсор был на 1 линии, то ставим ему признак перевода на 2 линию
    if ((curr_menu.lcd == 0) && (direction > 0))    // Анализируем, где находится курсор и куда крутим valcoder
        curr_menu.lcd = 1;
    else
        // Если valcoder крутится влево и курсор был на 2 линии, то ставим ему признак перевода на 1 линию
        if ((curr_menu.lcd == 1) && (direction < 0))
           curr_menu.lcd = 0;
        else
            // Если направление отличается от нуля, то модифицируем текущие пункты меню
            if (direction) calc_pos(&curr_menu, curr_menu.maximum, direction);
    // printf("LCDLine=%u, dir=%u, line0=%u, line1=%u\n", curr_menu.lcd, direction, curr_menu.line0, curr_menu.line1);
    lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 строки 0
    lcd_line_menu(param_str(curr_menu.line0, curr_menu.menu), curr_menu.menu[curr_menu.line0].can_edit);
    lcd_gotoxy(0,1);        // Устанавливаем курсор в позицию 0 строки 1
    lcd_line_menu(param_str(curr_menu.line1, curr_menu.menu), curr_menu.menu[curr_menu.line1].can_edit);
    // Устанавливаем курсор в ту строку, на которую указывает флага curr_menu.lcd
    if (curr_menu.lcd) lcd_gotoxy(LCD_DISP_LENGTH-1,1); else lcd_gotoxy(LCD_DISP_LENGTH-1,0);
    // Даем команду на моргание курсором
    lcd_command(LCD_DISP_ON_CURSOR_BLINK);
}
// Функция, инициализирующая режим редактирования
void lcd_init_edit(void) {
    struct st_parameter *lmenu;     // Временная переменная, которая будет ссылаться на выбранный элемент меню
    byte num_line;                  // Временная переменная, содержащая текущий элемент меню
    // Устанавливаем указатель на параметры в зависимости от строки дисплея
    if (curr_menu.lcd) num_line = curr_menu.line1; else num_line = curr_menu.line0;
    // Если сейчас нулевой уровень меню и выбранная позиция num_line > 4
    if ((curr_menu.level == 0) && (num_line >= 3)) {
        sync_set_par(SYNC_TO_MENU); // Синхронизируем меню с глобальными структурами
        switch (num_line) {
            case 3:
                    dt_curr_menu = curr_menu;
                    init_curr_menu(&sdt[0], NUM_DT);
                    curr_menu.level = 1; // printf("Вход в меню установки даты и времени\n");
                    mode.stop_sync_dt = 1;
                    // sync_set_taym(SYNC_TO_MENU);
                    break;
            case 4: init_curr_menu(&parameters[0], NUM_PARAMETERS);
                    curr_menu.level = 1; // printf("Вход в меню ПАРАМЕТРЫ\n");
                    break;
            case 5:
                    if (IS_ALERT) {
                        init_curr_menu(&alarms[0], MAX_ALARMS);
                        curr_menu.level = 1; // printf("Вход в меню ALERTS\n");
                    }
                    break;
            case 6: 
                    if (SETTINGS_OPEN) {
                        init_curr_menu(&settings[0], NUM_SETTINGS);
                        curr_menu.level = 1 ;//printf ("Вход в меню НАСТРОЙКИ\n");
                    }
                    break;
            default: ;
        }
        mode.menu = 1;              // Общий режим - меню
        lcd_menu(0);
        return;
    }
    lmenu = &(curr_menu.menu)[num_line];
    if (lmenu->can_edit == 0) {
        lcd_menu(0);
        return;
    } else {
        mode.menu += lmenu->can_edit; // mode.menu = mode.menu + lmenu->can_edit (значение элемента can_edit указателя lmenu на структуру)
    }
    curr_menu.par = lmenu;
    curr_menu.val_data = lmenu->val_data;        // Устанавливаем в текущем меню параметр для редактирования
    lcd_edit(0);
}
// Функция, обрабатывающая выход по Enter
void lcd_save_edit (void) {
    struct st_parameter *lmenu;     // Временная переменная, которая будет ссылаться на выбранный элемент меню
    byte num_line;                  // Временная переменная, содержащая текущий элемент меню
    // Устанавливаем указатель на параметры в зависимости от строки дисплея
    if (curr_menu.lcd) num_line = curr_menu.line1; else num_line = curr_menu.line0;
    lmenu = &(curr_menu.menu)[num_line];
    curr_menu.par = lmenu;
    lmenu->val_data = curr_menu.val_data;
    sync_set_par(SYNC_FROM_MENU);
    if (mode.stop_sync_dt) {
        // printf ("Вход в меню main_menu \n"); // Синхронизация по выходу
        // init_curr_menu(&main_menu[0], NUM_MENU);
        curr_menu = dt_curr_menu;
        curr_menu.level = 0;
        mode.stop_sync_dt = 0;      // снимаем флаг запрета синхронизации
    }
    // Здесь параметр надо записать в EEPROM !!!!
    if (need_eeprom_write) {
        eeprom_write_struct ((char *)&prim_par, sizeof(prim_par));
        // Снимаем флаг необходимости записи в EEPROM
        need_eeprom_write = 0;
    }
    lcd_menu(0);
    return;
}
// Функция, инициализирующая/деинициализирующая режим редактирования (0 - инициализация, 1 - выход по Enter, -1 - выход по Cancel
void lcd_esc_edit(void) {
    struct st_parameter *lmenu;     // Временная переменная, которая будет ссылаться на выбранный элемент меню
    byte num_line;                  // Временная переменная, содержащая текущий элемент меню
    // Устанавливаем указатель на параметры в зависимости от строки дисплея
    if (curr_menu.lcd) num_line = curr_menu.line1; else num_line = curr_menu.line0;
    // Перенесен блок в lcd_init_edit
    lmenu = &(curr_menu.menu)[num_line];
    // Если не нулевой уровень и нажата кнопка Cancel или ev_timer
    // printf("INIT EDIT(%d): %s = %i-->%i [%u]\n", initmode, lmenu->val_name, curr_menu.val_data, lmenu->val_data, curr_menu.level);
    if (curr_menu.level || mode.stop_sync_dt) {
        // printf ("Вход в меню main_menu \n"); // Синхронизация по выходу
        if (mode.stop_sync_dt)
            curr_menu = dt_curr_menu;
        else
            init_curr_menu(&main_menu[0], NUM_MENU);
        curr_menu.level = 0;
        mode.stop_sync_dt = 0;
        return;
    }
    // Перенесен блок в lcd_init_edit
    curr_menu.par = lmenu;
    // printf("INIT EDIT(%d): %s = %i [%i]\n", initmode, lmenu->val_name, lmenu->val_data, curr_menu.val_data);
    curr_menu.val_data = 0;
    lcd_menu(0);
    return;
}
// Функция редактирующая текущий параметр меню. В качестве параметра передается вращение valcoder
void lcd_edit(signed char direction) {
    struct st_parameter *lmenu;
    lmenu = curr_menu.par;
    // printf("EDIT (%d): LCDLine=%u, line0=%u, line1=%u\n", direction, curr_menu.lcd, curr_menu.line0, curr_menu.line1);
    // (Де-)Инкрементацию делать надо в зависимости от типа переменной.
    switch (lmenu->val_type) {
        case e_empty:
            // curr_menu.val_data - не меняет свое значение
            break;
        case e_clatsman:
            // curr_menu.val_data - принимает значение 0 или 1
            if (direction) if(curr_menu.val_data) curr_menu.val_data = 0; else curr_menu.val_data = 1;
            // curr_menu.val_data = ~curr_menu.val_data;
            break;
        case e_temperature:
            // Изменение температуры идет с шагом +/-0,1 градуса
            curr_menu.val_data += (int)direction * 10;
            if (curr_menu.val_data > 10000) curr_menu.val_data = -3000;
            if (curr_menu.val_data < -3000) curr_menu.val_data = 10000;
            break;
        case e_room:
            // Изменение температуры идет с шагом +/-0,1 градуса
            curr_menu.val_data += (int)direction * 10;
            if (curr_menu.val_data > 1000) curr_menu.val_data = 10;
            if (curr_menu.val_data < 10) curr_menu.val_data = 1000;
            break;
        case e_world:
            // Изменение температуры идет с шагом +/-1 градус
            curr_menu.val_data += (int)direction * 100;
            if (curr_menu.val_data > 3000) curr_menu.val_data = -3000;
            if (curr_menu.val_data < -3000) curr_menu.val_data = 3000;
            break;
        case e_water:
            // Изменение температуры идет с шагом +/-1 градус
            curr_menu.val_data += (int)direction * 100;
            if (curr_menu.val_data > 10000) curr_menu.val_data = 500;
            if (curr_menu.val_data < 500) curr_menu.val_data = 10000;
            break;
        case e_percent:
            // Изменение порцентов идет с шагом +/-1 %
            curr_menu.val_data += direction;
            if (curr_menu.val_data > 1000) curr_menu.val_data = 0;
            if (curr_menu.val_data < 0) curr_menu.val_data = 1000;
            break;
        case e_ADC:
            mode.run = mo_setup_output;
            curr_menu.val_data = ADC_VAR1;
            break;
        case e_PWM:
            mode.run = mo_setup_input;
        case e_coef:
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0 ) curr_menu.val_data = 0xFF;
            if (curr_menu.val_data > 0xFF) curr_menu.val_data = 0;
            break;
        case e_scale:
            curr_menu.val_data += direction;
            if (curr_menu.val_data < -128 ) curr_menu.val_data = 127;
            if (curr_menu.val_data > 127) curr_menu.val_data = -128;
            break;
        case e_stime:
            if (curr_menu.val_data < 100) curr_menu.val_data += direction; else
                if (curr_menu.val_data < 1000) curr_menu.val_data += (int)direction*10; else
                    curr_menu.val_data += (int)direction*100;
            if (curr_menu.val_data < 0) curr_menu.val_data = 3600;
            if (curr_menu.val_data > 3600) curr_menu.val_data = 0;
            break;
        case e_minute:          // Время в минутах [00..59]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0) curr_menu.val_data = 59;
            if (curr_menu.val_data > 59) curr_menu.val_data = 0;
            break;
        case e_hour:            // Время в часах [00..23]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0) curr_menu.val_data = 23;
            if (curr_menu.val_data > 23) curr_menu.val_data = 0;
            break;
        case e_day:              // Дни [01..31]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 1) curr_menu.val_data = 31;
            if (curr_menu.val_data > 31) curr_menu.val_data = 1;
            break;
        case e_month:            // Месяцы [01..12]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 1) curr_menu.val_data = 12;
            if (curr_menu.val_data > 12) curr_menu.val_data = 1;
            break;
        case e_year:             // Года [00..99]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 11) curr_menu.val_data = 99;
            if (curr_menu.val_data > 99) curr_menu.val_data = 11;
            break;
        case e_weekday:             // Года [00..99]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0) curr_menu.val_data = 6;
            if (curr_menu.val_data > 6) curr_menu.val_data = 0;
            break;
        case e_alarm:
            if (direction) curr_menu.val_data = 0;
            break;
        case e_password:
            curr_menu.val_data += direction;
            SETTINGS_OPEN = (curr_menu.val_data == 20); 
            if (curr_menu.val_data < 0) curr_menu.val_data = 99;
            if (curr_menu.val_data > 99) curr_menu.val_data = 0;
            break;
        default:
            curr_menu.val_data += direction;
    }
    // Для системы в целом присваиваем только что измененное значение
    menu_value = curr_menu.val_data;
    // Рисуем две строки.
    lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 строки 1
    lcd_line_menu(par_str(lmenu, 0, lmenu->val_data), 0);
    lcd_gotoxy(0,1);        // Устанавливаем курсор в позицию 0 строки 1
    lcd_line_menu(par_str(lmenu, 1, curr_menu.val_data), 0);
    lcd_command(LCD_DISP_ON_CURSOR_BLINK);
}
#pragma used+
// Функция печатающая текущий и следующий элемент меню с использованием направления (-1, 0, 1)
void print_curr_menu2(signed char direction) {
    calc_pos(&curr_menu, curr_menu.maximum, direction);
    printf("Выбор меню [%u, %u]", curr_menu.line0, curr_menu.line1);
    printf(" 1) ""%s"",\t2) ""%s""\n", main_menu[curr_menu.line0], main_menu[curr_menu.line1]);
}
// Печать всех значений меню параметров в отладочном терминале
void print_curr_menu(void) {
    register byte i;
    // Выведем на отладочную консоль все пункты текущего (!) меню
    printf ("Текущее меню: ");
    for (i=0; i<curr_menu.maximum; i++) printf ("%s\t", param_str(i, curr_menu.menu));
    printf ("\n");
}
#pragma used-
// Функция, возвращающая указатель на строку меню по порядковому номеру
char *getmenustr(unsigned char menu_num_pp) {
    static lcd_str menustr;
    if (menu_num_pp) {
        if (menu_num_pp < ALARM_POS)
            strcpyf (menustr, all_menu_str[menu_num_pp-1]);
        else
            strcpy (menustr, get_alarm_str(menu_num_pp - ALARM_POS));
        return menustr;
    } else
        return NULL;
}
// Функция преобразования абсолютного значения относительно границ в %  
int calc_percent(unsigned char x, unsigned char lo, unsigned char hi) {
    long result;
    if (x <= lo ) result = 0;
        else {
        if (x >= hi ) result = 1000;
            else  
            result = (long)(x - lo)* 1000 / (hi - lo);
        }
    
    return ((int) result);
}
