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
void setNvsVariableString(char*namespace,char*key,char*value){
    if (namespace==NULL) namespace = "storage";
    nvs_handle_t storage;
    nvs_open(namespace, NVS_READWRITE, &storage);
    nvs_set_str(storage, key, value);
    nvs_commit(storage);
    nvs_close(storage);
}

void printNvsData(const char* namespace) {
    // if(namespace==NULL) namespace = "storage";
    esp_err_t err= NULL;
    nvs_iterator_t it = NULL;
    nvs_handle_t storage;
    // err = nvs_open_from_partition("nvs", namespace, NVS_READONLY, &storage);
    err = nvs_open(namespace, NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }
    // namespace=NULL;
    err =  nvs_entry_find_in_handle(storage, NVS_TYPE_ANY, &it);
    if (err!=ESP_OK){
        printf("error:%s\n",esp_err_to_name(err));
    }
    while(err == ESP_OK) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
        if(info.type==33){//string
            size_t required_size = 0;
            err = nvs_get_str(storage, info.key, NULL, &required_size);
            if (err != ESP_OK) {
                printf("Error getting size for %s: %s\n", info.key, esp_err_to_name(err));
            }
            char* var = malloc(required_size);
            if (var == NULL) {
                printf("Error allocating memory for %s\n", "var");
            }
            err = nvs_get_str(storage, info.key, var, &required_size);
            if (err != ESP_OK) {
                printf("Error getting value for %s: %s\n", info.key, esp_err_to_name(err));
                free(var);
                var = NULL;
            }
            
            printf("key '%s', type '%d', value '%s' \n", info.key, info.type, var);
        }

        err = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);
}
void printNvsVariable(const char* namespace, const char* key) {
    printf("printNvsVariable namespace:%s ,key:%s\n",namespace,key);
    esp_err_t err= NULL;
    nvs_handle_t storage;
    err = nvs_open(namespace, NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }
    size_t required_size = 0;
    err = nvs_get_str(storage, key, NULL, &required_size);
    if (err != ESP_OK) {
        printf("Error getting size for %s: %s\n", key, esp_err_to_name(err));
        return;
    }
    char* var = malloc(required_size);
    if (var == NULL) {
        printf("Error allocating memory for %s\n", key);
        return;
    }
    err = nvs_get_str(storage, key, var, &required_size);
    if (err != ESP_OK) {
        printf("Error getting value for %s: %s\n", key, esp_err_to_name(err));
        free(var);
        var = NULL;
        return;
    }   
    printf("key '%s', value '%s' \n", key, var);
    free(var);
    var = NULL;
    return;
}

void deleteNvsVariable(char* namespace, char* var){
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        printf("Error opening NVS namespace '%s': %s\n", namespace, esp_err_to_name(err));
        return;
    }
    err = nvs_erase_key(nvs_handle, var);
    if (err != ESP_OK) {
        printf("Error deleting variable '%s' from namespace '%s': %s\n", var, namespace, esp_err_to_name(err));
    } else {
        printf("Successfully deleted variable '%s' from namespace '%s'\n", var, namespace);
    }
    nvs_close(nvs_handle);
}; 
