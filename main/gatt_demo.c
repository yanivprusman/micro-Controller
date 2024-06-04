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

char *TAG = "BLE-Server tag ";
uint8_t ble_addr_type;
void ble_app_advertise(void);

#define NVS_DATA_KEY "data"
#define DEFAULT_VALUE "{\"ssidName\":\"defaultSSID\",\"ssidPassword\":\"defaultPass\",\"remoteDeviceName\":\"defaultDevice\"}"
#define MAX_DATA_LENGTH 100  

char data[MAX_DATA_LENGTH + 1];  // Buffer to store data (+1 for null terminator)

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
    // Handle read operation
    os_mbuf_append(ctxt->om, data, strlen(data));
    printf("data from the esp storage: %s\n", data);
    return 0;
}

void write_default_value_to_nvs(nvs_handle_t nvs_handle) {
    esp_err_t err = nvs_set_str(nvs_handle, NVS_DATA_KEY, DEFAULT_VALUE);
    if (err != ESP_OK) {
        printf("Error writing default value: %s\n", esp_err_to_name(err));
        return;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        printf("Error committing default value: %s\n", esp_err_to_name(err));
        return;
    }

    printf("Default value written successfully.\n");
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
    device_name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
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

void app_main()
{
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
    nvs_close(nvs_handle);

    nimble_port_init();
    ble_svc_gap_device_name_set("MyRemoteDevice");
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
}
