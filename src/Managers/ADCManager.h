#pragma once
#include <Arduino.h>

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

    private:
        int adc0_read();
        int adc1_read();

        const uint8_t avgFactor = 5;
        int sensorValue = 0;
};