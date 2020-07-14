#include <Managers/ADCManager.h>
#include <avr/io.h>
#include <Arduino.h>

void ADCManager::adc_init(){
    //ADCSRA |= (1 << ADEN)|(1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0); // Включаем АЦП, устанавливаем предделитель преобразователя на 128 
	//Изменяем параметры таймера 2 для повышения частоты шим на 3 и 11
}

uint16_t ADCManager::get_battery_voltage(){
	for(int i = 0; i < 30; i++){
		batValue = (batValue * (avgFactor - 1) + analogRead(A0)) / avgFactor;
	}
	return batValue;
}

uint16_t ADCManager::get_hv()
{
	for(int i = 0; i < 30; i++){
		hvValue = (hvValue * (avgFactor - 1) + analogRead(A1)) / avgFactor;
	}
	return hvValue;
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