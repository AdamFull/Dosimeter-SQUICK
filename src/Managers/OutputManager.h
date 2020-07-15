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
        void beep(byte time, byte duration);
        #if defined(CAN_SLEEP)
        void going_to_sleep();
        #else
        inline void going_to_sleep() {}
        #endif

        inline void set_contrast(byte contrast) { display.setContrast(contrast); }

        void do_alarm();
    
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

        unsigned long voltage_update = 0;

        void draw_logo();
        void draw_main();

        void draw_menu();
};