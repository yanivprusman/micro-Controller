// TCP Server with WiFi connection communication via Socket

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_mac.h"
#include "esp_eth.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_system.h"

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

#include "nvs_flash.h"
#include "ping/ping_sock.h"
#include "driver/gpio.h"
// #include "my_data.h"

#define PORT 3333
static const char *TAG = "TCP_SOCKET";

static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    char rx_buffer[128];
    char string_data[128];
    char data_to_send[128];
    int data_len, string_data_len;
    int keepAlive = 1;
    int keepIdle = 5;
    int keepInterval = 5;
    int keepCount = 3;
    struct sockaddr_storage dest_addr;

    int counter = 0;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);
    
    // Open socket
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0); // 0 for TCP Protocol
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    printf("error Socket created\n");

    bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    printf("error Socket bound, port %d\n", PORT);

    // Listen to the socket
    listen(listen_sock, 1);

    while (1)
    {
        printf("error Socket listening\n");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);

        // Accept socket
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0)
        {
            printf("error Unable to accept connection: errno %d\n", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET)
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }

        printf("error Socket accepted ip address: %s\n", addr_str);
        
        recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);

        strcpy(string_data,"Response from ESP32 Server via Socket connection");
        string_data_len = strlen(string_data);
        sprintf(data_to_send, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", string_data_len);
        strcat(data_to_send, string_data);
        data_len = strlen(data_to_send);

        // Send data via socket
        send(sock, data_to_send, data_len, 0);

        counter++;
        printf("send_reply function number %d was activated\n", counter);
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        shutdown(sock, 0);
        close(sock);
    }
    close(listen_sock);
    vTaskDelete(NULL);
}

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting WIFI_EVENT_STA_START ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected WIFI_EVENT_STA_CONNECTED ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection WIFI_EVENT_STA_DISCONNECTED ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&wifi_initiation);
    if(err !=ESP_OK){
        printf("error1: %s\n",esp_err_to_name(err));
    };

    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    if(err !=ESP_OK){
        printf("error2: %s\n",esp_err_to_name(err));
    };
    err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    if(err !=ESP_OK){
        printf("error3: %s\n",esp_err_to_name(err));
    };
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "pruguy",
            .password = "03071979"}};
    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    if(err !=ESP_OK){
        printf("error2: %s\n",esp_err_to_name(err));
    };
    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if(err !=ESP_OK){
        printf("error2: %s\n",esp_err_to_name(err));
    };
    err = esp_wifi_start();
    if(err !=ESP_OK){
        printf("error2: %s\n",esp_err_to_name(err));
    };

    err = esp_wifi_connect();
    if(err !=ESP_OK){
        printf("error2: %s\n",esp_err_to_name(err));
    };
}
static bool is_wifi_connected(void)
{
    wifi_ap_record_t ap_info;
    esp_err_t err=esp_wifi_sta_get_ap_info(&ap_info);
    if ( err!= ESP_OK) {
        printf("not connected: %s\n", esp_err_to_name(err));
        return false;
    }
    return true;
}

void wifi(void)
{
    if(!is_wifi_connected()){
        printf("connectig\n");
        wifi_connection();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
    // xTaskCreate(tcp_server_task, "tcp_server", 4096, (void *)AF_INET, 5, NULL);
}
