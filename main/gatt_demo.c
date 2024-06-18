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

char *TAG = "BLE-Server tag ";
uint8_t ble_addr_type;
void ble_app_advertise(void);

#define NUM_VARIABLES (sizeof(variables) / sizeof(variables[0]))
#define INITIALIZED_FALSE_STRING "not initialized"
#define INITIALIZED_TRUE_STRING "initialized"

uint8_t initializedCustomDataTrue[4]  = {0x0, 0x0, 0x0, 0x0};
const uint8_t initializedCustomDataFalse[4] = {0xDF, 0xAD, 0xBE, 0xEE};

typedef struct {
    char *name;
    char *value;
    const char *nvsKey;
    const char *defaultValue;
} variables_t;
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
#define DEVICE_TYPE_UUID BLE_UUID16_DECLARE(0x180)
#define INITIALIZED_CHAR_UUID BLE_UUID16_DECLARE(0xFEF4)
#define UNINITIALIZED_CHAR_UUID BLE_UUID16_DECLARE(0xFEF5)

void printVariables(){
    for (int i = 0;i < NUM_VARIABLES;i++){
        if((variables[i]->value)&&(variables[i]->name)){
            printf("the value of %s is %s\n",variables[i]->name,variables[i]->value);
        }
    }
}

void fillVariablseFromJsonString(char* data){
    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        ESP_LOGE(TAG, "Error parsing JSON: %s", cJSON_GetErrorPtr());
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
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    for (size_t i = 0; i < NUM_VARIABLES; ++i) {
        size_t required_size = 0;
        err = nvs_get_str(storage, variables[i]->nvsKey, NULL, &required_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error getting size for %s: %s", variables[i]->name, esp_err_to_name(err));
            continue;
        }
        variables[i]->value = malloc(required_size);
        if (variables[i]->value == NULL) {
            ESP_LOGE(TAG, "Error allocating memory for %s", variables[i]->name);
            continue;
        }
        err = nvs_get_str(storage, variables[i]->nvsKey, variables[i]->value, &required_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error getting value for %s: %s", variables[i]->name, esp_err_to_name(err));
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

static int uninitializedCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
        readVariablesFromNvs();
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            cJSON *json_response = cJSON_CreateObject();
            if (json_response == NULL) {
                printf("Failed to create JSON object\n");
                return -1;
            }
            cJSON_AddStringToObject(json_response, myRemoteDeviceName.name, myRemoteDeviceName.value);
            cJSON_AddStringToObject(json_response, myRemoteDeviceID.name, myRemoteDeviceID.value);
            cJSON_AddStringToObject(json_response, initialized.name, initialized.value);

            char *json_string = cJSON_PrintUnformatted(json_response);
            if (json_string == NULL) {
                printf("Failed to print JSON string\n");
                cJSON_Delete(json_response);
                return -1;
            }

            printf("JSON Response: %s\n", json_string);
            os_mbuf_append(ctxt->om, json_string, strlen(json_string));
            cJSON_Delete(json_response);
            free(json_string);
            return 0;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            fillVariablseFromJsonString((char *) ctxt->om->om_data);
            initialized.value = INITIALIZED_TRUE_STRING;
            writeVariablesToNvs();
            printVariables();
            return 0;

        default:
            return BLE_ATT_ERR_UNLIKELY;
    }
    return 0;
}

static int initializedCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    // ble_svc_gap_device_name_set(myRemoteDeviceName);
    cJSON *json_response = cJSON_CreateObject();
    if (json_response == NULL) {
        printf("Failed to create JSON object\n");
        return -1;
    }
    readVariablesFromNvs();
    cJSON_AddStringToObject(json_response, "myRemoteDeviceName", myRemoteDeviceName.value);

    char *json_string = cJSON_PrintUnformatted(json_response);
    if (json_string == NULL) {
        printf("Failed to print JSON string\n");
        cJSON_Delete(json_response);
        return -1;
    }

    printf("JSON Response: %s\n", json_string);
    os_mbuf_append(ctxt->om, json_string, strlen(json_string));

    cJSON_Delete(json_response);
    free(json_string);

    return 0;
}

static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = DEVICE_TYPE_UUID,                
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = INITIALIZED_CHAR_UUID,
          .flags = BLE_GATT_CHR_F_WRITE|BLE_GATT_CHR_F_READ,
          .access_cb = initializedCallback},
         {.uuid = UNINITIALIZED_CHAR_UUID,           // Define UUID for writing
          .flags = BLE_GATT_CHR_F_WRITE|BLE_GATT_CHR_F_READ,
          .access_cb = uninitializedCallback},
        {0}}},
    {0}
};

static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT DISCONNECT reason: %d", event->disconnect.reason);
        vTaskDelay(pdMS_TO_TICKS(1000));
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}

void ble_app_advertise(void)
{
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    readVariablesFromNvs();
    ble_svc_gap_device_name_set(myRemoteDeviceName.value);
    device_name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    uint16_t company_id = 0xFFFA;
    
    uint8_t custom_data[4];
    if (strcmp(initialized.value, INITIALIZED_TRUE_STRING) == 0){
        printf("my remote device id:%d",atoi(myRemoteDeviceID.value));
        initializedCustomDataTrue[3] = atoi(myRemoteDeviceID.value);
        memcpy(custom_data, initializedCustomDataTrue, sizeof(initializedCustomDataTrue));
    }else if (strcmp(initialized.value, INITIALIZED_FALSE_STRING) == 0){
        memcpy(custom_data, initializedCustomDataFalse, sizeof(initializedCustomDataFalse));
    }
    uint8_t mfg_data[sizeof(company_id) + sizeof(custom_data)];
    memcpy(mfg_data, &company_id, sizeof(company_id));
    memcpy(mfg_data + sizeof(company_id), custom_data, sizeof(custom_data));

    fields.mfg_data = mfg_data;
    fields.mfg_data_len = sizeof(mfg_data);

    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_advertise();
}

void host_task(void *param)
{
    nimble_port_run();
}

void app2();

void writeDefaultValues() {
    esp_err_t err;
    nvs_handle_t storage;
    err = nvs_open("storage", NVS_READWRITE, &storage);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }

    for (size_t i = 0; i < NUM_VARIABLES; ++i) {
        err = nvs_set_str(storage, variables[i]->nvsKey, variables[i]->defaultValue);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error setting %s: %s", variables[i]->name, esp_err_to_name(err));
        }
    }

    err = nvs_commit(storage);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
    }

    nvs_close(storage);
}

bool variablesAreEmpty(){
    bool empty = false;
    if (myRemoteDeviceID.value==NULL){
        empty = true;
        printf("variable myRemoteDeviceId is NULL\n");
    }
    return empty;
}
#include "driver/gpio.h"

void app_main()
{
    // app2();
    // return;
    nvs_flash_init();
    readVariablesFromNvs();
    if(variablesAreEmpty()){
        writeDefaultValues();        
    }
    nimble_port_init();
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
}

// void deleteDataFromNvs(void)
// {
//     nvs_handle_t nvs_handle;
//     esp_err_t err;
//     size_t required_size;

//     // Initialize NVS
//     err = nvs_flash_init();
//     if (err != ESP_OK) {
//         printf("Error initializing NVS Flash: %s\n", esp_err_to_name(err));
//         return;
//     }

//     // Open NVS
//     err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
//     if (err != ESP_OK) {
//         printf("Error opening NVS: %s\n", esp_err_to_name(err));
//         return;
//     }
//     if (nvs_get_str(nvs_handle, NVS_DATA_KEY, NULL, &required_size) == ESP_OK) {
//         // Delete the NVS variable
//         err = nvs_erase_key(nvs_handle, NVS_DATA_KEY);
//         if (err != ESP_OK) {
//             printf("Error deleting NVS variable: %s\n", esp_err_to_name(err));
//         } else {
//             printf("NVS variable '%s' deleted successfully.\n", NVS_DATA_KEY);
//         }
//     }else if (nvs_get_str(nvs_handle, NVS_DATA_KEY, NULL, &required_size) == ESP_ERR_NVS_NOT_FOUND) {
//         printf("NVS variable '%s' not found.\n", NVS_DATA_KEY);
//     }

//     // Commit and close NVS
//     nvs_commit(nvs_handle);
//     nvs_close(nvs_handle);
// }

void app2() {
    int x = 18;
    for(;;){
        printf("%d\n",x);
        x++;
    }
}
