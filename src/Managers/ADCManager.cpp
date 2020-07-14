#include <Managers/ADCManager.h>
#include <avr/io.h>
#include <Arduino.h>

void ADCManager::adc_init(){
    //ADCSRA |= (1 << ADEN)|(1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0); // Включаем АЦП, устанавливаем предделитель преобразователя на 128 
	//Изменяем параметры таймера 2 для повышения частоты шим на 3 и 11
}

uint8_t ADCManager::get_battery_voltage(){
	for(int i = 0; i < 15; i++) sensorValue = (sensorValue * (avgFactor - 1) + adc0_read()) / avgFactor;
	return sensorValue;
}

uint16_t ADCManager::get_hv()
{
  	static uint16_t sum = 0;  // сумма
	for(int i = 0; i < 15; i++){
		for(int j = 0; j < 15; j++){
			sum += adc1_read();
		}
		sensorValue = (sensorValue * (avgFactor - 1) + sum / 30) / avgFactor;
	}
	return sensorValue;
}

byte ADCManager::adc1_read(){
	ADMUX = 0b11100001;//выбор внутреннего опорного 1,1В и А1
  	ADCSRA = 0b11100111;
  	_delay_us(20);
	ADCSRA |= 0x10;
	byte result = ADCH;
	//ADCSRA &= ~(1 << ADEN);
	return result;
}

byte ADCManager::adc0_read(){
	ADMUX = 0b11100000;//выбор внутреннего опорного 1,1В и А1
  	ADCSRA = 0b11100111;
  	_delay_us(20);
	while ((ADCSRA & 0x10) == 0);
	ADCSRA |= 0x10;
	byte result = ADCH;
	//ADCSRA &= ~(1 << ADEN);
	return result;
}

void ADCManager::pwm_PD3(byte pwm) { OCR2B = pwm; }
void ADCManager::pwm_PB3(byte pwm) { OCR2A = pwm; }