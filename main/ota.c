// #include "ota.h"

// #define OTA_URL "https://esp.ya-niv.com/myRemoteDeviceBin"

// extern const uint8_t cacert_pem_start[] asm("_binary_cacert_pem_start");
// extern const uint8_t cacert_pem_end[] asm("_binary_cacert_pem_end");

// const uint8_t *cacert_pem2 = cacert_pem_start;

// void ota_task(void *pvParameter) {
//     // printf("%s\n",(char*)cacert_pem2);
//     // vTaskDelete(NULL); 

//     size_t cacert_pem_len = cacert_pem_end - cacert_pem_start;
//     esp_http_client_config_t config = {
//         .url = OTA_URL,
//         // .cert_pem =  NULL, 
//         .cert_pem =  (const char *)cacert_pem2, 
//     };
//     printf("B4\n");
//     esp_err_t err = esp_https_ota(&config);
//     printf("after\n");
//     if (err == ESP_OK) {
//         printf("success\n");
//         // esp_restart();
//     } else {
//         printf("error:%s\n", esp_err_to_name(err));
//     }
//     vTaskDelete(NULL); 

// }
