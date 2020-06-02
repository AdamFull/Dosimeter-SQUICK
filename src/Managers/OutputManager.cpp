#include <Managers/OutputManager.h>
#include <macros.h>
#include <language.h>

void OutputManager::init(){
    display.begin();      
    display.setContrast(datamgr->contrast);
    display.display(); // show splashscreen
    display.clearDisplay();
    adcmgr.adc_init();
    for(int i = 0; i < 10; i++) datamgr->battery_voltage = adcmgr.get_battery_voltage();
}

void OutputManager::update(){
    switch (datamgr->page){
        case 0: draw_logo(); datamgr->page = 1; break;
        case 1: draw_main(); break;
        case 2: draw_menu(); break;
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

void OutputManager::beep(byte time, byte duration) { //индикация каждой частички звуком светом
    int d = duration;
	PORTB_WRITE(5, 1);
    while (d > 0) {
      	PORTD_WRITE(5, 1);
      	delayUs(time);
      	PORTD_WRITE(5, 0);
      	delayUs(time);
	  	asm("nop");
      	d--;
    }
	PORTB_WRITE(5, 0);
}

void OutputManager::do_alarm(){
    if(millis()-datamgr->alarm_timer > 500){
        datamgr->alarm_timer = millis();
        datamgr->detected = true;
    }
}

void OutputManager::draw_logo(){
    #if defined(SHOW_LOGO)
    display.clearDisplay();
    display.drawBitmap(0, 0, logo_Bitmap, 84, 48, BLACK);
	display.display();
    delay(LOGO_TIME);
    #endif
}

void OutputManager::draw_main(){
    display.clearDisplay();
    if(millis()-voltage_update > 30000){
        voltage_update = millis();
        datamgr->battery_voltage = adcmgr.get_battery_voltage();
        Serial.println(datamgr->battery_voltage);
    }
    int coeff = mapfloat(datamgr->battery_voltage, 3.0, 4.2, 0, 12);             //Значение сдвига пикселей для визуализации заряда аккумулятора

    display.drawBitmap(69, 0, battery_Bitmap, 15, 7, BLACK);
    display.fillRect(83-coeff, 1, 12, 5, BLACK);

    if(datamgr->counter_mode == 0){
        display.setTextColor(BLACK, WHITE);
        display.setCursor(0, 0);
        display.print(BACKGROUND);
        #if defined(ADVANCED_ERROR)
        uint16_t deviation = map(100-(datamgr->mean/(datamgr->mean+datamgr->std))*100, 0, 100, datamgr->geiger_error, 100);
        #else
        uint16_t deviation = 100;
        #endif
        if(deviation > 100) deviation = 100;
        display.setCursor(84 - (getNumOfDigits(deviation)+2)*6, 13);
        display.write(240);
        display.print(deviation);
        display.print("%");
        display.setTextSize(2);
        display.setCursor(0, 8);
        if(datamgr->rad_back > 1000) display.print((float)datamgr->rad_back/1000, getNumOfDigits(datamgr->rad_back/1000) > 2 ? 0 : 2);
        else if(datamgr->rad_back > 1000000) display.print((float)datamgr->rad_back/1000000, getNumOfDigits(datamgr->rad_back/1000000) > 2 ? 0 : 2);
        else display.print(datamgr->rad_back);
        display.setTextSize(0);
        display.setCursor(0, 23);
        if(datamgr->rad_back > 1000) display.print(T_MRH);
        else if(datamgr->rad_back > 1000000) display.print(T_RH);
        else display.print(T_URH);
        display.setCursor(84 - (getNumOfDigits(datamgr->rad_dose)+2)*6, 23);
        if(datamgr->rad_dose > 1000) display.print((float)datamgr->rad_dose/1000);
        else if(datamgr->rad_dose > 1000000) display.print((float)datamgr->rad_dose/1000000);
        else display.print(datamgr->rad_dose);
        if(datamgr->rad_dose > 1000) display.print(T_MR);
        else if(datamgr->rad_dose > 1000000) display.print(T_R);
        else display.print(T_UR);
        display.drawFastHLine(0,32,84,BLACK);
        
        #if defined(DRAW_GRAPH)
        for(byte i=0;i<83;i++){
            display.drawLine(i,47,i, 47-datamgr->mass[i], BLACK);
        }
        #endif
    }else if(datamgr->counter_mode == 1){
        display.setTextColor(BLACK, WHITE);
        display.setCursor(84/2 - 3*6, 0);
        if(!datamgr->next_step) display.print(BACKGROUND);
        else display.print(SAMPLE);
        display.setTextSize(2);
        display.setCursor(0, 8);
        if(datamgr->stop_timer && datamgr->next_step) display.print(abs((int)datamgr->rad_max - (int)datamgr->rad_back));
        else display.print(datamgr->rad_back);
        display.setTextSize(0);
        display.setCursor(84/2 - (getNumOfDigits(datamgr->time_min)+getNumOfDigits(datamgr->time_sec)+1)*3, 23);
        display.print(datamgr->time_min);
        display.print(":");
        display.print(datamgr->time_sec);
        display.drawFastHLine(0,32,84,BLACK);
        display.fillRect(0, 34, map(datamgr->timer_remain, datamgr->timer_time, 0, 0, 84), 12, BLACK);
        display.drawFastHLine(0,47,84,BLACK);
        display.setTextColor(WHITE, BLACK);
        display.setCursor(15, 36);
        if(datamgr->stop_timer && !datamgr->next_step) display.print(PRESSSET);
        display.setCursor(25, 36);
        if(datamgr->stop_timer && datamgr->next_step) display.print(SUCCESS);
    }else if(datamgr->counter_mode == 2){
        display.setTextColor(BLACK, WHITE);
        display.setCursor(0, 0);
        display.print(MODE_SEC);
        display.setTextColor(BLACK, WHITE);
        display.setTextSize(2);
        display.setCursor(0, 8);
        display.print(datamgr->rad_buff[0]);
        display.setTextSize(0);
        display.setCursor(0, 23);
        display.print(T_CPS);
        display.drawFastHLine(0,32,84,BLACK);

        #if defined(DRAW_GRAPH)
        for(byte i=0;i<83;i++){
            display.drawLine(i,47,i, 47-datamgr->mass[i], BLACK);
        }
        #endif
    }
    display.display();
}

void OutputManager::draw_menu(){
    display.setTextSize(0);
    const char *page_name = current_page_name[datamgr->menu_page];
    #if defined(RUS)
        size_t header_size = strlen(page_name)/1.5;
    #else
        size_t header_size = strlen(page_name);
    #endif
    display.clearDisplay();
    display.setTextColor(WHITE, BLACK);
    display.fillRect(0, 0, 84, 8, BLACK);
    display.setCursor((14 - header_size)*6/2, 0);
    display.print(page_name);
    display.drawFastHLine(0,8,83,BLACK);
    display.setCursor(0, 10);
    display.setTextColor(BLACK, WHITE);

    switch (datamgr->menu_page){
        case 0:{
            if (datamgr->cursor==0) display.print(T_CURSOR);
            display.print(MODE);
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(T_CURSOR);
            display.print(SETTINGS);
            display.setCursor(0, 30);
    
            if (datamgr->cursor==2) display.print(T_CURSOR);
            display.print(RESET);
            display.setCursor(0, 40);

            #if defined(CAN_SLEEP)
            if (datamgr->cursor==3) display.print(T_CURSOR);
            display.print(POFF);
            #endif
        }break;

        case 1:{
            if (datamgr->cursor==0) display.print(T_CURSOR);
            display.print(BACKGROUND);
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(T_CURSOR);
            display.print(ACTIVITY);
            display.setCursor(0, 30);

            if (datamgr->cursor==2) display.print(T_CURSOR);
            display.print(MODE_SEC);
        }break;

        case 2:{
            #if defined(UNIVERSAL_COUNTER)
            if (datamgr->cursor==0) display.print(T_CURSOR);
            display.print(GCOUNTER);
            display.setCursor(0, 20);
        
            if (datamgr->cursor==1) display.print(T_CURSOR);
            display.print(TONE);
            if(datamgr->cursor==1 && datamgr->editing_mode){
                display.setCursor(84 - getNumOfDigits(datamgr->editable)*6, 20);
                display.setTextColor(WHITE, BLACK);
                display.print(datamgr->editable);
            }else{
                display.setCursor(84 - getNumOfDigits(datamgr->ton_BUZZ)*6, 20);
                display.print(datamgr->ton_BUZZ);
            }
            display.setCursor(0, 30);
            display.setTextColor(BLACK, WHITE);    

            if (datamgr->cursor==2) display.print(T_CURSOR);
            display.print(BACKLIGHT);
            if(datamgr->cursor==2 && datamgr->editing_mode){
                display.setCursor(84 - getNumOfDigits(datamgr->editable)*6, 30);
                display.setTextColor(WHITE, BLACK);
                display.print(datamgr->editable);
            }else{
                display.setCursor(84 - getNumOfDigits(datamgr->backlight)*6, 30);
                display.print(datamgr->backlight);
            }
            display.setCursor(0, 40);
            display.setTextColor(BLACK, WHITE);

            if (datamgr->cursor==3) display.print(T_CURSOR);
            display.print(CONTRAST);
            if(datamgr->cursor==3 && datamgr->editing_mode){
                display.setCursor(84 - getNumOfDigits(datamgr->editable)*6, 40);
                display.setTextColor(WHITE, BLACK);
                display.print(datamgr->editable);
            }else{
                display.setCursor(84 - getNumOfDigits(datamgr->contrast)*6, 40);
                display.print(datamgr->contrast);
            }
            #else
            if (datamgr->cursor==0) display.print(T_CURSOR);
            display.print(TONE);
            if(datamgr->cursor==0 && datamgr->editing_mode){
                display.setCursor(84 - getNumOfDigits(datamgr->editable)*6, 10);
                display.setTextColor(WHITE, BLACK);
                display.print(datamgr->editable);
            }else{
                display.setCursor(84 - getNumOfDigits(datamgr->ton_BUZZ)*6, 10);
                display.print(datamgr->ton_BUZZ);
            }
            display.setCursor(0, 20);
            display.setTextColor(BLACK, WHITE);    

            if (datamgr->cursor==1) display.print(T_CURSOR);
            display.print(BACKLIGHT);
            if(datamgr->cursor==1 && datamgr->editing_mode){
                display.setCursor(84 - getNumOfDigits(datamgr->editable)*6, 20);
                display.setTextColor(WHITE, BLACK);
                display.print(datamgr->editable);
            }else{
                display.setCursor(84 - getNumOfDigits(datamgr->backlight)*6, 20);
                display.print(datamgr->backlight);
            }
            display.setCursor(0, 30);
            display.setTextColor(BLACK, WHITE);

            if (datamgr->cursor==2) display.print(T_CURSOR);
            display.print(CONTRAST);
            if(datamgr->cursor==2 && datamgr->editing_mode){
                display.setCursor(84 - getNumOfDigits(datamgr->editable)*6, 30);
                display.setTextColor(WHITE, BLACK);
                display.print(datamgr->editable);
            }else{
                display.setCursor(84 - getNumOfDigits(datamgr->contrast)*6, 30);
                display.print(datamgr->contrast);
            }
            #endif
            display.setTextColor(BLACK, WHITE);
        }break;

        case 3:{
            if (datamgr->cursor==0) display.print(T_CURSOR);
            display.print(SETTINGS);
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(T_CURSOR);
            display.print(DOSE);
            display.setCursor(0, 30);
    
            if (datamgr->cursor==2) display.print(T_CURSOR);
            display.print(ALL);
            display.setCursor(0, 40);
        }break;

        case 4:{
            if (datamgr->cursor==0) display.print(T_CURSOR);
            display.print(TIME);
            if(datamgr->cursor==0 && datamgr->editing_mode){
                display.setCursor(84 - (getNumOfDigits(datamgr->editable)+1)*6, 10);
                display.setTextColor(WHITE, BLACK);
                display.print(datamgr->editable);
            }else{
                display.setCursor(84 - (getNumOfDigits(datamgr->time_min)+1)*6, 10);
                display.print(datamgr->time_min);
            }
            display.print("m");
            display.setTextColor(BLACK, WHITE);
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(T_CURSOR);
            display.print(MEANS);
            display.setCursor(84 - 6, 20);
            if(datamgr->cursor==1 && datamgr->editing_mode){
                display.setTextColor(WHITE, BLACK);
                display.print(datamgr->editable == 0 ? 1 : 2);
            }else{
                display.print(datamgr->means_times == 0 ? 1 : 2);
            }
            display.setTextColor(BLACK, WHITE);
            display.setCursor(25, 30);

            if (datamgr->cursor==2) display.setTextColor(WHITE, BLACK);
            display.print(BEGIN);
            display.setTextColor(BLACK, WHITE);
        }break;

        #if defined(CAN_SLEEP)
        case 5:{
            if (datamgr->cursor==0) display.print(T_CURSOR);
            display.print(YES);
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(T_CURSOR);
            display.print(NO);
            display.setCursor(0, 30);
        }break;
        #endif

        #if defined(UNIVERSAL_COUNTER)
        case 6:{                            //Geiger counter select
            if (datamgr->cursor==0) display.print(T_CURSOR);
            display.print(G_SBM20);
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(T_CURSOR);
            display.print(G_SBM19);
            display.setCursor(0, 30);
    
            if (datamgr->cursor==2) display.print(T_CURSOR);
            display.print(G_BETA);
            display.setCursor(0, 40);

            if (datamgr->cursor==3) display.print(T_CURSOR);
            display.print(CUSTOM);
        }break;

        case 7:{                            //Geiger counter custom
            unsigned int hvoltage = adcmgr.get_hv();
            if (datamgr->cursor==0) display.print(T_CURSOR);
            display.print(VOLTAGE);
            if(datamgr->cursor==0 && datamgr->editing_mode) display.setTextColor(WHITE, BLACK);
            display.setCursor(84 - (getNumOfDigits(hvoltage)+1)*6, 10);
            display.print(hvoltage);
            display.print("V");
            display.setTextColor(BLACK, WHITE);
            display.setCursor(0, 20);

            if (datamgr->cursor==1) display.print(T_CURSOR);
            display.print(GTIME);
            if(datamgr->cursor==1 && datamgr->editing_mode){
                display.setCursor(84 - (getNumOfDigits(datamgr->editable)+1)*6, 20);
                display.setTextColor(WHITE, BLACK);
                display.print(datamgr->editable);
            }else{
                display.setCursor(84 - (getNumOfDigits(datamgr->GEIGER_TIME)+1)*6, 20);
                display.print(datamgr->GEIGER_TIME);
            }
            display.print("s");
            display.setCursor(0, 30);
            display.setTextColor(BLACK, WHITE);

            if (datamgr->cursor==2) display.print(T_CURSOR);
            display.print(ERROR);
            if(datamgr->cursor==2 && datamgr->editing_mode){
                display.setCursor(84 - (getNumOfDigits(datamgr->editable)+2)*6, 30);
                display.setTextColor(WHITE, BLACK);
                display.write(240);
                display.print(datamgr->editable, 1);
            }else{
                display.setCursor(84 - (getNumOfDigits(datamgr->geiger_error)+2)*6, 30);
                display.write(240);
                display.print(datamgr->geiger_error, 1);
            }
            display.print("%");
            display.setCursor(0, 40);
            display.setTextColor(BLACK, WHITE);
        }break;
        #endif
    }
    display.display();
}

#if defined(CAN_SLEEP)
void OutputManager::going_to_sleep(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(POFF);
    display.display();
    delay(1000);
    display.clearDisplay();
    display.display();
}
#endif