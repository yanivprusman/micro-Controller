#include "nvs_flash.h"
#include "esp_log.h"

void erase_nvs_data(void) {
    esp_err_t err;

    // Initialize NVS
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        // Retry nvs_flash_init
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open NVS handle
    nvs_handle_t nvs_handle;
    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return; // Return early if there is an error opening the handle
    } 

    // Erase all NVS data
    err = nvs_flash_erase();
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) erasing NVS partition!", esp_err_to_name(err));
    } else {
        // Commit the changes
        err = nvs_commit(nvs_handle);
        if (err != ESP_OK) {
            ESP_LOGE("NVS", "Error (%s) committing changes!", esp_err_to_name(err));
        } else {
            ESP_LOGI("NVS", "All NVS data erased successfully!");
        }
    }
    // Close NVS handle
    nvs_close(nvs_handle);
}
