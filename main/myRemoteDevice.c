#include "myRemoteDevice.h"

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
void app2();
void app_main()
{
    nvs_flash_init();
    console();
    // app2();
    // return;
    readVariablesFromNvs();
    // return;
    if(variablesAreEmpty()){
        writeDefaultValues();        
    }
    nimble();
}

void doFunction0(){
    printf("write var to nvs\n");
    char* varName = "myVar";
    nvs_handle_t storage;
    nvs_open("storage", NVS_READWRITE, &storage);
    nvs_set_str(storage, varName, "asdf");
    nvs_commit(storage);
    nvs_close(storage);
}
void doFunction1(){
    // printf("read var from nvs\n");    
    char*varName = varName;
    esp_err_t err;
    nvs_handle_t storage;
    err = nvs_open("storage", NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        printf("Error opening NVS handle: %s\n", esp_err_to_name(err));
        return;
    }
    size_t required_size = 0;
    err = nvs_get_str(storage, varName, NULL, &required_size);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND){

        }
        printf("Error getting size for %s: %s\n", varName, esp_err_to_name(err));
    }
    char* var = malloc(required_size);
    if (var == NULL) {
        printf("Error allocating memory for %s\n", "var");
    }
    err = nvs_get_str(storage, varName, var, &required_size);
    if (err != ESP_OK) {
        printf("Error getting value for %s: %s\n", varName, esp_err_to_name(err));
        free(var);
        var = NULL;
    }
    printf("read var %s:%s\n",varName,var);
}
void doFunction2(){
    printf("printing data\n");
    printNvsData("storage");
}
void doFunction3(){

}
void doFunction4(){

}

void app2() {
    // if(variablesAreEmpty()){
    //     writeDefaultValues();        
    // }
    // writeDefaultValues();        
    // printNvsData("storage");
    // printVariables();
    console();
    // printf("App2 end\n");
}
