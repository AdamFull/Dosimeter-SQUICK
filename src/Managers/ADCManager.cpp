#include <Managers/ADCManager.h>
#include <avr/io.h>

void ADCManager::adc_init(){
    ADCSRA |= (1 << ADEN)|(1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0); // Включаем АЦП, устанавливаем предделитель преобразователя на 128 
	//Изменяем параметры таймера 2 для повышения частоты шим на 3 и 11
}

float ADCManager::get_battery_voltage(){
	sensorValue = (sensorValue * (avgFactor - 1) + adc0_read()) / avgFactor;
	float voltage = 0.2 + (1125300UL / sensorValue) * 2;
	return voltage/1000;
}

unsigned ADCManager::get_hv()
{
	//ADCSRA |= (1 << ADEN);
	sensorValue = (sensorValue * (avgFactor - 1) + adc1_read()) / avgFactor;
	return (sensorValue);
	//ADCSRA &= ~(1 << ADEN);
}

int ADCManager::adc1_read(){
	ADMUX |= (0 << REFS1)|(1 << REFS0)|(1 << MUX0)|(0 << MUX1)|(0 << MUX2)|(0 << MUX3); // выставляем опорное напряжение Vcc, снимать сигнал будем с входа AC3
	do{ ADCSRA |= (1 << ADSC); } // Начинаем преобразование
	while ((ADCSRA & (1 << ADIF)) == 0); // пока не будет выставлен флаг об окончании преобразования
	return (ADCL | ADCH<<8);
}

int ADCManager::adc0_read(){
	ADMUX |=(1 << REFS0)|(0 << MUX0)|(0 << MUX1)|(0 << MUX2)|(0 << MUX3); // выставляем опорное напряжение Vcc, снимать сигнал будем с входа AC3
	do{ ADCSRA |= (1 << ADSC); } // Начинаем преобразование
	while ((ADCSRA & (1 << ADIF)) == 0); // пока не будет выставлен флаг об окончании преобразования
	return (ADCL | ADCH<<8);
}

void ADCManager::pwm_PD3(byte pwm) { OCR2B = pwm; }
void ADCManager::pwm_PB3(byte pwm) { OCR2A = pwm; }