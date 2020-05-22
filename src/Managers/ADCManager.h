#pragma once
#include <Arduino.h>

class ADCManager{
    public:
        inline static void adc_init(){
            ADCSRA |= (1 << ADEN)|(1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0); // Включаем АЦП, устанавливаем предделитель преобразователя на 128 
        }

        inline static int adc0_read(){
	        ADMUX |=(1 << REFS0)|(0 << MUX0)|(0 << MUX1)|(0 << MUX2)|(0 << MUX3); // выставляем опорное напряжение Vcc, снимать сигнал будем с входа AC3
	        do{ ADCSRA |= (1 << ADSC); } // Начинаем преобразование
	        while ((ADCSRA & (1 << ADIF)) == 0); // пока не будет выставлен флаг об окончании преобразования
	        return (ADCL | ADCH<<8);
        }

        inline static int adc1_read(){
	        ADMUX |= (0 << REFS1)|(1 << REFS0)|(1 << MUX0)|(0 << MUX1)|(0 << MUX2)|(0 << MUX3); // выставляем опорное напряжение Vcc, снимать сигнал будем с входа AC3
	        do{ ADCSRA |= (1 << ADSC); } // Начинаем преобразование
	        while ((ADCSRA & (1 << ADIF)) == 0); // пока не будет выставлен флаг об окончании преобразования
	        return (ADCL | ADCH<<8);
        }
};