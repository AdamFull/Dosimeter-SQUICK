#include <Managers/ADCManager.h>
#include <avr/io.h>
#include <Arduino.h>

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void ADCManager::adc_init(){
    //ADCSRA |= (1 << ADEN)|(1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0); // Включаем АЦП, устанавливаем предделитель преобразователя на 128 
	//Изменяем параметры таймера 2 для повышения частоты шим на 3 и 11
}

uint16_t ADCManager::get_battery_voltage(){
	uint16_t resulting_value = 0;
	for(int i = 0; i < 30; i++){
		batValue = (batValue * (avgFactor - 1) + analogRead(A0)) / avgFactor;
	}
	if(first_mean){
		for(uint8_t i = 0; i < BAT_BANK_SIZE; i++) battery_bank[i] = batValue;
		first_mean = false;
	}else{
		battery_bank[0] = batValue;
		for(uint8_t k = BAT_BANK_SIZE-1; k>0; k--) battery_bank[k]=battery_bank[k-1]; //перезапись массива
	}
	for(uint8_t i = 0; i < BAT_BANK_SIZE; i++) resulting_value += battery_bank[i];
	resulting_value = resulting_value/BAT_BANK_SIZE;
	
	return resulting_value;
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

void ADCManager::pwm_PD3(byte pwm) {
	sbi(TCCR2A, COM2B1);
	OCR2B = pwm;
}
void ADCManager::pwm_PB3(byte pwm) { 
	sbi(TCCR2A, COM2A1);
	OCR2A = pwm; 
}

void ADCManager::pwm_PD5(byte pwm){
	sbi(TCCR0A, COM0B1);
	OCR0B = pwm;
}