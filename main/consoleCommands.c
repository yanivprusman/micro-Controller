#include "consoleCommands.h"
#define PROMPT_STR CONFIG_IDF_TARGET

static int printNvsVariablesConsoleCommand(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: set <variable> <value> <namespace>\n");
        return 1; 
    };
    printNvsData("storage");
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

    setNvsVariableString(var,value,namespace);

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
void register_custom_commands() {
    esp_console_cmd_t cmd = {
        .command = STRIP_COMMAND,
        .help = "setLedStripColor",
        .hint = "strip index red green blue",
        .func = &STRIP_CALLBACK,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    cmd = (esp_console_cmd_t){
        .command = PRINT_MRD_VARIABLES,
        .help = "print nvs variables",
        .hint = "print [name-of-variable]",
        .func = &PRINT_MRD_VARIABLES_CALLBACK,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    cmd = (esp_console_cmd_t){
        .command = "printNvsVariables",
        .help = "print nvs variables",
        .hint = "print [name-of-variable]",
        .func = &printNvsVariablesConsoleCommand,
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
        .command = DO_COMMAND,
        .help = "do a function",
        .hint = "do <function-name>",
        .func = &DO_CALLBACK,
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
