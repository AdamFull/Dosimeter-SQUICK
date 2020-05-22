#include <Arduino.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <Libs/GyverButton.h>

#include <macros.h>
#include <Managers/ADCManager.h>
#include <Managers/OutputManager.h>
#include <Managers/DataManager.h>

/*TODO
1. Придумать нормальное выключение
2. Дисплей поставить на пин и включать через фет, так же как и эмитерный повторитель
3. Раскидать код по классам
4. Доработать код, оставить только побитовые операции.
5. Убрать всё что имеется от ардуино ide
6. Переписать библиотеку работы с экраном и оставить только необходимое
7. Подумать насчёт измерения в зивертах
8. Отвязать пищалку от задержек
*/

#define URLED_PIN 0
#define MRLED_PIN 1
#define RLED_PIN 2

#define COUNTER_PIN 3

GButton btn_reset(3, HIGH_PULL, NORM_OPEN);
GButton btn_set(12, HIGH_PULL, NORM_OPEN);

DataManager datamgr;
OutputManager outmgr;

void conv_pump(void);
void impulse(void);
void show_info(void);
void cancel(String);
void button_action(void);
void sleep(void);
void(* resetFunc) (void) = 0;

void setup() {
	//wdt_enable(WDTO_8S);				//Интервал сторожевого таймера 8 сек
	WDTCSR |= (1 << WDIE);				//Разрешить прерывания сторожевого таймера

	datamgr.init();

	btn_reset.setClickTimeout(10);
	btn_set.setClickTimeout(10);
	btn_reset.setTimeout(1000);
	btn_set.setTimeout(1000);

	//ACSR |= 1 << ACD; //отключаем компаратор

	ADCManager::adc_init();

	outmgr.init();

	//настраиваем Timer 1
	TIMSK1=0; //отключить таймер
	TCCR1A=0; //OC1A/OC1B disconnected
	TCCR1B=0b00000101; //предделитель 16M/1024=15625кГц
	TCNT1=TIMER1_PRELOAD;

	PORTC_MODE(2, 0);						//pin A2 (PC2) как выход, земля экрана
	PORTC_WRITE(2, 0);

	PORTC_MODE(3, 0); 						//pin A3 (PC3) как выход, замля повторителя
	PORTC_WRITE(3, 0);

	PORTD_MODE(5, 0); 						//pin 5 (PD5) как выход, звуковая индикация частицы
	PORTD_WRITE(5, 0);

	PORTB_MODE(0, 0); 						//pin 8 (PB0) как выход, единица измерения микрорентген
	PORTB_WRITE(0, 0);
	PORTB_MODE(1, 0); 						//pin 9 (PB1) как выход, единица измерения миллирентген
	PORTB_WRITE(1, 0);
	PORTB_MODE(2, 0); 						//pin 10 (PB2) как выход, единица измерения рентген
	PORTB_WRITE(2, 0);

	PORTD_MODE(3, 0); 						//pin 3 (PD3) как выход, блинк при засекании частицы
	PORTD_WRITE(3, 0);

	PORTB_MODE(3, 0); 						//pin 11 (PB3) как выход, уаравление преобразователем
	PORTB_WRITE(3, 0);

	PORTD_MODE(2, 1); 						//настраиваем пин 2 (PD2) на вход, импульсы от счетчика
	PORTD_WRITE(2, 1); 						//подтягивающий резистор	

	PORTC_WRITE(2, 1);						//Включить экран
	PORTC_WRITE(3, 1);						//Включить эмиттерный повторитель

	//Изменяем параметры таймера 2 для повышения частоты шим на 3 и 11
	TCCR2B = 0b00000010;  // x8
	TCCR2A = 0b00000011;  // fast pwm

  	TIMSK1=0b00000001; //запускаем Timer 1

	analogWrite(11, datamgr.pwm_converter);

	EICRA=0b00000010; //настриваем внешнее прерывание 0 по спаду
	EIMSK=0b00000001; //разрешаем внешнее прерывание 0
}

/*ISR(WDT_vect){
	if(wdt_counter > 0 && !is_sleeping){
		wdt_counter--;
		wdt_disable();
	}else{
		if(!is_sleeping) sleep();
		else wdt_reset();
	}
}*/

ISR(INT0_vect){ //внешнее прерывание //считаем импульсы от счетчика
	if(datamgr.rad_buff[0]!=65535) datamgr.rad_buff[0]++; //нулевой элемент массива - текущий секундный замер
	if(++datamgr.rad_sum>999999UL*3600/datamgr.GEIGER_TIME) datamgr.rad_sum=999999UL*3600/datamgr.GEIGER_TIME; //общая сумма импульсов
	//if(wdt_counter < 255) wdt_counter++;
	datamgr.detected = true;
}

ISR(TIMER1_OVF_vect){ //прерывание по переполнению Timer 1

static uint8_t cnt1;

TCNT1=TIMER1_PRELOAD;

if(++cnt1>=TIME_FACT) //расчет показаний один раз в секунду
	{
	cnt1=0;

	uint32_t tmp_buff=0;
	for(uint8_t i=0; i<datamgr.GEIGER_TIME; i++) tmp_buff+=datamgr.rad_buff[i]; //расчет фона мкР/ч
	if(tmp_buff>999999) tmp_buff=999999; //переполнение
	datamgr.rad_back=tmp_buff;

	if(datamgr.rad_back>datamgr.rad_max) datamgr.rad_max=datamgr.rad_back; //фиксируем максимум фона

	for(uint8_t k=datamgr.GEIGER_TIME-1; k>0; k--) datamgr.rad_buff[k]=datamgr.rad_buff[k-1]; //перезапись массива
	datamgr.rad_buff[0]=0; //сбрасываем счетчик импульсов

	datamgr.rad_dose=(datamgr.rad_sum*datamgr.GEIGER_TIME/3600); //расчитаем дозу

	if(datamgr.time_hrs<99) //если таймер не переполнен
		{
		if(++datamgr.time_sec>59) //считаем секунды
			{
			if(++datamgr.time_min>59) //считаем минуты
				{
				if(++datamgr.time_hrs>99) datamgr.time_hrs=99; //часы
				datamgr.time_min=0;
				}
			datamgr.time_sec=0;
			}
		}
	}
}


void sleep(){
	if(!datamgr.is_sleeping){
		analogWrite(11, 0);
		PORTB_WRITE(MRLED_PIN, 0);
		PORTB_WRITE(URLED_PIN, 0);
		PORTB_WRITE(RLED_PIN, 0);
		
		datamgr.is_sleeping = true;
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
		datamgr.is_sleeping = false;
		resetFunc();
	}
	
}

/*void button_action(){
	btn_reset.tick();
	btn_set.tick();

	bool btn_reset_isHolded = btn_reset.isHolded();
	bool btn_set_isHolded = btn_set.isHolded();

	if(btn_reset.isHold() && btn_set.isHold()){
		if(!menu_mode){
			menu_mode = true;
			editing_mode = false;
			btn_reset.resetStates();
			btn_set.resetStates();
		}
		if(wdt_counter < 255) wdt_counter++;
	}else if(btn_reset_isHolded){
		if(menu_mode || editing_mode){
			if(editing_mode && (mode == 4 || mode == 5)){
				editing_mode = false;
				menu_mode = true;
			}else{
				cancel("----");
			}
		}else{
			switch (mode)
			{
				case 0:{ rad_back = 0; } break;
				case 1:{ rad_sum = 0; } break;
				case 2:{ rad_max = 0; } break;
				case 3:{ rad_dose = 0; } break;
			}
		}
		if(wdt_counter < 255) wdt_counter++;
	}else if(btn_reset.isClick() && !btn_reset_isHolded){
		if(menu_mode || editing_mode)
		{
			if(editing_mode){
				if(mode == 4)
					if(pwm_converter > 0)
						pwm_converter--;
				if(mode == 5)
					if(GEIGER_TIME > 0)
						GEIGER_TIME--;
				if(mode == 6){
					if(ton_BUZZ > 0)
						ton_BUZZ -= 5;
					detected = true;
				}
			}else{
				if(mode > 0)
					mode--;
			}
		}else{
			show_mode = true;
		}
		if(wdt_counter < 255) wdt_counter++;
	}else if(btn_set_isHolded){
		if(menu_mode || editing_mode){
			if((mode == 4 || mode == 5 || mode == 6) && !editing_mode){
				editing_mode = true;
				menu_mode = false;
			}else if(editing_mode){
				if(mode == 4)
					save_voltage_config();
				if(mode == 5)
					save_geiger_time_config();
				if(mode == 6)
					save_tone_delay();
			}else{
				menu_mode = false;
			}
		}else{
			sleep();
		}
		if(wdt_counter < 255) wdt_counter++;
	}else if(btn_set.isClick() && !btn_set_isHolded){
		if(menu_mode || editing_mode)
		{
			if(editing_mode){
				if(mode == 4)
					if(pwm_converter < 255)
						pwm_converter++;
				if(mode == 5)
					if(GEIGER_TIME < 255)
						GEIGER_TIME++;
				if(mode == 6){
					if(ton_BUZZ < 255)
						ton_BUZZ += 5;
					detected = true;
				}
			}else{
				if(mode < 7)
					mode++;
			}
		}else{
			zivert = !zivert;
		}
		if(wdt_counter < 255) wdt_counter++;
	}
}*/

float get_battery_voltage(){
	datamgr.sensorValue = (datamgr.sensorValue * (datamgr.avgFactor - 1) + ADCManager::adc0_read()) / datamgr.avgFactor;
	float voltage = 0.2 + (1125300UL / datamgr.sensorValue) * 2;
	return voltage;
}

unsigned voltage_config()
{
	//ADCSRA |= (1 << ADEN);
	datamgr.sensorValue = (datamgr.sensorValue * (datamgr.avgFactor - 1) + ADCManager::adc1_read()) / datamgr.avgFactor;
	return (TARGET_VOLTAGE*datamgr.sensorValue/DIVIDER);
	//ADCSRA &= ~(1 << ADEN);
}

void voltage_editing(){
	analogWrite(11, datamgr.pwm_converter);
}

void loop() {
	if(!datamgr.is_sleeping) outmgr.update();
	//button_action();
}