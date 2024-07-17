#include "ble.h"
uint8_t ble_addr_type;
void ble_app_advertise(void);
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

            printf("In uninitializedCallback Read request\n");
            printVariables();
            os_mbuf_append(ctxt->om, json_string, strlen(json_string));
            cJSON_Delete(json_response);
            free(json_string);
            return 0;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            printf("In uninitializedCallback write request\n");
            fillVariablseFromJsonString((char *) ctxt->om->om_data);
            initialized.value = INITIALIZED_TRUE_STRING;
            writeVariablesToNvs();
            printVariables();
            ble_app_advertise();
            return 0;

        default:
            return BLE_ATT_ERR_UNLIKELY;
    }
    return 0;
}
static int initializedCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
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

            printf("In initializedCallback Read request\n");
            printVariables();
            os_mbuf_append(ctxt->om, json_string, strlen(json_string));
            cJSON_Delete(json_response);
            free(json_string);
            return 0;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            printf("In initializedCallback write request\n");
            fillVariablseFromJsonString((char *) ctxt->om->om_data);
            writeVariablesToNvs();
            printVariables();
            ble_app_advertise();
            return 0;

        default:
            return BLE_ATT_ERR_UNLIKELY;
    }
    return 0;
}

static int initializedCallback2(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
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

    printf("in initializedCallback JSON Response: %s\n", json_string);
    os_mbuf_append(ctxt->om, json_string, strlen(json_string));

    cJSON_Delete(json_response);
    free(json_string);

    return 0;
}

static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = DEVICE_TYPE_UUID,                
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = UNINITIALIZED_CHAR_UUID,           // Define UUID for writing
          .flags = BLE_GATT_CHR_F_WRITE|BLE_GATT_CHR_F_READ,
          .access_cb = uninitializedCallback},
         {.uuid = INITIALIZED_CHAR_UUID,
          .flags = BLE_GATT_CHR_F_WRITE|BLE_GATT_CHR_F_READ,
          .access_cb = initializedCallback},
        {0}}},
    {0}
};

static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        printf("BLE GAP EVENT CONNECT %s\n", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        printf("BLE GAP EVENT\n");
        ble_app_advertise();
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        printf("BLE GAP EVENT DISCONNECT reason: %d\n", event->disconnect.reason);
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
    if (myRemoteDeviceName.value!=NULL){
        ble_svc_gap_device_name_set(myRemoteDeviceName.value);
    };
    device_name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    uint16_t company_id = 0xFFFA;
    uint8_t custom_data[4];
    if (initialized.value==NULL) initialized.value = INITIALIZED_FALSE_STRING;
    if (strcmp(initialized.value, INITIALIZED_TRUE_STRING) == 0){
        printf("my remote device id:%d\n",atoi(myRemoteDeviceID.value));
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

void nimble(){
    nimble_port_init();
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
}
