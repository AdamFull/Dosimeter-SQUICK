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
		backlight = eeprom_read_byte((uint8_t*)0b100);
		//contrast = eeprom_read_byte((uint8_t*)0b101);
        update_rad_buffer();
    }
}

void DataManager::setup_eeprom(){
    eeprom_write_byte((uint8_t*)0b0, 0b1);
	eeprom_write_byte((uint8_t*)0b1, pwm_converter);
	eeprom_write_byte((uint8_t*)0b10, GEIGER_TIME);
	eeprom_write_byte((uint8_t*)0b11, ton_BUZZ);
	eeprom_write_byte((uint8_t*)0b100, backlight);
	eeprom_write_byte((uint8_t*)0b101, contrast);
}

void DataManager::save_all(void)
{
	eeprom_update_byte((uint8_t*)0b1, pwm_converter);
	eeprom_update_byte((uint8_t*)0b10, GEIGER_TIME);
	eeprom_update_byte((uint8_t*)0b11, ton_BUZZ);
	eeprom_update_byte((uint8_t*)0b100, backlight);
	eeprom_update_byte((uint8_t*)0b101, contrast);
	update_rad_buffer();
	analogWrite(3, pwm_converter);
}

void DataManager::reset_settings(void){
	eeprom_update_byte((uint8_t*)0b1, 45);
	eeprom_update_byte((uint8_t*)0b10, 37);
	eeprom_update_byte((uint8_t*)0b11, 200);
	eeprom_update_byte((uint8_t*)0b100, 0);
	eeprom_update_byte((uint8_t*)0b101, 50);
	update_rad_buffer();
	analogWrite(3, pwm_converter);
}