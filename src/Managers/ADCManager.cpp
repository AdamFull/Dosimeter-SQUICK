#include <Managers/ADCManager.h>
#include <avr/io.h>
#include <Arduino.h>

void ADCManager::adc_init(){
    //ADCSRA |= (1 << ADEN)|(1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0); // Включаем АЦП, устанавливаем предделитель преобразователя на 128 
	//Изменяем параметры таймера 2 для повышения частоты шим на 3 и 11
}

float ADCManager::get_battery_voltage(){
	sensorValue = (sensorValue * (avgFactor - 1) + adc0_read()) / avgFactor;
	float voltage = 0.2 + (1125300UL / sensorValue) * 2;
	return voltage/1000;
}

uint16_t ADCManager::get_hv()
{
	static byte counter = 0;     // счётчик
  	static uint16_t prevResult = 0; // хранит предыдущее готовое значение
  	static uint16_t sum = 0;  // сумма
  	sum += adc1_read();   // суммируем новое значение
	counter++;       // счётчик++
	if (counter == 30) {      // достигли кол-ва измерений
		prevResult = sum / 30;  // считаем среднее
		sum = 0;                      // обнуляем сумму
		counter = 0;                  // сброс счётчика
	}
	sensorValue = (sensorValue * (avgFactor - 1) + prevResult) / avgFactor;
	return ((float)sensorValue / (float)255)*515;
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