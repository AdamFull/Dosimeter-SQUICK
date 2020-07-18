#include <Managers/ADCManager.h>
#include <avr/io.h>

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define bit(b) (1UL << (b))

void ADCManager::adc_init(){
    sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);
}

uint16_t ADCManager::get_battery_voltage(){
	uint16_t resulting_value = 0;
	for(int i = 0; i < 30; i++){
		batValue = (batValue * (avgFactor - 1) + adc0_read()) / avgFactor;
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
		hvValue = (hvValue * (avgFactor - 1) + adc1_read()) / avgFactor;
	}
	return hvValue;
}

uint16_t ADCManager::adc1_read(){
	uint8_t low, high;
	ADMUX = 0b01000001;
	//ADMUX = (1 << 6) | (1 & 0x07);
	sbi(ADCSRA, ADSC);
	while(bit_is_set(ADCSRA, ADSC));
	low  = ADCL;
	high = ADCH;
  	return (high << 8) | low;
}

uint16_t ADCManager::adc0_read(){
	uint8_t low, high;
	ADMUX = 0b01000000;
	//ADMUX = (1 << 6) | (0 & 0x07);
	sbi(ADCSRA, ADSC);
	while(bit_is_set(ADCSRA, ADSC));
	low  = ADCL;
	high = ADCH;
  	return (high << 8) | low;
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