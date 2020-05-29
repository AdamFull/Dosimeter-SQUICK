#include <Managers/OutputManager.h>
#include <macros.h>
#include <logo.h>

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
		PORTB_WRITE(5, 1);
        tone(5, datamgr->ton_BUZZ*10, 5);
		PORTB_WRITE(5, 0);
	}
}

void OutputManager::do_alarm(){
    if(millis()-datamgr->alarm_timer > 500){
        datamgr->alarm_timer = millis();
        tone(5, 3000, 300);
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
    if(millis()-voltage_update > 30000){
        voltage_update = millis();
        datamgr->battery_voltage = adcmgr.get_battery_voltage();
        Serial.println(datamgr->battery_voltage);
    }
    int coeff = mapfloat(datamgr->battery_voltage, 3.0, 4.2, 0, 12);             //Значение сдвига пикселей для визуализации заряда аккумулятора

    display.drawBitmap(69, 0, battery_Bitmap, 15, 7, BLACK);
    display.fillRect(83-coeff, 1, 12, 5, BLACK);

    switch (datamgr->counter_mode){
        case 0:{
            uint16_t deviation = 100*(datamgr->tinv_value*datamgr->std/sqrt(datamgr->GEIGER_TIME))/datamgr->mean;
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
            if(datamgr->rad_back > 1000) display.print("mR/h");
            else if(datamgr->rad_back > 1000000) display.print("R/h");
            else display.print("uR/h");
            display.setCursor(84 - (getNumOfDigits(datamgr->rad_dose)+2)*6, 23);
            if(datamgr->rad_dose > 1000) display.print((float)datamgr->rad_dose/1000);
            else if(datamgr->rad_dose > 1000000) display.print((float)datamgr->rad_dose/1000000);
            else display.print(datamgr->rad_dose);
            if(datamgr->rad_dose > 1000) display.print("mR");
            else if(datamgr->rad_dose > 1000000) display.print("R");
            else display.print("uR");
            display.drawFastHLine(0,32,84,BLACK);
            
            for(byte i=0;i<83;i++){
                display.drawLine(i,47,i, 47-mass[i], BLACK);
            }
            if(millis()-tik_press>1000){
                tik_press=millis();
                mass[x_p]=map(datamgr->rad_back, 0, datamgr->rad_max < 40 ? 40 : datamgr->rad_max, 0, 15);
                if(x_p<83)x_p++;
                if(x_p==83){
                    for(byte i=0;i<83;i++)mass[i]=mass[i+1];
                }
            }
        }break;
        case 1:{
            display.setTextColor(BLACK, WHITE);
            display.setCursor(0, 0);
            if(!datamgr->next_step) display.print("back");
            else display.print("sample");
            display.setTextSize(2);
            display.setCursor(0, 8);
            if(datamgr->stop_timer && datamgr->next_step) display.print(abs((int)datamgr->rad_max - (int)datamgr->rad_back));
            else display.print(datamgr->rad_back);
            display.setTextSize(0);
            display.setCursor(30, 23);
            display.print(datamgr->time_min);
            display.print(":");
            display.print(datamgr->time_sec);
            display.drawFastHLine(0,32,84,BLACK);
            display.fillRect(0, 34, map(datamgr->timer_remain, datamgr->timer_time, 0, 0, 84), 12, BLACK);
            display.drawFastHLine(0,47,84,BLACK);
            display.setTextColor(WHITE, BLACK);
            display.setCursor(15, 36);
            if(datamgr->stop_timer && !datamgr->next_step) display.print("Press set");
            display.setCursor(20, 36);
            if(datamgr->stop_timer && datamgr->next_step) display.print("Success");

        }break;
    }
    display.display();
}

void OutputManager::draw_menu(){
    display.setTextSize(0);
    const char *page_name = current_page_name[datamgr->menu_page];
    size_t header_size = strlen(page_name);
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
            display.print("Activity");
        }break;

        case 2:{
            if(datamgr->cursor < 4){
                unsigned int hvoltage = adcmgr.get_hv();
                if (datamgr->cursor==0) display.print(">");
                display.print("Out HV:");
                if(datamgr->cursor==0 && datamgr->editing_mode) display.setTextColor(WHITE, BLACK);
                display.setCursor(84 - (getNumOfDigits(hvoltage)+1)*6, 10);
                display.print(hvoltage);
                display.print("V");
                display.setTextColor(BLACK, WHITE);
                display.setCursor(0, 20);

                if (datamgr->cursor==1) display.print(">");
                display.print("Cnt time:");
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

                if (datamgr->cursor==2) display.print(">");
                display.print("Error:");
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
        
                if (datamgr->cursor==3) display.print(">");
                display.print("Beep tone:");
                if(datamgr->cursor==3 && datamgr->editing_mode){
                    display.setCursor(84 - getNumOfDigits(datamgr->editable)*6, 40);
                    display.setTextColor(WHITE, BLACK);
                    display.print(datamgr->editable);
                }else{
                    display.setCursor(84 - getNumOfDigits(datamgr->ton_BUZZ)*6, 40);
                    display.print(datamgr->ton_BUZZ);
                }
                display.setTextColor(BLACK, WHITE);    
            }else{
                if (datamgr->cursor==4) display.print(">");
                display.print("Backlight:");
                if(datamgr->cursor==4 && datamgr->editing_mode){
                    display.setCursor(84 - getNumOfDigits(datamgr->editable)*6, 10);
                    display.setTextColor(WHITE, BLACK);
                    display.print(datamgr->editable);
                }else{
                    display.setCursor(84 - getNumOfDigits(datamgr->backlight)*6, 10);
                    display.print(datamgr->backlight);
                }
                display.setCursor(0, 20);
                display.setTextColor(BLACK, WHITE);

                if (datamgr->cursor==5) display.print(">");
                display.print("Contrast:");
                if(datamgr->cursor==5 && datamgr->editing_mode){
                    display.setCursor(84 - getNumOfDigits(datamgr->editable)*6, 20);
                    display.setTextColor(WHITE, BLACK);
                    display.print(datamgr->editable);
                }else{
                    display.setCursor(84 - getNumOfDigits(datamgr->contrast)*6, 20);
                    display.print(datamgr->contrast);
                }
                display.setTextColor(BLACK, WHITE);
            }
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

        case 5:{
            display.print("Time: ");
            display.setCursor(84 - (getNumOfDigits(datamgr->editable)+1)*6, 10);
            display.print(datamgr->editable);
            display.print("m");
        }break;
    }
    display.display();
}

void OutputManager::going_to_sleep(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("POWER OFF");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.display();
}