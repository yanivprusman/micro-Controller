#ifndef _MY_REMOTE_DEVICE_
#define _MY_REMOTE_DEVICE_
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"
#include "cJSON.h"
#include "esp_sleep.h"
#include <driver/gpio.h>
#include "led_strip.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "consoleCommands.h"
#include "ble.h"
extern char *TAG ;

extern uint8_t initializedCustomDataTrue[4];
extern const uint8_t initializedCustomDataFalse[4];

void console();
typedef struct {
    char *name;
    char *value;
    const char *nvsKey;
    const char *defaultValue;
} variables_t;

#define NUM_VARIABLES (sizeof(variables) / sizeof(variables[0]))
#define INITIALIZED_FALSE_STRING "not initialized"
#define INITIALIZED_TRUE_STRING "initialized"

extern variables_t initialized;
extern variables_t ssidName;
extern variables_t ssidPassword;
extern variables_t myRemoteDeviceName;
extern variables_t myRemoteDeviceID;
extern variables_t *variables[];
#define DEVICE_TYPE_UUID BLE_UUID16_DECLARE(0x180)
#define UNINITIALIZED_CHAR_UUID BLE_UUID16_DECLARE(0xFEF5)
#define INITIALIZED_CHAR_UUID BLE_UUID16_DECLARE(0xFEF4)
void readVariablesFromNvs();
void printVariables();
void fillVariablseFromJsonString(char* data);
void writeVariablesToNvs();
#endif // _MY_REMOTE_DEVICE_