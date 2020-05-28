#include <Managers/DataManager.h>
#include <EEPROM.h>
#include <logo.h>

void DataManager::update_rad_buffer() {
	delete []rad_buff;
	delete []rad_buff_back;
    rad_buff = new uint16_t[GEIGER_TIME];
	rad_buff_back = new uint16_t[GEIGER_TIME];
	for(unsigned i = 0; i < GEIGER_TIME; i++){ rad_buff[i] = 0; rad_buff_back[i] = 0; }
	rad_back = rad_max = 0;
	rad_dose = rad_dose_old;
	time_sec = time_min = time_hrs = 0;
	rad_sum_mens = rad_sum_mens_old = 0;
	get_quantile();
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
	EEPROM.get(0b1110, geiger_error);
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
	EEPROM.put(0b1110, geiger_error);
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

void DataManager::save_error(void){
	geiger_error = editable;
	EEPROM.put(0b1110, geiger_error);
	update_rad_buffer();
}

void DataManager::reset_dose(void){
	rad_sum = 0;
	EEPROM.put(0b110, 0);
}

void DataManager::reset_settings(void){
	EEPROM.put(0b0, (byte)0b1);
	EEPROM.put(0b1, (byte)45);
	EEPROM.put(0b10, (byte)200);
	EEPROM.put(0b11, (byte)0);
	EEPROM.put(0b100, (byte)60);
	EEPROM.put(0b101, (byte)37);
	EEPROM.put(0b1110, (float)5.f);
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

void DataManager::calc_std(){
	uint32_t mean_sum = 0;
	for(unsigned i = 0; i < GEIGER_TIME; i++) mean_sum+=rad_buff_back[i];
	mean = (float)mean_sum/GEIGER_TIME;
	uint32_t std_sum = 0;
	for(unsigned i = 0; i < GEIGER_TIME; i++) std_sum+=pow(rad_buff_back[i] - mean, 2);
	std = sqrt(std_sum/GEIGER_TIME-1);
	//if(std > 65535) std = 0;
}

void DataManager::get_quantile(){
	int true_percent = 100 - geiger_error;
	byte required_k =  GEIGER_TIME - 1;
	if(required_k >=50 && required_k < 100)	required_k = 49;
	else if(required_k >=100 && required_k < 255) required_k = 50;
	else if(required_k > 100) required_k = 51;

	if(true_percent < 70){
		tinv_value = (pgm_read_word(&t_quantile[required_k][0])/2)/100.f;
	}else if(true_percent >= 70 && true_percent < 80){
		tinv_value = pgm_read_word(&t_quantile[required_k][0])/100.f;
	}else if(true_percent >= 80 && true_percent < 90){
		tinv_value = pgm_read_word(&t_quantile[required_k][1])/100.f;
	}else if(true_percent >= 90 && true_percent < 95){
		tinv_value = pgm_read_word(&t_quantile[required_k][2])/100.f;
	}else if(true_percent >= 95 && true_percent < 98){
		tinv_value = pgm_read_word(&t_quantile[required_k][3])/100.f;
	}else if(true_percent == 98){
		tinv_value = pgm_read_word(&t_quantile[required_k][4])/100.f;
	}else if(true_percent > 98){
		tinv_value = pgm_read_word(&t_quantile[required_k][5])/100.f;
	}
}