#include "ota.h"
#define OTA_URL "https://esp.ya-niv.com/myRemoteDeviceBin"
// #define OTA_URL "https://esp.ya-niv.com/myRemoteDeviceBlink"

void ota_task(void *pvParameter) {
    char* url = pvParameter;
    esp_err_t err;
    esp_http_client_config_t httpConfig = {
        .url = url,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_https_ota_config_t config = {
        .http_config = &httpConfig
    };
    err = esp_https_ota(&config);
    if (err == ESP_OK) {
        esp_restart();
    } else {
        printf("error:%s\n", esp_err_to_name(err));
    }
    vTaskDelete(NULL); 
}
