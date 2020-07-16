#pragma once
#include <Arduino.h>
#include <configuration.h>

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
        void save_tone();
        void save_bl();
        void save_contrast();
        

        #if defined(UNIVERSAL_COUNTER)
        void save_time();
        void save_error();
        void reset_settings();
        void setup_sbm20();
        void setup_sbm19();
        void setup_beta();
        void save_interval();
        #else
        inline void save_time() {}
        inline void save_error() {}
        inline void reset_settings() {}
        inline void setup_sbm20() {}
        inline void setup_sbm19() {}
        inline void setup_beta() {}
        inline void save_interval() {}
        #endif
        void read_eeprom();

        void reset_dose();
        

        void reset_activity_test();

        #if defined(ADVANCED_ERROR)
        void calc_std();
        void get_quantile();
        #else
        inline void calc_std() {}
        inline void get_quantile() {}
        #endif

        #if defined(ADVANCED_ERROR)
        uint32_t *stat_buff;
        byte stat_time = 0;
        #endif
        #if defined(UNIVERSAL_COUNTER)
        byte geiger_error = 5;
        byte GEIGER_TIME = 37;
        byte pwm_converter = 40;
        #else
        static const byte pwm_converter = 45;
        static const byte geiger_error = 5;
        #endif
        byte contrast = 60;
        byte backlight = 0;
        byte ton_BUZZ = 20; //тональность буззера

        uint8_t save_dose_interval = 20;

        //-----------------------Флаги-----------------------
        bool is_sleeping = false;           //флаг сна
        bool editing_mode = false;          //флаг редактирования
        bool stop_timer = false;
        bool next_step = false;             //флаг для замера
        bool alarm = false;
        bool is_charging = false;
        bool is_charged = false;
        bool is_detected = false;

        //-----------------------Всё что связано с замером-----------------------
        #if defined(UNIVERSAL_COUNTER)
        uint16_t *rad_buff; //массив секундных замеров для расчета фона
        #else
        uint16_t rad_buff[GEIGER_TIME];
        #endif
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
        bool means_times = 0;

        bool mean_mode = false;

        #if defined(DRAW_GRAPH)
        byte mass[84];
        byte x_p = 0;
        #endif

        uint16_t battery_voltage = 0;
        //display//

        byte has_eeprom = 1;

        //-----------------------Измерение погрешности для статистики-----------------------
        #if defined(ADVANCED_ERROR)
        float mean = 0;  //Математическое ожидание
        float std = 0;    //Среднеквадратичное отклонение
        #endif

};