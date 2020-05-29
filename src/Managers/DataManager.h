#pragma once
#include <Arduino.h>

#define TIMER1_PRELOAD 64910 //65535-64910=625, 15625/625=25Гц
#define HVGEN_FACT 5 // 25/5=5Гц частота подкачки преобразователя
#define TIME_FACT 25 // 25Гц/25=1Гц секундные интервалы

class DataManager{
    public:
        static DataManager& getInstance()
        {
            static DataManager instance; // Guaranteed to be destroyed.
            return instance;                // Instantiated on first use.
        }

    private:
        DataManager() {}

        
    public:
        void init();
        void setup_eeprom();

        void update_rad_buffer();

        void save_dose();
        void save_pwm();
        void save_tone();
        void save_bl();
        void save_contrast();
        void save_time();
        void save_error();

        void read_eeprom();

        void reset_dose();
        void reset_settings();

        void reset_activity_test();

        void calc_std();
        void get_quantile();

        /*Оптимизировать переменные, т.к. занимают слишком много оперативной памяти
        */

        byte geiger_error = 5;
        float tinv_value = 0;
        byte GEIGER_TIME = 37;
        byte contrast = 60;
        bool backlight = 0;
        byte pwm_converter = 45;
        byte ton_BUZZ = 20; //тональность буззера

        //-----------------------Флаги-----------------------
        bool detected = false;              //флаг обнаружения частицы
        bool is_sleeping = false;           //флаг сна
        bool editing_mode = false;          //флаг редактирования
        bool saved = false;                 //флаг сохранения
        bool stop_timer = false;
        bool next_step = false;             //флаг для замера
        bool alarm = false;

        //-----------------------Всё что связано с замером-----------------------
        uint16_t *rad_buff; //массив секундных замеров для расчета фона
        uint32_t rad_sum, rad_back, rad_max, rad_dose, rad_dose_old; //сумма импульсов за все время/текущий фон/максимум фона/доза/предыдущая доза
        uint8_t time_min_old, time_min, time_sec; //счетчики времени
        uint16_t timer_time, timer_remain;

        unsigned long alarm_timer = 0;

        //display
        byte cursor = 0;
        byte page = 0;
        byte menu_page = 0;
        byte counter_mode = 0;
        byte editable = 0;

        float battery_voltage = 0;
        //display//

        byte has_eeprom = 1;

        //-----------------------Измерение погрешности для статистики-----------------------
        float mean = 0;  //Математическое ожидание
        float std = 0;    //Среднеквадратичное отклонение

};