#include "consoleCommands.h"
#define PROMPT_STR CONFIG_IDF_TARGET

void register_custom_commands() {
    esp_console_cmd_t 
        stripCmd = {
            "strip",                                //command
            "set led strip color",                  //help
            "strip <index> <red> <green> <blue>",   //hint
            &stripCB,
        },
        printMRD ={
            "printMRD",
            "print my remote device variables",
            "printMRD",
            &printMRDCB,
        },
        printNVS ={
            "printNvs",
            "print nvs variables",
            "printNvs <namespace> <variable or 'all'>",
            &printNvsCB,
        },
        setNvs ={
            "setNvs",
            "set nvs variable",
            "setNvs <namespace> <variable> <value>",
            &setNvsCB,
        },
        delNvs ={
            "delNvs",
            "delete nvs variable",
            "delNvs <namespace> <variable or 'all'>",
            &delNvsCB,
        },
        ota ={
            "ota",
            "ota update",
            "ota <url>",
            &otaCB,
        },
        doC ={
            "do",
            "do a function",
            "do <function-name>",
            &doCB,
        };
    esp_console_cmd_t *commands[] = {
        &stripCmd,
        &printMRD,
        &printNVS,
        &setNvs,
        &delNvs,
        &ota,
        &doC};
    
    for(int x=0; x<(sizeof(commands)/sizeof(commands[0]));x++){
        ESP_ERROR_CHECK(esp_console_cmd_register(commands[x]));
    }    
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
