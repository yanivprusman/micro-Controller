#ifndef _NVS_FUNCTIONS_
#define _NVS_FUNCTIONS_
#include "myRemoteDevice.h"
void eraseNvsData(char* namespace); 
void setNvsVariableString(char*var,char*value,char*namespace);
void printNvsData(const char* namespace);
void deleteNvsVariable(char* namespace, char* var); 
void printNvsVariable(const char* namespace,const char* var); 
#endif // _NVS_FUNCTIONS_
