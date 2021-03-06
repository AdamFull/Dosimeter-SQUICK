#include <Arduino.h>
#include <avr/power.h>
#include <Libs/GyverButton.h>

#include <Managers/OutputManager.h>
#include <macros.h>
/*
Исправить графические глюки
*/

GButton btn_reset(4, HIGH_PULL, NORM_OPEN);
GButton btn_set(5, HIGH_PULL, NORM_OPEN);

/*volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)*/

DataManager &datamgr = DataManager::getInstance();
ADCManager &adcmgr = ADCManager::getInstance();
OutputManager outmgr = OutputManager(&datamgr, &adcmgr);

void button_action(void);
void sleep(void);
void interrupt_setup(void);
void low_battery_kill(void);
void(* resetFunc) (void) = 0;

ISR(INT0_vect){ //внешнее прерывание //считаем импульсы от счетчика
	if(datamgr.counter_mode==0){    //Режим поиска	
		if(datamgr.rad_buff[0]!=65535) datamgr.rad_buff[0]++; //нулевой элемент массива - текущий секундный замер		
		#if defined(UNIVERSAL_COUNTER)
			if(++datamgr.rad_sum>999999UL*3600/datamgr.GEIGER_TIME) datamgr.rad_sum=999999UL*3600/datamgr.GEIGER_TIME; //общая сумма импульсов
		#else
			if(++datamgr.rad_sum>999999UL*3600/GEIGER_TIME) datamgr.rad_sum=999999UL*3600/GEIGER_TIME; //общая сумма импульсов
		#endif
		if(datamgr.page == 1 && !datamgr.do_alarm){ datamgr.is_detected = true; }
	}else if(datamgr.counter_mode==1){							//Режим измерения активности
		#if defined(UNIVERSAL_COUNTER)
			if(!datamgr.stop_timer) if(++datamgr.rad_back>999999UL*3600/datamgr.GEIGER_TIME) datamgr.rad_back=999999UL*3600/datamgr.GEIGER_TIME; //Сумма импульсов для режима измерения
		#else
			if(!datamgr.stop_timer) if(++datamgr.rad_back>999999UL*3600/GEIGER_TIME) datamgr.rad_back=999999UL*3600/GEIGER_TIME; //Сумма импульсов для режима измерения
		#endif
	}else if(datamgr.counter_mode==2){							//Режим измерения активности
		if(datamgr.rad_buff[0]!=65535) datamgr.rad_buff[0]++; //нулевой элемент массива - текущий секундный замеp
		outmgr.update_request();
	}
	if(datamgr.page == 1) {
		ADCManager::pwm_PD3(datamgr.pwm_converter + 10);
	}
}

/*ISR(TIMER0_OVF_vect)
{
	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;
	if (f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}*/

ISR(TIMER1_OVF_vect){ //прерывание по переполнению Timer 1
	static uint8_t cnt1;

	TCNT1=TIMER1_PRELOAD;

	if(++cnt1>=TIME_FACT){ //расчет показаний один раз в секунду
		cnt1=0;

		outmgr.update_request();

		if(datamgr.counter_mode == 0){
			uint32_t tmp_buff=0;
			#if defined(UNIVERSAL_COUNTER)
			for(uint8_t i=0; i<datamgr.GEIGER_TIME; i++) tmp_buff+=datamgr.rad_buff[i]; //расчет фона мкР/ч
			#else
			for(uint8_t i=0; i<GEIGER_TIME; i++) tmp_buff+=datamgr.rad_buff[i]; //расчет фона мкР/ч
			#endif
			if(tmp_buff>999999) tmp_buff=999999; //переполнение
			datamgr.rad_back=tmp_buff;
			datamgr.stat_buff[datamgr.stat_time] = datamgr.rad_back; //Записываю текущее значение мкр/ч для расчёта погрешности

			datamgr.calc_std();

			if(datamgr.rad_back>datamgr.rad_max) datamgr.rad_max=datamgr.rad_back; //фиксируем максимум фона

			#if defined(UNIVERSAL_COUNTER)
			for(uint8_t k=datamgr.GEIGER_TIME-1; k>0; k--) datamgr.rad_buff[k]=datamgr.rad_buff[k-1]; //перезапись массива
			#else
			for(uint8_t k=GEIGER_TIME-1; k>0; k--) datamgr.rad_buff[k]=datamgr.rad_buff[k-1]; //перезапись массива
			#endif
			
			datamgr.rad_buff[0]=0; //сбрасываем счетчик импульсов

			if(datamgr.mean_mode){
				datamgr.rad_back = (uint32_t)datamgr.mean;
			}

			#if defined(ADVANCED_ERROR)
			if(datamgr.stat_time > datamgr.GEIGER_TIME) datamgr.stat_time = 0; //Счётчик для расчёта статистической погрешности
			else datamgr.stat_time++;
			#endif

			#if defined(UNIVERSAL_COUNTER)
			datamgr.rad_dose=(datamgr.rad_sum*datamgr.GEIGER_TIME/3600); //расчитаем дозу
			#else
			datamgr.rad_dose=(datamgr.rad_sum*GEIGER_TIME/3600); //расчитаем дозу
			#endif

			#if defined(DRAW_GRAPH)
			datamgr.mass[datamgr.x_p]=map(datamgr.rad_back, 0, datamgr.rad_max < 40 ? 40 : datamgr.rad_max, 0, 15);
            if(datamgr.x_p<83)datamgr.x_p++;
            if(datamgr.x_p==83){
                for(byte i=0;i<83;i++)datamgr.mass[i]=datamgr.mass[i+1];
            }
			if(datamgr.rad_max > 1) datamgr.rad_max--;		//Потихоньку сбрасываем максимум
			#endif

		}else if(datamgr.counter_mode == 1){
			//ТАймер для второго режима. Обратный отсчёт
			bool stop_timer = datamgr.stop_timer;
			if(!stop_timer){
				if(datamgr.time_min != 0 && datamgr.time_sec == 0){
					--datamgr.time_min;
					datamgr.time_sec=60;
				} 
				if(datamgr.time_sec != 0){ --datamgr.time_sec; }
				datamgr.timer_remain--;
				if(datamgr.timer_remain == 0){
					datamgr.stop_timer = true;
					datamgr.do_alarm = true;
				}
			}
		}else if(datamgr.counter_mode == 2){
			//Секундный замер, сбрасываем счётчик каждую секунду
			if(datamgr.rad_max < datamgr.rad_buff[0]) datamgr.rad_max = datamgr.rad_buff[0];
			datamgr.sum_old=datamgr.rad_buff[0];
			#if defined(DRAW_GRAPH)
			datamgr.mass[datamgr.x_p]=map(datamgr.rad_buff[0], 0, datamgr.rad_max < 2 ? 2 : datamgr.rad_max, 0, 15);
            if(datamgr.x_p<83)datamgr.x_p++;
            if(datamgr.x_p==83){
                for(byte i=0;i<83;i++)datamgr.mass[i]=datamgr.mass[i+1];
            }
			#endif
			if(datamgr.rad_max > 1) datamgr.rad_max--;		//Потихоньку сбрасываем максимум
			datamgr.rad_buff[0]=0; //сбрасываем счетчик импульсов
		}
	}
}

void setup() {
  	//ADCSRA &= ~(1 << ADEN);  // отключаем АЦП,

	datamgr.init();

	btn_reset.setClickTimeout(100);
	btn_set.setClickTimeout(100);
	btn_reset.setTimeout(1000);
	btn_set.setTimeout(1000);

	outmgr.init();

	PORTD_MODE(0, OUTPUT);
	PORTD_WRITE(0, LOW);

	PORTD_MODE(1, OUTPUT);
	PORTD_WRITE(1, LOW);

	PORTD_MODE(2, INPUT); 						//настраиваем пин 2 (PD2) на вход, импульсы от счетчика
	PORTD_WRITE(2, LOW);

	PORTD_MODE(0, INPUT); 						//настраиваем пин 0 (PD0) на вход, заряжено
	PORTD_WRITE(0, HIGH);

	PORTD_MODE(1, INPUT); 						//настраиваем пин 1 (PD1) на вход, заряжается
	PORTD_WRITE(1, HIGH); 

	PORTD_MODE(3, OUTPUT); 						//pin 3 (PD3) как выход, блинк при засекании частицы
	PORTD_WRITE(3, LOW);

	PORTD_MODE(5, OUTPUT); 						//pin 5 (PD5) как выход, звуковая индикация частицы
	PORTD_WRITE(5, LOW);

	PORTB_MODE(3, OUTPUT); 						//pin 11 (PB3) как выход, уаравление подсветкой
	PORTB_WRITE(3, LOW);

	PORTC_MODE(2, OUTPUT);						//pin A2 (PC2) как выход, земля экрана
	PORTC_WRITE(2, LOW);

	PORTC_MODE(3, OUTPUT); 						//pin A3 (PC3) как выход, замля повторителя
	PORTC_WRITE(3, LOW);


	PORTC_WRITE(3, HIGH);						//Включить экран
	PORTC_WRITE(2, HIGH);						//Включить эмиттерный повторитель
	
	interrupt_setup();

	EICRA=0b00000010; //настриваем внешнее прерывание 0 по спаду
	EIMSK |= (1 << INT0); //разрешаем внешнее прерывание 0


	datamgr.battery_voltage = adcmgr.get_battery_voltage();
	if(datamgr.battery_voltage < BAT_ADC_MIN) datamgr.low_voltage = true;

	datamgr.counter_mode = 0;
}

#if defined(CAN_SLEEP)
void sleep(){
	if(!datamgr.is_sleeping){
		ADCManager::pwm_PB3(0);		//Отключить шим на экран
		//TODO: Снизить частоту процессора до 1 мгц
		datamgr.update_rad_buffer();
		
		datamgr.is_sleeping = true;

		PORTC_WRITE(3, LOW);						//Выключить экран
	}else{
		datamgr.is_sleeping = false;
		PORTC_WRITE(3, HIGH);
		//resetFunc();
	}
	
}
#endif

void interrupt_setup(){
	cli();
	ACSR |= 1 << ACD; //отключаем компаратор

	//_SFR_BYTE(TCCR0B) |= _BV(CS01);
	//_SFR_BYTE(TCCR0B) |= _BV(CS00);
	//_SFR_BYTE(TCCR0A) |= _BV(CS01);
	//_SFR_BYTE(TCCR0A) |= _BV(CS00);

	//настраиваем Timer 1
	TIMSK1=0; //отключить таймер
	TCCR1A=0; //OC1A/OC1B disconnected
	TCCR1B=0b00000101; //предделитель 16M/1024=15625кГц
	TCNT1=TIMER1_PRELOAD;

	TIMSK1=0b00000001; //запускаем Timer 1
	TIMSK1 |= (1 << TOIE1);

	//4khz
	TCCR2B = 0b00000010;  // x8
    TCCR2A = 0b00000001;  // phase correct

	sei();
}

void low_battery_kill(){
	ADCManager::pwm_PD3(0);
	ADCManager::pwm_PB3(0);
	PORTC_WRITE(2, LOW);
	datamgr.page = 3;
	while(true){
		datamgr.is_charging = !PORTD_READ(1);
		if(datamgr.is_charging) {
			PORTC_WRITE(2, HIGH);
			break;
		}
		outmgr.update();
	}
}

void button_action(){
	btn_reset.tick();
	btn_set.tick();

	bool btn_reset_isHolded = btn_reset.isHolded();
	bool btn_set_isHolded = btn_set.isHolded();

	bool menu_mode = datamgr.page == 2;
	bool editing_mode = datamgr.editing_mode;

	if(btn_reset.isHold() && btn_set.isHold()){
		if(!menu_mode){
			datamgr.page = 2;
			datamgr.menu_page = 0;
			datamgr.editing_mode = false;
		}else{
			datamgr.editing_mode = false;
			datamgr.page = 1;
		}
		outmgr.update_request();
		btn_reset.resetStates();
		btn_set.resetStates();
	}else if(btn_set.isHold() && !menu_mode){
		if(!menu_mode && !btn_reset.isPress()) outmgr.battery_request(true);
	}else if(btn_reset_isHolded){											//Удержание кнопки ресет
		if(!menu_mode && !btn_set.isPress()) datamgr.no_alarm = !datamgr.no_alarm;
		if(menu_mode && !editing_mode){										//Если находимся в меню
			datamgr.is_detected = true;
			if(datamgr.menu_page == 0) {datamgr.page = 1; datamgr.do_alarm = false;}
			else if(datamgr.menu_page == 6) datamgr.menu_page = 2;
			else if(datamgr.menu_page == 7) datamgr.menu_page = 6;
			else datamgr.menu_page = 0;
			datamgr.cursor = 0;
		}
		if(editing_mode){
			datamgr.is_detected = true;
			datamgr.editing_mode = false;
		}
		if(!menu_mode && datamgr.counter_mode == 1){
			datamgr.reset_activity_test();
			datamgr.timer_remain = datamgr.timer_time;
			datamgr.time_min = datamgr.time_min_old;
		}
		outmgr.update_request();
	}else if(btn_reset.isClick() && !btn_reset_isHolded){					//Клик кнопки ресет
		if(!menu_mode && !datamgr.do_alarm) datamgr.mute = !datamgr.mute;
		if(!menu_mode && datamgr.do_alarm) datamgr.no_alarm = !datamgr.no_alarm;
		if(menu_mode && !editing_mode && datamgr.cursor > 0) { datamgr.is_detected = true; datamgr.cursor--; }
		if(editing_mode){ 
			if(datamgr.menu_page == 2){
				#if defined(UNIVERSAL_COUNTER)
				if(datamgr.cursor == 1 && datamgr.editable > 0) datamgr.editable-=5;
				if(datamgr.cursor == 2 && datamgr.editable > 0) datamgr.editable-=51;
				if(datamgr.cursor == 3 && datamgr.editable > 0) datamgr.editable-=5;
				if(datamgr.cursor == 4 && datamgr.editable > 0) datamgr.editable--;
				if(datamgr.cursor == 5 && datamgr.editable > 30) datamgr.editable-=5;
				#else
				if(datamgr.cursor == 0 && datamgr.editable > 0) datamgr.editable-=5;
				if(datamgr.cursor == 1 && datamgr.editable > 0) datamgr.editable-=51;
				if(datamgr.cursor == 2 && datamgr.editable > 0) datamgr.editable-=5;
				if(datamgr.cursor == 3 && datamgr.editable > 0) datamgr.editable--;
				if(datamgr.cursor == 4 && datamgr.editable > 0) datamgr.editable-=5;
				#endif
			}else if(datamgr.menu_page == 4){
				switch (datamgr.cursor){
					case 0:{ if(datamgr.editable > 1) datamgr.editable--; }break;
					case 1:{ if(datamgr.editable > 0) datamgr.editable--; }break;
				}
			}
			#if defined(UNIVERSAL_COUNTER)
			else if(datamgr.menu_page == 7){
				switch (datamgr.cursor){
					case 0:{ if(datamgr.editable > 1) datamgr.editable--; } break;
					case 1:{ if(datamgr.editable > 1) datamgr.editable--; } break;
				}
			}
			#endif
		}
		outmgr.update_request();
	}else if(btn_set_isHolded){												//Удержание кнопки сет
		if(datamgr.is_sleeping) sleep();
		if(menu_mode && !editing_mode) {
			datamgr.is_detected = true;
			switch (datamgr.menu_page){
				case 0:{
					switch (datamgr.cursor){
						case 0:{ datamgr.menu_page = 1; }break;
						case 1:{ datamgr.menu_page = 2; }break;
						case 2:{ datamgr.menu_page = 3; }break;
						#if defined(CAN_SLEEP)
						case 3:{ datamgr.menu_page = 5; }break;
						#endif
					}
					datamgr.cursor = 0;
				}break;
				case 1:{
					switch (datamgr.cursor){
						case 0:{ datamgr.counter_mode = 0; datamgr.page = 1; }break;
						case 1:{ datamgr.menu_page = 4; }break;
						case 2:{ datamgr.counter_mode = 2; datamgr.page = 1; datamgr.rad_max = 0; 
						#if defined(DRAW_GRAPH)
							for(int i = 0; i < 83; i++) datamgr.mass[i] = 0;
						#endif
						}break;
					}
					datamgr.cursor = 0;
				}break;
				case 2:{
					switch (datamgr.cursor){
						#if defined(UNIVERSAL_COUNTER)
						case 0:{ datamgr.menu_page = 6; }break;
						case 1:{ datamgr.editable = datamgr.ton_BUZZ; }break;
						case 2:{ datamgr.editable = datamgr.backlight; }break;
						case 3:{ datamgr.editable = datamgr.contrast; }break;
						case 4:{ datamgr.editable = datamgr.save_dose_interval; }break;
						case 5:{ datamgr.editable = datamgr.alarm_threshold; }break;
						#else
						case 0:{ datamgr.editable = datamgr.ton_BUZZ; }break;
						case 1:{ datamgr.editable = datamgr.backlight; }break;
						case 2:{ datamgr.editable = datamgr.contrast; }break;
						case 3:{ datamgr.editable = datamgr.save_dose_interval; }break;
						case 4:{ datamgr.editable = datamgr.alarm_threshold; }break;
						#endif
					}
					#if defined(UNIVERSAL_COUNTER)
					if(datamgr.cursor != 0) datamgr.editing_mode = true;
					#else
					datamgr.editing_mode = true;
					#endif
				}break;
				case 3:{
					switch (datamgr.cursor){								//Стереть данные
						case 0:{ datamgr.reset_settings(); datamgr.menu_page = 0; }break;
						case 1:{ datamgr.reset_dose(); datamgr.menu_page = 0; }break;
						case 2:{ datamgr.reset_settings(); datamgr.reset_dose(); datamgr.menu_page = 0; }break;
					}
					datamgr.cursor = 0;
				}break;
				case 4:{
					switch (datamgr.cursor){
						case 0:{ datamgr.editable = datamgr.time_min; }break;
						case 1:{ datamgr.editable = datamgr.means_times; }break;
						case 2:{
							datamgr.reset_activity_test();
							datamgr.time_min_old = datamgr.time_min;
							datamgr.timer_time = datamgr.time_min * 60;
							datamgr.timer_remain = datamgr.timer_time;
						}break;
					}
					if(datamgr.cursor != 2) datamgr.editing_mode = true;
					
				}break;
				case 5:{
					switch (datamgr.cursor){								//Вообще это диалог выбора, но пока что это не он
						case 0:{
						#if defined(CAN_SLEEP)
						sleep(); 
						#endif
						}break;
						case 1:{ datamgr.menu_page = 0; }break;
					}
					datamgr.cursor = 0;
				}break;
				#if defined(UNIVERSAL_COUNTER)
				case 6:{
					switch (datamgr.cursor){
						case 0:{ datamgr.menu_page = 2; datamgr.setup_sbm20(); }break;
						case 1:{ datamgr.menu_page = 2; datamgr.setup_sbm19(); }break;
						case 2:{ datamgr.menu_page = 2; datamgr.setup_beta(); }break;
						case 3:{ datamgr.menu_page = 7; }break;
					}
					datamgr.cursor = 0;
				}break;
				case 7:{
					switch (datamgr.cursor){
						case 0:{ datamgr.editable = datamgr.GEIGER_TIME; }break;
						case 1:{ datamgr.editable = datamgr.geiger_error; }break;
					}
					datamgr.editing_mode = true;
				}break;
				#endif
			}
		}
		if(menu_mode && editing_mode){
			datamgr.is_detected = true;
			if(datamgr.menu_page == 4){
				switch (datamgr.cursor){
					case 0:{ datamgr.time_min = datamgr.editable; }break;
					case 1:{ datamgr.means_times = datamgr.editable; }break;
				}
			}
			#if defined(UNIVERSAL_COUNTER)
			else if(datamgr.menu_page == 7){
				switch (datamgr.cursor){
					case 0:{ datamgr.save_time(); }break;
					case 1:{ datamgr.save_error(); }break;
				}
			}
			#endif
			else{
				switch (datamgr.cursor){
					#if defined(UNIVERSAL_COUNTER)
					case 1:{ datamgr.save_tone(); }break;
					case 2:{ datamgr.save_bl(); }break;
					case 3:{ datamgr.save_contrast(); }break;
					case 4:{ datamgr.save_interval(); }break;
					case 5:{ datamgr.save_alarm(); }break;
					#else
					case 0:{ datamgr.save_tone(); }break;
					case 1:{ datamgr.save_bl(); }break;
					case 2:{ datamgr.save_contrast(); }break;
					case 3:{ datamgr.save_interval(); }break;
					case 4:{ datamgr.save_alarm(); }break;
					#endif
				}
			}
			datamgr.editing_mode = false;
		}
		outmgr.update_request();
	}else if(btn_set.isClick() && !btn_set_isHolded){					//Клик кнопки сет
		if(!menu_mode && datamgr.counter_mode == 1 && !datamgr.next_step && datamgr.stop_timer){
			datamgr.rad_max = datamgr.rad_back;
			datamgr.rad_back = 0;
			datamgr.next_step = true;
			datamgr.stop_timer = false;
			datamgr.do_alarm = false;
			datamgr.time_min = datamgr.time_min_old;
			datamgr.timer_remain = datamgr.timer_time;
			datamgr.time_sec = 0;
		}
		if(!menu_mode && datamgr.counter_mode == 1 && datamgr.do_alarm && datamgr.next_step && datamgr.stop_timer){
			datamgr.do_alarm = false;
		}

		if(!menu_mode && datamgr.counter_mode == 0){
			datamgr.mean_mode = !datamgr.mean_mode;
		}

		if(menu_mode && !editing_mode){						//Сдвинуть курсор, если можно
			datamgr.is_detected = true;
			switch (datamgr.menu_page){
				#if defined(CAN_SLEEP)
				case 0:{ if(datamgr.cursor < 3) datamgr.cursor++; } break;
				#else
				case 0:{ if(datamgr.cursor < 2) datamgr.cursor++; } break;
				#endif
				case 1:{ if(datamgr.cursor < 2) datamgr.cursor++; } break;
				#if defined(UNIVERSAL_COUNTER)
				case 2:{ if(datamgr.cursor < 5) datamgr.cursor++; } break;
				#else
				case 2:{ if(datamgr.cursor < 2) datamgr.cursor++; } break;
				#endif
				case 3:{ if(datamgr.cursor < 2) datamgr.cursor++; } break;
				case 4:{ if(datamgr.cursor < 2) datamgr.cursor++; } break;
				#if defined(CAN_SLEEP)
				case 5:{ if(datamgr.cursor < 1) datamgr.cursor++; } break;
				#endif
				#if defined(UNIVERSAL_COUNTER)
				case 6:{ if(datamgr.cursor < 3) datamgr.cursor++; } break;
				case 7:{ if(datamgr.cursor < 1) datamgr.cursor++; } break;
				#endif
			}
		}
		if(editing_mode){ 
			if(datamgr.menu_page == 2){
				#if defined(UNIVERSAL_COUNTER)
				if(datamgr.cursor == 1 && datamgr.editable < 255) datamgr.editable+=5;
				if(datamgr.cursor == 2 && datamgr.editable < 255) datamgr.editable+=51;
				if(datamgr.cursor == 3 && datamgr.editable < 255) datamgr.editable+=5;
				if(datamgr.cursor == 4 && datamgr.editable < 255) datamgr.editable++;
				if(datamgr.cursor == 5 && datamgr.editable < 255) datamgr.editable+=5;
				#else
				if(datamgr.cursor == 0 && datamgr.editable < 255) datamgr.editable+=5;
				if(datamgr.cursor == 1 && datamgr.editable < 255) datamgr.editable+=51;
				if(datamgr.cursor == 2 && datamgr.editable < 255) datamgr.editable+=5;
				if(datamgr.cursor == 3 && datamgr.editable < 255) datamgr.editable++;
				if(datamgr.cursor == 4 && datamgr.editable < 255) datamgr.editable+5;
				#endif
			}else if(datamgr.menu_page == 4){
				switch (datamgr.cursor){
					case 0:{ datamgr.editable++; }break;
					case 1:{ if(datamgr.editable < 1) datamgr.editable++; }break;
				}
			}
			#if defined(UNIVERSAL_COUNTER)
			else if(datamgr.menu_page == 7){
				switch (datamgr.cursor){
					case 0:{ if(datamgr.editable < 100) datamgr.editable++; } break;
					case 1:{ if(datamgr.editable < 40) datamgr.editable++; } break;
				}
			}
			#endif
		}
		outmgr.update_request();
	}
}

//При редактировании применяет текущие значения
void mode_handler(){
	if(datamgr.page == 2){
		if(datamgr.editing_mode){
			if(datamgr.menu_page == 2){
				switch (datamgr.cursor){
					case 1:{} break;
					case 2:{ ADCManager::pwm_PB3(datamgr.editable); } break;
					case 3:{ outmgr.set_contrast(datamgr.editable); } break;
				}
			}
		}

	}
}

unsigned long charge_state = 0;

void loop() {
	datamgr.is_charging = !PORTD_READ(1);
	datamgr.is_charged = !PORTD_READ(0);

	if((datamgr.battery_voltage < BAT_ADC_MIN) && !datamgr.low_voltage) datamgr.low_voltage = true;
	if(datamgr.low_voltage) low_battery_kill();

	if(!datamgr.is_charging){
		if(adcmgr.get_hv() < HV_ADC_REQ) { datamgr.pwm_converter++; }
		else { datamgr.pwm_converter--; }
		ADCManager::pwm_PD3(datamgr.pwm_converter);
		if((datamgr.rad_back > datamgr.alarm_threshold) && !datamgr.no_alarm && (datamgr.counter_mode == 0)) { datamgr.do_alarm = true; }
		else { if(datamgr.counter_mode == 0) datamgr.do_alarm = false; }
		if(!datamgr.do_alarm) ADCManager::pwm_PB3(datamgr.backlight);

		if(!datamgr.no_alarm) {
			if(datamgr.do_alarm){
				if(millis()-datamgr.alarm_timer > 300){
					datamgr.alarm_timer = millis();
					ADCManager::pwm_PD5(datamgr.alarm ? 100 : 200);
					ADCManager::pwm_PB3(datamgr.alarm ? 204 : 0);
					datamgr.alarm = !datamgr.alarm;
				}
			}
		}

		if(outmgr.get_battery_requet()) outmgr.update_request();
		outmgr.battery_request(false);
		button_action();

		if(datamgr.counter_mode==0){
			if(datamgr.rad_dose - datamgr.rad_dose_old > datamgr.save_dose_interval){
				datamgr.rad_dose_old = datamgr.rad_dose;
				datamgr.save_dose();
				datamgr.rad_max = datamgr.rad_back;
			}
		}
	}else{
		ADCManager::pwm_PD3(0);
		ADCManager::pwm_PB3(0);
	}
	

	if(!datamgr.is_sleeping){
		mode_handler();
		outmgr.update();
	}else{
		if(datamgr.rad_back > 20) sleep();
	}
}