#ifndef DISPLAY_H__
#define DISPLAY_H__

void display_init(void);
//void display_set_data(float temp, float hum, uint16_t light);
void display_refresh(const char *str_temp, const char  *str_hum, const char *str_light);
void display_time(char* date_str, char* time_str);    

#endif
