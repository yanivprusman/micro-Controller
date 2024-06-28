#include "myRemoteDevice.h"
#include "protocol_examples_common.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
char * TAG = "MyRemoteDevice";

uint8_t initializedCustomDataTrue[4]  = {0x0, 0x0, 0x0, 0x0};
const uint8_t initializedCustomDataFalse[4] = {0xDF, 0xAD, 0xBE, 0xEE};

variables_t 
    initialized = {
        "initialized",
        NULL,
        "var_1",
        INITIALIZED_FALSE_STRING},
    ssidName={
        "ssidName",
        NULL,
        "var_2",
        ""},
    ssidPassword={
        "ssidPassword",
        NULL,
        "var_3",
        ""},
    myRemoteDeviceName={
        "myRemoteDeviceName",
        NULL,
        "var_4",
        "MyRemoteDevice"},
    myRemoteDeviceID={
        "myRemoteDeviceID",
        NULL,
        "var_5",
        ""};
variables_t * variables[]={&initialized,&ssidName,&ssidPassword,&myRemoteDeviceName,&myRemoteDeviceID};

void printVariables(){
    for (int i = 0;i < NUM_VARIABLES;i++){
        if((variables[i]->value)&&(variables[i]->name)){
            printf("the value of %s is \"%s\"\n",variables[i]->name,variables[i]->value);
        }
    }
}

void fillVariablseFromJsonString(char* data){
    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        printf("Error parsing JSON: %s", cJSON_GetErrorPtr());
        return;
    }
    for (int i = 0;i < NUM_VARIABLES;i++){
        cJSON *got = cJSON_GetObjectItemCaseSensitive(json,variables[i]->name);
        if (cJSON_IsString(got) && (got->valuestring != NULL)) {
            if (variables[i]->value) free(variables[i]->value);
            variables[i]->value = strdup(got->valuestring);
        }
    }
    cJSON_Delete(json);
}

void readVariablesFromNvs(){
    esp_err_t err;
    nvs_handle_t storage;
    err = nvs_open("storage", NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        printf("Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    for (size_t i = 0; i < NUM_VARIABLES; ++i) {
        size_t required_size = 0;
        err = nvs_get_str(storage, variables[i]->nvsKey, NULL, &required_size);
        if (err != ESP_OK) {
            printf("Error getting size for %s: %s", variables[i]->name, esp_err_to_name(err));
            continue;
        }
        variables[i]->value = malloc(required_size);
        if (variables[i]->value == NULL) {
            printf("Error allocating memory for %s", variables[i]->name);
            continue;
        }
        err = nvs_get_str(storage, variables[i]->nvsKey, variables[i]->value, &required_size);
        if (err != ESP_OK) {
            printf("Error getting value for %s: %s", variables[i]->name, esp_err_to_name(err));
            free(variables[i]->value);
            variables[i]->value = NULL;
            continue;
        }
    }
    nvs_close(storage);
}
void writeVariablesToNvs(){
    nvs_handle_t storage;
    nvs_open("storage", NVS_READWRITE, &storage);
    for (size_t i = 0; i < NUM_VARIABLES; ++i) {
        if (variables[i]->value != NULL){
            nvs_set_str(storage, variables[i]->nvsKey, variables[i]->value);
        };
    }
    nvs_commit(storage);
    nvs_close(storage);
}

void writeDefaultValues() {
    esp_err_t err;
    nvs_handle_t storage;
    // printf("nvs_open\n");
    err = nvs_open("storage", NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        printf("Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    // printf("num vars: %d\n", NUM_VARIABLES);

    for (size_t i = 0; i < NUM_VARIABLES; ++i) {
        err = nvs_set_str(storage, variables[i]->nvsKey, variables[i]->defaultValue);
        if (err != ESP_OK) {
            printf("Error setting %s: %s", variables[i]->name, esp_err_to_name(err));
        }
    }
    // printf("nvs_commit\n");

    err = nvs_commit(storage);
    if (err != ESP_OK) {
        printf("Error committing NVS: %s", esp_err_to_name(err));
    }

    nvs_close(storage);
    // printf("end write\n");

}

bool variablesAreEmpty(){
    bool empty = false;
    if (myRemoteDeviceID.value==NULL){
        empty = true;
        printf("variable myRemoteDeviceId is NULL\n");
    }
    return empty;
}
void app_main()
{
    esp_reset_reason_t r = esp_reset_reason();
    printf("last reset reason: %d\n",r);
    esp_err_t err = nvs_flash_init();
    if(err !=ESP_OK){
        printf("error1: %s\n",esp_err_to_name(err));
    };    
    err = esp_netif_init();
    if(err !=ESP_OK){
        printf("error2: %s\n",esp_err_to_name(err));
    };
    err = esp_event_loop_create_default();
    if(err !=ESP_OK){
        printf("error3: %s\n",esp_err_to_name(err));
    };
    app2();
    return;
    // wifi();
    example_connect(); 
    nimble();
    doTime();
    readVariablesFromNvs();
    if(variablesAreEmpty()){
        writeDefaultValues();        
    }
    // char *argv[3];
    // argv[2] = "https://example.com";
    // doFunction1(3,argv);
    console();
    printf("ota \"https://esp.ya-niv.com/myRemoteDeviceBin\"\n");
}
int address = 0;
#include <esp_partition.h>
// #include "bootloader_support/include/bootloader_utility.h"
int getNumberOfAppPartitions(){
    esp_partition_iterator_t iterator = NULL;
    const esp_partition_t *partition = NULL;
    int ota_count = 0;
    iterator = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
    while (iterator != NULL) {
        partition = esp_partition_get(iterator);
        if (partition == NULL) {
            break;
        }
        ota_count++;
        iterator = esp_partition_next(iterator);
    }
    esp_partition_iterator_release(iterator);
    return ota_count;

}
void print_partition_info() {
    const esp_partition_t* partition;
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    
    if (it != NULL) {
        printf("App partitions:\n");
        do {
            partition = esp_partition_get(it);
            printf("Name: %s\n", partition->label);
            printf("Type: 0x%02x\n", partition->type);
            printf("Subtype: 0x%02x\n", partition->subtype);
            printf("Address: 0x%08lx\n", partition->address);
            printf("Size: 0x%08lx (%ld KB)\n", partition->size, partition->size / 1024);
            printf("Encrypted: %s\n", partition->encrypted ? "Yes" : "No");
            printf("\n");
        } while ((it = esp_partition_next(it)) != NULL);
        esp_partition_iterator_release(it);
    }

    it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);

    if (it != NULL) {
        printf("Data partitions:\n");
        do {
            partition = esp_partition_get(it);
            printf("Name: %s\n", partition->label);
            printf("Type: 0x%02x\n", partition->type);
            printf("Subtype: 0x%02x\n", partition->subtype);
            printf("Address: 0x%08lx\n", partition->address);
            printf("Size: 0x%08lx (%ld KB)\n", partition->size, partition->size / 1024);
            printf("Encrypted: %s\n", partition->encrypted ? "Yes" : "No");
            printf("\n");
        } while ((it = esp_partition_next(it)) != NULL);
        esp_partition_iterator_release(it);
    }
}

typedef void (*app_main_fn)(void);

void call_ota_0() {
    char * otaPartition = "ota_0";
    const esp_partition_t *ota_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, otaPartition);
    
    if (ota_partition == NULL) {
        printf("Failed to find OTA partition\n");
        return;
    }
    
    printf("Found OTA partition at address 0x%08lx\n", ota_partition->address);
    
    // Cast the address to a function pointer and call it
    app_main_fn ota_app_main = (app_main_fn) ota_partition->address+0x1000;
    ota_app_main = (app_main_fn) address;
    ota_app_main();
}

int doFunction1(int argc, char **argv){
    // if (argc != 3){
    //     printf("usage: do 1 <uri>\n");
    //     return -1;
    // }
    // int a = getNumberOfAppPartitions();
    // printf("partitions:%d\n",a);
    print_partition_info();
    return 0;
};
int doFunction2(int argc, char **argv){
    if (argc != 3){
        printf("usage: do 2 <address>\n");
        return -1;
    }
    address = atoi(argv[2]);
    // xTaskCreate(&ota_task, "asdf", 8192, NULL, 5, NULL);
    xTaskCreate((TaskFunction_t)call_ota_0, "call_ota_0", 8192, NULL, 5, NULL);    
    return 0;
}
void app2() {
    console();
}
