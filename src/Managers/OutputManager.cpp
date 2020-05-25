#include <Managers/OutputManager.h>
#include <macros.h>
#include <logo.h>

void OutputManager::init(){
    display.begin();      
    display.setContrast(datamgr->contrast);
    display.display(); // show splashscreen
    display.clearDisplay();
    adcmgr->adc_init();
}

void OutputManager::update(){
    if(1/*datamgr->redraw_required*/){
        switch (datamgr->page){
            case 0: draw_logo(); datamgr->page = 1; break;
            case 1: draw_main(); break;
            case 2: draw_menu(); break;
        }
        datamgr->redraw_required = false;
    }
    beep();
}

void OutputManager::delayUs(byte dtime){
	for(int i = 0; i < dtime; i++){
		_delay_us(1);
	}
}

void OutputManager::beep() { //индикация каждой частички звуком светом
	if(datamgr->detected){
		datamgr->detected = false;
    	int d = 30;
		PORTB_WRITE(5, 1);
    	while (d > 0) {
      		PORTD_WRITE(5, 1);
      		delayUs(datamgr->ton_BUZZ);
      		PORTD_WRITE(5, 0);
      		delayUs(datamgr->ton_BUZZ);
	  		asm("nop");
      		d--;
    	}
		PORTB_WRITE(5, 0);
	}
}

void OutputManager::draw_logo(){
    display.clearDisplay();
    display.drawBitmap(0, 0, logo_Bitmap, 84, 48, BLACK);
	display.display();
    delay(2000);
}

void OutputManager::draw_main(){
    display.clearDisplay();
    display.drawBitmap(69, 0, battery_Bitmap, 15, 7, BLACK);
    display.setTextSize(2);
    display.setCursor(0, 8);
    if(datamgr->rad_back > 1000) display.print(datamgr->rad_back/1000);
    else if(datamgr->rad_back > 1000000) display.print(datamgr->rad_back/1000000);
    else display.print(datamgr->rad_back);
    display.setTextSize(0);
    display.setCursor(0, 23);
    if(datamgr->rad_back > 1000) display.print("mR/h");
    else if(datamgr->rad_back > 1000000) display.print("R/h");
    else display.print("uR/h");
    display.drawFastHLine(0,33,84,BLACK);
    display.setCursor(0, 43);
    display.print("Привет мир");

    display.display();
}

void OutputManager::draw_menu(){
    display.setTextSize(0);
    display.clearDisplay();
    display.setTextColor(WHITE, BLACK);
    display.setCursor(15, 0);
    display.print(current_page_name[datamgr->menu_page]);
    //display.drawFastHLine(0,10,83,BLACK);
    display.setCursor(0, 10);
    display.setTextColor(BLACK, WHITE);

    switch (datamgr->menu_page){
        case 0:{
            if (datamgr->cursor==0) display.print(">");
            display.print("Mode");
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(">");
            display.print("Settings");
            display.setCursor(0, 30);
    
            if (datamgr->cursor==2) display.print(">");
            display.print("Reset");
            display.setCursor(0, 40);

            if (datamgr->cursor==3) display.print(">");
            display.print("Power off");
        }break;

        case 1:{
            if (datamgr->cursor==0) display.print(">");
            display.print("Background");
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(">");
            display.print("Beta");
            display.setCursor(0, 30);
    
            if (datamgr->cursor==2) display.print(">");
            display.print("Gamma");
        }break;

        case 2:{
            if (datamgr->cursor==0) display.print(">");
            display.print("Out HV:");
            if(datamgr->cursor==0 && datamgr->editing_mode) display.setTextColor(WHITE, BLACK);
            display.print(adcmgr->get_hv());
            display.print("V");
            display.setTextColor(BLACK, WHITE);
            display.setCursor(0, 20);
            display.setTextColor(BLACK, WHITE);

            if (datamgr->cursor==1) display.print(">");
            display.print("Cnt time:");
            if(datamgr->cursor==1 && datamgr->editing_mode) display.setTextColor(WHITE, BLACK);
            display.print(datamgr->GEIGER_TIME);
            display.print("s");
            display.setCursor(0, 30);
            display.setTextColor(BLACK, WHITE);
    
            if (datamgr->cursor==2) display.print(">");
            display.print("Beep tone:");
            if(datamgr->cursor==2 && datamgr->editing_mode) display.setTextColor(WHITE, BLACK);
            display.print(datamgr->ton_BUZZ);
            display.setCursor(0, 40);
            display.setTextColor(BLACK, WHITE);

            if (datamgr->cursor==3) display.print(">");
            display.print("Backlight:");
            if(datamgr->cursor==3 && datamgr->editing_mode) display.setTextColor(WHITE, BLACK);
            display.print(datamgr->backlight);
            display.setTextColor(BLACK, WHITE);
        }break;

        case 3:{
            if (datamgr->cursor==0) display.print(">");
            display.print("Settings");
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(">");
            display.print("Dose");
            display.setCursor(0, 30);
    
            if (datamgr->cursor==2) display.print(">");
            display.print("All");
            display.setCursor(0, 40);
        }break;

        case 4:{
            if (datamgr->cursor==0) display.print(">");
            display.print("YES");
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(">");
            display.print("NO");
            display.setCursor(0, 30);
        }break;

    }
    display.display();
}