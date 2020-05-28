#include <Managers/DataManager.h>
#include <EEPROM.h>

void DataManager::update_rad_buffer() {
	delete(rad_buff);
	delete(rad_buff_back);
    rad_buff = new uint16_t[GEIGER_TIME];
	rad_buff_back = new uint16_t[GEIGER_TIME];
	for(byte i = 0; i < GEIGER_TIME; i++){ rad_buff[i] = (uint16_t)0; }
	for(byte i = 0; i < GEIGER_TIME; i++){ rad_buff_back[i] = (uint16_t)0; }
	rad_back = rad_max = 0;
	rad_dose = rad_dose_old;
	time_sec = time_min = time_hrs = 0;
	rad_sum_mens = rad_sum_mens_old = 0;
}

void DataManager::init(){
	EEPROM.get(0b0, has_eeprom);
    if(eeprom_read_byte((uint8_t*)0b0) == 0b0) setup_eeprom();
    else read_eeprom();
}

void DataManager::read_eeprom(){
	EEPROM.get(0b1, pwm_converter);
	EEPROM.get(0b10, ton_BUZZ);
	EEPROM.get(0b11, backlight);
	EEPROM.get(0b100, contrast);
	EEPROM.get(0b101, GEIGER_TIME);
	EEPROM.get(0b1001, rad_sum);
    update_rad_buffer();
}

void DataManager::setup_eeprom(){
	EEPROM.put(0b0, has_eeprom);
	EEPROM.put(0b1, pwm_converter);
	EEPROM.put(0b10, ton_BUZZ);
	EEPROM.put(0b11, backlight);
	EEPROM.put(0b100, contrast);
	EEPROM.put(0b101, GEIGER_TIME);
	EEPROM.put(0b1001, rad_sum);
}

void DataManager::save_dose(void){
	EEPROM.put(0b1001, rad_sum);
}

void DataManager::save_pwm(void){
	pwm_converter = editable;
	EEPROM.put(0b1, pwm_converter);
	analogWrite(3, pwm_converter);
}

void DataManager::save_tone(void){
	ton_BUZZ = editable;
	EEPROM.put(0b10, ton_BUZZ);
}

void DataManager::save_bl(void){
	backlight = editable;
	EEPROM.put(0b11, backlight);
	analogWrite(11, !backlight);
}

void DataManager::save_contrast(void){
	contrast = editable;
	EEPROM.put(0b100, contrast);
}

void DataManager::save_time(void){
	GEIGER_TIME = editable;
	EEPROM.put(0b101, GEIGER_TIME);
	update_rad_buffer();
}

void DataManager::reset_dose(void){
	rad_sum = 0;
	EEPROM.put(0b110, 0);
}

void DataManager::reset_settings(void){
	EEPROM.put(0b0, 0b1);
	EEPROM.put(0b1, 45);
	EEPROM.put(0b10, 200);
	EEPROM.put(0b11, 0);
	EEPROM.put(0b100, 60);
	EEPROM.put(0b101, 37);
	read_eeprom();
	update_rad_buffer();
	analogWrite(3, pwm_converter);
}

void DataManager::reset_activity_test(){
	rad_sum_mens_old = 0;
	rad_sum_mens = 0;
	stop_timer = false;
	next_step = false;
	time_mens_sec = 0;
	time_mens_min = 0;
}

void DataManager::calc_mean(){
	uint32_t sum = 0;
	for(unsigned i = 0; i < GEIGER_TIME; i++) sum+=rad_buff_back[i];
	mean = (float)sum/GEIGER_TIME;
}

void DataManager::calc_std(){
	uint32_t sum = 0;
	for(unsigned i = 0; i < GEIGER_TIME; i++) sum+=pow(rad_buff_back[i] - mean, 2);
	std = sqrt(sum/GEIGER_TIME-1);
	if(std > 65535) std = 0;
}