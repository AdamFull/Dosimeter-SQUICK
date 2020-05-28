#pragma once
#include <Managers/DataManager.h>
#include <Managers/ADCManager.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define MENU_PAGES 6

#define min_rad 0

class OutputManager{
    public:
        OutputManager(DataManager *datamgr){
            this->datamgr = datamgr;
            for(int i = 0; i < 83; i++) mass[i] = 0;
        }
        void init();
        void update();
        void beep();

        inline void update_request(){ datamgr->redraw_required = true; }

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
        ADCManager &adcmgr = ADCManager::getInstance();
        Adafruit_PCD8544 display = Adafruit_PCD8544(10, 9, 8, 7, 6);

        //Нужна ссылка на data_manager - класс который хранит всю инфу, как о дисплее, так и о текущих значениях. Так же должен содержать все методы сохранения.

        const char current_page_name[MENU_PAGES][10] = {"MAIN MENU", "MODE", "SETTINGS", "RESET", "SURE?", "ACTIVITY"};

        inline int mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
            return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }

        unsigned long timing = 0, tik_press = 0, voltage_update = 0;
        byte mass[84];

        byte x_p = 0;

        void draw_logo();
        void draw_main();

        void draw_menu();
};