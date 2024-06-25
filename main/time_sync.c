/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "time_sync.h"

#define STORAGE_NAMESPACE "storage"
#define TIME_PERIOD (86400000000ULL)

void initialize_sntp(void)
{
    printf("Initializing SNTP\n");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(2,
                               ESP_SNTP_SERVER_LIST("time.windows.com", "pool.ntp.org" ) );
    esp_netif_sntp_init(&config);
}

static esp_err_t obtain_time(void)
{
    int retry = 0;
    const int retry_count = 10;
    while (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(2000)) != ESP_OK && ++retry < retry_count) {
        printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
    }
    if (retry == retry_count) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t fetch_and_store_time_in_nvs(void *args)
{
    nvs_handle_t my_handle = 0;
    esp_err_t err;

    initialize_sntp();
    if (obtain_time() != ESP_OK) {
        err = ESP_FAIL;
        goto exit;
    }

    time_t now;
    time(&now);

    //Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        goto exit;
    }

    //Write
    err = nvs_set_i64(my_handle, "timestamp", now);
    if (err != ESP_OK) {
        goto exit;
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        goto exit;
    }

exit:
    if (my_handle != 0) {
        nvs_close(my_handle);
    }
    esp_netif_sntp_deinit();

    if (err != ESP_OK) {
        printf("Error updating time in nvs\n");
    } else {
        printf("Updated time in NVS\n");
    }
    return err;
}

esp_err_t update_time_from_nvs(void)
{
    nvs_handle_t my_handle = 0;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error opening NVS\n");
        goto exit;
    }

    int64_t timestamp = 0;

    err = nvs_get_i64(my_handle, "timestamp", &timestamp);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        printf("Time not found in NVS. Syncing time from SNTP server.\n");
        if (fetch_and_store_time_in_nvs(NULL) != ESP_OK) {
            err = ESP_FAIL;
        } else {
            err = ESP_OK;
        }
    } else if (err == ESP_OK) {
        struct timeval get_nvs_time;
        get_nvs_time.tv_sec = timestamp;
        settimeofday(&get_nvs_time, NULL);
    }

exit:
    if (my_handle != 0) {
        nvs_close(my_handle);
    }
    return err;
}
void doTime(){
    esp_err_t err = esp_reset_reason();
    if (err == ESP_RST_POWERON) {
        printf("Updating time from NVS\n");
    }
    err = update_time_from_nvs();
    if (err != ESP_OK) {
        printf("error 1: %s\n",esp_err_to_name(err));
    }

    const esp_timer_create_args_t nvs_update_timer_args = {
        .callback = (void *)&fetch_and_store_time_in_nvs,
    };

    esp_timer_handle_t nvs_update_timer;
    err = esp_timer_create(&nvs_update_timer_args, &nvs_update_timer);
    if (err != ESP_OK) {
        printf("error 2: %s\n",esp_err_to_name(err));
    }

    err = esp_timer_start_periodic(nvs_update_timer, TIME_PERIOD);
    if (err != ESP_OK) {
        printf("error 3: %s\n",esp_err_to_name(err));
    }
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%c", timeinfo);
    printf("Current time: %s\n", time_str);

}