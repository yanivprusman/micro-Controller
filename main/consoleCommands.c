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
    if (argc == 0) {
        printf("Usage: printNvs <namespace> <variable or 'all'>\n");
        return 1; 
    };
    char*namespace=argv[1];
    if (argc != 3) {
        return 1; 
    };
    char*key=argv[2];
    if (strcmp(key,"all")==0){
        printNvsData(namespace);
        return 0;
    }
    printNvsVariable(namespace,key);
    return 0; 
}
int setNvsCB(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: set <namespace> <variable> <value> \n");
        return 1; 
    };
    char*namespace= argv[1];
    char*key=       argv[2];
    char*value=     argv[3];

    setNvsVariableString(namespace,key,value);

    return 0; 
}
int delNvsCB(int argc, char **argv) {
    if (argc == 0) {
        printf("Usage: dellNvs <namespace> <variable or 'all'>\n");
        return 1; 
    };
    char*namespace=argv[1];
    char*var=argv[2];
    if (strcmp(var,"all")==0){
        eraseNvsData(namespace);
        return 0;
    }
    deleteNvsVariable(namespace,var);
    // if (namespace==NULL) namespace = "storage";    

    return 0; 
}
int otaCB(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: ota <url>\n");
        return 1; 
    };
    char*url=argv[1];
    xTaskCreate(&ota_task, "ota", 8192, url, 5, NULL);
    return 0; 
}

int doCB(int argc, char **argv) {
    char*s=argv[1];
    int i = atoi(s);
    int (*doFunctions[])(int argc, char **argv) = {
        doFunction1,
        doFunction2
    };
    if (i < 1 || i >= sizeof(doFunctions) / sizeof(doFunctions[0])+1) {
        if (strcmp(s,"asdf")==0){
            setLedStripColor(atoi(argv[2]),atoi(argv[3]),atoi(argv[4]),atoi(argv[5]));
            return 0;
        }else{
            printf("Invalid function index %d\n", i);
            return 1;
        }
    }
    doFunctions[i-1](argc, argv);
    return 0; 
}
