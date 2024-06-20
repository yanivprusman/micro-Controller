#include "driver/uart.h"
#include "esp_console.h"
#define PROMPT_STR CONFIG_IDF_TARGET
void setLedStripColor(int index,int red,int green,int blue);
void printVariables();

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
