#ifndef _CONSOLE_COMMANDS_
#define _CONSOLE_COMMANDS_
#include "myRemoteDevice.h"

#define DO_COMMAND "do"
#define DO_CALLBACK doConsoleCommand
extern int DO_CALLBACK(int argc, char **argv);

#define STRIP_COMMAND "strip"
#define STRIP_CALLBACK setLedStripColorConsoleCommand
extern int STRIP_CALLBACK(int argc, char **argv);

#define PRINT_MRD_VARIABLES "printMRDVariables"
#define PRINT_MRD_VARIABLES_CALLBACK printMRDVariablesConsoleCommand
extern int PRINT_MRD_VARIABLES_CALLBACK(int argc, char **argv);

void console();

#endif // _CONSOLE_COMMANDS_
