#pragma once
#include <stdint.h>

typedef uint8_t byte;

#define TARGET_VOLTAGE 400				//Требуемое напряжение
#define DIVIDER 883						//Значение ацп делителя напряжения

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
        float get_battery_voltage();
        unsigned get_hv();

        static void pwm_PD3(byte pwm);
        static void pwm_PB3(byte pwm);

    private:
        int adc0_read();
        int adc1_read();

        const uint8_t avgFactor = 10;
        int sensorValue = 0;
};