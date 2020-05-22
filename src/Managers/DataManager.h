#pragma once
#include <Arduino.h>

#define TIMER1_PRELOAD 64910 //65535-64910=625, 15625/625=25Гц
#define HVGEN_FACT 5 // 25/5=5Гц частота подкачки преобразователя
#define TIME_FACT 25 // 25Гц/25=1Гц секундные интервалы

#define TARGET_VOLTAGE 400				//Требуемое напряжение
#define DIVIDER 883						//Значение ацп делителя напряжения

class DataManager{
    public:
        void init();
        void setup_eeprom();

        void update_rad_buffer();

        void save_voltage_config();
        void save_geiger_time_config();
        void save_tone_delay();

        byte GEIGER_TIME = 37;
        bool detected = false;

        const uint8_t avgFactor = 5;
        int sensorValue = 0;

        uint16_t *rad_buff;// = new uint16_t[GEIGER_TIME]; //массив секундных замеров для расчета фона
        uint32_t rad_sum; //сумма импульсов за все время
        uint32_t rad_back; //текущий фон
        uint32_t rad_max; //максимум фона
        uint32_t rad_dose; //доза
        uint8_t time_sec; //секунды //счетчики времени
        uint8_t time_min; //минуты
        uint8_t time_hrs; //часы

        uint8_t mode = 0; // режим выводимых на экран данных
        bool editing_mode = false;
        byte pwm_converter = 45;

        bool is_sleeping = false;

        byte ton_BUZZ = 200; //тональность буззера

        volatile byte wdt_counter;

};