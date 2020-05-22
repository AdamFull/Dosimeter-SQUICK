#include <Managers/DataManager.h>
#include <EEPROM.h>

void DataManager::update_rad_buffer() {
    rad_buff = new uint16_t[GEIGER_TIME];
	for(byte i = 0; i < GEIGER_TIME; i++){ rad_buff[i] = (uint16_t)0; }
	rad_back = rad_dose = rad_max = rad_sum = 0;
}

void DataManager::init(){
    if(eeprom_read_byte((uint8_t*)0b0) == 0b0) setup_eeprom();
    else{
        pwm_converter = eeprom_read_byte((uint8_t*)0b1);
	    GEIGER_TIME = eeprom_read_byte((uint8_t*)0b10);
	    ton_BUZZ = eeprom_read_byte((uint8_t*)0b11);
        update_rad_buffer();
    }
}

void DataManager::setup_eeprom(){
    eeprom_write_byte((uint8_t*)0b0, 0b1);
	eeprom_write_byte((uint8_t*)0b1, pwm_converter);
	eeprom_write_byte((uint8_t*)0b10, GEIGER_TIME);
	eeprom_write_byte((uint8_t*)0b11, ton_BUZZ);
}

void DataManager::save_voltage_config(void)
{
	eeprom_update_byte((uint8_t*)0b1, pwm_converter);
	analogWrite(11, pwm_converter);
	editing_mode = false;
}

void DataManager::save_geiger_time_config(void)
{
	eeprom_update_byte((uint8_t*)0b10, GEIGER_TIME);
	update_rad_buffer();
	editing_mode = false;
}

void DataManager::save_tone_delay(void){
	eeprom_update_byte((uint8_t*)0b11, ton_BUZZ);
	editing_mode = false;
}