#include "time.h"
void initialize_sntp(void) {
    printf("Initializing SNTP\n");

    sntp_setoperatingmode(SNTP_OPMODE_POLL);  
    sntp_setservername(0, "pool.ntp.org");    
    sntp_init();                              
}

void obtain_time(void) {
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for 2 seconds
        time(&now);                      // Get current time
        localtime_r(&now, &timeinfo);    // Convert to local time
    }

    // Set timezone and print obtained time
    setenv("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1);  // Set timezone to Central European Time
    tzset();                                            // Apply timezone settings

    // Print obtained time
    printf("Current time: %s\n", asctime(&timeinfo));
}

void doTime(){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    initialize_sntp();

    obtain_time();

}