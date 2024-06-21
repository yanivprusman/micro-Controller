#include "consoleCommands.h"
int DO_CALLBACK(int argc, char **argv) {
    char*s=argv[1];
    int i = atoi(s);
    void (*doFunctions[])(void) = {
        doFunction1,
        doFunction2
    };
    if (i < 1 || i >= sizeof(doFunctions) / sizeof(doFunctions[0])) {
        if (strcmp(s,"asdf")==0){
            setLedStripColor(atoi(argv[2]),atoi(argv[3]),atoi(argv[4]),atoi(argv[5]));
            return 0;
        }else{
            printf("Invalid function index %d\n", i);
            return 1;
        }
    }
    doFunctions[i]();
    return 0; 
}
int STRIP_CALLBACK(int argc, char **argv) {
    if (argc != 5) {
        printf("Usage: strip <index> <red> <green> <blue>\n");
        return 1; 
    }
    int index = atoi(argv[1]);
    int red = atoi(argv[2]);
    int green = atoi(argv[3]);
    int blue = atoi(argv[4]);
    setLedStripColor(index,red,green,blue);
    return 0; 
}
int PRINT_MRD_VARIABLES_CALLBACK(int argc, char **argv) {
    printVariables();
    return 0; 
}
