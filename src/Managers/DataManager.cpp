#include <Managers/DataManager.h>
#include <EEPROM.h>

void DataManager::update_rad_buffer() {
	#if defined(UNIVERSAL_COUNTER)
	delete []rad_buff;
	delete []stat_buff;
    rad_buff = new uint16_t[GEIGER_TIME];
	stat_buff = new uint32_t[GEIGER_TIME];
	for(unsigned i = 0; i < GEIGER_TIME; i++){ rad_buff[i] = 0;}
	for(unsigned i = 0; i < GEIGER_TIME; i++){ stat_buff[i] = 0;}
	#endif
	rad_back = rad_max = 0;
	rad_dose = rad_dose_old;
	time_sec = time_min = 0;
	time_min = 1;
	#if defined(DRAW_GRAPH)
	for(int i = 0; i < 83; i++) mass[i] = 0;
	#endif
}

void DataManager::init(){
	EEPROM.get(0b0, has_eeprom);
    if(has_eeprom == 0 || has_eeprom == 255) setup_eeprom();
    else read_eeprom();
	if(pwm_converter > 250){
		setup_eeprom();
		read_eeprom();
	}
}

void DataManager::read_eeprom(){
	#if defined(UNIVERSAL_COUNTER)
	EEPROM.get(0b1, pwm_converter);
	EEPROM.get(0b101, GEIGER_TIME);
	EEPROM.get(0b1110, geiger_error);
	#endif
	EEPROM.get(0b10, ton_BUZZ);
	EEPROM.get(0b11, backlight);
	EEPROM.get(0b100, contrast);
	EEPROM.get(0b1001, rad_sum);
    update_rad_buffer();
}

void DataManager::setup_eeprom(){
	#if defined(UNIVERSAL_COUNTER)
	EEPROM.put(0b1, (byte)60);
	EEPROM.put(0b101, (byte)21);
	EEPROM.put(0b1110, (byte)2);
	#endif
	EEPROM.put(0b0, (byte)1);
	EEPROM.put(0b10, (byte)250);
	EEPROM.put(0b11, (byte)0);
	EEPROM.put(0b100, (byte)60);
	
	EEPROM.put(0b1001, (byte)0);
}

void DataManager::save_dose(void){
	EEPROM.put(0b1001, rad_sum);
}

#if defined(UNIVERSAL_COUNTER)
void DataManager::save_pwm(void){
	pwm_converter = editable;
	EEPROM.put(0b1, pwm_converter);
	analogWrite(3, pwm_converter);
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

void DataManager::reset_settings(void){
	EEPROM.put(0b0, (byte)0b1);
	EEPROM.put(0b1, (byte)71);
	EEPROM.put(0b10, (byte)200);
	EEPROM.put(0b11, (byte)0);
	EEPROM.put(0b100, (byte)60);
	EEPROM.put(0b101, (byte)37);
	EEPROM.put(0b1110, (byte)5);
	read_eeprom();
	update_rad_buffer();
	analogWrite(3, pwm_converter);
}

void DataManager::setup_sbm20(){
	EEPROM.put(0b1, (byte)60);
	EEPROM.put(0b101, (byte)37);
	EEPROM.put(0b1110, (byte)5);
	read_eeprom();
	analogWrite(3, pwm_converter);
}
void DataManager::setup_sbm19(){
	EEPROM.put(0b1, (byte)60);
	EEPROM.put(0b101, (byte)25);
	EEPROM.put(0b1110, (byte)4);
	read_eeprom();
	analogWrite(3, pwm_converter);
}
void DataManager::setup_beta(){
	EEPROM.put(0b1, (byte)60);
	EEPROM.put(0b101, (byte)21);
	EEPROM.put(0b1110, (byte)2);
	read_eeprom();
	analogWrite(3, pwm_converter);
}
#endif
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

void DataManager::reset_dose(void){
	rad_sum = 0;
	rad_dose_old = 0;
	EEPROM.put(0b110, 0);
}

void DataManager::reset_activity_test(){
	alarm = false;
	rad_max = 0;
	rad_back = 0;
	stop_timer = false;
	if(means_times == 0) next_step = true;
	else next_step = false;
	time_sec = 0;
	menu_page = 0;
	counter_mode = 1;
	page = 1;
}

#if defined(ADVANCED_ERROR)
void DataManager::calc_std(){
	uint32_t _sum = 0;
	for(unsigned i = 0; i < GEIGER_TIME; i++) _sum+=stat_buff[i];
	mean = (float)_sum/GEIGER_TIME;
	_sum = 0;
	for(unsigned i = 0; i < GEIGER_TIME; i++) _sum+=pow(stat_buff[i] - mean, 2);
	std = (float)_sum/(float)(GEIGER_TIME-1);
	//if(std > 65535) std = 0;
}
#endif