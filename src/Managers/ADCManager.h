#pragma once
#include <Arduino.h>

#define TARGET_VOLTAGE 400				//Требуемое напряжение
#define DIVIDER 883						//Значение ацп делителя напряжения

class ADCManager{
    public:
        ADCManager() {}
        void adc_init();
        float get_battery_voltage();
        unsigned voltage_config();

    private:
        int adc0_read();
        int adc1_read();

        const uint8_t avgFactor = 5;
        int sensorValue = 0;
};