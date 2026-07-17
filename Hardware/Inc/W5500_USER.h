#ifndef __W5500_H_
#define __W5500_H_

#include "W5500.h"

void Delay(unsigned int d);
void W5500_Initialization(void);
void Load_Net_Parameters_Client(void);
void Load_Net_Parameters_Server(void);
void Load_Net_Parameters_UDP(void);
void W5500_Socket_Set(void);
void Process_Socket_Data(SOCKET s);

#endif