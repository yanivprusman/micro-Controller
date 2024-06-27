#ifndef _BLE_
#define _BLE_
// #include "freertos/FreeRTOS.h"
#include "myRemoteDevice.h"

extern uint8_t ble_addr_type;
void ble_app_advertise(void);
void nimble();
#endif // _BLE_