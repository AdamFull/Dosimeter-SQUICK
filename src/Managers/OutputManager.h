#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define MENU_PAGES 5

class OutputManager{
    public:
        void init();
        void update();
        void beep();
    
    private:
        void delayUs(byte dtime);
        
        Adafruit_PCD8544 display = Adafruit_PCD8544(10, 9, 8, 7, 6);

        //Нужна ссылка на data_manager - класс который хранит всю инфу, как о дисплее, так и о текущих значениях. Так же должен содержать все методы сохранения.

        byte cursor = 0;
        bool editing = false;
        bool redraw_required = true;

        int page = 0;
        int menu_page = 0;

        const char current_page_name[MENU_PAGES][13] = {"MAIN MENU", "MODE", "SETTINGS", "RESET", "ARE YOU SURE"};

        byte contrast = 50;
        byte backlight = 0;

        void draw_logo();
        void draw_main();

        void draw_menu();
};