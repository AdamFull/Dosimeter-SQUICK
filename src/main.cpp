#include <Arduino.h>
#include <TM1637.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <GyverButton.h>

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

#define CLK 6
#define DIO 7

#define BUZZER_PIN 5
#define URLED_PIN 0
#define MRLED_PIN 1
#define RLED_PIN 2
#define MODE_BTN 3

#define COUNTER_PIN 3

byte GEIGER_TIME = 37;

#define TIMER1_PRELOAD 64910 //65535-64910=625, 15625/625=25Гц
#define HVGEN_FACT 5 // 25/5=5Гц частота подкачки преобразователя
#define TIME_FACT 25 // 25Гц/25=1Гц секундные интервалы

#define TARGET_VOLTAGE 400				//Требуемое напряжение
#define DIVIDER 883						//Значение ацп делителя напряжения

bool detected = false;

uint16_t *rad_buff;// = new uint16_t[GEIGER_TIME]; //массив секундных замеров для расчета фона
uint32_t rad_sum; //сумма импульсов за все время
uint32_t rad_back; //текущий фон
uint32_t rad_max; //максимум фона
uint32_t rad_dose; //доза
uint8_t time_sec; //секунды //счетчики времени
uint8_t time_min; //минуты
uint8_t time_hrs; //часы

uint8_t mode = 0; // режим выводимых на экран данных
bool menu_mode = false;
bool editing_mode = false;
bool show_mode = false;
bool zivert = false;
byte pwm_converter = 45;

bool is_sleeping = false;

unsigned long timing = 0;

byte ton_BUZZ = 200; //тональность буззера
bool buzz_mode = false;

volatile byte wdt_counter;

TM1637 tm1637(CLK, DIO);
GButton btn_reset(3, HIGH_PULL, NORM_OPEN);
GButton btn_set(12, HIGH_PULL, NORM_OPEN);

void conv_pump(void);
void impulse(void);
void show_info(void);
void update_counter(void);
void save_voltage_config(void);
void save_geiger_time_config(void);
void save_tone_delay(void);
void cancel(String);
void button_action(void);
void sleep(void);
void(* resetFunc) (void) = 0;

void setup_defaults()
{
	eeprom_write_byte((uint8_t*)0b0, 0b1);
	eeprom_write_byte((uint8_t*)0b1, pwm_converter);
	eeprom_write_byte((uint8_t*)0b10, GEIGER_TIME);
	eeprom_write_byte((uint8_t*)0b11, ton_BUZZ);
}

void setup() {
	wdt_enable(WDTO_8S);				//Интервал сторожевого таймера 8 сек
	WDTCSR |= (1 << WDIE);				//Разрешить прерывания сторожевого таймера

	Serial.begin(9600);
	if(eeprom_read_byte((uint8_t*)0b0) == 0b0) setup_defaults();
	pwm_converter = eeprom_read_byte((uint8_t*)0b1);
	GEIGER_TIME = eeprom_read_byte((uint8_t*)0b10);
	ton_BUZZ = eeprom_read_byte((uint8_t*)0b11);

	Serial.println("PWM: " + String(pwm_converter));
	Serial.println("Geiger: " + String(GEIGER_TIME));

	update_counter();

	btn_reset.setClickTimeout(10);
	btn_set.setClickTimeout(10);
	btn_reset.setTimeout(1000);
	btn_set.setTimeout(1000);

	//ACSR |= 1 << ACD; //отключаем компаратор

	ADCSRA |= (1 << ADEN)|(1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0); // Включаем АЦП, устанавливаем предделитель преобразователя на 128 

	//настраиваем Timer 1
	TIMSK1=0; //отключить таймер
	TCCR1A=0; //OC1A/OC1B disconnected
	TCCR1B=0b00000101; //предделитель 16M/1024=15625кГц
	TCNT1=TIMER1_PRELOAD;

	tm1637.set();
	tm1637.init();

	Serial.begin(9600);

	bitSet(DDRD,5); 						//pin 5 (PD5) как выход, звуковая индикация частицы
	bitClear(PORTD,5);

	bitSet(DDRB,0); 						//pin 8 (PB0) как выход, единица измерения микрорентген
	bitClear(PORTB,0);
	bitSet(DDRB,1); 						//pin 9 (PB1) как выход, единица измерения миллирентген
	bitClear(PORTB,1);
	bitSet(DDRB,2); 						//pin 10 (PB2) как выход, единица измерения рентген
	bitClear(PORTB,2);

	bitSet(DDRB,5); 						//pin 13 (PB5) как выход, блинк при засекании частицы
	bitClear(PORTB,5);

	bitSet(DDRB,3); 						//pin 11 (PB3) как выход, уаравление преобразователем
	bitClear(PORTB,3);

	bitClear(DDRD,2); 						//настраиваем пин 2 (PD2) на вход, импульсы от счетчика
	bitSet(PORTD,2); 						//подтягивающий резистор	

	//Изменяем параметры таймера 2 для повышения частоты шим на 3 и 11
	TCCR2B = 0b00000010;  // x8
	TCCR2A = 0b00000011;  // fast pwm

  	TIMSK1=0b00000001; //запускаем Timer 1

	analogWrite(11, pwm_converter);

	EICRA=0b00000010; //настриваем внешнее прерывание 0 по спаду
	EIMSK=0b00000001; //разрешаем внешнее прерывание 0
	//tm1637.point(POINT_ON);
}

int adc0_read()
{
	ADMUX |=(1 << REFS0)|(0 << MUX0)|(0 << MUX1)|(0 << MUX2)|(0 << MUX3); // выставляем опорное напряжение Vcc, снимать сигнал будем с входа AC3
	do{ ADCSRA |= (1 << ADSC); } // Начинаем преобразование
	while ((ADCSRA & (1 << ADIF)) == 0); // пока не будет выставлен флаг об окончании преобразования
	return (ADCL | ADCH<<8);
}

int adc1_read()
{
	ADMUX |= (0 << REFS1)|(1 << REFS0)|(1 << MUX0)|(0 << MUX1)|(0 << MUX2)|(0 << MUX3); // выставляем опорное напряжение Vcc, снимать сигнал будем с входа AC3
	do{ ADCSRA |= (1 << ADSC); } // Начинаем преобразование
	while ((ADCSRA & (1 << ADIF)) == 0); // пока не будет выставлен флаг об окончании преобразования
	return (ADCL | ADCH<<8);
}

ISR(WDT_vect){
	if(wdt_counter > 0 && !is_sleeping){
		wdt_counter--;
		wdt_disable();
	}else{
		if(!is_sleeping) sleep();
		else wdt_reset();
	}
}

ISR(INT0_vect){ //внешнее прерывание //считаем импульсы от счетчика
	if(rad_buff[0]!=65535) rad_buff[0]++; //нулевой элемент массива - текущий секундный замер
	if(++rad_sum>999999UL*3600/GEIGER_TIME) rad_sum=999999UL*3600/GEIGER_TIME; //общая сумма импульсов
	if(wdt_counter < 255) wdt_counter++;
	detected = true;
}

ISR(TIMER1_OVF_vect){ //прерывание по переполнению Timer 1

static uint8_t cnt1;

TCNT1=TIMER1_PRELOAD;

if(++cnt1>=TIME_FACT) //расчет показаний один раз в секунду
	{
	cnt1=0;

	uint32_t tmp_buff=0;
	for(uint8_t i=0; i<GEIGER_TIME; i++) tmp_buff+=rad_buff[i]; //расчет фона мкР/ч
	if(tmp_buff>999999) tmp_buff=999999; //переполнение
	rad_back=tmp_buff;

	if(rad_back>rad_max) rad_max=rad_back; //фиксируем максимум фона

	for(uint8_t k=GEIGER_TIME-1; k>0; k--) rad_buff[k]=rad_buff[k-1]; //перезапись массива
	rad_buff[0]=0; //сбрасываем счетчик импульсов

	rad_dose=(rad_sum*GEIGER_TIME/3600); //расчитаем дозу

	if(time_hrs<99) //если таймер не переполнен
		{
		if(++time_sec>59) //считаем секунды
			{
			if(++time_min>59) //считаем минуты
				{
				if(++time_hrs>99) time_hrs=99; //часы
				time_min=0;
				}
			time_sec=0;
			}
		}
	}
}


void sleep(){
	if(!is_sleeping){
		tm1637.displayStr((char*)"P0FF");
		delay(1000);
		tm1637.clearDisplay();
		analogWrite(11, 0);
		bitClear(PORTB, MRLED_PIN);		
		bitClear(PORTB, URLED_PIN);
		bitClear(PORTB, RLED_PIN);
		
		is_sleeping = true;
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
		sei();
	}else{
		//Ставим делитель обратно, частота 16 МГц
		CLKPR = 1<<CLKPCE;
    	CLKPR = 0;
		// Отключаем детектор пониженного напряжения питания
  		MCUCR = (1 << BODS) | (1 << BODSE);
  		MCUCR |= (1 << BODSE);

		power_all_enable();
		is_sleeping = false;
		tm1637.displayStr((char*)"P ON");
		delay(1000);
		resetFunc();
	}
	
}

void button_action(){
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
}

void update_counter(void){
	rad_buff = new uint16_t[GEIGER_TIME];
	for(byte i = 0; i < GEIGER_TIME; i++){ rad_buff[i] = (uint16_t)0; }
	rad_back = rad_dose = rad_max = rad_sum = 0;
}

void delayUs(byte dtime){
	for(int i = 0; i < dtime; i++){
		_delay_us(1);
	}
}

void signa () { //индикация каждой частички звуком светом
	if(buzz_mode){
		if (millis()-timing>=ton_BUZZ){
			timing+=ton_BUZZ;
			if(detected){
				bitSet(PORTD, 5);
				bitSet(PORTB, 5);
				detected = false;
			}else{
				bitClear(PORTB, 5);
				bitClear(PORTD, 5);
			}
		}
	}else{
		if(detected){
			detected = false;
    		int d = 30;
			bitSet(PORTB, 5);
    		while (d > 0) {
      			bitSet(PORTD, 5);
      			delayUs(ton_BUZZ);
      			bitClear(PORTD, 5);
      			delayUs(ton_BUZZ);
	  			asm("nop");
      			d--;
    		}
			bitClear(PORTB, 5);
		}
	}
}

float get_battery_voltage(){
	float voltage = 0.2 + (1125300UL / adc0_read()) * 2;
	return voltage;
}

unsigned voltage_config()
{
	//ADCSRA |= (1 << ADEN);
	unsigned readed_value = adc1_read();
	return (TARGET_VOLTAGE*readed_value/DIVIDER);
	//ADCSRA &= ~(1 << ADEN);
}

void voltage_editing(){
	analogWrite(11, pwm_converter);
}

void cancel(String msg){
	char buff[4];
	msg.toCharArray(buff, 5);
	tm1637.displayStr(buff);
	delay(1000);
	mode = 0;
	editing_mode = false;
	menu_mode = false;
}

void save_voltage_config(void)
{
	//byte voltage_mult = (byte)map(adc0_read(), 0, 1023, 0, 255);
	eeprom_update_byte((uint8_t*)0b1, pwm_converter);
	analogWrite(11, pwm_converter);
	tm1637.displayStr((char*)"SAVE");
	delay(1000);
	mode = 0;
	editing_mode = false;
}

void save_geiger_time_config(void)
{
	eeprom_update_byte((uint8_t*)0b10, GEIGER_TIME);
	update_counter();
	tm1637.displayStr((char*)"SAVE");
	delay(1000);
	mode = 0;
	editing_mode = false;
}

void save_tone_delay(void){
	eeprom_update_byte((uint8_t*)0b11, ton_BUZZ);
	tm1637.displayStr((char*)"SAVE");
	delay(1000);
	mode = 0;
	editing_mode = false;
}

void display(void){
	String current_output = "";
	char buff[5];
	uint32_t cur_val = 0;
	float in_zivert = 0.f;
	if(menu_mode || show_mode){
		switch (mode)
		{
			case 0:{ current_output = "BACK"; } break;
			case 1:{ current_output = "PVLS"; } break;
			case 2:{ current_output = "PEAK"; } break;
			case 3:{ current_output = "D0SE"; } break;
			case 4:{ current_output = "HV0L"; } break;
			case 5:{ current_output = "CALC"; } break;
			case 6:{ current_output = "t0nE"; } break;
			case 7:{ current_output = "BAtt"; } break;
		}
	}else{
		switch (mode){
			case 0: { cur_val = rad_back; } break;		// Выбираем текущий фон
			case 1: { cur_val = rad_sum; } break;		// Выбираем число импульсов за всё время
			case 2: { cur_val = rad_max; } break;		// Выбираем максимальный фон
			case 3: { cur_val = rad_dose; } break;		// Выбираем накопленную дозу
			case 4: { cur_val = (uint32_t)pwm_converter; } break; //voltage_config() - Потом выводить на экран вольты, когда будет делитель
			case 5: { cur_val = (uint32_t)GEIGER_TIME; } break;
			case 6: { cur_val = (uint32_t)ton_BUZZ; } break;
			case 7: { cur_val = get_battery_voltage()/1000; } break;
		}

		if(cur_val > 9999){				//Переходим к микрорентгенам
			bitSet(PORTB, MRLED_PIN);		
			bitClear(PORTB, URLED_PIN);
			bitClear(PORTB, RLED_PIN);
			cur_val /= 1000;
		}else if(cur_val > 9999999){ 	//Переходим к рентгенам
			bitSet(PORTB, RLED_PIN);
			bitClear(PORTB, MRLED_PIN);
			cur_val /= 1000000;
		}else{
			bitSet(PORTB, URLED_PIN);
			bitClear(PORTB, MRLED_PIN);
		}

		if(zivert && mode != 1){
			in_zivert = cur_val/1000;
		}else{
			current_output = String(cur_val);
		}
	}
	current_output.toCharArray(buff, 5);
	tm1637.displayStr(buff);
	if(show_mode){
		delay(1000);
		show_mode = false;
	}
}

void loop() {
	if(!is_sleeping){
		display();
		signa();
		if(editing_mode && mode == 4) voltage_editing();
	}
	button_action();
}