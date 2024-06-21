#include "consoleCommands.h"
int stripCB(int argc, char **argv) {
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
int printMRDCB(int argc, char **argv) {
    printVariables();
    return 0; 
}
int printNvsCB(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: set <variable> <value> <namespace>\n");
        return 1; 
    };
    printNvsData("storage");
    return 0; 
}
int setNvsCB(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: set <variable> <value> <namespace>\n");
        return 1; 
    };
    char*var=argv[1];
    char*value=argv[2];
    char*namespace=argv[3];

    setNvsVariableString(var,value,namespace);

    return 0; 
}
int delNvsCB(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: dellAll <namespace>\n");
        return 1; 
    };
    char*namespace=argv[1];
    // printf("%s\n",namespace);
    if (namespace==NULL) namespace = "storage";    
    eraseNvsData(namespace);

    return 0; 
}

int doCB(int argc, char **argv) {
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
