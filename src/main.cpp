#include <Arduino.h>
#include <avr/power.h>
#include <Libs/GyverButton.h>

#include <macros.h>

#include <Managers/OutputManager.h>

GButton btn_reset(A4, HIGH_PULL, NORM_OPEN);
GButton btn_set(A5, HIGH_PULL, NORM_OPEN);

DataManager *datamgr = new DataManager();
OutputManager *outmgr = new OutputManager(datamgr);

Adafruit_PCD8544 display = Adafruit_PCD8544(10, 9, 8, 7, 6);

void button_action(void);
void sleep(void);
void(* resetFunc) (void) = 0;

void setup() {
	Serial.begin(9600);
	datamgr->init();
	Serial.println("datamgr init ok");

	btn_reset.setClickTimeout(10);
	btn_set.setClickTimeout(10);
	btn_reset.setTimeout(1000);
	btn_set.setTimeout(1000);

	outmgr->init();
	Serial.println("outmgr init ok");

	//настраиваем Timer 1
	TIMSK1=0; //отключить таймер
	TCCR1A=0; //OC1A/OC1B disconnected
	TCCR1B=0b00000101; //предделитель 16M/1024=15625кГц
	TCNT1=TIMER1_PRELOAD;
	Serial.println("Timer init ok");

	PORTD_MODE(0, 0);
	PORTD_WRITE(0, 0);

	PORTD_MODE(1, 0);
	PORTD_WRITE(1, 0);

	PORTD_MODE(2, 1); 						//настраиваем пин 2 (PD2) на вход, импульсы от счетчика
	PORTD_WRITE(2, 1); 						//подтягивающий резистор	

	PORTD_MODE(3, 0); 						//pin 3 (PD3) как выход, блинк при засекании частицы
	PORTD_WRITE(3, 0);

	PORTD_MODE(5, 0); 						//pin 5 (PD5) как выход, звуковая индикация частицы
	PORTD_WRITE(5, 0);

	PORTB_MODE(3, 0); 						//pin 11 (PB3) как выход, уаравление преобразователем
	PORTB_WRITE(3, 0);

	PORTC_MODE(2, 0);						//pin A2 (PC2) как выход, земля экрана
	PORTC_WRITE(2, 0);

	PORTC_MODE(3, 0); 						//pin A3 (PC3) как выход, замля повторителя
	PORTC_WRITE(3, 0);


	PORTC_WRITE(2, 1);						//Включить экран
	PORTC_WRITE(3, 1);						//Включить эмиттерный повторитель

	Serial.println("Outputs init ok");

	//Изменяем параметры таймера 2 для повышения частоты шим на 3 и 11
	TCCR2B = 0b00000010;  // x8
	TCCR2A = 0b00000011;  // fast pwm

  	TIMSK1=0b00000001; //запускаем Timer 1

	analogWrite(3, datamgr->pwm_converter);

	EICRA=0b00000010; //настриваем внешнее прерывание 0 по спаду
	EIMSK=0b00000001; //разрешаем внешнее прерывание 0
	Serial.println("All init ok");
}

ISR(INT0_vect){ //внешнее прерывание //считаем импульсы от счетчика
	if(datamgr->rad_buff[0]!=65535) datamgr->rad_buff[0]++; //нулевой элемент массива - текущий секундный замер
	if(++datamgr->rad_sum>999999UL*3600/datamgr->GEIGER_TIME) datamgr->rad_sum=999999UL*3600/datamgr->GEIGER_TIME; //общая сумма импульсов
	//if(wdt_counter < 255) wdt_counter++;
	datamgr->detected = true;
}

ISR(TIMER1_OVF_vect){ //прерывание по переполнению Timer 1

static uint8_t cnt1;

TCNT1=TIMER1_PRELOAD;

if(++cnt1>=TIME_FACT) //расчет показаний один раз в секунду
	{
	cnt1=0;

	uint32_t tmp_buff=0;
	for(uint8_t i=0; i<datamgr->GEIGER_TIME; i++) tmp_buff+=datamgr->rad_buff[i]; //расчет фона мкР/ч
	if(tmp_buff>999999) tmp_buff=999999; //переполнение
	datamgr->rad_back=tmp_buff;

	if(datamgr->rad_back>datamgr->rad_max) datamgr->rad_max=datamgr->rad_back; //фиксируем максимум фона

	for(uint8_t k=datamgr->GEIGER_TIME-1; k>0; k--) datamgr->rad_buff[k]=datamgr->rad_buff[k-1]; //перезапись массива
	datamgr->rad_buff[0]=0; //сбрасываем счетчик импульсов

	datamgr->rad_dose=(datamgr->rad_sum*datamgr->GEIGER_TIME/3600); //расчитаем дозу

	if(datamgr->time_hrs<99) //если таймер не переполнен
		{
		if(++datamgr->time_sec>59) //считаем секунды
			{
			if(++datamgr->time_min>59) //считаем минуты
				{
				if(++datamgr->time_hrs>99) datamgr->time_hrs=99; //часы
				datamgr->time_min=0;
				}
			datamgr->time_sec=0;
			}
		}
	}
}

void sleep(){
	if(!datamgr->is_sleeping){
		analogWrite(11, 0);
		
		datamgr->is_sleeping = true;
		//Уменьшаю задержку кнопки, т.к. на заниженых частотах всё работает гораздо медленнее, 6 сек на включение
		btn_set.setTimeout(1);
		//Замедляю микроконтроллер в 6 раз, частота 250 кГц (Остальное слишком медленно, он не хочет просыпаться)
		CLKPR = 1<<CLKPCE;
    	CLKPR = 6;
		cli();
		// Отключаем детектор пониженного напряжения питания
  		MCUCR != (1 << BODS) | (1 << BODSE);
  		MCUCR &= ~(1 << BODSE);
		//Отключаю всё кроме таймера 0, т.к. он нужен для обработки кнопки.
		power_timer1_disable();					//используется для расчётов, в выключеном состоянии они не нужны
		power_timer2_disable();					//используется для шим, он тоже не нужен.
		power_adc_disable();					//Читать данные с батареи и с вв источника не нужно, отключаем
		power_spi_disable();					//SPI в принципе не используется, нужно будет его тоже отключить
		power_usart0_disable();					//Юарт в дальнейшем тоже будет выпилен

		PORTC_WRITE(2, 0);						//Выключить экран
		PORTC_WRITE(3, 0);						//Выключить эмиттерный повторитель
		sei();
	}else{
		//Ставим делитель обратно, частота 16 МГц
		CLKPR = 1<<CLKPCE;
    	CLKPR = 0;
		// Отключаем детектор пониженного напряжения питания
  		MCUCR = (1 << BODS) | (1 << BODSE);
  		MCUCR |= (1 << BODSE);

		power_all_enable();
		datamgr->is_sleeping = false;
		resetFunc();
	}
	
}

void button_action(){
	btn_reset.tick();
	btn_set.tick();

	bool btn_reset_isHolded = btn_reset.isHolded();
	bool btn_set_isHolded = btn_set.isHolded();

	bool menu_mode = datamgr->page == 2;
	bool editing_mode = datamgr->editing_mode;

	if(btn_reset.isHold() && btn_set.isHold()){
		if(!menu_mode){
			datamgr->page = 2;
			datamgr->editing_mode = false;
			btn_reset.resetStates();
			btn_set.resetStates();
		}
	}else if(btn_reset_isHolded){											//Удержание кнопки ресет
		if(menu_mode && !editing_mode){
			if(datamgr->menu_page == 0) datamgr->page = 1;
			else datamgr->menu_page = 0;
		}
		if(editing_mode) datamgr->editing_mode = false;
	}else if(btn_reset.isClick() && !btn_reset_isHolded){					//Клик кнопки ресет
		if(menu_mode && !editing_mode && datamgr->cursor > 0) datamgr->cursor--;
		if(editing_mode){
			switch (datamgr->cursor){
				case 0:{ datamgr->pwm_converter--; }break;
				case 1:{ datamgr->GEIGER_TIME--; }break;
				case 2:{ datamgr->ton_BUZZ--; }break;
				case 3:{ datamgr->backlight--; }break;
			}
		}
	}else if(btn_set_isHolded){												//Удержание кнопки сет
		if(menu_mode && !editing_mode) {
			switch (datamgr->menu_page){
				case 0:{
					switch (datamgr->cursor){
						case 0:{ datamgr->menu_page = 1; }break;
						case 1:{ datamgr->menu_page = 2; }break;
						case 2:{ datamgr->menu_page = 3; }break;
						case 3:{ datamgr->menu_page = 4; }break;
					}
					datamgr->cursor = 0;
				}break;
				case 1:{
					switch (datamgr->cursor){
						case 0:{ /*Пока хз*/ }break;
						case 1:{ /*Пока хз*/ }break;
						case 2:{ /*Пока хз*/ }break;
					}
					datamgr->cursor = 0;
				}break;
				case 2:{
					datamgr->editing_mode = true;
					Serial.print("Set editing: ");
					Serial.println(datamgr->editing_mode);
				}break;
				case 3:{
					switch (datamgr->cursor){								//Стереть данные
						case 0:{ datamgr->reset_settings(); datamgr->menu_page = 0; }break;
						case 1:{ /*Пока хз*/ }break;
						case 2:{ datamgr->reset_settings(); datamgr->menu_page = 0; }break;
					}
					datamgr->cursor = 0;
				}break;
				case 4:{
					switch (datamgr->cursor){								//Вообще это диалог выбора, но пока что это не он
						case 0:{ datamgr->menu_page = 0; }break;
						case 1:{ sleep(); }break;
					}
					datamgr->cursor = 0;
				}break;
			}
		}
		if(menu_mode && editing_mode){
			datamgr->editing_mode = false;
			datamgr->save_all();
		}
	}else if(btn_set.isClick() && !btn_set_isHolded){					//Клик кнопки сет
		if(menu_mode && !editing_mode){						//Сдвинуть курсор, если можно
			switch (datamgr->menu_page){
				case 0:{ if(datamgr->cursor < 3) datamgr->cursor++; } break;
				case 1:{ if(datamgr->cursor < 2) datamgr->cursor++; } break;
				case 2:{ if(datamgr->cursor < 3) datamgr->cursor++; } break;
				case 3:{ if(datamgr->cursor < 2) datamgr->cursor++; } break;
				case 4:{ if(datamgr->cursor < 1) datamgr->cursor++; } break;
			}
		}
		if(editing_mode){										//Если редактируем
			switch (datamgr->cursor){
				case 0:{ datamgr->pwm_converter++; }break;
				case 1:{ datamgr->GEIGER_TIME++; }break;
				case 2:{ datamgr->ton_BUZZ++; }break;
				case 3:{ datamgr->backlight++; }break;
			}
		}
	}
}

void mode_handler(){
	if(datamgr->page == 2){
		if(datamgr->editing_mode)
		switch (datamgr->cursor){
			case 0:{ analogWrite(3, datamgr->pwm_converter); } break;
			case 1:{} break;
			case 2:{ datamgr->detected = true; } break;
			case 3:{ analogWrite(11, datamgr->backlight); } break;
			case 4:{ outmgr->set_contrast(datamgr->contrast); } break;
		}
	}
}

void loop() {
	if(!datamgr->is_sleeping){
		mode_handler();
		outmgr->update();
	}
	button_action();
}