#include "myRemoteDevice.h"
#define PROMPT_STR CONFIG_IDF_TARGET
// void setLedStripColor(int index,int red,int green,int blue);

static int setLedStripColorConsoleCommand(int argc, char **argv) {
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
static int printVariablesConsoleCommand() {
    printVariables();
    return 0; 
}
static int setNvsVariableConsoleCommand(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: set <variable> <value> <namespace>\n");
        return 1; 
    };
    char*var=argv[1];
    char*value=argv[2];
    char*namespace=argv[3];

    setNvsVariableString(var,value,NULL);

    return 0; 
}
static int eraseNvsDataConsoleCommand(int argc, char **argv) {
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
static int doConsoleCommand(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: do <n>\n");
        return 1; 
    };
    char*s=argv[1];
    int i = atoi(s);
    void (*doFunctions[])(void) = {
        printNvsData
    };
    if (i < 0 || i >= sizeof(doFunctions) / sizeof(doFunctions[0])) {
        printf("Invalid function index %d\n", i);
        return 1;
    }

    // Call the function based on the index i
    doFunctions[i]();

    return 0; 
}
void register_custom_commands() {
    esp_console_cmd_t cmd = {
        .command = "strip",
        .help = "setLedStripColor",
        .hint = "strip index red green blue",
        .func = &setLedStripColorConsoleCommand,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    cmd = (esp_console_cmd_t){
        .command = "print",
        .help = "print nvs variables",
        .hint = "print [name-of-variable]",
        .func = &printVariablesConsoleCommand,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    cmd = (esp_console_cmd_t){
        .command = "set",
        .help = "set nvs variables",
        .hint = "set variable value",
        .func = &setNvsVariableConsoleCommand,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    cmd = (esp_console_cmd_t){
        .command = "delAll",
        .help = "delete all nvs variables",
        .hint = "[namespace]",
        .func = &eraseNvsDataConsoleCommand,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    cmd = (esp_console_cmd_t){
        .command = "do",
        .help = "do a function",
        .hint = "do <n> (n is the number of the function)",
        .func = &doConsoleCommand,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
void console(){
    esp_console_register_help_command();
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = PROMPT_STR ">";
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    esp_console_new_repl_uart(&hw_config, &repl_config, &repl);
    register_custom_commands();
    esp_console_start_repl(repl);
}
