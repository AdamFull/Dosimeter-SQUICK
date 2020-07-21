#pragma once
#include <Managers/DataManager.h>
#include <Managers/ADCManager.h>
#include <configuration.h>
#include <Adafruit_GFX_rus.h>
#include <Adafruit_PCD8544_rus.h>

class OutputManager{
    public:
        OutputManager(DataManager *datamgr, ADCManager *adcmgr){
            this->datamgr = datamgr;
            this->adcmgr = adcmgr;
        }
        void init();
        void update();
        void beep();
        void beep(uint16_t time, byte duration);
        inline void update_request() { update_required = true; }
        inline void battery_request(bool val) { show_battery = val; }
        inline bool get_battery_requet() { return show_battery; }

        inline void set_contrast(byte contrast) { display.setContrast(contrast); }
    
    private:
        void delayUs(byte dtime);

        int getNumOfDigits(uint32_t number){
            int digits=1; uint32_t num = number;
            while ((num/=10) > 0) digits++;
            return digits;
        }
        
        DataManager *datamgr;
        ADCManager *adcmgr;
        Adafruit_PCD8544 display = Adafruit_PCD8544(10, 9, 8, 7, 6);

        inline float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
            return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }

        bool update_required = true;
        bool show_battery = false;

        unsigned long voltage_update = 0;

        bool no_volt_flag = false;
        unsigned long no_volt_ticker = 0;

        unsigned long beep_timer = 0;

        void draw_logo();
        void draw_main();

        void draw_menu();
        void draw_bat_low();
};