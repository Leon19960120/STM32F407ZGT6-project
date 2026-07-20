#ifndef __DISPLAY_H__
#define __DISPLAY_H__

void display_init(void);
void display_set_data(float temp, float hum, uint16_t light);
void display_refresh(u8 *str_temp, u8 *str_hum, u8 *str_light);
void display_time(char* date_str, char* time_str);    

#endif
