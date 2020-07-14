#pragma once
#include <stdint.h>

typedef uint8_t byte;

#define TARGET_VOLTAGE 403				//Требуемое напряжение
#define DIVIDER 190						//Значение ацп делителя напряжения

class ADCManager{
    public:
        static ADCManager& getInstance()
        {
            static ADCManager instance; // Guaranteed to be destroyed.
            return instance;                // Instantiated on first use.
        }

    private:
        ADCManager() {}
    public:
        void adc_init();
        uint16_t get_battery_voltage();
        uint16_t get_hv();

        static void pwm_PD3(byte pwm);
        static void pwm_PB3(byte pwm);

    private:
        byte adc0_read();
        byte adc1_read();

        byte avgFactor = 5;
        uint16_t batValue = 0;
        uint16_t hvValue = 0;
};