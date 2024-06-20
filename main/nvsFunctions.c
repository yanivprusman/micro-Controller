#include "nvsFunctions.h"

void eraseNvsData(char* namespace) {
    esp_err_t err;
    err = nvs_flash_erase();
    if (err != ESP_OK) {
        printf("Error (%s) erasing NVS partition!\n", esp_err_to_name(err));
    } else {
        printf("All NVS data erased successfully!\n");
    }
    nvs_flash_init();
}
void setNvsVariableString(char*var,char*value,char*namespace){
    if (namespace==NULL) namespace = "storage";
    nvs_handle_t storage;
    nvs_open(namespace, NVS_READWRITE, &storage);
    nvs_set_str(storage, var, value);
    nvs_commit(storage);
    nvs_close(storage);
}

void printNvsData(const char* namespace) {
    printf("in printNvsData\n");
    namespace = "storage";
    nvs_iterator_t it = NULL;
    esp_err_t err = nvs_entry_find(NULL, namespace, NVS_TYPE_ANY, &it);
    while(err == ESP_OK) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
        printf("key '%s', type '%d' \n", info.key, info.type);
        err = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);
}

