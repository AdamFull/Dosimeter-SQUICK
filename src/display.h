#pragma once
#include <Arduino.h>

class display{
    public:
        static inline void display_loop(uint8_t display_mode){
            switch (display_mode)
            {
                case 0: draw_logo(); break;
                case 1: draw_main(); break;
                case 2: draw_menu(); break;
            }
        }
    
    private:
        static inline void draw_logo(){}
        static inline void draw_main(){}
        static inline void draw_menu(){
            int page;
            int cursor;
            switch (page)
            {
                case 0: break;      //Надпись меню, пункты: режим, Настройки, сброс, выключить, назад
                case 1: break;      //Надпись режим, пункты: фон, бетта, гамма, назад
                case 2: break;      //Надпись настройки, пункты: напряжение, время счёта, тональность, подсветка, назад
                case 3: break;      //Надпись сброс, пункты: настройки, доза, всё, назад
            }
        }
};
extern display ui;