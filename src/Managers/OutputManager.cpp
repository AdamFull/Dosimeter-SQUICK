#include <Managers/OutputManager.h>
#include <macros.h>
#include <language.h>

void OutputManager::init(){
    display.begin();      
    display.setContrast(datamgr->contrast);
    display.display(); // show splashscreen
    display.clearDisplay();
    adcmgr->adc_init();
    datamgr->battery_voltage = adcmgr->get_battery_voltage();
}

void OutputManager::update(){
    if(update_required){
        switch (datamgr->page){
            case 0: draw_logo(); datamgr->page = 1; break;
            case 1: draw_main(); break;
            case 2: draw_menu(); break;
            case 3: draw_bat_low(); break;
        }
        update_required = false;
    }
    beep();

    //Battery voltage update
    if(millis()-voltage_update > 10000){
        voltage_update = millis();
        uint16_t new_voltage = adcmgr->get_battery_voltage();
        if(!first_meaning){
            if(new_voltage < datamgr->battery_voltage - 10){datamgr->battery_voltage = datamgr->battery_voltage; }
            else{ datamgr->battery_voltage = new_voltage; }
            first_meaning = false;
        }else{
            datamgr->battery_voltage = new_voltage;
        }
    }
}

void OutputManager::delayUs(byte dtime){
	for(int i = 0; i < dtime; i++){
		_delay_us(1);
	}
}

void OutputManager::beep() { //индикация каждой частички звуком светом
    if(micros() - beep_timer > 300){
        beep_timer = micros();
        if(datamgr->is_detected){
            ADCManager::pwm_PD5(datamgr->ton_BUZZ);
            PORTB_WRITE(5, HIGH);
        }else{
            ADCManager::pwm_PD5(0);
            PORTB_WRITE(5, LOW);
        }
        datamgr->is_detected = false;
    }
}

void OutputManager::beep(uint16_t time, byte duration) { //индикация каждой частички звуком светом
    if(micros() - beep_timer > time){
        beep_timer = micros();
        if(datamgr->is_detected){
            ADCManager::pwm_PD5(duration);
            PORTB_WRITE(5, HIGH);
        }else{
            ADCManager::pwm_PD5(0);
            PORTB_WRITE(5, LOW);
        }
        datamgr->is_detected = false;
    }
}

void OutputManager::do_alarm(){
    if(millis()-datamgr->alarm_timer > 500){
        datamgr->alarm_timer = millis();
        beep(200, 30); _delay_ms(50); beep(100, 50);
    }
}

void OutputManager::draw_logo(){
    #if defined(SHOW_LOGO)
    display.clearDisplay();
    display.drawBitmap(0, 0, logo_Bitmap, 84, 48, BLACK);
	display.display();
    _delay_ms(LOGO_TIME);
    #endif
}

void OutputManager::draw_main(){
    display.clearDisplay();
    int coeff = mapfloat(datamgr->battery_voltage, BAT_ADC_MIN, BAT_ADC_MAX, 0, 12);             //Значение сдвига пикселей для визуализации заряда аккумулятора

    display.drawBitmap(69, 0, battery_Bitmap, 15, 7, BLACK);
    display.fillRect(83-coeff, 1, 12, 5, BLACK);
    if(datamgr->is_charging) display.drawBitmap(60, 0, charge_Bitmap, 5, 7, BLACK);

    if(datamgr->mean_mode) display.drawBitmap(0, 0, mean_Bitmap, 5, 7, BLACK);

    if(datamgr->is_charging){
        uint8_t progress = map(datamgr->battery_voltage, BAT_ADC_MIN, BAT_ADC_MAX, 0, 42);
        display.drawBitmap(17, 12, big_battery_Bitmap, 50, 24, BLACK);
        display.fillRect(19, 14, progress, 20, BLACK);
        display.setCursor(0, 0);
        display.print("V:");
        display.print(mapfloat(datamgr->battery_voltage, BAT_ADC_MIN, BAT_ADC_MAX, 3.6, 4.2));
    }else{
        if(datamgr->counter_mode == 0){
            display.setTextColor(BLACK, WHITE);
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
            display.setCursor(0, 0);
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
            display.setCursor(84 - getNumOfDigits(datamgr->rad_max)*12, 8);
            display.print(datamgr->sum_old);
            display.setTextSize(0);
            display.setCursor(0, 23);
            display.print(T_CPS);
            display.setCursor(84 - 3*6, 23);
            display.print(T_OLD);
            display.drawFastHLine(0,32,84,BLACK);

            #if defined(DRAW_GRAPH)
            for(byte i=0;i<83;i++){
                display.drawLine(i,47,i, 47-datamgr->mass[i], BLACK);
            }
            #endif
        }
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
            if(datamgr->cursor < 4){
                uint8_t editable_perc = map(datamgr->editable, 0, 255, 0, 100);
                #if defined(UNIVERSAL_COUNTER)
                if (datamgr->cursor==0) display.print(T_CURSOR);
                display.print(GCOUNTER);
                display.setCursor(0, 20);
            
                if (datamgr->cursor==1) display.print(T_CURSOR);
                display.print(TONE);
                if(datamgr->cursor==1 && datamgr->editing_mode){
                    display.setCursor(84 - getNumOfDigits(editable_perc)*10, 20);
                    display.setTextColor(WHITE, BLACK);
                    display.print(editable_perc);
                }else{
                    uint8_t ton_buzz = map(datamgr->ton_BUZZ, 0, 255, 0, 100);
                    display.setCursor(84 - getNumOfDigits(ton_buzz)*10, 20);
                    display.print(ton_buzz);
                }
                display.print("%");
                display.setCursor(0, 30);
                display.setTextColor(BLACK, WHITE);    

                if (datamgr->cursor==2) display.print(T_CURSOR);
                display.print(BACKLIGHT);
                if(datamgr->cursor==2 && datamgr->editing_mode){
                    display.setCursor(84 - getNumOfDigits(editable_perc)*10, 30);
                    display.setTextColor(WHITE, BLACK);
                    display.print(editable_perc);
                }else{
                    uint8_t backlight = map(datamgr->backlight, 0, 255, 0, 100);
                    display.setCursor(84 - getNumOfDigits(backlight)*10, 30);
                    display.print(backlight);
                }
                display.print("%");
                display.setCursor(0, 40);
                display.setTextColor(BLACK, WHITE);

                if (datamgr->cursor==3) display.print(T_CURSOR);
                display.print(CONTRAST);
                if(datamgr->cursor==3 && datamgr->editing_mode){
                    display.setCursor(84 - getNumOfDigits(editable_perc)*10, 40);
                    display.setTextColor(WHITE, BLACK);
                    display.print(editable_perc);
                }else{
                    uint8_t contrast = map(datamgr->contrast, 0, 255, 0, 100);
                    display.setCursor(84 - getNumOfDigits(contrast)*10, 40);
                    display.print(contrast);
                }
                display.print("%");
                #else
                if (datamgr->cursor==0) display.print(T_CURSOR);
                display.print(TONE);
                if(datamgr->cursor==0 && datamgr->editing_mode){
                    display.setCursor(84 - getNumOfDigits(editable_perc)*6, 10);
                    display.setTextColor(WHITE, BLACK);
                    display.print(editable_perc);
                }else{
                    display.setCursor(84 - getNumOfDigits(datamgr->ton_BUZZ)*6, 10);
                    display.print(datamgr->ton_BUZZ);
                }
                display.setCursor(0, 20);
                display.setTextColor(BLACK, WHITE);    

                if (datamgr->cursor==1) display.print(T_CURSOR);
                display.print(BACKLIGHT);
                if(datamgr->cursor==1 && datamgr->editing_mode){
                    display.setCursor(84 - getNumOfDigits(editable_perc)*6, 20);
                    display.setTextColor(WHITE, BLACK);
                    display.print(editable_perc);
                }else{
                    display.setCursor(84 - getNumOfDigits(datamgr->backlight)*6, 20);
                    display.print(datamgr->backlight);
                }
                display.setCursor(0, 30);
                display.setTextColor(BLACK, WHITE);

                if (datamgr->cursor==2) display.print(T_CURSOR);
                display.print(CONTRAST);
                if(datamgr->cursor==2 && datamgr->editing_mode){
                    display.setCursor(84 - getNumOfDigits(editable_perc)*6, 30);
                    display.setTextColor(WHITE, BLACK);
                    display.print(editable_perc);
                }else{
                    display.setCursor(84 - getNumOfDigits(datamgr->contrast)*6, 30);
                    display.print(datamgr->contrast);
                }
                #endif
            }else{
                 if (datamgr->cursor==4) display.print(T_CURSOR);
                display.print(DOSE_SAVE);
                if(datamgr->cursor==4 && datamgr->editing_mode){
                    display.setCursor(84 - getNumOfDigits(datamgr->editable)*6, 10);
                    display.setTextColor(WHITE, BLACK);
                    display.print(datamgr->editable);
                }else{
                    display.setCursor(84 - getNumOfDigits(datamgr->save_dose_interval)*10, 10);
                    display.print(datamgr->save_dose_interval);
                }
                display.setCursor(0, 20);
                display.setTextColor(BLACK, WHITE); 
            }
            
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
            if (datamgr->cursor==1) display.print(T_CURSOR);
            display.print(GTIME);
            if(datamgr->cursor==1 && datamgr->editing_mode){
                display.setCursor(84 - (getNumOfDigits(datamgr->editable)+1)*6, 10);
                display.setTextColor(WHITE, BLACK);
                display.print(datamgr->editable);
            }else{
                display.setCursor(84 - (getNumOfDigits(datamgr->GEIGER_TIME)+1)*6, 10);
                display.print(datamgr->GEIGER_TIME);
            }
            display.print("s");
            display.setCursor(0, 20);
            display.setTextColor(BLACK, WHITE);

            if (datamgr->cursor==2) display.print(T_CURSOR);
            display.print(ERROR);
            if(datamgr->cursor==2 && datamgr->editing_mode){
                display.setCursor(84 - (getNumOfDigits(datamgr->editable)+2)*6, 20);
                display.setTextColor(WHITE, BLACK);
                display.write(240);
                display.print(datamgr->editable, 1);
            }else{
                display.setCursor(84 - (getNumOfDigits(datamgr->geiger_error)+2)*6, 20);
                display.write(240);
                display.print(datamgr->geiger_error, 1);
            }
            display.print("%");
            display.setTextColor(BLACK, WHITE);
        }break;
        #endif
    }
    display.display();
}

void OutputManager::draw_bat_low(){
    if(!datamgr->is_charging || datamgr->is_charged){ datamgr->counter_mode = 0; }
    display.clearDisplay();
    if(no_volt_flag) display.drawBitmap(17, 12, big_battery_Bitmap, 50, 24, BLACK);
    display.display();
    if(millis() - no_volt_ticker > 1000){
        no_volt_ticker = millis();
        no_volt_flag = !no_volt_flag;
    }
}

#if defined(CAN_SLEEP)
void OutputManager::going_to_sleep(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(POFF);
    display.display();
    _delay_ms(1000);
    display.clearDisplay();
    display.display();
}
#endif