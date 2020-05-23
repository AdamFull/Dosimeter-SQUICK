#pragma once
#include <Managers/DataManager.h>
#include <Managers/ADCManager.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define MENU_PAGES 5

class OutputManager{
    public:
        OutputManager(DataManager *datamgr){
            this->datamgr = datamgr;
        }
        void init();
        void update();
        void beep();

        inline void update_request(){ datamgr->redraw_required = true; }

        inline void set_contrast(byte contrast) { display.setContrast(contrast); }
    
    private:
        void delayUs(byte dtime);
        
        DataManager *datamgr;
        ADCManager *adcmgr = new ADCManager();
        Adafruit_PCD8544 display = Adafruit_PCD8544(10, 9, 8, 7, 6);

        //Нужна ссылка на data_manager - класс который хранит всю инфу, как о дисплее, так и о текущих значениях. Так же должен содержать все методы сохранения.

        const char current_page_name[MENU_PAGES][13] = {"MAIN MENU", "MODE", "SETTINGS", "RESET", "ARE YOU SURE"};

        void draw_logo();
        void draw_main();

        void draw_menu();
};