#pragma once
#include <stdint.h>
#include <configuration.h>

typedef uint8_t byte;

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
        static void pwm_PD5(byte pwm);

    private:
        byte adc0_read();
        byte adc1_read();

        uint16_t battery_bank[BAT_BANK_SIZE];
        bool first_mean = true;

        byte avgFactor = 5;
        uint16_t batValue = 0;
        uint16_t hvValue = 0;
};