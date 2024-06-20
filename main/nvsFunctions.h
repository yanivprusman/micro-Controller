#ifndef _NVS_FUNCTIONS_
#define _NVS_FUNCTIONS_
#include "myRemoteDevice.h"
void eraseNvsData(char* namespace); 
void setNvsVariableString(char*var,char*value,char*namespace);
void printNvsData(const char* namespace);
#endif // _NVS_FUNCTIONS_
