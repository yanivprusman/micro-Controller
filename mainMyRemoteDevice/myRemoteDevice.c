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
    // app2();
    // return;
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


int doFunction1(int argc, char **argv){
    printf("new version 2\n");
    if (argc != 3){
        printf("usage: do 1 <uri>\n");
        return -1;
    }
    args_t args = {argc,argv};
    // requestTaskBundle(argv[2]);

    return 0;
};
int doFunction2(int argc, char **argv){
    xTaskCreate(&ota_task, "asdf", 8192, NULL, 5, NULL);

    // args_t args = {argc,argv};
    // if (args.argc != 3){
    //     printf("usage: do 2 <uri>\n");
    //     return -1;
    // }
    
    return 0;
}
void app2() {
}