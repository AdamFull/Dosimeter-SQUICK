#include <Managers/OutputManager.h>
#include <macros.h>

void OutputManager::init(){
    display.begin();      
    display.setContrast(datamgr->contrast); //Set contrast to 50
    display.clearDisplay(); 
    display.display();  
}

void OutputManager::update(){
    new_update:
    if(datamgr->redraw_required){
        switch (datamgr->page){
            case 0: draw_logo(); datamgr->page = 1; break;
            case 1: draw_main(); break;
            case 2: draw_menu(); break;
        }
        datamgr->redraw_required = false;
    }else{
        if(1){
            goto new_update;
        }
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

}

void OutputManager::draw_main(){

}

void OutputManager::draw_menu(){
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(15, 0);
    display.print(current_page_name[datamgr->menu_page]);
    display.drawFastHLine(0,10,83,BLACK);
    display.setCursor(0, 15);

    switch (datamgr->menu_page){
        case 0:{
            if (datamgr->cursor==0) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">Mode");
            display.setCursor(0, 25);

            if (datamgr->cursor==1) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);  
            display.print(">Settings");
            display.setCursor(0, 35);
    
            if (datamgr->cursor==2) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">Reset");
            display.setCursor(0, 45);

            if (datamgr->cursor==3) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">Power off");
        }break;

        case 1:{
            if (datamgr->cursor==0) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">Background");
            display.setCursor(0, 25);

            if (datamgr->cursor==1) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);  
            display.print(">Beta");
            display.setCursor(0, 35);
    
            if (datamgr->cursor==2) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">Gamma");
        }break;

        case 2:{
            if (datamgr->cursor==0) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">Out voltage: ");
            display.setCursor(0, 25);

            if (datamgr->cursor==1) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);  
            display.print(">Counting time: ");
            display.setCursor(0, 35);
    
            if (datamgr->cursor==2) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">Beep tone: ");
            display.setCursor(0, 45);

            if (datamgr->cursor==3) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">Backlight: ");
        }break;

        case 3:{
            if (datamgr->cursor==0) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">Settings");
            display.setCursor(0, 25);

            if (datamgr->cursor==1) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);  
            display.print(">Dose");
            display.setCursor(0, 35);
    
            if (datamgr->cursor==2) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">All");
            display.setCursor(0, 45);
        }break;

        case 4:{
            if (datamgr->cursor==0) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);
            display.print(">YES");
            display.setCursor(0, 25);

            if (datamgr->cursor==1) display.setTextColor(WHITE, BLACK);
            else display.setTextColor(BLACK, WHITE);  
            display.print(">NO");
            display.setCursor(0, 35);
        }break;

    }
    display.display();
}