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

        void save_voltage_config();
        void save_geiger_time_config();
        void save_tone_delay();

        byte GEIGER_TIME = 37;
        bool detected = false;

        uint16_t *rad_buff;// = new uint16_t[GEIGER_TIME]; //массив секундных замеров для расчета фона
        uint32_t rad_sum; //сумма импульсов за все время
        uint32_t rad_back; //текущий фон
        uint32_t rad_max; //максимум фона
        uint32_t rad_dose; //доза
        uint8_t time_sec; //секунды //счетчики времени
        uint8_t time_min; //минуты
        uint8_t time_hrs; //часы

        byte pwm_converter = 45;

        bool is_sleeping = false;

        byte ton_BUZZ = 200; //тональность буззера

        //display
        byte cursor = 0;
        bool editing_mode = false;
        bool redraw_required = true;

        byte contrast = 50;
        byte backlight = 0;

        int page = 0;
        int menu_page = 0;
        //display//

        volatile byte wdt_counter;

};