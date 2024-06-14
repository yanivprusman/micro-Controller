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

#define NVS_DATA_KEY "data"
#define NVS_INITIALIZED_KEY "data"
#define DEFAULT_VALUE "{\"initialized\":\"not initialized\",\"ssidName\":\"defaultSSID\",\"ssidPassword\":\"defaultPass\",\"myRemoteDeviceName\":\"MyRemoteDevice\"}"
#define MAX_DATA_LENGTH 1000  

char data[MAX_DATA_LENGTH + 1]; 

char* initializedFalseString = "not initialized";
char* initializedTrueString = "initialized";
char* initialized = NULL;
char* ssidName = NULL;
char* ssidPassword = NULL;
char* myRemoteDeviceName = NULL;

const uint8_t initializedCustomDataTrue[4]  = {0xDF, 0xAD, 0xBE, 0xEF};
const uint8_t initializedCustomDataFalse[4] = {0xDF, 0xAD, 0xBE, 0xEE};

void parseJson(){
    esp_err_t err;

    err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error initializing NVS: %s", esp_err_to_name(err));
        return;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }

    size_t required_size = 0;
    err = nvs_get_str(my_handle, "data", NULL, &required_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error getting data size: %s", esp_err_to_name(err));
        nvs_close(my_handle);
        return;
    }

    char *data = malloc(required_size);
    if (data == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for data");
        nvs_close(my_handle);
        return;
    }

    err = nvs_get_str(my_handle, "data", data, &required_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error getting data: %s", esp_err_to_name(err));
        free(data);
        nvs_close(my_handle);
        return;
    }

    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        ESP_LOGE(TAG, "Error parsing JSON: %s", cJSON_GetErrorPtr());
        free(data);
        nvs_close(my_handle);
        return;
    }

    cJSON *cJsonInitialized = cJSON_GetObjectItemCaseSensitive(json, "initialized");
    cJSON *cJsonSsidName = cJSON_GetObjectItemCaseSensitive(json, "ssidName");
    cJSON *cJsonSsidPassword = cJSON_GetObjectItemCaseSensitive(json, "ssidPassword");
    cJSON *cJsonMyRemoteDeviceName = cJSON_GetObjectItemCaseSensitive(json, "myRemoteDeviceName");

    if (cJSON_IsString(cJsonInitialized) && (cJsonInitialized->valuestring != NULL)) {
        if (initialized) free(initialized);
        initialized = strdup(cJsonInitialized->valuestring);
    }
    if (cJSON_IsString(cJsonSsidName) && (cJsonSsidName->valuestring != NULL)) {
        if (ssidName) free(ssidName);
        ssidName = strdup(cJsonSsidName->valuestring);
    }
    if (cJSON_IsString(cJsonSsidPassword) && (cJsonSsidPassword->valuestring != NULL)) {
        if (ssidPassword) free(ssidPassword);
        ssidPassword = strdup(cJsonSsidPassword->valuestring);
    }
    if (cJSON_IsString(cJsonMyRemoteDeviceName) && (cJsonMyRemoteDeviceName->valuestring != NULL)) {
        if (myRemoteDeviceName) free(myRemoteDeviceName);
        myRemoteDeviceName = strdup(cJsonMyRemoteDeviceName->valuestring);
    }

    cJSON_Delete(json);
    free(data);
    nvs_close(my_handle);

}

static int write_data(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    // Handle write operation
    int len = ctxt->om->om_len;
    if (len > MAX_DATA_LENGTH) {
        // Truncate if the received data is longer than the maximum length
        len = MAX_DATA_LENGTH;
    }
    // Copy the data from the received buffer to the data variable
    memcpy(data, ctxt->om->om_data, len);
    // Null-terminate the string
    data[len] = '\0';
    // Save the data to NVS
    nvs_handle_t nvs_handle;
    nvs_open("storage", NVS_READWRITE, &nvs_handle);
    nvs_set_str(nvs_handle, NVS_DATA_KEY, data);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    printf("data from the client: %.*s\n", len, data);

    return 0;
}

static int read_data(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    parseJson();
    ble_svc_gap_device_name_set(myRemoteDeviceName);
    cJSON *json_response = cJSON_CreateObject();
    if (json_response == NULL) {
        printf("Failed to create JSON object\n");
        return -1;
    }

    cJSON_AddStringToObject(json_response, "myRemoteDeviceName", myRemoteDeviceName);

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
     .uuid = BLE_UUID16_DECLARE(0x180),                 // Define UUID for device type
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(0xFEF4),           // Define UUID for reading
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = read_data},
         {.uuid = BLE_UUID16_DECLARE(0xFEF5),           // Define UUID for writing
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = write_data},
        {0}}},
    {0}};

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
    parseJson();
    ble_svc_gap_device_name_set(myRemoteDeviceName);
    device_name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    uint16_t company_id = 0xFFFA;
    
    uint8_t custom_data[4];
    if (strcmp(initialized, initializedTrueString) == 0){
        memcpy(custom_data, initializedCustomDataTrue, sizeof(initializedCustomDataTrue));
    }else if (strcmp(initialized, initializedFalseString) == 0){
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
#define MAX_NAMESPACE_LEN 16
void app_main()
{
    // app2();
    // return;
    nvs_flash_init();
    nvs_handle_t nvs_handle;
    size_t required_size;
    nvs_open("storage", NVS_READWRITE, &nvs_handle);
    
    if (nvs_get_str(nvs_handle, NVS_DATA_KEY, NULL, &required_size) == ESP_ERR_NVS_NOT_FOUND) {
        nvs_set_str(nvs_handle, NVS_DATA_KEY, DEFAULT_VALUE);
        strcpy(data, DEFAULT_VALUE);
    } else {
        nvs_get_str(nvs_handle, NVS_DATA_KEY, data, &required_size);
    }
    nvs_commit(nvs_handle);
    parseJson();    
    nvs_close(nvs_handle);

    nimble_port_init();
    // printf("B4 calling parseJson\n");
    parseJson();    
    // printf("after calling parseJson \n");
    // if (ssidName!=NULL){
    //     printf("ssidName: %s\n",ssidName);
    // }
    // if (myRemoteDeviceName!=NULL){
    //     printf("myRemoteDeviceName: %s\n",myRemoteDeviceName);
    // }
    // if (initialized!=NULL){
    //     printf("initialized: %s\n",initialized);
    // }
    // printf("end\n");
    // return ;
    // printf("myRemoteDeviceName: %s \n",myRemoteDeviceName);

    // if (! initialized){
    //     // write_default_value_to_nvs();
    //     printf("myRemoteDevice initialized:%s\n",initialized);
    // };
    ble_svc_gap_device_name_set(myRemoteDeviceName);
    // ble_svc_gap_device_name_set("MyRemoteDevice1");
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
}

void deleteDataFromNvs(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    size_t required_size;

    // Initialize NVS
    err = nvs_flash_init();
    if (err != ESP_OK) {
        printf("Error initializing NVS Flash: %s\n", esp_err_to_name(err));
        return;
    }

    // Open NVS
    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        printf("Error opening NVS: %s\n", esp_err_to_name(err));
        return;
    }
    if (nvs_get_str(nvs_handle, NVS_DATA_KEY, NULL, &required_size) == ESP_OK) {
        // Delete the NVS variable
        err = nvs_erase_key(nvs_handle, NVS_DATA_KEY);
        if (err != ESP_OK) {
            printf("Error deleting NVS variable: %s\n", esp_err_to_name(err));
        } else {
            printf("NVS variable '%s' deleted successfully.\n", NVS_DATA_KEY);
        }
    }else if (nvs_get_str(nvs_handle, NVS_DATA_KEY, NULL, &required_size) == ESP_ERR_NVS_NOT_FOUND) {
        printf("NVS variable '%s' not found.\n", NVS_DATA_KEY);
    }

    // Commit and close NVS
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}

void app2(void)
{
    deleteDataFromNvs();
}
