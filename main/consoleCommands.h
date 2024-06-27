#ifndef _CONSOLE_COMMANDS_
#define _CONSOLE_COMMANDS_
#include "myRemoteDevice.h"

extern int stripCB(int argc, char **argv);
extern int printMRDCB(int argc, char **argv);
extern int printNvsCB(int argc, char **argv);
extern int setNvsCB(int argc, char **argv);
extern int delNvsCB(int argc, char **argv);
extern int otaCB(int argc, char **argv);
extern int doCB(int argc, char **argv);

void console();

#endif // _CONSOLE_COMMANDS_
