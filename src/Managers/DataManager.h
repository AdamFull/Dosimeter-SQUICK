#pragma once
#include <Arduino.h>

#define TIMER1_PRELOAD 64910 //65535-64910=625, 15625/625=25Гц
#define HVGEN_FACT 5 // 25/5=5Гц частота подкачки преобразователя
#define TIME_FACT 25 // 25Гц/25=1Гц секундные интервалы

class DataManager{
    public:
        DataManager() {}
        void init();
        void setup_eeprom();

        void update_rad_buffer();

        void save_dose();
        void save_pwm();
        void save_tone();
        void save_bl();
        void save_contrast();
        void save_time();

        void read_eeprom();

        void reset_dose();
        void reset_settings();

        void reset_activity_test();

        byte GEIGER_TIME = 37;
        byte contrast = 60;
        bool backlight = 0;
        byte pwm_converter = 45;
        byte ton_BUZZ = 200; //тональность буззера

        //-----------------------Флаги-----------------------
        bool detected = false;              //флаг обнаружения частицы
        bool is_sleeping = false;           //флаг сна
        bool editing_mode = false;          //флаг редактирования
        bool saved = false;                 //флаг сохранения
        bool redraw_required = true;        //флаг отрисовки
        bool stop_timer = false;
        bool next_step = false;             //флаг для замера

        uint16_t *rad_buff;// = new uint16_t[GEIGER_TIME]; //массив секундных замеров для расчета фона
        uint32_t rad_sum, rad_back, rad_max, rad_dose, rad_dose_old; //сумма импульсов за все время/текущий фон/максимум фона/доза/предыдущая доза
        uint8_t time_sec, time_min, time_hrs; //счетчики времени
        uint8_t time_mens_sec = 1, time_mens_min = 0; //счетчики времени для замера
        uint16_t timer_time, timer_remain;
        uint32_t rad_sum_mens, rad_sum_mens_old; //сумма импульсов при измерении, Сумма импульсов с предыдущего замера(фон)

        byte stat = 0;

        //display
        byte cursor = 0;
        int page = 0;
        int menu_page = 0;
        byte counter_mode = 0;
        byte editable = 0;

        float battery_voltage = 0;
        //display//

        byte has_eeprom = 1;
        volatile byte wdt_counter;

};